#pragma once

#include <hogboxDB/XmlAttributePtr.h>

//
//XmlAttribute types for wrapping std::map objects
//Basic xml representation of maps is like a list only the
//contents of the list is a set of mapvalue nodes, these are
//identified by the 'key' property, an example below would wrap a
//map<int, string>
//<MyMap cout='2'>
//	<MapValue key='0'>
//		StringValue
//	</MapValue>
//	<MapValue key='2'>
//		StringValue2
//	</MapValue>
//</MyMap>
//
namespace hogboxDB {

	//
	//Used to wrap a map of basic types to class pointers
	//a map list must start with a head node with a 'count' property followed by count
	//child nodes. Each child is a map node which has a 'key' into the map and contains a 
	//child pointer node e.g.
	//<TextureList count='1'>
	//	<Channel key='0'>
	//		<Texture useID='myTex'>
	//	</Channel>
	//The above example is a map of texture channel ids to textures. It declares a list
	//with 1 item. The item is a map value with 'key' 0 (which in this example represents the 
	//texture channel).
	//The map itself is controled by the mp_object class and values are accessed via
	//the classes get set methods.
	//C is the class containing the map
	//K is the type of the maps key values
	//T is the type that we are maping to
	template <class C, typename K, typename T> class CallbackXmlClassPointerMap: public XmlAttribute  
	{
	public:	
	
		//templates for get set mapped values list funcions
		typedef T* (C::* GetValueByKeyHandler)(const K&);
		typedef void (C::* SetValueByKeyHandler)(const K&, T*);
								
		CallbackXmlClassPointerMap(C *object, 
			GetValueByKeyHandler ghandler = 0,
			SetValueByKeyHandler shandler = 0) : 
			mp_object(object),
			f_getvaluebykeyhandler(ghandler),			
			f_setvaluebykeyhandler(shandler)
		{
		}
		
		virtual T* get(const K& key) const {
			return (mp_object->*f_getvaluebykeyhandler)(key);
		}
		
		virtual void set(const K& key, T* value) 
		{
			if (f_setvaluebykeyhandler) 
			{
				(mp_object->*f_setvaluebykeyhandler)(key, value);			
			} else 
			{
				osg::notify(osg::WARN) << "XML ERROR: No set method provided for CallbackXmlClassPointerMap Attribute. The msp value will not be set." << std::endl;
			}
		}

		//write to nodes contents
		virtual bool serialize(osgDB::XmlNode* out) 
		{
			//std::stringstream ss;
			//ss << this->get();
			//out->contents = ss.str();
			//out << *m_value;
			return true;
		}

		//
		//Pointer maps consist of a head list node followed by 'count' mapvalue
		//nodes, which are identifiyed by the 'key' property
		virtual bool deserialize(osgDB::XmlNode* in) 
		{	
			//
			if(!in){return false;}

			//see if we can get the count property (otherwise it's not a valid list
			unsigned int count = 0;
			if(!hogboxDB::getXmlPropertyValue(in, "count", count))
			{
				osg::notify(osg::WARN) << "XML ERROR: Parsing pointer map node '" << in->name << "'," << std::endl
									   << "                      Map nodes must contain a 'count' property. For example <" << in->name << " count='2>" << std::endl;
				return false;
			}

			//if count is 0 dont bother
			if(count == 0)
			{
				osg::notify(osg::WARN) << "XML WARN: Parsing pointer map node '" << in->name << "'," << std::endl
									  << "                      The map attribute has a count of '0', are you sure this is not a mistake?" << std::endl;

				return false;
			}


			//see if the count matches the number of children
			if(in->children.size() != count)
			{
				//warn user
				osg::notify(osg::WARN) << "XML ERROR: Parsing pointer map node '" << in->name << "'," << std::endl
									   << "                      Nodes 'count' property (" << count << ") does not match the number of child nodes (" << in->children.size() << ")." << std::endl
									   << "                      The list will not be passed." << std::endl;
				return false;
			}


			//now loop over the children and read the map values
			for(unsigned int i=0; i<count; i++)
			{
				//the mapvalue node
				osgDB::XmlNode* mapNode = in->children[i];

				//mapvalue nodes should contain a key property
				K mapKey = 0;
				if(!hogboxDB::getXmlPropertyValue(mapNode, "key", mapKey))
				{
					osg::notify(osg::WARN) << "XML ERROR: Parsing map value node '" << mapNode->name << "'," << std::endl
										   << "                      Map value nodes must contain a 'key' property. For example <" << mapNode->name << " key='1>" << std::endl;
					return false;
				}

				//we have our key into the map, now read the value node of the mapvalue
				if(mapNode->children.size() == 0)
				{
					osg::notify(osg::WARN) << "XML ERROR: Parsing map value node '" << mapNode->name << "'," << std::endl
										   << "                      Map value nodes must contain a child node representing the value" << std::endl;
					return false;
				}

				//read the mapvalue pointer node from the database
				hogbox::ObjectPtr ptr = hogboxDB::HogBoxManager::Instance()->ReadNode(mapNode->children[0]);
				
				//cast it to type and pass to set
				T* typePtr = dynamic_cast<T*>(ptr.get()); 
				if(typePtr){
					
					//call the set function using the mapKey as the key into the map
					set(mapKey, typePtr);

				}else{
					//XML CASTING ERROR
					return false;
				}
			}

			return true;
		}
		
	protected:
		//destructor
		virtual ~CallbackXmlClassPointerMap(){}
	
		//get set function pointers for the pointer list
		GetValueByKeyHandler f_getvaluebykeyhandler;
		SetValueByKeyHandler f_setvaluebykeyhandler;
		
		//pointer to the object containing the
		//pointer list
		C* mp_object;				
	};

 };//end hogboz namespace
 