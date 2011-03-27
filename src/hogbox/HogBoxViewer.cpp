#include <hogbox/HogBoxViewer.h>

#include <iostream>

#include <osg/GLExtensions>


using namespace hogbox;

HogBoxViewer::HogBoxViewer(HWND hwnd)
	: osg::Object(),
	m_p_scene(NULL),
	m_hwnd(hwnd),
	m_screenID(0),
	m_viewer(NULL),
	m_graphicsContext(NULL),
	m_graphicsWindow(NULL),
	m_resizeCallback(NULL),
	m_requestReset(false),
	//window size in pixels
	m_winSize(osg::Vec2(800, 600)),
	m_prevWinSize(m_winSize),
	//window corner in pixels (Windows bottom left is 0,0)
	m_winCorner(osg::Vec2(0, 0)),
	m_prevWinCorner(m_winCorner),
	//
	m_doubleBuffer(true),
	m_vSync(false),
	m_colorBits(24),
	m_depthBits(24),
	m_alphaBits(8),
	m_stencilBits(0),
	m_accumulationBits(0),
	//is app in fullscreen mode
	m_bIsFullScreen(false),
	m_bUsingBoarder(true),
	m_windowName("HogBoxViewer"),
	m_usingCursor(true),
//stereo
	//are we using stereo
	m_bStereoEnabled(false),
	//current eyeseperation
	m_fStereoSep(0.006f),
	m_swapEyes(false),
	//current convergence distance from camera
	m_fStereoConv(5.0f),
	//type of stereo display
	m_iStereoMode(1), //anaglyph
//rendering
	m_clearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f)),
	//antialiasing samples
	m_aaSamples(0), //try for 4, systeminfo will prevent it if not supported
//view/camera
	//field of view of camera
	m_vfov(45.0f),
	m_cameraViewMatrix(osg::Matrix::identity()),
	m_viewDistance(4.0f), //used for saving trackball distances
	m_cameraHomePos(osg::Vec3(0.0f,0.0f,0.0f)),
	m_cameraHomeLookAt(osg::Vec3(0.0f,0.0f,-1.0f)),
	m_cameraHomeUp(osg::Vec3(0,1,0)),
//saving
	m_viewerSettingsFile(""),
//renderOffscreen
	_frameBufferImage(NULL),
	m_bRenderOffscreen(false),
//assume the worst for support
	m_glSystemInfo(NULL)
{
	m_glSystemInfo = SystemInfo::Instance();
}

HogBoxViewer::~HogBoxViewer(void)
{
	OSG_NOTICE << "    Deallocating HogBoxViewer: Name '" << this->getName() << "'." << std::endl;

	m_p_scene = NULL;

	m_resizeCallback = NULL;
	
	for(unsigned int i=0; i<m_p_appEventHandlers.size(); i++)
	{m_p_appEventHandlers[i] = NULL;}
	m_p_appEventHandlers.clear();

	m_viewer = NULL;

	//handle to the context window for manipulaing fullscreen etc
	m_graphicsWindow = NULL;

	//single context to attach to the viewers camera
	m_graphicsContext = NULL;
}

