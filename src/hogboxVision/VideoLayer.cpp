#include <hogboxVision/VideoLayer.h>


#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Depth>
#include <osg/Geometry>
#include <osg/BlendFunc>
#include <osg/Notify>
#include <osg/Image>
#include <osg/TexMat>

#include <hogbox/NPOTResizeCallback.h>

using namespace hogboxVision;

// FSVideoLayer

FSVideoLayer::FSVideoLayer(int width, int height) 
	: osg::Group(),
	m_width(width), 
	m_height(height),
	m_rotDegrees(0.0f),
	m_orientation(0),
	m_hFlip(false),
	m_vFlip(false)
{
	// build the layer as a child of this group
	this->addChild(buildLayer().get());
}


FSVideoLayer::FSVideoLayer(const FSVideoLayer& FSVideoLayer,
	const osg::CopyOp& copyop)
{

}


FSVideoLayer::~FSVideoLayer()
{	

}

//
// Build the gemo and contruct the orth projectipon matrices
//
osg::ref_ptr<osg::CameraNode> FSVideoLayer::buildLayer() 
{

	float hWidth = m_width * 0.5f;
	float hHeight = m_height * 0.5f;
	
	m_camera = new osg::CameraNode;
	
    // set the projection matrix
    m_camera->setProjectionMatrix(osg::Matrix::ortho2D(-hWidth, hWidth, -hHeight, hHeight));
	
    // set the view matrix    
    m_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    m_camera->setViewMatrix(osg::Matrix::identity());
	
    // only clear the depth buffer as this is post render
    m_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	
    // draw hud after main camera view.
    m_camera->setRenderOrder(osg::CameraNode::PRE_RENDER);
	
	osg::Group* layerGroup = new osg::Group();
    //set the default draw mask
    layerGroup->setNodeMask(hogbox::MAIN_CAMERA_CULL);
	m_camera->addChild(layerGroup);

	layerGroup->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::ALWAYS, 1.0f, 1.0f));
	layerGroup->addChild(buildLayerGeometry().get());

	return m_camera;
}

//
//Build quad with 0,0 at center to make fliping easier
//
osg::ref_ptr<osg::Geode> FSVideoLayer::buildLayerGeometry() 
{
	float minU = 0.0f;
	float maxU = 1.0f;//m_width;//1.0f
	float minV = 1.0f;
	float maxV = 0.0f;//m_height;//1.0f;

	m_layerGeode = new osg::Geode();

	m_geometry = new osg::Geometry();
	
	osg::Vec3Array* coords = new osg::Vec3Array();
	m_geometry->setVertexArray(coords);

	osg::Vec2Array* tcoords = new osg::Vec2Array();
	m_geometry->setTexCoordArray(0, tcoords);

	float hWidth = m_width * 0.5f;
	float hHeight = m_height * 0.5f;

	coords->push_back(osg::Vec3(-hWidth, -hHeight, 0.0f));
	coords->push_back(osg::Vec3(hWidth, -hHeight, 0.0f));
	coords->push_back(osg::Vec3(hWidth, hHeight, 0.0f));
	coords->push_back(osg::Vec3(-hWidth, hHeight, 0.0f));
	
	/*coords->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
	coords->push_back(osg::Vec3(m_width, 0.0f, 0.0f));
	coords->push_back(osg::Vec3(m_width, m_height, 0.0f));
	coords->push_back(osg::Vec3(0.0f, m_height, 0.0f));*/

	tcoords->push_back(osg::Vec2(minU, minV));
	tcoords->push_back(osg::Vec2(maxU, minV));
	tcoords->push_back(osg::Vec2(maxU, maxV));
	tcoords->push_back(osg::Vec2(minU, maxV));
	

	m_geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    osg::ref_ptr<osg::Vec4Array> quad_colors = new osg::Vec4Array;
    quad_colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    m_geometry->setColorArray(quad_colors.get());
    m_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	
	//disable depth	
	this->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	//disable culling for flipping
	this->getOrCreateStateSet()->setMode(GL_CULL_FACE,osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

	//
	//this->setStateSet(m_material->GetStateSet());
	
	m_layerGeode->addDrawable(m_geometry.get());

	return m_layerGeode;

}

//apply all the orientation and transform information
void FSVideoLayer::ApplyTransforms()
{
	float rads = osg::DegreesToRadians(m_rotDegrees);	
	//m_layerModelViewMatrix->setMatrix(osg::Matrix::rotate(rads, 0,0,1)); 
	m_camera->setViewMatrix(osg::Matrix::rotate(rads, 0,0,1));
    
	//determin if we have changed the horizontal and vertical (left or right orientation)
	bool isRotated = false;
	if(m_orientation == 1 || m_orientation == 3)
	{isRotated = true;}
	
	float hWidth = m_width * 0.5f;
	float hHeight = m_height * 0.5f;
	
	if(isRotated)
	{
		float temp = hWidth;
		hWidth = hHeight;
		hHeight = temp;
	}
	
	hHeight = m_vFlip ? -hHeight : hHeight;
	hWidth = m_hFlip ? -hWidth : hWidth;
	
     m_camera->setProjectionMatrix(osg::Matrix::ortho2D(-hWidth, hWidth, -hHeight, hHeight));
	//m_layerProjectionMatrix->setMatrix(osg::Matrix::ortho2D(-hWidth, hWidth, -hHeight, hHeight));
	
}

void FSVideoLayer::SetRotation(const float& rotDegrees)
{
	m_rotDegrees = rotDegrees;
	ApplyTransforms();
}

//
//set the orientation/up side, will set the rotation to increments of 90
//plus also alter the ortho projection ratio. 0 is up, 1 is right, 2 is down
//
void FSVideoLayer::SetOrientation(const int& orientation)
{
	m_orientation = orientation;
	switch (m_orientation) {
		case 0://top
			SetRotation(0.0f);
			break;
		case 1://right
			SetRotation(90.0f);
			break;
		case 2://bottom
			SetRotation(180.0f);
			break;
		case 3://left
			SetRotation(270.0f);
			break;
		default:
			break;
	}
}

void FSVideoLayer::SetVerticalFlip(const bool& bVFlip)
{
	m_vFlip = bVFlip;
	ApplyTransforms();	
}

void FSVideoLayer::SetHorizontalFlip(const bool& bHFlip)
{
	m_hFlip = bHFlip;
	ApplyTransforms();
}


