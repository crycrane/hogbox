/* Written by Thomas Hogarth, (C) 2011
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 */

#pragma once

#include <hogbox/Export.h>
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
//but _useAsCallback is set to false to indicate we don't need to actually apply this as a callback
//
class HOGBOX_EXPORT NPOTResizeCallback : public osg::Texture2D::SubloadCallback
{
public:
	//
	//Contructor will take generic texture then determine if it is 2d or rect
	//if 2d we calc the power of 2 up scale and apply the matrix, flaging this to be used as a callback
	//if rect then we just set the tex matrix to scale to rect coords and flag to not be used as callback
	NPOTResizeCallback(osg::Texture* texture, int channel, osg::StateSet* state);

	bool useAsCallBack(){return _useAsCallback;}

	virtual bool textureObjectValid(const osg::Texture2D& texture, osg::State& state) const
    {
        return true;//texture.textureObjectValid(state);
    }

	void load(const osg::Texture2D& texture, osg::State&) const;
	void subload(const osg::Texture2D& texture, osg::State&) const;
	
	osg::TexMat* GetScaleMatrix(){return _texMat.get();};

	inline float getScaledTexCoordX() const { return (_scaledTexCoordX);};
	inline float getScaledTexCoordY() const { return (_scaledTexCoordY);};

	protected:

	//true if once contructor determine if we need the call back attached
	bool _useAsCallback;

	//our matrix for scaling the texture coords
	osg::ref_ptr<osg::TexMat> _texMat;

	
	//original image dimensions, sub loaded into texture
	int _imageWidth;
	int _imageHeight;

	//scaled up dimensions
	int _scaledWidth;
	int _scaledHeight;

	//the scaling factor for texture coords using the texture
	float _scaledTexCoordX;
	float _scaledTexCoordY;
	
	//mod count should match image mod count
	mutable unsigned int _modifiedCount;
};

};
