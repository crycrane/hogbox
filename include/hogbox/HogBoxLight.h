#pragma once

//needs review still esentially the same code from old hogbox

//////////////////////////////////////////////////////////
// Author:	Thomas Hogarth								//
// Date:	12/09/2007									//
//														//
// Class:												//
// Osg Material and texture helper class				//
//														//
// Description:											//
// Wraps up the functionality of loading and appying	//
// textures and material to a piece of geometry			//
//														//
//////////////////////////////////////////////////////////

#include <hogbox/Export.h>
#include <hogbox/HogBoxBase.h>

#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Matrix>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/HighlightMapGenerator>

//stuff for lights
#include <osg/Light>
#include <osg/LightSource>

//stuff for shadow map
#include <osg/Projection>
#include <osg/TexGen>
#include <osg/Texture2D>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/CameraNode>
#include <osg/TexGenNode>
#include <osg/TexMat>

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace osg;

namespace hogbox {

//
// Update callback for the rtt camera used for the shadow map
//
class UpdateCameraAndTexGenCallback : public osg::NodeCallback
{
    public:
    
		UpdateCameraAndTexGenCallback(osg::MatrixTransform* light_transform, osg::CameraNode* cameraNode, osg::TexGenNode* texgenNode, osg::TexMat* shadowMat):
            _light_transform(light_transform),
            _cameraNode(cameraNode),
            _texgenNode(texgenNode),
			_shadowMat(shadowMat)
        {
        }
       
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            // first update subgraph to make sure objects are all moved into postion
            traverse(node,nv);
            
            // now compute the camera's view and projection matrix to point at the shadower (the camera's children)
            osg::BoundingSphere bs;
            for(unsigned int i=0; i<_cameraNode->getNumChildren(); ++i)
            {
                bs.expandBy(_cameraNode->getChild(i)->getBound());
            }
            
            if (!bs.valid())
            {
                osg::notify(osg::WARN) << "bb invalid"<<_cameraNode.get()<<std::endl;
                return;
            }
            
            osg::Vec3 position = _light_transform->getMatrix().getTrans();

            float centerDistance = (position-bs.center()).length();

            float znear = centerDistance-bs.radius();
            float zfar  = centerDistance+bs.radius();
            float zNearRatio = 0.001f;
            if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

#if 0
            // hack to illustrate the precision problems of excessive gap between near far range.
            znear = 0.00001*zfar;
#endif
            float top   = (bs.radius()/centerDistance)*znear;
            float right = top;

            _cameraNode->setReferenceFrame(osg::CameraNode::ABSOLUTE_RF);
            _cameraNode->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
			_cameraNode->setViewMatrixAsLookAt(position, bs.center(), osg::Vec3(0.0f,1.0f,0.0f));

            // compute the matrix which takes a vertex from local coords into tex coords
            // will use this later to specify osg::TexGen..
            osg::Matrix MVPT = _cameraNode->getViewMatrix() * 
                               _cameraNode->getProjectionMatrix() *
                               osg::Matrix::translate(1.0,1.0,1.0) *
                               osg::Matrix::scale(0.5f,0.5f,0.5f);
                               
            _texgenNode->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
            _texgenNode->getTexGen()->setPlanesFromMatrix(MVPT);

			_shadowMat->setMatrix(MVPT);

        }
        
    protected:
    
        virtual ~UpdateCameraAndTexGenCallback() {}
        
        osg::ref_ptr<osg::MatrixTransform>  _light_transform;
        osg::ref_ptr<osg::CameraNode>       _cameraNode;
        osg::ref_ptr<osg::TexGenNode>       _texgenNode;
		osg::ref_ptr<osg::TexMat>			_shadowMat;

};

class HOGBOX_EXPORT HogBoxLight : public osg::Object
{
public:

