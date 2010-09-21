/*
 *  MSRegistry.cpp
 *  hogboxIPhone
 *
 *  Created by Thomas Hogarth on 01/10/2009.
 *  Copyright 2009 HogBox. All rights reserved.
 *
 */

#include <hogboxVision/VisionRegistry.h>

#include <hogbox/Version.h>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

using namespace hogboxVision;

//BUG@tom Since moving to CMake the Singleton class has been misbehaving. An app seems to
//use a different instance of the registry then the plugins seem to register too.
//Changing to the dreaded global instance below has fixed things but I need to try and correct this
osg::ref_ptr<VisionRegistry> s_hogboxVisRegistryInstance = NULL;

VisionRegistry* VisionRegistry::Instance(bool erase)
{
	if(s_hogboxVisRegistryInstance==NULL)
	{s_hogboxVisRegistryInstance = new VisionRegistry();}		
	if(erase)
	{
		s_hogboxVisRegistryInstance->destruct();
		s_hogboxVisRegistryInstance = 0;
	}
    return s_hogboxVisRegistryInstance.get();
}

//
//private constructor for singleton
//
VisionRegistry::VisionRegistry(void) : osg::Referenced()
{
	//add the aliases for the standard osg and hogbox 
	//types to the multi purpose hogbox xml plugin which 
	//should always be avaliable
	AddClassTypeAlias("node", "hogbox");

}

VisionRegistry::~VisionRegistry(void)
{
	m_videoFileStreamTypes.clear();
	m_webcamStreamTypes.clear();
	m_cameraBaseTrackerTypes.clear();
	m_dlList.clear();
}

//
//register a new object with the singleton
//
void VisionRegistry::AddVideoStreamTypeToRegistry(VideoFileStreamWrapper* protoWrapper)
{
	//already exist by manager class name
	//VideoFileStream* existing = GetXmlClassManager(object->className());
	//if(existing){return;}

	m_videoFileStreamTypes.push_back(protoWrapper);
}


//
//Allocate a VideoFileStream of the first registered type
VideoFileStreamPtr VisionRegistry::AllocateVideoFileStream(const std::string& plugin)
{
	//see if it's already loaded
	VideoFileStreamWrapperPtr protoWrapper = GetVideoFileStreamPluginProto(plugin);
	if(!protoWrapper.get())
	{
		if(!LoadVideoFileStreamPlugin(plugin))
		{
			//no plugins were loaded, inform user and return NULL
			osg::notify(osg::WARN) << "HogBoxVision Video Plugin ERROR: Failed to load any Video Plugins. Video files can not be handled." << std::endl;
			return NULL;
		}
	}

	//check the wrapper contains a valid videofilestream prototype
	if(protoWrapper->GetPrototype())
	{
		//clone the type
		VideoFileStreamPtr type = dynamic_cast<VideoFileStream*>(protoWrapper->GetPrototype()->cloneType());
		if(type.get())
		{return type;}
	}

	return NULL;
}


//
//
VideoFileStreamPtr VisionRegistry::CreateVideoFileStream(const std::string& fileName, const std::string& plugin,
														 bool hflip, bool vflip, bool deinter)
{
	//clone the type then try calling create, if it works return it
	VideoFileStreamPtr type = AllocateVideoFileStream(plugin);
	if(type.get())
	{
		if(type->CreateStream(fileName, hflip, vflip, deinter))
		{
			return type;
		}
	}
	return NULL;
}

//webcams

//
//register a new videoFileStream type with the registry
//
void VisionRegistry::AddWebCamStreamTypeToRegistry(WebCamStreamWrapper* protoWrapper)
{
	//already exist by manager class name
	//VideoFileStream* existing = GetXmlClassManager(object->className());
	//if(existing){return;}
	m_webcamStreamTypes.push_back(protoWrapper);
}

