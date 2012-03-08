#include "IPhoneWebCamStream.h"

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>


#include <hogboxVision/VisionRegistry.h>
REGISTER_VISION_WEBCA_PLUGIN( iphone, IPhoneWebCamStream )


//
//hogboxVision Capture device implementation

//
//fill the _formats list with data from our connected
//device filter
bool IPhoneCaptureDevice::CreateFormatsListImplementation()
{
	if(p_captureDevice)
	{
		BOOL supports = FALSE;
		//make 640x480 th default
		supports = [p_captureDevice supportsAVCaptureSessionPreset:AVCaptureSessionPreset640x480];
		if(supports)
		{
			IPhoneCaptureFormatPtr format = new IPhoneCaptureFormat(IPhoneCaptureFormat::VGA_640X480, AVCaptureSessionPreset640x480);
			_formats.push_back(format);
		}
		supports = [p_captureDevice supportsAVCaptureSessionPreset:AVCaptureSessionPresetPhoto];
		if(supports)
		{
			IPhoneCaptureFormatPtr format = new IPhoneCaptureFormat(IPhoneCaptureFormat::PHOTO, AVCaptureSessionPresetPhoto);
			//_formats.push_back(format);
		}
		supports = [p_captureDevice supportsAVCaptureSessionPreset:AVCaptureSessionPresetHigh];
		if(supports)
		{
			IPhoneCaptureFormatPtr format = new IPhoneCaptureFormat(IPhoneCaptureFormat::HIGH, AVCaptureSessionPresetHigh);
			_formats.push_back(format);
		}
		supports = [p_captureDevice supportsAVCaptureSessionPreset:AVCaptureSessionPresetMedium];
		if(supports)
		{
			IPhoneCaptureFormatPtr format = new IPhoneCaptureFormat(IPhoneCaptureFormat::MEDIUM, AVCaptureSessionPresetMedium);
			_formats.push_back(format);
		}
		supports = [p_captureDevice supportsAVCaptureSessionPreset:AVCaptureSessionPresetLow];
		if(supports)
		{
			IPhoneCaptureFormatPtr format = new IPhoneCaptureFormat(IPhoneCaptureFormat::LOW, AVCaptureSessionPresetLow);
			_formats.push_back(format);
		}
		supports = [p_captureDevice supportsAVCaptureSessionPreset:AVCaptureSessionPreset1280x720];
		if(supports)
		{
			IPhoneCaptureFormatPtr format = new IPhoneCaptureFormat(IPhoneCaptureFormat::HD_720P, AVCaptureSessionPreset1280x720);
			_formats.push_back(format);
		}
		return true;
	}
	return false;
}

//apply the format, making it the connected format
bool IPhoneCaptureDevice::ApplyFormat(hogboxVision::CaptureFormat* format)
{
	//cast the format to IPhone format
	IPhoneCaptureFormat* iphoneFormat = dynamic_cast<IPhoneCaptureFormat*>(format);
	if(!iphoneFormat){return false;}
	
	//check theres a Session present
	if(!p_captureSession)
	{
		osg::notify(osg::WARN) << "IPhoneCaptureDevice: ApplyFormat: ERROR Failed to apply format '" << iphoneFormat->GetFormatDescription() << "', on device '" << GetDeviceName() << "'," << std::endl
		<< "                                                                              IPhone Capture Device not connected or a valid capture session is not set" << std::endl;
		return false;
	}
	

	//check the preset is supported
	if([p_captureDevice supportsAVCaptureSessionPreset:iphoneFormat->GetOSPresetString()])
	{
		//apply format
		//do we need to re setup the capture session ? lets find out
		[p_captureSession beginConfiguration];
//		p_captureSession.sessionPreset = iphoneFormat->GetOSPresetString();
		[p_captureSession commitConfiguration];
		return true;
	}
	
	return false;
}

//IPhone webcamstream implemention


//
//
//
IPhoneWebCamStream::IPhoneWebCamStream() 
	: hogboxVision::WebCamStream(),
	_iphoneCameraController(nil)
{
}


IPhoneWebCamStream::~IPhoneWebCamStream(void)
{
	OSG_DEBUG << "Destruct IPhoneWebCamStream" << std::endl;

	if(_iphoneCameraController)
	{[_iphoneCameraController release];}
}

IPhoneWebCamStream::IPhoneWebCamStream(const IPhoneWebCamStream& image,const osg::CopyOp& copyop)
		: hogboxVision::WebCamStream(image,copyop)
{
}

//
//Actually setup the dshow capture stream using cofig as a IPhoneWebCamStream specific config file
//See LoadSettings for info of config format
//
bool IPhoneWebCamStream::CreateWebCamStream(const std::string& config, int targetWidth, int targetHeight, int targetFPS,
																	bool hflip, bool vflip, bool deInter)
{

	//create the webcam stream 
	if(!hogboxVision::WebCamStream::CreateWebCamStream(config, targetWidth, targetHeight, targetFPS, hflip, vflip, deInter))
	{
		return false;
	}

	//check final dimensions

	//this->_s = _osgRenderer->GetMediaWidth();
	//this->_t = _osgRenderer->GetMediaHeight();  


	[_iphoneCameraController mirrorOutput];
	
	//start capture

	//is valid
	_isValid=true;

	return true;
}

void IPhoneWebCamStream::PlayImplementation()
{
	if(_iphoneCameraController)
	{
		[_iphoneCameraController play];
	}
}

/// Stop stream at current position.
void IPhoneWebCamStream::PauseImplementation() 
{ 
	if(_iphoneCameraController)
	{
		[_iphoneCameraController pause];
	}
}

