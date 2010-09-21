#include "dsosgimagerender.h"

#define REGISTER_FILTERGRAPH

//-----------------------------------------------------------------------------
// DSOsgImageRender constructor
//-----------------------------------------------------------------------------
DSOsgImageRender::DSOsgImageRender( osg::Image* pImage, LPUNKNOWN pUnk, HRESULT *phr, bool hFlip, bool vFlip, bool deInter )
                                  : CBaseVideoRenderer(__uuidof(CLSID_OSGImageRenderer), NAME("OSG Image Renderer"), pUnk, phr), 
									m_renderSurface( pImage), m_hFlip(hFlip), m_vFlip(vFlip), m_deInter(deInter)
{

    // Store and AddRef the texture for our use.
    //ASSERT(phr);
    if (phr)
    {
        if( !pImage )
        {
            *phr = E_INVALIDARG;
        }
        else
        {
            *phr = S_OK;
        }
    }

	m_transBuffer = NULL;
}


//-----------------------------------------------------------------------------
// DSOsgImageRender destructor
//-----------------------------------------------------------------------------
DSOsgImageRender::~DSOsgImageRender()
{
    //cleanup the transform buffer
	if(m_transBuffer != NULL)
	{
		delete [] m_transBuffer;
		m_transBuffer = NULL;
	}
}


//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT DSOsgImageRender::CheckMediaType(const CMediaType *pmt)
{
    HRESULT   hr = E_FAIL;
    VIDEOINFO *pvi=0;

    //ASSERT( m_pCP && m_pCP->GetDevice());
    try
    {
        // Reject the connection if this is not a video type
        if( *pmt->FormatType() != FORMAT_VideoInfo ) 
        {
            return E_INVALIDARG;
        }
        
        // Only accept RGB24
        pvi = (VIDEOINFO *)pmt->Format();

        if( IsEqualGUID( *pmt->Type(), MEDIATYPE_Video) )
        {
            hr = S_OK;

            if( IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24) )
            {
				
               // m_MediaFormat = D3DFMT_R8G8B8;
            }
            else
            {
                hr = DDERR_INVALIDPIXELFORMAT;
            }
        }
        if( FAILED(hr))
        {
            return hr;
        }
        
    }// try
    catch(...)
    {
      //  Msg(TEXT("Failed to check media type in the renderer. Unhandled exception hr=0x%x"), E_UNEXPECTED);
        hr = E_UNEXPECTED;
    }   
    return hr;
}


