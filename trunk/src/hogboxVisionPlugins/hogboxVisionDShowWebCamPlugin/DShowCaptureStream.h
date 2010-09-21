#pragma once

#include <hogboxVision/webcamstream.h>

#include "DSUtils.h"
#include "DSOsgImageRender.h"

#include <streams.h>
#undef lstrlenW 


#include <stdio.h>
#include <atlbase.h>
#include <qedit.h>

#include <comdef.h>
#include <iostream>
#include <sstream>
#include <vector>

#include <string>
#include <list>
#include <dvdmedia.h>

//
//DirectShow implementation of captureformat
class DShowCaptureFormat : public hogboxVision::CaptureFormat
{
public:
	DShowCaptureFormat(AM_MEDIA_TYPE* format, VIDEO_STREAM_CONFIG_CAPS vscc) 
		: hogboxVision::CaptureFormat()
	{
		m_isValid = false;

		p_formatMediaType = format;
		m_formatConfig = vscc;

		VIDEOINFOHEADER* pvih = NULL;
		
		pvih = (VIDEOINFOHEADER*)p_formatMediaType->pbFormat;

		//store basic params
		m_width = pvih->bmiHeader.biWidth;
		m_height = pvih->bmiHeader.biHeight;
		m_bitRate = pvih->bmiHeader.biBitCount;
		m_fps = (float)UNITS / pvih->AvgTimePerFrame;;

		//store min and max frame rates
		if(m_formatConfig.MaxFrameInterval == 0 || m_formatConfig.MaxFrameInterval == 0)
		{
			m_formatConfig.MinFrameInterval = pvih->AvgTimePerFrame;
			m_formatConfig.MaxFrameInterval = pvih->AvgTimePerFrame;
		}

		m_minFps = (float)UNITS / m_formatConfig.MaxFrameInterval;
		m_maxFps = (float)UNITS / m_formatConfig.MinFrameInterval;

		//if we have any zero params return before flagging the format as valid
		if(m_width <=0 || m_height <=0 || m_fps <=0)
		{m_isValid = false; return;}

		//create friendly description of the format
		std::ostringstream oss(std::ostringstream::out);
		
		if(m_minFps == 0 || m_maxFps == 0)
		{
			oss << m_width << "x" << m_height << " " << (int)m_minFps << "-" << (int)m_maxFps << "-fps " << m_bitRate << "-bit";
		}else{
			oss << m_width << "x" << m_height << " " << (int)m_fps << "-fps " << m_bitRate << "-bit";
		}
		m_formatDescription = oss.str();

		m_isValid = true;
	}


	//
	//some implementations allow you to change the fps to
	//to a value between min and max fps. This must
	//be done before the format is applied. Will clamp 
	//any out of range values
	virtual bool SetFormatFps(float fps)
	{
		if(!p_formatMediaType){return false;}

		//clamp range
		if(fps < m_minFps){fps = m_minFps;}
		if(fps > m_maxFps){fps = m_maxFps;}

		//apply it to the dshow AM_MEDIA_TYPE
		VIDEOINFOHEADER* pvih = NULL;
		pvih = (VIDEOINFOHEADER*)p_formatMediaType->pbFormat;

		pvih->AvgTimePerFrame = (1.0f/fps)*UNITS;
		m_fps = fps;

		return true;
	}

	//DShow specific returns
	AM_MEDIA_TYPE* GetFormat(){return p_formatMediaType;}

	VIDEO_STREAM_CONFIG_CAPS GetFormatConfig(){return m_formatConfig;}

	
protected:

	virtual ~DShowCaptureFormat(){
		if(p_formatMediaType){
			//DeleteMediaType(p_formatMediaType);
		}
	}

protected:

	//the format itself
	AM_MEDIA_TYPE* p_formatMediaType;

	//the ways the format can be changed
	VIDEO_STREAM_CONFIG_CAPS m_formatConfig;

};

typedef osg::ref_ptr<DShowCaptureFormat> DShowCaptureFormatPtr;

//
//
//
class DirectShowCaptureDevice : public hogboxVision::CaptureDevice
{
public:
	//base contructor expects a device name, which
	//can be used to identify the device
	DirectShowCaptureDevice(const std::string& deviceName, const std::string& uniqueID) 
		: hogboxVision::CaptureDevice(deviceName, uniqueID),
		deviceFilter(NULL)
	{
	}

