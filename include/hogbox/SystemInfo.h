#pragma once

#include <hogbox/Export.h>
#include <hogbox/HogBoxBase.h>
//#include <hogbox/Singleton.h>

#include <osgViewer/Viewer>
#include <osg/GLExtensions>
#include <osg/GL2Extensions>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>

#include <osg/Multisample>

#include <iostream>

namespace hogbox {

//
//Gathers infomation on avaliable dedicated video memory (currently nvidia only)
//
class GPUMemoryCallback : public osg::Camera::DrawCallback
{
public:
	GPUMemoryCallback()
		: osg::Camera::DrawCallback(),
		m_totalDedicatedVideoMemory(0),
		m_avavliableMemory(0),
		m_avavliableDedicatedVideoMemory(0)
	{
	}

	virtual void operator () (osg::RenderInfo& renderInfo) const
	{
	   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

	   unsigned int contextID = renderInfo.getContextID();
	  
   		osg::GL2Extensions* extensions = osg::GL2Extensions::Get(contextID,true);

		bool nvidiaMemoryInfoSupport = osg::isGLExtensionSupported(contextID,"GL_NVX_gpu_memory_info");
		if(nvidiaMemoryInfoSupport)
		{
			//try calling the nvidia versions ( http://developer.download.nvidia.com/opengl/specs/GL_NVX_gpu_memory_info.txt)
			//GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
			//GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
			//GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
			//GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
			//GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B
			int dedVideoMem=0,totalAvailableMem=0, curAvailableMem=0, evictionCount=0, evictedMem=0;
			glGetIntegerv(0x9047, &dedVideoMem);
			glGetIntegerv(0x9048, &totalAvailableMem);
			glGetIntegerv(0x9049, &curAvailableMem);
			glGetIntegerv(0x904A, &evictionCount);
			glGetIntegerv(0x904B, &evictedMem);

			m_totalDedicatedVideoMemory = dedVideoMem;
			m_avavliableMemory = totalAvailableMem;
			m_avavliableDedicatedVideoMemory = curAvailableMem;
		
		}else{

			//try the ati version ( http://www.opengl.org/registry/specs/ATI/meminfo.txt )
			bool atiMemoryInfoSupport = osg::isGLExtensionSupported(contextID,"GL_ATI_meminfo");
			if(atiMemoryInfoSupport)
			{
				//ati returns totally different stuff, 

				//were going to only handle texture memory for now and just hack it into the current variales
				//to get an idea of whats avaliable
				//TEXTURE_FREE_MEMORY_ATI                 0x87FC
				GLint textureFreeMemoryInfo[4];
				glGetIntegerv(0x87FC, textureFreeMemoryInfo);

				m_totalDedicatedVideoMemory = textureFreeMemoryInfo[2]; //total auxiliary memory free (close :) )
				m_avavliableMemory = m_totalDedicatedVideoMemory;
				m_avavliableDedicatedVideoMemory = textureFreeMemoryInfo[0]; //total memory free in the pool
			}
		}
	}

	//total memory on the card kb
	const int totalDedicatedVideoMemory(){return m_totalDedicatedVideoMemory;}

	//total memory avaliable including ram kb
	const int avaliableMemory(){return m_avavliableMemory;}

	//total memory avaliable on the card kb 
	const int avaliableDedicatedVideoMemory(){return m_avavliableDedicatedVideoMemory;}

protected:
    mutable OpenThreads::Mutex  _mutex;

	//total memory on the card kb
	mutable int m_totalDedicatedVideoMemory;

	//total memory avaliable including ram kb
	mutable int m_avavliableMemory;

