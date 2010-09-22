#include "dshowvideostream.h"

#include <mtype.h>
#include "DSUtils.h"
#include <osgDB/FileNameUtils>
#include <wmsdkidl.h>

#include <hogboxVision/VisionRegistry.h>

REGISTER_VISION_VIDEO_PLUGIN(dshow, DShowVideoStream)

//
//
//
DShowVideoStream::DShowVideoStream() 
	: hogboxVision::VideoFileStream(),
	m_lVolume(VOLUME_FULL),
    pGraph(NULL),
	m_pImageRenderer(NULL)
{
	osg::notify(osg::DEBUG_INFO) << "DShowVideoStream" << std::endl;
}


DShowVideoStream::~DShowVideoStream(void)
{
	osg::notify(osg::DEBUG_INFO) << "~DShowVideoStream" << std::endl;
	if(pControl != NULL)
	{
		pause();
	}

	if(m_pImageRenderer != NULL)
	{
		m_pImageRenderer = NULL;
	}

	DestoryGraph();	
}

DShowVideoStream::DShowVideoStream(const DShowVideoStream& image,const osg::CopyOp& copyop)
		: hogboxVision::VideoFileStream(image,copyop)
{
}

//
//Actually setup the dshow stream using cofig as the video file name
//
bool DShowVideoStream::CreateStream(const std::string& config, bool hflip, bool vflip, bool deInter)
{
	hogboxVision::VideoFileStream::CreateStream(config, hflip, vflip, deInter);

	//init direct show
	CoInitialize (NULL);
	USES_CONVERSION;

    HRESULT hr;

    // Create the graph
    pGraph.CoCreateInstance( CLSID_FilterGraph );
    if( !pGraph )
    {
		osg::notify(osg::WARN) << "DShowVideoStream: Error: Failed to create filtergraph for file '" << config << "'" << std::endl;
        return false;
    }
	
	//convert our name to wchar for use with dshow
	WCHAR wFileName[512];
	mbstowcs(wFileName, osgDB::convertFileNameToNativeStyle(config).c_str(), 512);

	//try to open video file
	pGraph->AddSourceFilter(wFileName, L"SOURCE", &pSource);
	if( !pSource )
    {
		osg::notify(osg::WARN) << "DShowVideoStream: ERROR: Failed to open video file '" << config << "', please make sure the file exists and is of a valid mediatype" << std::endl; 
        return false;
    }

	//store the index of the first video and audio streams
	int videoStreamIndex=-1;
	int audioStreamIndex=-1;


	//get the IMediaDet interface for our source so we can read framerate etc
	//and determine if the stream is valid
	CComQIPtr<IMediaDet, &IID_IMediaDet>  pMediaDet;
	if (SUCCEEDED(CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, 
                                       IID_IMediaDet, (void **)&pMediaDet)))
	{
		long streamCount = 0;
		hr = pMediaDet->put_Filter(pSource);
		if(SUCCEEDED(hr))
		{
			if(FAILED(pMediaDet->get_OutputStreams(&streamCount)))
			{
				osg::notify(osg::WARN) << "DShowVideoStream::CreateStream: ERROR: Can't open video file '" << config << "'. Failed to find any valid Streams" << std::endl;
				return false;
			}

			//should be at least one stream in the file
			if(streamCount <= 0){
				osg::notify(osg::WARN) << "DShowVideoStream::CreateStream: ERROR: Can't open video file '" << config << "'. File contains no Streams" << std::endl;
				return false;
			}
			//loop the streams and check for video or audio types
			for(long i=0; i<streamCount; i++)
			{
				//set the stream as our imediadets current
				pMediaDet->put_CurrentStream(i);
				//retirive the mediatpye
				GUID mediaType;
				if(FAILED( pMediaDet->get_StreamType(&mediaType) ))
				{
					osg::notify(osg::WARN) << "DShowVideoStream::CreateStream: ERROR: Reading video file '" << config << "'. Failed to get the mediatype of Stream '" << i << "'" << std::endl;
					return false;
				}
				//check the major type of the media
				if(mediaType == MEDIATYPE_Video)
				{
					//we found our video stream, store its index
					videoStreamIndex = i;
				}else if(mediaType == MEDIATYPE_Audio){
					//we found our audio stream, store its index
					audioStreamIndex = i;
				}
			}

			//by here we should have a video stream index, if not then there is an error
			if(videoStreamIndex == -1)
			{
				osg::notify(osg::WARN) << "DShowVideoStream::CreateStream: ERROR: Reading video file '" << config << "'. The file does not contain any Video Streams." << std::endl;
				return false;
			}

			//set the current stream to videoStreamIndex to retrive it's info
			pMediaDet->put_CurrentStream(videoStreamIndex);
			pMediaDet->get_FrameRate(&m_frameRate);

		}else{
			osg::notify(osg::WARN) << "DShowVideoStream::CreateStream: ERROR: Failed to retrive infomation on file '" << config << "'." << std::endl 
								   << "                                                                            DirectShow returned the following error '" << DSUtils::GetHResultAsString(hr) << "'" << std::endl;
			return false;
		}
	}
	pMediaDet.Release();

	// Get the output pin for the video stream
	CComPtr <IPin> pOutputVideo1Pin = this->GetOutPin(pSource, videoStreamIndex);
	// Get the output pin for the audio stream
	CComPtr <IPin> pOutputAudio1Pin = this->GetOutPin(pSource, audioStreamIndex);

	//Now create a basic audio renderer to play the audio of our stream
	if (SUCCEEDED(CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, 
                                       IID_IBaseFilter, (void **)&pAudioRenderer)))
    {
		// The audio renderer was successfully created, so add it to the graph
		pGraph->AddFilter(pAudioRenderer, L"Audio Renderer");
		//now get our basicaudio handle to control volume etc
		pAudioRenderer->QueryInterface(IID_IBasicAudio, (void **)&pBasicAudio);
		pBasicAudio->put_Volume(m_lVolume);
    }

	//create our image renderer to render dshow smaples into this image
	m_pImageRenderer = new DSOsgImageRender(this, NULL, &hr, m_hFlip,m_vFlip,m_isInter);
	if(!m_pImageRenderer)
	{
		osg::notify(osg::WARN) << "DShowVideoStream::CreateStream : ERROR: Creating Stream Renderer" << std::endl;
		return false;
	}
	//add the renderer to the graph
	pGraph->AddFilter( m_pImageRenderer, L"Renderer" );


	//now request the render of the file, which should use our attached renderer (by magic or something)
	hr = pGraph->RenderFile(wFileName, NULL);
	if( FAILED( hr ) )
    {
		osg::notify(osg::WARN) << "DShowVideoStream::CreateStream ERROR: Failed to render file source '" << config << "'" << std::endl
							   << "                                                                             DirectShow rendererer returned the following error code '" << DSUtils::GetHResultAsString(hr) << "'" << std::endl;
        return false;
    }

	//get dimensions from the renderer which will have been 
	//properly inited by pGraph->RenderFile
	this->_s = m_pImageRenderer->GetMediaWidth();
	this->_t = m_pImageRenderer->GetMediaHeight();


	//get our media control handles for the graph
	pControl = pGraph;
	pGraphFilter = pGraph;
	pGraphFilter->SetDefaultSyncSource();

    pGraph.QueryInterface(&pMediaPosition);
	pGraph.QueryInterface(&pMediaSeeking);
	if(pMediaSeeking){pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);}
    pGraph.QueryInterface(&pMediaEvent);

