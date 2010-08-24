#pragma once

#include <hogboxDB/XmlAttribute.h>

//
//XmlAtrribute types for wrapping lists of basic types
//the list should be an stl container of some sort
//A lists values are contained in the contents
//of the attributelist nxmlode
//Example
//
//<MyList count='2'>
//	12.0
//	23.0
//</MyList>
//
//Notice the 'count' property. All list nodes should have a count 
//property indicating the number of items in the list. List types 
//can be any supported by hogboxDB::stringStreamToType 
//
namespace hogboxDB {

	//base interface for list types
	class XmlAttributeList : public XmlAttribute {		
	public:
		XmlAttributeList(){}
		
		//write into passed names contents
		inline virtual bool serialize(osgDB::XmlNode*) 
		{
			return false;
		}
		//read from the passed contents
		inline virtual bool deserialize(osgDB::XmlNode*) 
		{
			return false;
		}
	
	protected:

		XmlAttributeList(const XmlAttributeList&){}

		virtual ~XmlAttributeList(){}
	};	

	//
	//Used for lists of basic types, node should contain a count property to
	//indicate it's a list i.e. <MyList count='2'>listValue1 listValue2</MyList>
	//the xml list content is epected to be values seperated by spaces 
	//The template requires two types, L the list type and T the type the
	//list contains. The list type should be an stl container of some sort
	//i.e. std::vector, std::deque etc
	template <typename L, typename T> class TypedXmlAttributeList: public XmlAttribute 
	{
	public:
	
		TypedXmlAttributeList(L* list = 0) 
			: XmlAttribute(),			
			m_list(list) 
		{
		}
		
		//get the value at a particular index in the list
		virtual T get(unsigned int index) const { 
			//check against list bounds
			if(index < 0 || index >=m_list->size()){return T();}
			return m_list->at(index); 
		}
		
		//set a value at a particular index in the list
		virtual void set(unsigned int index, const T& value) {	
			//check against list bounds
			if(index < 0 || index >=m_list->size())
			{return;}
			m_list->at(index) = value;
		}
		//write to nodes contents
		virtual bool serialize(osgDB::XmlNode* out) {
			//std::stringstream ss;
			//ss << *m_value;
			//out->contents = ss.str();
			//out << *m_value;
			return true;
		}

		//
		//deserialize
		//in node should contain a 'count' property describing
		//how many space seperated values are contained in the in nodes
		//contents.
		//The contents is deserialised with stringStreamToType which will handle
		//T types with multiple values. i.e. if you have a list of 2 vec3 values
		//<VecList count='2'>
		//	1.0 1.1 1.3
		//	2.0 2.1 2.3
		//</VecList>
		//You have 9 space sperated values but as stringStreamToType has a vec3
		//overload it will handle reading in three values at a time so we
		//only need two reads
		virtual bool deserialize(osgDB::XmlNode* in) 
		{
			//
			if(!in){return false;}

			//see if we can get the count property (otherwise it's not a valid list
			unsigned int count = 0;
			if(!hogboxDB::getXmlPropertyValue(in, "count", count))
			{
				osg::notify(osg::WARN) << "XML ERROR: Parsing node '" << in->name << "'," << std::endl
									   << "                      List nodes must contain a 'count' property. For example <" << in->name << " count='2>" << std::endl;
				return false;
			}

			//if count is 0 dont bother
			if(count <= 0)
			{
				osg::notify(osg::WARN) << "XML WARN: Parsing pointer list node '" << in->name << "'," << std::endl
									  << "                      The list attribute hs a count of '0', are you sure this is not a mistake?" << std::endl;

				return false;
			}

			//allocate the list
			T empty;
			m_list->assign(count, empty);

			//now get the contents, deserialise individual values
			//and push_back onto our list
			std::stringstream listStrStream(in->contents);
			for(unsigned int i=0; i<count; i++)
			{
				T item;
				//pass the listStrStream to have its next word converted
				//to the item type.
				if(hogboxDB::stringStreamToType(listStrStream, item))
				{
					//the word conveted to the type correctly so
					//set the value of i in our list to item
					this->set(i, item);
				}else{
					//error occured, rest of list is invalid
					osg::notify(osg::WARN) << "XML ERROR: Parsing list node '" << in->name << "'," << std::endl
										   << "                      Failed while reading item '" << i << "' from list with size '" << count << "' the rest of the items will be skipped." << std::endl;
					return false;
				}
			}
			return true;
		}
		
		//void (TypedXmlAttribute<T>::*f_sethandler)(T&, const T&);				
	protected:
				
		TypedXmlAttributeList<L,T>& operator = (const L& list) {
			*m_list = list;		
			return *this;
		}
		//void operator = (const TypedXmlAttributeList<L,T>& list) {
		//	*m_list = list.get();		
		//}
		
		//the pointer to the member list
		//should be an stl container of some sort
		L* m_list;
	};