	//total memory avaliable on the card kb 
	mutable int m_avavliableDedicatedVideoMemory;
};


//
//Use the draw callback to get hold of the glcontext and query for supported extensions
//
class GLSupportCallback : public osg::Camera::DrawCallback
{
public:
		GLSupportCallback()
			: osg::Camera::DrawCallback(),
			_rendererName(""),
			_quadBufferedStereoSupported(false),
			_multiSamplingSupported(false),
			_maxMultiSamples(0),
			_bStencilBufferedSupported(false),
			_glslLangSupported(false),
			_shaderObjectSupported(false),
			_gpuShader4Supported(false),
			_vertexShadersSupported(false),
			_fragmentShadersSupported(false),
			_maxTex2DSize(0),
			_texRectangleSupported(false),
			_maxTextureUnits(0),
			_maxFragmentTextureUnits(0),
			_maxVertexTextureUnits(0),
			_maxGeometryTextureUnits(0),
			_totalTextureUnits(0),
			_maxTextureCoordUnits(0),
			_frameBufferObjectSupported(false)
		{
        }

		virtual void operator () (osg::RenderInfo& renderInfo) const
        {
           OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

		   unsigned int contextID = renderInfo.getContextID();
		  
		   	osg::GL2Extensions* extensions = osg::GL2Extensions::Get(contextID,true);

		   //get gl versions
			_glVersionNumber = osg::getGLVersionNumber();

			// get the renderer ID
			const GLubyte* renderer = glGetString(GL_RENDERER);
			_rendererName = renderer ? (const char*)renderer : "";

			//get vendor id
			const GLubyte* vendor = glGetString(GL_VENDOR);
			_vendorName = vendor ? (const char*)vendor : "";
			
		//MULTISAMPLING
			//Multisampling, should alreay be known based of the creation of the graphics context
			//but this will confirm it worked correctly

			_multiSamplingSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_multisample");//GL_ARB_multisample");
			//if(!_multiSamplingSupported){_multiSamplingSupported = osg::isGLExtensionSupported(contextID,"GL_NV_multisample_filter_hint");}

			//if it is supported also checl for max samples
			if (_multiSamplingSupported)
			{
				GLint iMaxSamples;
				glGetIntegerv( GL_SAMPLES_ARB, &iMaxSamples);
				_maxMultiSamples = (int)iMaxSamples;

			}else{
				_maxMultiSamples = 0;
			}

		//STENCIL BUFFER SUPPORT

			//stencil buffer support
			GLboolean bStencilAvailable;
			glGetBooleanv (GL_STENCIL, &bStencilAvailable);

			_bStencilBufferedSupported = bStencilAvailable!=0;


		//STEREO

			GLboolean bStereoAvailable;
			glGetBooleanv (GL_STEREO, &bStereoAvailable);

			_quadBufferedStereoSupported = bStereoAvailable!=0;

		//GLSL SHADER SUPPORT

			_glslLangSupported = extensions->isGlslSupported();

			_glslVersionNumber = extensions->getLanguageVersion();

			_shaderObjectSupported = extensions->isShaderObjectsSupported();

			_gpuShader4Supported = extensions->isGpuShader4Supported();

			_vertexShadersSupported = extensions->isVertexShaderSupported();
			_fragmentShadersSupported = extensions->isFragmentShaderSupported();
			_geometryShadersSupported = extensions->isGeometryShader4Supported();

		//TEXTURES

			//max fixed func tex size
			GLint maxTextureSize = 0;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
			_maxTex2DSize = (int)maxTextureSize;

			//texture rectangle support
			_texRectangleSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_texture_rectangle") ||
									osg::isGLExtensionSupported(contextID,"GL_EXT_texture_rectangle") ||
									osg::isGLExtensionSupported(contextID,"GL_NV_texture_rectangle");

			//max texture units (fixed function)
			GLint maxTextureUnits = 0;
			glGetIntegerv(GL_MAX_TEXTURE_UNITS , &maxTextureUnits);
			_maxTextureUnits = (int)maxTextureUnits;

			//max texture units avaliable to vertex shader
			GLint maxVertTextureUnits = 0;
			glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS , &maxVertTextureUnits);
			_maxVertexTextureUnits = (int)maxVertTextureUnits;

