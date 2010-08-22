#pragma once

#include <hogboxVision/Export.h>

#include <osg/Geode>
#include <osg/Node>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/Drawable>
#include <osg/BoundingBox>
#include <osg/Projection>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

#include <hogboxVision/VideoStream.h>

namespace hogboxVision {

//
//Drawable video layer
//
class HOGBOXVIS_EXPORT FSVideoLayer : public osg::Group
{
public:        
	//contruct passing the video stream to render on the layer
	FSVideoLayer(int width = 640, int height = 480);
	
	//osg inherits
	FSVideoLayer(const FSVideoLayer&,const osg::CopyOp& = osg::CopyOp::SHALLOW_COPY);
	META_Node(NULL,FSVideoLayer)


	void SetRotation(float x, float y, float z, int vFlip, int hFlip);	
		
	//apply a different texture to our fs quad
	void setTextureFromVideoStream(VideoStreamBase* videoStream, int channel);
	void setTextureFromTex2D(osg::Texture2D* texture, int channel);
	void setTextureFromTexRect(osg::TextureRectangle* texture, int channel);
	
	
	//get the screen space width
	inline float getWidth() const {
		return m_width;
	}	
	//get the screen space height
	inline float getHeight() const {
		return m_height;
	}
	

	//for osg drawpixels update
	void setViewPortSize(osg::Vec2 viewPortSize);
	

	
protected: 
	
	//referanced
	virtual ~FSVideoLayer();
	
			
	float m_width;
	float m_height;

	osg::ref_ptr<osg::Geometry>			m_geometry;

private:

	//build the actual drawable object and attach to the rendering system			
	osg::ref_ptr<osg::Projection> buildLayer();
	//build the actual geometry of the object
	osg::ref_ptr<osg::Geode> buildLayerGeometry();

	//draw matrices for ortho projection
	osg::ref_ptr<osg::MatrixTransform>	m_layerModelViewMatrix;
	osg::ref_ptr<osg::Projection>		m_layerProjectionMatrix;
	//actual verts etc
	osg::ref_ptr<osg::Geode>			m_layerGeode;
};


};