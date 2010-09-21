#include "dsutils.h"

/////

//-----------------------------------------------------------------------------
// DSUtils - Directshow utility class
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// AddFilter
// Attempts to locate a filter of a given class ID and name	and add it to the graph
HRESULT DSUtils::AddFilter(IGraphBuilder* pGraph, const GUID &clsid, LPCWSTR pName, IBaseFilter** ppFilter)
{
	HRESULT hr = S_OK;
	
	if (pGraph && pName && ppFilter)
	{
		*ppFilter = NULL;
		IBaseFilter* pFilter = NULL;
		hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pFilter));
		if (SUCCEEDED(hr))
		{
			hr = pGraph->AddFilter(pFilter, pName);
			if (SUCCEEDED(hr))
			{
				*ppFilter = pFilter;
			}
			else
			{
				SAFE_RELEASE(pFilter);
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// AddFilter2
// Attempts to locate a filter of a given class category and name
HRESULT DSUtils::AddFilter2(IGraphBuilder* pGraph, const GUID &clsid, LPCWSTR pName, const std::string& uniqueID, IBaseFilter** ppFilter)
{
	HRESULT hr = S_OK;
	
	if (pGraph && pName && ppFilter)
	{
		// first enumerate the system devices for the specifed class and filter name
		CComPtr<ICreateDevEnum> pSysDevEnum = NULL;
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pSysDevEnum));

		if (SUCCEEDED(hr))
		{
			CComPtr<IEnumMoniker> pEnumCat = NULL;
			hr = pSysDevEnum->CreateClassEnumerator(clsid, &pEnumCat, 0);

			if (S_OK == hr)
			{
				IMoniker* pMoniker = NULL;
				bool Loop = true;
				while ((S_OK == pEnumCat->Next(1, &pMoniker, NULL)) && Loop)
				{
					IPropertyBag* pPropBag = NULL;
					hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<void**>(&pPropBag));

					if (SUCCEEDED(hr))
					{
						//if we are passed a uniqueID use that to search, else use name
						if(uniqueID.empty())
						{
							VARIANT varName;
							VariantInit(&varName);
							hr = pPropBag->Read(L"FriendlyName", &varName, 0);
							if (SUCCEEDED(hr))
							{
								if (0 == wcscmp(varName.bstrVal, pName))
								{
									hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, reinterpret_cast<void**>(ppFilter));
									Loop = false;
								}
							}

							VariantClear(&varName);
						}else{

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

							//compare the passed uniqueID to the deviceGUID
							if (uniqueID.compare(std::string(objectname)) == 0)
							{
								hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, reinterpret_cast<void**>(ppFilter));
								Loop = false;
							}
						}
						
						// contained within a loop, decrement the reference count
						SAFE_RELEASE(pPropBag);
					}
					SAFE_RELEASE(pMoniker);
				}
			}
		}
		
		// if a filter has been located add it to the graph
		if (*ppFilter)
		{
			hr = pGraph->AddFilter(reinterpret_cast<IBaseFilter*>(*ppFilter), pName);
		}
		else
		{
			hr = E_FAIL;
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-------------------------------------------------------------------------------------------------
// GetUnconnectedPin
// Attemptes to locate an unconnected pin on filter
HRESULT DSUtils::GetUnconnectedPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin)
{
	HRESULT hr = S_OK;

	if (pFilter && ppPin)
	{
		CComPtr<IEnumPins> pEnum = NULL;
		IPin* pPin = NULL;

		hr = pFilter->EnumPins(&pEnum);
		if (SUCCEEDED(hr))
		{
			while (pEnum->Next(1, &pPin, NULL) == S_OK)
			{
				PIN_DIRECTION ThisPinDir;
				pPin->QueryDirection(&ThisPinDir);
				if (ThisPinDir == PinDir)
				{
					IPin* pPinTemp = NULL;

					hr = pPin->ConnectedTo(&pPinTemp);
					if (SUCCEEDED(hr))
					{
						SAFE_RELEASE(pPinTemp);
					}
					else
					{
						// unconnected, return this pin
						*ppPin = pPin;
						hr = S_OK;
						break;
					}
				}
				SAFE_RELEASE(pPin);
			}
		}

		if (NULL == *ppPin)
		{
			// failed to find an unconnected pin
			hr = E_FAIL;
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// GetPin
// Find the pin of the specified name on the given filter
// This method leaves an outstanding reference on the pin if successful
HRESULT DSUtils::GetPin(IBaseFilter* pFilter, LPCWSTR pName, IPin** ppPin)
{
	HRESULT hr = S_OK;

	if (pFilter && pName && ppPin)
	{
		CComPtr<IEnumPins> pIEnumPins = NULL;
		hr = pFilter->EnumPins(&pIEnumPins);
		if (SUCCEEDED(hr))
		{
			IPin* pIPin = NULL;
			while (S_OK == pIEnumPins->Next(1, &pIPin, NULL))
			{
				PIN_INFO info = {0};
				hr = pIPin->QueryPinInfo(&info);
				if (SUCCEEDED(hr))
				{
					SAFE_RELEASE(info.pFilter);

					if (0 == wcsncmp(info.achName, pName, wcslen(pName)))
					{
						// matched the pin category
						*ppPin = pIPin;
						break;
					}
				}
				SAFE_RELEASE(pIPin);
			}
		}

		if (NULL == *ppPin)
		{
			// failed to find the named pin
			hr = E_FAIL;
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// GetPin
// Find the pin of the specified format type on the given filter
// This method leaves an outstanding reference on the pin if successful
HRESULT DSUtils::GetPin(IBaseFilter* pFilter, const GUID* pFormat, PIN_DIRECTION PinDir, IPin** ppPin)
{
	HRESULT hr = S_OK;

	if (pFilter && pFormat && ppPin)
	{
		CComPtr<IEnumPins> pIEnumPins = NULL;
		hr = pFilter->EnumPins(&pIEnumPins);
		if (SUCCEEDED(hr))
		{
			// find the pin with the specified format
			IPin* pIPin = NULL;
			while (S_OK == pIEnumPins->Next(1, &pIPin, NULL))
			{
				// match the pin direction
				PIN_DIRECTION pinDir;
				pIPin->QueryDirection(&pinDir);
				if (pinDir == PinDir)
				{
					// match pin direction check the first media type returned from the upstream pin
					CComPtr<IEnumMediaTypes> pIEnumMT = NULL;
					hr = pIPin->EnumMediaTypes(&pIEnumMT);
					if (SUCCEEDED(hr))
					{
						AM_MEDIA_TYPE* pmt = NULL;
						hr = pIEnumMT->Next(1, &pmt, NULL);
						if (S_OK == hr)
						{
							if (pmt->majortype == *pFormat)
							{
								// found the pin with the specified format
								*ppPin = pIPin;
								DeleteMediaType(pmt);
								break;
							}
							else
							{
								DeleteMediaType(pmt);
							}
						}
					}
				}
				SAFE_RELEASE(pIPin);
			}

			if (NULL == *ppPin)
			{
				// failed to find the named pin
				hr = E_FAIL;
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// ConnectFiltersNamedPin
// Connects two filters using the pin names, if no name is supplied the first
// unconnected pin is used
HRESULT DSUtils::ConnectFilters(IGraphBuilder* pGraph, IBaseFilter* pUpstream, LPCWSTR pUpstreamPinName, IBaseFilter* pDownstream, LPCWSTR pDownstreamPinName)
{
	HRESULT hr = S_OK;
	
	if (pUpstream && pDownstream)
	{
		// get the upstream output pin
		CComPtr<IPin> pIPinOutput = NULL;
		if (pUpstreamPinName)
		{
			hr = GetPin(pUpstream, pUpstreamPinName, &pIPinOutput);
		}
		else
		{
			hr = GetUnconnectedPin(pUpstream, PINDIR_OUTPUT, &pIPinOutput);
		}

		if (SUCCEEDED(hr))
		{
			// get the downstream input pin
			CComPtr<IPin> pIPinInput = NULL;
			if (pDownstreamPinName)
			{
				hr = GetPin(pDownstream, pDownstreamPinName, &pIPinInput);
			}
			else
			{
				hr = GetUnconnectedPin(pDownstream, PINDIR_INPUT, &pIPinInput);
			}
			
			if (SUCCEEDED(hr))
			{
				// connect the pins
				hr = pGraph->Connect(pIPinOutput, pIPinInput);
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// ConnectFilters
// Connects two filters by finding a pin on the upstream filter with the specified
// major format type, e.g. For connecting an audio pin to a downstream filter
HRESULT DSUtils::ConnectFilters(IGraphBuilder* pGraph, IBaseFilter* pUpstream, IBaseFilter* pDownstream, const GUID* pFormat)
{
	HRESULT hr = S_OK;
	
	if (pUpstream && pDownstream && pFormat)
	{
		// find the upstream output pin with the specified format
		CComPtr<IPin> pIPinOutput = NULL;
		hr = GetPin(pUpstream, pFormat, PINDIR_OUTPUT, &pIPinOutput);
		if (SUCCEEDED(hr))
		{
			// get the downstream input pin
			CComPtr<IPin> pIPinInput = NULL;
			hr = GetPin(pDownstream, pFormat, PINDIR_INPUT, &pIPinInput);
			
			if (SUCCEEDED(hr))
			{
				// connect the pins
				hr = pGraph->Connect(pIPinOutput, pIPinInput);
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// RenderFilter
// Renders the named output pin of the filter, or the first unconnected output if 
// no name is provided
HRESULT DSUtils::RenderFilter(IGraphBuilder* pGraph, IBaseFilter* pUpstream, LPCWSTR pUpstreamPinName)
{
	HRESULT hr = S_OK;

	if (pUpstream)
	{
		CComPtr<IPin> pIPinOutput = NULL;
		if (pUpstreamPinName)
		{
			hr = GetPin(pUpstream, pUpstreamPinName, &pIPinOutput);
		}
		else
		{
			hr = GetUnconnectedPin(pUpstream, PINDIR_OUTPUT, &pIPinOutput);
		}
		
		if (SUCCEEDED(hr))
		{
			hr = pGraph->Render(pIPinOutput);
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// DisconnectPins
// Disconnect all the pins of the specified filter
HRESULT DSUtils::DisconnectPins(IGraphBuilder* pGraph, IBaseFilter* pFilter)
{
	HRESULT hr = S_OK;

	if (pGraph && pFilter)
	{
		CComPtr<IEnumPins> pIEnumPins = NULL;
		hr = pFilter->EnumPins(&pIEnumPins);
		if (SUCCEEDED(hr))
		{
			IPin* pIPin = NULL;
			while (S_OK == pIEnumPins->Next(1, &pIPin, NULL))
			{
				IPin* pIPinConnection = NULL;
				if (S_OK == pIPin->ConnectedTo(&pIPinConnection))
				{
					// to disconnect a filter both pins must be disconnected
					hr = pGraph->Disconnect(pIPin);
					hr = pGraph->Disconnect(pIPinConnection);
					SAFE_RELEASE(pIPinConnection);
				}
				SAFE_RELEASE(pIPin);
			}
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// DisconnectAllPins
// Disconnect all the pins of the filters in a graph
HRESULT DSUtils::DisconnectAllPins(IGraphBuilder* pGraph)
{
	HRESULT hr = S_OK;

	if (pGraph)
	{
		CComPtr<IEnumFilters> pIEnumFilters = NULL;
		hr = pGraph->EnumFilters(&pIEnumFilters);
		if (SUCCEEDED(hr))
		{
			IBaseFilter* pFilter = NULL;
			while (S_OK == pIEnumFilters->Next(1, &pFilter, NULL))
			{
				CComPtr<IEnumPins> pIEnumPins = NULL;
				hr = pFilter->EnumPins(&pIEnumPins);
				if (SUCCEEDED(hr))
				{
					IPin* pIPin = NULL;
					while (S_OK == pIEnumPins->Next(1, &pIPin, NULL))
					{
						IPin* pIPinConnection = NULL;
						if (S_OK == pIPin->ConnectedTo(&pIPinConnection))
						{
							// pins are connected, to disconnect filters, both pins must be disconnected
							hr = pGraph->Disconnect(pIPin);
							hr = pGraph->Disconnect(pIPinConnection);
							SAFE_RELEASE(pIPinConnection);
						}
						SAFE_RELEASE(pIPin);
					}
				}
				SAFE_RELEASE(pFilter);
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// FindFilterInterface
// Attempt to locate the specified interface
HRESULT DSUtils::FindFilterInterface(IBaseFilter* pFilter, const IID& riid, void** ppvInterface)
{
	HRESULT hr = S_OK;

	if (pFilter && ppvInterface)
	{
		hr = pFilter->QueryInterface(riid, ppvInterface);
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// FindPinInterface
// Attempt to locate the interface on the named pin or on the first pin if no
// name is provided.
HRESULT DSUtils::FindPinInterface(IBaseFilter* pFilter, LPCWSTR pName, const IID& riid, void** ppvInterface)
{
	HRESULT hr = S_OK;

	if (pFilter && ppvInterface)
	{
		CComPtr<IPin> pIPin = NULL;
		if (pName)
		{
			hr = GetPin(pFilter, pName, &pIPin);
		}
		else
		{
			CComPtr<IEnumPins> pIEnumPins = NULL;
			hr = pFilter->EnumPins(&pIEnumPins);
			if (SUCCEEDED(hr))
			{
				hr = pIEnumPins->Next(1, &pIPin, NULL);
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = pIPin->QueryInterface(riid, ppvInterface);
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// FindPinInterface
// Attempt to locate the interface on the pin with the specified format or on the first pin if no
// format is provided.
HRESULT DSUtils::FindPinInterface(IBaseFilter* pFilter, const GUID* pFormat, PIN_DIRECTION PinDir, const IID& riid, void** ppvInterface)
{
	HRESULT hr = S_OK;

	if (pFilter && ppvInterface)
	{
		CComPtr<IPin> pIPin = NULL;
		if (pFormat)
		{
			hr = GetPin(pFilter, pFormat, PinDir, &pIPin);
		}
		else
		{
			CComPtr<IEnumPins> pIEnumPins = NULL;
			hr = pFilter->EnumPins(&pIEnumPins);
			if (SUCCEEDED(hr))
			{
				hr = pIEnumPins->Next(1, &pIPin, NULL);
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = pIPin->QueryInterface(riid, ppvInterface);
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

HRESULT DSUtils::DisplayPinProperties(CComPtr<IPin> pSrcPin)
{
	CComPtr<ISpecifyPropertyPages> pPages;

	HRESULT hr = pSrcPin->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pPages);
	if (SUCCEEDED(hr)) {
		PIN_INFO PinInfo;
		pSrcPin->QueryPinInfo(&PinInfo);

		CAUUID caGUID;
		pPages->GetPages(&caGUID);

		OleCreatePropertyFrame( NULL, 0, 0, L"Property Sheet", 1, (IUnknown **)&(pSrcPin.p), caGUID.cElems, caGUID.pElems, 0, 0, NULL);
		CoTaskMemFree(caGUID.pElems);
		PinInfo.pFilter->Release();
	}
	else return(hr);

	return(S_OK);
}


void  NonBlockingFilterDialog( void *arg )
{
	HRESULT			hr;
	
	IBaseFilter *pFilter = static_cast<IBaseFilter*> (arg);

    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;

    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages,
        (void **)&pSpec);
    if(hr == S_OK)
    {
        hr = pSpec->GetPages(&cauuid);

        hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
            (IUnknown **)&pFilter, cauuid.cElems,
            (GUID *)cauuid.pElems, 0, 0, NULL);

        CoTaskMemFree(cauuid.pElems);
        pSpec->Release();
		//exit(0);
    }
   
}

HRESULT DSUtils::DisplayFilterProperties( IBaseFilter *pFilter, HWND parent )
{

 /*   ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;

    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages,
        (void **)&pSpec);
    if(hr == S_OK)
    {
        hr = pSpec->GetPages(&cauuid);

        hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
            (IUnknown **)&pFilter, cauuid.cElems,
            (GUID *)cauuid.pElems, 0, 0, NULL);

        CoTaskMemFree(cauuid.pElems);
        pSpec->Release();
		//exit(0);
    }*/
	_beginthread( NonBlockingFilterDialog, 0, (void*)pFilter );
	return(S_OK);
}


//-----------------------------------------------------------------------------
// EnumerateDevices
// Find all the capture devices in the system
HRESULT DSUtils::EnumerateDevices(const GUID* pCategory, std::list< std::basic_string<WCHAR> >& DeviceList)
{
	// first enumerate the system devices for the specifed class and filter name
	CComPtr<ICreateDevEnum> pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pSysDevEnum));

	if (SUCCEEDED(hr))
	{
		CComPtr<IEnumMoniker> pEnumCat = NULL;
		hr = pSysDevEnum->CreateClassEnumerator(*pCategory, &pEnumCat, 0);

		if (S_OK == hr)
		{
			IMoniker* pMoniker = NULL;
			while (S_OK == pEnumCat->Next(1, &pMoniker, NULL))
			{
				IPropertyBag* pPropBag = NULL;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<void**>(&pPropBag));

				if (SUCCEEDED(hr))
				{
					VARIANT varName;
					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					if (SUCCEEDED(hr))
					{
						CComBSTR name(varName.bstrVal);	// the recommended way of handling BSTR
						DeviceList.push_back((LPWSTR)name);
					}

					VariantClear(&varName);
					
					// contained within a loop, decrement the reference count
					SAFE_RELEASE(pPropBag);
				}
				SAFE_RELEASE(pMoniker);
			}
		}
	}

	return hr;
}

//-----------------------------------------------------------------------------
// AddGraphToRot
// Adds a DirectShow filter graph to the Running Object Table,
// allowing GraphEdit to "spy" on a remote filter graph.
HRESULT DSUtils::AddGraphToRot(IUnknown* pUnkGraph, DWORD* pdwRegister)
{
    HRESULT hr = S_OK;

    if (pUnkGraph && pdwRegister)
	{
		CComPtr<IRunningObjectTable> pROT = NULL;
		hr = GetRunningObjectTable(0, &pROT);
		if (SUCCEEDED(hr))
		{
			WCHAR wsz[128];
			//2008 port
			//StringCchPrintfW(wsz, 128, L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());

			CComPtr<IMoniker> pMoniker = NULL;
			hr = CreateItemMoniker(L"!", wsz, &pMoniker);
			if(SUCCEEDED(hr))
			{
				// Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
				// to the object.  Using this flag will cause the object to remain
				// registered until it is explicitly revoked with the Revoke() method.
				//
				// Not using this flag means that if GraphEdit remotely connects
				// to this graph and then GraphEdit exits, this object registration 
				// will be deleted, causing future attempts by GraphEdit to fail until
				// this application is restarted or until the graph is registered again.
				hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, pMoniker, pdwRegister);
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

    return hr;
}

//-----------------------------------------------------------------------------
// RemoveGraphFromRot
// Removes a filter graph from the Running Object Table
void DSUtils::RemoveGraphFromRot(DWORD pdwRegister)
{
    CComPtr<IRunningObjectTable> pROT = NULL;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT)))
    {
        pROT->Revoke(pdwRegister);
    }
}

//
//helper to convert HResult errors to human readable strings
//
const std::string DSUtils::GetHResultAsString(HRESULT hr)
{
	LPTSTR errorText = NULL;
	std::string messageStr = "";

	FormatMessage(
		// use system message tables to retrieve error text
		FORMAT_MESSAGE_FROM_SYSTEM
		// allocate buffer on local heap for error text
		|FORMAT_MESSAGE_ALLOCATE_BUFFER
		// Important! will fail otherwise, since we're not 
		// (and CANNOT) pass insertion parameters
		|FORMAT_MESSAGE_IGNORE_INSERTS,  
		NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&errorText,  // output 
		0, // minimum size for output buffer
		NULL);   // arguments - see note 

	if ( errorText )
	{
		// ... do something with the string - log it, display it to the user, etc.
		messageStr = std::string(errorText);
		// release memory allocated by FormatMessage()
		LocalFree(errorText);
		errorText = NULL;
	}
	return messageStr;
}

////

//------------------------------------------------------------------------------------
// CUtils - utility class
//------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// GetBMIHeader
// Returns the BITMAPINFOHEADER structure from media type format extension
BITMAPINFOHEADER* CUtils::GetBMIHeader(const AM_MEDIA_TYPE* pamt)
{
	BITMAPINFOHEADER* pbmih = NULL;

	if (pamt)
	{
		if (FORMAT_VideoInfo == pamt->formattype)
		{
			VIDEOINFOHEADER* pvih = reinterpret_cast<VIDEOINFOHEADER*>(pamt->pbFormat);
			//ASSERT(sizeof(VIDEOINFOHEADER) <= pamt->cbFormat);
			pbmih = &pvih->bmiHeader;
		}
		else if (FORMAT_VideoInfo2 == pamt->formattype)
		{
			VIDEOINFOHEADER2* pvih = reinterpret_cast<VIDEOINFOHEADER2*>(pamt->pbFormat);
			//ASSERT(sizeof(VIDEOINFOHEADER2) <= pamt->cbFormat);
			pbmih = &pvih->bmiHeader;
		}
	}

	return pbmih;
}

//-----------------------------------------------------------------------------
// GetBMIHeader
// Returns the BITMAPINFOHEADER structure from media type format extension
BITMAPINFOHEADER* CUtils::GetBMIHeader(const CMediaType& mt)
{
	BITMAPINFOHEADER* pbmih = NULL;

	if (FORMAT_VideoInfo == mt.formattype)
	{
		VIDEOINFOHEADER* pvih = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
		//ASSERT(sizeof(VIDEOINFOHEADER) <= mt.cbFormat);
		pbmih = &pvih->bmiHeader;
	}
	else if (FORMAT_VideoInfo2 == mt.formattype)
	{
		VIDEOINFOHEADER2* pvih = reinterpret_cast<VIDEOINFOHEADER2*>(mt.pbFormat);
		//ASSERT(sizeof(VIDEOINFOHEADER2) <= mt.cbFormat);
		pbmih = &pvih->bmiHeader;
	}

	return pbmih;
}

//-----------------------------------------------------------------------------
// GetAvgTimePerFrame
// Returns the average time per frame from media type format extension
REFERENCE_TIME CUtils::GetAvgTimePerFrame(const AM_MEDIA_TYPE* pamt)
{
	REFERENCE_TIME rtAvgTimePerFrame = 0;

	if (pamt)
	{
		if (FORMAT_VideoInfo == pamt->formattype)
		{
			VIDEOINFOHEADER* pvih = reinterpret_cast<VIDEOINFOHEADER*>(pamt->pbFormat);
			//ASSERT(sizeof(VIDEOINFOHEADER) <= pamt->cbFormat);
			rtAvgTimePerFrame = pvih->AvgTimePerFrame;
		}
		else if (FORMAT_VideoInfo2 == pamt->formattype)
		{
			VIDEOINFOHEADER2* pvih = reinterpret_cast<VIDEOINFOHEADER2*>(pamt->pbFormat);
			//ASSERT(sizeof(VIDEOINFOHEADER2) <= pamt->cbFormat);
			rtAvgTimePerFrame = pvih->AvgTimePerFrame;
		}
	}

	return rtAvgTimePerFrame;
}

//------------------------------------------------------------------------------------
// GetImageSize
// Calculates the image size
unsigned long CUtils::GetImageSize(const BITMAPINFOHEADER* pbmih)
{
	unsigned long dwImageSize = 0;

	if (pbmih)
	{
		switch (pbmih->biCompression)
		{
			default:
			case BI_RGB:
				dwImageSize = (pbmih->biWidth * abs(pbmih->biHeight) * pbmih->biBitCount) >> 3;
				break;

			case 'YVYU':
			case '2YUY':
				dwImageSize = (pbmih->biWidth * abs(pbmih->biHeight) * 16) >> 3;
				break;
		}
	}

	return dwImageSize;
}
