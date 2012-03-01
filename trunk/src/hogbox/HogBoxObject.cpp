#include <hogbox/HogBoxObject.h>
#include <osg/ShapeDrawable>

#include <osgUtil/TangentSpaceGenerator>

#include <hogbox/HogBoxUtils.h>

using namespace hogbox;

HogBoxObject::HogBoxObject(void) 
	: osg::Object()
{ 	
	_scaleMat = osg::Matrix::identity();
	_rotationMat = osg::Matrix::identity();
	_translationMat = osg::Matrix::identity(); 

	_root = new osg::MatrixTransform();
	_root->setName("Object_Root_World_Transform");

	_localTransform = new osg::MatrixTransform();
	_localTransform->setName("Object_Local_Transform");

	//hook together the basic subgraph
	_root->addChild(_localTransform.get()); 

	SetLocalScale(1.0f);
	SetLocalRotation(0.0f,0.0f,0.0f);
	SetLocalTranslation(0.0f,0.0f,0.0f);

	_loadScale=1.0f;
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
HogBoxObject::HogBoxObject(const HogBoxObject& object,const osg::CopyOp& copyop) 
		: osg::Object(object, copyop),
        _scaleMat(object._scaleMat),
        _rotationMat(object._rotationMat),
        _translationMat(object._translationMat),
		_scale(object._scale),
		_rotDegrees(object._rotDegrees),
        _position(object._position),
		_loadScale(object._loadScale)
{

	//copy of root 
	_root = dynamic_cast<osg::MatrixTransform*>(copyop(object._root.get()));
	//copy of root will have copied the local transform, we need to get it
	for(unsigned int i=0; i<_root->getNumChildren(); i++)
	{
		if(_root->getChild(i)->getName()==object._localTransform->getName())
		{
			_localTransform = dynamic_cast<osg::MatrixTransform*>(_root->getChild(i));
			break;
		}
	}

	SetLocalScale(_scale);
	SetLocalRotation(_rotDegrees);
	SetLocalTranslation(_position);
}

HogBoxObject::~HogBoxObject(void)
{
	OSG_NOTICE << "    Deallocating HogBoxObject: Name '" << this->getName() << "'." << std::endl;
	//transformation members
	//_p_worldTrans = NULL; // the final world transform matrix for the model

	//_subGeometry.clear();
	//_subGeode.clear();
}


//
//get and set visibility on object and sub meshes
//
void HogBoxObject::SetVisible(const bool& vis)
{
	if(vis)
	{
		_root->setNodeMask(0xffffffff);
	}else{
		_root->setNodeMask(0x0);
	}
}

bool HogBoxObject::GetVisible() const
{
	if(_root->getNodeMask() == 0x0)
	{
		return false;
	}else{
		return true;
	}
}

osg::Node* HogBoxObject::GetNodeByName(const std::string& name, const bool& subString)
{
	FindNodesByName findNodes(name,subString);
	_root->accept(findNodes);
	if(findNodes._count>0)
	{return findNodes._foundList[0];}
	return NULL;
}

//
//Return all nodes matching the passed name
//
std::vector<osg::Node*> HogBoxObject::GetNodesByName(const std::string& name, const bool& subString)
{
	FindNodesByName findNodes(name,subString);
	_root->accept(findNodes);
	return findNodes._foundList;
}

//
//Set the world transform
//
void HogBoxObject::SetWorldTransform(const osg::Matrix& trans)
{
	_root->setMatrix(trans);
}

//
//Get the world transform
//
const osg::Matrix& HogBoxObject::GetWorldTransform()const
{
	return _root->getMatrix();
}

//
//Set local transforms scale
//
void HogBoxObject::SetLocalScale(const osg::Vec3& scale)
{
	_scale = scale;
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalScale(const double& x, const double& y, const double& z)
{
	_scale = osg::Vec3(x,y,z);
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalScale(const double& u)
{
	_scale = osg::Vec3(u,u,u);
	UpdateLocalTransform();
}


//
//Set the local transforms rotation
//
void HogBoxObject::SetLocalRotation(const osg::Vec3& rot)
{
	_rotDegrees = rot;
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalRotationRadians(const osg::Vec3& rot)
{
	_rotDegrees.x() = osg::RadiansToDegrees(rot.x());
	_rotDegrees.y() = osg::RadiansToDegrees(rot.y());
	_rotDegrees.z() = osg::RadiansToDegrees(rot.z());
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalRotation(const double& x, const double& y, const double& z)
{
	_rotDegrees = osg::Vec3(x,y,z);
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalRotationRadians(const double& x, const double& y, const double& z)
{
	_rotDegrees.x() = osg::RadiansToDegrees(x);
	_rotDegrees.y() = osg::RadiansToDegrees(y);
	_rotDegrees.z() = osg::RadiansToDegrees(z);
	UpdateLocalTransform();
}


//
//Set the local transforms translation
//
void HogBoxObject::SetLocalTranslation(const osg::Vec3& pos)
{
	_position = pos;
	UpdateLocalTransform();
}
void HogBoxObject::SetLocalTranslation(const double& x, const double& y, const double& z)
{
	_position = osg::Vec3(x,y,z);
	UpdateLocalTransform();
}


//
// Update the scale matrix using the values of
// _scaleX,Y,Z
//
void HogBoxObject::UpdateLocalScaleMatrix()
{
	_scaleMat.set( osg::Matrix::scale(_scale) );  
}

//
// Update the rotation matrix using the values of
// _rotDegrees
//
void HogBoxObject::UpdateLocalRotationMatrix()
{
	//have to create a sub matrix for each axis
	//get rotation as radians
	osg::Vec3 rads = GetLocalRotationRadians();
	_rotationMat.set(	osg::Matrix::rotate(rads.x(), osg::Vec3(1,0,0)) *
						osg::Matrix::rotate(rads.y(), osg::Vec3(0,1,0)) *
						osg::Matrix::rotate(rads.z(), osg::Vec3(0,0,1)) );
}

//
// Update the translation matrix using the values of
// _position
//
void HogBoxObject::UpdateLocalTranslationMatrix()
{
	_translationMat.set( osg::Matrix::translate(_position) ); 
}


//
// Set the local transform matrix using the sub matrices
//
void HogBoxObject::UpdateLocalTransform()
{
	UpdateLocalTranslationMatrix();
	UpdateLocalRotationMatrix();
	UpdateLocalScaleMatrix();
	_localTransform->setMatrix( _scaleMat * _rotationMat * _translationMat );
}


//
//Attaches the node to our localTransfrom for rendering 
//then applies all the current  
//
bool HogBoxObject::AddNodeToObject(osg::ref_ptr<osg::Node> node)
{
	if(!node){return false;}

	//attach the node to our local transform
	_localTransform->addChild(node);

	//bounding sphere of entire model
	_totalBounds = _root->computeBound();

	_wrappedNodes.push_back(node);

	//apply the default node masks to our geom
	ApplyDefaultNodeMaskVisitor applyNodeMasks;
	node->accept(applyNodeMasks);
    
#ifdef TARGET_OS_IPHONE
    ApplyVBOVisitor applyVBO;
    node->accept(applyVBO);
#endif

	//pass all the meshmappings over the the new node
	//applying the material and other parameters to any geodes
	//in the subgraph matching the mapto list
	for(unsigned int i=0; i<_meshMappings.size(); i++)
	{
		node->accept(*_meshMappings[i]->_visitor);
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
	_subGeometry.push_back(l_subM); 
}
*/


//
//get set the vector of nodes being wrapped by this object
//
NodePtrVector HogBoxObject::GetWrappedNodes() const
{
	return _wrappedNodes;
}
//
//on set the nodes are passed through AddNodeToObject to 
//find any meshes etc
void HogBoxObject::SetWrappedNodes(const NodePtrVector& nodes)
{
	//iterate the list and pass through add to object
	for(unsigned int i=0; i<nodes.size(); i++)
	{OSG_FATAL << "Add Wrapped Node" << std::endl;
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
	_root->accept(*mapping->_visitor);

	//add to our list
	_meshMappings.push_back(mapping);
	return true;
}

//
//get set the list of mesh mappings
//
std::vector<MeshMappingPtr> HogBoxObject::GetMeshMappings() const
{
	return _meshMappings;
}

void HogBoxObject::SetMeshMappings(const std::vector<MeshMappingPtr>& mappings)
{
	//pass each of the list through AddMeshMapping
	for(unsigned int i=0; i<mappings.size(); i++)
	{
		AddMeshMapping(mappings[i]);
	}
}



