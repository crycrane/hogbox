#include <hogbox/SystemInfo.h>

#include <iostream>
#include <sstream>
#include <string>


using namespace hogbox;

//BUG@tom Since moving to CMake the Singleton class has been misbehaving. An app seems to
//use a different instance of the registry then the plugins seem to register too.
//Changing to the dreaded global instance below has fixed things but I need to try and correct this
osg::ref_ptr<SystemInfo> s_hogboxSystemInfoInstance = NULL;

SystemInfo* SystemInfo::Instance(GatherLevel level, bool erase)
{
	if(s_hogboxSystemInfoInstance==NULL)
	{s_hogboxSystemInfoInstance = new SystemInfo(level);}		
	if(erase)
	{
		s_hogboxSystemInfoInstance->destruct();
		s_hogboxSystemInfoInstance = 0;
	}
    return s_hogboxSystemInfoInstance.get();
}

SystemInfo::SystemInfo(GatherLevel level) 
	: osg::Referenced(),
	_gatherLevel(level),
	_osName(""),
//Default info
	m_maxTestedBuffers(2),
	m_maxTestedStencilBits(0),
	m_maxTestedDepthBits(8),
	m_maxTestedMultiSamples(0),
	//The system info values
	_glVersionNumber(0.0f),
	_glslVersionNumber(0.0f),
	_rendererName(""),
	_vendorName(""),
	//buffers
	_quadBufferedStereoSupported(false),
	_multiSamplingSupported(false),
	_maxMultiSamples(0),
	_bStencilBufferedSupported(false),
	//shaders
	_glslLangSupported(false),
	_shaderObjectSupported(false),
	_gpuShader4Supported(false),
	_vertexShadersSupported(false),
	_fragmentShadersSupported(false),
	_geometryShadersSupported(false),
	//texturing
	_maxTex2DSize(256),
	_texRectangleSupported(false),
	_maxTextureUnits(2),
	_maxVertexTextureUnits(0),
	_maxFragmentTextureUnits(0),
	_maxGeometryTextureUnits(0),
	_totalTextureUnits(2),
	_maxTextureCoordUnits(2),
	//framebufers / render to texture stuff 
	_frameBufferObjectSupported(false),
	_totalDedicatedGLMemory(0),
	_avaliableGLMemory(0),
	_avliableDedicatedGLMemory(0),
	//list of Systeminfolevels
	m_renderSupportInfo(NULL)
{
	//call init to get info
	this->Init(true);
}

SystemInfo::~SystemInfo(void)
{
	m_renderSupportInfo = NULL;
}

//
//Run init on first instance
//will gather info at the requested level
//
int SystemInfo::Init(bool printReport)
{
	//determine our os version
#ifdef _WIN32
	//GetVersionEx();
	_osName = "Windows";
#elif __APPLE__ & __MACH__ 
	_osName = "MacOSX";
#elif __APPLE__ //
	osName = "iOS";
#endif
	
	osg::ref_ptr<osg::GraphicsContext> graphicsContext;
	
	switch (_gatherLevel) {
		case CONFIG:
			SetSystemInfoFromConfig("SystemInfo.xml");
			break;
		case FULL:
			//create graphics context using the maximum value for each trait (i.e. samples, depth buffer bits etc)
			//this will also set our max tested values
			graphicsContext = FindGoodContext();
			SetGLSystemInfoFromGather(graphicsContext);
			break;
			
		case GL_GATHER:
			SetGLSystemInfoFromGather(NULL);
			break;
			
		default:
			break;
	}
	
	if(printReport){PrintReportToLog();}

	return 1;
}

//
//Load system info from an xml config file, returns true if used
bool SystemInfo::SetSystemInfoFromConfig(const std::string& config)
{
	return false;
}