//
//Allocate a VideoFileStream of the first registered type
//
WebCamStreamPtr VisionRegistry::AllocateWebCamStream(const std::string& plugin)
{
	//see if it's already loaded
	WebCamStreamWrapperPtr protoWrapper = GetWebCamStreamPluginProto(plugin);
	if(!protoWrapper.get())
	{
		if(!LoadWebCamStreamPlugin(plugin))
		{
			//no plugins were loaded, inform user and return NULL
			osg::notify(osg::WARN) << "HogBoxVision WebCam Plugin ERROR: Failed to load any WebCam Plugins. Video files can not be handled." << std::endl;
			return NULL;
		}
		protoWrapper = GetWebCamStreamPluginProto(plugin);
		if(!protoWrapper.get()){return NULL;}
	}

	//check the wrapper contains a valid videofilestream prototype
	if(protoWrapper->GetPrototype())
	{
		//clone the type then try calling create, if it works return it
		WebCamStreamPtr type = dynamic_cast<WebCamStream*>(protoWrapper->GetPrototype()->cloneType());
		if(type)
		{return type;}
	}

	return NULL;
}

//
//Try to allocate, create and return a video file stream using one of the registered types
//
WebCamStreamPtr VisionRegistry::CreateWebCamStream(const std::string& fileName, const std::string& plugin,
													bool hflip, bool vflip, bool deinter)
{
	//clone the type then try calling create, if it works return it
	WebCamStreamPtr type = AllocateWebCamStream(plugin);
	if(type.get())
	{
		if(type->CreateStream(fileName, hflip, vflip, deinter))
		{
			return type;
		}
	}
	return NULL;
}


//
//Add an alias for a classtype to the library that will load it
void VisionRegistry::AddClassTypeAlias(const std::string mapClassType, const std::string toLibraryName)
{
    m_classTypeAliasMap[mapClassType] = toLibraryName;
}

//
//Load all avaliable plugins in the hogboxVisionPlugins folder matching the
//VideoFileStreamPluginPrepend. Returns the number of plugins loaded
//
int VisionRegistry::LoadVideoFileStreamPlugin(const std::string plugin)
{
   //open the hogboxVisionPlugins folder and find all files matching the prepend
	std::string visPluginsFolder = std::string("./hogboxVisPlugins-"+std::string(hogboxGetVersion()));
	osgDB::DirectoryContents pluginsFolder = osgDB::getDirectoryContents(visPluginsFolder);

	std::string ext = GetPluginExtension();
	std::string prepend = GetVideoFileStreamPluginPrepend();
	int loadCount = 0;

	std::string libraryName = "";

	//if we have a plugin name try to load it
	if(!plugin.empty()){
		std::string requestedPluginLibrary = visPluginsFolder + "/" + prepend + "_" + plugin + ext;
		libraryName = requestedPluginLibrary;
	}else{
		libraryName = FindVideoFileLibraryName(0);
	}

	//it matches the prepend definition, load it.
	if(!libraryName.empty())
	{		
		osgDB::Registry::LoadStatus result = this->LoadLibrary(libraryName);
		if(result == osgDB::Registry::LOADED)
		{
			osg::notify(osg::INFO) << "HogBoxVision Video Plugin INFO: Plugin '" << libraryName << "', was loaded successfully." << std::endl;
			loadCount++;
			return true;
		}else if(result == osgDB::Registry::PREVIOUSLY_LOADED) {

			osg::notify(osg::WARN) << "HogBoxVision Video Plugin WARN: Plugin '" << libraryName << "' has already been loaded." << std::endl; 
		}else{
			//failed to load the libray
			osg::notify(osg::WARN) << "HogBoxVision Video Plugin WARN: Failed to load Plugin '" << libraryName << "'." << std::endl; 
		}
	}
	return false;
}

//
//return the path of the index file in the visPlugins folder
//that matches the video plugin library naming convention
//
const std::string VisionRegistry::FindVideoFileLibraryName(int index)
{
   //open the hogboxVisionPlugins folder and find all files matching the prepend
	std::string visPluginsFolder = std::string("./hogboxVisPlugins-"+std::string(hogboxGetVersion()));
	osgDB::DirectoryContents pluginsFolder = osgDB::getDirectoryContents(visPluginsFolder);

	std::string ext = GetPluginExtension();
	std::string prepend = GetVideoFileStreamPluginPrepend();
	int foundCount = 0;

	//loop over contents looking for any files with the corrent extension
	for(unsigned int i=0; i<pluginsFolder.size(); i++)
	{
		if(osgDB::getFileExtensionIncludingDot(pluginsFolder[i]) == ext)
		{
			//check if the begining matches the videofilestream prepend
			//stream upto the last _
			size_t found;
			found = pluginsFolder[i].find_last_of("_");
			if(found != std::string::npos)
			{
				std::string strippedName = pluginsFolder[i].substr(0,found);
				std::string prependName = visPluginsFolder+strippedName;
				std::string libraryName = visPluginsFolder+"/"+pluginsFolder[i];
				//compare to prepend
				if(prepend.compare(prependName) == 0)
				{
					//it matches the prepend definition, load it.
					if(!libraryName.empty())
					{		
						//we have a valid plugin name increase the fund count
						foundCount++;
						if(foundCount == index){return libraryName;}
					}
				}
			}
		}
	}
	return "";
}