//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made. 
//-----------------------------------------------------------------------------
HRESULT DSOsgImageRender::SetMediaType(const CMediaType *pmt)
{
    HRESULT hr = S_OK;
    VIDEOINFO *pviBmp = NULL;   // Bitmap info header

    try
    {
        // Retreive the size of this media type
        pviBmp = (VIDEOINFO *)pmt->Format();

        Width  = (int)pviBmp->bmiHeader.biWidth;
        Height = (int)abs(pviBmp->bmiHeader.biHeight);
        Pitch  = (int)(Width * 3 + 3) & ~(3); // We are forcing RGB24

		if(!m_deInter)
		{
			m_renderSurface->allocateImage(Width, Height, 1, GL_BGR, GL_UNSIGNED_BYTE, 1);
		}else{

			m_renderSurface->allocateImage(Width, Height/2, 1, GL_BGR, GL_UNSIGNED_BYTE, 1);
			//m_lVidHeight = m_lVidHeight/2;
		}

		if(m_transBuffer){delete [] m_transBuffer;m_transBuffer=NULL;}
		if(m_transBuffer == NULL)
		{
			if(!m_deInter)
			{
				m_transBuffer = new BYTE[(Width*Height)*3];//*4];
			}else{
				m_transBuffer = new BYTE[(Width*(Height/2))*3];//*4];
			}
		}

		if( FAILED(hr))
        {return hr;}

    }// try
    catch(...)
    {
       // Msg(TEXT("Failed to set media type in the renderer. Unhandled exception hr=0x%x"), E_UNEXPECTED);
        return hr;
    }

    return hr;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT DSOsgImageRender::DoRenderSample( IMediaSample * pSample )
{
    HRESULT hr = S_OK;
    BYTE * pBuffer = NULL;

    try
    {
		// Get the video bitmap buffer
		hr = pSample->GetPointer( &pBuffer );
		int length = (int)pSample->GetActualDataLength();
		if(m_deInter) //DEINTERLACED
		{
			if(m_vFlip && !m_hFlip) //deinter and flipv
			{
				//every other row
				BYTE *sourceTemp=pBuffer;
				//destination, start from top to flip
				BYTE *destTemp=m_transBuffer;
				destTemp += (Width*3)*((Height/2)-1);

				for(int r=0; r<Height/2; r++)
				{
					memcpy(destTemp,sourceTemp,(Width*3));
					destTemp-=(Width*3); //move one line in the dest
					sourceTemp+=(Width*3)*2; //skip a line of the source for inter
				}
			
				m_renderSurface->setImage(Width, Height/2,1, GL_RGB, GL_BGR, 
									GL_UNSIGNED_BYTE, m_transBuffer, osg::Image::NO_DELETE, 1);
			
			}else if(m_vFlip && m_hFlip){ //deinter with both flips
			
				//pointer to begining of source
				BYTE *sourceTemp=pBuffer;
				//destination, end of last line
				BYTE *destTemp=m_transBuffer;
				destTemp += ((Width*3)*Height/2)-3;

				//loop lines
				for(int r=0; r<Height/2; r++) //half height to deinter
				{
					//copyline of pixels 
					for(int p=0; p<Width; p++)
					{
						memcpy(destTemp,sourceTemp,3);
						destTemp-=(3); //move back one pixel
						sourceTemp+=(3); //move to the next pixel
					}
					//skip the next line
					sourceTemp+=(Width*3);

				}
				m_renderSurface->setImage(Width, Height/2,1, GL_RGB, GL_BGR, 
					GL_UNSIGNED_BYTE, m_transBuffer, osg::Image::NO_DELETE, 1);

			
			}else{ //just deinter
				
				//very other row
				BYTE *sourceTemp=pBuffer;
				//destination, start from top to flip
				BYTE *destTemp=m_transBuffer;
				//destTemp += (Width*4)*((Height/2)-1);

				for(int r=0; r<Height/2; r++)
				{
					memcpy(destTemp,sourceTemp,(Width*3));
					destTemp+=(Width*3); //move one line in the dest
					sourceTemp+=(Width*3)*2; //skip a line of the source for inter
				}
			
				m_renderSurface->setImage(Width, Height/2,1, GL_RGB, GL_BGR, 
									GL_UNSIGNED_BYTE, m_transBuffer, osg::Image::NO_DELETE, 1);
				
			}
		
		}else if(m_vFlip && !m_hFlip)//NO DEINTERLACED
		{
			//very other row
			BYTE *sourceTemp=pBuffer;
			//destination, start from top to flip
			BYTE *destTemp=m_transBuffer;
			destTemp += (Width*3)*((Height)-1);

			for(int r=0; r<Height; r++)
			{
				memcpy(destTemp,sourceTemp,(Width*3));
				destTemp-=(Width*3); //move one line in the dest
				sourceTemp+=(Width*3); //skip a line of the source for inter
			}

			m_renderSurface->setImage(Width, Height,1, GL_RGB, GL_BGR, 
								GL_UNSIGNED_BYTE, m_transBuffer, osg::Image::NO_DELETE, 1);
			//m_p_image->l
			//m_p_image->flipHorizontal(); 


		}else if(m_hFlip && !m_vFlip) { //just horiflip

				//pointer to begining of source
				BYTE *sourceTemp=pBuffer;
				//destination, end of first line
				BYTE *destTemp=m_transBuffer;
				destTemp += (Width*3)-3;

				//loop lines
				for(int r=0; r<Height; r++)
				{
					//copyline of pixels 
					for(int p=0; p<Width; p++)
					{
						memcpy(destTemp,sourceTemp,3);
						destTemp-=(3); //move back one pixel
						sourceTemp+=(3); //move to the next pixel
					}
					//move the destination to the end of the next line (should be move two lines
					destTemp+= ((Width*3)*2);
				}
				m_renderSurface->setImage(Width, Height,1, GL_RGB, GL_BGR, 
					GL_UNSIGNED_BYTE, m_transBuffer, osg::Image::NO_DELETE, 1);//
				
		}else if(m_hFlip && m_vFlip) { // both

				//pointer to begining of source
				BYTE *sourceTemp=pBuffer;
				//destination, end of last line
				BYTE *destTemp=m_transBuffer;
				destTemp += ((Width*3)*Height)-3;

				//loop lines
				for(int r=0; r<Height; r++)
				{
					//copyline of pixels 
					for(int p=0; p<Width; p++)
					{
						memcpy(destTemp,sourceTemp,3);
						destTemp-=(3); //move back one pixel
						sourceTemp+=(3); //move to the next pixel
					}
					//move the destination to the end of the next line (should be move two lines
					//destTemp -= 4;//(Width*4)+4;
				}
				m_renderSurface->setImage(Width, Height,1, GL_RGB, GL_BGR, 
					GL_UNSIGNED_BYTE, m_transBuffer, osg::Image::NO_DELETE, 1);
		}else{
			m_renderSurface->setImage(Width, Height,1, GL_RGB, GL_BGR, 
								GL_UNSIGNED_BYTE, pBuffer, osg::Image::NO_DELETE, 1);
			
		}


//////////////////

    }
    catch(...)
    {
    
    }
    return hr;
}


void DSOsgImageRender::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
	if (pMediaSample)
	{
		DoRenderSample(pMediaSample);
	}
}