//
//Set the local glSystemInfo from the GL gathers
//will also fire off the gather if it hasn't been done yet (will create a viewer)
//
bool SystemInfo::SetGLSystemInfoFromGather(osg::ref_ptr<osg::GraphicsContext> graphicsContext)
{
	//if the m_renderSupportInfo isn't valid, then a gather needs to be performed
	if(!m_renderSupportInfo.get())
	{
		//no external context supplied, create one using our defaults
		if(!graphicsContext.get())
		{
			graphicsContext = CreateGLContext(osg::Vec2(128,128));
			if(!graphicsContext.get())
			{osg::notify(osg::FATAL) << "SystemInfo::SetGLSystemInfoFromGather: ERROR: Failed to create a GL context." << std::endl;return false;}
				
		}
		
		osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer();
		if(!viewer.get())
		{osg::notify(osg::FATAL) << "SystemInfo::SetGLSystemInfoFromGather ." << std::endl;return false;}
		//attach the graphics context to the viewers camera
		viewer->getCamera()->setGraphicsContext(graphicsContext.get());
		viewer->getCamera()->setViewport(0,0,0,0);
		
		//create our gl info gathering callback
		m_renderSupportInfo = new GLSupportCallback();
		if(!m_renderSupportInfo){return false;}
		
		//atach our info gathering frame callback to the viewer camera
		viewer->getCamera()->setFinalDrawCallback(m_renderSupportInfo.get());
		viewer->getCamera()->setClearColor(osg::Vec4( 1.0, 1.0, 1.0f, 1.0));
		
		//launch the window and render a frame so our callback can get at the graphics context
		//and gather the required gl support info. Single threaded as we want to be sure the
		//callback is triggered before return from frame.
		viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
		viewer->realize();
		viewer->frame();
		
		//now create the memory info callback and attach as final draw
		m_glMemoryInfo = new GPUMemoryCallback();
		if(m_glMemoryInfo.get())
		{
			viewer->getCamera()->setFinalDrawCallback(m_glMemoryInfo.get());
			//fire another frame to fill GPUMemoryInfo
			viewer->frame();
		}
	}
	
	//now set locals from m_renderSupportInfo
	_glVersionNumber = m_renderSupportInfo->getGLVersionNumber();
	_glslVersionNumber = m_renderSupportInfo->getGLSLVersionNumber();
	
	_rendererName = m_renderSupportInfo->getRendererName();
	_vendorName = m_renderSupportInfo->getVendorName();
	
	//buffers
	
	_quadBufferedStereoSupported = m_renderSupportInfo->quadBufferedStereoSupported();
	
	_multiSamplingSupported = m_renderSupportInfo->multiSamplingSupported();
	_maxMultiSamples = m_renderSupportInfo->maxMultiSamplesSupported();
	
	_bStencilBufferedSupported = m_renderSupportInfo->stencilBufferSupported();
	
	//shaders
	
	_glslLangSupported = m_renderSupportInfo->glslLangSupported();
	_shaderObjectSupported = m_renderSupportInfo->shaderObjectSupported();
	_gpuShader4Supported = m_renderSupportInfo->gpuShader4Supported();
	_vertexShadersSupported = m_renderSupportInfo->vertexShadersSupported();
	_fragmentShadersSupported = m_renderSupportInfo->fragmentShadersSupported();
	_geometryShadersSupported = m_renderSupportInfo->geometryShadersSupported();
	
	//texturing
	
	_maxTex2DSize = m_renderSupportInfo->maxTex2DSize();
	_texRectangleSupported = m_renderSupportInfo->textureRectangleSupported();
	
	_maxTextureUnits = m_renderSupportInfo->maxTextureUnits();
	_maxVertexTextureUnits = m_renderSupportInfo->maxVertexTextureUnits();
	_maxFragmentTextureUnits = m_renderSupportInfo->maxFragmentTextureUnits();
	_maxGeometryTextureUnits = m_renderSupportInfo->maxGeometryTextureUnits();
	_totalTextureUnits = m_renderSupportInfo->totalTextureUnitsAvaliable();
	
	_maxTextureCoordUnits = m_renderSupportInfo->maxTextureCoordUnits();
	
	//framebufers / render to texture stuff 
	
	_frameBufferObjectSupported = m_renderSupportInfo->frameBufferObjectSupported();
	
	//GL Memory
	if(m_glMemoryInfo.get())
	{
		_totalDedicatedGLMemory = m_glMemoryInfo->totalDedicatedVideoMemory();
		_avaliableGLMemory = m_glMemoryInfo->avaliableMemory();
		_avliableDedicatedGLMemory = m_glMemoryInfo->avaliableDedicatedVideoMemory();
	}
	
	return true;
}


