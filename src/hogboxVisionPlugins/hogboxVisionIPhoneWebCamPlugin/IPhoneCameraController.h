#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>

#include <osg/Image>
#include <osg/Texture2D>

class IPhoneCaptureFormat;
class IPhoneCaptureDevice;

//
//Sets up a capture session to stream an IPhone camera feed into an osg image
@interface IPhoneCameraController : UIViewController <AVCaptureVideoDataOutputSampleBufferDelegate> 
{	
	AVCaptureSession *_captureSession;
	AVCaptureDeviceInput* _captureInput;
	AVCaptureVideoDataOutput* _captureOutput;

	//pointer to the image we bust fill
	osg::ref_ptr<osg::Image> p_image;
	//copy the iphone image buffer so it won't get deleted out from under us
	//bit of an over head but so far seems needed
	uint8_t* _imageBuffer;
	GLenum _format;
	int _bits;
	
	bool _hFlip;
	bool _vFlip;
	
	
}

//return the capture session used by the controller
@property (nonatomic, retain) AVCaptureSession *captureSession;


//
//Init the capture controller to fill the passed image
//connect to requsted device at format
//
- (void)initCapture : (osg::Image*)image : 
						(IPhoneCaptureDevice*)device : 
						(IPhoneCaptureFormat*)format :
						(bool)hflip :
						(bool)vflip;

//
//Change the format of the device
//setting the captureSession sessionPreset
- (void)setFormat : (IPhoneCaptureFormat*)format;

- (void)mirrorOutput; 

//expose start/stop session
- (void)play;
- (void)pause;

@end