//
//init contructing window and viewer, optionaly passing in render mode args
//
int HogBoxViewer::Init(osg::Node* scene, bool fullScreen, 
					   osg::Vec2 winSize, osg::Vec2 winCr, 
					   bool realizeNow, unsigned int screenID, bool useStereo, 
					   int stereoMode, bool renderOffscreen)
{

	m_screenID = screenID;

	//good default fov
	int width = m_glSystemInfo->getScreenWidth(m_screenID);
    int height = m_glSystemInfo->getScreenHeight(m_screenID);
	osg::Vec2 screenRes = osg::Vec2(width,height);
    double distance = 0.5; //osg::DisplaySettings::instance()->getScreenDistance();
   // m_vfov = osg::RadiansToDegrees(atan2(height/2.0f,distance)*2.0);

	//set the scene to render
	m_p_scene = scene;

	//window size in pixels
	m_winSize = winSize;
	//window corner in pixels ( Windows bottom left is 0,0)
	if(winCr.x()==-1)//pass neg to center
	{
		//set the default window to center
		if(m_winSize.x()>screenRes.x())
		{m_winSize.x() = screenRes.x();}
		if(m_winSize.y()>screenRes.y())
		{m_winSize.y() = screenRes.y();}
		m_winCorner = osg::Vec2((screenRes.x()/2)-(m_winSize.x()/2), (screenRes.y()/2)-(m_winSize.y()/2));
	}else{
		m_winCorner = winCr;
	}

	//is app in fullscreen mode
	m_bIsFullScreen = fullScreen;

	//are we using stereo
	m_bStereoEnabled = useStereo;

	//type of stereo display
	m_iStereoMode = stereoMode; //anaglyph

	//Do we want to render to offscreen image target
	m_bRenderOffscreen = renderOffscreen;

	//if we have a valid scene then construc the window, else divert till later
	if(m_p_scene.valid() && realizeNow)
	{
		return CreateAppWindow();
	}

	return 0;
}

//
//perform viewer pass, remndering etc
//
void HogBoxViewer::frame()
{
	if(m_viewer.valid())
	{
		if(m_resizeCallback != NULL)
		{
			m_winSize = m_resizeCallback->GetWinSize();
			m_winCorner = m_resizeCallback->GetWinCorner();
		}
		m_viewer->frame();
	}
}

//
//return viewer done state
//
bool HogBoxViewer::done()
{
	if(m_viewer.valid())
	{
		return m_viewer->done();
	}
	return true;
}

void HogBoxViewer::addEventHandler(osgGA::GUIEventHandler* eventHandler)
{
	//add to our list of eventHandler
	m_p_appEventHandlers.push_back((EventHandlerObserver)eventHandler);
	if(m_viewer.valid())
	{
		m_viewer->addEventHandler(eventHandler);
	}
}