//
//Creates a gl context from the current info settings
//
osg::ref_ptr<osg::GraphicsContext> SystemInfo::CreateGLContext(osg::Vec2 size)
{
	// create the bare minimum traits that should work on all systems
	// then use the find traits functions to find the rest of the values
	osg::ref_ptr<osg::GraphicsContext::Traits> graphicsTraits = new osg::GraphicsContext::Traits;
	graphicsTraits->x = 0;
	graphicsTraits->y =	 0;
	graphicsTraits->width = size.x();
	graphicsTraits->height = size.y();
	graphicsTraits->doubleBuffer = m_maxTestedBuffers >= 2 ? true : false;
	graphicsTraits->sharedContext = 0;
	graphicsTraits->windowDecoration = false;
	graphicsTraits->windowName = "";
	graphicsTraits->stencil = m_maxTestedStencilBits;
	graphicsTraits->depth = m_maxTestedDepthBits;
	graphicsTraits->samples = m_maxTestedMultiSamples;
	graphicsTraits->quadBufferStereo = m_maxTestedBuffers >= 4 ? true : false;

	//create context
	osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(graphicsTraits.get());
	return graphicsContext;
}


//
//Screen info
//
const unsigned int SystemInfo::getNumberOfScreens()
{
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		osg::notify(osg::WARN)<<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
		return -1;
	} 
	//number of screens
	return wsi->getNumScreens();
}

const int SystemInfo::getScreenWidth(unsigned int screenID)
{
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		osg::notify(osg::WARN)<<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
		return -1;
	} 

	//Screen 0 dimensions
	unsigned int pWidth, pHeight;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(screenID), pWidth, pHeight);
	return pWidth;
}

const int SystemInfo::getScreenHeight(unsigned int screenID)
{
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		osg::notify(osg::WARN)<<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
		return -1;
	} 

	//Screen 0 dimensions
	unsigned int pWidth, pHeight;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(screenID), pWidth, pHeight);
	return pHeight;
}

const osg::Vec2 SystemInfo::getScreenResolution(unsigned int screenID)
{
	return osg::Vec2(getScreenWidth(screenID), getScreenHeight(screenID));
}

const double SystemInfo::getScreenRefreshRate(unsigned int screenID)
{
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		osg::notify(osg::WARN)<<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
		return -1;
	} 

	//Screen 0 dimensions
	osg::GraphicsContext::ScreenSettings screen;
	wsi->getScreenSettings(osg::GraphicsContext::ScreenIdentifier(screenID), screen);
	
	return screen.refreshRate;
}

const int SystemInfo::getScreenColorDepth(unsigned int screenID)
{
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		osg::notify(osg::WARN)<<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
		return -1;
	} 

	//Screen 0 dimensions
	osg::GraphicsContext::ScreenSettings screen;
	wsi->getScreenSettings(osg::GraphicsContext::ScreenIdentifier(screenID), screen);
	
	return screen.colorDepth;
}

const double SystemInfo::getScreenAspectRatio(unsigned int screenID)
{
	osg::Vec2 res = this->getScreenResolution(screenID);
	return res.x()/res.y();
}

//set sreen settings
bool SystemInfo::setScreenResolution(osg::Vec2 pixels, unsigned int screenID)
{
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		osg::notify(osg::WARN)<<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
		return -1;
	} 

	//
	osg::GraphicsContext::ScreenSettings screen;
	wsi->getScreenSettings(osg::GraphicsContext::ScreenIdentifier(screenID), screen);

	//change the res settings
	screen.width = pixels.x();
	screen.height = pixels.y();

	return wsi->setScreenSettings(osg::GraphicsContext::ScreenIdentifier(screenID), screen);
}

bool SystemInfo::setScreenRefreshRate(double hz, unsigned int screenID)
{
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		osg::notify(osg::WARN)<<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
		return -1;
	} 

	//
	osg::GraphicsContext::ScreenSettings screen;
	wsi->getScreenSettings(osg::GraphicsContext::ScreenIdentifier(screenID), screen);

	//change the refresh rate settings
	screen.refreshRate = hz;

	return wsi->setScreenSettings(osg::GraphicsContext::ScreenIdentifier(screenID), screen);
}

