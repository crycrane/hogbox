#include <hogbox/HogBoxViewer.h>

#include <iostream>

#include <osg/GLExtensions>


using namespace hogbox;

HogBoxViewer::HogBoxViewer(HWND hwnd)
	: osg::Object(),
    //assume the worst for support
    _glSystemInfo(NULL),
    _screenID(0),	
    _viewer(NULL),	
    _hwnd(hwnd),
    _graphicsContext(NULL),
    _graphicsWindow(NULL),
    _resizeCallback(NULL),
    //saving
    _viewerSettingsFile(""),
    _scene(NULL),
	_requestReset(false),
	//window size in pixels
	_winSize(osg::Vec2(800, 600)),
	_prevWinSize(_winSize),
	//window corner in pixels (Windows bottom left is 0,0)
	_winCorner(osg::Vec2(0, 0)),
	_prevWinCorner(_winCorner),
	//
	_doubleBuffer(true),
	_vSync(false),
	_colorBits(24),
	_depthBits(24),
	_alphaBits(8),
	_stencilBits(0),
	_accumulationBits(0),
	//is app in fullscreen mode
	_bIsFullScreen(false),
	_bUsingBoarder(true),
	_windowName("HogBoxViewer"),
	_usingCursor(true),
//stereo
	//are we using stereo
	_bStereoEnabled(false),
	//current eyeseperation
	_fStereoSep(0.006f),
	_swapEyes(false),
	//current convergence distance from camera
	_fStereoConv(5.0f),
	//type of stereo display
	_iStereoMode(1), //anaglyph
//rendering
	_clearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f)),
	//antialiasing samples
	_aaSamples(0), //try for 4, systeminfo will prevent it if not supported
//view/camera
	//field of view of camera
	_vfov(45.0f),
	_cameraViewMatrix(osg::Matrix::identity()),
	_viewDistance(4.0f), //used for saving trackball distances
	_cameraHomePos(osg::Vec3(0.0f,0.0f,0.0f)),
	_cameraHomeLookAt(osg::Vec3(0.0f,0.0f,-1.0f)),
	_cameraHomeUp(osg::Vec3(0,1,0)),
//renderOffscreen
	_bRenderOffscreen(false),
    _frameBufferImage(NULL),
//IOS specific
	_deviceOrientationFlags(IGNORE_ORIENTATION),
    _contentScale(1.0f)
{
	_glSystemInfo = SystemInfo::Inst();
}

HogBoxViewer::~HogBoxViewer(void)
{
	OSG_NOTICE << "    Deallocating HogBoxViewer: Name '" << this->getName() << "'." << std::endl;

	_scene = NULL;

	_resizeCallback = NULL;
	
	for(unsigned int i=0; i<_appEventHandlers.size(); i++)
	{_appEventHandlers[i] = NULL;}
	_appEventHandlers.clear();

	_viewer = NULL;

	//handle to the context window for manipulaing fullscreen etc
	_graphicsWindow = NULL;

	//single context to attach to the viewers camera
	_graphicsContext = NULL;
}

//
//init contructing window and viewer, optionaly passing in render mode args
//
int HogBoxViewer::Init(osg::Node* scene, bool fullScreen, 
					   osg::Vec2 winSize, osg::Vec2 winCr, 
					   bool realizeNow, unsigned int screenID, bool useStereo, 
					   int stereoMode, bool renderOffscreen)
{

	_screenID = screenID;

	//good default fov
	int width = _glSystemInfo->getScreenWidth(_screenID);
    int height = _glSystemInfo->getScreenHeight(_screenID);
	osg::Vec2 screenRes = osg::Vec2(width,height);
    //double distance = 0.5; //osg::DisplaySettings::instance()->getScreenDistance();
   // _vfov = osg::RadiansToDegrees(atan2(height/2.0f,distance)*2.0);

	//set the scene to render
	_scene = scene;

    //is app in fullscreen mode
	_bIsFullScreen = fullScreen;
    
	//window size in pixels
	_winSize = winSize;
    
    if(_bIsFullScreen){
        _winSize = screenRes;
    }
    
    //window corner in pixels ( Windows bottom left is 0,0)
	if(winCr.x()==-1)//pass neg to center
	{
		//set the default window to center
		if(_winSize.x()>screenRes.x()){_winSize.x() = screenRes.x();}
		if(_winSize.y()>screenRes.y()){_winSize.y() = screenRes.y();}
		_winCorner = osg::Vec2((screenRes.x()/2)-(_winSize.x()/2), (screenRes.y()/2)-(_winSize.y()/2));
	}else{
		_winCorner = winCr;
	}
    
	//are we using stereo
	_bStereoEnabled = useStereo;

	//type of stereo display
	_iStereoMode = stereoMode; //anaglyph

	//Do we want to render to offscreen image target
	_bRenderOffscreen = renderOffscreen;

	//if we have a valid scene then construc the window, else divert till later
	if(_scene.valid() && realizeNow)
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
	if(_viewer.valid())
	{
		if(_resizeCallback != NULL)
		{
			_winSize = _resizeCallback->GetWinSize();
			_winCorner = _resizeCallback->GetWinCorner();
		}
		_viewer->frame();
	}
}

