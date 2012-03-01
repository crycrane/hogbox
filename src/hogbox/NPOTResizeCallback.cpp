#include <hogbox/NPOTResizeCallback.h>

#include <osg/Image>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

#include <iostream>

#if !defined(_MSC_VER) || !defined(_X86_)
# include <cmath>
#endif

using namespace hogbox;


//
//Compute next power of two up from x
//
unsigned int computeNextPowerOfTwo(unsigned int x)
{
#if defined(WIN32)
	int val = x; 
	val--;
	val = (val >> 1) | val;
	val = (val >> 2) | val;
	val = (val >> 4) | val;
	val = (val >> 8) | val;
	val = (val >> 16) | val;
	val++; // Val is now the next highest power of 2.
	return val;
#else
	return ((unsigned int)(exp2((double)((int)(log2((double)x)) + 1))));
#endif
}


NPOTResizeCallback::NPOTResizeCallback(osg::Texture* texture, int channel, osg::StateSet* state)
	: osg::Texture2D::SubloadCallback(),
	_useAsCallback(false),
	_scaledTexCoordX(1.0f),
	_scaledTexCoordY(1.0f),
    _modifiedCount(0)
{
	_texMat = new osg::TexMat;
	//set our scale matrix to crop any empty region left by the power of two size up
	_texMat->setMatrix(osg::Matrix::scale(_scaledTexCoordX, _scaledTexCoordY, 0));	
	
	if(!texture){return;}
	
	//if its a rectangle set rect matrix and return
	if(dynamic_cast<osg::TextureRectangle*>(texture))
	{
		_useAsCallback = false;
		_texMat->setScaleByTextureRectangleSize(true);
		if(state){
			state->setTextureAttributeAndModes(channel, _texMat.get(), osg::StateAttribute::ON);
		}
		return;

	}else{

		//if its a texture2D we need to find the power of two size up
		//and calc a texture matrix to scale coord to crop any boarder
		osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(texture); 
        if(!texture2D){return;}

		osg::Image* _image = texture2D->getImage(0);
		//if no image the we don't want to use this (i.e. we've been passed a render to texture texture)
		if(!_image){
			return;
		}

		//set the true image size
		_imageWidth = _image->s();
		_imageHeight = _image->t();
		
		//first find nearest to see if it is a power of two already (having problems with calcnext power of two)
		_scaledWidth = osg::Image::computeNearestPowerOfTwo((int)_imageWidth);//
		_scaledHeight = osg::Image::computeNearestPowerOfTwo((int)_imageHeight); 
		
		//its not power of two, find next up
		if(_scaledWidth != _imageWidth || _scaledHeight != _imageHeight)
		{
			_scaledWidth = computeNextPowerOfTwo((unsigned int) _imageWidth);
			//same for height
			_scaledHeight = computeNextPowerOfTwo((unsigned int) _imageHeight);
		}else{
			//they are both already power of two so treturn before flaging to use
			return;
		}
		
		
		//we have a texture that isn't power of two so flag to use the callback
		_useAsCallback = true;
		
		//set the texture to beleive it is of power of two size
		texture2D->setTextureSize(_scaledWidth, _scaledHeight);

		//set to linear to disable mipmap generation
		texture2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		texture2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		//clamp as we wont be getting a full buffer in most cases so wrap will look odd
		texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
		

		//calc the scaled coords as the original dimensions divided by the new scaled up dimensions
		_scaledTexCoordX = _imageWidth / (float)_scaledWidth;
		_scaledTexCoordY = _imageHeight / (float)_scaledHeight;

		//set our scale matrix to crop any empty region left by the power of two size up
		_texMat->setMatrix(osg::Matrix::scale(_scaledTexCoordX, _scaledTexCoordY, 0));	
		//apply our scale matrix to the state
		if(state){
			state->setTextureAttributeAndModes(channel, _texMat.get(), osg::StateAttribute::ON);
		}
	}
}

void NPOTResizeCallback::load(const osg::Texture2D& texture, osg::State&) const 
{
	const osg::Image* _image = texture.getImage();

	//create texture space to include the power of two size up
	//this way writes to the video memory will be quicker
	glTexImage2D(GL_TEXTURE_2D, 0, 
		_image->getInternalTextureFormat(), 
		(int)_scaledWidth, 
		(int)_scaledHeight, 
		0, _image->getPixelFormat(), 
		_image->getDataType(), 0);	
}

void NPOTResizeCallback::subload(const osg::Texture2D& texture, osg::State& state) const 
{
	//const unsigned int contextID = state.getContextID();
	const osg::Image* _image = texture.getImage();

	if(_image->valid() && _modifiedCount != _image->getModifiedCount())
	{
		//copy the actual res image into the sized up buffer
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
						_imageWidth, _imageHeight, _image->getPixelFormat(), 
						_image->getDataType(), _image->data());
		
		// update the modified tag to show that it is up to date.
		_modifiedCount = _image->getModifiedCount();
	}
}