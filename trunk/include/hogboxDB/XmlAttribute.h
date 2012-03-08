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

#include <map>
#include <iostream>
#include <osgDB/XmlParser>
#include <hogbox/HogBoxBase.h>

#include <hogboxDB/XmlUtils.h>

#include <osg/Referenced>
#include <osg/ref_ptr>

//
//XmlAttribute
//Used by an Xml wrapper to provide convenient access to a classes members.
//Allows mapping of pointers or get-set func pointers to an Attibute name.
//A class being wrapped for xml will provide access to it's members
//via the various flavous of XmlAttribute. The attributes name in a XmlAttributeMap
//reflects it's definition in an xml document i.e. 
//
//class MyClass{
//public:
//    std::string nameStr;
//};
//MyClass wrapClass;
//_attributeMap["MyNameAttribute"] = new TypedXmlAttribute<std::string>(&wrapClass.nameStr);
//
//In xml we would be looking at
//
//<MyClass uniqueID='MyXmlInstance>
//	<MyNameAttribute>VALUE</MyNameAttribute>
//</MyClass>
//
namespace hogboxDB {

	//
	//Base XmlAttribute class provides interface for deserialisation
	//of XmlNodes
	class XmlAttribute : public osg::Referenced {	
	public:
		XmlAttribute(const std::string& name)
            : osg::Referenced(),
            _name(name)
        {
        }
		
		inline virtual void operator = (const XmlAttribute&){}
		
		//write into passed names contents
		inline virtual osgDB::XmlNodePtr serialize()
		{return NULL;}
		//read from the passed contents
		inline virtual bool deserialize(osgDB::XmlNode*)
		{return false;}

		//
		//Release the attribute from the i.e. release any memory and remove
		//attribute pointers from database
		virtual bool releaseAttribute(){
			return true;
		}
	
        //
        const std::string& getName(){return _name;}
        
	protected:
		XmlAttribute(const XmlAttribute& copy) : osg::Referenced(copy){}

		virtual ~XmlAttribute(){}
    protected:
        //name in xml of the attribute/node
        std::string _name;
	};

	//Map an Attribute name to its XmlAttribute
	typedef std::map<std::string, osg::ref_ptr<XmlAttribute> > XmlAttributeMap;
	
	
	//
	//TypedXmlAttribute
	//Used to Xrap public members of a class. A pointer to the wrapped 
	//member is passed to the constructor, which should be the smae type as T.
	//Basic value types are handled in de/serialisation see hogboxDB::getXmlContents
	//for supported types
	template <typename T> class TypedXmlAttribute: public XmlAttribute
	{
	public:
	
		TypedXmlAttribute(const std::string& name, T* value = 0) 
			: XmlAttribute(name),			
			_value(value)
		{
		}
		
		virtual const T& get() const { return *_value; }
		
		virtual void set(const T& value) {			
			*_value = value;
		}
        
        //
		//write a single node with the content as a string version of the type value
		virtual osgDB::XmlNodePtr serialize() {
            OSG_FATAL << "Serialize Attribute" << std::endl;
            //OSG_FATAL << "    Value: " << this->get() << std::endl;
            osgDB::XmlNodePtr attNode = new osgDB::XmlNode();
            attNode->name = this->getName();
            attNode->type = osgDB::XmlNode::NODE;
            hogboxDB::setXmlContents(attNode.get(), this->get());
            
			return attNode;
		}

		virtual bool deserialize(osgDB::XmlNode* in) 
		{
			//
			if(!in){return false;}

			T readValue;
			if(hogboxDB::getXmlContents(in, readValue))
			{set(readValue);return true;}

			return false;
		}
		
		void (TypedXmlAttribute<T>::*f_sethandler)(T&, const T&);	
					
	protected:
				
		TypedXmlAttribute<T>& operator = (const T& value) {
			*_value = value;		
			return *this;
		}
	
		void operator = (const TypedXmlAttribute<T>& value) {
			*_value = value.get();		
		}
		
		//the pointer to the member this
		//attribute represents
		T* _value;
	};
	

	//
	//CallbackXmlAttribute
	//Provides access to a classes protected member via the classes public get set methods
	//the get set methods must follow the GetHandler, SetHandler typedefs provided i.e.
	//const int& GetMyValue() const;
	//void SetMyValue(const int& value);
	//Again de/serialisation is handled mainly by hogboxDB::getXmlContents see that for supported
	//types
	//
	template <class C, typename T> class CallbackXmlAttribute : public TypedXmlAttribute<T> 
	{
	public:	
	
		typedef const T& (C::* GetHandler)() const;  //get value function definition
		typedef void (C::* SetHandler)(const T&); //set value function definition
								
		CallbackXmlAttribute(const std::string& name, C *object, 
                            GetHandler ghandler = 0,
                            SetHandler shandler = 0) 
            : TypedXmlAttribute<T>(name), 
			f_gethandler(ghandler),			
			f_sethandler(shandler),
            mp_object(object)
		{
		}
		
		virtual const T& get() const {
			return (mp_object->*f_gethandler)();
		}
		
		virtual void set(const T& value) 
		{
			if (f_sethandler) 
			{
				(mp_object->*f_sethandler)(value);			
			} else 
			{
				OSG_WARN << "XML ERROR: No set method provided for XmlAttribute. Objects value will not be set." << std::endl;
			}
		}
		
	protected:	

		CallbackXmlAttribute<T,C>& operator = (const T& value) {
			set(value);		
			return *this;
		}
	
		void operator = (const CallbackXmlAttribute<T,C>& value) {
			set(value.get());		
		}
	
		//function pointers to the get/set 
		//methods of mp_object
		GetHandler f_gethandler;
		SetHandler f_sethandler;
		
		//the pointer to the class instance
		//we are calling our get set methods on
		C* mp_object;				
	};


 };//end hogboxDB namespace
 