bool SystemInfo::setScreenColorDepth(unsigned int depth, unsigned int screenID)
{
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		osg::notify(osg::WARN)<<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
		return -1;
	} 

	//
	osg::GraphicsContext::ScreenSettings screen;
	wsi->getScreenSettings(osg::GraphicsContext::ScreenIdentifier(screenID), screen);

	//change the refresh rate settings
	screen.colorDepth = depth;

	return wsi->setScreenSettings(osg::GraphicsContext::ScreenIdentifier(screenID), screen);
}

//
//keeps trying various contexts till it finds the max values for each trait
//
osg::ref_ptr<osg::GraphicsContext> SystemInfo::FindGoodContext()
{

	// create the bare minimum traits that should work on all systems
	// then use the find traits functions to find the rest of the values
	osg::ref_ptr<osg::GraphicsContext::Traits> graphicsTraits = new osg::GraphicsContext::Traits;
	graphicsTraits->x = 1;
	graphicsTraits->y =	 1;
	graphicsTraits->width = 1;
	graphicsTraits->height = 1;
	graphicsTraits->samples = 0;
	graphicsTraits->doubleBuffer = false; //surely
	graphicsTraits->sharedContext = 0;
	graphicsTraits->windowDecoration = false;
	graphicsTraits->windowName = "OSG SysytemInfo";

	//the below find functions are passed a pointer to a Traits struct created in FindGoodContext
	//they apply the maximum working values to Traits struct and also return it as a value
	//they will also disable any unsupported traits

	try
	{
		if(this->FindDoubleBufferTraits( graphicsTraits.get() ))
		{
			m_maxTestedBuffers = 2;
		}
		if(this->FindQuadBufferedTraits( graphicsTraits.get() ))
		{
			m_maxTestedBuffers = 4;
		}
		m_maxTestedDepthBits = this->FindDepthTraits(graphicsTraits.get(), 24);
		m_maxTestedStencilBits = this->FindStencilTraits( graphicsTraits.get(), 8 );
		m_maxTestedMultiSamples = this->FindMultiSamplingTraits( graphicsTraits.get(), 8 );
	

	}catch( std::string str ) {

	}

//create context
	//create the final context using the maximum traits
	osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(graphicsTraits.get());

	if(!graphicsContext)
	{
		return NULL;
	}


	return graphicsContext;
}

//
// try out double buffering and report the result
//
bool SystemInfo::FindDoubleBufferTraits(osg::GraphicsContext::Traits* currentTraits)
{
	currentTraits->doubleBuffer = true;

	//create graphics context using requted traits
 	osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(currentTraits);

	if(!graphicsContext)
	{
		currentTraits->doubleBuffer = false;
		return false;
	}

	//release the test context
	//graphicsContext.release();
	//graphicsContext = NULL;
	return true;
}

//
// try quad beffering and report
//
bool SystemInfo::FindQuadBufferedTraits(osg::GraphicsContext::Traits* currentTraits)
{
	currentTraits->quadBufferStereo = true;

	//create graphics context using requted traits
	osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(currentTraits);

	if(!graphicsContext)
	{
		currentTraits->quadBufferStereo = false;
		return false;
	}

	//release the test context
	//graphicsContext = NULL;
	return true;
}

// find the max number of depth buffer bits
unsigned int SystemInfo::FindDepthTraits(osg::GraphicsContext::Traits* currentTraits, unsigned int bits)
{
	currentTraits->depth = bits;

	//create graphics context using requted traits
	osg::ref_ptr<osg::GraphicsContext> graphicsContext = NULL;
	
	try{
		
		graphicsContext = osg::GraphicsContext::createGraphicsContext(currentTraits);
		
	}catch( std::string str){
	
		graphicsContext = NULL;
		printf("Exception faliure\n");

	}

	if(!graphicsContext)
	{
		if(bits == 0)//stop infintie loop
		{
			currentTraits->depth = 0;
			return 0;

		}else
		{
			//we should probably check it is divisible by 8
			return FindDepthTraits(currentTraits, bits-8);
		}

	}

	//graphicsContext = NULL;
	return bits;
}