//
//contruct the window and viewer using existing settings
//returns false if no scene node set
//
bool HogBoxViewer::CreateAppWindow()
{
	//if not philips mode
	if( m_iStereoMode != 9 && m_iStereoMode != 10)
	{
		//if the implentation window exists get the current dimensions and close it
		if(m_graphicsWindow != NULL)
		{
			int x, y, w, h;
			m_graphicsWindow->getWindowRectangle(x, y, w, h);
			m_winSize = osg::Vec2(w,h);
			m_winCorner = osg::Vec2(x,y);
			m_graphicsWindow->close(); //setCursor(osgViewer::GraphicsWindow::MouseCursor::
			m_graphicsWindow = NULL;
		}

		// create the window to draw to.
		osg::GraphicsContext::Traits* graphicsTraits = new osg::GraphicsContext::Traits;
		graphicsTraits->screenNum = m_screenID;
		graphicsTraits->x = m_winCorner.x();
		graphicsTraits->y = m_winCorner.y();
		graphicsTraits->width = m_winSize.x();
		graphicsTraits->height = m_winSize.y();
		graphicsTraits->red = m_colorBits/3;
		graphicsTraits->green = m_colorBits/3;
		graphicsTraits->blue = m_colorBits/3;
		graphicsTraits->depth = m_depthBits;//m_glSystemInfo->maxDepthBufferBits();
		graphicsTraits->alpha = m_alphaBits;
		graphicsTraits->stencil = m_stencilBits;
		graphicsTraits->doubleBuffer = m_doubleBuffer; //m_glSystemInfo->doubleBufferedStereoSupported();
		graphicsTraits->sharedContext = 0;
		graphicsTraits->windowDecoration = m_bUsingBoarder;
		graphicsTraits->useCursor = m_usingCursor;
		graphicsTraits->windowName = m_windowName;
		graphicsTraits->vsync = m_vSync;

		//attach to any handle if avaliable
		if(m_hwnd != NULL)
		{
			// Init the Windata Variable that holds the handle for the Window to display OSG in.
			graphicsTraits->setInheritedWindowPixelFormat = true;
			osg::ref_ptr<osg::Referenced> windata;
			
#ifdef WIN32 
			windata = new osgViewer::GraphicsWindowWin32::WindowData(m_hwnd);
#else
			#if (TARGET_OS_IPHONE)
				windata = new osgViewer::GraphicsWindowIOS::WindowData((UIWindow*)m_hwnd);
			#else
				windata = new osgViewer::GraphicsWindowCarbon::WindowData((OpaqueWindowPtr*)m_hwnd);
			#endif
#endif
			graphicsTraits->inheritedWindowData = windata;
			graphicsTraits->windowDecoration = false;
			m_bUsingBoarder = false;
		}


		//apply samles if supported
		if(m_glSystemInfo->multiSamplingSupported())
        {
            //if the requested samples are within supported range
            if(m_glSystemInfo->maxMultiSamplesSupported() >= m_aaSamples)
            {
                graphicsTraits->samples = m_aaSamples;
            }else{
                graphicsTraits->samples = m_glSystemInfo->maxMultiSamplesSupported();
            }
        }
      
		//apply extra traits required for stereo modes
		if(m_bStereoEnabled)//add quad buffered if requested
		{
			if(m_iStereoMode==0) //Quadbuffered
			{
				//check its supported
				if(m_glSystemInfo->quadBufferedStereoSupported())
				{
					graphicsTraits->quadBufferStereo = true;
				}else{
					osg::notify(osg::WARN) << "HogBoxViewer CreateWindow ERROR: QuadBuffered Stereo has been requested but is not supported. Defaulting to Anaglyph."<<std::endl;
					m_iStereoMode = 1;//default to anaglyph
				}
				//interlace and checker modes require stencil
			}else if(m_iStereoMode==6 || m_iStereoMode==7 || m_iStereoMode==8){
				
				m_stencilBits = 8;
				graphicsTraits->stencil = m_stencilBits;
			}
		}

		//if a context exists, release it pointer
		if(m_graphicsContext != NULL)
		{
			m_graphicsContext = NULL;
		}

		//now check for offscreen rendering and change our traits accordingly
		if(m_bRenderOffscreen)
		{
			_frameBufferImage = new osg::Image();
			_frameBufferImage->allocateImage(m_winSize.x(), m_winSize.y(), 1, GL_BGR, GL_UNSIGNED_BYTE);
			graphicsTraits->windowDecoration = false;
			//graphicsTraits->doubleBuffer = false; //m_glSystemInfo->doubleBufferedStereoSupported();
			graphicsTraits->sharedContext = 0;
			graphicsTraits->pbuffer = true;
		}

		//create graphics context using requested traits
		m_graphicsContext = osg::GraphicsContext::createGraphicsContext(graphicsTraits);

		if(!m_graphicsContext.valid())
		{
			osg::notify(osg::WARN) << "HogBoxViewer CreateWindow ERROR: Failed to create graphicsContext, viewer is not vaild." << std::endl;
			return false;
		}

		//cast the context to a window to set position etc
		m_graphicsWindow = dynamic_cast<osgViewer::GraphicsWindow*>(m_graphicsContext.get());

		if (!m_graphicsWindow && !m_bRenderOffscreen)
		{
			osg::notify(osg::WARN) << "HogBoxViewer CreateWindow ERROR: Failed to create graphicsContext, viewer is not vaild." << std::endl;
			return false;
		}

		//now window is created apply settings that don't seem to persist from the traits (useCursor)
		this->SetCursorVisible(m_usingCursor);

		// create the view of the scene.
		if(m_viewer != NULL)
		{
			//release old
			m_viewer->done();
			m_viewer = NULL;
		}

		//create the osg viewer
		m_viewer = new osgViewer::Viewer();
		m_viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
		
		//set cameras projection and viewport
		double height = m_glSystemInfo->getScreenWidth(m_screenID);
		double width = m_glSystemInfo->getScreenHeight(m_screenID);
		osg::Vec2 screenRes = osg::Vec2(width,height);

		m_viewer->getCamera()->setProjectionMatrixAsPerspective( m_vfov, width/height, 1.0f,10000.0f);

		//attach the graphics context to the viewers camera
		m_viewer->getCamera()->setGraphicsContext(m_graphicsContext.get());
		m_viewer->getCamera()->setViewport(0,0,m_winSize.x(),m_winSize.y());
		//m_viewer->getCamera()->setClearMask(GL_DEPTH_BUFFER_BIT);
		m_viewer->getCamera()->setClearColor(m_clearColor);
		
		//also bind our buffer image if rendering offscreen
		if(m_bRenderOffscreen)
		{
			// tell the camera to use OpenGL frame buffer objects
			m_viewer->getCamera()->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
			m_viewer->getCamera()->attach(osg::Camera::COLOR_BUFFER, _frameBufferImage.get());
		}
		
		//attach the resize callback
		//destroy old callback if it exists]
		if(m_resizeCallback)
		{
			m_resizeCallback = NULL;
		}
		m_resizeCallback = new HogBoxViewerResizedCallback(m_viewer.get(), m_winCorner.x(), m_winCorner.y(), m_winSize.x(), m_winSize.y());
		m_graphicsContext->setResizedCallback(m_resizeCallback);
		
		//force a resize to acount for the initial state
		//m_resizeCallback->resizedImplementation(m_graphicsContext, m_winCorner.x(), m_winCorner.y(), m_winSize.x(), m_winSize.y());
	
		// set the scene to render
		//check a scene has been set
		if(!m_p_scene)
		{
			osg::notify(osg::WARN) << "HogBoxViewer CreateWindow WARN: No SceneNode has been set the viewer will only render the ClearColor"<<std::endl;
		}else{
			m_viewer->setSceneData(m_p_scene.get());
		}

		//set our default cull masks
		m_viewer->getCamera()->setCullMask(NodeMasks::MAIN_CAMERA_CULL);
		m_viewer->getCamera()->setCullMaskLeft(NodeMasks::MAIN_CAMERA_LEFT_CULL);
		m_viewer->getCamera()->setCullMaskRight(NodeMasks::MAIN_CAMERA_RIGHT_CULL);

		// set up the use of stereo by default. (Is this the only way of doing this ?)
		osg::DisplaySettings::instance()->setStereo(m_bStereoEnabled);
		osg::DisplaySettings::instance()->setStereoMode((osg::DisplaySettings::StereoMode) m_iStereoMode);

		//set seperation based on swap
		if(m_swapEyes)
		{
			osg::DisplaySettings::instance()->setEyeSeparation( -m_fStereoSep );
		}else{
			osg::DisplaySettings::instance()->setEyeSeparation( m_fStereoSep );
		}
		m_viewer->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE, m_fStereoConv );


		//add any external app event handler
		for(unsigned int i=0; i<m_p_appEventHandlers.size(); i++)
		{
			m_viewer->addEventHandler(m_p_appEventHandlers[i].get());
		}

		//everything is inplace so realize our viewer (creating window etc)
		m_viewer->realize();

		//force a resize to get a proper initial state
		m_resizeCallback->resizedImplementation(m_graphicsContext,m_winCorner.x(),m_winCorner.y(),m_winSize.x(),m_winSize.y());
	
		//set request back to false
		m_requestReset = false;

		if(m_viewer->isRealized())
		{
			//if we want full sceen mode set it now
			if(m_bIsFullScreen)
			{
//				CMsgLog::Inst()->WriteToLog("Window Created");
				this->SetFullScreen(m_bIsFullScreen);
			}
			return true;
		}
	}

	return true;
}

