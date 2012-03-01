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

#include <hogboxDB/HogBoxRegistry.h>

#include <hogboxDB/XmlClassWrapper.h>
#include <hogbox/HogBoxBase.h>

namespace hogboxDB {

class Base : public osg::Object
{
public:
	Base(void) : osg::Object(){}

	Base(const Base&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
	{
	}

	META_Box(hogboxDB,Base)

	std::string getName()const {return name;}
	void setName(const std::string& str){name=str;}

protected:
	virtual ~Base(void){
		osg::notify(osg::WARN) << "Delete Base '"<<getName()<<"'"<<std::endl;
	}

	std::string name;
};

typedef osg::ref_ptr<Base> BasePtr;

class Inherited : public Base
{
public:
	Inherited(void) : Base(){}

	Inherited(const Inherited&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
	{}

	META_Box(hogboxDB,Inherited)

	float getExtraData()const {return extraData;}
	void setExtraData(const float& val){extraData=val;}

	std::vector<osg::Vec3> getExtraList()const{return extraList;}
	void setExtraList(const std::vector<osg::Vec3>& list){extraList=list;}

protected:
	virtual ~Inherited(void){
		osg::notify(osg::WARN) << "Delete Inherited '"<<getName()<<"'"<<std::endl;
	}

	float extraData;
	std::vector<osg::Vec3> extraList;
};

typedef osg::ref_ptr<Inherited> InheritedPtr;

class Group : public Base
{
public:
	Group(void) : Base(){}

	Group(const Group&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
	{}

	META_Box(hogboxDB,Group)

	void AddChild(BasePtr child);
	void RemoveChild(BasePtr child);

	Base* GetCurrent(){return _current.get();}
	void SetCurrent(Base* current){_current=current;}

	std::vector<BasePtr> GetChildren()const{return _children;}
	void SetChildren(const std::vector<BasePtr>& list){_children = list;}

protected:

	virtual ~Group(void){
		osg::notify(osg::WARN) << "Delete Group '"<<getName()<<"'"<<std::endl;
	}

	BasePtr _current;
	std::vector<BasePtr> _children;
};

typedef osg::ref_ptr<Group> GroupPtr;


//
//
//
class BaseXmlWrapper : public XmlClassWrapper
{
public:
	BaseXmlWrapper(Base* wrap) : XmlClassWrapper(wrap)
	{
		// create a new field 
		_xmlAttributes["Name"] = new CallbackXmlAttribute<Base,std::string>(wrap,
																&Base::getName,
																&Base::setName);
		// attach a new field to the name "debug"
		//_fields["debug"] = new TypedField<bool>(&_debugmode);
	}
	virtual ~BaseXmlWrapper(void){}


protected:
};

typedef std::vector<osg::Vec3> Vec3Vector;
class InheritedXmlWrapper : public BaseXmlWrapper
{
public:
	InheritedXmlWrapper(Inherited* wrap) : BaseXmlWrapper(wrap)
	{
		// create a new field 
		_xmlAttributes["ExtraData"] = new CallbackXmlAttribute<Inherited,float>(wrap,
																				&Inherited::getExtraData,
																				&Inherited::setExtraData);
		// attach a new field to the name "debug"
		_xmlAttributes["ExtraList"] = new CallbackXmlAttributeList<Inherited,Vec3Vector, osg::Vec3>(wrap,
																								&Inherited::getExtraList,
																								&Inherited::setExtraList);
	}
	virtual ~InheritedXmlWrapper(void){}

protected:

};

typedef std::vector<BasePtr> BasePtrVector;
class GroupXmlWrapper : public BaseXmlWrapper
{
public:
	GroupXmlWrapper(Group* wrap) : BaseXmlWrapper(wrap)
	{
		// create a new field 
		_xmlAttributes["Current"] = new CallbackXmlClassPointer<Group,Base>(wrap,
																			 &Group::GetCurrent,
																			 &Group::SetCurrent);

		// attach a new field to the name "debug"
		_xmlAttributes["Children"] = new CallbackXmlAttributePointerList<Group,BasePtrVector, Base>(wrap,
																									&Group::GetChildren,
																									&Group::SetChildren);
		// attach a new field to the name "debug"
		//_fields["debug"] = new TypedField<bool>(&_debugmode);
	}
	virtual ~GroupXmlWrapper(void){}

protected:
};


//
//Managers osgobject nodes in an xml document
//
class BaseManager : public hogboxDB::XmlClassManager
{
public:

	BaseManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Base", "BaseType");
		SupportsClassType("Inherited", "Inherits from Base");
		SupportsClassType("Group", "Stores a current and a list of pointer to Base objects");
	}
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	BaseManager(const BaseManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxDB, BaseManager)

	//
	//Create an HogBoxObject from xml
	//
	virtual hogbox::ObjectPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		if(!xmlNode){return NULL;}

		hogboxDB::BasePtr object;
		osg::ref_ptr<hogboxDB::BaseXmlWrapper> xmlWrapper;

		//Get the actual classType
		std::string nodeClass = xmlNode->name;

		//all loaded xml nodes should have a uniqueID, which will also 
		//by used as the Objects name
		std::string uniqueID;
		if(!hogboxDB::getXmlPropertyValue(xmlNode, "uniqueID", uniqueID))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype'" << nodeClass << "' should have a uniqueID property." <<std::endl 
									<< "       i.e. <" << nodeClass << " uniqueID='myID'>" << std::endl;
			return NULL;
		}

		//conruct the type of object and xml interface wrapper
		//required by the node class
		if(nodeClass == "Base")
		{
			//allocate the object as base
			//object = new hogboxDB::Base();
			xmlWrapper = new hogboxDB::BaseXmlWrapper(new hogboxDB::Base());
		
		}else if(nodeClass == "Inherited"){
		
			//allocate the object as Inherited
			//object = new hogboxDB::Inherited();
			xmlWrapper = new hogboxDB::InheritedXmlWrapper(new hogboxDB::Inherited());
		
		}else if(nodeClass == "Group"){
		
			//allocate the object as Inherited
			//object = new hogboxDB::Inherited();
			xmlWrapper = new hogboxDB::GroupXmlWrapper(new hogboxDB::Group());
		}

		
		//
		//Now a class of the relevant type has been wrapped in the matching xmlwrapper
		//the node can be deserialised. We pass over the nodes children and see if the
		//nodes name matches any of the xmlXrappers attributes. If one does it is passed
		//to the attribute for deserialising into object
		for(osgDB::XmlNode::Children::iterator itr = xmlNode->children.begin();
			itr != xmlNode->children.end();
			++itr)
		{
			osgDB::XmlNode* cur = itr->get();
			hogboxDB::XmlAttribute* xmlAttribute = xmlWrapper->get(cur->name);
			if(xmlAttribute)
			{
				if(!xmlAttribute->deserialize(cur))
				{
					osg::notify(osg::WARN) << "XML ERROR: While reading uniqueID node '" << uniqueID << "' of type '" << nodeClass << "'" << std::endl
										   <<"           Failed to read value of attribute '" << cur->name << "', it's possible the attribute exists" << std::endl
										   <<"           but the value type is incorect" << std::endl;
				}
			}

		}
		object = xmlWrapper->GetWrapped();
		return object;
	}
};

REGISTER_HOGBOXPLUGIN(Base, BaseManager)

}; //end hogboxDB namespace


