/* Written by Thomas Hogarth, (C) 2011
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 */

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
    
//device strings
#define IPHONE_SIM_STR	"i386"	 //480×320 res
#define IPHONE_BASE_STR "iPhone1,1"	//480×320 res
#define IPHONE_3G_STR	"iPhone1,2"	//480×320 res
#define IPHONE_3GS_STR  "iPhone2,1"	//480×320 res
#define IPHONE_4G_STR	"iPhone3,1"	//960×640 res (retina display)
#define IPHONE_4S_STR	"iPhone4,1"	//960×640 res (retina display)
#define IPOD_GEN1_STR	"iPod1,1"	 //480×320 res
#define IPOD_GEN2_STR	"iPod2,1"	 //480×320 res
#define IPOD_GEN3_STR	"iPod3,1"	 //480×320 res
#define IPOD_GEN4_STR	"iPod4,1"	 //960×640 res (retina display)
#define IPAD1_STR	    "iPad1,1"	 //1024×768 res (hd)
#define IPAD2_WIFI_STR  "iPad2,1"
#define IPAD2_STR	    "iPad2,3"	 //1024×768 res (hd)
	
//Need to define GL_STENCIL and GL_STEREO an IPhone
#if defined( __APPLE__ )
	
	#include "TargetConditionals.h"
	#if (TARGET_OS_IPHONE) || (TARGET_IPHONE_SIMULATOR) //only when on device
		#define GL_STENCIL 1
		#define GL_STEREO 1
	#endif
#endif
	
//
//Gathers infomation on available dedicated video memory (currently nvidia only)
//
class GPUMemoryCallback : public osg::Camera::DrawCallback
{
public:
	GPUMemoryCallback()
		: osg::Camera::DrawCallback(),
		_totalDedicatedVideoMemory(0),
		_availableMemory(0),
		_availableDedicatedVideoMemory(0)
	{
	}

	virtual void operator () (osg::RenderInfo& renderInfo) const
	{
	   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

	   unsigned int contextID = renderInfo.getContextID();
	  
   		//osg::GL2Extensions* extensions = osg::GL2Extensions::Get(contextID,true);

		bool nvidiaMemoryInfoSupport = osg::isGLExtensionSupported(contextID,"GL_NVX_gpu_memory_info");
		if(nvidiaMemoryInfoSupport)
		{
			//try calling the nvidia versions ( http://developer.download.nvidia.com/opengl/specs/GL_NVX_gpu_memory_info.txt)
			//GPU_MEMORY_INFO_DEDICATED_VIDME_NVX          0x9047
			//GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
			//GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDME_NVX  0x9049
			//GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
			//GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B
			int dedVideoMem=0,totalAvailableMem=0, curAvailableMem=0, evictionCount=0, evictedMem=0;
			glGetIntegerv(0x9047, &dedVideoMem);
			glGetIntegerv(0x9048, &totalAvailableMem);
			glGetIntegerv(0x9049, &curAvailableMem);
			glGetIntegerv(0x904A, &evictionCount);
			glGetIntegerv(0x904B, &evictedMem);

			_totalDedicatedVideoMemory = dedVideoMem;
			_availableMemory = totalAvailableMem;
			_availableDedicatedVideoMemory = curAvailableMem;
		
		}else{

			//try the ati version ( http://www.opengl.org/registry/specs/ATI/meminfo.txt )
			bool atiMemoryInfoSupport = osg::isGLExtensionSupported(contextID,"GL_ATI_meminfo");
			if(atiMemoryInfoSupport)
			{
				//ati returns totally different stuff, 

				//were going to only handle texture memory for now and just hack it into the current variales
				//to get an idea of whats available
				//TEXTURE_FREE_MEMORY_ATI                 0x87FC
				GLint textureFreeMemoryInfo[4];
				glGetIntegerv(0x87FC, textureFreeMemoryInfo);

				_totalDedicatedVideoMemory = textureFreeMemoryInfo[2]; //total auxiliary memory free (close :) )
				_availableMemory = _totalDedicatedVideoMemory;
				_availableDedicatedVideoMemory = textureFreeMemoryInfo[0]; //total memory free in the pool
			}
		}
	}

