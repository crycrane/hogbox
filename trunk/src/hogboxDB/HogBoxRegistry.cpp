/*
 *  MSRegistry.cpp
 *  hogboxIPhone
 *
 *  Created by Thomas Hogarth on 01/10/2009.
 *  Copyright 2009 HogBox. All rights reserved.
 *
 */

#include <hogboxDB/HogBoxRegistry.h>
#include <hogbox/Version.h>

using namespace hogboxDB;

//BUG@tom Since moving to CMake the Singleton class has been misbehaving. An app seems to
//use a different instance of the registry then the plugins seem to register too.
//Changing to the dreaded global instance below has fixed things but I need to try and correct this
osg::ref_ptr<HogBoxRegistry> s_hogboxRegistryInstance = NULL;

HogBoxRegistry* HogBoxRegistry::Instance(bool erase)
{
	if(s_hogboxRegistryInstance==NULL)
	{s_hogboxRegistryInstance = new HogBoxRegistry();}		
	if(erase)
	{
		s_hogboxRegistryInstance->destruct();
		s_hogboxRegistryInstance = 0;
	}
    return s_hogboxRegistryInstance.get();
}

//
//private constructor for singleton
//
HogBoxRegistry::HogBoxRegistry(void) : osg::Referenced()
{
	//add the aliases for the standard osg and hogbox 
	//types to the multi purpose hogbox xml plugin which 
	//should always be avaliable
	AddClassTypeAlias("node", "hogbox");
	AddClassTypeAlias("image", "hogbox");
	AddClassTypeAlias("texture", "hogbox");
	AddClassTypeAlias("shader", "hogbox");
	AddClassTypeAlias("program", "hogbox");
	AddClassTypeAlias("uniform", "hogbox");
	AddClassTypeAlias("hogboxobject", "hogbox");
	AddClassTypeAlias("hogboxmaterial", "hogbox");
	AddClassTypeAlias("hogboxlight", "hogbox");
	AddClassTypeAlias("featurelevel", "hogbox");
	AddClassTypeAlias("hogboxviewer", "hogbox");

	//temp, hogboxvision aliasas should be added by itself
	AddClassTypeAlias("videostream", "vision");
}

HogBoxRegistry::~HogBoxRegistry(void)
{
	m_xmlNodeManagers.clear();
	m_dlList.clear();
}

//
//register a new object with the singleton
//
void HogBoxRegistry::AddXmlNodeManagerToRegistry(XmlClassManagerWrapper* object)
{
	//already exist by manager class name
	XmlClassManager* existing = GetXmlClassManager(object->className());
	if(existing){return;}

	m_xmlNodeManagers.push_back(object);
}

//
//return any wrapper using name, if node exists return NULL
//
XmlClassManager* HogBoxRegistry::GetXmlClassManager(const std::string& managerName)
{
	for(unsigned int i=0; i<m_xmlNodeManagers.size(); i++)
	{
		if(m_xmlNodeManagers[i]->className() == managerName)
		{return m_xmlNodeManagers[i]->GetPrototype();}
	}
	return NULL;
}

XmlClassManager* HogBoxRegistry::GetXmlClassManagerForClassType(const std::string& classType)
{
	//check already loaded managers
	for(unsigned int i=0; i<m_xmlNodeManagers.size(); i++)
	{
		//check the wrapper contains a valid manager prototype
		if(m_xmlNodeManagers[i]->GetPrototype())
		{
			//does the classmanager accept the class type 
			if(m_xmlNodeManagers[i]->GetPrototype()->AcceptsClassType(classType))
			{return m_xmlNodeManagers[i]->GetPrototype();}
		}
	}

	//if none of the loaded xml class manager can read the classType
	//try and load a library that can.
	std::string libraryName = CreateXmlLibraryNameForClassType(classType);
	//try to load the lib
	if(!libraryName.empty())
	{		
		osgDB::Registry::LoadStatus result = this->LoadLibrary(libraryName);
		
		if(result == osgDB::Registry::LOADED)
		{
			osg::notify(osg::INFO) << "XML Plugin INFO: Plugin '" << libraryName << "', was loaded successfully." << std::endl;
			//now a library claiming to handle the type has been loaded, try to read again
			return GetXmlClassManagerForClassType(classType);

		}else if(result == osgDB::Registry::PREVIOUSLY_LOADED) {

			osg::notify(osg::WARN) << "XML Plugin WARN: Library '" << libraryName << "' has already been loaded, but isn't accepting ClassType '" << classType << "'." << std::endl; 
			return NULL;
		}else{
			//failed to load the libray
			osg::notify(osg::WARN) << "XML Plugin WARN: Failed to load Library '" << libraryName << "' ClassType '" << classType << "' will not be handled." << std::endl; 
			return NULL;
		}
	}	
	return NULL;
}

//
//Add an alias for a classtype to the library that will load it
void HogBoxRegistry::AddClassTypeAlias(const std::string mapClassType, const std::string toLibraryName)
{
    m_classTypeAliasMap[mapClassType] = toLibraryName;
}

//
//Create a library name from a classtype name, including extension
//and folder hogboxXmlPlugins/
//
std::string HogBoxRegistry::CreateXmlLibraryNameForClassType(const std::string& classtype)
{
    std::string lowercase_classtype;
    for(std::string::const_iterator sitr=classtype.begin();
        sitr!=classtype.end();
        ++sitr)
    {
        lowercase_classtype.push_back(tolower(*sitr));
    }

	//see if the requested classtype is mapped to another class name
    ClassTypeAliasMap::iterator itr=m_classTypeAliasMap.find(lowercase_classtype);
    if (itr!=m_classTypeAliasMap.end() && classtype != itr->second) return CreateXmlLibraryNameForClassType(itr->second);

#if defined(OSG_JAVA_BUILD)
    static std::string prepend = std::string("hogboxPlugins-")+std::string(hogboxGetVersion())+std::string("/java");
#else
    static std::string prepend = std::string("hogboxPlugins-")+std::string(hogboxGetVersion())+std::string("/");
#endif

#if defined(__CYGWIN__)
    return prepend+"cygwin_"+"hogboxxml_"+lowercase_classtype+".dll";
#elif defined(__MINGW32__)
    return prepend+"mingw_"+"hogboxxml_"+lowercase_classtype+".dll";
#elif defined(WIN32)
    return prepend+"hogboxxml_"+lowercase_classtype+".dll";
#elif macintosh
    return prepend+"hogboxxml_"+lowercase_classtype;
#else
    return prepend+"hogboxxml_"+lowercase_classtype+".so";//ADDQUOTES(OSG_PLUGIN_EXTENSION);
#endif

}

HogBoxRegistry::DynamicLibraryList::iterator HogBoxRegistry::GetLibraryItr(const std::string& fileName)
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
osgDB::Registry::LoadStatus HogBoxRegistry::LoadLibrary(const std::string& fileName)
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