//
// check if the viewer want a reset inorder to reflect a render mode change
//
bool HogBoxViewer::isRequestingReset()
{
	return m_requestReset;
}

//
// return the viewer to be passed to eventhandlers etc
//
osg::ref_ptr<osgViewer::Viewer> HogBoxViewer::GetViewer()
{
	if(m_viewer.valid())
	{return m_viewer.get();}

	return NULL;
}

//
//set the scene node, resteting the viewer if requested
//
void HogBoxViewer::SetSceneNode(osg::Node* scene, bool resetWindow)
{
	m_p_scene = scene;
	if(m_viewer.valid()){m_viewer->setSceneData(m_p_scene.get());}
	if(resetWindow){CreateAppWindow();}
}

//
//window

void HogBoxViewer::SetScreenID(const unsigned int& screen)
{
	//check it's in range of total displays
	if(screen >= m_glSystemInfo->getNumberOfScreens()){return;}
	
	//if it's a different screen, reset
	if(screen != m_screenID){m_requestReset=true;}

	m_screenID = screen;
}

const unsigned int& HogBoxViewer::GetScreenID()const
{
	return m_screenID;
}

//
//set the window size
//
void HogBoxViewer::SetWindowSize(const osg::Vec2& winSize)
{
	m_winSize = winSize;
	//if window exists apply straight away
	if(m_graphicsWindow != NULL)
	{
		m_graphicsWindow->setWindowRectangle(m_winCorner.x(), m_winCorner.y(), m_winSize.x(), m_winSize.y());
		//also set the viewport
		m_viewer->getCamera()->getViewport()->setViewport(0,0,m_winSize.x(),m_winSize.y()); 
	}
}