	//total memory on the card kb
	const int totalDedicatedVideoMemory(){return _totalDedicatedVideoMemory;}

	//total memory available including ram kb
	const int availableMemory(){return _availableMemory;}

	//total memory available on the card kb 
	const int availableDedicatedVideoMemory(){return _availableDedicatedVideoMemory;}

protected:
    mutable OpenThreads::Mutex  _mutex;

	//total memory on the card kb
	mutable int _totalDedicatedVideoMemory;

	//total memory available including ram kb
	mutable int _availableMemory;

	//total memory available on the card kb 
	mutable int _availableDedicatedVideoMemory;
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
            _supportsNPOTTexture(false),
			_maxTextureUnits(0),
			_maxVertexTextureUnits(0),
            _maxFragmentTextureUnits(0),
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
			osg::Texture::Extensions* texExtensions = osg::Texture::getExtensions(contextID,true);
			
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

			//npot texturing
			_supportsNPOTTexture = texExtensions->isNonPowerOfTwoTextureSupported(osg::Texture::NEAREST);
				
			
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

			//max texture units available to vertex shader
			GLint maxVertTextureUnits = 0;
			glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS , &maxVertTextureUnits);
			_maxVertexTextureUnits = (int)maxVertTextureUnits;

			//max texture units available to fragment shader
			GLint maxFragTextureUnits = 0;
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS , &maxFragTextureUnits);
			_maxFragmentTextureUnits = (int)maxFragTextureUnits;

			//max texture units available to geometry shader
			GLint maxGeomTextureUnits = 0;
			glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT , &maxGeomTextureUnits);
			_maxGeometryTextureUnits = (int)maxGeomTextureUnits;

			//total combined units
			GLint totalTextureUnits = 0;
			glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS , &totalTextureUnits);
			_totalTextureUnits = (int)totalTextureUnits;

			//max texture coord channels available
			GLint maxTextureCoordUnits = 0;
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
			glGetIntegerv(GL_MAX_TEXTURE_COORDS , &maxTextureCoordUnits);
