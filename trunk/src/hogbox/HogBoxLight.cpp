#include <hogbox/HogBoxLight.h>
#include <osg/BlendEquation>
#include <osg/TexMat>

//#include "OsgComposite.h"

using namespace hogbox;

/////////////////////////////////////LIGHT////////////////////////////////

HogBoxLight::HogBoxLight(int id) 
	: osg::Object(),
	m_glID(id),
	m_polygonOffset(NULL),
	m_shadowMap(NULL),
	m_shadowMatrix(NULL),
	m_lightPos(osg::Vec4(0.0f,0.0f,0.0f,0.0f)),
	m_diffuseColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f)),
	m_lightAmbi(osg::Vec4(0.1f,0.1f,0.1f,1.0f)),
	m_lightSpec(osg::Vec4(1.0f,1.0f,1.0f,1.0f))
{
	//create required osg objects and states
	
	//create light, ID has to be set manually
	m_light = new osg::Light();
	 m_light->setLightNum(m_glID);
	m_light->setPosition(osg::Vec4(0,0,0,1));

	//create the transform to attach the light to
	m_transform = new osg::MatrixTransform();
	m_transform->setMatrix( osg::Matrix::translate(m_lightPos.x(), m_lightPos.y(), m_lightPos.z()) );

	m_lightSource = new osg::LightSource();
	m_lightSource->setLight(m_light.get());
	m_lightSource->setLocalStateSetModes(osg::StateAttribute::ON); 
 
	//add the lightsource to a transform 
	m_transform->addChild(m_lightSource.get());

	
	std::ostringstream oss;
	oss << "hb_lightPosition" << m_glID;
	//sprintf(name,"light[%d].lightPos", id); 
	m_uLightPos = new osg::Uniform(oss.str().c_str(), m_lightPos);
	
	oss.str("");
	
	oss << "hb_lightColor" << m_glID;
	m_uLightColor = new osg::Uniform(oss.str().c_str(), m_diffuseColor);
	
	oss.str("");
	
	//sprintf(name,"light[%d].constant", id); 
	//m_uConstant = new osg::Uniform(name, constant);
	
	//sprintf(name,"light[%d].linear", id); 
	//m_uLinear = new osg::Uniform(name, lin);
	
	//sprintf(name,"light[%d].quadratic", id); 
	//m_uQuadratic = new osg::Uniform(name, quad);

	//delete [] name;

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
	m_lightSource->setStateSetModes(*root->getOrCreateStateSet() ,osg::StateAttribute::ON);
#else
	AttachUniformsToStateSet(root->getOrCreateStateSet());
#endif
}


void HogBoxLight::SetPosition(const osg::Vec4& pos)
{
	m_lightPos=pos;
	m_transform->setMatrix( osg::Matrix::translate(osg::Vec3(pos.x(),pos.y(),pos.z())) );   
	m_light->setPosition(osg::Vec4(0,0,0,pos.w()));
	m_light->setPosition(pos);
	
	m_uLightPos->set(m_lightPos);
}

void HogBoxLight::SetDiffuse(const osg::Vec4& color)
{
	m_diffuseColor = color;
    m_light->setDiffuse(color);
}

void HogBoxLight::SetAmbient(const osg::Vec4& ambi)
{
	m_lightAmbi = ambi;
	m_light->setAmbient(ambi);
}

void HogBoxLight::SetSpecular(const osg::Vec4& spec)
{
	m_lightSpec = spec;
	m_light->setSpecular(spec); 
}

void HogBoxLight::SetConstant(const float& constant)
{
	m_constant = constant;
	m_light->setConstantAttenuation(constant);

}

void HogBoxLight::SetLinear(const float& linear)
{
	m_linear = linear;
	m_light->setLinearAttenuation(linear);
}

void HogBoxLight::SetQuadratic(const float& quad)
{
	m_quadratic = quad;
	m_light->setQuadraticAttenuation(quad); 
}

//
// Attach a model node tot the lights transform so the 
// light can be visualised
//
void HogBoxLight::AttachVisNode(osg::Node* node)
{
	m_visNode=node;
	m_transform->addChild(node); 
}

void HogBoxLight::AttachUniformsToStateSet(osg::StateSet* state)
{
	state->addUniform( m_uLightPos, osg::StateAttribute::ON);
	state->addUniform( m_uLightColor, osg::StateAttribute::ON);
//	state->addUniform( m_uConstant, osg::StateAttribute::ON);
//	state->addUniform( m_uLinear, osg::StateAttribute::ON);
//	state->addUniform( m_uQuadratic, osg::StateAttribute::ON);
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
    
    m_shadowMap = new osg::Texture2D;
    m_shadowMap->setTextureSize(tex_width, tex_height);

    m_shadowMap->setInternalFormat(GL_DEPTH_COMPONENT);
    m_shadowMap->setShadowComparison(true);
    m_shadowMap->setShadowTextureMode(Texture::LUMINANCE);
	m_shadowMap->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);//LINEAR);
    m_shadowMap->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    
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


        m_factor = 0.1f;
        m_units = 1.0f;

		//if( m_polygonOffset==NULL)
		m_polygonOffset = new PolygonOffset;
		m_polygonOffset->setFactor(m_factor);
        m_polygonOffset->setUnits(m_units);
        _local_stateset->setAttribute(m_polygonOffset.get(), StateAttribute::ON | StateAttribute::OVERRIDE);
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
        camera->attach(osg::CameraNode::DEPTH_BUFFER, m_shadowMap.get());

        // add subgraph to render
        camera->addChild(root);
        
        group->addChild(camera);
        
        // create the texgen node to project the tex coords onto the subgraph
		osg::ref_ptr<osg::TexGenNode> texgenNode = new osg::TexGenNode;
        texgenNode->setTextureUnit(coordGenUnit);
        group->addChild(texgenNode.get());

		m_shadowMatrix = new osg::TexMat();
		

        // set an update callback to keep moving the camera and tex gen in the right direction.
        group->setUpdateCallback(new UpdateCameraAndTexGenCallback(m_transform.get(), camera, texgenNode.get(), m_shadowMatrix.get()));
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
	if(m_polygonOffset!=NULL)
	{
		m_units = units;
		m_polygonOffset->setUnitsMultiplier(m_units);	
	}
}
void HogBoxLight::SetPolyFactor(const float& factor)
{
	if(m_polygonOffset!=NULL)
	{
		m_factor = factor;
		m_polygonOffset->setFactor(m_factor);
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