const osg::Vec2& HogBoxViewer::GetWindowSize()const
{
	return m_winSize;
}

//
//position window
//
void HogBoxViewer::SetWindowCorner(const osg::Vec2& winCr)
{
	m_winCorner = winCr;
	//if window exists apply straight away
	if(m_graphicsWindow != NULL)
	{
		m_graphicsWindow->setWindowRectangle(m_winCorner.x(), m_winCorner.y(), m_winSize.x(), m_winSize.y());
	}
}

const osg::Vec2& HogBoxViewer::GetWindowCorner()const
{
	return m_winCorner;
}

void HogBoxViewer::SetWindowDecoration(const bool& enableBoarder)
{
	m_bUsingBoarder = enableBoarder;
	if(m_graphicsWindow != NULL)
	{
		m_graphicsWindow->setWindowDecoration(m_bUsingBoarder);
	}
}

const bool& HogBoxViewer::GetWindowDecoration()const
{
	return m_bUsingBoarder;
}

void HogBoxViewer::SetDoubleBuffered(const bool& doubleBuffer)
{
	if(!m_glSystemInfo->doubleBufferSupported()){
		m_doubleBuffer = false;
	}else{
		m_doubleBuffer = doubleBuffer;
	}
	this->m_requestReset = true;
}

const bool& HogBoxViewer::GetDoubleBuffered()const
{
	return m_doubleBuffer;
}

void HogBoxViewer::SetVSync(const bool& vsync)
{
	m_vSync = vsync;
	if(m_graphicsWindow != NULL)
	{
		m_graphicsWindow->setSyncToVBlank(m_vSync);
	}
}

const bool& HogBoxViewer::GetVSync()const
{
	return m_vSync;
}

void HogBoxViewer::SetColorBits(const unsigned int& bits)
{
	m_colorBits = bits;
	this->m_requestReset = true;
}

const unsigned int& HogBoxViewer::GetColorBits()const
{
	return m_colorBits;
}

void HogBoxViewer::SetDepthBits(const unsigned int& bits)
{
	if(bits > m_glSystemInfo->maxDepthBufferBits())
	{
		m_depthBits = m_glSystemInfo->maxDepthBufferBits();
	}else{
		m_depthBits = bits;
	}
	this->m_requestReset = true;	
}

const unsigned int& HogBoxViewer::GetDepthBits()const
{
	return m_depthBits;
}

void HogBoxViewer::SetAlphaBits(const unsigned int& bits)
{
	m_alphaBits = bits;
	this->m_requestReset = true;
}

const unsigned int& HogBoxViewer::GetAlphaBits()const
{
	return m_alphaBits;
}