			//max texture units avaliable to fragment shader
			GLint maxFragTextureUnits = 0;
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS , &maxFragTextureUnits);
			_maxFragmentTextureUnits = (int)maxFragTextureUnits;

			//max texture units avaliable to geometry shader
			GLint maxGeomTextureUnits = 0;
			glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT , &maxGeomTextureUnits);
			_maxGeometryTextureUnits = (int)maxGeomTextureUnits;

			//total combined units
			GLint totalTextureUnits = 0;
			glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS , &totalTextureUnits);
			_totalTextureUnits = (int)totalTextureUnits;

			//max texture coord channels avaliable
			GLint maxTextureCoordUnits = 0;
			glGetIntegerv(GL_MAX_TEXTURE_COORDS , &maxTextureCoordUnits);
			_maxTextureCoordUnits = (int)maxTextureCoordUnits;

			
		//FrameBuffer objects
			_frameBufferObjectSupported = osg::isGLExtensionSupported(contextID,"GL_EXT_framebuffer_object");
			
	}

	//gets
	const float getGLVersionNumber(){return _glVersionNumber;}
	const float getGLSLVersionNumber(){return _glslVersionNumber;}

	const std::string getRendererName(){return _rendererName;}
	const std::string getVendorName(){return _vendorName;}

	const bool quadBufferedStereoSupported(){ return _quadBufferedStereoSupported;}

	const bool multiSamplingSupported(){ return _multiSamplingSupported;}
	const int  maxMultiSamplesSupported(){return _maxMultiSamples;}

	const bool stencilBufferSupported(){return _bStencilBufferedSupported;}

	const bool glslLangSupported(){return _glslLangSupported;}
	const bool shaderObjectSupported(){return _shaderObjectSupported;}
	const bool gpuShader4Supported(){return _gpuShader4Supported;}
	const bool vertexShadersSupported(){ return _vertexShadersSupported;}
	const bool fragmentShadersSupported(){ return _fragmentShadersSupported;}
	const bool geometryShadersSupported(){ return _geometryShadersSupported;}

	const int maxTex2DSize(){ return _maxTex2DSize;}
	const bool textureRectangleSupported(){ return _texRectangleSupported;}

	//fixed function limit
	const int maxTextureUnits(){return _maxTextureUnits;}

	//shader limits
	const int maxVertexTextureUnits(){return _maxFragmentTextureUnits;}
	const int maxFragmentTextureUnits(){return _maxFragmentTextureUnits;}
	const int maxGeometryTextureUnits(){return _maxGeometryTextureUnits;}

	//is a total across all types 
	const int totalTextureUnitsAvaliable(){return _totalTextureUnits;}

	const int maxTextureCoordUnits(){return _maxTextureCoordUnits;}


	const bool frameBufferObjectSupported(){return _frameBufferObjectSupported;}


protected:

    mutable OpenThreads::Mutex  _mutex;

	mutable float _glVersionNumber;
	mutable float _glslVersionNumber;

	mutable std::string _rendererName;
	mutable std::string _vendorName;

	//buffers

	mutable bool _quadBufferedStereoSupported;

	mutable bool _multiSamplingSupported;
	mutable int _maxMultiSamples;

	mutable bool _bStencilBufferedSupported;
	
	//shaders

	mutable bool _glslLangSupported;
	mutable bool _shaderObjectSupported;
	mutable bool _gpuShader4Supported;
	mutable bool _vertexShadersSupported;
	mutable bool _fragmentShadersSupported;
	mutable bool _geometryShadersSupported;

	//texturing

	mutable int  _maxTex2DSize;
	mutable bool _texRectangleSupported;

	mutable int _maxTextureUnits;
	mutable int _maxVertexTextureUnits;
	mutable int _maxFragmentTextureUnits;
	mutable int _maxGeometryTextureUnits;
	mutable int _totalTextureUnits;

	mutable int _maxTextureCoordUnits;

	//framebufers / render to texture stuff 

	mutable bool _frameBufferObjectSupported;

	//opengl memory info

};