#endif
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
	const bool npotTextureSupported(){return _supportsNPOTTexture;}
	const bool textureRectangleSupported(){ return _texRectangleSupported;}

	//fixed function limit
	const int maxTextureUnits(){return _maxTextureUnits;}

	//shader limits
	const int maxVertexTextureUnits(){return _maxFragmentTextureUnits;}
	const int maxFragmentTextureUnits(){return _maxFragmentTextureUnits;}
	const int maxGeometryTextureUnits(){return _maxGeometryTextureUnits;}

	//is a total across all types 
	const int totalTextureUnitsAvailable(){return _totalTextureUnits;}

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

	mutable bool _supportsNPOTTexture;
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

	META_Object(hogbox, SystemFeatureLevel);

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

	//
	//System info can perform a few different favours of info gather
	//
	enum GatherLevel
	{
		FULL, //perfrom all gathers incuding the physical tests of the contexts
		GL_GATHER, //gather info with glGetString etc using the GLSupportCallback (will launch a viewer for a single frame)
		CONFIG, //use an external config file to set the system info
		DEFAULTS //nothing is gathered from the system and some reasonable defaults are set targeting gl 1.x and intel cards
	};
	
    //screen density used for loading diferent asset resolutions
    enum ScreenDensity{
        LOW_DENSITY = 0, //480x320
        MEDIUM_DENSITY =  1, //960X640
        HIGH_DENSITY = 2, //1080p
    };
    
	//friend hogbox::Singleton<SystemInfo>;

	static SystemInfo* Inst(GatherLevel level = FULL, bool erase = false);
		
	//
	//Creates a gl context from the current info settings
	osg::ref_ptr<osg::GraphicsContext> CreateGLContext(osg::Vec2 size);

	//Screens
	const unsigned int getNumberOfScreens();
	const int getScreenWidth(unsigned int screenID=0);
	const int getScreenHeight(unsigned int screenID=0);
	const osg::Vec2 getScreenResolution(unsigned int screenID=0);
	const double getScreenRefreshRate(unsigned int screenID=0);
	const int getScreenColorDepth(unsigned int screenID=0);

	const double getScreenAspectRatio(unsigned int screenID=0);

    //get the screens density/res group, used for loading different assets on different displays
    ScreenDensity getScreenDensity();
    
    //Seting or screen resolution is supported on some platforms
	bool setScreenResolution(osg::Vec2 pixels, unsigned int screenID=0);
	bool setScreenRefreshRate(double hz, unsigned int screenID=0);
	bool setScreenColorDepth(unsigned int depth, unsigned int screenID=0);

	//Gl Version
	const float getGLVersionNumber(){return _glVersionNumber;}
	const float getGLSLVersionNumber(){return _glslVersionNumber;}

	//Renderer info (Graphics Card)
	const std::string getRendererName(){return _rendererName;}
	const std::string getVendorName(){return _vendorName;}

	//Buffer Support
    const bool doubleBufferSupported(){ return _maxTestedBuffers>= 2;}

	const bool quadBufferedStereoSupported(){ return _quadBufferedStereoSupported && _maxTestedBuffers== 4;}

	const bool depthBufferSupported(){return _maxTestedDepthBits> 0;} //TODO
	const unsigned int maxDepthBufferBits(){ return _maxTestedDepthBits;}

	const bool stencilBufferSupported(){return _bStencilBufferedSupported && _maxTestedStencilBits> 0;}
	const unsigned int& maxStencilBitsSupported(){return _maxTestedStencilBits;}

	//multispampling uses a combination of gl info and actual test data
	const bool multiSamplingSupported(){ return _multiSamplingSupported && _maxTestedMultiSamples> 0;}
	const int  maxMultiSamplesSupported(){return _maxTestedMultiSamples;}

	//shader support

	//helper to determin a general state for shader support
	const bool AreShadersSupported(){return glslSupported() &&
										shaderObjectSupported() &&
										vertexShadersSupported() &&
										fragmentShadersSupported();}

	const bool glslSupported(){return _glslLangSupported;}

	const bool shaderObjectSupported(){return _shaderObjectSupported;}
	const bool gpuShader4Supported(){return _gpuShader4Supported;}
	const bool vertexShadersSupported(){return _vertexShadersSupported;}
	const bool fragmentShadersSupported(){return _fragmentShadersSupported;}
	const bool geometryShadersSupported(){return _geometryShadersSupported;}

	//texturing
	const bool npotTextureSupported(){return _supportsNPOTTexture;}
	
	const int maxTexture2DSize(){ return _maxTex2DSize;}

	const bool textureRectangleSupported(){ 
		#ifdef WIN32
			return _texRectangleSupported;
		#else
			return false;//osx is returing true, but texrect isn't working
		#endif
	}

	//number of fixed function units
	const int maxTextureUnits(){return _maxTextureUnits;}

	//number of shader units
	const int maxVertexTextureUnits(){return _maxVertexTextureUnits;}
	const int maxFragmentTextureUnits(){return _maxFragmentTextureUnits;}
	const int maxGeometryTextureUnits(){return _maxGeometryTextureUnits;}
	const int totalTextureUnitsAvailable(){return _totalTextureUnits;}
	
	//number of texture coord channels	
	const int maxTextureCoordUnits(){return _maxTextureCoordUnits;}

	//framebuffer object support
	const bool frameBufferObjectSupported(){return _frameBufferObjectSupported;}

	//video memory info (these only really make sense on nvidia cards at the moment)

	//total memory on the card kb
	const int totalDedicatedGLMemory(){return _totalDedicatedGLMemory;}

	//total memory available including ram kb
	const int availableGLMemory(){return _availableGLMemory;}

	//total memory available on the card kb 
	const int availableDedicatedGLMemory(){return _avliableDedicatedGLMemory;}
	
	//simplified supported feature level info

	//add a new feature level by name, if feature level already exists, it
	//will be overwritten
	bool SetFeatureLevel(const std::string& name, SystemFeatureLevel* featureLevel);

	//get featurelevel by name, returns null if not in list
	SystemFeatureLevel* GetFeatureLevel(const std::string& name);

	//compare a feature level to the current system infomation
	//return true if the system info matches or exceeds the feature level
	//return false is the system info is below the feature level
	bool IsFeatureLevelSupported(SystemFeatureLevel* featureLevel);
	//compare one of the stored feature levels
	bool IsFeatureLevelSupported(const std::string& name);

    
    //determine device type
    
    //
    //Return the device id string
    const std::string GetDeviceString();

    //
    //File path helpers
    
    //Return the systems documents path, which should be safe for
    //saving user generated documents
    const std::string GetDocumentPath();
    
	//
	void PrintReportToLog();

