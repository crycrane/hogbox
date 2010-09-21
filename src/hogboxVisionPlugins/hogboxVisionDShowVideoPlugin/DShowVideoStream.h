#pragma once
#include <hogboxVision/videofilestream.h>

#include <windows.h>
#include <stdio.h>
#include <atlbase.h>
#include <qedit.h>

#include <comdef.h>
#include <iostream>
#include <vector>

#include "DSOsgImageRender.h"

#define VOLUME_FULL     0L
#define VOLUME_SILENCE  -10000L

//dshow implementation of syncclock
class DShowSyncClock : public hogboxVision::VideoFileSyncClock
{
public:
	DShowSyncClock(IReferenceClock* clock=NULL){m_dshowRefClock=clock;}

	IReferenceClock* m_dshowRefClock;

protected:
	virtual ~DShowSyncClock(){m_dshowRefClock=NULL;}
};

//
// DShowVideoStream uses MicroSoft direct show and windows media sdk
// to allow the opening and playback of directshow supported video files
// including WMV
//
class DShowVideoStream : public hogboxVision::VideoFileStream
{
public:
	DShowVideoStream(void);

    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
	DShowVideoStream(const DShowVideoStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Stream(hogboxVision, DShowVideoStream);


	//
	//Actually setup the dshow stream using cofig as the video file name
	virtual bool CreateStream(const std::string& config, bool hflip = false, bool vflip = false, bool deInter = false);


	//
	//Implement stream specific technique for flipping and deinterlacing of stream
	//The DShowVideoStream used the DSOsgImage renderer to perform the pixel manipulations

	//
	//Set the DSOsgImageRenderer to Vertical Flip
	virtual void SetVerticalFlip(bool flip);
	
	//
	//Set the DSOsgImageRenderer to Horizontal flip
	virtual void SetHorizontalFlip(bool flip);

	//Set the DSOsgImageRenderer to deinterlace the stream
	virtual void SetDeinterlace(bool deInter);


	virtual bool HasVideoEnded();


	//video length and frame rate
    virtual double getLength() const;
    virtual double getFrameRate() const;

    virtual void setReferenceTime(double);
    virtual double getReferenceTime() const;

	//audio
    virtual void setVolume(float volume);
    virtual float getVolume() const;


protected:

	virtual ~DShowVideoStream(void);

	//Update stream from thread
	void UpdateStream();


	//child class to implement
	virtual void PlayImplementation();

	//child class to implement
	virtual void PauseImplementation();

	//child class to implement
	virtual void RewindImplementation();

	//child class to implement
	virtual void QuitImplementation();

	//syncing stuff

	//returns the graphs current ref clock time
	IReferenceClock* GetGraphClock();
	void SetGraphClock(IReferenceClock* clock);

	HRESULT GetGraphState();

	virtual void CreateSyncClockImplementation();
	virtual void SetSyncClockImplementation(hogboxVision::VideoFileSyncClock* clock);


	//
	void DestoryGraph();

	//direct show helpers
	HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin);

	IPin * GetInPin( IBaseFilter * pFilter, int nPin );
	IPin * GetOutPin( IBaseFilter * pFilter, int nPin );


protected:

	//The direct show source (imagine this as the file)
    CComPtr< IBaseFilter >    pSource;

	//Direct show audio rendering filter to allow playback of audio contained in file
	CComPtr< IBaseFilter >	  pAudioRenderer;
	//
	CComPtr< IBasicAudio >	  pBasicAudio;
	
	//The current volume/gain of the Audio
	LONG					  m_lVolume;

	//Our main direct show graph builder, used to automate alot of the filter linking
	CComPtr< IGraphBuilder >  pGraph;
    CComQIPtr< IFilterGraph, &IID_IFilterGraph > pGraphFilter;

	//A DSOsgImageRender used to get the dshow images into our osg image buffer
	DSOsgImageRender*		  m_pImageRenderer;

	//
	CComQIPtr< IMediaControl, &IID_IMediaControl > pControl;
	CComPtr<IMediaPosition> pMediaPosition;  //Media Position used for length etc
	CComPtr<IMediaSeeking > pMediaSeeking;	 //used for selecting specific frames
	CComPtr<IMediaEvent>    pMediaEvent;          // Media Event

};
