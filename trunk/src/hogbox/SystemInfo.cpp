#include <hogbox/SystemInfo.h>

#include <osgDB/XmlParser>
#include <osgDB/FileUtils>

#include <iostream>
#include <sstream>
#include <string>

#ifdef TARGET_OS_IPHONE
#include <sys/types.h>
#include <sys/sysctl.h>
#import <UIKit/UIScreen.h>
#endif


using namespace hogbox;

//BUG@tom Since moving to CMake the Singleton class has been misbehaving. An app seems to
//use a different instance of the registry then the plugins seem to register too.
//Changing to the dreaded global instance below has fixed things but I need to try and correct this
osg::ref_ptr<SystemInfo> s_hogboxSystemInfoInstance = NULL;

SystemInfo* SystemInfo::Inst(GatherLevel level, bool erase)
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
    //gl version numbers
    _glVersionNumber(0.0f),
    _glslVersionNumber(0.0f),	
    _rendererName(""),
    _vendorName(""),
    _manualScreenSize(osg::Vec2(-1,-1)),
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
	_supportsNPOTTexture(true), //just till we get iphone systeminfo working
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
    //Default buffers
    _maxTestedBuffers(2),
    _maxTestedStencilBits(0),
    _maxTestedDepthBits(16),
    _maxTestedMultiSamples(0),
    //list of Systeminfolevels
    _renderSupportInfo(NULL),
	_totalDedicatedGLMemory(0),
	_availableGLMemory(0),
	_avliableDedicatedGLMemory(0)
{
	//call init to get info
	this->Init(true);
}

SystemInfo::~SystemInfo(void)
{
	OSG_NOTICE << "    Deallocating SystemInfo Instance."<< std::endl;
	_renderSupportInfo = NULL;
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
			SetSystemInfoFromConfig("./Data/systemInfo.xml");
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
    //open our config and find the system info node
	std::string layoutFile = osgDB::findDataFile( config );
	if(layoutFile.empty())
	{
		osg::notify(osg::WARN) << "SystemInfo::SetSystemInfoFromConfig ERROR: Could not find systemInfo.xml file. " << std::endl;
		return false;
	}
	
	//allocate the document node
	osg::ref_ptr<osgDB::XmlNode> doc = new osgDB::XmlNode;
	osgDB::XmlNode* root = 0;
	
	//open the file with xmlinput
	osgDB::XmlNode::Input input;
	input.open(layoutFile);
	input.readAllDataIntoBuffer();
	
	//read the file into out document
	doc->read(input);
	
	//iterate over the document nodes and try and find a HogBoxDatabase node to
	//use as a root
	for(osgDB::XmlNode::Children::iterator itr = doc->children.begin();
		itr != doc->children.end() && !root;
		++itr)
	{
		if ((*itr)->name=="SystemInfoConfig") root = (*itr);
	}
	
	if (root == NULL)
	{
		osg::notify(osg::WARN) << "SystemInfo::SetSystemInfoFromConfig ERROR: Failed to read xml file '" << layoutFile << "'," << std::endl
		<< "                                         Layout XML file must contain a <SystemInfoConfig> node." << std::endl;
		return false;
	}
    
    if(root){
     
        for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
            itr != root->children.end();
            ++itr)
        {
            //get child node pointer
            osgDB::XmlNode* attNode = itr->get();
            //check current is valid
            if(attNode)
            {
                //get any attribute matching the nodes name
                std::string attName = attNode->name;
                
                if(attName == "GLVersion"){

                    _glVersionNumber = osg::asciiToFloat(attNode->contents.c_str());
                    
                }else if(attName == "GLSLVersion"){
                    _glslVersionNumber=osg::asciiToFloat(attNode->contents.c_str());
                    
                }else if(attName == "QuadBuffered"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _quadBufferedStereoSupported=readValue;
                    
                }else if(attName == "MultiSampling"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _multiSamplingSupported=readValue;
                    
                }else if(attName == "MaxMultiSamples"){
                    int readValue = atoi(attNode->contents.c_str());
                    _maxMultiSamples=readValue;
                    
                }else if(attName == "StencilBuffered"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _bStencilBufferedSupported=readValue;
                    
                }else if(attName == "GLSLSupported"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _glslLangSupported=readValue;
                    
                }else if(attName == "ShaderObjectsSupported"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _shaderObjectSupported=readValue;
                    
                }else if(attName == "Shader4Supported"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _gpuShader4Supported=readValue;
                    
                }else if(attName == "VertexShadersSupported"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _vertexShadersSupported=readValue;
                    
                }else if(attName == "FragmentShadersSupported"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _fragmentShadersSupported=readValue;
                    
                }else if(attName == "GeometryShadersSupported"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _geometryShadersSupported=readValue;
                    
                }else if(attName == "NPOTSupported"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _supportsNPOTTexture=readValue;
                    
                }else if(attName == "MaxTexture2DSize"){
                    int readValue = atoi(attNode->contents.c_str());
                    _maxTex2DSize=readValue;
                    
                }else if(attName == "TextureRectanglesSupported"){
                    int readValue = atoi(attNode->contents.c_str());
                    _texRectangleSupported=readValue;
                    
                }else if(attName == "MaxTextureUnits"){
                    int readValue = atoi(attNode->contents.c_str());
                    _maxTextureUnits=_totalTextureUnits=readValue;
                    
                }else if(attName == "FrameBufferObjectsSupported"){
                    bool readValue = atoi(attNode->contents.c_str()) != 0;
                    _frameBufferObjectSupported=readValue;
                    
                }
            }
        }
    
    }
	return false;
}

