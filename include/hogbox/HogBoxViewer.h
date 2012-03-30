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

#include <osgViewer/Viewer>
#ifdef WIN32
	#include <windows.h> //for window handle stuff
	#include <osgViewer/api/win32/GraphicsWindowWin32> //include platform specifiec graphics window implementation
#else
	#include <TargetConditionals.h>
	#if (TARGET_OS_IPHONE)
		#include <osgViewer/api/IOS/GraphicsWindowIOS> 
		#define HWND UIView*
		//USE_GRAPHICSWINDOW()
	#else
		#include <osgViewer/api/Carbon/GraphicsWindowCarbon> 
		#define HWND unsigned long
	#endif
#endif

#include <osgGA/TrackballManipulator>
#include <hogbox/SystemInfo.h>


namespace hogbox {


//
// HogViewer Resize callback,
// Handle resizing glViewport as well as informing HogHud of a resize event
//
class HogBoxViewerResizedCallback : public osg::GraphicsContext::ResizedCallback //Referenced
{
public:
	HogBoxViewerResizedCallback(osgViewer::Viewer* viewer,int x, int y, int width, int height)
	{
		p_viewer = viewer;
		_winSize = osg::Vec2(width,height);
		_winCorner = osg::Vec2(x,y);
	}
	
	virtual void resizedImplementation(osg::GraphicsContext* gc, int x, int y, int width, int height)
	{
		_winSize = osg::Vec2(width,height);
		_winCorner = osg::Vec2(x,y);

		p_viewer->getCamera()->getViewport()->setViewport(0,0,width,height);

		double fovy, aspect, zNear, zFar;
		p_viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspect, zNear, zFar);
		aspect = (float)width/height;
		p_viewer->getCamera()->setProjectionMatrixAsPerspective(fovy, aspect, zNear, zFar);

		gc->resizedImplementation(x,y,width, height);
	}
	
	osg::Vec2 GetWinSize(){return _winSize;}
	osg::Vec2 GetWinCorner(){return _winCorner;}

protected:

	virtual ~HogBoxViewerResizedCallback(void)
	{	
	}

protected:

	osgViewer::Viewer* p_viewer;
	osg::Vec2 _winSize;
	osg::Vec2 _winCorner;
};


//
// HogBox viewer encapsulates the setting up of graphics contexts, with the setting
// of various stereo modes
//
class HOGBOX_EXPORT HogBoxViewer : public osg::Object 
{

public:
    
    enum DeviceOrientation{
        IGNORE_ORIENTATION = 0,
        PORTRAIT_ORIENTATION = 1<<0,
        PORTRAIT_UPSIDEDOWN_ORIENTATION  = 1<<1,
        LANDSCAPE_LEFT_ORIENTATION  = 1<<2,
        LANDSCAPE_RIGHT_ORIENTATION  = 1<<3,
        ALL_ORIENTATIONS = PORTRAIT_ORIENTATION  | PORTRAIT_UPSIDEDOWN_ORIENTATION  | LANDSCAPE_LEFT_ORIENTATION  | LANDSCAPE_RIGHT_ORIENTATION
    };
    typedef unsigned int DeviceOrientationFlags;

	HogBoxViewer(HWND hwnd = NULL);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxViewer(const HogBoxViewer&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY){}
	META_Object(hogbox,HogBoxViewer);
	

	//init contructing window and viewer, optionaly passing in render mode args
	int Init(osg::Node* scene,  bool fullScreen = false, 
				osg::Vec2 winSize=osg::Vec2(640, 480), osg::Vec2 winCr=osg::Vec2(-1,-1), 
				bool realizeNow = true,unsigned int screenID = 0, 
				bool useStereo = false, int stereoMode = 1, 
				bool renderOffscreen = false);


	//mimic osgViewer funcs
	void frame();

	bool done();

	void addEventHandler(osgGA::GUIEventHandler* eventHandler);
	void removeEventHandler(osgGA::GUIEventHandler* eventHandler);


	//contruct the window and viewer using existing settings
	//returns false if no scene node set
	//if bool renderOffScreenImage is true a pbuffer object is used instead and the
	//_frameBufferImage is attached to catch our renders
	bool CreateAppWindow();