	HogBoxLight(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxLight(const HogBoxLight&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
	META_Box(hogbox,HogBoxLight);


	osg::MatrixTransform* GetLight(){return m_transform.get();}
	osg::MatrixTransform* GetMatrix(){return m_transform.get();} 
	void SetLight(int id, osg::Vec4 pos, osg::Vec4 colour, float constant, float lin, float quad);
	
	void ApplyLightToGraph(osg::Node* root); 
	osg::Group* CreateShadowMap(osg::Group* root, unsigned int coordGenUnit, int size);

	osg::Texture2D* GetShadowMap(){return m_shadowMap.get();}
	osg::Texture2D* GetOrCreateShadowMap()
	{
		if(!m_shadowMap)
		{
			m_shadowMap = new osg::Texture2D();
		}
		return m_shadowMap.get();
	}

	osg::TexMat* GetShadowMarix(){return m_shadowMatrix.get();}
	osg::TexMat* GetOrCreateShadowMarix()
	{
		if(!m_shadowMatrix)
		{
			m_shadowMatrix = new osg::TexMat();
		}
		return m_shadowMatrix.get();
	}

	osg::PolygonOffset* GetOrCreatePolygonOffset()
	{
		if(!m_polygonOffset)
		{
			m_polygonOffset = new osg::PolygonOffset();
		}
		return m_polygonOffset.get();
	}

	const float& GetPolyUnits()const{return m_units;}
	void SetPolyUnits(const float& units);
	const float& GetPolyFactor()const{return m_factor;}
	void SetPolyFactor(const float& factor);

	void AttachUniformsToStateSet(osg::StateSet* state);


	//fixed function atts

	const int& GetGLID()const{return m_glID;}
	void SetGLID(const int& id);

	const osg::Vec4& GetPosition()const{return m_lightPos;}
	void SetPosition(const osg::Vec4& pos);
	
	const osg::Vec4& GetDiffuse()const{return m_diffuseColor;}
	const osg::Vec4& GetAmbient()const{return m_lightAmbi;}
	const osg::Vec4& GetSpecular()const{return m_lightSpec;}

	void SetDiffuse(const osg::Vec4& color);
	void SetAmbient(const osg::Vec4& ambi);
	void SetSpecular(const osg::Vec4& spec);
	
	const float& GetConsant()const{return m_constant;}
	const float& GetLinear()const{return m_linear;}
	const float& GetQuadratic()const{return m_quadratic;}

	void SetConstant(const float& constant);
	void SetLinear(const float& linear);
	void SetQuadratic(const float& quad);

	void AttachVisNode(osg::Node* node); 

protected:
	virtual ~HogBoxLight(void);
protected:

	int m_glID;

	osg::ref_ptr<osg::Light> m_light;
	osg::ref_ptr<osg::LightSource> m_lightSource;
	osg::ref_ptr<osg::MatrixTransform> m_transform; 

	// Actual light data.
	osg::Vec4 m_diffuseColor;
	osg::Vec4 m_lightAmbi;
	osg::Vec4 m_lightSpec;
	osg::Vec4 m_lightPos;
	float m_constant, m_linear, m_quadratic;

	// Light shader parameters.
	osg::ref_ptr<osg::Uniform> m_uLightPos;
	osg::ref_ptr<osg::Uniform> m_uLightColor;
	osg::ref_ptr<osg::Uniform> m_uConstant;
	osg::ref_ptr<osg::Uniform> m_uLinear;
	osg::ref_ptr<osg::Uniform> m_uQuadratic;

	//visualiseation node
	osg::ref_ptr<osg::Node> m_visNode;

	//shadows from light source 
	osg::ref_ptr<osg::Texture2D> m_shadowMap;
	osg::ref_ptr<osg::TexMat> m_shadowMatrix;

	//poly offset for depth render
	osg::ref_ptr<osg::PolygonOffset> m_polygonOffset;
	float m_units;
	float m_factor;

};

typedef osg::ref_ptr<HogBoxLight> HogBoxLightPtr;


//Transform callback for lights

class LightTransformCallback: public osg::NodeCallback
{

public:

	LightTransformCallback(osg::Uniform* lightPos, float angular_velocity, float height, float radius):
    _angular_velocity(angular_velocity),
	_posUniform(lightPos),
    _height(height),
    _radius(radius),
    _previous_traversal_number(-1),
    _previous_time(-1.0f),
    _angle(0)
  {
  }

  void operator()(Node* node, NodeVisitor* nv);

protected:
    
  float                                  _angular_velocity;
  float                                  _height;
  float                                  _radius;
  int                                    _previous_traversal_number;
  double                                 _previous_time;
  float                                  _angle;
  //the handle to this lights position uniform used for pp lighting
  osg::Uniform*							_posUniform; 
};


}; //end hogbox namespace