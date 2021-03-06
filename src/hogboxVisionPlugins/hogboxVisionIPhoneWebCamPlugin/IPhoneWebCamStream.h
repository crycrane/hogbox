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

#include <hogboxVision/webcamstream.h>
#include <iostream>
#include <sstream>

/*#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>*/

#ifdef __OBJC__
@class AVCaptureDevice;
@class AVCaptureSession;
@class IPhoneCameraController;
@class NSString;
#else
class AVCaptureDevice;
class AVCaptureSession;
class IPhoneCameraController;
class NSString;
#endif



//
//IPhone implementation of captureformat
//IPhone uses presets for formats that take the form of a string
//
class IPhoneCaptureFormat : public hogboxVision::CaptureFormat
{
public:
	
	enum CaptureFormatPreset{
		PHOTO,
		HIGH,
		MEDIUM,
		LOW, //192x144 on IPod Touch 4th gen
		VGA_640X480,
		HD_720P			
	};
	
	IPhoneCaptureFormat(CaptureFormatPreset capturePreset, NSString* osPresetString) 
		: hogboxVision::CaptureFormat(),
		_capturePreset(capturePreset),
		_osPresetString(osPresetString)
	{
		_isValid = false;

		switch (_capturePreset) {
			case PHOTO:
				//store basic params
				_width = 1280;//?
				_height = 720;//?
				_bitRate = 24;//?
				_fps = 30;
				_minFps = 1;
				_maxFps = 30;
				break;
			case HIGH:
				//store basic params
				_width = 1280;//?
				_height = 720;//?
				_bitRate = 24;//?
				_fps = 30;
				_minFps = 1;
				_maxFps = 30;
				break;
			case MEDIUM:
				//store basic params
				_width = 640;//?
				_height = 480;//?
				_bitRate = 24;//?
				_fps = 30;
				_minFps = 1;
				_maxFps = 30;
				break;
			case LOW:
				//store basic params
				_width = 192;// on rear cam of ipod 4
				_height = 144;// on rear cam of ipod 4
				_bitRate = 24;//?
				_fps = 30;
				_minFps = 1;
				_maxFps = 30;
				break;
			case VGA_640X480:
				//store basic params
				_width = 640;// should always be available
				_height = 480;// 
				_bitRate = 24;//?
				_fps = 30;
				_minFps = 1;
				_maxFps = 30;
				break;
			case HD_720P:
				//store basic params
				_width = 1280;// not sure if all cams do this
				_height = 720;// 
				_bitRate = 24;//?
				_fps = 30;
				_minFps = 1;
				_maxFps = 30;
				break;
			default:
				break;
		}


		//if we have any zero params return before flagging the format as valid
		if(_width <=0 || _height <=0 || _fps <=0)
		{_isValid = false; return;}

		//create friendly description of the format
		std::ostringstream oss(std::ostringstream::out);
		oss << _width << "x" << _height << " " << (int)_fps << "-fps " << _bitRate << "-bit";
		_formatDescription = oss.str();

		_isValid = true;
	}


	//
	//some implementations allow you to change the fps to
	//to a value between min and max fps. This must
	//be done before the format is applied. Will clamp 
	//any out of range values
	virtual bool SetFormatFps(float fps)
	{
		_fps = fps;
		return true;
	}

	NSString* GetOSPresetString(){return _osPresetString;}
	
protected:

	virtual ~IPhoneCaptureFormat(){

	}

protected:

	CaptureFormatPreset _capturePreset;
	NSString* _osPresetString; 

};

typedef osg::ref_ptr<IPhoneCaptureFormat> IPhoneCaptureFormatPtr;

//
//
//
class IPhoneCaptureDevice : public hogboxVision::CaptureDevice
{
public:
	//base contructor expects a device name, which
	//can be used to identify the device
	IPhoneCaptureDevice(const std::string& deviceName, const std::string& uniqueID, AVCaptureDevice* captureDevice) 
		: hogboxVision::CaptureDevice(deviceName, uniqueID),
		p_captureDevice(captureDevice)
	{
	}
	
	//
	//fill the _formats list with data from our connected
	//device filter
	virtual bool CreateFormatsListImplementation();

	//
	//apply the format, making it the connected format
	virtual bool ApplyFormat(hogboxVision::CaptureFormat* format);

	//IPhone stuff
	
	//return pointer to the os device this represents
	AVCaptureDevice* GetAVCaptureDevice(){return p_captureDevice;}
	
	//
	//If this device becomes connected then we need to set the
	//capture session associated with it
	void SetAVCaptureSession(AVCaptureSession* session){p_captureSession = session;}
	
	//
	//return the av capture session if connected
	AVCaptureSession* GetAVCaptureSession(){return p_captureSession;}

protected:
	virtual ~IPhoneCaptureDevice(){
	}

	//the device object from ios
	AVCaptureDevice* p_captureDevice;
	
	//if a device is connected then this is a pointer to
	//the AVCaptureSession being used to stream it
	AVCaptureSession* p_captureSession;

};

//
//IPhoneWebCamStream
//
//Used to create an osg image stream of a IPhone capture source, i.e. a webcam
//A DSCaptureStream config file is used to supply a device name and format
//
class IPhoneWebCamStream : public hogboxVision::WebCamStream
{
public:
	IPhoneWebCamStream(void);
	
    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
	IPhoneWebCamStream(const IPhoneWebCamStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Stream(hogboxVision, IPhoneWebCamStream);

	//
	//Actually setup the capture stream using config as the device name, and target width,height and fps to
	//try and find the nearest matching format to connect to
	virtual bool CreateWebCamStream(const std::string& config, int targetWidth=800, int targetHeight=600, int targetFPS=60,
											bool hflip = false, bool vflip = false, bool deInter = false);


	virtual void SetVerticalFlip(bool flip);
	virtual void SetHorizontalFlip(bool flip);
	virtual void SetDeinterlace(bool deInter);

	//TEMP@tom
	//
	//Inherit from stream
	virtual void UpdateStream();

protected:

	virtual ~IPhoneWebCamStream(void);

	//Inherit from streambase
	//virtual void UpdateStream();


	//child class to implement
	virtual void PlayImplementation();

	//child class to implement
	virtual void PauseImplementation();

	//child class to implement
	virtual void RewindImplementation();

	//child class to implement
	virtual void QuitImplementation();


	//perform implementation specific showing of props
	virtual bool ShowPropertiesDialogImplementation();

	//return a list of capture devices available to the implementation
	virtual std::vector<hogboxVision::CaptureDevicePtr> GetConnectedDevicesListImplementation();

	//try and connect to the passed device
	virtual bool ConnectToDeviceImplementation(hogboxVision::CaptureDevice* device);

	//reconfigure the device to stream the passed format if supported
	virtual bool ApplyFormatImplementation(hogboxVision::CaptureFormat* format);


protected:

	IPhoneCameraController* _iphoneCameraController;

};