//
//return viewer done state
//
bool HogBoxViewer::done()
{
	if(_viewer.valid())
	{
		return _viewer->done();
	}
	return true;
}

void HogBoxViewer::addEventHandler(osgGA::GUIEventHandler* eventHandler)
{
	//add to our list of eventHandler
	_appEventHandlers.push_back((EventHandlerObserver)eventHandler);
	if(_viewer.valid())
	{
		_viewer->addEventHandler(eventHandler);
	}
}

void HogBoxViewer::removeEventHandler(osgGA::GUIEventHandler* eventHandler)
{
	//add to our list of eventHandler
    std::vector<EventHandlerObserver>::iterator itr = std::find(_appEventHandlers.begin(), _appEventHandlers.end(), eventHandler);
    if (itr == _appEventHandlers.end())
    {
        _appEventHandlers.push_back(eventHandler);
    }
    
	if(_viewer.valid())
	{
		_viewer->removeEventHandler(eventHandler);
	}
}

//
//contruct the window and viewer using existing settings
//returns false if no scene node set
//
bool HogBoxViewer::CreateAppWindow()
{
	//if not philips mode
	if( _iStereoMode != 9 && _iStereoMode != 10)
	{
		//if the implentation window exists get the current dimensions and close it
		if(_graphicsWindow != NULL)
		{
			int x, y, w, h;
			_graphicsWindow->getWindowRectangle(x, y, w, h);
			_winSize = osg::Vec2(w,h);
			_winCorner = osg::Vec2(x,y);
			_graphicsWindow->close(); //setCursor(osgViewer::GraphicsWindow::MouseCursor::
			_graphicsWindow = NULL;
		}

		// create the window to draw to.
		osg::GraphicsContext::Traits* graphicsTraits = new osg::GraphicsContext::Traits;
		graphicsTraits->screenNum = _screenID;
		graphicsTraits->x = _winCorner.x();
		graphicsTraits->y = _winCorner.y();
		graphicsTraits->width = _winSize.x();
		graphicsTraits->height = _winSize.y();
		graphicsTraits->red = _colorBits/3;
		graphicsTraits->green = _colorBits/3;
		graphicsTraits->blue = _colorBits/3;
		graphicsTraits->depth = _depthBits;//_glSystemInfo->maxDepthBufferBits();
		graphicsTraits->alpha = _alphaBits;
		graphicsTraits->stencil = _stencilBits;
		graphicsTraits->doubleBuffer = _doubleBuffer; //_glSystemInfo->doubleBufferedStereoSupported();
		graphicsTraits->sharedContext = 0;
		graphicsTraits->windowDecoration = _bUsingBoarder;
		graphicsTraits->useCursor = _usingCursor;
		graphicsTraits->windowName = _windowName;
		graphicsTraits->vsync = _vSync;
        
		//attach to any handle if available
		if(_hwnd != NULL)
		{
			// Init the Windata Variable that holds the handle for the Window to display OSG in.
			graphicsTraits->setInheritedWindowPixelFormat = true;
			osg::ref_ptr<osg::Referenced> windata;
			
#ifdef WIN32 
			windata = new osgViewer::GraphicsWindowWin32::WindowData(_hwnd);
#else
			#if (TARGET_OS_IPHONE)
				windata = new osgViewer::GraphicsWindowIOS::WindowData((UIView*)_hwnd, _deviceOrientationFlags, _contentScale);
			#else
                windata = NULL;//new osgViewer::GraphicsWindowCarbon::WindowData((OpaqueWindowPtr*)_hwnd);
			#endif
#endif
			graphicsTraits->inheritedWindowData = windata;
			graphicsTraits->windowDecoration = false;
			_bUsingBoarder = false;
		}else{
			//IOS pass windata anyhow so we can set auto rotate
			#if (TARGET_OS_IPHONE)
				osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowIOS::WindowData(NULL, _deviceOrientationFlags, _contentScale);
				graphicsTraits->inheritedWindowData = windata;
			#endif			
		}
        
        //
        graphicsTraits->supportsResize = true;


		//apply samles if supported
		if(_glSystemInfo->multiSamplingSupported())
        {
            //if the requested samples are within supported range
            if(_glSystemInfo->maxMultiSamplesSupported() >= _aaSamples)
            {
                graphicsTraits->samples = _aaSamples;
            }else{
                graphicsTraits->samples = _glSystemInfo->maxMultiSamplesSupported();
            }
            if(graphicsTraits->samples > 0){
                graphicsTraits->sampleBuffers = 1;
            }
        }
      
		//apply extra traits required for stereo modes
		if(_bStereoEnabled)//add quad buffered if requested
		{
			if(_iStereoMode==0) //Quadbuffered
			{
				//check its supported
				if(_glSystemInfo->quadBufferedStereoSupported())
				{
					graphicsTraits->quadBufferStereo = true;
				}else{
					OSG_WARN << "HogBoxViewer CreateWindow ERROR: QuadBuffered Stereo has been requested but is not supported. Defaulting to Anaglyph."<<std::endl;
					_iStereoMode = 1;//default to anaglyph
				}
				//interlace and checker modes require stencil
			}else if(_iStereoMode==6 || _iStereoMode==7 || _iStereoMode==8){
				
				_stencilBits = 8;
				graphicsTraits->stencil = _stencilBits;
			}
		}

		//if a context exists, release it pointer
		if(_graphicsContext != NULL)
		{
			_graphicsContext = NULL;
		}

		//now check for offscreen rendering and change our traits accordingly
		if(_bRenderOffscreen)
		{
			_frameBufferImage = new osg::Image();
			_frameBufferImage->allocateImage(_winSize.x(), _winSize.y(), 1, GL_BGR, GL_UNSIGNED_BYTE);
			graphicsTraits->windowDecoration = false;
			//graphicsTraits->doubleBuffer = false; //_glSystemInfo->doubleBufferedStereoSupported();
			graphicsTraits->sharedContext = 0;
			graphicsTraits->pbuffer = true;
		}

		//create graphics context using requested traits
		_graphicsContext = osg::GraphicsContext::createGraphicsContext(graphicsTraits);

		if(!_graphicsContext.valid())
		{
			OSG_WARN << "HogBoxViewer CreateWindow ERROR: Failed to create graphicsContext, viewer is not vaild." << std::endl;
			return false;
		}

		//cast the context to a window to set position etc
		_graphicsWindow = dynamic_cast<osgViewer::GraphicsWindow*>(_graphicsContext.get());

		if (!_graphicsWindow && !_bRenderOffscreen)
		{
			OSG_WARN << "HogBoxViewer CreateWindow ERROR: Failed to create graphicsContext, viewer is not vaild." << std::endl;
			return false;
		}

		//now window is created apply settings that don't seem to persist from the traits (useCursor)
		this->SetCursorVisible(_usingCursor);

		// create the view of the scene.
		if(_viewer != NULL)
		{
			//release old
			_viewer->done();
			_viewer = NULL;
		}

		//create the osg viewer
		_viewer = new osgViewer::Viewer();
		_viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
		
		//set cameras projection and viewport
		double height = _glSystemInfo->getScreenWidth(_screenID);
		double width = _glSystemInfo->getScreenHeight(_screenID);
		osg::Vec2 screenRes = osg::Vec2(width,height);

		_viewer->getCamera()->setProjectionMatrixAsPerspective( _vfov, _winSize.x()/_winSize.y(), 1.0f,1000.0f);

		//attach the graphics context to the viewers camera
		_viewer->getCamera()->setGraphicsContext(_graphicsContext.get());
		_viewer->getCamera()->setViewport(0,0,_winSize.x(),_winSize.y());
		_viewer->getCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		_viewer->getCamera()->setClearColor(_clearColor);
		
		//also bind our buffer image if rendering offscreen
		if(_bRenderOffscreen)
		{
			// tell the camera to use OpenGL frame buffer objects
			_viewer->getCamera()->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
			_viewer->getCamera()->attach(osg::Camera::COLOR_BUFFER, _frameBufferImage.get());
		}
		
		//attach the resize callback
		//destroy old callback if it exists]
		if(_resizeCallback)
		{
			_resizeCallback = NULL;
		}
		#ifndef TARGET_OS_IPHONE
			_resizeCallback = new HogBoxViewerResizedCallback(_viewer.get(), _winCorner.x(), _winCorner.y(), _winSize.x(), _winSize.y());
			_graphicsContext->setResizedCallback(_resizeCallback);
		#endif
		//force a resize to acount for the initial state
		//_resizeCallback->resizedImplementation(_graphicsContext, _winCorner.x(), _winCorner.y(), _winSize.x(), _winSize.y());
	
		// set the scene to render
		//check a scene has been set
		if(!_scene)
		{
			OSG_WARN << "HogBoxViewer CreateWindow WARN: No SceneNode has been set the viewer will only render the ClearColor"<<std::endl;
		}else{
			_viewer->setSceneData(_scene.get());
		}

		//set our default cull masks
		_viewer->getCamera()->setCullMask(MAIN_CAMERA_CULL);
		_viewer->getCamera()->setCullMaskLeft(MAIN_CAMERA_LEFT_CULL);
		_viewer->getCamera()->setCullMaskRight(MAIN_CAMERA_RIGHT_CULL);

		// set up the use of stereo by default. (Is this the only way of doing this ?)
		osg::DisplaySettings::instance()->setStereo(_bStereoEnabled);
		osg::DisplaySettings::instance()->setStereoMode((osg::DisplaySettings::StereoMode) _iStereoMode);

		//set seperation based on swap
		if(_swapEyes)
		{
			osg::DisplaySettings::instance()->setEyeSeparation( -_fStereoSep );
		}else{
			osg::DisplaySettings::instance()->setEyeSeparation( _fStereoSep );
		}
		_viewer->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE, _fStereoConv );


		//add any external app event handler
		for(unsigned int i=0; i<_appEventHandlers.size(); i++)
		{
			_viewer->addEventHandler(_appEventHandlers[i].get());
		}

		//everything is inplace so realize our viewer (creating window etc)
		#ifndef TARGET_OS_IPHONE
			_viewer->realize();
			//force a resize to get a proper initial state
			_resizeCallback->resizedImplementation(_graphicsContext,_winCorner.x(),_winCorner.y(),_winSize.x(),_winSize.y());
		#endif

	
		//set request back to false
		_requestReset = false;

		if(_viewer->isRealized())
		{
			//if we want full sceen mode set it now
			if(_bIsFullScreen)
			{
//				CMsgLog::Inst()->WriteToLog("Window Created");
				this->SetFullScreen(_bIsFullScreen);
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
	return _requestReset;
}

//
// return the viewer to be passed to eventhandlers etc
//
osg::ref_ptr<osgViewer::Viewer> HogBoxViewer::GetViewer()
{
    return _viewer;
}

//
//Return the context created by CreateAppWindow
osg::ref_ptr<osg::GraphicsContext> HogBoxViewer::GetContext()
{
    return _graphicsContext;
}

//
//set the scene node, resteting the viewer if requested
//
void HogBoxViewer::SetSceneNode(osg::Node* scene, bool resetWindow)
{
	_scene = scene;
	if(_viewer.valid()){_viewer->setSceneData(_scene.get());}
	if(resetWindow){CreateAppWindow();}
}

//
//window

void HogBoxViewer::SetScreenID(const unsigned int& screen)
{
	//check it's in range of total displays
	if(screen >= _glSystemInfo->getNumberOfScreens()){return;}
	
	//if it's a different screen, reset
	if(screen != _screenID){_requestReset=true;}

	_screenID = screen;
}

const unsigned int& HogBoxViewer::GetScreenID()const
{
	return _screenID;
}

//
//set the window size
//
void HogBoxViewer::SetWindowSize(const osg::Vec2& winSize)
{
	_winSize = winSize;
	//if window exists apply straight away
	if(_graphicsWindow != NULL)
	{
		_graphicsWindow->setWindowRectangle(_winCorner.x(), _winCorner.y(), _winSize.x(), _winSize.y());
		//also set the viewport
		_viewer->getCamera()->getViewport()->setViewport(0,0,_winSize.x(),_winSize.y()); 
	}
}

const osg::Vec2& HogBoxViewer::GetWindowSize()const
{
	return _winSize;
}

//
//position window
//
void HogBoxViewer::SetWindowCorner(const osg::Vec2& winCr)
{
	_winCorner = winCr;
	//if window exists apply straight away
	if(_graphicsWindow != NULL)
	{
		_graphicsWindow->setWindowRectangle(_winCorner.x(), _winCorner.y(), _winSize.x(), _winSize.y());
	}
}

const osg::Vec2& HogBoxViewer::GetWindowCorner()const
{
	return _winCorner;
}

void HogBoxViewer::SetWindowDecoration(const bool& enableBoarder)
{
	_bUsingBoarder = enableBoarder;
	if(_graphicsWindow != NULL)
	{
		_graphicsWindow->setWindowDecoration(_bUsingBoarder);
	}
}

const bool& HogBoxViewer::GetWindowDecoration()const
{
	return _bUsingBoarder;
}

void HogBoxViewer::SetDoubleBuffered(const bool& doubleBuffer)
{
	if(!_glSystemInfo->doubleBufferSupported()){
		_doubleBuffer = false;
	}else{
		_doubleBuffer = doubleBuffer;
	}
	this->_requestReset = true;
}

const bool& HogBoxViewer::GetDoubleBuffered()const
{
	return _doubleBuffer;
}

void HogBoxViewer::SetVSync(const bool& vsync)
{
	_vSync = vsync;
	if(_graphicsWindow != NULL)
	{
		_graphicsWindow->setSyncToVBlank(_vSync);
	}
}

const bool& HogBoxViewer::GetVSync()const
{
	return _vSync;
}

void HogBoxViewer::SetColorBits(const unsigned int& bits)
{
	_colorBits = bits;
	this->_requestReset = true;
}

const unsigned int& HogBoxViewer::GetColorBits()const
{
	return _colorBits;
}

void HogBoxViewer::SetDepthBits(const unsigned int& bits)
{
	if(bits > _glSystemInfo->maxDepthBufferBits())
	{
		_depthBits = _glSystemInfo->maxDepthBufferBits();
	}else{
        _depthBits = bits;
	}
	this->_requestReset = true;	
}

const unsigned int& HogBoxViewer::GetDepthBits()const
{
	return _depthBits;
}

void HogBoxViewer::SetAlphaBits(const unsigned int& bits)
{
	_alphaBits = bits;
	this->_requestReset = true;
}

const unsigned int& HogBoxViewer::GetAlphaBits()const
{
	return _alphaBits;
}

void HogBoxViewer::SetStencilBits(const unsigned int& bits)
{
	if(bits > _glSystemInfo->maxStencilBitsSupported())
	{
		_stencilBits = _glSystemInfo->maxStencilBitsSupported();
	}else{
		_stencilBits = bits;
	}
	this->_requestReset = true;
}
const unsigned int& HogBoxViewer::GetStencilBits()const
{
	return _stencilBits;
}

void HogBoxViewer::SetAccumulationBits(const unsigned int& bits)
{
	_accumulationBits = bits;
	this->_requestReset = true;
}

const unsigned int& HogBoxViewer::GetAcculationBits()const
{
	return _accumulationBits;
}

//
//set fullscreen resizing to _sSize, must be set by init()
//
void HogBoxViewer::SetFullScreen(const bool& fullScreen)
{
	_bIsFullScreen = fullScreen;

	if(!_graphicsWindow){return;}

	if(fullScreen) //fullscreen mode
	{
		//capture current window size and corner, so we can swap back when we leave fullscreen mode
		int x, y, w, h;
		_graphicsWindow->getWindowRectangle(x, y, w, h);
		_winSize = osg::Vec2(w,h);
		_prevWinSize = _winSize;
		_winCorner = osg::Vec2(x,y);
		_prevWinCorner = _winCorner;

		//set the window to sSize at 0,0
		_graphicsWindow->setWindowRectangle(0, 0, _glSystemInfo->getScreenWidth(_screenID), _glSystemInfo->getScreenHeight(_screenID));
		//also set the viewport
		_viewer->getCamera()->getViewport()->setViewport(0,0,_glSystemInfo->getScreenWidth(_screenID),_glSystemInfo->getScreenWidth(_screenID)); 
		//remove decoration
		_bUsingBoarder=false;
		_graphicsWindow->setWindowDecoration(_bUsingBoarder);
		_bIsFullScreen=true;

	}else{
		_bUsingBoarder=true;
		_graphicsWindow->setWindowDecoration(_bUsingBoarder);
		//get previous window corner
		_winSize = _prevWinSize;
		_winCorner = _prevWinCorner;

		//clip to screen
		//ClipWinToScreen(_screenID);

		_graphicsWindow->setWindowRectangle(_winCorner.x(), _winCorner.y(), _winSize.x(), _winSize.y());
		//also set the viewport
		_viewer->getCamera()->getViewport()->setViewport(0,0,_winSize.x(),_winSize.y()); 
		_bIsFullScreen=false;
	}
}

const bool& HogBoxViewer::isFullScreen()const
{
	return _bIsFullScreen;
}

void HogBoxViewer::ClipWinToScreen(int screenID)
{

	//float boarder = 20;
	//float xMax = _winCorner.x() + _winSize.x();
	//float xMin = _winCorner.x();
	//if(xMin

	//float yMax = _winCorner.y() + _winSize.y();
	float yMin = _winCorner.y();
	OSG_FATAL << "Win corner y " << _winCorner.y() << std::endl;
	//if the win is off top of screen and we have a boarder
	//move down so user can still use bar
	/*if( (yMax+boarder >= _sSize.y()) && _bUsingBoarder)
	{
		float dif = (yMax)-_sSize.y()+boarder;
		printf("Win Diff y %f\n", dif);
		_winCorner.y() +=  dif;
	}*/

	if(yMin<0)
	{
		_winCorner.y() = -_winCorner.y();
	}
}

void HogBoxViewer::SetWindowName(const std::string& windowName)
{
	_windowName = windowName;
	if(_graphicsWindow == NULL){return;}
	_graphicsWindow->setWindowName( _windowName);
}

const std::string& HogBoxViewer::GetWindowName()const
{
	return _windowName;
}

void HogBoxViewer::SetCursorVisible(const bool& visible)
{
	_usingCursor = visible;
	if(_graphicsWindow == NULL){return;}
	_graphicsWindow->useCursor(visible);
}

const bool& HogBoxViewer::isCursorVisible()const
{
	return _usingCursor;
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
	_bStereoEnabled = useStereo;

	if(_viewer.valid())
	{
		osg::DisplaySettings::instance()->setStereo(_bStereoEnabled);
	}
}

const bool& HogBoxViewer::isUsingStereo()const
{
	return _bStereoEnabled;
}

void HogBoxViewer::SetStereoConvDistance(const float& distance)
{
	_fStereoConv = distance;
	if(_viewer.valid())
	{
		_viewer->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE, _fStereoConv);
	}
}

const float& HogBoxViewer::GetStereoConvDistance()const
{
	return _fStereoConv;
}

void HogBoxViewer::SetStereoEyeSeperation(const float& distance)
{
	_fStereoSep = distance;
	if(_viewer.valid())
	{
		//apply the eye swapping
		if(_swapEyes)
		{
			osg::DisplaySettings::instance()->setEyeSeparation(-_fStereoSep);
		}else{
			osg::DisplaySettings::instance()->setEyeSeparation(_fStereoSep);
		}
	}
}

const float& HogBoxViewer::GetStereoEyeSeperation()const
{
	return _fStereoSep;
}

//
// Tell viewer to swap eyes when applying setEyeSeparation
//
void HogBoxViewer::SetSwapEyes(const bool& swap)
{
	_swapEyes = swap;
	SetStereoEyeSeperation(GetStereoEyeSeperation());
}

const bool& HogBoxViewer::GetSwapEyes()const
{
	return _swapEyes;
}

//
//set stereo mopde. Will also recreate window with existing scene data if the mode require a different context
//
void HogBoxViewer::SetStereoMode(const int& mode)
{
	//store so we can check if the context needs re creating
	int prevMode = _iStereoMode;

	//no need to set if there the same
	if(prevMode == mode)
	{return;}

	_iStereoMode = mode;

	//if viewer exists then set the stereo imidiatly
	if(_viewer.valid())
	{
		//check for philips modes
		if(_iStereoMode == 9 || _iStereoMode == 10)
		{

		}else{

			//do we need to recreate window
			_requestReset = false;

			//if the previous mode was qad buffered we always need to recreate
			if( prevMode == osg::DisplaySettings::QUAD_BUFFER)
			{
				_requestReset = true;
			}

			//check for particular different modes that will require a new context
			switch(_iStereoMode)
			{
				//if its a stencil mode (interlaced/checkboard) then we need to reset
				case osg::DisplaySettings::HORIZONTAL_INTERLACE:
				case osg::DisplaySettings::VERTICAL_INTERLACE:
				case osg::DisplaySettings::CHECKERBOARD:
				{
					_requestReset = true;//only way to currently force a redrawe of the stencil mask
					break;
				}
				case osg::DisplaySettings::QUAD_BUFFER:
				{
				    if(_glSystemInfo->quadBufferedStereoSupported())
                    {
                        _requestReset = true;
                    }else{
                        //if it's not supported ensure we revert to a safe on
                        _iStereoMode = prevMode;
						return;
                    }
					break;
				}
				default:break;
			}

			osg::DisplaySettings::instance()->setStereoMode((osg::DisplaySettings::StereoMode)_iStereoMode);
			return;
		}
	}
	return;
}

const int& HogBoxViewer::GetStereoMode()const
{
	return _iStereoMode;
}

//
//rendering

//
//set the clear color for rendering
//
void HogBoxViewer::SetClearColor(const osg::Vec4& color)
{
	_clearColor = color;

	if(_viewer)
	{_viewer->getCamera()->setClearColor(_clearColor);}
}

const osg::Vec4& HogBoxViewer::GetClearColor()const
{
	return _clearColor;
}

//
//set the number of aa samples reseting context if required
//
void HogBoxViewer::SetAASamples(const int& samples)
{
	//return if its the same or out of range
	if( (_aaSamples < 0) || (_aaSamples == samples))
	{return;}

	if(_aaSamples>_glSystemInfo->maxMultiSamplesSupported()) 
	{
		_aaSamples = _glSystemInfo->maxMultiSamplesSupported();
	}else{
		_aaSamples = samples;
	}
	_requestReset = true;
}

const int& HogBoxViewer::GetAASamples()const
{
	return _aaSamples;
}

//
//Return viewers camera
//
osg::Camera* HogBoxViewer::GetCamera()
{
   	if(_viewer.valid())
	{
		return _viewer->getCamera();
	} 
    return NULL;
}

//
//view/camera
//
void HogBoxViewer::SetProjectionMatrix(osg::Projection* projection)
{
	if(_viewer.valid())
	{
		_viewer->getCamera()->setProjectionMatrix(projection->getMatrix());
	}
}

//
//set cameras view matix
//
void HogBoxViewer::SetCameraViewMatrix(const osg::Matrix& viewMatrix)
{
	_cameraViewMatrix = viewMatrix;
	if(_viewer.valid())
	{
		_viewer->getCamera()->setViewMatrix(viewMatrix);
	}
}

const osg::Matrix& HogBoxViewer::GetCameraViewMatrix()const
{
	return _cameraViewMatrix;
}

//
// Set the home lookat vectors
//
void HogBoxViewer::SetCameraViewMatrixFromLookAt(osg::Vec3 camPos, osg::Vec3 lookAtPos, osg::Vec3 upVec)
{

    osg::Matrix lookAt = osg::Matrix::lookAt(camPos, lookAtPos, upVec);
    this->SetCameraViewMatrix(lookAt);
}

//
// Compute a good camera view matrix for the current scene
//
void HogBoxViewer::SetCameraViewMatrixFromSceneBounds(osg::Vec3 forwardAxis, osg::Vec3 upAxis)
{
    if(!_scene.get()){
        return;
    }
    forwardAxis.normalize();
    osg::BoundingSphere bounds = _scene->computeBound();
    osg::Vec3 lookAt = bounds.center();
    osg::Vec3 camPos = lookAt + (-forwardAxis * (bounds.radius()*3.0f));
    osg::Vec3 up = upAxis;
    up.normalize();
    this->SetCameraViewMatrixFromLookAt(camPos, lookAt, up);
}

void HogBoxViewer::SetCameraViewDistance(const float& distance)
{
	_viewDistance = distance;
}

const float& HogBoxViewer::GetCameraViewDistance()const
{
	return _viewDistance;
}

//
// Set the home lookat vectors
bool HogBoxViewer::SetHomeLookAtVectors(osg::Vec3 camPos, osg::Vec3 lookAtPos, osg::Vec3 upVec)
{
	_cameraHomePos = camPos;
	_cameraHomeLookAt = lookAtPos;
	_cameraHomeUp = upVec;
	return true;
}

//
//returns true if a home matrix has been set passin the values through the paased in args
//
bool HogBoxViewer::GetHomeLookAtVectors(osg::Vec3& camPos, osg::Vec3& lookAtPos, osg::Vec3& upVec)
{
	camPos = _cameraHomePos;
	lookAtPos = _cameraHomeLookAt;
	upVec = _cameraHomeUp;

	//float distance = 0.0f;
	//_cameraHomeMatrix.getLookAt(camPos, lookAtPos, upVec, distance);

	return true;
}

//
//Helper to set a lookat using a trackball (captures the current state of the track ball as lookat vectors)
bool HogBoxViewer::SetHomeLookAtVectorsFromTrackBall(osgGA::TrackballManipulator* trackBall)
{
	//_hogViewer->SetCameraHomeMatrix(_hogViewer->GetCameraViewMatrix());
	//_hogViewer->SetCameraHomeDistance(cameraManipulator->getDistance());
	osg::Vec3 center = trackBall->getCenter();

	osg::Vec3 camPos;
	osg::Vec3 lookAt;
	osg::Vec3 up;
	trackBall->getInverseMatrix().getLookAt(camPos, lookAt, up);

	lookAt = center;

	_cameraHomePos = camPos;
	_cameraHomeLookAt = lookAt;
	_cameraHomeUp = up;
	trackBall->setHomePosition(camPos, lookAt, up);
	return true;
}

//
//Set camera view matrix to home position
//
void HogBoxViewer::SetCameraViewMatrixToHomePosition()
{
    this->SetCameraViewMatrixFromLookAt(_cameraHomePos, _cameraHomeLookAt, _cameraHomeUp);
}

//
//Set cameras vfov
//
void HogBoxViewer::SetCameraFOV(const double& fov)
{
	_vfov = fov;
	if(_viewer.valid())
	{
		double fovy, aspect, zNear, zFar;
		_viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspect, zNear, zFar);
		_viewer->getCamera()->setProjectionMatrixAsPerspective(_vfov, aspect, zNear,zFar);
	}
}

const double& HogBoxViewer::GetCameraFOV()const
{
	return _vfov;
}

void HogBoxViewer::SetDeviceOrientationFlags(const DeviceOrientationFlags& flags)
{
	_deviceOrientationFlags = flags;
}

const HogBoxViewer::DeviceOrientationFlags& HogBoxViewer::GetDeviceOrientationFlags()const
{
	return _deviceOrientationFlags;
}

void HogBoxViewer::SetContentScaleFactor(const float& scale)
{
    _contentScale = scale;
}

const float& HogBoxViewer::GetContentScaleFactor()const
{
    return _contentScale;
}