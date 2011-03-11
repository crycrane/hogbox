#include <hogbox/HogBoxObject.h>
#include <osg/ShapeDrawable>

#include <osgUtil/TangentSpaceGenerator>

#include <hogbox/HogBoxUtils.h>

using namespace hogbox;

HogBoxObject::HogBoxObject(void) 
	: osg::Object()
{ 	
	m_scaleMat = osg::Matrix::identity();
	m_rotationMat = osg::Matrix::identity();
	m_translationMat = osg::Matrix::identity(); 

	m_root = new osg::MatrixTransform();
	m_root->setName("Object_Root_World_Transform");

	m_localTransform = new osg::MatrixTransform();
	m_localTransform->setName("Object_Local_Transform");

	//hook together the basic subgraph
	m_root->addChild(m_localTransform.get()); 

	SetLocalScale(1.0f);
	SetLocalRotation(0.0f,0.0f,0.0f);
	SetLocalTranslation(0.0f,0.0f,0.0f);

	m_loadScale=1.0f;
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
HogBoxObject::HogBoxObject(const HogBoxObject& object,const osg::CopyOp& copyop) 
		: osg::Object(object, copyop),
		m_scale(object.m_scale),
		m_position(object.m_position),
		m_rotDegrees(object.m_rotDegrees),
		m_scaleMat(object.m_scaleMat),
		m_rotationMat(object.m_rotationMat),
		m_translationMat(object.m_translationMat),
		m_loadScale(object.m_loadScale)
{

	//copy of root 
	m_root = dynamic_cast<osg::MatrixTransform*>(copyop(object.m_root.get()));
	//copy of root will have copied the local transform, we need to get it
	for(unsigned int i=0; i<m_root->getNumChildren(); i++)
	{
		if(m_root->getChild(i)->getName()==object.m_localTransform->getName())
		{
			m_localTransform = dynamic_cast<osg::MatrixTransform*>(m_root->getChild(i));
			break;
		}
	}

	SetLocalScale(m_scale);
	SetLocalRotation(m_rotDegrees);
	SetLocalTranslation(m_position);
}

HogBoxObject::~HogBoxObject(void)
{
	//transformation members
	//m_p_worldTrans = NULL; // the final world transform matrix for the model

	//m_subGeometry.clear();
	//m_subGeode.clear();
}


//
//get and set visibility on object and sub meshes
//
void HogBoxObject::SetVisible(const bool& vis)
{
	if(vis)
	{
		m_root->setNodeMask(0xFFFFFFFF);
	}else{
		m_root->setNodeMask(0x0);
	}
}

bool HogBoxObject::GetVisible() const
{
	if(m_root->getNodeMask() == 0x0)
	{
		return false;
	}else{
		return true;
	}
}

osg::Node* HogBoxObject::GetNodeByName(const std::string& name, const bool& subString)
{
	FindNodesByName findNodes(name,subString);
	m_root->accept(findNodes);
	if(findNodes._count>0)
	{return findNodes._foundList[0];}
	return NULL;
}

//
//Set the world transform
//
void HogBoxObject::SetWorldTransform(const osg::Matrix& trans)
{
	m_root->setMatrix(trans);
}

//
//Get the world transform
//
const osg::Matrix& HogBoxObject::GetWorldTransform()const
{
	return m_root->getMatrix();
}

//
//Set local transforms scale
//
void HogBoxObject::SetLocalScale(const osg::Vec3& scale)
{
	m_scale = scale;
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalScale(const double& x, const double& y, const double& z)
{
	m_scale = osg::Vec3(x,y,z);
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalScale(const double& u)
{
	m_scale = osg::Vec3(u,u,u);
	UpdateLocalTransform();
}


//
//Set the local transforms rotation
//
void HogBoxObject::SetLocalRotation(const osg::Vec3& rot)
{
	m_rotDegrees = rot;
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalRotationRadians(const osg::Vec3& rot)
{
	m_rotDegrees.x() = osg::RadiansToDegrees(rot.x());
	m_rotDegrees.y() = osg::RadiansToDegrees(rot.y());
	m_rotDegrees.z() = osg::RadiansToDegrees(rot.z());
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalRotation(const double& x, const double& y, const double& z)
{
	m_rotDegrees = osg::Vec3(x,y,z);
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalRotationRadians(const double& x, const double& y, const double& z)
{
	m_rotDegrees.x() = osg::RadiansToDegrees(x);
	m_rotDegrees.y() = osg::RadiansToDegrees(y);
	m_rotDegrees.z() = osg::RadiansToDegrees(z);
	UpdateLocalTransform();
}


//
//Set the local transforms translation
//
void HogBoxObject::SetLocalTranslation(const osg::Vec3& pos)
{
	m_position = pos;
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalTranslation(const double& x, const double& y, const double& z)
{
	m_position = osg::Vec3(x,y,z);
	UpdateLocalTransform();
}


//
// Update the scale matrix using the values of
// m_scaleX,Y,Z
//
void HogBoxObject::UpdateLocalScaleMatrix()
{
	m_scaleMat.set( osg::Matrix::scale(m_scale) );  
}

//
// Update the rotation matrix using the values of
// m_rotDegrees
//
void HogBoxObject::UpdateLocalRotationMatrix()
{
	//have to create a sub matrix for each axis
	//get rotation as radians
	osg::Vec3 rads = GetLocalRotationRadians();
	m_rotationMat.set(	osg::Matrix::rotate(rads.x(), osg::Vec3(1,0,0)) *
						osg::Matrix::rotate(rads.y(), osg::Vec3(0,1,0)) *
						osg::Matrix::rotate(rads.z(), osg::Vec3(0,0,1)) );
}

//
// Update the translation matrix using the values of
// m_position
//
void HogBoxObject::UpdateLocalTranslationMatrix()
{
	m_translationMat.set( osg::Matrix::translate(m_position) ); 
}


//
// Set the local transform matrix using the sub matrices
//
void HogBoxObject::UpdateLocalTransform()
{
	UpdateLocalTranslationMatrix();
	UpdateLocalRotationMatrix();
	UpdateLocalScaleMatrix();
	m_localTransform->setMatrix( m_scaleMat * m_rotationMat * m_translationMat );
}


//
//Attaches the node to our localTransfrom for rendering 
//then applies all the current  
//
bool HogBoxObject::AddNodeToObject(osg::ref_ptr<osg::Node> node)
{
	if(!node){return false;}

	//attach the node to our local transform
	m_localTransform->addChild(node);

	//bounding sphere of entire model
	m_totalBounds = m_root->computeBound();

	m_wrappedNodes.push_back(node);

	//pass all the meshmappings over the the new node
	//applying the material and other parameters to any geodes
	//in the subgraph matching the mapto list
	for(unsigned int i=0; i<m_meshMappings.size(); i++)
	{
		node->accept(*m_meshMappings[i]->m_visitor);
	}
	
	return true;
}
/*
//
//Wrap the geometry and its geode in a mesh object
//handles ensuring the geode has a unique name for picking
//the meshes name is set to the same as the geodes
//
void HogBoxObject::WrapGeometryNode(osg::Geode* parentGeode, osg::Geometry* geometry, const std::string& name)
{
	std::ostringstream oss(std::ostringstream::out);

	//use passed name if it doesn't have one
	if(geometry->getName().size() == 0)
	{
		geometry->setName(name);
	}

	//check if any with the same name exist
	std::vector<HogBoxMeshPtr> meshes = this->getSubObjectsBySubName(geometry->getName());
	if(meshes.size() > 0)
	{
		//one already exists, append number to the end
		oss.str("");
		oss << geometry->getName() << "-" << meshes.size();
		geometry->setName(oss.str());
	}

	HogBoxMeshPtr l_subM = new HogBoxMesh(parentGeode, geometry);
	l_subM->setName(name);
	m_subGeometry.push_back(l_subM); 
}
*/


//
//get set the vector of nodes being wrapped by this object
//
NodePtrVector HogBoxObject::GetWrappedNodes() const
{
	return m_wrappedNodes;
}
//
//on set the nodes are passed through AddNodeToObject to 
//find any meshes etc
void HogBoxObject::SetWrappedNodes(const NodePtrVector& nodes)
{
	//iterate the list and pass through add to object
	for(unsigned int i=0; i<nodes.size(); i++)
	{
		this->AddNodeToObject(nodes[i]);
	}
}

//
//add a mesh mapping to our list, the added mappings will be passed
//across all existing wrapped nodes to apply the mapping
//
bool HogBoxObject::AddMeshMapping(MeshMapping* mapping)
{
	if(!mapping){return false;}

	//pass to our wrapped nodes via the root
	m_root->accept(*mapping->m_visitor);

	//add to our list
	m_meshMappings.push_back(mapping);
	return true;
}

//
//get set the list of mesh mappings
//
std::vector<MeshMappingPtr> HogBoxObject::GetMeshMappings() const
{
	return m_meshMappings;
}

void HogBoxObject::SetMeshMappings(const std::vector<MeshMappingPtr>& mappings)
{
	//pass each of the list through AddMeshMapping
	for(unsigned int i=0; i<mappings.size(); i++)
	{
		AddMeshMapping(mappings[i]);
	}
}