//
// find the max number of stencil bits
//(need to look into a sensible sart value for this, I can't remember seeing more than 8, but we'll try 16)
//
unsigned int SystemInfo::FindStencilTraits(osg::GraphicsContext::Traits* currentTraits, unsigned int bits)
{

	currentTraits->stencil = bits;

	//create graphics context using requted traits
	osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(currentTraits);

	if(!graphicsContext)
	{
		if(bits == 1)//stop infintie loop
		{
			currentTraits->stencil = 0;
			return 0;
		}else{
			return FindStencilTraits(currentTraits, (unsigned int)(bits/2));
		}
	}

	//graphicsContext = NULL;
	return bits;
}

//
// Find the max number of multisamples
//
unsigned int SystemInfo::FindMultiSamplingTraits(osg::GraphicsContext::Traits* currentTraits, unsigned int samples)
{
	currentTraits->samples = samples;

	//create graphics context using requted traits
	osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(currentTraits);

	if(!graphicsContext)
	{
		if(samples == 1)//stop infintie loop
		{
			currentTraits->samples = 0;
			return 0;
		}else{
			return FindMultiSamplingTraits(currentTraits, (unsigned int)(samples/2));
		}
	}

	//graphicsContext = NULL;
	return samples;
}


//simplified supported feature level info

//
//add a new feature level by name, if feature level already exists, it
//will be overriden
//
bool SystemInfo::SetFeatureLevel(const std::string& name, SystemFeatureLevel* featureLevel)
{
	if(!featureLevel){return false;}
	m_featureLevels[name] = featureLevel;
	return true;
}

//
//get featurelevel by name, returns null if not in list
//
SystemFeatureLevel* SystemInfo::GetFeatureLevel(const std::string& name)
{
	//see if we can find a texture in channel
	if(m_featureLevels.count(name) > 0)
	{
		return m_featureLevels[name];
	}
	return NULL;	
}

//
//compare a feature level to the current system infomation
//return true if the system info matches or exceeds the feature level
//return false is the system info is below the feature level
//
bool SystemInfo::IsFeatureLevelSupported(SystemFeatureLevel* featureLevel)
{
	//
	if(!featureLevel){return false;}

	if(featureLevel->glVersion > this->getGLVersionNumber())
	{return false;}
	if(featureLevel->glslVersion > this->getGLSLVersionNumber())
	{return false;}
	
	if(featureLevel->textureUnits > this->totalTextureUnitsAvaliable())
	{return false;}

	if(featureLevel->textureCoordUnits > this->maxTextureCoordUnits())
	{return false;}

	if(featureLevel->vertexAndFragmentShaders && !this->AreShadersSupported())
	{return false;}

	if(featureLevel->geometryShaders && !this->geometryShadersSupported())
	{return false;}

	if(featureLevel->screenRes.x() > this->getScreenWidth() )
	{return false;}

	if(featureLevel->screenRes.y() > this->getScreenHeight() )
	{return false;}

	return true;
}

//
//same but by name
//
bool SystemInfo::IsFeatureLevelSupported(const std::string& name)
{
	SystemFeatureLevel* level = GetFeatureLevel(name);
	if(!level)
	{
		osg::notify(osg::WARN) << "SystemInfo SUPPORT WARN: Feature Level '" << name << "' does not exist and so can't be checked for support." << std::endl;
		return false;
	}
	return IsFeatureLevelSupported(level);
}


