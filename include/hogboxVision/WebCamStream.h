/* Written by Thomas Hogarth, (C) 2011
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 */


#pragma once

#include "VideoStream.h"

namespace hogboxVision {

//
//CaptureFormat
//Different capture source implementations will have different ways of representing
//the formats they support. This class acts as wrapper for a single format type
//Format can also flag itself as invalid during contructor i.e. dshow passes a wierd
//0 height format, it would flag itself as invalid
class CaptureFormat : public osg::Referenced
{
public:
	CaptureFormat() : osg::Referenced()
	{}

	bool isValid(){return _isValid;}

	int GetWidth(){return _width;}
	int GetHeight(){ return _height;}
	float GetFPS(){return _fps;}
	int GetBitRate(){return _bitRate;}


	//
	//some implementations allow you to change the fps to
	//to a value between min and max fps. This must
	//be done before the format is applied. Clamps
	//any out of range values
	virtual bool SetFormatFps(float fps) = 0;

	//
	//Return a string representation of the format
	std::string GetFormatDescription()const{return _formatDescription;}

	//
	//helper function to compare a desired format to this one the
	//returned value represents how close the passed target values are to
	//this format.
	float CompareFormat(int targetWidth, int targetHeight, int targetFPS)
	{
		float targetPixels = targetWidth * targetHeight;
		float thisPixels = _width * _height;

		//calc difference in number of pixels
		float pixelDifference = fabs(thisPixels-targetPixels);

		//get as scaler of this
		float pixelPC = (float)(pixelDifference / thisPixels);

		return pixelPC;
	}

protected:
	virtual ~CaptureFormat(){}
protected:

	//is it a valid format for capturing
	bool _isValid;

	//friendly name string for the format
	std::string _formatDescription;

	//width and height of the captured images
	int _width;
	int _height;

	int _bitRate;
	
	//frames per second for the format
	float _fps;

	//some implementation can return a min and max frame rate for the format type
	float _minFps;
	float _maxFps;
};

typedef osg::ref_ptr<CaptureFormat> CaptureFormatPtr;

//
//CaptureDevice
//Wrap an individual capture device, making its
//format list etc easliy accessible
//The formats list of a capture device, is populated once
//we connect to it
class CaptureDevice : public osg::Referenced
{
public:
	//base contructor expects a device name, which
	//can be used to identify the device
	CaptureDevice(const std::string& deviceName, const std::string& uniqueID) 
			: osg::Referenced(),
			_deviceName(deviceName),
			_uniqueID(uniqueID),
			_connectedFormat(NULL)
	{}

	//
	std::string GetDeviceName()const{return _deviceName;}
	std::string GetUniqueID()const{return _uniqueID;}

	//
	//return the list of formats supported by the device
	std::vector<CaptureFormatPtr> GetFormats(){
		//if the formats list is empty try to fill it
		if(_formats.empty()){this->CreateFormatsListImplementation();}
		return _formats;
	}

	//apply the format, making it the connected format
	virtual bool ApplyFormat(CaptureFormat* format)=0;

	//apply format by index in format list
	bool ApplyFormat(const unsigned int& index){
		if(index > _formats.size())
		{return false;}
		return ApplyFormat(_formats[index]);
	}

	//get the current connected format
	CaptureFormat* GetConnectedFormat(){
		return _connectedFormat;
	}

protected:
	virtual ~CaptureDevice(){
		_formats.clear();
		_connectedFormat=NULL;
	}
	
	//
	//Fill the _formats structure, the device  should
	//be connected by the WebCamStream implementation
	virtual bool CreateFormatsListImplementation() = 0;

protected:
	//friendly name for the device
	std::string _deviceName;

	//the unique id for the device stored as a string (in dshow this is the clsid)
	//this is needed as two of the same cmaera will have the same friendly name
	std::string _uniqueID;

	//the list of supported formats, should be filled by the
	//concrete imps contructor
	std::vector<CaptureFormatPtr> _formats;

