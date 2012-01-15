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

#include <hogboxVision/Export.h>

#include <osg/ImageStream>
#include <osg/notify>


namespace hogboxVision {


//
//Macro fro defining the osg base funcs
#define META_Stream(libraryname, classname) \
	virtual osg::Object* cloneType() const { return new classname(); } \
	virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new classname(*this,copyop); } \
	virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const classname*>(obj)!=0; } \
	virtual const char* libraryName() const { return #libraryname; } \
	virtual const char* className() const { return #classname; }


//
//VideoStream
//
// Base class for running video streams in osg
// Inherits from osg image so the stream can be used as a texture in osg
// also inherits from Thread so it can be auto updated if required
// Call create stream to setup a stream rendering into an osg image
// Call play to run the stream from its current position
// Call pause to stop at current position
// Call rewind to return to the first frame of a stream
//
class HOGBOXVIS_EXPORT VideoStream : public osg::ImageStream
{
public:
	VideoStream();
	
    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
	VideoStream(const VideoStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Stream(hogboxVision, VideoStream);

	//
	//Actually setup the stream using file name as a video or capture source config file
	//Also request the fliping and deinterlacing modes for the stream
	//Returns true is the stream is setup correctly and ready to stream/play
	virtual bool CreateStream(const std::string& config, bool hflip = false, bool vflip = false, bool deinter = false);

	bool isValid(){return m_isValid;}

	//
	//Play stream from current position only if not already playing
	void play(){ 
		if(_status != PLAYING )
		{
			osg::ImageStream::play();
			PlayImplementation();
		}
	}
	//
	//Stop stream at current position only if not already stopped
	void pause(){
		if(_status != PAUSED && _status != INVALID)
		{
			osg::ImageStream::pause();
			PauseImplementation();
		}
	}
	//
	//Rewind stream to beginning, calls RewindImplementation 
	//when thread is ready
	void rewind() {
		RewindImplementation();
	}

	//
	//Quit the thread if it is running, calls QuitImplementation 
	//when thread is ready
	void quit() { QuitImplementation(); }
	
	//
	//Request a stream to be fliped vertically
	virtual void SetVerticalFlip(bool flip){m_vFlip = flip;}
	bool GetVerticalFlip(){return m_vFlip;}

	//
	//Request a stream to be flipped horizontally
	virtual void SetHorizontalFlip(bool flip){m_hFlip=flip;}
	bool GetHorizontalFlip(){return m_hFlip;}

	//
	//Request a stream have deinterlacing applied, i.e. remo
	virtual void SetDeinterlace(bool deInter){m_isInter=deInter;}
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

	virtual ~VideoStream(void);

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

typedef osg::ref_ptr<VideoStream> VideoStreamPtr;

};