void SystemInfo::PrintReportToLog()
{
	osg::notify(osg::NOTICE) << "OSG SystemInfo Report" <<std::endl;

	//report all screens
	osg::notify(osg::NOTICE) << "        Number of Screens:= '" << this->getNumberOfScreens() << "'" << std::endl; 
	for(unsigned int i=0; i<this->getNumberOfScreens(); i++)
	{
		osg::notify(osg::NOTICE) << "                Screen " << i <<std::endl;
		osg::notify(osg::NOTICE) << "                        Resolution (pixels) := " << this->getScreenWidth(i) << ", " << this->getScreenHeight(i) <<std::endl;
		osg::notify(osg::NOTICE) << "                        Resolution Aspect := " << this->getScreenAspectRatio(i) <<std::endl;
		osg::notify(osg::NOTICE) << "                        Refresh Rate (Hz) := " << this->getScreenRefreshRate(i) <<std::endl;
		osg::notify(osg::NOTICE) << "                        ColorDepth (bits) := " << this->getScreenColorDepth(i) <<std::endl;
	}
		
	osg::notify(osg::NOTICE) << "        GPU Name:= '" << this->getRendererName() << "'" << std::endl; 

	osg::notify(osg::NOTICE) << "        GPU Vendor:= '" << this->getVendorName() << "'" << std::endl; 

	osg::notify(osg::NOTICE) << "        GL Version number:= " << this->getGLVersionNumber() <<std::endl; 

	osg::notify(osg::NOTICE) << "        GL Memory:" <<std::endl;

	osg::notify(osg::NOTICE) << "                Onboard (MB):= " << (this->totalDedicatedGLMemory() > 0 ? this->totalDedicatedGLMemory()/1024 : 0) <<std::endl; 

	osg::notify(osg::NOTICE) << "                Avaliable Onboard (MB):= " << (this->avaliableDedicatedGLMemory() > 0 ? this->avaliableDedicatedGLMemory()/1024 : 0) <<std::endl; 

	osg::notify(osg::NOTICE) << "                Total Avaliable (MB):= " << (this->avaliableGLMemory() > 0 ? this->avaliableGLMemory()/1024 : 0) <<std::endl; 

	osg::notify(osg::NOTICE) << "        Buffer Formats:" <<std::endl;
	
	osg::notify(osg::NOTICE) << "                Quad Buffered Stereo supported:= " <<  (this->quadBufferedStereoSupported() ? "True" : "False") <<std::endl; 

	osg::notify(osg::NOTICE) << "                Max Depth bits:= " << m_maxTestedDepthBits <<std::endl; 
	
	osg::notify(osg::NOTICE) << "                Max Stencil bits:= " << m_maxTestedStencilBits <<std::endl;  

	osg::notify(osg::NOTICE) << "                Max Multi Samples:= " << m_maxTestedMultiSamples <<std::endl; 

	osg::notify(osg::NOTICE) << "        Shaders:" <<std::endl;

	osg::notify(osg::NOTICE) << "                GLSL supported:= " << (this->glslSupported() ? "True" : "False") <<std::endl; 

	osg::notify(osg::NOTICE) << "                GLSL Version:= " << this->getGLSLVersionNumber() <<std::endl;  

	osg::notify(osg::NOTICE) << "                Shader Objects supported:= " << (this->shaderObjectSupported() ? "True" : "False") <<std::endl; 

	osg::notify(osg::NOTICE) << "                Vertex Shaders supported:= " << (this->vertexShadersSupported() ? "True" : "False") <<std::endl; 

	osg::notify(osg::NOTICE) << "                Fragment Shaders supported:= " << (this->fragmentShadersSupported() ? "True" : "False") <<std::endl; 
	
	osg::notify(osg::NOTICE) << "                Geometry Shaders supported:= " << (this->geometryShadersSupported() ? "True" : "False") <<std::endl; 

	osg::notify(osg::NOTICE) << "                GPU Shader 4 supported:= " << (this->gpuShader4Supported() ? "True" : "False") <<std::endl; 

	osg::notify(osg::NOTICE) << "        Texture Units:" <<std::endl;
	
	osg::notify(osg::NOTICE) << "                Max Texture Size:= " << this->maxTexture2DSize() <<std::endl; 

	osg::notify(osg::NOTICE) << "                Texture Rectangles supported:= " << (this->textureRectangleSupported() ? "True" : "False") <<std::endl; 

	osg::notify(osg::NOTICE) << "                Max Fixed Function Texture Units:= " << this->maxTextureUnits() <<std::endl; 
	
	osg::notify(osg::NOTICE) << "                Max Vertex Shader Texture Units:= " << this->maxVertexTextureUnits() <<std::endl;

	osg::notify(osg::NOTICE) << "                Max Fragment Shader Texture Units:= " << this->maxFragmentTextureUnits() <<std::endl;

	osg::notify(osg::NOTICE) << "                Max Geometry Shader Texture Units:= " << this->maxGeometryTextureUnits() <<std::endl;

	osg::notify(osg::NOTICE) << "                Total avaliable Texture Units:= " << this->totalTextureUnitsAvaliable() <<std::endl;

	osg::notify(osg::NOTICE) << "                Max Texture Coord Units:= " << this->maxTextureCoordUnits() <<std::endl;

}