void HogBoxViewer::SetStencilBits(const unsigned int& bits)
{
	if(bits > m_glSystemInfo->maxStencilBitsSupported())
	{
		m_stencilBits = m_glSystemInfo->maxStencilBitsSupported();
	}else{
		m_stencilBits = bits;
	}
	this->m_requestReset = true;
}
const unsigned int& HogBoxViewer::GetStencilBits()const
{
	return m_stencilBits;
}

void HogBoxViewer::SetAccumulationBits(const unsigned int& bits)
{
	m_accumulationBits = bits;
	this->m_requestReset = true;
}

const unsigned int& HogBoxViewer::GetAcculationBits()const
{
	return m_accumulationBits;
}

//
//set fullscreen resizing to m_sSize, must be set by init()
//
void HogBoxViewer::SetFullScreen(const bool& fullScreen)
{
	m_bIsFullScreen = fullScreen;

	if(!m_graphicsWindow){return;}

	if(fullScreen) //fullscreen mode
	{
		//capture current window size and corner, so we can swap back when we leave fullscreen mode
		int x, y, w, h;
		m_graphicsWindow->getWindowRectangle(x, y, w, h);
		m_winSize = osg::Vec2(w,h);
		m_prevWinSize = m_winSize;
		m_winCorner = osg::Vec2(x,y);
		m_prevWinCorner = m_winCorner;

		//set the window to sSize at 0,0
		m_graphicsWindow->setWindowRectangle(0, 0, m_glSystemInfo->getScreenWidth(m_screenID), m_glSystemInfo->getScreenHeight(m_screenID));
		//also set the viewport
		m_viewer->getCamera()->getViewport()->setViewport(0,0,m_glSystemInfo->getScreenWidth(m_screenID),m_glSystemInfo->getScreenWidth(m_screenID)); 
		//remove decoration
		m_bUsingBoarder=false;
		m_graphicsWindow->setWindowDecoration(m_bUsingBoarder);
		m_bIsFullScreen=true;

	}else{
		m_bUsingBoarder=true;
		m_graphicsWindow->setWindowDecoration(m_bUsingBoarder);
		//get previous window corner
		m_winSize = m_prevWinSize;
		m_winCorner = m_prevWinCorner;

		//clip to screen
		//ClipWinToScreen(m_screenID);

		m_graphicsWindow->setWindowRectangle(m_winCorner.x(), m_winCorner.y(), m_winSize.x(), m_winSize.y());
		//also set the viewport
		m_viewer->getCamera()->getViewport()->setViewport(0,0,m_winSize.x(),m_winSize.y()); 
		m_bIsFullScreen=false;
	}
}

const bool& HogBoxViewer::isFullScreen()const
{
	return m_bIsFullScreen;
}

void HogBoxViewer::ClipWinToScreen(int screenID)
{

	float boarder = 20;
	float xMax = m_winCorner.x() + m_winSize.x();
	float xMin = m_winCorner.x();
	//if(xMin

	float yMax = m_winCorner.y() + m_winSize.y();
	float yMin = m_winCorner.y();
	printf("Win corner y %f\n", m_winCorner.y());
	//if the win is off top of screen and we have a boarder
	//move down so user can still use bar
	/*if( (yMax+boarder >= m_sSize.y()) && m_bUsingBoarder)
	{
		float dif = (yMax)-m_sSize.y()+boarder;
		printf("Win Diff y %f\n", dif);
		m_winCorner.y() +=  dif;
	}*/

	if(yMin<0)
	{
		m_winCorner.y() = -m_winCorner.y();
	}
}

void HogBoxViewer::SetWindowName(const std::string& windowName)
{
	m_windowName = windowName;
	if(m_graphicsWindow == NULL){return;}
	m_graphicsWindow->setWindowName( m_windowName);
}

const std::string& HogBoxViewer::GetWindowName()const
{
	return m_windowName;
}

void HogBoxViewer::SetCursorVisible(const bool& visible)
{
	m_usingCursor = visible;
	if(m_graphicsWindow == NULL){return;}
	m_graphicsWindow->useCursor(visible);
}

const bool& HogBoxViewer::isCursorVisible()const
{
	return m_usingCursor;
}