//	IReferenceClock* clock;
//	pControl->GetSyncSource(&clock);
//	REFERENCE_TIME time = 0;
//	clock->GetTime(&time);

	//call pause, to push out the first frame
	//to our renderer
	this->play();

	//the stream is now valid
	m_isValid=true;

	osg::notify(osg::NOTICE) << "DShowVideoStream::CreateStream: Successfully opened and rendered video file '" << config << "'." << std::endl;

	return true;
}

//
//Thread update checks the video state for any required changes
//i.e. end of file
//
void DShowVideoStream::UpdateStream()
{	
	Sleep(33);
	//this->lock();
	//this->CheckVideoState(); 
	//this->unlock();
}


void DShowVideoStream::PlayImplementation()
{
	HRESULT hr;
	if(pControl)
	{
		/*IReferenceClock* clock;
		pControl->GetSyncSource(&clock);
		REFERENCE_TIME time = 0;
		clock->GetTime(&time);*/

		hr = pControl->Run();
	}
}


/// Stop stream at current position.
void DShowVideoStream::PauseImplementation() 
{ 
	if(pControl){pControl->Pause();}
}

/// Rewind stream to beginning.
void DShowVideoStream::RewindImplementation() 
{   
	//pControl->set
	this->setReferenceTime(0.0f);
}

