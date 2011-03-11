#pragma once

//////////////////////////////////////////////////////////
// Author:	Thomas Hogarth								//
// Date:	12/09/2007									//
//														//
// Class:												//
// Osg object helper class								//
//														//
// Description:											//
// Wraps a set of osg nodes to allow easy access to the //
// nodes geometry meshes etc.							//
// HogBoxMaterials can then be directly applied to the  //
// geometries using their name in the osg graph         //
//                                                      //
//////////////////////////////////////////////////////////

#include <hogbox/Export.h>

#include <osg/MatrixTransform>
#include <osg/Matrix>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>

#include <osgDB/ReadFile>

#include <list>

#include <hogbox/HogBoxMesh.h>
#include <hogbox/HogBoxMaterial.h>

namespace hogbox {

const osg::Vec3 Xaxis(1.0f,0.0f,0.0f);
const osg::Vec3 Yaxis(0.0f,1.0f,0.0f);
const osg::Vec3 Zaxis(0.0f,0.0f,1.0f);


typedef std::list< osg::Geode* > SubGeodeList;

//
// Wrapper for a 3d model exported from 3ds max
//
class HOGBOX_EXPORT HogBoxObject : public osg::Object
{
public:

	HogBoxObject(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxObject(const HogBoxObject&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
	
	META_Box(hogbox,HogBoxObject);


	//returns the root node to allow this object to be added
	//to a parent graph/world
	virtual osg::ref_ptr<osg::MatrixTransform> GetRootNode(){return m_root;}

	//Attaches the node to our localTransfrom for rendering 
	//then passes all maeshmapping visitors down the new subgraph
	//to check for any mapto nodes
	bool AddNodeToObject(osg::ref_ptr<osg::Node> node);

	//get and set visibility / node mask of the
	//root object
	void SetVisible(const bool& vis);
	bool GetVisible() const;

	osg::Node* GetNodeByName(const std::string& name, const bool& subString);

	void SetSubMeshVisible(int index, bool vis);
	bool GetSubMeshVisible(int index);
	void SetSubMeshVisible(const std::string& name, const bool& vis);
	bool GetSubMeshVisible(const std::string& name);

//Transforms
	//Set the world transform
	void SetWorldTransform(const osg::Matrix& trans);
	//Get the world transform
	const osg::Matrix& GetWorldTransform()const;

	//Set the scale on each axis for the local transfrom
	void SetLocalScale(const osg::Vec3& scale);
	void SetLocalScale(const double& x, const double& y, const double& z);
	//sets a uniform scale for local transform, all same value
	void SetLocalScale(const double& u);
	const osg::Vec3& GetLocalScale()const{return m_scale;}

	//scale the file was loaded at
	const float& GetLocalLoadScale()const{return m_loadScale;}

	//set the local trans rotation for each axis
	void SetLocalRotation(const osg::Vec3& rot);
	void SetLocalRotationRadians(const osg::Vec3& rot);
	void SetLocalRotation(const double& x, const double& y, const double& z);
	void SetLocalRotationRadians(const double& x, const double& y, const double& z);
	const osg::Vec3& GetLocalRotation() const{return m_rotDegrees;}
	const osg::Vec3 GetLocalRotationRadians()const{
		osg::Vec3 radians;
		radians.x() = osg::DegreesToRadians(m_rotDegrees.x());
		radians.y() = osg::DegreesToRadians(m_rotDegrees.y());
		radians.z() = osg::DegreesToRadians(m_rotDegrees.z());
		return radians;
	}

	//set the translation of the local transform for each axis
	void SetLocalTranslation(const osg::Vec3& pos);
	void SetLocalTranslation(const double& x, const double& y, const double& z);
	const osg::Vec3& GetLocalTranslation() const{return m_position;}

	//bounds of object including scale matrices etc
	const osg::BoundingSphere GetTotalBounds(){return m_totalBounds;}
	//bounds of the unscaled geodes
	const osg::BoundingSphere GetGeodeBounds(){return m_geodeBounds;}


//Material stuff
	void ApplyMaterialToSubObject(const std::string name, HogBoxMaterial* mat);
	void GenerateTangentSpaceVectors();


//Models
	//get set the vector of nodes being wrapped by this object
	NodePtrVector GetWrappedNodes() const;
	void SetWrappedNodes(const NodePtrVector& nodes);

//MeshMappings
	
	//add a mesh mapping to our list, the added mappings will be passed
	//across all existing wrapped nodes to apply the mapping
	bool AddMeshMapping(MeshMapping* mapping);

	//get set the list of mesh mappings
	std::vector<MeshMappingPtr> GetMeshMappings() const;
	void SetMeshMappings(const std::vector<MeshMappingPtr>& mappings);

protected:

	virtual ~HogBoxObject(void);


	//Apply the translate rotate and scale values to thier matrices
	void UpdateLocalScaleMatrix();
	void UpdateLocalRotationMatrix();
	void UpdateLocalTranslationMatrix();

	//Apply the translate rotate and scale matrices to our
	//local transform node
	void UpdateLocalTransform();

protected:

	//the list of osg nodes that have been added to the object
	//to be wrapped and controlled.
	hogbox::NodePtrVector m_wrappedNodes;

	//root node for the object/model is also the world space transform
	osg::ref_ptr<osg::MatrixTransform> m_root;

	//osg::ref_ptr<osg::Node> m_model; // node storing a referance to the loaded or generated model

	//transformation members
	osg::ref_ptr<osg::MatrixTransform> m_localTransform; // the final world transform matrix for the model

	//local sub mats
	osg::Matrix m_scaleMat;		// the scale matrix used to set the local transform
	osg::Matrix m_rotationMat;	// the rotation matrix used to set the local transform
	osg::Matrix m_translationMat; // the translation matrix used for the loacl transform

	//local vars to set mats
	osg::Vec3 m_scale;
	osg::Vec3 m_rotDegrees;
	osg::Vec3 m_position;

	float m_loadScale; //the uniform scale appended to a fille name
	double m_size; //the radius of the mnodels bounding sphere at load time

	//bounding sphere of entire model
	osg::BoundingSphere m_totalBounds; 
	//bounds of just geometry (no lights etc)
	osg::BoundingSphere m_geodeBounds; 

	//the list of mesh mappings to apply to this object (and all attched model nodes)
	std::vector<MeshMappingPtr> m_meshMappings;

	int m_nodeMasks;//additional to visible/not

};

typedef osg::ref_ptr<HogBoxObject> HogBoxObjectPtr;

};//end hogbox namespace