#include "DShowCaptureStream.h"
#include <mtype.h>

#include <hogboxVision/VisionRegistry.h>
REGISTER_VISION_WEBCAM_PLUGIN(dshow, DShowCaptureStream)

//
//
//
DShowCaptureStream::DShowCaptureStream() 
	: hogboxVision::WebCamStream(),
	m_osgRenderer(NULL)
{
}


DShowCaptureStream::~DShowCaptureStream(void)
{
	osg::notify(osg::WARN) << "Destruct DShowCaptureStream" << std::endl;
	pause();
	DestoryGraph();

	//image renderer is deleted by destory scene graph,
	//just need to null the pointer
	if(m_osgRenderer != NULL)
	{
		m_osgRenderer = NULL;
	}	

}

DShowCaptureStream::DShowCaptureStream(const DShowCaptureStream& image,const osg::CopyOp& copyop)
		: hogboxVision::WebCamStream(image,copyop)
{
}

//
//Actually setup the dshow capture stream using cofig as a dshowcapturestream specific config file
//See LoadSettings for info of config format
//
bool DShowCaptureStream::CreateWebCamStream(const std::string& config, int targetWidth, int targetHeight, int targetFPS,
																	bool hflip, bool vflip, bool deInter)
{
	//init direct show
	CoInitialize (NULL);
	USES_CONVERSION;
    HRESULT hr;
 
    // Create the graph and builder
    //
    InitCaptureGraphBuilder(&pGraph, &pCaptureBuilder);
	if( !pGraph )
    {
		osg::notify(osg::WARN) << "DShowCaptureStream: CreateStream: ERROR: Failed to create filter graph" << std::endl;  
        return false;
    }


	//create the webcam stream 
	if(!hogboxVision::WebCamStream::CreateWebCamStream(config, targetWidth, targetHeight, targetFPS, hflip, vflip, deInter))
	{
		return false;
	}


	this->_s = m_osgRenderer->GetMediaWidth();
	this->_t = m_osgRenderer->GetMediaHeight();  


	//start the capture source
	pControl = pGraph;
    hr = pControl->Run( );

	m_isValid=true;

	return true;
}

void DShowCaptureStream::PlayImplementation()
{
	HRESULT hr;
	if(pControl)
	{hr = pControl->Run();}
}

/// Stop stream at current position.
void DShowCaptureStream::PauseImplementation() 
{ 
	if(pControl){pControl->Stop();}
}

/// Rewind stream to beginning. (can't rewind a capturestream)
void DShowCaptureStream::RewindImplementation() 
{   
	
}

//
//child class to implement
//
void DShowCaptureStream::QuitImplementation()
{
	this->pause();//?
	PauseImplementation();//?

	//destory the graph
	this->DestoryGraph();
}

//
//perform implementation specific showing of props
//
bool DShowCaptureStream::ShowPropertiesDialogImplementation()
{
	//cast capture device to dshow
	DirectShowCaptureDevice* dshowDevice = dynamic_cast<DirectShowCaptureDevice*>(m_captureDevice.get());
	if(!dshowDevice){return false;}

	DSUtils::DisplayFilterProperties(*dshowDevice->GetFilter(), NULL);
	
	return true;
}

//
//return a list of capture devices avaliable to the implementation
//
std::vector<hogboxVision::CaptureDevicePtr> DShowCaptureStream::GetConnectedDevicesListImplementation()
{
	HRESULT hr;
	std::vector<hogboxVision::CaptureDevicePtr> deviceList;

	// first enumerate the system devices for the specifed class and filter name
	CComPtr<ICreateDevEnum> pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pSysDevEnum));

	if (SUCCEEDED(hr))
	{
		CComPtr<IEnumMoniker> pEnumCat = NULL;
		hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

		if (S_OK == hr)
		{
			IMoniker* pMoniker = NULL;
			bool Loop = true;
			while ((S_OK == pEnumCat->Next(1, &pMoniker, NULL)) && Loop)
			{
				IPropertyBag* pPropBag = NULL;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<void**>(&pPropBag));

				VARIANT varName;
				VARIANT varFilterClsid;
				//CLSID clsidFilter;

				VariantInit(&varName);
				VariantInit(&varFilterClsid);
				if (SUCCEEDED(hr))
				{
					//get friendly name
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				}

				//read uniqueID
				LPOLESTR ppszDisplaynamefull;
				char objectname[300];
				IBindCtx* pbcfull = NULL;
				if (SUCCEEDED(CreateBindCtx( 0, &pbcfull )))
				{
					if(SUCCEEDED(pMoniker->GetDisplayName(pbcfull,NULL,
															&ppszDisplaynamefull)))
					{
						wcstombs(objectname,ppszDisplaynamefull,300);
					}
				}
				pbcfull->Release();

				
				//convert name to std string
				_bstr_t bstr_t(varName.bstrVal);
				std::string deviceName(bstr_t);

				//add the device to our list
				DirectShowCaptureDevice* device = new DirectShowCaptureDevice(deviceName,std::string(objectname));
				deviceList.push_back(device);


				VariantClear(&varName);
				//VariantClear(&varFilterClsid);

				// contained within a loop, decrement the reference count
				SAFE_RELEASE(pPropBag);

				SAFE_RELEASE(pMoniker);
			}
		}
	}

	return deviceList;
}