//
//save the current frame if using render offscreen mode
//
void HogBoxViewer::SaveCurrentFrameBuffer(const std::string& fileName)
{
	if(_frameBufferImage.valid())
	{
		osgDB::writeImageFile(*_frameBufferImage.get(), fileName);
	}
}


osg::Image* HogBoxViewer::GetCurrentFrameBuffer()
{
	return _frameBufferImage.get();
}


//stereo

void HogBoxViewer::SetUseStereo(const bool& useStereo)
{
	m_bStereoEnabled = useStereo;

	if(m_viewer.valid())
	{
		osg::DisplaySettings::instance()->setStereo(m_bStereoEnabled);
	}
}

const bool& HogBoxViewer::isUsingStereo()const
{
	return m_bStereoEnabled;
}

void HogBoxViewer::SetStereoConvDistance(const float& distance)
{
	m_fStereoConv = distance;
	if(m_viewer.valid())
	{
		m_viewer->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE, m_fStereoConv);
	}
}

const float& HogBoxViewer::GetStereoConvDistance()const
{
	return m_fStereoConv;
}

void HogBoxViewer::SetStereoEyeSeperation(const float& distance)
{
	m_fStereoSep = distance;
	if(m_viewer.valid())
	{
		//apply the eye swapping
		if(m_swapEyes)
		{
			osg::DisplaySettings::instance()->setEyeSeparation(-m_fStereoSep);
		}else{
			osg::DisplaySettings::instance()->setEyeSeparation(m_fStereoSep);
		}
	}
}

const float& HogBoxViewer::GetStereoEyeSeperation()const
{
	return m_fStereoSep;
}

//
// Tell viewer to swap eyes when applying setEyeSeparation
//
void HogBoxViewer::SetSwapEyes(const bool& swap)
{
	m_swapEyes = swap;
	SetStereoEyeSeperation(GetStereoEyeSeperation());
}

const bool& HogBoxViewer::GetSwapEyes()const
{
	return m_swapEyes;
}

//
//set stereo mopde. Will also recreate window with existing scene data if the mode require a different context
//
void HogBoxViewer::SetStereoMode(const int& mode)
{
	//store so we can check if the context needs re creating
	int prevMode = m_iStereoMode;

	//no need to set if there the same
	if(prevMode == mode)
	{return;}

	m_iStereoMode = mode;

	//if viewer exists then set the stereo imidiatly
	if(m_viewer.valid())
	{
		//check for philips modes
		if(m_iStereoMode == 9 || m_iStereoMode == 10)
		{

		}else{

			//do we need to recreate window
			m_requestReset = false;

			//if the previous mode was qad buffered we always need to recreate
			if( prevMode == osg::DisplaySettings::QUAD_BUFFER)
			{
				m_requestReset = true;
			}

			//check for particular different modes that will require a new context
			switch(m_iStereoMode)
			{
				//if its a stencil mode (interlaced/checkboard) then we need to reset
				case osg::DisplaySettings::HORIZONTAL_INTERLACE:
				case osg::DisplaySettings::VERTICAL_INTERLACE:
				case osg::DisplaySettings::CHECKERBOARD:
				{
					m_requestReset = true;//only way to currently force a redrawe of the stencil mask
					break;
				}
				case osg::DisplaySettings::QUAD_BUFFER:
				{
				    if(m_glSystemInfo->quadBufferedStereoSupported())
                    {
                        m_requestReset = true;
                    }else{
                        //if it's not supported ensure we revert to a safe on
                        m_iStereoMode = prevMode;
						return;
                    }
					break;
				}
				default:break;
			}

			osg::DisplaySettings::instance()->setStereoMode((osg::DisplaySettings::StereoMode)m_iStereoMode);
			return;
		}
	}
	return;
}

const int& HogBoxViewer::GetStereoMode()const
{
	return m_iStereoMode;
}

//
//rendering

