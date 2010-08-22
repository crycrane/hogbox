#pragma once
#include <hogboxDB/XmlClassManager.h>
#include "OsgImageXmlWrapper.h"

#include <osg/Image>


//
//Manages the loading of osg Image files using osgDB
//
class OsgImageManager : public hogboxDB::XmlClassManager
{
public:
	OsgImageManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Image", "Xml definition of osg Image");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	OsgImageManager(const OsgImageManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}
	
	META_Box(hogboxDB, OsgImageManager)

protected:

	virtual ~OsgImageManager(void)
	{
	}

	//
	//Create a HogBoxObject and read it's data in from disk
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		if(!xmlNode){return NULL;}

		hogboxDB::XmlClassWrapperPtr xmlWrapper;

		xmlWrapper = new OsgImageXmlWrapper(xmlNode);

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

};

//
//Will register the xml mamager plugin with the hogbox registry automatically
//then this dll is loaded
REGISTER_HOGBOXPLUGIN(Image, OsgImageManager)

