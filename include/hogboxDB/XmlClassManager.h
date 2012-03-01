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

//#include <hogboxDB/XmlClassWrapper.h>

#include <hogbox/HogBoxBase.h>
#include <hogboxDB/Export.h>
#include <hogboxDB/XmlUtils.h>



#include <osgDB/XmlParser>
#include <osgDB/FileNameUtils>

namespace hogboxDB {

typedef osg::ref_ptr<osgDB::XmlNode> XmlNodePtr;

//forward delcare XmlClassWrapper base type
class XmlClassWrapper;
typedef osg::ref_ptr<XmlClassWrapper> XmlClassWrapperPtr;


//
//Baseclass for managing reading and writing of xml nodes 
//in the main database
//An Implementor of XmlClassManager will inform the main database manager
//which nodes/classes they support through implementing GetSupportedClassTypes
//Finally REGISTER_HOGBOXPLUGIN should be used to register the new Manager type
//with the HogBoxRegistry
//
//The managers create an XmlClassWrapper for each object they allocate
//
//XmlClassWrapper types are registered with the Manager so automatic allocation
//of the correct type can be done
//
class HOGBOXDB_EXPORT XmlClassManager : public osg::Object
{
public:

	XmlClassManager();

	XmlClassManager(const XmlClassManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    META_Object(hogboxDB, XmlClassManager);

	typedef std::map<std::string, XmlClassWrapperPtr> ClassTypeWrapperMap;

	bool AcceptsClassType(const std::string& classType) const;
    bool AcceptsClassType(osgDB::XmlNode* xmlNode) const;


	//
	//Reads the object in from an xml node pointing to its opening tag
	//if the node has already been loaded based on its uniqueID property
	//it is returned
	osg::ObjectPtr GetOrLoadNode(osg::ref_ptr<osgDB::XmlNode> xmlNode);

	//Find the node if it's already been loaded
	osg::ObjectPtr GetNodeObjectByID(const std::string& uniqueID);

	//
	//Release the node object if it has already been loaded
	bool ReleaseNodeByID(const std::string& uniqueID);
	
    //
    //register a new class wrapper with the name it supports
    void SupportsClassType(const std::string& className, XmlClassWrapperPtr wrapper);
    
    //
    //Allocate a new xml class wrapper for the passed type, returns null
    //if the class type is not supported by this manager
    XmlClassWrapper* allocateXmlClassWrapperForType(const std::string& className);
    
    //
    //Allocate a new xml class wrapper based on the name and "type" property 
    //of the passed xml node
    XmlClassWrapper* allocateXmlClassWrapperForType(osg::ref_ptr<osgDB::XmlNode> xmlNode);
    
protected:

	virtual ~XmlClassManager(void);

    //
	//Allocates the relevant wrapper type then deserialize the node with it
    //the wrapper is then returned an should contain the correct type
    virtual XmlClassWrapperPtr readObjectFromXmlNode(osgDB::XmlNode* xmlNode);

protected:

	//Manger type uniquename
	const std::string _xmlManagerName;

	//list of supported class types and their wrappers, the wrapper may be fore an inherited type
    //which is handled by the 'type' property of an xml node
	ClassTypeWrapperMap _supportedClassTypes;

	//map the xml node to the object it created
	typedef std::map<XmlNodePtr, XmlClassWrapperPtr> XmlNodeToObjectMap;
	typedef std::pair<XmlNodePtr, XmlClassWrapperPtr> XmlNodeToObjectPair;//hogbox::ObjectPtr

	XmlNodeToObjectMap _objectList;

};


}; //end hogboxDB namespace