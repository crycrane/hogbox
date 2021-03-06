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

	//static VisionRegistry* Instance(bool erase = false);

    struct vrEmpty
    {
        VisionRegistry* getIt()
        {
            static osg::ref_ptr<VisionRegistry> it = new VisionRegistry;
            return it.get();
        }
    };
    
    static VisionRegistry* Instance(bool erase = false){
        vrEmpty empty;
        return empty.getIt();
    }    
    
	typedef std::vector< osg::ref_ptr<osgDB::DynamicLibrary> >		DynamicLibraryList;
	typedef std::map< std::string, std::string>						ClassTypeAliasMap;

	//videofiles
	
	//register a new videoFileStream type with the registry
	void AddVideoStreamTypeToRegistry(VideoFileStreamWrapper* protoWrapper);

	//
	//Allocate a VideoFileStream of the first registered type
	VideoFileStreamPtr AllocateVideoFileStream(const std::string& plugin="");

	//
	//Try to allocate, create and return a video file stream using one of the registered types
	VideoFileStreamPtr CreateVideoFileStream(const std::string& fileName, const std::string& plugin = "",
											bool hflip=false, bool vflip=false, bool deinter=false);

	//webcams

	//register a new videoFileStream type with the registry
	void AddWebCamStreamTypeToRegistry(WebCamStreamWrapper* protoWrapper);

	//
	//Allocate a VideoFileStream of the first registered type
	WebCamStreamPtr AllocateWebCamStream(const std::string& plugin="");

	//
	//Try to allocate, create and return a video file stream using one of the registered types
	WebCamStreamPtr CreateWebCamStream(const std::string& fileName, const int& width=640, const int& height=480, const int& fps=30,
									   bool hflip=false, bool vflip=false, bool deinter=false, const std::string& plugin="");



	//
	void AddClassTypeAlias(const std::string mapClassType, const std::string toLibraryName);

	//
	//Loads a specific video plugin, unless no plugin name is given,
	//in which case the first plugin found is loaded
	//return 0 on success -1 on fail
	int LoadVideoFileStreamPlugin(const std::string plugin="");

	//
	//return the path of the index file in the visPlugins folder
	//that matches the video plugin library naming convention
	const std::string FindVideoFileLibraryName(int index);

	//
	//Return a videostream plugin prototype based on it's name
	//if no name is passed the first available plugin is returned
	VideoFileStreamWrapperPtr GetVideoFileStreamPluginProto(const std::string& plugin);
	
	//
	//Loads a specific webcam plugin, unless no plugin name is given,
	//in which case the first plugin found is loaded
	//return 0 on success -1 on fail
	int LoadWebCamStreamPlugin(const std::string plugin="");

	//
	//return the path of the index file in the visPlugins folder
	//that matches the video plugin library naming convention
	const std::string FindWebCamLibraryName(int index);

	//
	//Return a videostream plugin prototype based on it's name
	//if no name is passed the first available plugin is returned
	WebCamStreamWrapperPtr GetWebCamStreamPluginProto(const std::string& plugin);

	//
	//Load a library, which should register an plugin of some sort
	osgDB::Registry::LoadStatus LoadLibrary(const std::string& fileName);

	DynamicLibraryList::iterator GetLibraryItr(const std::string& fileName);

protected:
	//private constructor for singleton
	VisionRegistry(void);
	virtual ~VisionRegistry(void);

	//get the platform specific prepend for videoFileStream plugins i.e. 'hogboxVisionPlugins/hogboxVision_video_'
	const std::string GetVideoFileStreamPluginPrepend();

	const std::string GetWebCamStreamPluginPrepend();

	//get the extension used for plugins (including the . )
	const std::string GetPluginExtension();

	virtual void destruct(){
		_videoFileStreamTypes.clear();
	}

protected:
	
	//Map class types to particular library names
	ClassTypeAliasMap _classTypeAliasMap;

	
	//List of registered videofilestream types
	std::vector<VideoFileStreamWrapperPtr> _videoFileStreamTypes;

	//List of registered webcamstream types
	std::vector<WebCamStreamWrapperPtr> _webcamStreamTypes;

	//List of registered camerabased tracker types
	std::vector<CameraBaseTrackerWrapperPtr> _cameraBaseTrackerTypes;


	//list of loaded libraries
	DynamicLibraryList _dlList;

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
			_wrapper = new VideoFileStreamWrapper(std::string(name), proto);
			
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
//register proxy
//
class WebCamStreamRegistryProxy
{
public:
	WebCamStreamRegistryProxy(WebCamStream* proto, const char* name)
	{
		//check the registry instance
		if(VisionRegistry::Instance())
		{
			//create the wrapper and add to the reg
			proto->setName(name);
			_wrapper = new WebCamStreamWrapper(std::string(name), proto);
			OSG_FATAL << "Register webcam plugin" << std::endl;
			VisionRegistry::Instance()->AddWebCamStreamTypeToRegistry(_wrapper);
					
		}
	}
	
	virtual ~WebCamStreamRegistryProxy(void)
	{
		_wrapper = NULL;
	}
	
protected:
	
	WebCamStreamWrapperPtr _wrapper;
	
};

#define USE_VISION_VIDEO_PLUGIN(ext) \
extern "C" void hogboxvision_video_##ext(void); \
static hogboxDB::PluginFunctionProxy proxy_##ext(hogboxvision_video_##ext);


//
//Register a new videofilestream type plugin 
#define REGISTER_VISION_VIDEO_PLUGIN(ext, classname) \
	extern "C" void hogboxvision_video_##ext(void) {} \
	static hogboxVision::VideoFileStreamRegistryProxy g_proxy_##ext(new classname, #ext );


#define USE_VISION_WEBCA_PLUGIN(ext) \
extern "C" void hogboxvision_webca_##ext(void); \
static hogboxDB::PluginFunctionProxy proxy_##ext(hogboxvision_webca_##ext);

//
//Register a new videofilestream type plugin 
#define REGISTER_VISION_WEBCA_PLUGIN(ext, classname) \
	extern "C" void hogboxvision_webca_##ext(void) {} \
	static hogboxVision::WebCamStreamRegistryProxy g_proxy_##ext(new classname, #ext );

	
	
	

}; //end hogboxVision namespace
	