//
//Set the local glSystemInfo from the GL gathers
//will also fire off the gather if it hasn't been done yet (will create a viewer)
//
bool SystemInfo::SetGLSystemInfoFromGather(osg::ref_ptr<osg::GraphicsContext> graphicsContext)
{
	//if the _renderSupportInfo isn't valid, then a gather needs to be performed
	if(!_renderSupportInfo.get())
	{
		//no external context supplied, create one using our defaults
		if(!graphicsContext.get())
		{
			graphicsContext = CreateGLContext(osg::Vec2(128,128));
			if(!graphicsContext.get())
			{OSG_FATAL << "SystemInfo::SetGLSystemInfoFromGather: ERROR: Failed to create a GL context." << std::endl;return false;}
				
		}
		
		osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer();
		if(!viewer.get())
		{OSG_FATAL << "SystemInfo::SetGLSystemInfoFromGather ERROR: Failed to create GL Viewer." << std::endl;return false;}
		//attach the graphics context to the viewers camera
		viewer->getCamera()->setGraphicsContext(graphicsContext.get());
		viewer->getCamera()->setViewport(0,0,128,128);
		
		//create our gl info gathering callback
		_renderSupportInfo = new GLSupportCallback();
		if(!_renderSupportInfo){return false;}
		
		//atach our info gathering frame callback to the viewer camera
		viewer->getCamera()->setFinalDrawCallback(_renderSupportInfo.get());
		viewer->getCamera()->setClearColor(osg::Vec4( 1.0, 1.0, 1.0f, 1.0));
		
		//launch the window and render a frame so our callback can get at the graphics context
		//and gather the required gl support info. Single threaded as we want to be sure the
		//callback is triggered before return from frame.
		viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
		//viewer->realize();
		viewer->frame();
		
		//now create the memory info callback and attach as final draw
		_glMemoryInfo = new GPUMemoryCallback();
		if(_glMemoryInfo.get())
		{
			viewer->getCamera()->setFinalDrawCallback(_glMemoryInfo.get());
			//fire another frame to fill GPUMemoryInfo
			viewer->frame();
		}
	}
	
	//now set locals from _renderSupportInfo
	_glVersionNumber = _renderSupportInfo->getGLVersionNumber();
	_glslVersionNumber = _renderSupportInfo->getGLSLVersionNumber();
	
	_rendererName = _renderSupportInfo->getRendererName();
	_vendorName = _renderSupportInfo->getVendorName();
	
	//buffers
	
	_quadBufferedStereoSupported = _renderSupportInfo->quadBufferedStereoSupported();
	
	_multiSamplingSupported = _renderSupportInfo->multiSamplingSupported();
	_maxMultiSamples = _renderSupportInfo->maxMultiSamplesSupported();
	
	_bStencilBufferedSupported = _renderSupportInfo->stencilBufferSupported();
	
	//shaders
	
	_glslLangSupported = _renderSupportInfo->glslLangSupported();
	_shaderObjectSupported = _renderSupportInfo->shaderObjectSupported();
	_gpuShader4Supported = _renderSupportInfo->gpuShader4Supported();
	_vertexShadersSupported = _renderSupportInfo->vertexShadersSupported();
	_fragmentShadersSupported = _renderSupportInfo->fragmentShadersSupported();
	_geometryShadersSupported = _renderSupportInfo->geometryShadersSupported();
	
	//texturing
	
	_supportsNPOTTexture = _renderSupportInfo->npotTextureSupported();
	_maxTex2DSize = _renderSupportInfo->maxTex2DSize();
	_texRectangleSupported = _renderSupportInfo->textureRectangleSupported();
	
	_maxTextureUnits = _renderSupportInfo->maxTextureUnits();
	_maxVertexTextureUnits = _renderSupportInfo->maxVertexTextureUnits();
	_maxFragmentTextureUnits = _renderSupportInfo->maxFragmentTextureUnits();
	_maxGeometryTextureUnits = _renderSupportInfo->maxGeometryTextureUnits();
	_totalTextureUnits = _renderSupportInfo->totalTextureUnitsAvailable();
	
	_maxTextureCoordUnits = _renderSupportInfo->maxTextureCoordUnits();
	
	//framebufers / render to texture stuff 
	
	_frameBufferObjectSupported = _renderSupportInfo->frameBufferObjectSupported();
	
	//GL Memory
	if(_glMemoryInfo.get())
	{
		_totalDedicatedGLMemory = _glMemoryInfo->totalDedicatedVideoMemory();
		_availableGLMemory = _glMemoryInfo->availableMemory();
		_avliableDedicatedGLMemory = _glMemoryInfo->availableDedicatedVideoMemory();
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
	graphicsTraits->doubleBuffer = _maxTestedBuffers >= 2 ? true : false;
	graphicsTraits->sharedContext = 0;
	graphicsTraits->windowDecoration = false;
	graphicsTraits->windowName = "";
	graphicsTraits->stencil = _maxTestedStencilBits;
	graphicsTraits->depth = _maxTestedDepthBits;
	graphicsTraits->samples = _maxTestedMultiSamples;
	graphicsTraits->quadBufferStereo = _maxTestedBuffers >= 4 ? true : false;

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
		OSG_WARN << "SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created." << std::endl;
		return -1;
	} 
	//number of screens
	return wsi->getNumScreens();
}

