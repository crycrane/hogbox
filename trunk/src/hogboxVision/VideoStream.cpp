#include <hogboxVision/videostream.h>

using namespace hogboxVision;

VideoStream::VideoStream() 
		: osg::ImageStream(),
		m_frameRate(0.0f),
		m_isValid(false),
		m_hFlip(false),
		m_vFlip(false),
		m_isInter(false)
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
		m_frameRate(image.m_frameRate),
		m_isValid(image.m_isValid),
		m_isInter(image.m_isInter),
		m_hFlip(image.m_hFlip),
		m_vFlip(image.m_vFlip)
{
}

//
//Actually setup the stream using file name as a video or capture source config file
//Also request the fliping and deinterlacing modes for the stream
//Returns true is the stream is setup correctly and ready to stream/play
//
bool VideoStream::CreateStream(const std::string& config, bool hflip, bool vflip, bool deinter)
{
	this->m_hFlip = hflip;
	this->m_vFlip = vflip;
	this->m_isInter = deinter;

	//use file name as images file name
	this->setFileName(config);

	//set image as upside down if required
	if(m_vFlip)
	{osg::Image::setOrigin(osg::Image::TOP_LEFT);}

	return true;
}



