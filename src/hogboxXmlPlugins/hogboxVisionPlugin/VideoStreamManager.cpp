#pragma once
#include <hogboxDB/HogBoxRegistry.h>
#include <hogboxDB/XmlClassManager.h>
#include <hogboxDB/HogBoxManager.h>
#include <hogboxVision/VideoStream.h>

#include "VideoStreamXmlWrapper.h"

//
//Managers VideoStream nodes in an xml document
//
class VideoStreamManager : public hogboxDB::XmlClassManager
{
public:

	VideoStreamManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("VideoStream", "Xml definition of hogVision VideoStream");
	}
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	VideoStreamManager(const VideoStreamManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxDB, VideoStreamManager)

protected:

	virtual ~VideoStreamManager(void)
	{
	}
	
	//
	//Create an HogBoxObject from xml
	//
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		hogbox::ObjectPtr object;
		VideoStreamXmlWrapperPtr xmlWrapper;

		//create our object and it's xml wrapper.
		xmlWrapper = new VideoStreamXmlWrapper(xmlNode);
		
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
REGISTER_HOGBOXPLUGIN(VideoStream,VideoStreamManager)

