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

#include <hogboxDB/XmlAttribute.h>
#include <hogboxDB/XmlAttributeList.h>
#include <hogboxDB/XmlAttributeEnum.h>
#include <hogboxDB/XmlAttributePtr.h>
#include <hogboxDB/XmlAttributeMap.h>

#include <hogbox/AssetManager.h>
#include <hogboxDB/XmlUtils.h>


namespace hogboxDB {

//
//XmlClassWrapper
//
//Is esentially a node of the xml database, all objects added to the database are initalised
//and stored within an XmlClassWrapper. The nodes are identified by 'uniqueID' property
//
//Provides an interface for wrapping objects of type osg::Object for use in xml database
//Each classtype being wrapped provides a set of xmlAtrributes which are stored in
//the _xmlAttributes list. These provied the means of de/serialising the classes
//member variables from an xmlnode
//The classtype being wrapped should be of type osg::Object so that the xmlnode name
//can use the osg::Object::className (Usually just base class)
//
//Classnodes in xml files should always provide a 'uniqueID' property so that the object
//it represents can be indetified by other nodes and by the user. A node trying to reference an existing
//classnode should provide a 'useID' property. For example,
//<Base uniqueID='myobj1'>
//...
//</Base>
//Is declaring an instance of type 'Base' with the uniqueID 'myobj1'
//<CurrentObjectPtr>
//	<Base useID='myobj1'>
//</CurrentObjectPtr>
//Is declaring some member attribute named 'CurrentObjectPtr' which holds a 
//pointer to instance 'myobj1 of type 'Base'.
//
//The 'uniqueID' is used by the user to access the instance of an object in the
//main xml database. So be sure the name is 'unique'. The names provided are stored
//by the objects wrapper and in most cases also directly become the objects name. Special cases include
//uniforms, their names have to match the names in a shader file. In this case the Uniforms wrapper will use
//the full name, but its wrapped object will use only the word after the final dot. For example
//<Uniform uniqueID='MyMaterial.diffuseColor'>
//In this example the Uniform would be retrived from the database by its full name 'MyMaterial.diffuseColor'
//But the uniform object itself will be named 'diffuseColor' to match the shader variable it represents.
//The same is true of textures which use the word after the final dot as their sampler name.
//
//Classes that inherit from your base wrapped class should be handled by the same wrapper
//and identified by a 'type' property. So if you inherited from Base and new class named Inherited
//it would be handled by creating an xmlnode like the following
//<Base uniqueID='myInherit1' type='Inherited>
//...
//</Base>
//
class HOGBOXDB_EXPORT XmlClassWrapper : public osg::Referenced
{
public:
	//
	//The wrapper is contructed with an xml node that should resemble
	//the type it wraps. Depending on the implementation the p_wrappedObject
	//is allocated to the correct type. Once this is done deserialize will be ready
	//to read the node into our wrapped object
	//The Wrapper nodes _uniqueID should come from the xmlNodes 'uniqueID' property
	XmlClassWrapper(const std::string& classType);

	//
	//Return base class type for this type of node
	const std::string& getClassType()const;
    
    //
    //Allocate a new object of the wrapped type, this way inherited types can
    //implement a new wrapping inheriting from the base types wrapper and overloading
    //the allocateClassType method
    virtual osg::Object* allocateClassType(){
        OSG_FATAL << "XmlClassWrapper::allocateClassType: WARNING: Need to overide this method to allocate your wrapped class type." << std::endl;
        return NULL;
    }
    
    //
    //clone this type, used so we can register an empty wrapper with the wrapper manager
    //which can be used to allocate the relevent wrapper based on class
    virtual XmlClassWrapper* cloneType(){return new XmlClassWrapper("Base");} 

	//
	//Return the uniqueID of the node
	const std::string& getUniqueID()const;

	//
	//return the wrapped object pointer, 
	osg::Object* getWrappedObject();
    
    //
    //Manually set an existing object to wrap
    void setWrappedObject(osg::Object* object);

	//
	//Read the xmlNode into our wrapped object.
	virtual bool deserialize(osgDB::XmlNode* in);
    
    //
    //Write the wrapped object into an xmlNode
    virtual osgDB::XmlNodePtr serialize();
	
	//
	//Return the attribute with the name provided, the attribute
	//requires casting to the correct type for get set methods
	//or can be deserialised from an xmlnode
	XmlAttribute* get(const std::string& name);

	//
	//Some objects require special case name setting. the default set the objects name
	//to that of the nodes uniqueID. 
	virtual void setObjectNameFromUniqueID(const std::string& name);

	//
	//print the xml available attributes etc of the wrapped class
	void printXmlInterface();

protected:

	virtual ~XmlClassWrapper(void);
    
    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){}
    
    //
    //Return the class name from an xml node, default is the nodes name,
    //but if a type property exists that is used instead
    const std::string getClassNameFromXmlNode(osgDB::XmlNode* xmlNode);

protected:

	//the base classtype name being wrapped
	const std::string _classType;

	//the uniqueID of the node
	std::string _uniqueID;

	//the object the node wrapper represents
	osg::ref_ptr<osg::Object> p_wrappedObject;

	//the map of attributes to xml attribute node names
	XmlAttributeMap _xmlAttributes;
    
    //we also store a map of derived types, to compre against a nodes
    //'type' property to allow us to allocate the correct type for p_wrappedObject
    
};

typedef osg::ref_ptr<XmlClassWrapper> XmlClassWrapperPtr;


 };//end hogboxDB namespace
 