	//return the dshow filter to represent this device
	IBaseFilter**  GetFilter(){return &deviceFilter.p;}
	IBaseFilter*  GetFilterSinglePtr(){return deviceFilter.p;}

	//
	//fill the m_formats list with data from our connected
	//device filter
	virtual bool CreateFormatsListImplementation()
	{
		HRESULT hr;

		if (deviceFilter)
		{
			// locate the video capture pin and QI for stream control
			CComPtr<IAMStreamConfig> pISC = NULL;
			hr = DSUtils::FindPinInterface(deviceFilter, &MEDIATYPE_Video, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
			if (SUCCEEDED(hr))
			{
				// get the current video format
				AM_MEDIA_TYPE* pamtCurrent = NULL;
				hr = pISC->GetFormat(&pamtCurrent);
				if (SUCCEEDED(hr))
				{
					BITMAPINFOHEADER* pbmihCurrent = CUtils::GetBMIHeader(pamtCurrent);
					REFERENCE_TIME rtAvgTimePerFrame = CUtils::GetAvgTimePerFrame(pamtCurrent);
					

					// loop through all the capabilities (video formats) and populate the control
					int count, size;
					hr = pISC->GetNumberOfCapabilities(&count, &size);
					if (SUCCEEDED(hr))
					{
						//check ut's the right type
						if (sizeof(VIDEO_STREAM_CONFIG_CAPS) == size)
						{
							AM_MEDIA_TYPE* pmt = NULL;
							VIDEO_STREAM_CONFIG_CAPS vscc;
							VIDEOINFOHEADER* pvih = NULL;

							for (int index=0; index<count; ++index)
							{
								hr = pISC->GetStreamCaps(index, &pmt, reinterpret_cast<BYTE*>(&vscc));
								if (SUCCEEDED(hr))
								{
									//create a new format and add to our formats list
									DShowCaptureFormatPtr format = new DShowCaptureFormat(pmt, vscc);
									if(format)
									{
										//add the format to our list only if the format is valid
										if(format->isValid()){
											m_formats.push_back(format.get());
										}else{
											DeleteMediaType(pmt);
										}
									}else{
										DeleteMediaType(pmt);
									}
								}
							}
						}
						else
						{
							//NO FORMATS ERROR
							return false;
						}
					}
					DeleteMediaType(pamtCurrent);
				}
			}
		}
		else
		{
			hr = E_POINTER;
			return false;
		}
		return true;
	}

	//apply the format, making it the connected format
	virtual bool ApplyFormat(hogboxVision::CaptureFormat* format)
	{
		//cast the format to dshow format
		DShowCaptureFormat* dshowFormat = dynamic_cast<DShowCaptureFormat*>(format);
		if(!dshowFormat){return false;}

		HRESULT hr;

		//connect to the format
		CComPtr<IAMStreamConfig> pISC = NULL;
		hr = DSUtils::FindPinInterface(deviceFilter, &MEDIATYPE_Video, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
		if (SUCCEEDED(hr))
		{
			hr = pISC->SetFormat(dshowFormat->GetFormat());
			if (SUCCEEDED(hr))
			{
				//sucsess applying a format, now use get format to find the final format
				AM_MEDIA_TYPE* pmt = NULL;
				VIDEO_STREAM_CONFIG_CAPS vscc;
				vscc.MinFrameInterval = 0;
				vscc.MaxFrameInterval = 0;

				pISC->GetFormat(&pmt);
				m_connectedFormat = new DShowCaptureFormat(pmt, vscc);
				
				return true;
			}else{
				if(hr == E_OUTOFMEMORY)
				{
					osg::notify(osg::WARN) << "DirectShowCaptureDevice: ApplyFormat: ERROR Failed to apply format '" << dshowFormat->GetFormatDescription() << "', on device '" << GetDeviceName() << "'," << std::endl
										   << "                                                                              DirectShow has returned 'Out Of Memory' error." << std::endl;
				}
				if(hr == VFW_E_NOT_CONNECTED)
				{
					osg::notify(osg::WARN) << "DirectShowCaptureDevice: ApplyFormat: ERROR Failed to apply format '" << dshowFormat->GetFormatDescription() << "', on device '" << GetDeviceName() << "'," << std::endl
										   << "                                                                              DirectShow has returned 'Input Pin Not Connected' error." << std::endl;
				}
				if(hr == VFW_E_INVALIDMEDIATYPE)
				{
					osg::notify(osg::WARN) << "DirectShowCaptureDevice: ApplyFormat: ERROR Failed to apply format '" << dshowFormat->GetFormatDescription() << "', on device '" << GetDeviceName() << "'," << std::endl
										   << "                                                                              DirectShow has returned 'Invalid MediaType' error." << std::endl;
				}
				if(hr == VFW_E_NOT_STOPPED || hr == VFW_E_WRONG_STATE)
				{
					osg::notify(osg::WARN) << "DirectShowCaptureDevice: ApplyFormat: ERROR Failed to apply format '" << dshowFormat->GetFormatDescription() << "', on device '" << GetDeviceName() << "'," << std::endl
										   << "                                                                              DirectShow has returned 'Graph Not Stopped/Invalid State' error." << std::endl;
				}
				if(hr == E_POINTER)
				{
					osg::notify(osg::WARN) << "DirectShowCaptureDevice: ApplyFormat: ERROR Failed to apply format '" << dshowFormat->GetFormatDescription() << "', on device '" << GetDeviceName() << "'," << std::endl
										   << "                                                                              DirectShow has returned 'Invalid Pointer' error." << std::endl;
				}
				return false;
			}
		}
		osg::notify(osg::WARN) << "DirectShowCaptureDevice: ApplyFormat: ERROR Failed to apply format '" << dshowFormat->GetFormatDescription() << "', on device '" << GetDeviceName() << "'," << std::endl
							   << "                                                                              DirectShow has failed to find a valid output pin" << std::endl;
		return false;
	}


protected:
	virtual ~DirectShowCaptureDevice(){
		if(deviceFilter){deviceFilter.Release();deviceFilter=NULL;}
	}

	//this is the dshow sourc filter, representing
	//the device, it needs to be connected to a show graph
	CComPtr< IBaseFilter >    deviceFilter;

};

//
//DShowCaptureStream
//
//Used to create an osg image stream of a dshow capture source, i.e. a webcam
//A DSCaptureStream config file is used to supply a device name and format
//
class DShowCaptureStream : public hogboxVision::WebCamStream
{
public:
	DShowCaptureStream(void);
	
    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
	DShowCaptureStream(const DShowCaptureStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Stream(hogboxVision, DShowCaptureStream);

	//
	//Actually setup the capture stream using config as the device name, and target width,height and fps to
	//try and find the nearest matching format to connect to
	virtual bool CreateWebCamStream(const std::string& config, int targetWidth=800, int targetHeight=600, int targetFPS=60,
											bool hflip = false, bool vflip = false, bool deInter = false);


	virtual void SetVerticalFlip(bool flip);
	virtual void SetHorizontalFlip(bool flip);
	virtual void SetDeinterlace(bool deInter);

	//TEMP@tom
	//
	//Inherit from stream
	virtual void UpdateStream();

protected:

	virtual ~DShowCaptureStream(void);

	//Inherit from streambase
	//virtual void UpdateStream();


	//child class to implement
	virtual void PlayImplementation();

	//child class to implement
	virtual void PauseImplementation();

	//child class to implement
	virtual void RewindImplementation();

	//child class to implement
	virtual void QuitImplementation();


	//perform implementation specific showing of props
	virtual bool ShowWebCamPropertiesImplementation();

	//return a list of capture devices avaliable to the implementation
	virtual std::vector<hogboxVision::CaptureDevicePtr> GetConnectedDevicesListImplementation();

	//try and connect to the passed device
	virtual bool ConnectToDeviceImplementation(hogboxVision::CaptureDevice* device);

	//reconfigure the device to stream the passed format if supported
	virtual bool ApplyFormatImplementation(hogboxVision::CaptureFormat* format);


	//direct show helpers
	
	void DestoryGraph();

	HRESULT InitCaptureGraphBuilder(IGraphBuilder **ppGraph, ICaptureGraphBuilder2 **ppBuild);

	HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin);
	IPin * GetInPin( IBaseFilter * pFilter, int nPin );
	IPin * GetOutPin( IBaseFilter * pFilter, int nPin );

protected:

	CComQIPtr< IMediaControl, &IID_IMediaControl > pControl;
    CComPtr< IGraphBuilder >  pGraph;
	CComPtr< ICaptureGraphBuilder2 > pCaptureBuilder;
	DSOsgImageRender*	  m_osgRenderer;		

};
