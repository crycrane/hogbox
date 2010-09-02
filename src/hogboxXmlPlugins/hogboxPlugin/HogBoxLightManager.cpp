#pragma once
#include <hogboxDB/HogBoxRegistry.h>
#include <hogboxDB/XmlClassManager.h>
#include <hogboxDB/HogBoxManager.h>
#include <hogbox/HogBoxLight.h>

#include "HogBoxLightXmlWrapper.h"

//
//Managers HogBoxLight nodes in an xml document
//
class HogBoxLightManager : public hogboxDB::XmlClassManager
{
public:

	HogBoxLightManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("HogBoxLight", "Xml definition of HogBoxLight");
	}
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxLightManager(const HogBoxLightManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxDB, HogBoxLightManager)

protected:

	virtual ~HogBoxLightManager(void)
	{
	}
	
	//
	//Create an HogBoxObject from xml
	//
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		hogboxDB::XmlClassWrapperPtr xmlWrapper;

		xmlWrapper = new HogBoxLightXmlWrapper(xmlNode);

		//create our object and it's xml wrapper.
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

		return xmlWrapper;
	}
};

//
//Will register the xml mamager plugin with the hogbox registry automatically
//then this dll is loaded
REGISTER_HOGBOXPLUGIN( HogBoxLight, HogBoxLightManager )

