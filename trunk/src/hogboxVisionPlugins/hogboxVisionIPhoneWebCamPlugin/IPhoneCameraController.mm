#import "IPhoneCameraController.h"

#include <osg/ImageUtils>
#include "IPhoneWebCamStream.h"

@implementation IPhoneCameraController

@synthesize captureSession = _captureSession;


#pragma mark -
#pragma mark Initialization
- (id)init {
	self = [super init];
	if (self) {

	}
	return self;
}

- (void)viewDidLoad {
	/*We intialize the capture*/
	//[self initCapture];
}


- (void)initCapture : (osg::Image*)image : 
						(IPhoneCaptureDevice*)device : 
						(IPhoneCaptureFormat*)format  :
						(bool)hflip :
						(bool)vflip
{
	
	p_image = image;
	_hFlip = hflip;
	_vFlip = vflip;
	
	//connect to the requested device (I think actual contect is cause later by the capture session)
	_captureInput = [AVCaptureDeviceInput 
										  deviceInputWithDevice:device->GetAVCaptureDevice()
										  error:nil];
	
	//create the output to deliver use sample buffers
	_captureOutput = [[AVCaptureVideoDataOutput alloc] init];
	//drop frames for realtime
	_captureOutput.alwaysDiscardsLateVideoFrames = YES; 
	//set the frame rate of the output
	_captureOutput.minFrameDuration = CMTimeMake(1, format->GetFPS());
	
	//for now rgba/4 components, but this seems dumb
	_format = GL_RGBA;
	_bits = 4;

	
	//create a queue then set this as the sampleBuffer delegate for the
	//captureOutput. This is what causes our captureOutput() function to be called
	dispatch_queue_t queue;
	queue = dispatch_queue_create("cameraQueue", NULL);
	[_captureOutput setSampleBufferDelegate:self queue:queue];
	dispatch_release(queue);
	
	// Set the video output to store frame in BGRA for now till we have res working
	NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey; 
	NSNumber* value = [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA];  //kCVPixelFormatType_32BGRA
	NSDictionary* videoSettings = [NSDictionary dictionaryWithObject:value forKey:key]; 
	[_captureOutput setVideoSettings:videoSettings]; 
	
	
	//create the capture session to retrive the frames from the camera
	self.captureSession = [[AVCaptureSession alloc] init];
	//set our inout outputs
	[self.captureSession addInput:_captureInput];
	[self.captureSession addOutput:_captureOutput];

	//set sessionPreset which basically controls the resolution, will also allocate
	//the image space rewuired
	[self setFormat:format];
	
	[self mirrorOutput];
	
}

//
//Change the format of the device
//setting the captureSession sessionPreset
//
- (void)setFormat : (IPhoneCaptureFormat*)format
{
	[self.captureSession beginConfiguration];
	//set sessionPreset which basically controls the resolution
    self.captureSession.sessionPreset = format->GetOSPresetString();//AVCaptureSessionPreset640x480;
	[self.captureSession commitConfiguration];
	
	//if the res changes resize our imahe
	int width = format->GetWidth();
	int height = format->GetHeight();
	if(_imageBuffer){delete [] _imageBuffer;}
	p_image->allocateImage(width, height, 1, _format, GL_UNSIGNED_BYTE, 1);
	osg::clearImageToColor(p_image.get(), osg::Vec4(0.1, 0.1, 0.1, 1.0));
	_imageBuffer = new uint8_t[(width * height) * _bits];
}

- (void)play 
{
	if (self.captureSession) {
		//Start capturing
		[self.captureSession startRunning];
	}	
}

- (void)pause
{
	if (self.captureSession) {
		//Start capturing
		[self.captureSession stopRunning];
	}	
}

