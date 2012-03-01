#include <hogbox/HogBoxLight.h>
#include <osg/BlendEquation>
#include <osg/TexMat>

//#include "OsgComposite.h"

using namespace hogbox;

/////////////////////////////////////LIGHT////////////////////////////////

HogBoxLight::HogBoxLight(int id) 
	: osg::Object(),
	_glID(id),
	_position(osg::Vec4(0.0f,0.0f,0.0f,0.0f)),
    _direction(osg::Vec3(0,1,0)),
	_diffuseColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f)),
	_ambientColor(osg::Vec4(0.1f,0.1f,0.1f,1.0f)),
	_specularColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f)),
    _constant(1.0f),
    _linear(1.0f),
    _quadratic(1.0f),
    _shadowMap(NULL),
    _shadowMatrix(NULL),
    _polygonOffset(NULL)
{
	//create required osg objects and states
	
    //create light, ID has to be set manually
    _light = new osg::Light();
    _light->setLightNum(_glID);
    _light->setPosition(osg::Vec4(0,0,0,1));

	//create the transform to attach the light to
	_transform = new osg::MatrixTransform();
	_transform->setMatrix( osg::Matrix::translate(_position.x(), _position.y(), _position.z()) );

	_lightSource = new osg::LightSource();
	_lightSource->setLight(_light.get());
	_lightSource->setLocalStateSetModes(osg::StateAttribute::ON); 
 
	//add the lightsource to a transform 
	_transform->addChild(_lightSource.get());

	
	std::ostringstream oss;
	oss << "hb_lights[" << _glID << "].position";
	_uPosition = new osg::Uniform(oss.str().c_str(), _position);
	
	oss.str("");
	
	oss << "hb_lights[" << _glID << "].ambient";
	_uAmbientColor = new osg::Uniform(oss.str().c_str(), _ambientColor);
	
	oss.str("");
    
	oss << "hb_lights[" << _glID << "].diffuse";
	_uDiffuseColor = new osg::Uniform(oss.str().c_str(), _diffuseColor);
	
	oss.str("");
    
	oss << "hb_lights[" << _glID << "].specular";
	_uSpecularColor = new osg::Uniform(oss.str().c_str(), _specularColor);
	
	oss.str("");
    
	oss << "hb_lights[" << _glID << "].constant";
	_uConstant = new osg::Uniform(oss.str().c_str(), _constant);
	
	oss.str("");
    
	oss << "hb_lights[" << _glID << "].linear";
	_uLinear = new osg::Uniform(oss.str().c_str(), _linear);
	
	oss.str("");
    
	oss << "hb_lights[" << _glID << "].quadratic";
	_uQuadratic = new osg::Uniform(oss.str().c_str(), _quadratic);
	
	oss.str("");

}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
HogBoxLight::HogBoxLight(const HogBoxLight& light,const osg::CopyOp& copyop) 
	: osg::Object(light, copyop)
{

}

HogBoxLight::~HogBoxLight(void)
{
	OSG_NOTICE << "    Deallocating HogBoxLight: Name '" << this->getName() << "'." << std::endl;
}

void HogBoxLight::ApplyLightToGraph(osg::Node* root)
{
	if(!root){return;}
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
	_lightSource->setStateSetModes(*root->getOrCreateStateSet() ,osg::StateAttribute::ON);
#else
	AttachUniformsToStateSet(root->getOrCreateStateSet());
#endif
}


void HogBoxLight::SetPosition(const osg::Vec4& pos)
{
	_position=pos;
	_transform->setMatrix( osg::Matrix::translate(osg::Vec3(pos.x(),pos.y(),pos.z())) );   
	_light->setPosition(osg::Vec4(0,0,0,pos.w()));
	_light->setPosition(pos);
	
	_uPosition->set(_position);
}

void HogBoxLight::SetDiffuse(const osg::Vec4& color)
{
	_diffuseColor = color;
    _light->setDiffuse(color);
    _uDiffuseColor->set(color);
}

void HogBoxLight::SetAmbient(const osg::Vec4& ambi)
{
	_ambientColor = ambi;
	_light->setAmbient(ambi);
    _uAmbientColor->set(ambi);
}

void HogBoxLight::SetSpecular(const osg::Vec4& spec)
{
	_specularColor = spec;
	_light->setSpecular(spec); 
    _uSpecularColor->set(spec);
}

void HogBoxLight::SetConstant(const float& constant)
{
	_constant = constant;
	_light->setConstantAttenuation(constant);
    _uConstant->set(constant);
}

void HogBoxLight::SetLinear(const float& linear)
{
	_linear = linear;
	_light->setLinearAttenuation(linear);
    _uLinear->set(linear);
}

void HogBoxLight::SetQuadratic(const float& quad)
{
	_quadratic = quad;
	_light->setQuadraticAttenuation(quad);
    _uQuadratic->set(quad);
}

//
// Attach a model node tot the lights transform so the 
// light can be visualised
//
void HogBoxLight::AttachVisNode(osg::Node* node)
{
	_visNode=node;
	_transform->addChild(node); 
}

