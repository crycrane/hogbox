#pragma once


#include <osg/Image>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TexMat>

namespace hogbox {

//
// Resizes the texture to next power of two to allow
// decent frame rate on tex2d
// the resized texture requires that we scale the tex coords of any object using it (to account for the size up)
// If the texture passed to the contructor is a tex rect than a texRect coord size up matrix is applied
//but m_useAsCallback is set to false to indicate we don't need to actually apply this as a callback
//
class NPOTResizeCallback :	public osg::Texture2D::SubloadCallback
{
public:
	//
	//Contructor will take generic texture then determine if it is 2d or rect
	//if 2d we calc the power of 2 up scale and apply the matrix, flaging this to be used as a callback
	//if rect then we just set the tex matrix to scale to rect coords and flag to not be used as callback
	NPOTResizeCallback(osg::Texture* texture, int channel, osg::StateSet* state);

	bool useAsCallBack(){return m_useAsCallback;}

	void load(const osg::Texture2D& texture, osg::State&) const;
	void subload(const osg::Texture2D& texture, osg::State&) const;

	inline float getScaledTexCoordX() const { return (m_scaledTexCoordX);};
	inline float getScaledTexCoordY() const { return (m_scaledTexCoordY);};

	protected:

	//true if once contructor determine if we need the call back attached
	bool m_useAsCallback;

	//our matrix for scaling the texture coords
	osg::ref_ptr<osg::TexMat> m_texMat;

	
	//original image dimensions, sub loaded into texture
	int m_imageWidth;
	int m_imageHeight;

	//scaled up dimensions
	int m_scaledWidth;
	int m_scaledHeight;

	//the scaling factor for texture coords using the texture
	float m_scaledTexCoordX;
	float m_scaledTexCoordY;
	
	//mod count should match image mod count
	mutable unsigned int m_modifiedCount;
};

};
