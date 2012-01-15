#include <hogboxVision/VideoLayer.h>

#include <hogbox/HogBoxBase.h>
#include <osg/Depth>

using namespace hogboxVision;

Ortho2DLayer::Ortho2DLayer(int width, int height, RenderStage stage) 
	: osg::Group(),
    _renderStage(stage),
	_width(width), 
	_height(height),
	_rotDegrees(0.0f),
	_orientation(0),
	_hFlip(false),
	_vFlip(false)
{
	// build the layer as a child of this group
	this->addChild(buildLayer().get());
}


Ortho2DLayer::Ortho2DLayer(const Ortho2DLayer& layer, const osg::CopyOp& copyop)
{

}


Ortho2DLayer::~Ortho2DLayer()
{	

}

//
// Build the gemo and contruct the orth projectipon matrices
//
osg::ref_ptr<osg::Camera> Ortho2DLayer::buildLayer() 
{

	float hWidth = _width * 0.5f;
	float hHeight = _height * 0.5f;
	
	_camera = new osg::Camera();
	
    _camera->setClearColor(osg::Vec4(1,0,0,1));
    
    // set the projection matrix
    _camera->setProjectionMatrix(osg::Matrix::ortho2D(-hWidth, hWidth, -hHeight, hHeight));
	
    // set the view matrix    
    _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _camera->setViewMatrix(osg::Matrix::identity());
	
    // only clear the depth buffer 
    _camera->setClearMask(0);//GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    // draw hud after main camera view.
    if(_renderStage == ORTHO_BACKGROUND){
        _camera->setRenderOrder(osg::Camera::PRE_RENDER);
    }else if(_renderStage == ORTHO_NESTED){
        _camera->setRenderOrder(osg::Camera::NESTED_RENDER);
    }else if(_renderStage == ORTHO_OVERLAY){
        _camera->setRenderOrder(osg::Camera::POST_RENDER);
    }
	
	osg::Group* layerGroup = new osg::Group();

    //set the default draw mask
    layerGroup->setNodeMask(hogbox::MAIN_CAMERA_CULL);
	_camera->addChild(layerGroup);
    

	layerGroup->addChild(buildLayerGeometry().get());

	return _camera;
}

//
//Build quad with 0,0 at center
//
osg::ref_ptr<osg::Geode> Ortho2DLayer::buildLayerGeometry() 
{
	float minU = 0.0f;
	float maxU = 1.0f;//m_width;//1.0f
	float minV = 0.0f;
	float maxV = 1.0f;//m_height;//1.0f;

	_layerGeode = new osg::Geode();

	_geometry = new osg::Geometry();
	
	osg::Vec3Array* coords = new osg::Vec3Array();
	_geometry->setVertexArray(coords);

	osg::Vec2Array* tcoords = new osg::Vec2Array();
	_geometry->setTexCoordArray(0, tcoords);

	float hWidth = _width * 0.5f;
	float hHeight = _height * 0.5f;

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
	

	_geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    osg::ref_ptr<osg::Vec4Array> quad_colors = new osg::Vec4Array;
    quad_colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    _geometry->setColorArray(quad_colors.get());
    _geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	
    // diable depth read and write and culling for fastest render   
    this->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
    this->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    osg::Depth* depth = new osg::Depth();
    depth->setWriteMask(false);
    this->getOrCreateStateSet()->setAttributeAndModes(depth);

	
	_layerGeode->addDrawable(_geometry.get());

	return _layerGeode;

}

//apply all the orientation and transform information
void Ortho2DLayer::ApplyTransforms()
{
	float rads = osg::DegreesToRadians(_rotDegrees);	

	_camera->setViewMatrix(osg::Matrix::rotate(rads, 0,0,1));

	//determin if we have changed the horizontal and vertical (left or right orientation)
	bool isRotated = false;
	if(_orientation == 1 || _orientation == 3)
	{isRotated = true;}
	
	float hWidth = _width * 0.5f;
	float hHeight = _height * 0.5f;
	
	if(isRotated)
	{
		float temp = hWidth;
		hWidth = hHeight;
		hHeight = temp;
	}
	
	hHeight = _vFlip ? -hHeight : hHeight;
	hWidth = _hFlip ? -hWidth : hWidth;
	
     _camera->setProjectionMatrix(osg::Matrix::ortho2D(-hWidth, hWidth, -hHeight, hHeight));
	
}

void Ortho2DLayer::SetRotation(const float& rotDegrees)
{
	_rotDegrees = rotDegrees;
	ApplyTransforms();
}

//
//set the orientation/up side, will set the rotation to increments of 90
//plus also alter the ortho projection ratio. 0 is up, 1 is right, 2 is down
//
void Ortho2DLayer::SetOrientation(const int& orientation)
{
	_orientation = orientation;
	switch (_orientation) {
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

void Ortho2DLayer::SetVerticalFlip(const bool& bVFlip)
{
	_vFlip = bVFlip;
	ApplyTransforms();	
}

void Ortho2DLayer::SetHorizontalFlip(const bool& bHFlip)
{
	_hFlip = bHFlip;
	ApplyTransforms();
}

//
//helper to apply a texture to a particular channel of the geodes stateset
//
void Ortho2DLayer::ApplyTexture(osg::Texture2D* tex, int channel)
{
    this->getOrCreateStateSet()->setTextureAttributeAndModes(channel, tex, osg::StateAttribute::ON);
}


