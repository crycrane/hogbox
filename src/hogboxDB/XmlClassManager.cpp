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
							 << "                                                           Releasing '" << _objectList.size() << "' managed objects." << std::endl;
	
	_supportedClassTypes.clear();
	//release objects
	for(XmlNodeToObjectMap::iterator itr=_objectList.begin();
		itr!=_objectList.end();
		itr++)
	{
		(*itr).second = NULL;
		//osg::notify(osg::WARN)<<(*itr).second.get()->referenceCount()<<std::endl;
	}
	_objectList.clear();
}


bool XmlClassManager::AcceptsClassType(const std::string& classType) const
{
	// check for an exact match
	std::string lowercase_ext = classType;
	if (_supportedClassTypes.count(lowercase_ext)!=0) return true;
    
	// if plugin supports wildcard extension then passthrough all types
	return (_supportedClassTypes.count("*")!=0);
}

bool XmlClassManager::AcceptsClassType(osgDB::XmlNode* xmlNode) const
{
    if(!xmlNode){
        return false;
    }
    std::string className = xmlNode->name; 
    //see if there is a type property, if so it overrides the name
    std::string streamTypeStr = "";
    if(hogboxDB::getXmlPropertyValue(xmlNode, "type", streamTypeStr))
    {
        //if type wasn't empty
        if(!streamTypeStr.empty() && streamTypeStr != "Base"){
            className = streamTypeStr;
        }
    }
    return AcceptsClassType(className);
}

//
//Reads the object in from an xml node pointing to its opening tag
//if the node has already been loaded based on its uniqueID property
//it is returned
//
osg::ObjectPtr XmlClassManager::GetOrLoadNode(osg::ref_ptr<osgDB::XmlNode> xmlNode)
{
	//check node is valid
	if(!xmlNode){return NULL;}

	//check we can load this class of node
	if(!this->AcceptsClassType(xmlNode.get())){return NULL;}
	
	//try to find the node in our list of already loaded nodes
	std::string uniqueIDStr;
	getXmlPropertyValue(xmlNode, "uniqueID", uniqueIDStr);
	
	osg::ObjectPtr existingObject = GetNodeObjectByID(uniqueIDStr);
	
	//if we find it, return it
	if(existingObject.get())
	{return existingObject;}

	//it wasn't found so create one by passing this xml node to ReadObjectFromXmlNodeImplementation
	//which will allocate a new object of the managers type and deserialise the nodes contents
	XmlClassWrapperPtr newObject = this->readObjectFromXmlNode(xmlNode);
	//check it loaded
	if(newObject.get())
	{
        OSG_FATAL << "XmlClassManager::GetOrLoadNode: INFO: Adding Node Object with uniqueID '" << uniqueIDStr << "' to database." << std::endl;
		//add to our list of loaded nodes
		XmlNodeToObjectPair newObjectEntry(xmlNode, newObject);
		_objectList.insert(newObjectEntry);
		return newObject->getWrappedObject();
	}

	return NULL;
}

//
//Find the node if it's already been loaded
//
osg::ObjectPtr XmlClassManager::GetNodeObjectByID(const std::string& uniqueID)
{
	for(XmlNodeToObjectMap::iterator itr=_objectList.begin();
		itr != _objectList.end();
		itr++)
	{
		std::string loadedObjectsUniqueID = (*itr).first->properties["uniqueID"];
		if(loadedObjectsUniqueID == uniqueID)
		{return (*itr).second->getWrappedObject();}
	}
	return NULL;//not found
}

//
//Write an object to an xmlnode using one of the xmlclasswrappers
//
osgDB::XmlNodePtr XmlClassManager::WriteXmlNode(osg::ObjectPtr object)
{
    //check we can load this class of node
	if(!this->AcceptsClassType(object->className())){return NULL;}
    
    return this->writeObjectToXmlNode(object);
}

//
//Release the node object if it has already been loaded
bool XmlClassManager::ReleaseNodeByID(const std::string& uniqueID)
{
	for(XmlNodeToObjectMap::iterator itr=_objectList.begin();
		itr != _objectList.end();
		itr++)
	{
		std::string loadedObjectsUniqueID = (*itr).first->properties["uniqueID"];
		if(loadedObjectsUniqueID == uniqueID)
		{
			_objectList.erase(itr);
			return true;
		}
	}
	return false;
}


