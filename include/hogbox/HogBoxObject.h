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
	//to a parent graph
	virtual osg::ref_ptr<osg::Group> GetRootNode(){return m_root;}

	//Attaches the node to our localTransfrom for rendering 
	//then passes all maeshmapping visitors down the new subgraph
	//to check for any mapto nodes
	bool AddNodeToObject(osg::ref_ptr<osg::Node> node);

	//see if the object needs updating
	bool NeedUpdating(){return m_needUpdating;}
	//force an update, (HACK till we have our update callback system inplace we will just update matrix on dirty)
	void Dirty(){UpdateLocalTransform();m_needUpdating=true;}
	void Clean(){m_needUpdating = false;}

	//get and set visibility / node mask of the
	//root object
	void SetVisible(const bool& vis);
	bool GetVisible() const;

	void SetSubMeshVisible(int index, bool vis);
	bool GetSubMeshVisible(int index);
	void SetSubMeshVisible(std::string name, bool vis);
	bool GetSubMeshVisible(std::string name);


	//Set the scale on each axis for the local transfrom
	void SetScale(const osg::Vec3& scale);
	void SetScale(double x, double y, double z);
	//sets a uniform scale for local transform, all same value
	void SetScale(double u);
	const osg::Vec3& GetScale()const{return m_scale;}

	//scale the file was loaded at
	float GetLoadScale(){return m_loadScale;}


	//set the local trans rotation for each axis
	void SetRotation(const osg::Vec3& rot);
	void SetRotationRadians(const osg::Vec3& rot);
	void SetRotation(double x, double y, double z);
	void SetRotationRadians(double x, double y, double z);
	const osg::Vec3& GetRotation() const{return m_rotDegrees;}
	const osg::Vec3 GetRotationRadians()const{
		osg::Vec3 radians;
		radians.x() = osg::DegreesToRadians(m_rotDegrees.x());
		radians.y() = osg::DegreesToRadians(m_rotDegrees.y());
		radians.z() = osg::DegreesToRadians(m_rotDegrees.z());
		return radians;
	}

	//set the translation of the local transform for each axis
	void SetTranslation(const osg::Vec3& pos);
	void SetTranslation(double x, double y, double z);
	const osg::Vec3& GetTranslation() const{return m_position;}

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
	void UpdateScaleMatrix();
	void UpdateRotationMatrix();
	void UpdateTranslationMatrix();

	//Apply the translate rotate and scale matrices to our
	//local transform node
	void UpdateLocalTransform();

protected:

	//the list of osg nodes that have been added to the object
	//to be wrapped and controlled.
	hogbox::NodePtrVector m_wrappedNodes;

	//root node for the object/model
	osg::ref_ptr<osg::Group> m_root;

	//osg::ref_ptr<osg::Node> m_model; // node storing a referance to the loaded or generated model

	//transformation members
	osg::ref_ptr<osg::MatrixTransform> m_localTransform; // the final world transform matrix for the model

	osg::Matrix m_scaleMat;		// the scale matrix used to set the local transform
	osg::Matrix m_rotationMat;	// the rotation matrix used to set the local transform
	osg::Matrix m_translationMat; // the translation matrix used for the loacl transform

	osg::Vec3 m_scale;
	osg::Vec3 m_rotDegrees;
	osg::Vec3 m_position;

	float m_loadScale; //the uniform scale appended to a fille name
	double m_size; //the radius of the mnodels bounding sphere at load time

	//bounding sphere of entire model
	osg::BoundingSphere m_totalBounds; 
	//bounds of just geometry (no lights etc)
	osg::BoundingSphere m_geodeBounds; 

	bool m_needUpdating; //has the objects transform etc changed since last update


	//the list of mesh mappings to apply to this object (and all attched model nodes)
	std::vector<MeshMappingPtr> m_meshMappings;

	int m_nodeMasks;//additional to visible/not

};

typedef osg::ref_ptr<HogBoxObject> HogBoxObjectPtr;

};//end hogbox namespace