const int SystemInfo::getScreenWidth(unsigned int screenID)
{
    if(_manualScreenSize.x() != -1){
        return _manualScreenSize.x();
    }
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		OSG_WARN << "SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created." << std::endl;
		return -1;
	} 

	//Screen 0 dimensions
	unsigned int pWidth, pHeight;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(screenID), pWidth, pHeight);
	return pWidth;
}

const int SystemInfo::getScreenHeight(unsigned int screenID)
{
    if(_manualScreenSize.y() != -1){
        return _manualScreenSize.y();
    }
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		OSG_WARN << "SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created." << std::endl;
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
		OSG_WARN << "SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created." << std::endl;
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
		OSG_WARN << "SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created." << std::endl;
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

//get the screens density/res group, used for loading different assets on different displays
SystemInfo::ScreenDensity SystemInfo::getScreenDensity()
{
#ifdef TARGET_OS_IPHONE
    if([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
    {
        if([[UIScreen mainScreen] scale] > 2){
            return HIGH_DENSITY;
        }else if([[UIScreen mainScreen] scale] == 2){
            return MEDIUM_DENSITY;
        }else if([[UIScreen mainScreen] scale] == 1){
            return LOW_DENSITY;
        }
    }else{
        return LOW_DENSITY;
    }
#else
    
    
    osg::Vec2 screenSize = this->getScreenResolution();
    
    //check res has been set
    if(screenSize.x() <= 0 || screenSize.y() <= 0){
        OSG_FATAL << "OsgModelCache::GetScreenType: WARN: Screen res has not been set, defaulting to LOW_RES." << std::endl;
        return LOW_DENSITY;
    }
    
    //use length of screen res vector for comparison to account for reses being in portrait or landscape
    float screenLength = screenSize.length();
    osg::Vec2 QHD(540,960);
    osg::Vec2 WVGA(480,800);
    osg::Vec2 HVGA(320,480);
    
    OSG_FATAL << "ScreenLength: " << screenLength << std::endl;
    
    if(screenLength >= QHD.length()){
        return HIGH_DENSITY;
    }else if(screenLength >= WVGA.length()){
        return MEDIUM_DENSITY;
    }else{
        return LOW_DENSITY;
    }
    
#endif
    return LOW_DENSITY;
}

//set sreen settings
bool SystemInfo::setScreenResolution(osg::Vec2 pixels, unsigned int screenID)
{
	//get basic params
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
	{
		OSG_WARN <<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
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
		OSG_WARN <<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
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
		OSG_WARN <<"SystemInfo ERROR: No WindowSystemInterface available, osg windows can not be created."<<std::endl;
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
			_maxTestedBuffers = 2;
		}
		if(this->FindQuadBufferedTraits( graphicsTraits.get() ))
		{
			_maxTestedBuffers = 4;
		}
		_maxTestedDepthBits = this->FindDepthTraits(graphicsTraits.get(), 24);
		_maxTestedStencilBits = this->FindStencilTraits( graphicsTraits.get(), 8 );
		_maxTestedMultiSamples = this->FindMultiSamplingTraits( graphicsTraits.get(), 8 );
	

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
	
	//osg doesn't throw exceptions?
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
	_featureLevels[name] = featureLevel;
	return true;
}

//
//get featurelevel by name, returns null if not in list
//
SystemFeatureLevel* SystemInfo::GetFeatureLevel(const std::string& name)
{
	//see if we can find a texture in channel
	if(_featureLevels.count(name) > 0)
	{
		return _featureLevels[name];
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
	
	if(featureLevel->textureUnits > this->totalTextureUnitsAvailable())
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
		OSG_WARN << "SystemInfo SUPPORT WARN: Feature Level '" << name << "' does not exist and so can't be checked for support." << std::endl;
		return false;
	}
	return IsFeatureLevelSupported(level);
}

//
//Return the device id string
//
const std::string SystemInfo::GetDeviceString()
{
#ifdef TARGET_OS_IPHONE 
    size_t size;
    
    // Set 'oldp' parameter to NULL to get the size of the data
    // returned so we can allocate appropriate amount of space
    sysctlbyname("hw.machine", NULL, &size, NULL, 0); 
    
    // Allocate the space to store name
    char* name = (char*)malloc(size);
    
    // Get the platform name
    sysctlbyname("hw.machine", name, &size, NULL, 0);
    
    // Place name into a string
    std::string deviceName = std::string(name);
    
    // Done with this
    free(name);
    
    OSG_FATAL << "Device str: '" << deviceName << "'." << std::endl;
    
    return deviceName;
    
#elif defined(WIN32
    return "Windows";
#elif defined(__APPLE__)
    return "OSX";
#elif defined(ANDROID)
    return "AndroidDevice";
#endif
}

//
//Return the device id string
//
const std::string GetDeviceString()
{
#ifdef TARGET_OS_IPHONE
    //ensure a player data file exists in documents folder
    NSArray     *docsPathList = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString    *docsPath     = [docsPathList  objectAtIndex:0];
    const char* docsPathChar  = [docsPath UTF8String];
    return std::string(docsPathChar);
#else
    return "Documents;
#endif
}


void SystemInfo::PrintReportToLog()
{
	OSG_NOTICE << "OSG SystemInfo Report" <<std::endl;

	//report all screens
	OSG_NOTICE << "        Number of Screens:= '" << this->getNumberOfScreens() << "'" << std::endl; 
	for(unsigned int i=0; i<this->getNumberOfScreens(); i++)
	{
		OSG_NOTICE << "                Screen " << i <<std::endl;
		OSG_NOTICE << "                        Resolution (pixels) := " << this->getScreenWidth(i) << ", " << this->getScreenHeight(i) <<std::endl;
		OSG_NOTICE << "                        Resolution Aspect := " << this->getScreenAspectRatio(i) <<std::endl;
		OSG_NOTICE << "                        Refresh Rate (Hz) := " << this->getScreenRefreshRate(i) <<std::endl;
		OSG_NOTICE << "                        ColorDepth (bits) := " << this->getScreenColorDepth(i) <<std::endl;
	}
		
	OSG_NOTICE << "        GPU Name:= '" << this->getRendererName() << "'" << std::endl; 

	OSG_NOTICE << "        GPU Vendor:= '" << this->getVendorName() << "'" << std::endl; 

	OSG_NOTICE << "        GL Version number:= " << this->getGLVersionNumber() <<std::endl; 

	OSG_NOTICE << "        GL Memory:" <<std::endl;

	OSG_NOTICE << "                Onboard (MB):= " << (this->totalDedicatedGLMemory() > 0 ? this->totalDedicatedGLMemory()/1024 : 0) <<std::endl; 

	OSG_NOTICE << "                available Onboard (MB):= " << (this->availableDedicatedGLMemory() > 0 ? this->availableDedicatedGLMemory()/1024 : 0) <<std::endl; 

	OSG_NOTICE << "                Total available (MB):= " << (this->availableGLMemory() > 0 ? this->availableGLMemory()/1024 : 0) <<std::endl; 

	OSG_NOTICE << "        Buffer Formats:" <<std::endl;
	
	OSG_NOTICE << "                Quad Buffered Stereo supported:= " <<  (this->quadBufferedStereoSupported() ? "True" : "False") <<std::endl; 

	OSG_NOTICE << "                Max Depth bits:= " << _maxTestedDepthBits <<std::endl; 
	
	OSG_NOTICE << "                Max Stencil bits:= " << _maxTestedStencilBits <<std::endl;  

	OSG_NOTICE << "                Max Multi Samples:= " << _maxTestedMultiSamples <<std::endl; 

	OSG_NOTICE << "        Shaders:" <<std::endl;

	OSG_NOTICE << "                GLSL supported:= " << (this->glslSupported() ? "True" : "False") <<std::endl; 

	OSG_NOTICE << "                GLSL Version:= " << this->getGLSLVersionNumber() <<std::endl;  

	OSG_NOTICE << "                Shader Objects supported:= " << (this->shaderObjectSupported() ? "True" : "False") <<std::endl; 

	OSG_NOTICE << "                Vertex Shaders supported:= " << (this->vertexShadersSupported() ? "True" : "False") <<std::endl; 

	OSG_NOTICE << "                Fragment Shaders supported:= " << (this->fragmentShadersSupported() ? "True" : "False") <<std::endl; 
	
	OSG_NOTICE << "                Geometry Shaders supported:= " << (this->geometryShadersSupported() ? "True" : "False") <<std::endl; 

	OSG_NOTICE << "                GPU Shader 4 supported:= " << (this->gpuShader4Supported() ? "True" : "False") <<std::endl; 

	OSG_NOTICE << "        Texture Units:" <<std::endl;
	
	OSG_NOTICE << "                Max Texture Size:= " << this->maxTexture2DSize() <<std::endl; 
	
	OSG_NOTICE << "                NPOT Textures supported:= " << (this->npotTextureSupported() ? "True" : "False") <<std::endl; 

	OSG_NOTICE << "                Texture Rectangles supported:= " << (this->textureRectangleSupported() ? "True" : "False") <<std::endl; 

	OSG_NOTICE << "                Max Fixed Function Texture Units:= " << this->maxTextureUnits() <<std::endl; 
	
	OSG_NOTICE << "                Max Vertex Shader Texture Units:= " << this->maxVertexTextureUnits() <<std::endl;

	OSG_NOTICE << "                Max Fragment Shader Texture Units:= " << this->maxFragmentTextureUnits() <<std::endl;

	OSG_NOTICE << "                Max Geometry Shader Texture Units:= " << this->maxGeometryTextureUnits() <<std::endl;

	OSG_NOTICE << "                Total available Texture Units:= " << this->totalTextureUnitsAvailable() <<std::endl;

	OSG_NOTICE << "                Max Texture Coord Units:= " << this->maxTextureCoordUnits() <<std::endl;

}
