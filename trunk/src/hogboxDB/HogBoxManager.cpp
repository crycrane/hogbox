
#include <hogboxDB/HogBoxManager.h>

using namespace hogboxDB;

//BUG@tom Since moving to CMake the Singleton class has been misbehaving. An app seems to
//use a different instance of the registry then the plugins seem to register too.
//Changing to the dreaded global instance below has fixed things but I need to try and correct this
osg::ref_ptr<HogBoxManager> s_hogboxManagerInstance = NULL;

HogBoxManager* HogBoxManager::Instance(bool erase)
{
	if(s_hogboxManagerInstance==NULL)
	{s_hogboxManagerInstance = new HogBoxManager();}		
	if(erase)
	{
		s_hogboxManagerInstance->destruct();
		s_hogboxManagerInstance = 0;
	}
    return s_hogboxManagerInstance.get();
}

HogBoxManager::HogBoxManager(void)
	: osg::Referenced(),
	m_databaseNode(NULL)
{
}

HogBoxManager::~HogBoxManager(void)
{
	m_databaseNode = NULL;
}

//
//Reads in a new HogBoxDataBase xml file, files root
//node should be of type HogBoxDataBase. If no other database
//has been loaded this files rootnode is used as the database.
//If a database already exists then this files rootnode children
//are added to the databases list of children
//
bool HogBoxManager::ReadDataBaseFile(const std::string& fileName)
{
	//check the file exists
	if(!osgDB::fileExists(fileName))
	{
		osg::notify(osg::WARN) << "XML Database Error: Failed to read xml file '" << fileName << "'," << std::endl
							   << "                                        The file does not exists." << std::endl;
		//IPHONE_PORT@tom osgDB::fileExists needs fixing on iphone by the look of things
		//return false;
	}

	//allocate the document node
	osgDB::XmlNode* doc = new osgDB::XmlNode;
	osgDB::XmlNode* root = 0;

	//open the file with xmlinput
	osgDB::XmlNode::Input input;
	input.open(fileName);
	input.readAllDataIntoBuffer();

	//read the file into out document
	doc->read(input);

	//iterate over the document nodes and try and find a HogBoxDatabase node to
	//use as a root
	for(osgDB::XmlNode::Children::iterator itr = doc->children.begin();
		itr != doc->children.end() && !root;
		++itr)
	{
		if ((*itr)->name=="HogBoxDatabase") root = (*itr);
	}

	if (root == NULL)
	{
		osg::notify(osg::WARN) << "XML Database Error: Failed to read xml file '" << fileName << "'," << std::endl
							   << "                                        Hogbox XML Database files must contain a <HogBoxDatabase> node." << std::endl;
		return false;
	}

	m_databaseNode = root;
	return true;
}


//
//Reads an xml node into a hogbox::ObjectPtr object using
//the xml nodes uniqueID property to find it in the database.
//The nodes name/head is used as a classType to find an approprite
//XmlClassManager to handle construction of the object.
hogbox::ObjectPtr HogBoxManager::ReadNodeByID(const std::string& uniqueID)
{
	//see if we can find a node with the uniqueID requested
	osgDB::XmlNode* uniqueIDNode = FindNodeByUniqueIDProperty(uniqueID);
	//if we find the node pass it to readnode and return the result
	if(uniqueIDNode)
	{return ReadNode(uniqueIDNode);}
	return NULL;
}

//
//Use the nodes name as a classtype, then query the hogbox registry
//for an XmlClassManager capable of reading the node. If an XmlClassManager
//is found that states it can read the classtype then the node is passed to
//the XmlNodeManagers GetOrLoadNode function returning the result
//If n oXmlNodeManager is found then NULL is returned
//
hogbox::ObjectPtr HogBoxManager::ReadNode(osgDB::XmlNode* inNode)
{
	if(!inNode){return NULL;}
	
	//the xml node name represents its classType
	std::string requestedClass = inNode->name;
	
	//node could be a pointer check for useID property
	std::string useIDStr;
	if(getXmlPropertyValue(inNode, "useID", useIDStr))
	{
		//if it has one then call getnodebyid to return useID pointer
		//if the useID node exists in the xml database it is returned
		//loading it and all sub nodes if nessacary
		if(!useIDStr.empty()){return ReadNodeByID(useIDStr);}
	}

	//use the requested class name to query the hogbox registry for an xmlnodemanager that is
	//cabale of loading the class type

	XmlClassManager* readManager = hogboxDB::HogBoxRegistry::Instance()->GetXmlClassManagerForClassType(requestedClass);
	if(readManager)
	{
		//read the node with XmlClassManager type returned by the registry 
		hogbox::ObjectPtr readResult = readManager->GetOrLoadNode(inNode);
		return readResult;
	}
	
	//no XmlClassManagerWrapper was found so return null
	return NULL;
}


//
//find a node in the database with the uniqueID property
//
osgDB::XmlNode* HogBoxManager::FindNodeByUniqueIDProperty(const std::string& uniqueID, osgDB::XmlNode* xmlNode)
{
	if(xmlNode==NULL){
		if(!m_databaseNode){this->ReadDataBaseFile("Data/hogboxDB.xml");}
		xmlNode = m_databaseNode;
	}

	//iterate through children of the node
	for(osgDB::XmlNode::Children::iterator itr = xmlNode->children.begin();
		itr != xmlNode->children.end();
		itr++)
	{
		osgDB::XmlNode* cur = itr->get();
		if(cur)
		{
			//get the value of the nodes uniqueID property if it has one
			std::string id; 
			if(getXmlPropertyValue(cur, "uniqueID", id))
			{
				if(id == uniqueID){return cur;}
			}
			//check children
			osgDB::XmlNode* result = FindNodeByUniqueIDProperty(uniqueID, cur);
			if(result)
			{
				return result;
			}
		}
	}
	return NULL;
}	