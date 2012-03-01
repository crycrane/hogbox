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

#include <hogboxDB/Export.h>
//#include <hogbox/Singleton.h>

#include <hogboxDB/XmlClassManagerWrapper.h>
#include <osgDB/DynamicLibrary>

extern "C"
{
	typedef void (* CPluginFunction) (void);
}

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
    
    struct sEmpty
    {
        HogBoxRegistry* getIt()
        {
            static osg::ref_ptr<HogBoxRegistry> it = new HogBoxRegistry;
            return it.get();
        }
    };

    static HogBoxRegistry* Inst(bool erase = false){
        sEmpty empty;
        return empty.getIt();
    }

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
		_xmlNodeManagers.clear();
	}

protected:
	
	//Map class types to particular library names
	ClassTypeAliasMap _classTypeAliasMap;

	//the list of xml node managers used to read/write nodes
	//to the database
	std::vector<XmlClassManagerWrapperPtr> _xmlNodeManagers;

	//list of loaded libraries
	DynamicLibraryList _dlList;

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
		HogBoxRegistry* registry = HogBoxRegistry::Inst();
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

struct PluginFunctionProxy
{
	PluginFunctionProxy(CPluginFunction function) { (function)(); }
};
	
#define USE_HOGBOXPLUGIN(ext) \
extern "C" void hogboxxml_##ext(void); \
static hogboxDB::PluginFunctionProxy proxy_##ext(hogboxxml_##ext);
	
//
//Register a new XmlClassManager type plugin 
//classname is the base classtype the plugin can handle, other types
//plugin is an implementation of XmlClassManager
#define REGISTER_HOGBOXPLUGIN(ext, classname) \
	extern "C" void hogboxxml_##ext(void) {} \
	static hogboxDB::XmlNodeManagerRegistryProxy g_proxy_##ext(new classname , #ext );


}; //end hogboxDB namespace
	