#pragma once

#include <hogboxVision/VideoStream.h>
#include <hogboxVision/VisionRegistry.h>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for HogBoxObject, used by HogBoxObjectManager
//to read write HogBoxObjects to xml
//
class VideoStreamXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass HogBoxObject to be wrapped
	VideoStreamXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "VideoStream")
	{
		hogboxVision::VideoStreamBase* stream = NULL;

		//get the Texture type from the nodes 'type' property
		std::string streamTypeStr;
		if(!hogboxDB::getXmlPropertyValue(node, "type", streamTypeStr))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype 'VideoStream' should have a 'type' property." <<std::endl 
									<< "                    i.e. <VideoStream uniqueID='myID' type='VideoFile'>" << std::endl;
			return;
		}
		
		//allocate the correct type
		if(streamTypeStr == "VideoFile")
		{
			stream = hogboxVision::VisionRegistry::Instance()->AllocateVideoFileStream();
		}

		//add the base attributes
		
		//src is file or device name
		m_xmlAttributes["Src"] = new hogboxDB::TypedXmlAttribute<std::string>(&_srcName);

		//
		m_xmlAttributes["VFlip"] = new hogboxDB::TypedXmlAttribute<bool>(&_vFlip);
		m_xmlAttributes["HFlip"] = new hogboxDB::TypedXmlAttribute<bool>(&_hFlip);
		m_xmlAttributes["Deinterlace"] = new hogboxDB::TypedXmlAttribute<bool>(&_deinter);

		//store the VideoFileStrem as the wrapped object
		p_wrappedObject = stream;
	}

	//overload deserialise to call create stream once our atts are loaded
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//cast wrapped to VideoFileStream
		hogboxVision::VideoFileStream* fileStream = dynamic_cast<hogboxVision::VideoFileStream*>(p_wrappedObject.get());
		if(fileStream)
		{
			return fileStream->CreateStream(_srcName, _hFlip, _vFlip, _deinter);
		}

		return false;
	}

protected:

	virtual ~VideoStreamXmlWrapper(void){}

protected:

	//helper loading variables
	std::string _srcName;

	bool _vFlip;
	bool _hFlip;
	bool _deinter;

	//webcam stream only
	int _width;
	int _height;
	int _fps;
};

typedef osg::ref_ptr<VideoStreamXmlWrapper> VideoStreamXmlWrapperPtr;


