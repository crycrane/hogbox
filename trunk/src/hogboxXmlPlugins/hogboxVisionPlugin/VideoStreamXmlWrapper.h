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
	VideoStreamXmlWrapper() 
			: hogboxDB::XmlClassWrapper("VideoStream"),
			_streamTypeStr(""),
			_srcName(""),
			_vFlip(false),
			_hFlip(false),
			_deinter(false),
			//good cam defaults
			_width(640),
			_height(480),
			_fps(30),
			_bitrate(16)
	{
		hogboxVision::VideoStreamPtr stream = NULL;

		//get the Texture type from the nodes 'type' property
		std::string streamTypeStr;
		if(!hogboxDB::getXmlPropertyValue(in, "type", streamTypeStr))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype 'VideoStream' should have a 'type' property." <<std::endl 
									<< "                    i.e. <VideoStream uniqueID='myID' type='VideoFile'>" << std::endl;
			return;
		}

		_streamTypeStr = streamTypeStr;

		//see if a specific plugin is specified
		std::string plugin = "";
		if(!hogboxDB::getXmlPropertyValue(in, "plugin", plugin))
		{
		}

		//allocate the correct type
		if(_streamTypeStr == "VideoFile")
		{
			stream = hogboxVision::VisionRegistry::Instance()->AllocateVideoFileStream(plugin);
		}else if(_streamTypeStr == "WebCam"){
			stream = hogboxVision::VisionRegistry::Instance()->AllocateWebCamStream(plugin);
		}

		//add the base attributes
		
		//src is file or device name
		_xmlAttributes["Src"] = new hogboxDB::TypedXmlAttribute<std::string>(&_srcName);

		//
		_xmlAttributes["VFlip"] = new hogboxDB::TypedXmlAttribute<bool>(&_vFlip);
		_xmlAttributes["HFlip"] = new hogboxDB::TypedXmlAttribute<bool>(&_hFlip);
		_xmlAttributes["Deinterlace"] = new hogboxDB::TypedXmlAttribute<bool>(&_deinter);

		_xmlAttributes["Width"] = new hogboxDB::TypedXmlAttribute<int>(&_width);
		_xmlAttributes["Height"] = new hogboxDB::TypedXmlAttribute<int>(&_height);
		_xmlAttributes["Fps"] = new hogboxDB::TypedXmlAttribute<int>(&_fps);
		_xmlAttributes["Bitrate"] = new hogboxDB::TypedXmlAttribute<int>(&_bitrate);

		//store the VideoFileStrem as the wrapped object
		p_wrappedObject = stream;
	}

	//overload deserialise to call create stream once our atts are loaded
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}


		//allocate the correct type
		if(_streamTypeStr == "VideoFile")
		{
			//cast wrapped to VideoFileStream
			hogboxVision::VideoFileStream* fileStream = dynamic_cast<hogboxVision::VideoFileStream*>(p_wrappedObject.get());
			if(fileStream)
			{
				return fileStream->CreateStream(_srcName, _hFlip, _vFlip, _deinter);
			}
		}else if(_streamTypeStr == "WebCam"){
			//cast wrapped to WebCamStream
			hogboxVision::WebCamStream* webcamStream = dynamic_cast<hogboxVision::WebCamStream*>(p_wrappedObject.get());
			if(webcamStream)
			{
				return webcamStream->CreateWebCamStream(_srcName, _width, _height, _fps, _hFlip, _vFlip, _deinter);
			}
		}

		return false;
	}

protected:

	virtual ~VideoStreamXmlWrapper(void){}

protected:

	//type of stream
	//VideoFile = VideoFileStream
	//WebCamS = WebCamStream
	std::string _streamTypeStr;

	//helper loading variables
	std::string _srcName;

	bool _vFlip;
	bool _hFlip;
	bool _deinter;

	//webcam stream only
	int _width;
	int _height;
	int _fps;
	int _bitrate;
};

typedef osg::ref_ptr<VideoStreamXmlWrapper> VideoStreamXmlWrapperPtr;