//
//set the clear color for rendering
//
void HogBoxViewer::SetClearColor(const osg::Vec4& color)
{
	m_clearColor = color;

	if(m_viewer)
	{m_viewer->getCamera()->setClearColor(m_clearColor);}
}

const osg::Vec4& HogBoxViewer::GetClearColor()const
{
	return m_clearColor;
}

//
//set the number of aa samples reseting context if required
//
void HogBoxViewer::SetAASamples(const int& samples)
{
	//return if its the same or out of range
	if( (m_aaSamples < 0) || (m_aaSamples == samples))
	{return;}

	if(m_aaSamples>m_glSystemInfo->maxMultiSamplesSupported()) 
	{
		m_aaSamples = m_glSystemInfo->maxMultiSamplesSupported();
	}else{
		m_aaSamples = samples;
	}
	m_requestReset = true;
}

const int& HogBoxViewer::GetAASamples()const
{
	return m_aaSamples;
}

//
//view/camera
//
void HogBoxViewer::SetProjectionMatrix(osg::Projection* projection)
{
	if(m_viewer.valid())
	{
		m_viewer->getCamera()->setProjectionMatrix(projection->getMatrix());
	}
}

//
//set cameras view matix
//
void HogBoxViewer::SetCameraViewMatrix(const osg::Matrix& viewMatrix)
{
	m_cameraViewMatrix = viewMatrix;
	if(m_viewer.valid())
	{
		m_viewer->getCamera()->setViewMatrix(viewMatrix);
	}
}

const osg::Matrix& HogBoxViewer::GetCameraViewMatrix()const
{
	return m_cameraViewMatrix;
}

void HogBoxViewer::SetCameraViewDistance(const float& distance)
{
	m_viewDistance = distance;
}

const float& HogBoxViewer::GetCameraViewDistance()const
{
	return m_viewDistance;
}

//
// Set the home lookat vectors
bool HogBoxViewer::SetHomeLookAtVectors(osg::Vec3 camPos, osg::Vec3 lookAtPos, osg::Vec3 upVec)
{
	m_cameraHomePos = camPos;
	m_cameraHomeLookAt = lookAtPos;
	m_cameraHomeUp = upVec;
	return true;
}

//
//returns true if a home matrix has been set passin the values through the paased in args
//
bool HogBoxViewer::GetHomeLookAtVectors(osg::Vec3& camPos, osg::Vec3& lookAtPos, osg::Vec3& upVec)
{
	camPos = m_cameraHomePos;
	lookAtPos = m_cameraHomeLookAt;
	upVec = m_cameraHomeUp;

	//float distance = 0.0f;
	//m_cameraHomeMatrix.getLookAt(camPos, lookAtPos, upVec, distance);

	return true;
}

//
//Helper to set a lookat using a trackball (captures the current state of the track ball as lookat vectors)
bool HogBoxViewer::SetHomeLookAtVectorsFromTrackBall(osgGA::TrackballManipulator* trackBall)
{
	//m_hogViewer->SetCameraHomeMatrix(m_hogViewer->GetCameraViewMatrix());
	//m_hogViewer->SetCameraHomeDistance(cameraManipulator->getDistance());
	osg::Vec3 center = trackBall->getCenter();

	osg::Vec3 camPos;
	osg::Vec3 lookAt;
	osg::Vec3 up;
	trackBall->getInverseMatrix().getLookAt(camPos, lookAt, up);

	lookAt = center;

	m_cameraHomePos = camPos;
	m_cameraHomeLookAt = lookAt;
	m_cameraHomeUp = up;
	trackBall->setHomePosition(camPos, lookAt, up);
	return true;
}


//
//Set cameras vfov
//
void HogBoxViewer::SetCameraFOV(const double& fov)
{
	m_vfov = fov;
	if(m_viewer.valid())
	{
		double fovy, aspect, zNear, zFar;
		m_viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspect, zNear, zFar);
		m_viewer->getCamera()->setProjectionMatrixAsPerspective(m_vfov, aspect, zNear,zFar);
	}
}

const double& HogBoxViewer::GetCameraFOV()const
{
	return m_vfov;
}