protected:

	//constructor is protected for singleton 
	SystemInfo(GatherLevel level = FULL);
	~SystemInfo(void);

	virtual void destruct(){
		_renderSupportInfo = NULL;
	}

	//
	//colects the system infomation, should be called when the single instance
	//is first created
	int Init(bool printReport = true);
	
	//
	//Load system info from an xml config file, returns true if used
	bool SetSystemInfoFromConfig(const std::string& config);
	
	//
	//Set the local glSystemInfo from the GL gathers
	//will also fire off the gather if it hasn't been done yet (will create a viewer)
	bool SetGLSystemInfoFromGather(osg::ref_ptr<osg::GraphicsContext> graphicsContext);

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
	
	//gather level if not default (FULL) then needs to be set on very first call to instance. As at mo
	//we are lazily loading the singleton this could become a problem
	GatherLevel _gatherLevel;
	
	//
	//The system info values, these maybe gathered on demand or come from a predefined config
	//the predefined config is used on platforms like iphone where we already know the config
	
	std::string _osName;

	
	//GL stuff
	
	float _glVersionNumber;
	float _glslVersionNumber;
	
	std::string _rendererName;
	std::string _vendorName;
	
    //for platforms like android a manual screen size 
    //can be set, otherwise this is set to -1,-1 to indicate
    //that osg windowsysteminterface should be used to get screen size
    osg::Vec2 _manualScreenSize;
    
	//buffers
	
	bool _quadBufferedStereoSupported;
	
	bool _multiSamplingSupported;
	int _maxMultiSamples;
	
	bool _bStencilBufferedSupported;
	
	//shaders
	
	bool _glslLangSupported;
	bool _shaderObjectSupported;
	bool _gpuShader4Supported;
	bool _vertexShadersSupported;
	bool _fragmentShadersSupported;
	bool _geometryShadersSupported;
	
	//texturing
	
	bool _supportsNPOTTexture;
	
	int  _maxTex2DSize;
	bool _texRectangleSupported;
	
	int _maxTextureUnits;
	int _maxVertexTextureUnits;
	int _maxFragmentTextureUnits;
	int _maxGeometryTextureUnits;
	int _totalTextureUnits;
	
	int _maxTextureCoordUnits;
	
	//framebufers / render to texture stuff 
	
	bool _frameBufferObjectSupported;
	

	//these represent the max bits achived in a real world
	//test (via the FindGoodContext function)
	unsigned int _maxTestedBuffers;
	unsigned int _maxTestedStencilBits;
	unsigned int _maxTestedDepthBits;
	unsigned int _maxTestedMultiSamples;

	//callback attched to a render viewer camera to gather gl info
	osg::ref_ptr<GLSupportCallback> _renderSupportInfo;

	
	//gl memory info
	int _totalDedicatedGLMemory;
	int _availableGLMemory;
	int _avliableDedicatedGLMemory;
	
	osg::ref_ptr<GPUMemoryCallback> _glMemoryInfo;

	//the store map of registered SystemFeatureLevels to their friendly names. 
	//The friendly name/uniqueID can then be used by material etc to describe the
	//features they need
	std::map<std::string, SystemFeatureLevelPtr> _featureLevels;
};


};//end hogbox namespace