void HogBoxLight::AttachUniformsToStateSet(osg::StateSet* state)
{
    if(!state){return;}
	state->addUniform( _uPosition, osg::StateAttribute::ON);
	state->addUniform( _uAmbientColor, osg::StateAttribute::ON);
    state->addUniform( _uDiffuseColor, osg::StateAttribute::ON);
    state->addUniform( _uSpecularColor, osg::StateAttribute::ON);
	state->addUniform( _uConstant, osg::StateAttribute::ON);
	state->addUniform( _uLinear, osg::StateAttribute::ON);
	state->addUniform( _uQuadratic, osg::StateAttribute::ON);
}


//
// create the shadow map render to texture systen
// pass in the root node, add it to a rtt, 
// tex gen cords and the original root are then attached
// to a new group and returned
//

osg::Group* HogBoxLight::CreateShadowMap(osg::Group* root, unsigned int coordGenUnit, int size)
{
	return NULL;
//	return COsgComposite::CreateShadowMap(root, this, coordGenUnit, size);
	/*
    osg::Group* group = new osg::Group;
    
    unsigned int tex_width = size;
    unsigned int tex_height = size;
    
    _shadowMap = new osg::Texture2D;
    _shadowMap->setTextureSize(tex_width, tex_height);

    _shadowMap->setInternalFormat(GL_DEPTH_COMPONENT);
    _shadowMap->setShadowComparison(true);
    _shadowMap->setShadowTextureMode(Texture::LUMINANCE);
	_shadowMap->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);//LINEAR);
    _shadowMap->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    
    // set up the render to texture camera.
    {

        // create the camera
        osg::CameraNode* camera = new osg::CameraNode;

        camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        camera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        camera->setComputeNearFarMode(osg::CameraNode::DO_NOT_COMPUTE_NEAR_FAR);

        // set viewport
        camera->setViewport(0,0,tex_width,tex_height);

        osg::StateSet*  _local_stateset = camera->getOrCreateStateSet();

        _local_stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);


        _factor = 0.1f;
        _units = 1.0f;

		//if( _polygonOffset==NULL)
		_polygonOffset = new PolygonOffset;
		_polygonOffset->setFactor(_factor);
        _polygonOffset->setUnits(_units);
        _local_stateset->setAttribute(_polygonOffset.get(), StateAttribute::ON | StateAttribute::OVERRIDE);
        _local_stateset->setMode(GL_POLYGON_OFFSET_FILL, StateAttribute::ON | StateAttribute::OVERRIDE);

        ref_ptr<CullFace> cull_face = new CullFace;
        cull_face->setMode(CullFace::FRONT);
        _local_stateset->setAttribute(cull_face.get(), StateAttribute::ON | StateAttribute::OVERRIDE);
        _local_stateset->setMode(GL_CULL_FACE, StateAttribute::ON | StateAttribute::OVERRIDE);


        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::CameraNode::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::CameraNode::DEPTH_BUFFER, _shadowMap.get());

        // add subgraph to render
        camera->addChild(root);
        
        group->addChild(camera);
        
        // create the texgen node to project the tex coords onto the subgraph
		osg::ref_ptr<osg::TexGenNode> texgenNode = new osg::TexGenNode;
        texgenNode->setTextureUnit(coordGenUnit);
        group->addChild(texgenNode.get());

		_shadowMatrix = new osg::TexMat();
		

        // set an update callback to keep moving the camera and tex gen in the right direction.
        group->setUpdateCallback(new UpdateCameraAndTexGenCallback(_transform.get(), camera, texgenNode.get(), _shadowMatrix.get()));
    }
    
	//return the new root node
	group->addChild(root);
	//root->addChild(group);  
    return group;*/
}

//
// Set the polygon offset values for the depth render
//
void HogBoxLight::SetPolyUnits(const float& units)
{
	if(_polygonOffset!=NULL)
	{
		_units = units;
		_polygonOffset->setUnitsMultiplier(_units);	
	}
}
void HogBoxLight::SetPolyFactor(const float& factor)
{
	if(_polygonOffset!=NULL)
	{
		_factor = factor;
		_polygonOffset->setFactor(_factor);
	}
}
///////////////////////////////////END LIGHT//////////////////////////////

/////////////////////////////////Light transform callbacks/////////////////

//
// Callback for light to be applied to its transform node 
//
void LightTransformCallback::operator()(Node* node, NodeVisitor* nv)
{
  MatrixTransform* transform = dynamic_cast<MatrixTransform*>(node);
  if (nv && transform)
    {
      const FrameStamp* fs = nv->getFrameStamp();
      if (!fs) return; // not frame stamp, no handle on the time so can't move.
        
      double new_time = fs->getReferenceTime();
      if (nv->getTraversalNumber() != _previous_traversal_number)
        {
          _angle += _angular_velocity * (new_time - _previous_time);

          Matrix matrix = Matrix::rotate(atan(_height / _radius), -X_AXIS) *
            Matrix::rotate(PI_2, Y_AXIS) *
            Matrix::translate(Vec3(_radius, 0, 0)) *
            Matrix::rotate(_angle, Y_AXIS) *
            Matrix::translate(Vec3(0, _height, 0));

		  Vec3 pos = matrix.getTrans()+Vec3(0,0,0);
		  _posUniform->set(Vec4(pos.x(),pos.y(),pos.z(),0.0f)); 
          // update the specified transform
          transform->setMatrix(matrix);

          _previous_traversal_number = nv->getTraversalNumber();
        }

      _previous_time = new_time; 
    }

  // must call any nested node callbacks and continue subgraph traversal.
  traverse(node,nv);
}

