#pragma once
#include <hogboxDB/XmlClassManager.h>
#include "OsgNodeXmlWrapper.h"

#include <osg/Node>

//
//Manages the loading of osg nodes
//
class OsgNodeManager : public hogboxDB::XmlClassManager
{
public:

	OsgNodeManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Node", "Xml definition of osg Node");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	OsgNodeManager(const OsgNodeManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxDB, OsgNodeManager)

protected:

	virtual ~OsgNodeManager(void)
	{
	}


	//
	//Create a HogBoxObject and read it's data in from disk
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		if(!xmlNode){return NULL;}

		hogboxDB::XmlClassWrapperPtr xmlWrapper;

		xmlWrapper = new OsgNodeXmlWrapper(xmlNode);

		if(!xmlWrapper){return NULL;}
		//did the wrapper alocate an object
		if(!xmlWrapper->getWrappedObject()){return NULL;}

		//if the wrapper was created properly then use it 
		//to deserialize our the xmlNode into it's wrapped object
		if(!xmlWrapper->deserialize(xmlNode))
		{
			//an error occured deserializing the xml node
			return NULL;
		}

		//we're done deserialising the object
		return xmlWrapper;
	}


	//
	//Handles the childnodes/members of this object, this func is feed
	//ReadObjectFromXmlNodeImplementation as it iterates over each child of this
	//object xml representation
	bool ParseMemberNode(osg::ref_ptr<osgDB::XmlNode> memberNode, osg::Group* nodeGroup)
	{
		//can't pass members if object isn't already alocated
		if(!nodeGroup){return false;}

		//filename used to load an osg supported file into the group
		if(memberNode->name=="File")
		{
			osg::ref_ptr<osg::Node> file = osgDB::readRefNodeFile(memberNode->contents);
			if(file)
			{
				nodeGroup->addChild(file);
				return true;
			}
		}
		return false;
	}

};

//
//Will register the xml mamager plugin with the hogbox registry automatically
//then this dll is loaded
REGISTER_HOGBOXPLUGIN(Node, OsgNodeManager)