void XmlClassManager::SupportsClassType(const std::string& className,  XmlClassWrapperPtr wrapper)
{
	_supportedClassTypes[className] = wrapper;
}

//
//Allocate a new xml class wrapper for the passed type, returns null
//if the class type is not supported by this manager
//
XmlClassWrapper* XmlClassManager::allocateXmlClassWrapperForType(const std::string& className)
{
    //check the type is supported
    if(!this->AcceptsClassType(className)){
        OSG_FATAL << "XmlClassManager::allocateXmlClassWrapperForType: ERROR: Manager does not support class type '" << className << "'." << std::endl;
        return NULL;
    }
    return _supportedClassTypes[className]->cloneType();
}

//
//Allocate a new xml class wrapper based on the name or "type" property 
//of the passed xml node
//
XmlClassWrapper* XmlClassManager::allocateXmlClassWrapperForType(osg::ref_ptr<osgDB::XmlNode> xmlNode)
{
    if(!xmlNode.get()){
        return NULL;
    }
    std::string className = xmlNode->name; 
    //see if there is a type property, if so it overrides the name
    std::string streamTypeStr = "";
    if(hogboxDB::getXmlPropertyValue(xmlNode.get(), "type", streamTypeStr))
    {
        //if type wasn't empty
        if(!streamTypeStr.empty() && streamTypeStr != "Base"){
            className = streamTypeStr;
        }
    }
    return allocateXmlClassWrapperForType(className);
}

//
//
hogboxDB::XmlClassWrapperPtr XmlClassManager::readObjectFromXmlNode(osgDB::XmlNode* xmlNode)
{
    hogboxDB::XmlClassWrapperPtr xmlWrapper = this->allocateXmlClassWrapperForType(xmlNode);
    
    //create our object and it's xml wrapper.
    if(!xmlWrapper.get()){
        OSG_FATAL << "XmlClassManager::readObjectFromXmlNode: ERROR: Failed to allocate XmlWrapper for node '" << xmlNode->name << "'" << std::endl;
        return NULL;
    }
    
    //if the wrapper was created properly then use it 
    //to deserialize our the xmlNode into it's wrapped object
    if(!xmlWrapper->deserialize(xmlNode))
    {
        //an error occured deserializing the xml node
        return NULL;
    }
    
    //did the wrapper alocate an object
    if(!xmlWrapper->getWrappedObject()){
        OSG_FATAL << "XmlClassManager::readObjectFromXmlNode: ERROR: No object type allocated by XmlWrapper for node '" << xmlNode->name << "'" << std::endl;
        return NULL;
    }

    return xmlWrapper;
}

//
//will allocate the relevant xmlclasswrapper for the object
//then use it to serialize the object to a new xml node
//
osgDB::XmlNodePtr XmlClassManager::writeObjectToXmlNode(osg::ObjectPtr object)
{
    if(!object.get()){
        OSG_FATAL << "XmlClassManager::writeObjectToXmlNode: ERROR: Invalid Object." << std::endl;
        return NULL;
    }
    //create the xrapper for the object
    hogboxDB::XmlClassWrapperPtr xmlWrapper = this->allocateXmlClassWrapperForType(object->className());
    if(!xmlWrapper.get()){
        OSG_FATAL << "XmlClassManager::writeObjectToXmlNode: ERROR: Failed to allocate XmlWrapper for node '" << object->className() << "'" << std::endl;
        return NULL;
    }
    xmlWrapper->setWrappedObject(object.get());
    
    return this->writeObjectToXmlNode(xmlWrapper);
}

//
//Serialize the passed XmlClassWrapper into a new xml node and return
//
osgDB::XmlNodePtr XmlClassManager::writeObjectToXmlNode(XmlClassWrapperPtr xmlWrapper)
{
    osgDB::XmlNodePtr writeNode = xmlWrapper->serialize();
    return writeNode;
}


