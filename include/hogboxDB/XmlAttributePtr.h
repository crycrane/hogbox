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

#include <hogboxDB/XmlAttributeList.h>
#include <hogboxDB/HogBoxManager.h>

//
//XmlAttribute types that are pointers to other xml class nodes
//Example
//
//<MyClass uniqueID='myobject1'>
//  <Description>It's myobject1, declared outside the list</Description>
//</MyClass>
//<MyGroup uniqueID='mygroup1'>
//	<Children count='2'>
//		<MyClass useID='myobject1'/>
//		<MyClass uniqueID='myobject2'>
//			<Description>It's myobject2, declared in the list</Description>
//		</MyClass>
//	</Children>
//
//In the above example the MyGroup class instance has an attribute named
//'Children', which contains a list of pointers to other xml classes.
//As you can see in the above example the pointer class nodes can either
//be references to existing nodes via the use of 'useID' or created in the list
//as we do with 'myobject2' (See XmlClassWrapper for more on 'useID' and uniqueID usage)
//
namespace hogboxDB {

	//
	//used for attributes that are pointers to other classes
	//C type of class containing the pointer
	//T tpye of the pointer in the class
	//Uses get set function pointers to access the pointer. The pointer
	//should be of type osg::referenced so allow for auto memory managment
	template <class C, typename T> 
	class CallbackXmlClassPointer : public XmlAttribute 
	{
	public:	
	
		//templates for get and set pointer functions
		typedef T* (C::* GetPtrHandler)();
		typedef void (C::* SetPtrHandler)(T*);
		

		//
		//Contructor
		//object = class containing the pointer and get set methods
		//ghandler = pointer to the classes get funtion
		//shandler = pointer to the classes set function
		CallbackXmlClassPointer(const std::string& name, C *object, 
								GetPtrHandler ghandler = 0,
								SetPtrHandler shandler = 0) 
			: XmlAttribute(name),
            f_getptrhandler(ghandler),			
			f_setptrhandler(shandler),
            mp_object(object)
		{
		}
		
		virtual T* get() {
			return (mp_object->*f_getptrhandler)();
		}
		
		virtual void set(T* value) 
		{
			if (f_setptrhandler) 
			{
				(mp_object->*f_setptrhandler)(value);			
			} else 
			{
				osg::notify(osg::WARN) << "XML ERROR: No set method provided for XmlClassPointer Attribute. The pointer will not be set." << std::endl;
			}
		}

		//
		//release the pointers object from the database
		virtual bool releaseAttribute(){
			//get the name of the object pointed to
			T* pointerObject = this->get();
			if(pointerObject){
				std::string pointerObjectName = pointerObject->getName();
				return hogboxDB::HogBoxManager::Inst()->ReleaseNodeByID(pointerObjectName);
			}
			return false;
		}

		//write to nodes contents
		virtual osgDB::XmlNodePtr serialize() {
			//create the head node which just wraps the xmlclass node
            osgDB::XmlNodePtr attNode = new osgDB::XmlNode();
            attNode->name = this->getName();
            attNode->type = osgDB::XmlNode::GROUP;
            
            osgDB::XmlNodePtr classNode = hogboxDB::HogBoxManager::Inst()->WriteXmlNode(this->get());
            if(classNode.get()){
                attNode->children.push_back(classNode.get());
                return attNode;
            }
            return NULL;
		}

		//
		//pointer nodes contain a child that is a pointer to another class type
		//the hogbox ReadNode function is used to read the child node and return 
		//its pointer which is then passed to set
		virtual bool deserialize(osgDB::XmlNode* in) 
		{
			//
			if(!in){return false;}

			//check we have some children
			if(in->children.size()==0)
			{
				OSG_WARN << "XML WARN: Parsing pointer attribute '" << in->name << "'," << std::endl
									   << "                       Pointer attributes should contian a child node of the desired classtype. CLASSTYPE" << std::endl
									   << "                       The child node can be a full definition of the classtype required or a useID reference. For example," << std::endl
									   << "                       <" << in->name << ">" << std::endl
									   << "						      <CLASSTYPE uniqueID='object1'>" << std::endl
									   << "						          ..." << std::endl
									   << "						      </CLASSTYPE>" << std::endl
									   << "                       </" << in->name << ">" << std::endl
									   << "                       Or" << std::endl
									   << "                       <" << in->name << ">" << std::endl
									   << "						      <CLASSTYPE useID='object1'>" << std::endl
									   << "                       </" << in->name << ">" << std::endl;

				return false;
			}

			//get the first (and should be the only) child
			osgDB::XmlNode* pointerNode = in->children[0];

			//try and read the pointer node from the database
			osg::ObjectPtr ptr = hogboxDB::HogBoxManager::Inst()->ReadNode(pointerNode);
			
			//check result
			if(!ptr)
			{
				//pointer was not found or failed to be read
				OSG_WARN << "XML ERROR: Parsing pointer attribute '" << in->name << "'," << std::endl
									   << "                        ReadNode did not return a valid object for child node '" << pointerNode->name << "'." << std::endl
									   << "                        The attribute pointer will not be set." << std::endl;
				return false;
			}

			//cast the result to type T
			T* typePtr = dynamic_cast<T*>(ptr.get()); 
			if(!typePtr)
			{
				//error casting to type
				OSG_WARN << "XML ERROR: Parsing pointer attribute '" << in->name << "'," << std::endl
									   << "                        Failed to cast the object returned from ReadNode to the corrent type." << std::endl
									   << "                        Please ensure '" << in->name << "' is expecting a classnode of type '" << pointerNode->name << "'." << std::endl;
				return false;
			}
			//pass the typed pointer to set
			this->set(typePtr);
			return true;
		}
		