//Wraps a desription of the system functionality required to support
//a certain feature for example shaders. The SystemFeatureLevel is referenced
//by a simple friendly name to help users not have to worry about specifics
//and create general target levels i.e. high, medium and low
class SystemFeatureLevel : public osg::Object
{
public:
	SystemFeatureLevel(void) 
		: osg::Object(),
		glVersion(0.0f),
		glslVersion(0.0f),
		textureUnits(0),
		textureCoordUnits(0),
		vertexAndFragmentShaders(false),
		geometryShaders(false),
		screenRes(0,0)
	{
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	SystemFeatureLevel(const SystemFeatureLevel& level,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(level,copyop)
	{}

	META_Box(hogbox, SystemFeatureLevel);

protected:

	 virtual ~SystemFeatureLevel(){}

public:

	//minimum gl requirements, if zero then not required
	float glVersion;
	float glslVersion;

	int textureUnits;
	int textureCoordUnits;

	bool vertexAndFragmentShaders;
	bool geometryShaders;

	//minimum screen resolution
	osg::Vec2 screenRes;

};

typedef osg::ref_ptr<SystemFeatureLevel> SystemFeatureLevelPtr;

//
// Singleton info class, to allow all classes to access the supported extension info
//
class HOGBOX_EXPORT SystemInfo : public osg::Referenced //, public hogbox::Singleton<SystemInfo>
{
public:

	//friend hogbox::Singleton<SystemInfo>;

	static SystemInfo* Instance(bool erase = false);

	//Screens
	const unsigned int getNumberOfScreens();
	const int getScreenWidth(unsigned int screenID=0);
	const int getScreenHeight(unsigned int screenID=0);
	const osg::Vec2 getScreenResolution(unsigned int screenID=0);
	const double getScreenRefreshRate(unsigned int screenID=0);
	const int getScreenColorDepth(unsigned int screenID=0);

	const double getScreenAspectRatio(unsigned int screenID=0);

	bool setScreenResolution(osg::Vec2 pixels, unsigned int screenID=0);
	bool setScreenRefreshRate(double hz, unsigned int screenID=0);
	bool setScreenColorDepth(unsigned int depth, unsigned int screenID=0);

	//Gl Version
	const float getGLVersionNumber(){return m_renderSupportInfo->getGLVersionNumber();}
	const float getGLSLVersionNumber(){return m_renderSupportInfo->getGLSLVersionNumber();}

	//Renderer info (Graphics Card)
	const std::string getRendererName(){return m_renderSupportInfo->getRendererName();}
	const std::string getVendorName(){return m_renderSupportInfo->getVendorName();}

	//Buffer Support
    const bool doubleBufferSupported(){ return m_maxTestedBuffers >= 2;}

	const bool quadBufferedStereoSupported(){ return m_renderSupportInfo->quadBufferedStereoSupported() && m_maxTestedBuffers == 4;}

	const bool depthBufferSupported(){return m_maxTestedDepthBits > 0;} //TODO
	const int maxDepthBufferBits(){ return m_maxTestedDepthBits;}

	const bool stencilBufferSupported(){return m_renderSupportInfo->stencilBufferSupported() && m_maxTestedStencilBits > 0;}
	const unsigned int& maxStencilBitsSupported(){return m_maxTestedStencilBits;}

	//multispampling uses a combination of gl info and actual test data
	const bool multiSamplingSupported(){ return m_renderSupportInfo->multiSamplingSupported() && m_maxTestedMultiSamples > 0;}
	const int  maxMultiSamplesSupported(){return m_maxTestedMultiSamples;}

	//shader support

	//helper to determin a general state for shader support
	const bool AreShadersSupported(){return glslSupported() &&
										shaderObjectSupported() &&
										vertexShadersSupported() &&
										fragmentShadersSupported();}

	const bool glslSupported(){return m_renderSupportInfo->glslLangSupported();}

	const bool shaderObjectSupported(){return m_renderSupportInfo->shaderObjectSupported();}
	const bool gpuShader4Supported(){return m_renderSupportInfo->gpuShader4Supported();}
	const bool vertexShadersSupported(){return m_renderSupportInfo->vertexShadersSupported();}
	const bool fragmentShadersSupported(){return m_renderSupportInfo->fragmentShadersSupported();}
	const bool geometryShadersSupported(){return m_renderSupportInfo->geometryShadersSupported();}

	//texturing
	const int maxTexture2DSize(){ return m_renderSupportInfo->maxTex2DSize();}

	const bool textureRectangleSupported(){ 
		#ifdef WIN32
			return m_renderSupportInfo->textureRectangleSupported();
		#else
			return false;//osx is returing true, but texrect isn't working
		#endif
	}

	//number of fixed function units
	const int maxTextureUnits(){return m_renderSupportInfo->maxTextureUnits();}

	//number of shader units
	const int maxVertexTextureUnits(){return m_renderSupportInfo->maxVertexTextureUnits();}
	const int maxFragmentTextureUnits(){return m_renderSupportInfo->maxFragmentTextureUnits();}
	const int maxGeometryTextureUnits(){return m_renderSupportInfo->maxGeometryTextureUnits();}
	const int totalTextureUnitsAvaliable(){return m_renderSupportInfo->totalTextureUnitsAvaliable();}
	
	//number of texture coord channels	
	const int maxTextureCoordUnits(){return m_renderSupportInfo->maxTextureCoordUnits();}

	//framebuffer object support
	const bool frameBufferObjectSupported(){return m_renderSupportInfo->frameBufferObjectSupported();}

	//video memory info

	//total memory on the card kb
	const int totalDedicatedVideoMemory(){return m_memoryInfo->totalDedicatedVideoMemory();}

	//total memory avaliable including ram kb
	const int avaliableMemory(){return m_memoryInfo->avaliableMemory();}

	//total memory avaliable on the card kb 
	const int avaliableDedicatedVideoMemory(){return m_memoryInfo->avaliableDedicatedVideoMemory();}

	
	//simplified supported feature level info

	//add a new feature level by name, if feature level already exists, it
	//will be overriden
	bool SetFeatureLevel(const std::string& name, SystemFeatureLevel* featureLevel);

	//get featurelevel by name, returns null if not in list
	SystemFeatureLevel* GetFeatureLevel(const std::string& name);

	//compare a feature level to the current system infomation
	//return true if the system info matches or exceeds the feature level
	//return false is the system info is below the feature level
	bool IsFeatureLevelSupported(SystemFeatureLevel* featureLevel);
	//same but by name
	bool IsFeatureLevelSupported(const std::string& name);

	//
	void PrintReportToLog();

protected:

	//constructor is protected for singleton 
	SystemInfo(void);
	~SystemInfo(void);

	virtual void destruct(){
		m_renderSupportInfo = NULL;
	}

	//
	//colects the system infomation, should be called when the single instance
	//is first created
	int Init(bool printReport = true);

	//
	//keeps trying various contexts till it finds the max values for each trait
	//
	osg::ref_ptr<osg::GraphicsContext> FindGoodContext();

	//the below find functions are passed a pointer to a Traits struct created in FindGoodContext
	//they apply the maximum working values to Traits struct and also return it as a value
	//they will also disable any unsupported traits

	// try out double buffering and report the result
	bool FindDoubleBufferTraits(osg::GraphicsContext::Traits* currentTraits);
	// try out quad buffering and report
	bool FindQuadBufferedTraits(osg::GraphicsContext::Traits* currentTraits);
	// find the max number of depth buffer bits
	unsigned int FindDepthTraits(osg::GraphicsContext::Traits* currentTraits, unsigned int bits);
	// find the max number of stencil bits
	unsigned int FindStencilTraits(osg::GraphicsContext::Traits* currentTraits, unsigned int bits);
	//fin the max number of multi samples supported
	unsigned int FindMultiSamplingTraits(osg::GraphicsContext::Traits* currentTraits, unsigned int samples);

private:

	//these represent the max bits achived in a real world
	//test (via the FindGoodContext function)
	unsigned int m_maxTestedBuffers;
	unsigned int m_maxTestedStencilBits;
	unsigned int m_maxTestedDepthBits;
	unsigned int m_maxTestedMultiSamples;

	//callback attched to a render viewer camera to gather gl info
	osg::ref_ptr<GLSupportCallback> m_renderSupportInfo;

	osg::ref_ptr<GPUMemoryCallback> m_memoryInfo;

	//the store map of registered SystemFeatureLevels to their friendly names. 
	//The friendly name/uniqueID can then be used by material etc to describe the
	//features they need
	std::map<std::string, SystemFeatureLevelPtr> m_featureLevels;
};


};//end hogbox namespace