#include <hogboxVision/videostream.h>

using namespace hogboxVision;

VideoStream::VideoStream() 
		: osg::ImageStream(),
		_frameRate(0.0f),
		_isValid(false),
		_hFlip(false),
		_vFlip(false),
		_isInter(false)
{

}

//
//
//
VideoStream::~VideoStream(void)
{
	pause();
	//terminate the threads etc 
	this->quit(); 
}

VideoStream::VideoStream(const VideoStream& image,const osg::CopyOp& copyop)
		: osg::ImageStream(image,copyop),
		_frameRate(image._frameRate),
		_isValid(image._isValid),
		_isInter(image._isInter),
		_hFlip(image._hFlip),
		_vFlip(image._vFlip)
{
}

//
//Actually setup the stream using file name as a video or capture source config file
//Also request the fliping and deinterlacing modes for the stream
//Returns true is the stream is setup correctly and ready to stream/play
//
bool VideoStream::CreateStream(const std::string& config, bool hflip, bool vflip, bool deinter)
{
	this->_hFlip = hflip;
	this->_vFlip = vflip;
	this->_isInter = deinter;

	//use file name as images file name
	this->setFileName(config);

	//set image as upside down if required
	if(_vFlip)
	{osg::Image::setOrigin(osg::Image::TOP_LEFT);}

	return true;
}



