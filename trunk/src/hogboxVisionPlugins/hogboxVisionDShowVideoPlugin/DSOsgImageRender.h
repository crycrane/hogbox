#pragma once

#include <windows.h>
#include <tchar.h>
#include <dshow.h>

#include <mmsystem.h>
#include <atlbase.h>
#include <stdio.h>
#include <Streams.h>

#include <osg/image>

struct __declspec(uuid("{183A989C-9CBC-4f68-BB3E-A193A0CBD5B7}")) CLSID_OSGImageRenderer;

class DSOsgImageRender :
	public CBaseVideoRenderer
{
public:
	DSOsgImageRender(osg::Image* pImage, LPUNKNOWN pUnk,HRESULT *phr, bool hFlip, bool vFlip, bool deInter);
	~DSOsgImageRender(void);

public:
    HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
    HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification
    HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample
	void OnReceiveFirstSample(IMediaSample *pMediaSample);

//	STDMETHODIMP_(ULONG) NonDelegatingRelease(void){return 1;}

	int GetMediaWidth(){return Width;}
	
	int GetMediaHeight()
	{
		if(!m_deInter)
		{return Height;}
		return Height/2;
	}

	bool GetHFlip(){return m_hFlip;}
	bool GetVFlip(){return m_vFlip;}

	void SetHFlip(bool hFlip){m_hFlip = hFlip;}
	void SetVFlip(bool vFlip){m_vFlip = vFlip;}

	bool GetDeInter(){return m_deInter;}

private:

	BYTE *m_transBuffer;

	osg::Image* m_renderSurface;

    int Width;   // Video width
    int Height;  // Video Height
    int Pitch;   // Video Pitch

	bool m_vFlip;
	bool m_hFlip;
	bool m_deInter;
};