//
//Return a videostream plugin prototype based on it's name
//if no name is passed the first avaliable plugin is returned
//
VideoFileStreamWrapperPtr VisionRegistry::GetVideoFileStreamPluginProto(const std::string& plugin)
{
	//if no plugin name return the first
	if(plugin.empty())
	{
		if(m_videoFileStreamTypes.size() > 0){return m_videoFileStreamTypes[0];}
		return NULL;
	}
	//try to find requested
	for(unsigned int i=0; i<m_videoFileStreamTypes.size(); i++)
	{
		//compare to plugin base name
		if(m_videoFileStreamTypes[i]->GetPluginName() == plugin)
		{
			return m_videoFileStreamTypes[i];
		}
	}
	return NULL;
}

//
//Loads a specific webcam plugin, unless no plugin name is given,
//in which case the first plugin found is loaded
//return 0 on success -1 on fail
//
int VisionRegistry::LoadWebCamStreamPlugin(const std::string plugin)
{
   //open the hogboxVisionPlugins folder and find all files matching the prepend
	std::string visPluginsFolder = std::string("./hogboxVisPlugins-"+std::string(hogboxGetVersion()));
	osgDB::DirectoryContents pluginsFolder = osgDB::getDirectoryContents(visPluginsFolder);

	std::string ext = GetPluginExtension();
	std::string prepend = GetWebCamStreamPluginPrepend();
	int loadCount = 0;

	std::string libraryName = "";

	//if we have a plugin name try to load it
	if(!plugin.empty()){
		std::string requestedPluginLibrary = prepend + "_" + plugin + ext;
		libraryName = requestedPluginLibrary;
	}else{
		libraryName = FindWebCamLibraryName(0);
	}

	//it matches the prepend definition, load it.
	if(!libraryName.empty())
	{		
		osgDB::Registry::LoadStatus result = this->LoadLibrary(libraryName);
		if(result == osgDB::Registry::LOADED)
		{
			osg::notify(osg::INFO) << "HogBoxVision WebCam Plugin INFO: Plugin '" << libraryName << "', was loaded successfully." << std::endl;
			loadCount++;
			return true;
		}else if(result == osgDB::Registry::PREVIOUSLY_LOADED) {

			osg::notify(osg::WARN) << "HogBoxVision WebCam Plugin WARN: Plugin '" << libraryName << "' has already been loaded." << std::endl; 
		}else{
			//failed to load the libray
			osg::notify(osg::WARN) << "HogBoxVision WebCam Plugin WARN: Failed to load Plugin '" << libraryName << "'." << std::endl; 
		}
	}
	return false;
}

//
//return the path of the index file in the visPlugins folder
//that matches the video plugin library naming convention
//
const std::string VisionRegistry::FindWebCamLibraryName(int index)
{
   //open the hogboxVisionPlugins folder and find all files matching the prepend
	std::string visPluginsFolder = std::string("./hogboxVisPlugins-"+std::string(hogboxGetVersion()));
	osgDB::DirectoryContents pluginsFolder = osgDB::getDirectoryContents(visPluginsFolder);

	std::string ext = GetPluginExtension();
	std::string prepend = GetWebCamStreamPluginPrepend();
	int foundCount = 0;

	//loop over contents looking for any files with the corrent extension
	for(unsigned int i=0; i<pluginsFolder.size(); i++)
	{
		if(osgDB::getFileExtensionIncludingDot(pluginsFolder[i]) == ext)
		{
			//check if the begining matches the videofilestream prepend
			//stream upto the last _
			size_t found;
			found = pluginsFolder[i].find_last_of("_");
			if(found != std::string::npos)
			{
				std::string strippedName = pluginsFolder[i].substr(0,found);
				std::string prependName = visPluginsFolder+strippedName;
				std::string libraryName = visPluginsFolder+"/"+pluginsFolder[i];
				//compare to prepend
				if(prepend.compare(prependName) == 0)
				{
					//it matches the prepend definition, load it.
					if(!libraryName.empty())
					{		
						//we have a valid plugin name increase the fund count
						foundCount++;
						if(foundCount == index){return libraryName;}
					}
				}
			}
		}
	}
	return "";
}

