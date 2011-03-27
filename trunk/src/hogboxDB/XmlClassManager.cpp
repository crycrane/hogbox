#include <hogboxDB/XmlClassManager.h>

#include <hogboxDB/XmlClassWrapper.h>

using namespace hogboxDB;

XmlClassManager::XmlClassManager()
	:osg::Object()
{
}

XmlClassManager::XmlClassManager(const XmlClassManager& manager,const osg::CopyOp& copyop)
	: osg::Object(manager, copyop)
{

}

XmlClassManager::~XmlClassManager(void)
{
	OSG_NOTICE << "Deallocating XmlClassManager: Type '" << this->getName() << "'," << std::endl
							 << "                                                           Releasing '" << m_objectList.size() << "' managed objects." << std::endl;
	
	m_supportedClassTypes.clear();
	//release objects
	for(XmlNodeToObjectMap::iterator itr=m_objectList.begin();
		itr!=m_objectList.end();
		itr++)
	{
		(*itr).second = NULL;
		//osg::notify(osg::WARN)<<(*itr).second.get()->referenceCount()<<std::endl;
	}
	m_objectList.clear();
}


bool XmlClassManager::AcceptsClassType(const std::string& classType) const
{
	// check for an exact match
	std::string lowercase_ext = classType;
	if (m_supportedClassTypes.count(lowercase_ext)!=0) return true;
    
	// if plugin supports wildcard extension then passthrough all types
	return (m_supportedClassTypes.count("*")!=0);
}

//
//Reads the object in from an xml node pointing to its opening tag
//if the node has already been loaded based on its uniqueID property
//it is returned
//
hogbox::ObjectPtr XmlClassManager::GetOrLoadNode(osg::ref_ptr<osgDB::XmlNode> xmlNode)
{
	//check node is valid
	if(!xmlNode){return NULL;}

	//check we can load this class of node
	if(!this->AcceptsClassType(xmlNode->name)){return NULL;}
	
	//try to find the node in our list of already loaded nodes
	std::string uniqueIDStr;
	getXmlPropertyValue(xmlNode, "uniqueID", uniqueIDStr);
	
	hogbox::ObjectPtr existingObject = GetNodeObjectByID(uniqueIDStr);
	
	//if we find it, return it
	if(existingObject.get())
	{return existingObject;}

	//it wasn't found so create one by passing this xml node to ReadObjectFromXmlNodeImplementation
	//which will allocate a new object of the managers type and deserialise the nodes contents
	XmlClassWrapperPtr newObject = this->ReadObjectFromXmlNodeImplementation(xmlNode);
	//check it loaded
	if(newObject.get())
	{
		//add to our list of loaded nodes
		XmlNodeToObjectPair newObjectEntry(xmlNode, newObject);
		m_objectList.insert(newObjectEntry);
		return newObject->getWrappedObject();
	}

	return NULL;
}

//Find the node if it's already been loaded
hogbox::ObjectPtr XmlClassManager::GetNodeObjectByID(const std::string& uniqueID)
{
	for(XmlNodeToObjectMap::iterator itr=m_objectList.begin();
		itr != m_objectList.end();
		itr++)
	{
		std::string loadedObjectsUniqueID = (*itr).first->properties["uniqueID"];
		if(loadedObjectsUniqueID == uniqueID)
		{return (*itr).second->getWrappedObject();}
	}
	return NULL;//not found
}

//
//Release the node object if it has already been loaded
bool XmlClassManager::ReleaseNodeByID(const std::string& uniqueID)
{
	for(XmlNodeToObjectMap::const_iterator itr=m_objectList.begin();
		itr != m_objectList.end();
		itr++)
	{
		std::string loadedObjectsUniqueID = (*itr).first->properties["uniqueID"];
		if(loadedObjectsUniqueID == uniqueID)
		{
			m_objectList.erase(itr);
			return true;
		}
	}
	return false;
}

void XmlClassManager::SupportsClassType(const std::string& className, const std::string& description)
{
	m_supportedClassTypes[className] = description;
}

//
//Read this xml node into one of the supported types and add to the m_objectList
XmlClassWrapperPtr XmlClassManager::ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
{
	return NULL;
}

