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
VideoFileStream* VisionRegistry::AllocateVideoFileStream()
{
	//if the sixe of m_videoFileStreamTypes is zero, try to load all video plugins
	if(m_videoFileStreamTypes.size() == 0)
	{
		if(LoadVideoFileStreamPlugins() == 0)
		{
			//no plugins were loaded, inform user and return NULL
			osg::notify(osg::WARN) << "HogBoxVision Video Plugin ERROR: Failed to load any Video Plugins. Video files can not be handled." << std::endl;
			return NULL;
		}
	}
	//try to use one of the already registered types
	for(unsigned int i=0; i<m_videoFileStreamTypes.size(); i++)
	{
		//check the wrapper contains a valid videofilestream prototype
		if(m_videoFileStreamTypes[i]->GetPrototype())
		{
			//clone the type then try calling create, if it works return it
			VideoFileStream* type = dynamic_cast<VideoFileStream*>(m_videoFileStreamTypes[i]->GetPrototype()->cloneType());
			if(type)
			{return type;}
		}
	}
	return NULL;
}


//
//
VideoFileStream* VisionRegistry::CreateVideoFileStream(const std::string& fileName, bool hflip, bool vflip, bool deinter)
{
	//if the sixe of m_videoFileStreamTypes is zero, try to load all video plugins
	if(m_videoFileStreamTypes.size() == 0)
	{
		if(LoadVideoFileStreamPlugins() == 0)
		{
			//no plugins were loaded, inform user and return NULL
			osg::notify(osg::WARN) << "HogBoxVision Video Plugin ERROR: Failed to load any Video Plugins. Video files can not be handled." << std::endl;
			return NULL;
		}
	}

	//try to use one of the already registered types
	for(unsigned int i=0; i<m_videoFileStreamTypes.size(); i++)
	{
		//check the wrapper contains a valid videofilestream prototype
		if(m_videoFileStreamTypes[i]->GetPrototype())
		{
			//clone the type then try calling create, if it works return it
			VideoFileStream* type = dynamic_cast<VideoFileStream*>(m_videoFileStreamTypes[i]->GetPrototype()->cloneType());
			if(type)
			{
				if(type->CreateStream(fileName, hflip, vflip, deinter))
				{
					return type;
				}
				type = NULL;
			}
		}
	}

	osg::notify(osg::WARN) << "HogBoxVision Video Plugin ERROR: No avaliable Video Plugin was able to load file '" << fileName << "." << std::endl;
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
int VisionRegistry::LoadVideoFileStreamPlugins()
{
   //open the hogboxVisionPlugins folder and find all files matching the prepend
	std::string visPluginsFolder = std::string("./hogboxVisPlugins-"+std::string(hogboxGetVersion()));
	osgDB::DirectoryContents pluginsFolder = osgDB::getDirectoryContents(visPluginsFolder);

	std::string ext = GetPluginExtension();
	std::string prepend = GetVideoFileStreamPluginPrepend();
	int loadCount = 0;

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
				std::string libraryName = visPluginsFolder+pluginsFolder[i];
				//compare to prepend
				if(prepend.compare(prependName) == 0)
				{
					//it matches the prepend definition, load it.
					if(!libraryName.empty())
					{		
						osgDB::Registry::LoadStatus result = this->LoadLibrary(libraryName);
						
						if(result == osgDB::Registry::LOADED)
						{
							osg::notify(osg::INFO) << "HogBoxVision Video Plugin INFO: Plugin '" << libraryName << "', was loaded successfully." << std::endl;
							loadCount++;
						}else if(result == osgDB::Registry::PREVIOUSLY_LOADED) {

							osg::notify(osg::WARN) << "HogBoxVision Video Plugin WARN: Plugin '" << libraryName << "' has already been loaded." << std::endl; 
						}else{
							//failed to load the libray
							osg::notify(osg::WARN) << "HogBoxVision Video Plugin WARN: Failed to load Plugin '" << libraryName << "'." << std::endl; 
						}
					}
				}
			}
		}
	}

	return loadCount;
}

//
//get the platform specific prepend for videoFileStream plugins i.e. 'hogboxVisionPlugins/hogboxVision_Video_'
//
const std::string VisionRegistry::GetVideoFileStreamPluginPrepend()
{
#if defined(OSG_JAVA_BUILD)
    static std::string prepend = std::string("hogboxVisionPlugins")+std::string("/java");
#else
    static std::string prepend = std::string("hogboxVisionPlugins")+std::string("/");
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