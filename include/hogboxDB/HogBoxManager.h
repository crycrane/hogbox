#pragma once
#include <hogbox/Export.h>
//#include <hogbox/Singleton.h>
#include <hogbox/HogBoxBase.h>

#include <hogboxDB/XmlUtils.h>
#include <hogboxDB/HogBoxRegistry.h>

#include <osgDB/XmlParser>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

namespace hogboxDB {

//
//HogBoxManager
//Used to load an Xml Database then read write nodes from it
//using any avaliable xmlManagers in the hogboxRegistry
//
class HOGBOXDB_EXPORT HogBoxManager : public osg::Referenced //, public hogbox::Singleton<HogBoxManager>
{
public:

	//friend hogbox::Singleton<HogBoxManager>;

    static HogBoxManager* Instance(bool erase = false);

	//
	//Reads in a new HogBoxDataBase xml file, files root
	//node should be of type HogBoxDataBase. If no other database
	//has been loaded this files rootnode is used as the database.
	//If a database already exists then this files rootnode children
	//are added to the databases list of children
	bool ReadDataBaseFile(const std::string& fileName);


	//
	//Reads an xml node into a hogbox::ObjectPtr object using
	//the xml nodes uniqueID property to find it in the database.
	//The nodes name/head is used as a classType to find an approprite
	//XmlClassManager to handle construction of the object.
	hogbox::ObjectPtr ReadNodeByID(const std::string& uniqueID);

	//
	//Use the nodes name as a classtype, then query the hogbox registry
	//for an XmlClassManager capable of reading the node. If an XmlClassManager
	//is found that states it can read the classtype then the node is passed to
	//the XmlNodeManagers GetOrLoadNode function returning the result
	//If n oXmlNodeManager is found then NULL is returned
	hogbox::ObjectPtr ReadNode(osgDB::XmlNode* inNode);

	//
	//Same as call to ReadNodeByID, but also performs
	//a dynamic_cast of the loaded object to ref_ptr
	//of the Template type T
	template <typename T>
	osg::ref_ptr<T> ReadNodeByIDTyped(const std::string& uniqueID)
	{
		//see if we can find a node with the uniqueID requested
		osgDB::XmlNode* uniqueIDNode = FindNodeByUniqueIDProperty(uniqueID);
		//if we find the node pass it to readnode and return the result
		if(uniqueIDNode)
		{return ReadNodeTyped<T>(uniqueIDNode);}
		return NULL;
	}


	//
	//Same as call to ReadNode, but also performs
	//a dynamic_cast of the loaded object to ref_ptr
	//of the Template type T
	template <typename T>
	osg::ref_ptr<T> ReadNodeTyped(osgDB::XmlNode* uniqueIDNode)
	{
		//read the node with XmlClassManager type returned by the registry 
		hogbox::ObjectPtr readResult = this->ReadNode(uniqueIDNode); //readWrapper->GetPrototype()->GetOrLoadNode(uniqueIDNode);
		if(readResult)
		{
			osg::ref_ptr<T> castToTemplate = dynamic_cast<T*>(readResult.get());
			return castToTemplate;
		}
		return NULL;
	}

	//
	//Release a node from the database by name
	bool ReleaseNodeByID(const std::string& uniqueID);


protected:
	
	HogBoxManager(void);
	virtual ~HogBoxManager(void);

	virtual void destruct(){
		m_databaseNode = NULL;
	}


	//xml helpers
	//find a node in the database with the uniqueID property
	osgDB::XmlNode* FindNodeByUniqueIDProperty(const std::string& uniqueID, osgDB::XmlNode* xmlNode=NULL);

protected:

	osg::ref_ptr<osgDB::XmlNode> m_databaseNode;
	

};

}; //end hogbox namespace
