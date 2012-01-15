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

#include <hogboxVision/Export.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Camera>
#include <osg/Texture2D>
#include <osg/TextureRectangle>


namespace hogboxVision {

//
//Draws an ortho2d quad over the entire screen
//
class HOGBOXVIS_EXPORT Ortho2DLayer : public osg::Group
{
public:    
    
    //layer has render mode of prerender, post render or nested
    enum RenderStage{
        ORTHO_BACKGROUND,
        ORTHO_NESTED,
        ORTHO_OVERLAY
    };
    
	//contruct passing the video stream to render on the layer
	Ortho2DLayer(int width = 640, int height = 480, RenderStage stage = ORTHO_BACKGROUND);
	
	//osg inherits
	Ortho2DLayer(const Ortho2DLayer&,const osg::CopyOp& = osg::CopyOp::SHALLOW_COPY);
	META_Node(hogboxVision,Ortho2DLayer)

	//set the model view matrix z rotation
	void SetRotation(const float& rotDegrees);
	//set the orientation/up side, will set the rotation to increments of 90
	//plus also alter the ortho projection ration. 0 is up, 1 is right, 2 is down
	void SetOrientation(const int& orientation);
	
	void SetVerticalFlip(const bool& bVFlip);
	void SetHorizontalFlip(const bool& bHFlip);	
	
	//get the layer width
	inline float GetWidth() const {
		return _width;
	}	
	//get the layer height
	inline float GetHeight() const {
		return _height;
	}
	

    //
    //helper to apply a texture to a particular channel of the geodes stateset
    void ApplyTexture(osg::Texture2D* tex, int channel=0);
	

	
protected: 
	
	//
	virtual ~Ortho2DLayer();
	
	//build the actual drawable object and attach to the rendering system			
	osg::ref_ptr<osg::Camera> buildLayer();
	//build the actual geometry of the object
	osg::ref_ptr<osg::Geode> buildLayerGeometry();
	
	//apply all the orientation and transform information
	void ApplyTransforms();
	
protected:
	
    RenderStage _renderStage;
    
	//dimensions of layer, should match the ortho projection dimensions
	float _width;
	float _height;
	
	float _rotDegrees;
	int _orientation;
	bool _hFlip;
	bool _vFlip;

	//actual verts etc
	osg::ref_ptr<osg::Geode> _layerGeode;
	osg::ref_ptr<osg::Geometry> _geometry;

    //camera for pre or post ortho projection
    osg::ref_ptr<osg::Camera> _camera;
    
	//draw matrices for ortho projection
	

};
    
typedef osg::ref_ptr<Ortho2DLayer> Ortho2DLayerPtr;

};