#pragma once

#include <hogboxVision/Export.h>

#include <osg/ImageStream>
#include <osg/notify>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

namespace hogboxVision {

#define NUM_CMD_INDEX 5

//
//Macro fro defining the osg base funcs
#define META_Stream(libraryname, classname) \
	virtual osg::Object* cloneType() const { return new classname(); } \
	virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new classname(*this,copyop); } \
	virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const classname*>(obj)!=0; } \
	virtual const char* libraryName() const { return #libraryname; } \
	virtual const char* className() const { return #classname; }


//
//VideoStreamBase
//
// Base class for running video streams in osg
// Inherits from osg image so the stream can be used as a texture in osg
// also inherits from Thread so it can be auto updated if required
// Call create stream to setup a stream rendering into an osg image
// Call play to run the stream from its current position
// Call pause to stop at current position
// Call rewind to return to the first frame of a stream
//
class HOGBOXVIS_EXPORT VideoStreamBase : public osg::ImageStream, protected OpenThreads::Thread
{
public:
	VideoStreamBase();
	
    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
	VideoStreamBase(const VideoStreamBase& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Stream(MagicSymbol, VideoStreamBase);

	//
	//Actually setup the stream using file name as a video or capture source config file
	//Also request the fliping and deinterlacing modes for the stream
	//Returns true is the stream is setup correctly and ready to stream/play
	virtual bool CreateStream(const std::string& config, bool hflip = false, bool vflip = false, bool deinter = false);

	bool isValid(){return m_isValid;}

	//
	//Play stream from current position, calls PlayImplementation 
	//when thread is ready
	void play(){ PlayImplementation();setCmd(THREAD_PLAY); }
	//
	//Stop stream at current position, calls StopImplementation 
	//when thread is ready
	void pause() { PauseImplementation();setCmd(THREAD_STOP); }
	//
	//Rewind stream to beginning, calls RewindImplementation 
	//when thread is ready
	void rewind() { RewindImplementation();setCmd(THREAD_REWIND); }

	//
	//Quit the thread if it is running, calls QuitImplementation 
	//when thread is ready
	void quit() { setCmd(THREAD_QUIT); }
	
	//
	//Request a stream to be fliped vertically
	void SetVerticalFlip(bool flip){m_vFlip = flip;}
	bool GetVerticalFlip(){return m_vFlip;}

	//
	//Request a stream to be flipped horizontally
	void SetHorizontalFlip(bool flip){m_hFlip=flip;}
	bool GetHorizontalFlip(){return m_hFlip;}

	//
	//Request a stream have deinterlacing applied, i.e. remo
	void SetDeinterlace(bool deInter){m_isInter=deInter;}
	bool GetDeinterlace(){return m_isInter;}

	//
	//Hack for callback when the video reaches its end (yuck)
	virtual bool HasVideoEnded(){ 
		return false;
	}

	//
	//Helper to calculate the next power of two up from value x
	static unsigned int computeNextPowerOfTwo(unsigned int x);

protected:

	virtual ~VideoStreamBase(void);

	//
	//The threaded function
	void run();

	//
	//Get the next frame in the stream and insert in this image
	//called by run when in play mode
	virtual void UpdateStream(){}

	//child class to implement
	virtual void PlayImplementation(){}

	//child class to implement
	virtual void PauseImplementation(){}

	//child class to implement
	virtual void RewindImplementation(){}

	//child class to implement
	virtual void QuitImplementation(){}


protected:
	enum ThreadCommand {
		THREAD_IDLE = 0,  //thread calls sleep
		THREAD_STOP,      //thread request stopping of stream, stops calls to updateStream
		THREAD_PLAY,      //thread request play stream, starts calls to update stream
		THREAD_REWIND,    //request rewind does not effect thread run state
		THREAD_QUIT      //stop threading function
	};
	ThreadCommand _cmd[NUM_CMD_INDEX];
	int _wrIndex, _rdIndex;

	OpenThreads::Mutex _mutex;

	// Lock/unlock object.
	inline void lock() { _mutex.lock(); }
	inline void unlock() { _mutex.unlock(); }

	/// Set command.
	void setCmd(ThreadCommand cmd);

	/// Get command.
	ThreadCommand getCmd();
protected:

	//is the stream ready to play
	bool m_isValid;

	//frame rate of video in frames per second
	double m_frameRate;

	//width and height of the stream are stored in the s() and t() of the osg::Image

	//request deinterlacing
	bool m_isInter;
	//flip the stream vertically
	bool m_vFlip;
	//flip the stream horizontally
	bool m_hFlip;

};

typedef osg::ref_ptr<VideoStreamBase> VideoStreamBasePtr;

};

