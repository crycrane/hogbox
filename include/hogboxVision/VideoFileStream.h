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
						m_syncClock(NULL), 
						m_isSynced(false)
	{
	}
	
    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
	VideoFileStream(const VideoFileStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: VideoStream(image, copyop),
		m_syncClock(image.m_syncClock),
		m_isSynced(image.m_isSynced)
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
		if(!m_syncClock){CreateSyncClockImplementation();}
		return m_syncClock;
	}

	//Set our clock pointer then call SetSyncClockImplementation
	//to do implementation specific syncing stuff
	void SetSyncClock(VideoFileSyncClock* clock){
		m_syncClock = clock;
		SetSyncClockImplementation(m_syncClock);
	}

protected:

	virtual ~VideoFileStream(void){
		m_syncClock=NULL;
	}

	//Should be pure virtual but inheriting from osg::Object require cloneType
	//which cause can't init abstract bullshit
	virtual void CreateSyncClockImplementation(){}
	virtual void SetSyncClockImplementation(VideoFileSyncClock* clock){}

protected:

	//The syncing clock being used
	osg::ref_ptr<VideoFileSyncClock> m_syncClock;

	//syncing 
	bool m_isSynced;
};

typedef osg::ref_ptr<VideoFileStream> VideoFileStreamPtr;

};