	//
	//Allows use of a Classes get and set methods to get and
	//set a member stl container then deserialise the individual items
	//C type of class containing the stl container
	//L the type of stl container
	//T the type contained in the stl container
	template <class C, typename L, typename T> class CallbackXmlAttributeList : public TypedXmlAttributeList<L,T> 
	{
	public:	
	
		typedef const L& (C::* GetListHandler)() const;
		typedef void (C::* SetListHandler)(const L&);
								
		CallbackXmlAttributeList(C *object, 
			GetListHandler ghandler = 0,
			SetListHandler shandler = 0) : 
			mp_object(object),
			f_getlisthandler(ghandler),			
			f_setlisthandler(shandler)
		{
		}
		
		virtual const L& get() const {
			return (mp_object->*f_getlisthandler)();
		}
		
		virtual void set(const L& list) 
		{
			if (f_setlisthandler) 
			{
				(mp_object->*f_setlisthandler)(list);			
			} else 
			{
				osg::notify(osg::WARN) << "XML ERROR: Node set method provided for AttributeList. List will not be set." << std::endl;
			}
		}

		//write to nodes contents
		virtual bool serialize(osgDB::XmlNode* out) {

			//
			//std::stringstream ss;
			//ss << this->get();
			//out->contents = ss.str();
			//out << *m_value;
			return true;
		}

		//
		//the deserialize is the same as TypedXmlAttributeList only each item
		//is added to a local list of type L then the list is passed to the
		//set method
		virtual bool deserialize(osgDB::XmlNode* in) 
		{
			//
			if(!in){return false;}

			//see if we can get the count property (otherwise it's not a valid list
			unsigned int count = 0;
			if(!hogboxDB::getXmlPropertyValue(in, "count", count))
			{
				osg::notify(osg::WARN) << "XML ERROR: Parsing list attribute '" << in->name << "'," << std::endl
									   << "                      List nodes must contain a 'count' property. For example <" << in->name << " count='2>" << std::endl;

				return false;
			}

			//if count is 0 dont bother
			if(count == 0)
			{
				osg::notify(osg::WARN) << "XML WARN: Parsing pointer list node '" << in->name << "'," << std::endl
									  << "                      The list attribute has a count of '0', are you sure this is not a mistake?" << std::endl;

				return false;
			}

			//allocate the list space
			//create a new list to fill with the node data
			L list;
			T empty;
			list.assign(count, empty);

			//now get the contents, deserialise individual values
			//and push_back onto our list
			std::stringstream listStrStream(in->contents);
			for(unsigned int i=0; i<count; i++)
			{
				T item;
				if(hogboxDB::stringStreamToType(listStrStream, item))
				{
					list.at(i) = item;
				}else{
					osg::notify(osg::WARN) << "XML ERROR: Parsing list node '" << in->name << "'," << std::endl
										   << "                      Failed while reading item '" << i << "' from list with size '" << count << "' the rest of the items will be skipped." << std::endl;

					return false;
				}
			}

			//set the new list via the set callbaxk
			this->set(list);
			return true;
		}
		
	protected:	
	
		//function pointers to mp_objects get set methods
		//for the list
		GetListHandler f_getlisthandler;
		SetListHandler f_setlisthandler;
		
		C* mp_object;				
	};

 };//end hogboz namespace
 