	protected:	
	
		//function pointers to the get and set 
		//methods of mp_object
		GetPtrHandler f_getptrhandler;
		SetPtrHandler f_setptrhandler;
		
		C* mp_object;				
	};

	//
	//Allows use of a Classes get and set methods to get and set an stl 
	//container of osg::referenced pointers then de/serialise the individual items
	//C type of class containing the stl container
	//L the type of stl container
	//T the type of pointer contained in the stl container (should inherit from osg::referenced)
	template <class C, typename L, typename T> class CallbackXmlClassPointerList : public XmlAttribute  
	{
	public:	
	
		//templates for get set pinter list funcions
		typedef L (C::* GetPtrListHandler)() const;
		typedef void (C::* SetPtrListHandler)(const L&);
		
		//typedef L::iterator ListIter;
								
		CallbackXmlClassPointerList(const std::string& name, C *object, 
                                    GetPtrListHandler ghandler = 0,
                                    SetPtrListHandler shandler = 0) 
            : XmlAttribute(name),
            f_getptrlisthandler(ghandler),			
			f_setptrlisthandler(shandler),
            mp_object(object)
		{
		}
		
		virtual L get() const {
			return (mp_object->*f_getptrlisthandler)();
		}
		
		virtual void set(const L& list) 
		{
			if (f_setptrlisthandler) 
			{
				(mp_object->*f_setptrlisthandler)(list);			
			} else 
			{
				OSG_WARN << "XML ERROR: No set method provided for XmlClassPointerList Attribute. The pointer list will not be set." << std::endl;
			}
		}

		//
		//release the pointers object from the database
		virtual bool releaseAttribute(){
			bool result = true;
			//get the name of the object pointed to
			L pointerObjectList = this->get();
			if(pointerObjectList.size() > 0){
				typename L::iterator itr = pointerObjectList.begin();
				for( ; itr != pointerObjectList.end(); itr++){
					T* listObject = (*itr);
					std::string listObjectName = listObject->getName();
					if(!hogboxDB::HogBoxManager::Inst()->ReleaseNodeByID(listObjectName)){
						result=false;
					}
				}
			}
			return result;
		}

		//write to nodes contents
		virtual osgDB::XmlNodePtr serialize() 
		{
            if(!mp_object){return NULL;}
            
			//create the head node which will provide the count as a property
            osgDB::XmlNodePtr listNode = new osgDB::XmlNode();
            listNode->name = this->getName();
            listNode->type = osgDB::XmlNode::GROUP;
            
            //get local ref to the objects list 
            L list = this->get();
            
            //set the count property
            hogboxDB::setXmlPropertyValue(listNode.get(), "count", (unsigned int)list.size());
            
            //now loop each item of serialize to an xmlnode and add as child
            for(int i=0; i<list.size(); i++){
                osgDB::XmlNodePtr classNode = hogboxDB::HogBoxManager::Inst()->WriteXmlNode(list.at(i).get());
                if(classNode.get()){
                    listNode->children.push_back(classNode.get());
                }
            }
            return listNode;
		}

		//
		//pointer lists contain a count property as do all lists
		//Each child node should represent a class node of type T
		virtual bool deserialize(osgDB::XmlNode* in) 
		{	
			//
			if(!in){return false;}

			//see if we can get the count property (otherwise it's not a valid list
			unsigned int count = 0;
			if(!hogboxDB::getXmlPropertyValue(in, "count", count))
			{
				OSG_WARN << "XML ERROR: Parsing pointer list node '" << in->name << "'," << std::endl
									   << "                      List nodes must contain a 'count' property. For example <" << in->name << " count='2'>" << std::endl;
				return false;
			}

			//if count is 0 dont bother
			if(count == 0)
			{
				OSG_WARN << "XML WARN: Parsing pointer list node '" << in->name << "'," << std::endl
									  << "                      The list attribute has a count of '0', are you sure this is not a mistake?" << std::endl;

				return false;
			}


			//see if the count matches the number of children
			if(in->children.size() != count)
			{
				//warn user
				OSG_WARN << "XML ERROR: Parsing pointer list attribute '" << in->name << "'," << std::endl
									   << "                      Nodes 'count' property (" << count << ") does not match the number of child nodes (" << in->children.size() << ")." << std::endl
									   << "                      The list will not be parsed." << std::endl;
                //for(unsigned int i=0;i<in->children.size(); i++){
                //    OSG_FATAL << "CHILD '" << i << "' name: '" << in->children[i]->name << "'." << std::endl;
                //}
				return false;
			}

			//allocate the list space
			//create a new list to fill with the node data
			L list;
			list.assign(count, NULL);


			//now get the contents, deserialise individual values
			//and push_back onto our list
			std::stringstream listStr(in->contents);
			for(unsigned int i=0; i<count; i++)
			{

				osgDB::XmlNode* pointerNode = in->children[i];

				//read the pointer node from the database
				osg::ObjectPtr ptr = hogboxDB::HogBoxManager::Inst()->ReadNode(pointerNode);
				//cast it to type and pass to set
				T* typePtr = dynamic_cast<T*>(ptr.get()); 
				if(typePtr){
					list.at(i) = typePtr;
				}else{
					//XML CASTING ERROR
					return false;
				}
			}

			//set the new list via the set callbaxk
			this->set(list);
			return true;
		}
		
	protected:
		//destructor
		virtual ~CallbackXmlClassPointerList(){}
	
		//get set function pointers for the pointer list
		GetPtrListHandler f_getptrlisthandler;
		SetPtrListHandler f_setptrlisthandler;
		
		//pointer to the object containing the
		//pointer list
		C* mp_object;				
	};

 };//end hogboz namespace
 