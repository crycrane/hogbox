#pragma once
#include <hogboxDB/HogBoxRegistry.h>
#include <hogboxDB/XmlClassManager.h>
#include <hogboxDB/HogBoxManager.h>
#include <hogboxHUD/HudRegion.h>

#include "HudRegionXmlWrapper.h"

//
//Managers VideoStream nodes in an xml document
//
class HudRegionManager : public hogboxDB::XmlClassManager
{
public:

	HudRegionManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("HudRegion", "Xml definition of HudRegion and basic derived types");
	}
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HudRegionManager(const HudRegionManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxDB, HudRegionManager)

protected:

	virtual ~HudRegionManager(void)
	{
	}
	
	//
	//Create an HudRegion from xml
	//
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		HudRegionXmlWrapperPtr xmlWrapper;

		//create our object and it's xml wrapper.
		xmlWrapper = new HudRegionXmlWrapper(xmlNode);
		
		if(!xmlWrapper.get()){return NULL;}
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
REGISTER_HOGBOXPLUGIN(HudRegion,HudRegionManager)