	bool isRequestingReset();
    
    //get common viewer stuff (perhaps this should even just inherit from osgViewer?)
	osg::ref_ptr<osgViewer::Viewer> GetViewer();
    osg::ref_ptr<osg::GraphicsContext> GetContext();
    

	//set the scene node, resteting the viewer if requested
	void SetSceneNode(osg::Node* scene, bool resetWindow = false);

//window

	//get set screenID
	void SetScreenID(const unsigned int& screen);
	const unsigned int& GetScreenID()const;

	//set the window size
	void SetWindowSize(const osg::Vec2& winSize);
	const osg::Vec2& GetWindowSize()const;

	//position window
	void SetWindowCorner(const osg::Vec2& winCr);
	const osg::Vec2& GetWindowCorner()const;

	void SetWindowDecoration(const bool& enableBoarder);
	const bool& GetWindowDecoration()const;

	void SetDoubleBuffered(const bool& doubleBuffer);
	const bool& GetDoubleBuffered()const;

	void SetVSync(const bool& vsync);
	const bool& GetVSync()const;

	void SetColorBits(const unsigned int& bits);
	const unsigned int& GetColorBits()const;

	void SetDepthBits(const unsigned int& bits);
	const unsigned int& GetDepthBits()const;

	void SetAlphaBits(const unsigned int& bits);
	const unsigned int& GetAlphaBits()const;

	void SetStencilBits(const unsigned int& bits);
	const unsigned int& GetStencilBits()const;

	void SetAccumulationBits(const unsigned int& bits);
	const unsigned int& GetAcculationBits()const;

	//set fullscreen resizing to _sSize, must be set by init()
	void SetFullScreen(const bool& fullScreen);
	const bool& isFullScreen()const;

	void ClipWinToScreen(int screenID);

	void SetWindowName(const std::string& windowName);
	const std::string& GetWindowName()const;

	void SetCursorVisible(const bool& visible);
	const bool& isCursorVisible()const;


	//save the current frame if using render offscreen mode
	void SaveCurrentFrameBuffer(const std::string& fileName);
	osg::Image* GetCurrentFrameBuffer();

//stereo

	void SetUseStereo(const bool& useStereo);
	const bool& isUsingStereo()const;

	void SetStereoConvDistance(const float& distance);
	const float& GetStereoConvDistance()const;

	void SetStereoEyeSeperation(const float& distance);
	const float& GetStereoEyeSeperation()const;

	void SetSwapEyes(const bool& swap);
	const bool& GetSwapEyes()const;

	//set stereo mopde. Will also recreate window with existing scene data if the mode require a different context
	void SetStereoMode(const int& mode);
	const int& GetStereoMode()const;

//rendering

	void SetClearColor(const osg::Vec4& color);
	const osg::Vec4& GetClearColor()const;

	//set the number of aa samples reseting context if required
	void SetAASamples(const int& samples);
	const int& GetAASamples()const;

//view/camera

    osg::Camera* GetCamera();
    
	void SetProjectionMatrix(osg::Projection* projection); 

	void SetCameraViewMatrix(const osg::Matrix& viewMatrix);
	const osg::Matrix& GetCameraViewMatrix()const;
    
	//
	// Set the home lookat vectors
	void SetCameraViewMatrixFromLookAt(osg::Vec3 camPos, osg::Vec3 lookAtPos, osg::Vec3 upVec);
    
    //
    // Compute a good camera view matrix for the current scene
    void SetCameraViewMatrixFromSceneBounds(osg::Vec3 axis = osg::Y_AXIS, osg::Vec3 upAxis = osg::Z_AXIS);

	void SetCameraViewDistance(const float& distance);
	const float& GetCameraViewDistance()const;

	//
	// Set the home lookat vectors
	bool SetHomeLookAtVectors(osg::Vec3 camPos, osg::Vec3 lookAtPos, osg::Vec3 upVec);
	//
	//returns true if a home matrix has been set passin the values through the paased in args
	bool GetHomeLookAtVectors(osg::Vec3& camPos, osg::Vec3& lookAtPos, osg::Vec3& upVec);

