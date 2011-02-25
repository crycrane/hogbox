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
		#define HWND UIWindow*
		//USE_GRAPHICSWINDOW()
	#else
		#include <osgViewer/api/Carbon/GraphicsWindowCarbon> 
		#define HWND unsigned long
	#endif
#endif

#include <osgGA/TrackballManipulator>
#include <hogbox/SystemInfo.h>


namespace hogbox {


typedef const void (*OnViewerRebuiltFunc)(void);

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
		m_winSize = osg::Vec2(width,height);
		m_winCorner = osg::Vec2(x,y);
	}
	
	virtual void resizedImplementation(osg::GraphicsContext* gc, int x, int y, int width, int height)
	{
		m_winSize = osg::Vec2(width,height);
		m_winCorner = osg::Vec2(x,y);

		p_viewer->getCamera()->getViewport()->setViewport(0,0,width,height);

		double fovy, aspect, zNear, zFar;
		p_viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspect, zNear, zFar);
		aspect = (float)width/height;
		p_viewer->getCamera()->setProjectionMatrixAsPerspective(fovy, aspect, zNear, zFar);

		gc->resizedImplementation(x,y,width, height);
	}
	
	osg::Vec2 GetWinSize(){return m_winSize;}
	osg::Vec2 GetWinCorner(){return m_winCorner;}

protected:

	virtual ~HogBoxViewerResizedCallback(void)
	{	
	}

protected:

	osgViewer::Viewer* p_viewer;
	osg::Vec2 m_winSize;
	osg::Vec2 m_winCorner;
};


//
// HogBox viewer encapsulates the setting up of graphics contexts, with the setting
// of various stereo modes
//
class HOGBOX_EXPORT HogBoxViewer : public osg::Object 
{

public:

	HogBoxViewer(HWND hwnd = NULL);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxViewer(const HogBoxViewer&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY){}
	META_Box(hogbox,HogBoxViewer);
	

	//init contructing window and viewer, optionaly passing in render mode args
	int Init(osg::Node* scene, OnViewerRebuiltFunc onView = NULL, bool realizeNow = true, bool fullScreen = false, 
				osg::Vec2 winSize = osg::Vec2(640, 480), osg::Vec2 winCr = osg::Vec2(-1,-1), 
				unsigned int screenID = 0, bool useStereo = false, int stereoMode = 1, bool renderOffscreen = false);


	//mimic osgViewer funcs
	void frame();

	bool done();

	void addEventHandler(osgGA::GUIEventHandler* eventHandler);


	//contruct the window and viewer using existing settings
	//returns false if no scene node set
	//if bool renderOffScreenImage is true a pbuffer object is used instead and the
	//_frameBufferImage is attached to catch our renders
	bool CreateAppWindow();

	bool isRequestingReset();
	osg::ref_ptr<osgViewer::Viewer> GetViewer();

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

	//set fullscreen resizing to m_sSize, must be set by init()
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

	void SetProjectionMatrix(osg::Projection* projection); 

	void SetCameraViewMatrix(const osg::Matrix& viewMatrix);
	const osg::Matrix& GetCameraViewMatrix()const;

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


	void SetCameraFOV(const double& fov);
	const double& GetCameraFOV()const ;


protected:

	//protected destructor for use with ref_ptr
	virtual ~HogBoxViewer(void);

protected:

    //pointer to the system info singleton
    SystemInfo* m_glSystemInfo;

//window and viewer system

	//the screen we wish to setup our viewer on
	unsigned int m_screenID;

	osg::ref_ptr<osgViewer::Viewer> m_viewer;
	//osg::ref_ptr<osg::GraphicsContext::Traits> m_graphicsTraits;

	//handle to any external window we are attached to 
	HWND m_hwnd;

	//single context to attach to the viewers camera also used as pixel buffer object
	//if renderOffScreenImage = true is passed to create app window
	osg::ref_ptr<osg::GraphicsContext> m_graphicsContext;

	//handle to the context window for manipulaing fullscreen etc
	osgViewer::GraphicsWindow* m_graphicsWindow;
	
	//resize callback handle adjusting viewport and projection when
	//the window is resized
	osg::ref_ptr<HogBoxViewerResizedCallback> m_resizeCallback;

//exterior application data (scene/actionadaptors

	//store the config file for saving to later
	std::string m_viewerSettingsFile;

	//pointer to the application scene
	osg::observer_ptr<osg::Node> m_p_scene;

	//list of pointers to app event handlers, stored for if a rebuild of the
	//osgViewer is required
	typedef osg::observer_ptr<osgGA::GUIEventHandler> EventHandlerObserver;
	std::vector<EventHandlerObserver> m_p_appEventHandlers;

	bool m_requestReset;

	//callback function for when window is reset internally (i.e. from switching stereoModes)
	OnViewerRebuiltFunc p_onViewRebuiltFunc;

//Window buffer res

	//window size in pixels
	osg::Vec2 m_winSize;
	osg::Vec2 m_prevWinSize;

	//window corner in pixels (MS Windows top left is 0,0)
	osg::Vec2 m_winCorner;
	osg::Vec2 m_prevWinCorner; //used to store windowed corner while we are in fullScreen mode

	//
	bool m_doubleBuffer;
	bool m_vSync;

	unsigned int m_colorBits;
	unsigned int m_depthBits;
	unsigned int m_alphaBits;
	unsigned int m_stencilBits;
	unsigned int m_accumulationBits;

	//is app in fullscreen mode
	bool m_bIsFullScreen;

	//tracked for rebuilds
	bool m_bUsingBoarder;

	//title/name of window
	std::string m_windowName;

	//using cursor
	bool m_usingCursor;

//stereo
	//are we using stereo
	bool m_bStereoEnabled;
	//current eyeseperation
	float m_fStereoSep;
	//flip eyes
	bool m_swapEyes;
	//current convergence distance from camera
	float m_fStereoConv;
	//type of stereo display
	int m_iStereoMode; //anaglyph

//rendering

	osg::Vec4 m_clearColor;

	//antialiasing samples
	int m_aaSamples;

//view/camera

	//field of view of camera
	double m_vfov;

	osg::Matrix m_cameraViewMatrix;
	float m_viewDistance; //used for saving trackball distances
	//osg::Matrix m_cameraHomeMatrix;
	osg::Vec3 m_cameraHomePos;
	osg::Vec3 m_cameraHomeLookAt;
	osg::Vec3 m_cameraHomeUp;
	//float m_homeDistance;

//render offscreen
	
	bool m_bRenderOffscreen;
	//rendered image used when in offscreen render mode
	osg::ref_ptr<osg::Image> _frameBufferImage;



};

typedef osg::ref_ptr<HogBoxViewer> HogBoxViewerPtr;

}; //end hogbox namespace