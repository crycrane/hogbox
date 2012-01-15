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

//Not sure why, but can't include XmlClassWrapper so use forward declare
//and include in the cpp (?)
class XmlClassWrapper;
typedef osg::ref_ptr<XmlClassWrapper> XmlClassWrapperPtr;


//
//Baseclass for managing reading and writing of xml nodes 
//in the main database
//An Implementor of XmlClassManager will inform the maindatabase manager
//which nodes/classes they support through implementing GetSupportedClassTypes
//Finally REGISTER_HOGBOXPLUGIN should be used to register the new Manager type
//with the HogBoxRegistry
//
//The managers create an XmlClassWrapper for each object they allocate
//
class HOGBOXDB_EXPORT XmlClassManager : public osg::Object
{
public:

	XmlClassManager();

	XmlClassManager(const XmlClassManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    META_Box(hogboxDB, XmlClassManager);

	typedef std::map<std::string, std::string> ClassTypeDescriptionMap;

	bool AcceptsClassType(const std::string& classType) const;

	//
	//Reads the object in from an xml node pointing to its opening tag
	//if the node has already been loaded based on its uniqueID property
	//it is returned
	hogbox::ObjectPtr GetOrLoadNode(osg::ref_ptr<osgDB::XmlNode> xmlNode);

	//Find the node if it's already been loaded
	hogbox::ObjectPtr GetNodeObjectByID(const std::string& uniqueID);

	//
	//Release the node object if it has already been loaded
	bool ReleaseNodeByID(const std::string& uniqueID);
	
protected:

	virtual ~XmlClassManager(void);

	void SupportsClassType(const std::string& className, const std::string& description);

	//
	//Read this xml node into one of the supported types and add to the m_objectList
	virtual XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode);

protected:

	//Manger type uniquename
	const std::string m_xmlManagerName;

	//list of supported class types
	ClassTypeDescriptionMap m_supportedClassTypes;

	//map the xml node to the object it created
	typedef std::map<XmlNodePtr, XmlClassWrapperPtr> XmlNodeToObjectMap;
	typedef std::pair<XmlNodePtr, XmlClassWrapperPtr> XmlNodeToObjectPair;//hogbox::ObjectPtr

	XmlNodeToObjectMap m_objectList;

};


}; //end hogboxDB namespace