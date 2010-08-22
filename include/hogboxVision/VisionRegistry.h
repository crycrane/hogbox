#pragma once

#include <hogboxVision/Export.h>
//#include <hogbox/Singleton.h>

#include <osgDB/DynamicLibrary>
#include <osgDB/Registry>

#include <hogboxVision/VisionRegistryWrappers.h>

#include <hogboxVision/VideoFileStream.h>
#include <hogboxVision/WebCamStream.h>

#include <hogboxVision/HogTracker.h>
#include <hogboxVision/CameraBasedTracker.h>

#include <hogboxVision/PlanarTrackedObject.h>


namespace hogboxVision {

//
//VisionRegistry
//Used to register implementations of VideoFileStream, WebCamStream, HogBoxTracker and 
//various tracked object types
//
class HOGBOXVIS_EXPORT VisionRegistry : public osg::Referenced //, public hogbox::Singleton<VisionRegistry>
{
public:

	//friend hogbox::Singleton<VisionRegistry>;

	static VisionRegistry* Instance(bool erase = false);

	typedef std::vector< osg::ref_ptr<osgDB::DynamicLibrary> >		DynamicLibraryList;
	typedef std::map< std::string, std::string>						ClassTypeAliasMap;

	
	//register a new videoFileStream type with the registry
	void AddVideoStreamTypeToRegistry(VideoFileStreamWrapper* protoWrapper);
	
	//
	//Allocate a VideoFileStream of the first registered type
	VideoFileStream* AllocateVideoFileStream();

	//
	//Try to allocate, create and return a video file stream using one of the registered types
	VideoFileStream* CreateVideoFileStream(const std::string& fileName, bool hflip=false, bool vflip=false, bool deinter=false);



	void AddClassTypeAlias(const std::string mapClassType, const std::string toLibraryName);

	//
	//Load all avaliable plugins in the hogboxVisionPlugins folder matching the
	//VideoFileStreamPluginPrepend. Returns the number of plugins loaded
	int LoadVideoFileStreamPlugins();


	//
	//Load a library, which should register an plugin of some sort
	osgDB::Registry::LoadStatus LoadLibrary(const std::string& fileName);

	DynamicLibraryList::iterator GetLibraryItr(const std::string& fileName);

protected:
	//private constructor for singleton
	VisionRegistry(void);
	virtual ~VisionRegistry(void);

	//get the platform specific prepend for videoFileStream plugins i.e. 'hogboxVisionPlugins/hogboxVision_Video_'
	const std::string GetVideoFileStreamPluginPrepend();

	//get the extension used for plugins (including the . )
	const std::string GetPluginExtension();

	virtual void destruct(){
		m_videoFileStreamTypes.clear();
	}

protected:
	
	//Map class types to particular library names
	ClassTypeAliasMap m_classTypeAliasMap;

	
	//List of registered videofilestream types
	std::vector<VideoFileStreamWrapperPtr> m_videoFileStreamTypes;

	//List of registered webcamstream types
	std::vector<WebCamStreamWrapperPtr> m_webcamStreamTypes;

	//List of registered camerabased tracker types
	std::vector<CameraBaseTrackerWrapperPtr> m_cameraBaseTrackerTypes;


	//list of loaded libraries
	DynamicLibraryList m_dlList;

};

//
//register proxy
//
class VideoFileStreamRegistryProxy
{
public:
	VideoFileStreamRegistryProxy(VideoFileStream* proto, const char* name)
	{
		//check the registry instance
		if(VisionRegistry::Instance())
		{
			//create the wrapper and add to the reg
			proto->setName(name);
			_wrapper = new VideoFileStreamWrapper(proto);
			
			VisionRegistry::Instance()->AddVideoStreamTypeToRegistry(_wrapper);
					
		}
	}
	
	virtual ~VideoFileStreamRegistryProxy(void)
	{
		_wrapper = NULL;
	}
	
protected:
	
	VideoFileStreamWrapperPtr _wrapper;
	
};

//
//Register a new XmlClassManager type plugin 
//classname is the base classtype the plugin can handle, other types
//plugin is an implementation of XmlClassManager
#define REGISTER_VISION_VIDEO_PLUGIN(ext, classname) \
	extern "C" void hogboxvision_##ext(void) {} \
	static hogboxVision::VideoFileStreamRegistryProxy g_proxy_##ext(new (##classname), #classname );


}; //end hogboxVision namespace
	