/// Rewind stream to beginning. (can't rewind a capturestream)
void IPhoneWebCamStream::RewindImplementation() 
{   
	
}

//
//child class to implement
//
void IPhoneWebCamStream::QuitImplementation()
{
	//this->pause();//?
	PauseImplementation();//?
}

//
//perform implementation specific showing of props
//
bool IPhoneWebCamStream::ShowPropertiesDialogImplementation()
{
	//cast capture device to dshow
	IPhoneCaptureDevice* iphoneDevice = dynamic_cast<IPhoneCaptureDevice*>(_captureDevice.get());
	if(!iphoneDevice){return false;}

	return true;
}

//
//return a list of capture devices available to the implementation
//
std::vector<hogboxVision::CaptureDevicePtr> IPhoneWebCamStream::GetConnectedDevicesListImplementation()
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	std::vector<hogboxVision::CaptureDevicePtr> deviceList;

	//get camera devices array from AVCaptureDevice
	NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    NSEnumerator* enumerator = [devices objectEnumerator];
    AVCaptureDevice* pDevice;
	
	//iterate over the devices array
    while ( pDevice = [enumerator nextObject] ) 
	{
		NSString* nsName = [pDevice localizedName];
		NSString* nsID = [pDevice uniqueID]; //modelID];
		
		std::string deviceName = [nsName cStringUsingEncoding:NSASCIIStringEncoding];
		std::string deviceID = [nsID cStringUsingEncoding:NSASCIIStringEncoding];
		
		OSG_INFO << "IPhoneWebCamStream::GetConnectedDevicesListImplementation: INFO: Found Device of name '" << deviceName << "' with ID of '" << deviceID << "'." << std::endl;
		
		//add the device to our list
		osg::ref_ptr<IPhoneCaptureDevice> device = new IPhoneCaptureDevice(deviceName,deviceID,pDevice);
		deviceList.push_back(device);
    }
	
	
	[pool release];
	
	return deviceList;
}

//
//try and connect to the passed device
//the dshow graph should have already been created by createStream
//
bool IPhoneWebCamStream::ConnectToDeviceImplementation(hogboxVision::CaptureDevice* device)
{

	//the device type needs to be IPhoneCaptureDevice
	IPhoneCaptureDevice* iphoneDevice = dynamic_cast<IPhoneCaptureDevice*>(device);
	if(!iphoneDevice){return false;}

	//allocate the iphonecameracontroller
	if(_iphoneCameraController)
	{
		pause();
		[_iphoneCameraController release];
	}
	_iphoneCameraController = [[IPhoneCameraController alloc] init];

	//get the device default format
	std::vector<hogboxVision::CaptureFormatPtr> formats = iphoneDevice->GetFormats();
	
	//ceck we get at least one device
	if(formats.size() == 0)
	{
		OSG_WARN << "IPhoneWebCamStream::ConnectToDeviceImplementation: ERROR: Failed to connect to device '" << device->GetDeviceName() << "', no formats were found." <<std::endl;
		return false;
	}
	
	IPhoneCaptureFormat* iphoneDefaultFormat = dynamic_cast<IPhoneCaptureFormat*> (formats[0].get());
	
	//init the capture
	[_iphoneCameraController initCapture:this :iphoneDevice :iphoneDefaultFormat: _hFlip: _vFlip];
	
	//now we're connected set the devices AVCaptureSession pointer to ours
	iphoneDevice->SetAVCaptureSession([_iphoneCameraController captureSession]);
	
	//set the pointer to the connected device
	_captureDevice = iphoneDevice;

	
	//now try a basic connection to the device using the first format (this seems to be the only way to tell
	//if the capture pin is in use)
	//@TO_001, Applying format twice could cause errors with rebuilding the renderer
	//needs testing
	
	/*if(formats.size()>0)
	{
		if(!this->ApplyFormat(formats[0]))
		{_captureDevice = NULL; return false;}
	}*/

	return true;
}

//
//reconfigure the device to stream the passed format if supported
//
bool IPhoneWebCamStream::ApplyFormatImplementation(hogboxVision::CaptureFormat* format)
{
	//check we have a device
	if(!_captureDevice){return false;}

	//we should now be connected to valid capture device, cast it to dshowdevice
	IPhoneCaptureDevice* iphoneDevice = dynamic_cast<IPhoneCaptureDevice*>(_captureDevice.get());
	if(!iphoneDevice){return false;}

	if(_iphoneCameraController)
	{
		[_iphoneCameraController setFormat:format];
	}
	/*
	//try and apply the new format to the device
	if(!_captureDevice->ApplyFormat(format))
	{
		OSG_WARN << "IPhoneWebCamStream::ApplyFormatImplementation: ERROR: Failed to apply IPhone format, " << std::endl
		<<	"													   " << format->GetFormatDescription() << std::endl;
		return false;
	}*/

	return true;
}


//
// Set the flipping states of the callbacks
//
void IPhoneWebCamStream::SetVerticalFlip(bool flip)
{
	hogboxVision::WebCamStream::SetVerticalFlip(flip);

}

void IPhoneWebCamStream::SetHorizontalFlip(bool flip)
{
	hogboxVision::WebCamStream::SetHorizontalFlip(flip);
}

//
// Deinterlace requires a reconnection as the image size will change
//
void IPhoneWebCamStream::SetDeinterlace(bool deInter)
{
	hogboxVision::WebCamStream::SetDeinterlace(deInter);
}

void IPhoneWebCamStream::UpdateStream()
{	
	//this->lock();
	//this->unlock();
}

