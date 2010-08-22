#include <hogbox/HogBoxObject.h>
#include <osg/ShapeDrawable>

#include <osgUtil/TangentSpaceGenerator>

#include <osgFX/Scribe>
#include <osgFX/Cartoon>
#include <osgFX/BumpMapping>
#include <osgFX/AnisotropicLighting>

using namespace hogbox;

HogBoxObject::HogBoxObject(void) 
	: osg::Object()
{
	m_localTransform = new osg::MatrixTransform();
	m_localTransform->setName("Object World Trans"); 

	m_scaleMat = osg::Matrix::identity();
	m_rotationMat = osg::Matrix::identity();
	m_translationMat = osg::Matrix::identity(); 

	m_root = new osg::Group();
	m_root->setName("Object Root");

	//hook together the basic subgraph
	m_root->addChild(m_localTransform.get()); 

	//add the callbacks for the group node so that we now when
	//the object is entered by the scene visitor
	//m_p_root->setUpdateCallback(this);

	//shouldn't need updating by default
	m_needUpdating=false;

	SetScale(1.0f);
	SetRotation(0.0f,0.0f,0.0f);
	SetTranslation(0.0f,0.0f,0.0f);

	Dirty();

	m_loadScale=0.0f;
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
HogBoxObject::HogBoxObject(const HogBoxObject& object,const osg::CopyOp& copyop) 
		: osg::Object(object, copyop),
		m_scale(object.m_scale),
		m_position(object.m_position),
		m_rotDegrees(object.m_rotDegrees)
{
	m_localTransform = new osg::MatrixTransform();
	m_localTransform->setName("Object World Trans"); 

	m_scaleMat = osg::Matrix::identity();
	m_rotationMat = osg::Matrix::identity();
	m_translationMat = osg::Matrix::identity(); 

	m_root = new osg::Group();
	m_root->setName("Object Root");

	//hook together the basic subgraph
	m_root->addChild(m_localTransform.get()); 

	Dirty();
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

//
//Set local transforms scale
//
void HogBoxObject::SetScale(const osg::Vec3& scale)
{
	m_scale = scale;
	Dirty();
}
void HogBoxObject::SetScale(double x, double y, double z)
{
	m_scale = osg::Vec3(x,y,z);
	Dirty();
}
void HogBoxObject::SetScale(double u)
{
	m_scale = osg::Vec3(u,u,u);
	Dirty();
}


//
//Set the local transforms rotation
//
void HogBoxObject::SetRotation(const osg::Vec3& rot)
{
	m_rotDegrees = rot;
	Dirty();
}
void HogBoxObject::SetRotationRadians(const osg::Vec3& rot)
{
	m_rotDegrees.x() = osg::RadiansToDegrees(rot.x());
	m_rotDegrees.y() = osg::RadiansToDegrees(rot.y());
	m_rotDegrees.z() = osg::RadiansToDegrees(rot.z());
	Dirty();
}
void HogBoxObject::SetRotation(double x, double y, double z)
{
	m_rotDegrees = osg::Vec3(x,y,z);
	Dirty();
}
void HogBoxObject::SetRotationRadians(double x, double y, double z)
{
	m_rotDegrees.x() = osg::RadiansToDegrees(x);
	m_rotDegrees.y() = osg::RadiansToDegrees(y);
	m_rotDegrees.z() = osg::RadiansToDegrees(z);
	Dirty();
}


//
//Set the local transforms translation
//
void HogBoxObject::SetTranslation(const osg::Vec3& pos)
{
	m_position = pos;
	Dirty();
}
void HogBoxObject::SetTranslation(double x, double y, double z)
{
	m_position = osg::Vec3(x,y,z);
	Dirty();
}


//
// Update the scale matrix using the values of
// m_scaleX,Y,Z
//
void HogBoxObject::UpdateScaleMatrix()
{
	m_scaleMat.set( osg::Matrix::scale(m_scale) );  
}

//
// Update the rotation matrix using the values of
// m_rotDegrees
//
void HogBoxObject::UpdateRotationMatrix()
{
	//have to create a sub matrix for each axis
	//get rotation as radians
	osg::Vec3 rads = GetRotationRadians();
	m_rotationMat.set(	osg::Matrix::rotate(rads.x(), osg::Vec3(1,0,0)) *
							osg::Matrix::rotate(rads.y(), osg::Vec3(0,1,0)) *
							osg::Matrix::rotate(rads.z(), osg::Vec3(0,0,1)) );
}

//
// Update the translation matrix using the values of
// m_position
//
void HogBoxObject::UpdateTranslationMatrix()
{
	m_translationMat.set( osg::Matrix::translate(m_position) ); 
}


//
// Set the local transform matrix using the sub matrices
//
void HogBoxObject::UpdateLocalTransform()
{
	UpdateTranslationMatrix();
	UpdateRotationMatrix();
	UpdateScaleMatrix();
	m_localTransform->setMatrix( m_scaleMat * m_rotationMat * m_translationMat );
}


//
//Attaches the node to our localTransfrom for rendering 
//then calls @@ to transverse the children and add the new
//nodes meshes to our object
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

	//generate tangent space vectors for new model
	//GenerateTangentSpaceVectors();
	
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
//Generate tangent space vectors for all geoms found in the model
//then attavch them as per vertex attributes for use in shaders
//
void HogBoxObject::GenerateTangentSpaceVectors()
{
	//loop the geoms
/*	for(SubMeshList::iterator itr = m_subGeometry.begin(); itr != m_subGeometry.end(); ++itr)
	{

		//check if the geom already has the vectors
		if( ((*itr)->GetGeom()->getVertexAttribArray(6) == 0) && ((*itr)->GetGeom()->getVertexAttribArray(7) == 0) )
		{

			osgUtil::TangentSpaceGenerator* tangGen = new osgUtil::TangentSpaceGenerator();
			tangGen->generate((*itr)->GetGeom(), 0);

			if(tangGen)
			{
				osg::Vec4Array* tangentArray = tangGen->getTangentArray(); 
				osg::Vec4Array* biNormalArray = tangGen->getBinormalArray();

				int size = tangentArray->size();
				int sizeb = biNormalArray->size();

				if( (size>0) && (sizeb>0))
				{
					(*itr)->GetGeom()->setVertexAttribArray(6, tangentArray);
					(*itr)->GetGeom()->setVertexAttribBinding(6, osg::Geometry::BIND_PER_VERTEX);  

					(*itr)->GetGeom()->setVertexAttribArray(7, biNormalArray);
					(*itr)->GetGeom()->setVertexAttribBinding(7, osg::Geometry::BIND_PER_VERTEX);  
				}
			}
		}
	}*/
}

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
	osg::notify(osg::WARN) << "AddMeshMapping" << std::endl;
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