//
//Return a videostream plugin prototype based on it's name
//if no name is passed the first avaliable plugin is returned
//
WebCamStreamWrapperPtr VisionRegistry::GetWebCamStreamPluginProto(const std::string& plugin)
{
	//if no plugin name return the first
	if(plugin.empty())
	{
		if(m_webcamStreamTypes.size() > 0){return m_webcamStreamTypes[0];}
		return NULL;
	}
	//try to find requested
	for(unsigned int i=0; i<m_webcamStreamTypes.size(); i++)
	{
		//compare to plugin base name
		if(m_webcamStreamTypes[i]->GetPluginName() == plugin)
		{
			return m_webcamStreamTypes[i];
		}
	}
	return NULL;
}

//
//get the platform specific prepend for videoFileStream plugins i.e. 'hogboxVisionPlugins/hogboxVision_Video_'
//
const std::string VisionRegistry::GetVideoFileStreamPluginPrepend()
{
#if defined(OSG_JAVA_BUILD)
    static std::string prepend = std::string("hogboxVisionPlugins")+std::string("/java");
#else
    static std::string prepend = std::string("hogboxVisPlugins-"+std::string(hogboxGetVersion()))+std::string("/");
#endif

#if defined(__CYGWIN__)
    return prepend+"cygwin_"+"hogboxvision_video";
#elif defined(__MINGW32__)
    return prepend+"mingw_"+"hogboxvision_video";
#elif defined(WIN32)
    return prepend+"hogboxvision_video";
#elif macintosh
    return prepend+"hogboxvision_video";
#else
    return prepend+"hogboxvision_video";
#endif	
}

//
//get the platform specific prepend for videoFileStream plugins i.e. 'hogboxVisionPlugins/hogboxVision_Video_'
//
const std::string VisionRegistry::GetWebCamStreamPluginPrepend()
{
#if defined(OSG_JAVA_BUILD)
    static std::string prepend = std::string("hogboxVisionPlugins")+std::string("/java");
#else
    static std::string prepend = std::string("hogboxVisPlugins-"+std::string(hogboxGetVersion()))+std::string("/");
#endif

#if defined(__CYGWIN__)
    return prepend+"cygwin_"+"hogboxvision_webcam";
#elif defined(__MINGW32__)
    return prepend+"mingw_"+"hogboxvision_webcam";
#elif defined(WIN32)
    return prepend+"hogboxvision_webcam";
#elif macintosh
    return prepend+"hogboxvision_webcam";
#else
    return prepend+"hogboxvision_webcam";
#endif	
}

//
//get the extension used for plugins (including the . )
//
const std::string VisionRegistry::GetPluginExtension()
{
#if defined(__CYGWIN__)
    return ".dll";
#elif defined(__MINGW32__)
    return ".dll";
#elif defined(WIN32)
    return ".dll";
#elif macintosh
    return "";
#else
    return ADDQUOTES(OSG_PLUGIN_EXTENSION);
#endif
}

VisionRegistry::DynamicLibraryList::iterator VisionRegistry::GetLibraryItr(const std::string& fileName)
{
    DynamicLibraryList::iterator ditr = m_dlList.begin();
    for(;ditr!=m_dlList.end();++ditr)
    {
        if ((*ditr)->getName()==fileName) return ditr;
    }
    return m_dlList.end();
}

//
//Load a library, which should register an plugin of some sort
//
osgDB::Registry::LoadStatus VisionRegistry::LoadLibrary(const std::string& fileName)
{
   // OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);

    DynamicLibraryList::iterator ditr = GetLibraryItr(fileName);
	if (ditr!=m_dlList.end()) return osgDB::Registry::PREVIOUSLY_LOADED;

    //_openingLibrary=true;

	osgDB::DynamicLibrary* dl = osgDB::DynamicLibrary::loadLibrary(fileName);
    
	//_openingLibrary=false;

    if (dl)
    {
        m_dlList.push_back(dl);
        return osgDB::Registry::LOADED;
    }
    return osgDB::Registry::NOT_LOADED;
}