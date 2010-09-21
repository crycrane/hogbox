#pragma once

#include <streams.h>
#undef lstrlenW 


#include <stdio.h>
#include <atlbase.h>
#include <qedit.h>

#include <comdef.h>
#include <iostream>
#include <vector>

#include <string>
#include <list>
#include <dvdmedia.h>

#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }


//------------------------------------------------------------------------------------
// DSUtils - Directshow utility class
//------------------------------------------------------------------------------------
class DSUtils
{
public:
	static HRESULT AddFilter(IGraphBuilder* pGraph, const GUID &clsid, LPCWSTR pName, IBaseFilter** ppFilter);
	static HRESULT AddFilter2(IGraphBuilder* pGraph, const GUID &clsid, LPCWSTR pName, const std::string& uniqueID, IBaseFilter** ppFilter);

	static HRESULT GetUnconnectedPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin);
	static HRESULT GetPin(IBaseFilter* pFilter, LPCWSTR pName, IPin** ppPin);
	static HRESULT GetPin(IBaseFilter* pFilter, const GUID* pFormat, PIN_DIRECTION PinDir, IPin** ppPin);

	static HRESULT ConnectFilters(IGraphBuilder* pGraph, IBaseFilter* pUpstream, LPCWSTR pUpstreamPinName, IBaseFilter* pDownstream, LPCWSTR pDownstreamPinName);
	static HRESULT ConnectFilters(IGraphBuilder* pGraph, IBaseFilter* pUpstream, IBaseFilter* pDownstream, const GUID* pFormat);
	static HRESULT RenderFilter(IGraphBuilder* pGraph, IBaseFilter* pUpstream, LPCWSTR pUpstreamPinName);

	static HRESULT DisconnectPins(IGraphBuilder* pGraph, IBaseFilter* pFilter);
	static HRESULT DisconnectAllPins(IGraphBuilder* pGraph);
	
	static HRESULT FindFilterInterface(IBaseFilter* pFilter, const IID& riid, void** ppvInterface);
	static HRESULT FindPinInterface(IBaseFilter* pFilter, LPCWSTR pName, const IID& riid, void** ppvInterface);
	static HRESULT FindPinInterface(IBaseFilter* pFilter, const GUID* pFormat, PIN_DIRECTION PinDir, const IID& riid, void** ppvInterface);

	static HRESULT DisplayPinProperties(CComPtr<IPin> pSrcPin);
	static HRESULT DisplayFilterProperties( IBaseFilter *pFilter, HWND parent);

	static HRESULT EnumerateDevices(const GUID* pCategory, std::list< std::basic_string<WCHAR> >& DeviceList);

	static HRESULT AddGraphToRot(IUnknown* pUnkGraph, DWORD* pdwRegister);
	static void RemoveGraphFromRot(DWORD pdwRegister);

	//helper to convert HResult errors to human readable strings
	static const std::string GetHResultAsString(HRESULT hr);
};

class CUtils
{
public:
	static BITMAPINFOHEADER* GetBMIHeader(const AM_MEDIA_TYPE* pamt);
	static BITMAPINFOHEADER* GetBMIHeader(const CMediaType& mt);
	static REFERENCE_TIME GetAvgTimePerFrame(const AM_MEDIA_TYPE* pamt);
	static unsigned long GetImageSize(const BITMAPINFOHEADER* pbmih);
};