//
//try and connect to the passed device
//the dshow graph should have already been created by createStream
//
bool DShowCaptureStream::ConnectToDeviceImplementation(hogboxVision::CaptureDevice* device)
{
	HRESULT hr;

	if(!device){return false;}
	if(!pGraph){return false;}

	//the device type needs to be DirectShowCaptureDevice
	DirectShowCaptureDevice* dshowDevice = dynamic_cast<DirectShowCaptureDevice*>(device);
	if(!dshowDevice){return false;}

	//convert name to wchar for dshow
	WCHAR captureName[512];
	mbstowcs(captureName, dshowDevice->GetDeviceName().c_str(), 512);

	//try and connect
	hr = DSUtils::AddFilter2(pGraph, CLSID_VideoInputDeviceCategory, captureName, device->GetUniqueID(), dshowDevice->GetFilter());

	if(!SUCCEEDED(hr))
	{
		if(hr == E_OUTOFMEMORY)
		{
			osg::notify(osg::WARN) << "DirectShowCaptureDevice: ConnectToDevice: ERROR Failed to connect to device '" << dshowDevice->GetDeviceName() << "'," << std::endl
								   << "                                                                                      DirectShow has returned 'Out Of Memory' error." << std::endl;
		}
		if(hr == E_FAIL)
		{
			osg::notify(osg::WARN) << "DirectShowCaptureDevice: ConnectToDevice: ERROR Failed to connect to device '" << dshowDevice->GetDeviceName() << "'," << std::endl
								   << "                                                                                      DirectShow has returned 'General Fail' error." << std::endl;
		}
		if(hr == E_POINTER)
		{
			osg::notify(osg::WARN) << "DirectShowCaptureDevice: ConnectToDevice: ERROR Failed to connect to device '" << dshowDevice->GetDeviceName() << "'," << std::endl
								   << "                                                                                      DirectShow has returned 'NULL Pointer' error." << std::endl;
		}
		if(hr == VFW_E_CERTIFICATION_FAILURE)
		{
			osg::notify(osg::WARN) << "DirectShowCaptureDevice: ConnectToDevice: ERROR Failed to connect to device '" << dshowDevice->GetDeviceName() << "'," << std::endl
								   << "                                                                                      DirectShow has returned 'Use of this filter is restricted by a software key' error." << std::endl;
		}
		if(hr == VFW_E_DUPLICATE_NAME)
		{
			osg::notify(osg::WARN) << "DirectShowCaptureDevice: ConnectToDevice: ERROR Failed to connect to device '" << dshowDevice->GetDeviceName() << "'," << std::endl
								   << "                                                                                      DirectShow has returned 'Failed to add a filter with a duplicate name' error." << std::endl;
		}
		return false;
	}

	if( !dshowDevice->GetFilter() )
	{	
		return false;
	}

	//set the pointer to the connected device
	m_captureDevice = device;

	//now try a basic connection to the device using the first format (this seems to be the only way to tell
	//if the capture pin is in use)
	//@TOM_001, Applying format twice could cause errors with rebuilding the renderer
	//needs testing
	std::vector<hogboxVision::CaptureFormatPtr> formats = m_captureDevice->GetFormats();
	if(formats.size()>0)
	{
		if(!this->ApplyFormat(formats[0]))
		{m_captureDevice = NULL; return false;}
	}

	return true;
}

