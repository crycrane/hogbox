#include <hogbox/HogBoxLight.h>
#include <osg/BlendEquation>
#include <osg/TexMat>

//#include "OsgComposite.h"

using namespace hogbox;

/////////////////////////////////////LIGHT////////////////////////////////

HogBoxLight::HogBoxLight(void) 
	: osg::Object(),
	m_glID(0),
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
	m_light->setPosition(osg::Vec4(0,0,0,1));

	//create the transform to attach the light to
	m_transform = new osg::MatrixTransform();
	m_transform->setMatrix( osg::Matrix::translate(m_lightPos.x(), m_lightPos.y(), m_lightPos.z()) );

	m_lightSource = new osg::LightSource();
	m_lightSource->setLight(m_light.get());
	m_lightSource->setLocalStateSetModes(osg::StateAttribute::ON); 
 
	//add the lightsource to a transform 
	m_transform->addChild(m_lightSource.get());

	//create the light uniforms
/*	char* name = new char[64];

	sprintf(name,"light[%d].lightPos", id); 
	m_uLightPos = new osg::Uniform(name, pos);
	
	sprintf(name,"light[%d].lightColor", id); 
	m_uLightColor = new osg::Uniform(name, colour);
	
	sprintf(name,"light[%d].constant", id); 
	m_uConstant = new osg::Uniform(name, constant);
	
	sprintf(name,"light[%d].linear", id); 
	m_uLinear = new osg::Uniform(name, lin);
	
	sprintf(name,"light[%d].quadratic", id); 
	m_uQuadratic = new osg::Uniform(name, quad);

	delete [] name;*/

}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
HogBoxLight::HogBoxLight(const HogBoxLight& light,const osg::CopyOp& copyop) 
	: osg::Object(light, copyop)
{

}

HogBoxLight::~HogBoxLight(void)
{

}

//
// Set a light, it is also given a unique id between 0 and MAXLIGHTS
//
void HogBoxLight::SetLight(int id, osg::Vec4 pos, osg::Vec4 colour, float constant, float lin, float quad)
{
	// Actual light data.
	m_diffuseColor = colour;
//	m_lightAmbi=colour*0.001f;
//	m_lightSpec=colour;


	m_lightPos = pos;
	m_constant = constant; m_linear = lin; m_quadratic = quad;

	//create and set the light transform
	m_transform = new osg::MatrixTransform();
	m_transform->setMatrix( osg::Matrix::translate(pos.x(), pos.y(), pos.z()) );   

	//tore the id
	//SetNameID(id);

	char* name = new char[64];

		sprintf(name,"light[%d].lightPos", id); 
		m_uLightPos = new osg::Uniform(name, pos);
		
		sprintf(name,"light[%d].lightColor", id); 
		m_uLightColor = new osg::Uniform(name, colour);
		
		sprintf(name,"light[%d].constant", id); 
		m_uConstant = new osg::Uniform(name, constant);
		
		sprintf(name,"light[%d].linear", id); 
		m_uLinear = new osg::Uniform(name, lin);
		
		sprintf(name,"light[%d].quadratic", id); 
		m_uQuadratic = new osg::Uniform(name, quad);

		delete [] name;

	//create and set the lights attributes
	m_light = new osg::Light();
    m_light->setLightNum(id);
	m_light->setPosition(osg::Vec4(0,0,0,1));
	m_light->setAmbient(m_lightAmbi);
    m_light->setDiffuse(m_diffuseColor);
	m_light->setSpecular(m_lightSpec); 
	m_light->setConstantAttenuation(constant);
	m_light->setLinearAttenuation(lin);
	m_light->setQuadraticAttenuation(quad); 

	m_lightSource = new osg::LightSource();
	m_lightSource->setLight(m_light.get());
	m_lightSource->setLocalStateSetModes(osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 
 
	//add the lightsource to a transform 
	m_transform->addChild(m_lightSource.get());

}

void HogBoxLight::ApplyLightToGraph(osg::Node* root)
{
	if(!root){return;}
	m_lightSource->setStateSetModes(*root->getOrCreateStateSet() ,osg::StateAttribute::ON);
}

void HogBoxLight::SetGLID(const int& id)
{
	m_glID = id;
	m_light->setLightNum(m_glID);
}

void HogBoxLight::SetPosition(const osg::Vec4& pos)
{
	m_lightPos=pos;
	m_transform->setMatrix( osg::Matrix::translate(osg::Vec3(pos.x(),pos.y(),pos.z())) );   
	//m_light->setPosition(osg::Vec4(0,0,0,pos.w()));
	m_light->setPosition(pos);

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
	state->addUniform( m_uConstant, osg::StateAttribute::ON);
	state->addUniform( m_uLinear, osg::StateAttribute::ON);
	state->addUniform( m_uQuadratic, osg::StateAttribute::ON);
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

