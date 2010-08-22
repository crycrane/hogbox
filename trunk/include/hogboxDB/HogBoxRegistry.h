/*
 *  MSRegistry.h
 *  hogboxIPhone
 *
 *  Created by Thomas Hogarth on 01/10/2009.
 *  Copyright 2009 HogBox. All rights reserved.
 *
 */

#pragma once

#include <hogboxDB/Export.h>
//#include <hogbox/Singleton.h>

#include <hogboxDB/XmlClassManagerWrapper.h>
#include <osgDB/DynamicLibrary>

namespace hogboxDB {

//
//HogBoxRegistry
//Currently tracks all XmlNodeManger types registered for
//reading from the hogbox xml database
//
class HOGBOXDB_EXPORT HogBoxRegistry : public osg::Referenced //, public hogbox::Singleton<HogBoxRegistry>
{
public:

	//friend hogbox::Singleton<HogBoxRegistry>;

    static HogBoxRegistry* Instance(bool erase = false);

	typedef std::vector< osg::ref_ptr<osgDB::DynamicLibrary> >		DynamicLibraryList;
	typedef std::map< std::string, std::string>						ClassTypeAliasMap;

	
	//register a new xml node type manager
	void AddXmlNodeManagerToRegistry(XmlClassManagerWrapper* object);
	
	//returns the wrapper by managerName
	XmlClassManager* GetXmlClassManager(const std::string& managerName);

	XmlClassManager* GetXmlClassManagerForClassType(const std::string& classType);


	void AddClassTypeAlias(const std::string mapClassType, const std::string toLibraryName);

	//
	//create a library name from a classtype name
	std::string CreateXmlLibraryNameForClassType(const std::string& classtype);

	//
	//Load a library, which should register an plugin of some sort
	osgDB::Registry::LoadStatus LoadLibrary(const std::string& fileName);

	DynamicLibraryList::iterator GetLibraryItr(const std::string& fileName);

protected:
	//private constructor for singleton
	HogBoxRegistry(void);
	virtual ~HogBoxRegistry(void);

	virtual void destruct(){
		m_xmlNodeManagers.clear();
	}

protected:
	
	//Map class types to particular library names
	ClassTypeAliasMap m_classTypeAliasMap;

	//the list of xml node managers used to read/write nodes
	//to the database
	std::vector<XmlClassManagerWrapperPtr> m_xmlNodeManagers;

	//list of loaded libraries
	DynamicLibraryList m_dlList;

};

//
//register proxy
//
class XmlNodeManagerRegistryProxy
{
public:
	XmlNodeManagerRegistryProxy(XmlClassManager* proto, const char* name)
	{
		//check the registry instance
		HogBoxRegistry* registry = HogBoxRegistry::Instance();
		if(registry)
		{
			//create the wrapper and add to the reg
			proto->setName(name);
			_wrapper = new XmlClassManagerWrapper(proto);
			
			registry->AddXmlNodeManagerToRegistry(_wrapper);
		}
	}
	
	virtual ~XmlNodeManagerRegistryProxy(void)
	{
		_wrapper = NULL;
	}
	
protected:
	
	osg::ref_ptr<XmlClassManagerWrapper> _wrapper;
	
};

//
//Register a new XmlClassManager type plugin 
//classname is the base classtype the plugin can handle, other types
//plugin is an implementation of XmlClassManager
#define REGISTER_HOGBOXPLUGIN(ext, classname) \
	extern "C" void hogboxdb_##ext(void) {} \
	static hogboxDB::XmlNodeManagerRegistryProxy g_proxy_##ext(new (##classname), #classname );


}; //end hogboxDB namespace
	