//
//reconfigure the device to stream the passed format if supported
//
bool DShowCaptureStream::ApplyFormatImplementation(hogboxVision::CaptureFormat* format)
{
	HRESULT hr;

	//check we have a device
	if(!m_captureDevice){return false;}

	//graph should be stopped at this point, try and disconect the 
	
	//	
	if(!m_osgRenderer)
	{
		m_osgRenderer = new DSOsgImageRender(this, NULL, &hr, m_hFlip, m_vFlip, m_isInter);
		pGraph->AddFilter( m_osgRenderer, L"Renderer" );
	}


	//we should now be connected to valid capture device, cast it to dshowdevice
	DirectShowCaptureDevice* dshowDevice = dynamic_cast<DirectShowCaptureDevice*>(m_captureDevice.get());
	if(!dshowDevice){return false;}


	hr = DSUtils::DisconnectAllPins(pGraph);

	//try and apply the new format to the device
	if(!m_captureDevice->ApplyFormat(format))
	{return false;}

	//now try to render the device into our osg renderer

	try{

		hr = pCaptureBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, *dshowDevice->GetFilter(), NULL, m_osgRenderer);
	
	}
	catch(char * str){
		osg::notify(osg::WARN) << "WebCamStream ApplyFormat ERROR: Exception thrown while attempting to render DShow capture graph. '" << str << "'" <<std::endl;
	}
	//check errors
	if( FAILED( hr ) )
    {	
		if(hr == E_FAIL)
		{
			osg::notify(osg::WARN) << "DShowCaptureStream::ApplyFormat: ERROR rendering capture device '" << dshowDevice->GetDeviceName() << "'," << std::endl
								   << "                                                                        DirectShow returned 'Generic Faliure' error." << std::endl;
		}
		if(hr == E_INVALIDARG)
		{
			osg::notify(osg::WARN) << "DShowCaptureStream::ApplyFormat: ERROR rendering capture device '" << dshowDevice->GetDeviceName() << "'," << std::endl
								   << "                                                                        DirectShow returned 'Invalid Argument' error." << std::endl;
		}
		if(hr == E_POINTER)
		{
			osg::notify(osg::WARN) << "DShowCaptureStream::ApplyFormat: ERROR rendering capture device '" << dshowDevice->GetDeviceName() << "'," << std::endl
								   << "                                                                        DirectShow returned 'Null Pointer Argument' error." << std::endl;
		}
		if(hr == VFW_E_NOT_IN_GRAPH)
		{
			osg::notify(osg::WARN) << "DShowCaptureStream::ApplyFormat: ERROR rendering capture device '" << dshowDevice->GetDeviceName() << "'," << std::endl
								   << "                                                                        DirectShow returned 'Filter is not in Graph' error." << std::endl;
		}
		return false;
    }

	//it worked so get the new image dimensions
	this->_s = m_osgRenderer->GetMediaWidth();
	this->_t = m_osgRenderer->GetMediaHeight();  


	return true;
}


//
// Set the flipping states of the callbacks
//
void DShowCaptureStream::SetVerticalFlip(bool flip)
{
	m_osgRenderer->SetVFlip(flip); 
	hogboxVision::WebCamStream::SetVerticalFlip(flip);

}

void DShowCaptureStream::SetHorizontalFlip(bool flip)
{
	m_osgRenderer->SetHFlip(flip); 
	hogboxVision::WebCamStream::SetHorizontalFlip(flip);
}

//
// Deinterlace requires a reconnection as the image size will change
//
void DShowCaptureStream::SetDeinterlace(bool deInter)
{
	hogboxVision::WebCamStream::SetDeinterlace(deInter);
}

void DShowCaptureStream::UpdateStream()
{	
	Sleep(33);
	//this->lock();
	//this->unlock();
}

void DShowCaptureStream::DestoryGraph()
{
	HRESULT hr = S_OK;

	if (pGraph)
	{
		if(pControl){pControl->Stop();}

		WCHAR pName[512];
		// convert file name to wide char
		if(m_captureDevice)
		{
			mbstowcs(pName, m_captureDevice->GetDeviceName().c_str(), 512);
		}else{
			mbstowcs(pName, "", 1);
		}


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
					
					if ((NULL == wcsstr(filterInfo.achName, pName)) && (NULL == wcsstr(filterInfo.achName, L"SampleGrabber")) && (NULL == wcsstr(filterInfo.achName, L"Renderer")))
					{
						hr = pGraph->RemoveFilter(pFilter);
						if (SUCCEEDED(hr))
						{
							hr = pEnum->Reset();
						}
					}
					else
					{
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
				}
				SAFE_RELEASE(pFilter);
			}
		}
	}

    if (!(!pControl)) pControl.Release();
	if (!(!pGraph)) pCaptureBuilder.Release();
    if (!(!pGraph)) pGraph.Release();

}
HRESULT DShowCaptureStream::GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
    CComPtr< IEnumPins > pEnum;
    *ppPin = NULL;

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

IPin * DShowCaptureStream::GetInPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin=0;
    GetPin(pFilter, PINDIR_INPUT, nPin, &pComPin);
    return pComPin;
}


IPin * DShowCaptureStream::GetOutPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin=0;
    GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
    return pComPin;
}

HRESULT DShowCaptureStream::InitCaptureGraphBuilder(
								IGraphBuilder **ppGraph,  // Receives the pointer.
								ICaptureGraphBuilder2 **ppBuild  // Receives the pointer.
								)
{
    if (!ppGraph || !ppBuild)
    {
        return E_POINTER;
    }
    IGraphBuilder *pGraph = NULL;
    ICaptureGraphBuilder2 *pBuild = NULL;

    // Create the Capture Graph Builder.
    HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, 
        CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild );
    if (SUCCEEDED(hr))
    {
        // Create the Filter Graph Manager.
        hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder, (void**)&pGraph);
        if (SUCCEEDED(hr))
        {
            // Initialize the Capture Graph Builder.
            pBuild->SetFiltergraph(pGraph);

            // Return both interface pointers to the caller.
            *ppBuild = pBuild;
            *ppGraph = pGraph; // The caller must release both interfaces.
            return S_OK;
        }
        else
        {
            pBuild->Release();
        }
    }
    return hr; // Failed
}