//
//child class to implement
//
void DShowVideoStream::QuitImplementation()
{
	this->pause();//?
	PauseImplementation();//?

	//destory the graph
	this->DestoryGraph();
}

//
//
//
void DShowVideoStream::CreateSyncClockImplementation()
{
	if(!pControl){return;}
//	IReferenceClock* clock;
//	pControl->GetSyncSource(&clock);
//	m_syncClock = new DShowSyncClock(clock);
}

//
//
//
void DShowVideoStream::SetSyncClockImplementation(hogboxVision::VideoFileSyncClock* clock)
{
	if(!pControl){return;}
	//needs to be a DShowSyncClock so try and cast
	DShowSyncClock* castClock = dynamic_cast<DShowSyncClock*>(clock);
	//if(castClock){pControl->SetSyncSource(castClock->m_dshowRefClock);}
}

//
//Have we reached the end
//
bool DShowVideoStream::HasVideoEnded() 
{	
    HRESULT hr;

	if(!pMediaPosition){return false;}

	REFTIME curTime;
	REFTIME length;
	pMediaPosition->get_CurrentPosition(&curTime);
    pMediaPosition->get_Duration(&length);

	if(curTime >= length)
	{
		hr = pMediaPosition->put_CurrentPosition(0);
		return true;
	}
	return false;
}

//
//video length and frame rate
//
double DShowVideoStream::getLength() const
{
	HRESULT hr=S_OK;
	REFTIME length;

	if(pMediaPosition)
	{
		hr = pMediaPosition->get_Duration(&length);
		if (hr == E_NOTIMPL){
			return -1.0f;
		}
	}

	return (double)length;
}

double DShowVideoStream::getFrameRate() const
{
	return m_frameRate;
}

