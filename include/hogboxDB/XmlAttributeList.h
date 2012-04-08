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
		XmlAttributeList(const std::string& name)
            : XmlAttribute(name)
        {
        }
		
		//write into passed names contents
		inline virtual osgDB::XmlNodePtr serialize() 
		{
			return NULL;
		}
		//read from the passed contents
		inline virtual bool deserialize(osgDB::XmlNode*) 
		{
			return false;
		}
	
	protected:

		virtual ~XmlAttributeList(){}
	};	

	//
	//Used for lists of basic types, node should contain a count property to
	//indicate it's a list i.e. <MyList count='2'>listValue1 listValue2</MyList>
	//the xml list content is epected to be values seperated by spaces 
	//The template requires two types, L the list type and T the type the
	//list contains. The list type should be an stl container of some sort
	//i.e. std::vector, std::deque etc
	template <typename L, typename T> class TypedXmlAttributeList : public XmlAttribute 
	{
	public:
	
		TypedXmlAttributeList(const std::string& name, L* list = 0) 
			: XmlAttribute(name),			
			_list(list) 
		{
		}
		
		//get the value at a particular index in the list
		virtual T get(unsigned int index) const { 
			//check against list bounds
			if(index >=_list->size()){return T();}
			return _list->at(index); 
		}
		
		//set a value at a particular index in the list
		virtual void set(unsigned int index, const T& value) {	
			//check against list bounds
			if(index >=_list->size())
			{return;}
			_list->at(index) = value;
		}
		//write to nodes contents
		virtual osgDB::XmlNodePtr serialize() {
            
            //
            if(!_list){return NULL;}
            
			//create the head node which will provide the count as a property
            osgDB::XmlNodePtr listNode = new osgDB::XmlNode();
            listNode->name = this->getName();
            listNode->type = osgDB::XmlNode::NODE;
            
            //set the count property
            hogboxDB::setXmlPropertyValue(listNode.get(), "count", (unsigned int)_list->size());
            
            //now loop each item of the list and write it to a stringstream
            std::stringstream listss;
            for(unsigned int i=0; i<_list->size(); i++){
                hogboxDB::typeToStringStream(this->get(i), listss);
                if(i != _list->size()-1){listss << " ";}
            }
            listNode->contents = listss.str();
            return listNode;
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
			_list->assign(count, empty);

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
			*_list = list;		
			return *this;
		}
		//void operator = (const TypedXmlAttributeList<L,T>& list) {
		//	*_list = list.get();		
		//}
		
		//the pointer to the member list
		//should be an stl container of some sort
		L* _list;
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
								
		CallbackXmlAttributeList(const std::string& name, C *object, 
                                 GetListHandler ghandler = 0,
                                 SetListHandler shandler = 0) 
            : TypedXmlAttributeList<L, T>(name),
            f_getlisthandler(ghandler),			
			f_setlisthandler(shandler),
			mp_object(object)
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
		virtual osgDB::XmlNodePtr serialize() {

            if(!mp_object){return NULL;}
            
			//create the head node which will provide the count as a property
            osgDB::XmlNodePtr listNode = new osgDB::XmlNode();
            listNode->name = this->getName();
            listNode->type = osgDB::XmlNode::NODE;

            //get local ref to the objects list 
            L list = this->get();
            
            //set the count property
            hogboxDB::setXmlPropertyValue(listNode.get(), "count", (unsigned int)list.size());
            OSG_FATAL << "Serialize XmlList" << std::endl;
            //now loop each item of the list and write it to a stringstream
            std::stringstream listss;
            for(unsigned int i=0; i<list.size(); i++){
                hogboxDB::typeToStringStream(list.at(i), listss);
                if(i != list.size()-1){listss << " ";}
                OSG_FATAL << "    Serialize list value '" << listss.str() << "'." <<  std::endl;
            }
            listNode->contents = listss.str();
            return listNode;
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
									   << "                      List nodes must contain a 'count' property. For example <" << in->name << " count='2'>" << std::endl;

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

			//set the new list via the set callback
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
 