- (void)mirrorOutput 
{
    NSArray *connections = _captureOutput.connections;
    for ( AVCaptureConnection *conn in connections ) {
        // Ask the current connection's input for its media type
       // conn.videoMirrored = YES;//![conn isVideoMirrored];
		if([conn isVideoMirrored])
		{
			OSG_WARN << "Video IS mirrored!" << std::endl;
			conn.videoMirrored = NO;
		}else{
			OSG_WARN << "Video is NOT mirrored!" << std::endl;
			if([conn isVideoMirroringSupported])
			{
				OSG_WARN << "Video Mirroring IS Supported!" << std::endl;
				conn.videoMirrored = YES;
			}else {
				OSG_WARN << "Video Mirroring is NOT Supported!" << std::endl;
			}

		}
        // Ask the current connection's input for its media type
        NSArray *inputPorts = conn.inputPorts;
        for ( AVCaptureInputPort *port in inputPorts ) {
            if ( [port.mediaType isEqual:AVMediaTypeVideo] ) {
                // Found a match.
				if([conn isVideoOrientationSupported])
				{
					OSG_NOTICE << "Video Orientation IS supported."<<std::endl;
				}else{
					OSG_NOTICE << "Video Orientation is NOT supported."<<std::endl;
				}
                return;
            }
        }
		

		/*if([conn isVideoOrientationSupported:AVCaptureVideoOrientationPortraitUpsideDown])
		{
			OSG_NOTICE << "Video Supports Potrait Upside down orientation."<<std::endl;
		}
		if([conn isVideoOrientationSupported:AVCaptureVideoOrientationLandscapeLeft])
		{
			OSG_NOTICE << "Video Supports Landscape Left orientation."<<std::endl;
		}
		if([conn isVideoOrientationSupported:AVCaptureVideoOrientationLandscapeRight])
		{
			OSG_NOTICE << "Video Supports Landscape Right orientation."<<std::endl;
		}*/
    }
}

#pragma mark -
#pragma mark AVCaptureSession delegate
- (void)captureOutput:(AVCaptureOutput *)captureOutput 
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer 
	   fromConnection:(AVCaptureConnection *)connection 
{ 
	//connection.videoMirrored = YES;
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	//get the image buffer then lock it so we can safely copy it
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer); 
    CVPixelBufferLockBaseAddress(imageBuffer,0); 
	
    //get pointer to raw pixel buffer
    uint8_t *baseAddress = (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer); 
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer); 
    size_t width = CVPixelBufferGetWidth(imageBuffer); 
    size_t height = CVPixelBufferGetHeight(imageBuffer); 
	
	//NSLog(@"BUFFER wh %d, %d bpr %d",width, height, bytesPerRow);
	
	//allocate temo RGB buffer

	/*
	 //convert to RGB
	unsigned char* oriBuff = baseAddress;
	unsigned char* copyImage = _imageBuffer;
	int pixel = 0;
	while(pixel<width*height)
	{
		
		memcpy(copyImage,oriBuff,3);
		oriBuff+=4; //move rgba
		copyImage+=3; //move rgb
		pixel++;
	}*/
	
	/*if(_hFlip)//way too slow
	{
		//pointer to begining of source
		unsigned char* sourceTemp=baseAddress;
		//destination, end of first line
		unsigned char* destTemp=_imageBuffer;
		destTemp += (width*4)-4;
		
		//loop lines
		for(int r=0; r<height; r++)
		{
			//copyline of pixels 
			for(int p=0; p<width; p++)
			{
				memcpy(destTemp,sourceTemp,4);
				destTemp-=(4); //move back one pixel
				sourceTemp+=(4); //move to the next pixel
			}
			//move the destination to the end of the next line (should be move two lines
			destTemp+= ((width*4)*2);
		}
	}*/
	if(_vFlip)
	{
		//very other row
		unsigned char* sourceTemp=baseAddress;
		//destination, end of first line
		unsigned char* destTemp=_imageBuffer;
		destTemp += (width*4)*((height)-1);
		
		for(int r=0; r<height; r++)
		{
			memcpy(destTemp,sourceTemp,(width*4));
			destTemp-=(width*4); //move one line in the dest
			sourceTemp+=(width*4); //skip a line of the source for inter
		}
	}else{
		//copy buffer to local image buffer
		memcpy(_imageBuffer, baseAddress, (width*height)*_bits);
	}
	
	//set image with image copy, although we could probably copy straight to image.data
	p_image->setImage(width, height,1, _format, GL_BGRA, 
					 GL_UNSIGNED_BYTE, _imageBuffer, osg::Image::NO_DELETE, 1);

	//unlock the buffer
	CVPixelBufferUnlockBaseAddress(imageBuffer,0);
	
	[pool drain];
} 

#pragma mark -
#pragma mark Memory management

- (void)viewDidUnload {

}

- (void)dealloc {
	if(_imageBuffer){delete _imageBuffer;_imageBuffer=NULL;}
	[self.captureSession release];
    [super dealloc];
}


@end