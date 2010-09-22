#include <hogboxVision/VideoTex2DCallBack.h>

#include <osg/Image>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

#include <hogboxVision/VideoStream.h>
#include <iostream>

using namespace hogboxVision;

VideoTex2DCallBack::VideoTex2DCallBack(osg::Texture* texture, int channel, osg::StateSet* state): osg::Texture2D::SubloadCallback()
{
	m_texMat = new osg::TexMat;
	//if its a rectangle set rect matrix and return
	if(dynamic_cast<osg::TextureRectangle*>(texture))
	{
		m_useAsCallback = false;
		m_texMat->setScaleByTextureRectangleSize(true);
		if(state){
			state->setTextureAttributeAndModes(channel, m_texMat.get(), osg::StateAttribute::ON);
		}
		return;

	}else{

		//if its a texture2D we need to find the power of two size up
		//and calc a texture matrix to scale coord to crop any boarder
		m_useAsCallback = true;

		osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(texture); 

		const osg::Image* _image = texture2D->getImage();
		//set the new scaled cords
		m_imageWidth = _image->s();
		m_imageHeight = _image->t();

		//first find nearest to see if it is a power of two already (having problems with calcnext power of two)
		m_scaledWidth = osg::Image::computeNearestPowerOfTwo((int)_image->s());//
		m_scaledHeight = osg::Image::computeNearestPowerOfTwo((int)_image->t()); 
		
		//its not power of two, find next up
		if(m_scaledWidth != m_imageWidth)
		{m_scaledWidth = VideoStreamBase::computeNextPowerOfTwo((unsigned int) m_imageWidth);}
		//same for height
		if(m_scaledHeight != m_imageHeight)
		{m_scaledHeight = VideoStreamBase::computeNextPowerOfTwo((unsigned int) m_imageHeight);}
		
		//set the texture to beleive it is of power of two size
		texture2D->setTextureSize(m_scaledWidth, m_scaledHeight);

		//set to linear to disable mipmap generation
		texture2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		//clamp as we wont be getting a full buffer in most cases so wrap will look odd
		texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
		texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);
		

		//calc the scaled coords as the original dimensions divided by the new scaled up dimensions
		m_scaledTexCoordX = m_imageWidth / (float)m_scaledWidth;
		m_scaledTexCoordY = m_imageHeight / (float)m_scaledHeight;

		//set our scale matrix to crop any empty region left by the power of two size up
		m_texMat->setMatrix(osg::Matrix::scale(m_scaledTexCoordX, m_scaledTexCoordY, 0));	
		//apply our scale matrix to the state
		if(state){
			state->setTextureAttributeAndModes(channel, m_texMat.get(), osg::StateAttribute::ON);
		}
	}
}

void VideoTex2DCallBack::load(const osg::Texture2D& texture, osg::State&) const 
{
	
	const osg::Image* _image = texture.getImage();

	//create texture space to include the power of two size up
	//this way writes to the video memory will be quicker
	glTexImage2D(GL_TEXTURE_2D, 0, 
		osg::Image::computeNumComponents(_image->getInternalTextureFormat()), 
		(int)m_scaledWidth, 
		(int)m_scaledHeight, 
		0, _image->getPixelFormat(), 
		_image->getDataType(), 0);

}

void VideoTex2DCallBack::subload(const osg::Texture2D& texture, osg::State&) const 
{

	const osg::Image* _image = texture.getImage();

	//copy the actual res image into the sized up buffer
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
					m_imageWidth, m_imageHeight, _image->getPixelFormat(), 
					_image->getDataType(), _image->data());

}