	//Helper to set a lookat using a trackball (captures the current state of the track ball as lookat vectors)
	bool SetHomeLookAtVectorsFromTrackBall(osgGA::TrackballManipulator* trackBall);

    //
    //Set camera view matrix to home position
    void SetCameraViewMatrixToHomePosition();

	void SetCameraFOV(const double& fov);
	const double& GetCameraFOV()const ;
	
//IOS Specific
	
	void SetDeviceOrientationFlags(const DeviceOrientationFlags& flags);
	const DeviceOrientationFlags& GetDeviceOrientationFlags()const;

    void SetContentScaleFactor(const float& scale);
    const float& GetContentScaleFactor()const;

protected:

	//protected destructor for use with ref_ptr
	virtual ~HogBoxViewer(void);

protected:

    //pointer to the system info singleton
    SystemInfo* _glSystemInfo;

//window and viewer system

	//the screen we wish to setup our viewer on
	unsigned int _screenID;

	osg::ref_ptr<osgViewer::Viewer> _viewer;
	//osg::ref_ptr<osg::GraphicsContext::Traits> _graphicsTraits;

	//handle to any external window we are attached to 
	HWND _hwnd;

	//single context to attach to the viewers camera also used as pixel buffer object
	//if renderOffScreenImage = true is passed to create app window
	osg::ref_ptr<osg::GraphicsContext> _graphicsContext;

	//handle to the context window for manipulaing fullscreen etc
	osgViewer::GraphicsWindow* _graphicsWindow;
	
	//resize callback handle adjusting viewport and projection when
	//the window is resized
	osg::ref_ptr<HogBoxViewerResizedCallback> _resizeCallback;

//exterior application data (scene/actionadaptors

	//store the config file for saving to later
	std::string _viewerSettingsFile;

	//pointer to the application scene
	osg::ref_ptr<osg::Node> _scene;

	//list of pointers to app event handlers, stored for if a rebuild of the
	//osgViewer is required
	typedef osg::observer_ptr<osgGA::GUIEventHandler> EventHandlerObserver;
	std::vector<EventHandlerObserver> _appEventHandlers;

    //the osg viewer needs rebuilding (e.g. changed stereo mode)
	bool _requestReset;


//Window buffer res

	//window size in pixels
	osg::Vec2 _winSize;
	osg::Vec2 _prevWinSize;

	//window corner in pixels (MS Windows top left is 0,0)
	osg::Vec2 _winCorner;
	osg::Vec2 _prevWinCorner; //used to store windowed corner while we are in fullScreen mode

	//
	bool _doubleBuffer;
	bool _vSync;

	unsigned int _colorBits;
	unsigned int _depthBits;
	unsigned int _alphaBits;
	unsigned int _stencilBits;
	unsigned int _accumulationBits;

	//is app in fullscreen mode
	bool _bIsFullScreen;

	//tracked for rebuilds
	bool _bUsingBoarder;

	//title/name of window
	std::string _windowName;

	//using cursor
	bool _usingCursor;

//stereo
	//are we using stereo
	bool _bStereoEnabled;
	//current eyeseperation
	float _fStereoSep;
	//flip eyes
	bool _swapEyes;
	//current convergence distance from camera
	float _fStereoConv;
	//type of stereo display
	int _iStereoMode; //anaglyph

//rendering

	osg::Vec4 _clearColor;

	//antialiasing samples
	int _aaSamples;

//view/camera

	//field of view of camera
	double _vfov;

	osg::Matrix _cameraViewMatrix;
	float _viewDistance; //used for saving trackball distances
	//osg::Matrix _cameraHomeMatrix;
	osg::Vec3 _cameraHomePos;
	osg::Vec3 _cameraHomeLookAt;
	osg::Vec3 _cameraHomeUp;
	//float _homeDistance;

//render offscreen
	
	bool _bRenderOffscreen;
	//rendered image used when in offscreen render mode
	osg::ref_ptr<osg::Image> _frameBufferImage;

//IOS Specific
	
	DeviceOrientationFlags _deviceOrientationFlags;
    float _contentScale;

};

typedef osg::ref_ptr<HogBoxViewer> HogBoxViewerPtr;

}; //end hogbox namespace