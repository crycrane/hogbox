#include <hogboxDB/XmlClassWrapper.h>

using namespace hogboxDB;


XmlClassWrapper::XmlClassWrapper(const std::string& classType) 
	: osg::Referenced(),
	_classType(classType),
	p_wrappedObject(NULL)
{

}

XmlClassWrapper::~XmlClassWrapper(void)
{
	OSG_NOTICE << "Deallocating XmlClassWrapper: Type '" << this->getClassType() << "', UniqueID '" << this->getUniqueID() << "'." << std::endl;
	//iterate our attributes and release also
	XmlAttributeMap::iterator itr = _xmlAttributes.begin();
	for( ; itr != _xmlAttributes.end(); itr++)
	{	
		(*itr).second->releaseAttribute();
	}
	p_wrappedObject=NULL;
}

//
//Return the uniqueID of the node
//
const std::string& XmlClassWrapper::getClassType()const
{
	return _classType;
}

//
//Return the uniqueID of the node
//
const std::string& XmlClassWrapper::getUniqueID()const
{
	return _uniqueID;
}

//
//return the wrapped object pointer
//
osg::Object* XmlClassWrapper::getWrappedObject()
{
	return p_wrappedObject;
}

//
//Manually set an existing object to wrap
//
void XmlClassWrapper::setWrappedObject(osg::Object* object)
{
    //
    p_wrappedObject = object;
    //clear any existing attributes
    _xmlAttributes.clear();
    this->bindXmlAttributes();
}


//
//Return the attribute with the name provided, the attribute
//requires casting to the correct type for get set methods
//or can be deserialised from an xmlnode
//
XmlAttribute* XmlClassWrapper::get(const std::string& name)
{
	XmlAttributeMap::iterator _found = _xmlAttributes.find(name);
	return (_found != _xmlAttributes.end()) ? _found->second.get() : NULL;
}

//
//Some objects require special case name setting. the default set the objects name
//to that of the nodes uniqueID. 
//
void XmlClassWrapper::setObjectNameFromUniqueID(const std::string& name)
{
	p_wrappedObject->setName(name);
}

//
//Read the in xmlNodes into our wrapped object via the
//registered XmlAtributes
//
bool XmlClassWrapper::deserialize(osgDB::XmlNode* in)  
{
	//we have to have been passed a valid object in the contructor
	if(!p_wrappedObject.get()){
        p_wrappedObject = this->allocateClassType();
        if(p_wrappedObject.get()){
            this->bindXmlAttributes();
        }else{
            return false;
        }
    }
	if(!in){return false;}

    std::string xmlClassName = this->getClassNameFromXmlNode(in);
    
	//check it's of the correct class
	if(xmlClassName != this->getClassType())
	{
		OSG_WARN << "XML ERROR: Parsing classtype '" << xmlClassName << "'," << std::endl
							   << "                   Was expecting classtype '" << this->getClassType() << "'" << std::endl;
		return false;
	}

	//all loaded xml nodes should have a uniqueID, which will also 
	//by used as the Objects name
	std::string uniqueID;
	if(!hogboxDB::getXmlPropertyValue(in, "uniqueID", uniqueID))
	{
		OSG_WARN	<< "XML ERROR: Nodes of classtype '" << in->name << "' should have a uniqueID property." <<std::endl 
								<< "                    i.e. <" << in->name << " uniqueID='myID'>" << std::endl;
		return false;
	}
	
	//pass over the nodes children and see if the
	//child nodes name matches any of the xmlXrappers attributes. If one does it is passed
	//to the matching attribute for deserialising into object
	for(osgDB::XmlNode::Children::iterator itr = in->children.begin();
		itr != in->children.end();
		++itr)
	{
		//get child node pointer
		osgDB::XmlNode* attNode = itr->get();
		//check current is valid
		if(attNode)
		{
			//get any attribute matching the nodes name
			XmlAttribute* xmlAttribute = this->get(attNode->name);
			if(xmlAttribute)
			{
				//if we get a matching attribute, pass the node for deserialising
				if(!xmlAttribute->deserialize(attNode))
				{
					//returned an error while deserializing the attribute, inform the user
					//but carry on trying the other attributes as they are in seperate xmlnodes
					OSG_WARN << "XML ERROR: While reading uniqueID node '" << uniqueID << "' of type '" << in->name << "'" << std::endl
										   << "                      Failed to read value of attribute '" << attNode->name << "', it's possible the attribute exists" << std::endl
										   << "                      but the value type is incorect" << std::endl;
				}
			}else{
				//no attribute matching that name, inform user 
				OSG_NOTICE << "XML WARN: While reading uniqueID node '" << uniqueID << "' of type '" << in->name << "'" << std::endl
										 << "                    The attribute '" << attNode->name << "', was not recognised by the object type." << std::endl
										 << "                    Could it be a typo?" << std::endl;
			}
		}

	}

	//do any implementation specific setname stuff, default implementation
	//sets p_wrappedObjects name to uniqueID
	this->setObjectNameFromUniqueID(uniqueID);
	
	//the node wrapper has successfully deserialised the object, set it's name to uniqueID
	//for future identification
	_uniqueID = uniqueID;

	return true;
}

//
//Write the wrapped object into an xmlNode
//
osgDB::XmlNodePtr XmlClassWrapper::serialize()
{
    //check we have a wrapped object to write
    if(!p_wrappedObject.get()){
        return NULL;
    }
    
    //allocate the xmlnode
    osgDB::XmlNodePtr xmlNode = new osgDB::XmlNode();
    xmlNode->name = p_wrappedObject->className();
    xmlNode->type = osgDB::XmlNode::GROUP;
    
    //set the unique id property
    hogboxDB::setXmlPropertyValue(xmlNode.get(), "uniqueID", p_wrappedObject->getName());
    
    //iterate the xmlattributes and serialize each to an xmlnode attached as a child of this xmlnode
    XmlAttributeMap::iterator attItr = _xmlAttributes.begin();
    for( ; attItr!=_xmlAttributes.end(); attItr++){
        XmlAttribute* att = (*attItr).second;
        //now try to serialise the attribute
        osgDB::XmlNodePtr attNode = att->serialize();
        if(attNode.valid()){
            //successfull serialized, add the node as child of this
            xmlNode->children.push_back(attNode.get());
        }
    }
    return xmlNode;
}

//
//Return the class name from an xml node, default is the nodes name,
//but if a type property exists that is used instead
//
const std::string XmlClassWrapper::getClassNameFromXmlNode(osgDB::XmlNode* xmlNode)
{
    if(!xmlNode){
        return "";
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
    return className;
}


//
//print the xml available attributes etc of the wrapped class
void XmlClassWrapper::printXmlInterface()
{
	//check we're properly wrapping something
	if(p_wrappedObject)
	{
		//print class type def with uniqueID property example
		OSG_NOTICE << "XML INTERFACE: Classtype '" << p_wrappedObject->className() << "':" <<std::endl
								 << "                            <" << p_wrappedObject->className() << " uniqueID=''>" << std::endl;

		//write out all the attributes
		XmlAttributeMap::iterator _i = _xmlAttributes.begin();
		for (;_i != _xmlAttributes.end(); ++_i) 
		{
			OSG_NOTICE << "                                    <" << (*_i).first << ">" << std::endl;
		}
		OSG_NOTICE << "                            </" << p_wrappedObject->className() << ">" << std::endl;
	}
}


 