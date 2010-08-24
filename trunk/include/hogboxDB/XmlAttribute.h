#pragma once

#include <map>
#include <iostream>
#include <osgDB/XmlParser>

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
//m_attributeMap["MyNameAttribute"] = new TypedXmlAttribute<std::string>(&wrapClass.nameStr);
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
		XmlAttribute() : osg::Referenced(){}
		
		inline virtual void operator = (const XmlAttribute&){}
		
		//write into passed names contents
		inline virtual bool serialize(osgDB::XmlNode*) 
		{return false;}
		//read from the passed contents
		inline virtual bool deserialize(osgDB::XmlNode*) 
		{return false;}
	
	protected:
		XmlAttribute(const XmlAttribute& copy) : osg::Referenced(copy){}

		virtual ~XmlAttribute(){}
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
	
		TypedXmlAttribute(T* value = 0) 
			: XmlAttribute(),			
			m_value(value) 
		{
		}
		
		virtual const T& get() const { return *m_value; }
		
		virtual void set(const T& value) {			
			*m_value = value;
		}
		//write to nodes contents
		virtual bool serialize(osgDB::XmlNode* out) {
			//std::stringstream ss;
			//ss << *m_value;
			//out->contents = ss.str();
			//out << *m_value;
			return true;
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
			*m_value = value;		
			return *this;
		}
	
		void operator = (const TypedXmlAttribute<T>& value) {
			*m_value = value.get();		
		}
		
		//the pointer to the member this
		//attribute represents
		T* m_value;
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
								
		CallbackXmlAttribute(C *object, 
			GetHandler ghandler = 0,
			SetHandler shandler = 0) : 
			mp_object(object),
			f_gethandler(ghandler),			
			f_sethandler(shandler)
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
				osg::notify(osg::WARN) << "XML ERROR: No set method provided for XmlAttribute. Objects value will not be set." << std::endl;
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
 