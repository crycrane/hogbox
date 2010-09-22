#include <hogboxVision/videostream.h>

#include <osg/Notify>
#include <osg/Timer>

using namespace hogboxVision;


#if !defined(_MSC_VER) || !defined(_X86_)
# include <cmath>
#endif

#ifdef WIN32
#define IDLE_TIMEOUT 33
#include <windows.h>
#else
#define IDLE_TIMEOUT 0.3 //MacPort, due to unix sleep being seconds
//#include "windowsOSX.h"   //for sleep redefines
#endif

	/*static*/
unsigned int VideoStreamBase::computeNextPowerOfTwo(unsigned int x)
{
#if defined(_MSC_VER) && defined(_X86_)
	int val = x; // Get input
	val--;
	val = (val >> 1) | val;
	val = (val >> 2) | val;
	val = (val >> 4) | val;
	val = (val >> 8) | val;
	val = (val >> 16) | val;
	val++; // Val is now the next highest power of 2.
	return val;
#else
	return ((unsigned int)(exp2((double)((int)(log2((double)x)) + 1))));
#endif
}

VideoStreamBase::VideoStreamBase() 
		: osg::ImageStream(),
		m_frameRate(0.0f),
		m_isValid(false),
		m_hFlip(false),
		m_vFlip(false),
		m_isInter(false)
{
	for (int i = 0; i < NUM_CMD_INDEX; i++)
		_cmd[i] = THREAD_IDLE;//clear all commands
	_wrIndex = _rdIndex = 0;

	//start the thread to run run()
	//this->start();
}

//
//
//
VideoStreamBase::~VideoStreamBase(void)
{
	pause();
	
	//terminate the thread if it is running
	this->quit(); 
    if( isRunning() )
    {
        // cancel the thread..
        cancel();
        // then wait for the the thread to stop running.
        while(isRunning())
        {
            OpenThreads::Thread::YieldCurrentThread();
        }   
    }
}

VideoStreamBase::VideoStreamBase(const VideoStreamBase& image,const osg::CopyOp& copyop)
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
bool VideoStreamBase::CreateStream(const std::string& config, bool hflip, bool vflip, bool deinter)
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

//
//Thread safe command stuff 
//
void VideoStreamBase::setCmd(ThreadCommand cmd)
{
    lock();
    _cmd[_wrIndex] = cmd;
    _wrIndex = (_wrIndex + 1) % NUM_CMD_INDEX;
    unlock();
}

//
// Get command
//
VideoStreamBase::ThreadCommand VideoStreamBase::getCmd()
{
    ThreadCommand cmd = THREAD_IDLE;
    lock();
    if (_rdIndex != _wrIndex) {
        cmd = _cmd[_rdIndex];
        _rdIndex = (_rdIndex + 1) % NUM_CMD_INDEX;
    }
    unlock();
    return cmd;
}

//
// Run the thread
//
void VideoStreamBase::run()
{
    bool done = false;

   const osg::Timer* timer = osg::Timer::instance();
    osg::Timer_t start_tick = timer->tick();
    osg::Timer_t last_frame_tick = start_tick;
     
    while (!done) {

        // Handle commands
        ThreadCommand cmd = getCmd();
        if (cmd != THREAD_IDLE)
		{
            switch (cmd) {
            case THREAD_STOP: // XXX
				osg::ImageStream::pause();
				this->PauseImplementation();
                break;

            case THREAD_PLAY: // XXX
				osg::ImageStream::play();
				this->PlayImplementation();
                break;

            case THREAD_REWIND: // XXX
				start_tick = timer->tick();
				last_frame_tick = start_tick;
				osg::ImageStream::rewind();
				this->RewindImplementation();
                break;

            case THREAD_QUIT: // XXX
				osg::notify(osg::NOTICE) << "quit" << std::endl;
				osg::ImageStream::quit();
				this->QuitImplementation();
                done = true;
                break;

            default:
                osg::notify(osg::WARN) << "Unknown command " << cmd << std::endl;
                break;
            }
		}
        if (_status == PLAYING) {
				UpdateStream();
        }
        else {
           // Sleep(IDLE_TIMEOUT);
        }
    }

	//exit thread
}

