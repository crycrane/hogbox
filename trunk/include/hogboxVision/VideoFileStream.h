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
//Temp wrapper,
//We will need a way of passing around whatever interface allows us
//to sync videos properly. DShow has the IReferenceClock interface
//not sure what others will have so we will use this class to wrap them
//for passing around without worrying about implementation specifics
class VideoFileSyncClock : public osg::Referenced
{
public:
	VideoFileSyncClock(){}
protected:
	virtual ~VideoFileSyncClock(){}
};

//
//
class HOGBOXVIS_EXPORT VideoFileStream : public VideoStream
{
public:
	VideoFileStream() : VideoStream(),
						_syncClock(NULL), 
						_isSynced(false)
	{
	}
	
    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
	VideoFileStream(const VideoFileStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: VideoStream(image, copyop),
		_syncClock(image._syncClock),
		_isSynced(image._isSynced)
	{
	}

	META_Stream(hogboxVision, VideoFileStream);


	//
	//At mo VideoFileStream doesn't do any specific create stuff
	virtual bool CreateStream(const std::string& config, bool hflip = false, bool vflip = false, bool deinter = false)
	{
		return VideoStream::CreateStream(config,hflip,vflip,deinter);
	}


	//
	//Get sync clock, if it hasn't got a sync clock then one is allocated
	//via CreateSyncClockImplementation
	VideoFileSyncClock* GetSyncClock(){
		if(!_syncClock){CreateSyncClockImplementation();}
		return _syncClock;
	}

	//Set our clock pointer then call SetSyncClockImplementation
	//to do implementation specific syncing stuff
	void SetSyncClock(VideoFileSyncClock* clock){
		_syncClock = clock;
		SetSyncClockImplementation(_syncClock);
	}

protected:

	virtual ~VideoFileStream(void){
		_syncClock=NULL;
	}

	//Should be pure virtual but inheriting from osg::Object require cloneType
	//which cause can't init abstract bullshit
	virtual void CreateSyncClockImplementation(){}
	virtual void SetSyncClockImplementation(VideoFileSyncClock* clock){}

protected:

	//The syncing clock being used
	osg::ref_ptr<VideoFileSyncClock> _syncClock;

	//syncing 
	bool _isSynced;
};

typedef osg::ref_ptr<VideoFileStream> VideoFileStreamPtr;

};
