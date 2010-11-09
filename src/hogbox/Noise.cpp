#include <hogbox/Noise.h>
#include <osg/Notify>
#include <osg/ImageUtils>

#include <math.h>
#include <stdlib.h>

using namespace hogbox; 

//operators for osg imageutils modify image func

//
//Generate a random noise value for each pixel passed
//pixel is either color1 or color2 i.e. binary noise
//
struct GenerateBinaryNoiseOperator
{
	GenerateBinaryNoiseOperator(const float& c1, const float& c2, const unsigned int& seed=-1)
	{
		hogbox::SetSeed(seed);
		_color1 = osg::Vec4(c1,c1,c1,c1);
		_color2 = osg::Vec4(c2,c2,c2,c2);
	}
	GenerateBinaryNoiseOperator(const osg::Vec3& c1, const osg::Vec3& c2, const unsigned int& seed=-1)
	{
		hogbox::SetSeed(seed);
		_color1 = osg::Vec4(c1, 1.0f);
		_color2 = osg::Vec4(c2, 1.0f);
	}
	GenerateBinaryNoiseOperator(const osg::Vec4& c1, const osg::Vec4& c2, const unsigned int& seed=-1)
	{
		hogbox::SetSeed(seed);
		_color1 = c1;
		_color2 = c2;
	}
	
	//generate the noise for this pixel, the noise value is then used
	//as a threshold mask to use either color1 or color2,
	inline osg::Vec4 genNoiseColor()const{
	
		float noisy = hogbox::RandFloat(0.0f, 1.0f);
		if(noisy >= 0.5f)
		{
			return _color2;
		}
		return _color1;
	}
	
	inline void luminance(float& l) const {l = genNoiseColor().x(); } 
	inline void alpha(float& a) const { a = genNoiseColor().a(); } 
	inline void luminance_alpha(float& l,float& a) const {
		osg::Vec4 noisy = genNoiseColor();
		l = noisy.x(); a = noisy.a(); 
	}
	inline void rgb(float& r,float& g,float& b) const {
		osg::Vec4 noisy = genNoiseColor();
		r = noisy.r(); g = noisy.g(); b = noisy.b(); 
	}
	inline void rgba(float& r,float& g,float& b,float& a) const {
		osg::Vec4 noisy = genNoiseColor();
		r = noisy.r(); g = noisy.g(); b = noisy.b(); a = noisy.a(); 
	}
	
	osg::Vec4 _color1;
	osg::Vec4 _color2;
};


osg::ref_ptr<osg::Image> hogbox::CreateGreyScaleBinaryNoiseImage2D(const int& width, const int& height,
																	const float& color1, const float& color2,
																	const unsigned int& seed)
{
	osg::ref_ptr<osg::Image> image = new osg::Image();
	
	//alocate the storage for greyscale image
	image->allocateImage(width, height, 1, GL_LUMINANCE, GL_UNSIGNED_BYTE, 1);
	
	FillGreyScaleBinaryNoiseImage2D(image.get(), color1, color2, seed);
	return image;
}

osg::ref_ptr<osg::Image> hogbox::CreateRGBBinaryNoiseImage2D(const int& width, const int& height,
															 const osg::Vec3& color1, const osg::Vec3& color2,
															 const unsigned int& seed)
{
	osg::ref_ptr<osg::Image> image = new osg::Image();
	
	//alocate the storage for greyscale image
	image->allocateImage(width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, 1);
	
	FillRGBBinaryNoiseImage2D(image.get(), color1, color2, seed);
	
	return image;
}

osg::ref_ptr<osg::Image> hogbox::CreateRGBABinaryNoiseImage2D(const int& width, const int& height, 
															 const osg::Vec4& color1, const osg::Vec4& color2,
															 const unsigned int& seed)
{
	osg::ref_ptr<osg::Image> image = new osg::Image();
	
	//alocate the storage for greyscale image
	image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, 1);
	
	FillRGBABinaryNoiseImage2D(image.get(), color1, color2, seed);	
	
	return image;	
}

bool hogbox::FillGreyScaleBinaryNoiseImage2D(osg::Image* image,
											const float& color1, const float& color2,
											const unsigned int& seed)
{
	if(!image){return false;}
	if(image->getPixelFormat() != GL_LUMINANCE){return false;}
	osg::modifyImage(image, GenerateBinaryNoiseOperator(color1,color2,seed));
	image->dirty();
	return true;
}
bool hogbox::FillRGBBinaryNoiseImage2D(osg::Image* image, 
									  const osg::Vec3& color1, const osg::Vec3& color2,
									  const unsigned int& seed)
{
	if(!image){return false;}
	if(image->getPixelFormat() != GL_RGB){return false;}
	osg::modifyImage(image, GenerateBinaryNoiseOperator(color1,color2,seed));
	image->dirty();
	return true;
}

bool hogbox::FillRGBABinaryNoiseImage2D(osg::Image* image, 
									   const osg::Vec4& color1, const osg::Vec4& color2,
									   const unsigned int& seed)
{
	if(!image){return false;}
	if(image->getPixelFormat() != GL_RGBA){return false;}
	osg::modifyImage(image, GenerateBinaryNoiseOperator(color1,color2,seed));
	image->dirty();
	return true;	
}


