#include <hogboxDB/XmlClassWrapper.h>

using namespace hogboxDB;


XmlClassWrapper::XmlClassWrapper(osgDB::XmlNode* node, const std::string& classType) 
	: osg::Referenced(),
	m_classType(classType),
	p_wrappedObject(NULL)
{
}

XmlClassWrapper::~XmlClassWrapper(void)
{
	OSG_NOTICE << "Deallocating XmlClassWrapper: Type '" << this->GetClassType() << "', UniqueID '" << this->GetUniqueID() << "'." << std::endl;
	//iterate our attributes and release also
	XmlAttributeMap::iterator itr = m_xmlAttributes.begin();
	for( ; itr != m_xmlAttributes.end(); itr++)
	{	
		(*itr).second->releaseAttribute();
	}
	p_wrappedObject=NULL;
}

//
//Return the uniqueID of the node
//
const std::string& XmlClassWrapper::GetClassType()const
{
	return m_classType;
}

//
//Return the uniqueID of the node
//
const std::string& XmlClassWrapper::GetUniqueID()const
{
	return m_uniqueID;
}

//
//return the wrapped object pointer
//
osg::Object* XmlClassWrapper::getWrappedObject()
{
	return p_wrappedObject;
}


//
//Return the attribute with the name provided, the attribute
//requires casting to the correct type for get set methods
//or can be deserialised from an xmlnode
//
XmlAttribute* XmlClassWrapper::get(const std::string& name)
{
	XmlAttributeMap::iterator _found = m_xmlAttributes.find(name);
	return (_found != m_xmlAttributes.end()) ? _found->second.get() : NULL;
}

//
//Some objects require special case name setting. the default set the objects name
//to that of the nodes uniqueID. 
//
void XmlClassWrapper::SetObjectNameFromUniqueID(const std::string& name)
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
	if(!p_wrappedObject){return false;}
	if(!in){return false;}

	//check it's of the correct class
	if(in->name != this->GetClassType())
	{
		osg::notify(osg::WARN) << "XML ERROR: Parsing classtype '" << in->name << "'," << std::endl
							   << "                   Was expecting classtype '" << this->GetClassType() << "'" << std::endl;
		return false;
	}

	//all loaded xml nodes should have a uniqueID, which will also 
	//by used as the Objects name
	std::string uniqueID;
	if(!hogboxDB::getXmlPropertyValue(in, "uniqueID", uniqueID))
	{
		osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype '" << in->name << "' should have a uniqueID property." <<std::endl 
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
					osg::notify(osg::WARN) << "XML ERROR: While reading uniqueID node '" << uniqueID << "' of type '" << in->name << "'" << std::endl
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
	this->SetObjectNameFromUniqueID(uniqueID);
	
	//the node wrapper has successfully deserialised the objec, set it's name to uniqueID
	//for future identification
	m_uniqueID = uniqueID;

	return true;
}




//
//print the xml avaliable attributes etc of the wrapped class
void XmlClassWrapper::printXmlInterface()
{
	//check we're properly wrapping something
	if(p_wrappedObject)
	{
		//print class type def with uniqueID property example
		OSG_NOTICE << "XML INTERFACE: Classtype '" << p_wrappedObject->className() << "':" <<std::endl
								 << "                            <" << p_wrappedObject->className() << " uniqueID=''>" << std::endl;

		//write out all the attributes
		XmlAttributeMap::iterator _i = m_xmlAttributes.begin();
		for (;_i != m_xmlAttributes.end(); ++_i) 
		{
			OSG_NOTICE << "                                    <" << (*_i).first << ">" << std::endl;
		}
		OSG_NOTICE << "                            </" << p_wrappedObject->className() << ">" << std::endl;
	}
}


 