void DShowVideoStream::setReferenceTime(double time)
{
	HRESULT hr=S_OK;
	if(pMediaSeeking)
	{
		//check time format
		GUID timeformat = TIME_FORMAT_MEDIA_TIME;
		pMediaSeeking->GetTimeFormatA(&timeformat);

		if(timeformat == TIME_FORMAT_MEDIA_TIME)
		{
			//convert to seeking time format 100 nanosecs
			LONGLONG nanosecs = time * 10000000;
			LONGLONG endnanosecs = getLength() * 10000000;
			hr = pMediaSeeking->SetPositions(&nanosecs, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
			if(hr != S_OK)
			{
				if(hr == E_INVALIDARG){
					osg::notify(osg::WARN) << "DShowVideoStream::setReferenceTime: WARN IMediaSeeking->SetPositions Invalid Args" << std::endl;
				}
				if(hr == E_NOTIMPL){
					osg::notify(osg::WARN) << "DShowVideoStream::setReferenceTime: WARN IMediaSeeking->SetPositions not implemented" << std::endl;
				}
				if(hr == S_FALSE){
					osg::notify(osg::WARN) << "DShowVideoStream::setReferenceTime: WARN IMediaSeeking->SetPositions no position change" << std::endl;
				}
				if(hr == E_POINTER){
					osg::notify(osg::WARN) << "DShowVideoStream::setReferenceTime: WARN IMediaSeeking->SetPositions Null pointer arg" << std::endl;
				}
			}

		}else if(timeformat == TIME_FORMAT_FRAME){
			
			LONGLONG frame = time;
			LONGLONG endFrame = time;
		}
	}
}

double DShowVideoStream::getReferenceTime() const
{
	if(pMediaPosition)
	{
		LONGLONG curTime;
		pMediaSeeking->GetCurrentPosition(&curTime);
		//convert to second from seeking time format
		double secs = curTime / 10000000;
		return secs;
	}
	return -1.0f;
}

//Audio
void DShowVideoStream::setVolume(float volume)
{
	if(pBasicAudio)
	{
		//scale value to -10,000 to 0 range
		float flip = 1.0f-volume;
		float sVol = ( (float)VOLUME_SILENCE * flip);
		m_lVolume = (long)sVol;
		// Set new volume
		pBasicAudio->put_Volume(m_lVolume);
	}

}

float DShowVideoStream::getVolume() const
{
	HRESULT hr=S_OK;
	if(pBasicAudio)
	{
		hr = pBasicAudio->get_Volume((long*)m_lVolume);
		if (hr == E_NOTIMPL){
			return 0.0f;
		}
	}
	return (float)m_lVolume;
}

//
// Set the flipping states of the callbacks
//
void DShowVideoStream::SetVerticalFlip(bool flip)
{
	m_vFlip = flip;
	m_pImageRenderer->SetVFlip(flip); 

}

void DShowVideoStream::SetHorizontalFlip(bool flip)
{
	m_hFlip = flip;
	m_pImageRenderer->SetHFlip(flip); 
}

//
// Deinterlace requires a reconnection as the image size will change
//
void DShowVideoStream::SetDeinterlace(bool deInter)
{
	m_isInter=deInter;
}

//
//
//
void DShowVideoStream::DestoryGraph()
{
	HRESULT hr = S_OK;
	// Shut down the graph
	if(!pControl){return;}

	pControl->Stop();

		CComPtr<IEnumFilters> pEnum = NULL;
		hr = pGraph->EnumFilters(&pEnum);
		if (SUCCEEDED(hr))
		{
			IBaseFilter* pFilter = NULL;
			while (S_OK == pEnum->Next(1, &pFilter, NULL))
			{
				FILTER_INFO filterInfo = {0};
				hr = pFilter->QueryFilterInfo(&filterInfo);
				if (SUCCEEDED(hr))
				{
					SAFE_RELEASE(filterInfo.pGraph);
					
				
						// disconnect the pins on this filter
						CComPtr<IEnumPins> pIEnumPins = NULL;
						hr = pFilter->EnumPins(&pIEnumPins);
						if (SUCCEEDED(hr))
						{
							IPin* pIPin = NULL;
							while (S_OK == pIEnumPins->Next(1, &pIPin, NULL))
							{
								pGraph->Disconnect(pIPin);
								SAFE_RELEASE(pIPin);
							}
						}
					
				}
				SAFE_RELEASE(pFilter);
			}
		}
    if (!(!pControl)) pControl.Release();
    if (!(!pMediaEvent)) pMediaEvent.Release();
    if (!(!pMediaPosition)) pMediaPosition.Release();
    if (!(!pGraph)) pGraph.Release();
   // if (!(!m_pImageRenderer)) m_pImageRenderer->Release();
	///delete m_pImageRenderer;
}


HRESULT DShowVideoStream::GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
    CComPtr< IEnumPins > pEnum;
    *ppPin = NULL;

	if(!pFilter){return -1;}

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if(FAILED(hr)) 
        return hr;

    ULONG ulFound;
    IPin *pPin;
    hr = E_FAIL;

    while(S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        PIN_DIRECTION pindir = (PIN_DIRECTION)3;

        pPin->QueryDirection(&pindir);
        if(pindir == dirrequired)
        {
            if(iNum == 0)
            {
                *ppPin = pPin;  // Return the pin's interface
                hr = S_OK;      // Found requested pin, so clear error
                break;
            }
            iNum--;
        } 

        pPin->Release();
    } 

    return hr;
}

IPin * DShowVideoStream::GetInPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin=0;
    GetPin(pFilter, PINDIR_INPUT, nPin, &pComPin);
    return pComPin;
}


IPin * DShowVideoStream::GetOutPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin=0;
    GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
    return pComPin;
}


//
//returns the graphs current ref clock time
//
IReferenceClock* DShowVideoStream::GetGraphClock()
{
	IReferenceClock* clock;
//	pControl->GetSyncSource(&clock);
	return clock;
}

void DShowVideoStream::SetGraphClock(IReferenceClock* clock)
{
//	pControl->SetSyncSource(clock);
}

HRESULT DShowVideoStream::GetGraphState()
{
//	FILTER_STATE state;
//	return pControl->GetState(INFINITE, &state);
	return S_OK;
}