	//the currently connected format, null if not connected
	CaptureFormatPtr _connectedFormat;
};
typedef osg::ref_ptr<CaptureDevice> CaptureDevicePtr;


//
//WebCamStream
//
//Base class for webcam/capture sources
//Exposes extra functionality such as showing properties dialogs etc
//and handling of the various formats a capture device supports
//
//The user uses GetConnectedDevicesList, to retrieve a
//list of CaptureDevices connected to the machine.
//CreateWebCamStream should be used over the base CreateStream to
//pass extra config info
//
class HOGBOXVIS_EXPORT WebCamStream : public VideoStream
{
public:
	WebCamStream() : VideoStream(),
					_captureDevice(NULL)
	{
	}
	
    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
	WebCamStream(const WebCamStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: VideoStream(image, copyop)
	{
	}

	META_Stream(hogboxVision, WebCamStream);

	//
	//Create default type stream
	virtual bool CreateStream(const std::string& config, bool hflip = false, bool vflip = false, bool deinter = false)
	{
		return CreateWebCamStream(config, 640, 480, 30, hflip, vflip, deinter);
	}

	//
	//Actually setup the capture stream using config as the device name, and target width,height and fps to
	//try and find the nearest matching format to connect to
	virtual bool CreateWebCamStream(const std::string& config, int targetWidth=640, int targetHeight=480, int targetFPS=30,
															bool hflip = false, bool vflip = false, bool deInter = false)
	{
		//pass to base
		VideoStream::CreateStream(config,hflip,vflip,deInter);

		//get the list of connected devices 
		std::vector<CaptureDevicePtr> devices = GetConnectedDevicesList();

		//try to find the requested device
		int deviceIndex = -1;
		for(unsigned int i=0; i<devices.size(); i++)
		{
			if(devices[i]->GetDeviceName().compare(config) == 0)
			{
				//found device
				deviceIndex = i;
			}
		}

		//connect to the device, 
		bool connected = false;
		if(deviceIndex != -1)
		{
			connected = this->ConnectToDeviceImplementation(devices[deviceIndex]);
		}else{
			OSG_WARN << "WebCamStream::CreateWebCamStream: WARN: The requested device '" << config << "', was not found, trying the next available." << std::endl; 	
		}

		//if we're still not connected try each device until one works
		if(!connected)
		{
			for(unsigned int i=0; (i<devices.size())&&(!connected); i++)
			{deviceIndex=i;connected = this->ConnectToDeviceImplementation(devices[i]);}
		}

		//if we're still not connected it's game over. There are no functioning devices attached
		//to the machine
		if(!connected || !_captureDevice){
			OSG_WARN << "WebCamStream::CreateWebCamStream: ERROR: Failed to connect a capture device." << std::endl; 	
			return false;
		}

		//we're connected to a device, deviceIndex should tell us which one in the list worked
		OSG_NOTICE << "WebCamStream::CreateWebCamStream: Connected to capture device '" << _captureDevice->GetDeviceName() << "'." << std::endl;

		//set name of stream to device name
		this->setName(_captureDevice->GetDeviceName());

		//we're connected to a device, now try to find the format closest to
		//our target width, height and fps

		//first create the format list for the connected device
		std::vector<CaptureFormatPtr> formats = _captureDevice->GetFormats();

		if(formats.empty())
		{
			OSG_WARN << "WebCamStream::CreateWebCamStream: ERROR: Failed to find any available formats for capture device '" << _captureDevice->GetDeviceName() << "'. It can't be used." << std::endl;
			return false;
		}

		//now loop the formats and use the format compare func to find the closest to our target
		float closestCompareDist = 999999999.0f;//FLT_MAX;
		int closestCompareIndex = -1;
		for(unsigned int i=0; i< formats.size(); i++)
		{
			float compareVal = formats[i]->CompareFormat(targetWidth, targetHeight, targetFPS);
			//get distance from 1 (1==same format)
			float compareDist = compareVal;
			if(compareDist < closestCompareDist)
			{
				closestCompareDist = compareDist;
				closestCompareIndex = i;
			}
		}

		if(closestCompareIndex == -1)
		{
			//no format found close enough, shouldn't happen as long as we have at least one format
			return false;
		}

		//try and set the fps for the selected format to match our target
		formats[closestCompareIndex]->SetFormatFps(targetFPS);

		//we have the index of the closest format to the target so apply it
		bool formatApplied = this->ApplyFormat(formats[closestCompareIndex]);
		return true;
	}

	//
	//Show a properties dialog/settings for the webcam
	bool ShowPropertiesDialog(){
		return ShowPropertiesDialogImplementation();
	}

	//
	//function to return a list of connected devices available to the
	//implementation (can we have virtual static functions ?)
	std::vector<CaptureDevicePtr> GetConnectedDevicesList(){
		return GetConnectedDevicesListImplementation();
	}

	//
	//Return the device we are actually connected to
	CaptureDevicePtr GetCaptureDevice(){
		return _captureDevice;
	}

	//apply a new format to the connected device
	//returns false on fail or if not connected to a dvice
	bool ApplyFormat(CaptureFormat* format)
	{
		if(!format){return false;}
		if(!_captureDevice)
		{
			OSG_WARN << "WebCamStream: CreateStream: ERROR: Can not apply format '" << format->GetFormatDescription() << "', the WebCamStream is not connected to a valid capture device." << std::endl;
			return false;
		}
		
		//stop the capture before changing
		this->pause();
		
		//inform of format change
		OSG_NOTICE << "WebCamStream: CreateStream: Attempting to change format of device '" << _captureDevice->GetDeviceName() << "', to '" << format->GetFormatDescription() << "'." << std::endl;
		
		if(!ApplyFormatImplementation(format))
		{
			OSG_WARN << "WebCamStream: CreateStream: ERROR: failed to change format of device '" << _captureDevice->GetDeviceName() << "', to '" << format->GetFormatDescription() << "'," << std::endl
									 << "                                                         The device will not be restarted." << std::endl;
			return false;
		}
		
		//start again
		this->play();
		return true;
	}


protected:

	virtual ~WebCamStream(void){
		_captureDevice = NULL;
	}

	//perform implementation specific showing of props
	virtual bool ShowPropertiesDialogImplementation(){return false;}

	//return a list of capture devices available to the implementation
	virtual std::vector<CaptureDevicePtr> GetConnectedDevicesListImplementation(){return std::vector<CaptureDevicePtr>();}

	//try and connect to the passed device
	virtual bool ConnectToDeviceImplementation(CaptureDevice* device){return false;}

	//reconfigure the device to stream the passed format if supported
	virtual bool ApplyFormatImplementation(CaptureFormat* format){return false;}

protected:

	//the device we are capturing from
	CaptureDevicePtr _captureDevice;

};

typedef osg::ref_ptr<WebCamStream> WebCamStreamPtr;

};

