#include <hogbox/HogBoxMesh.h>

using namespace hogbox;


MeshMappingVisitor::MeshMappingVisitor(std::string name, hogbox::HogBoxMaterial* mat, bool vis, bool checkGeoms)
	: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)        
{
	//add the name to the mapTo list
	if(!name.empty()){_mapToList.push_back(name);}

	//parameters
	_material = mat;
	_isVisible = vis;

	_checkGeoms = checkGeoms;
}

MeshMappingVisitor::~MeshMappingVisitor()
{

}

void MeshMappingVisitor::apply(osg::Node& node)
{
	//is this a geode
	osg::Geode* geo = dynamic_cast<osg::Geode*>(&node);
	if (geo)
    {
		//see if the geodes name matches
		if(compareWithMapToList(geo->getName()))
		{
			//if so apply our mapping to the geode
			ApplyMappingParams(geo);
		}

		if(_checkGeoms)
		{
			//check drawable names
			for(unsigned int i=0;i<geo->getNumDrawables(); i++)
			{
				if(compareWithMapToList(geo->getDrawable(i)->getName()))
				{
					//if so apply our mapping to the geode
					ApplyMappingParams(geo);
				}
			}
		}

	}

	traverse(node);
}

	//
	//check if the passed string matches any of our matTo names
bool MeshMappingVisitor::compareWithMapToList( const std::string& name)
{
	for(unsigned int i=0; i<_mapToList.size(); i++)
	{
		//check for wildcard
		size_t found = _mapToList[i].rfind(".*");
		if (found!=std::string::npos && found == 0)//found wild card
		{
			//strip the wild card extension
			std::string search = _mapToList[i];
			search.erase((int)found, 2);

			found=name.rfind(search);
			if (found!=std::string::npos)
			{return true;}

		}else{
			//standard compare
			if(_mapToList[i].compare(name) == 0)
			{return true;}
		}
	}
	return false;
}

//
//Apply the mapping paramters to the geode
bool MeshMappingVisitor::ApplyMappingParams(osg::Geode* geode)
{
	//check geode
	if(!geode){return false;}

	//if we have a material apply it's stateset
	if(_material)
	{
		//first get the working material, accounting for fallbacks
		HogBoxMaterial* useMat = _material->GetFunctionalMaterial();
		if(useMat)
		{
			geode->setStateSet(useMat->GetStateSet());
			//also check if the material requires tangent space vectors generating
			if(useMat->IsUsingTangetSpace())
			{
				//
				//loop the geoms
				for(unsigned int i=0; i<geode->getNumDrawables(); i++)
				{
					//cast drawable to geometry
					osg::Geometry* geom = geode->getDrawable(i)->asGeometry();
					if(geom)
					{
						//check if the geom already has the vectors
						if( (geom->getVertexAttribArray(6) == 0) && (geom->getVertexAttribArray(7) == 0) )
						{

							osgUtil::TangentSpaceGenerator* tangGen = new osgUtil::TangentSpaceGenerator();
							tangGen->generate(geom, 0);

							if(tangGen)
							{
								osg::Vec4Array* tangentArray = tangGen->getTangentArray(); 
								osg::Vec4Array* biNormalArray = tangGen->getBinormalArray();

								int size = tangentArray->size();
								int sizeb = biNormalArray->size();

								if( (size>0) && (sizeb>0))
								{
									geom->setVertexAttribArray(6, tangentArray);
									geom->setVertexAttribBinding(6, osg::Geometry::BIND_PER_VERTEX);  

									geom->setVertexAttribArray(7, biNormalArray);
									geom->setVertexAttribBinding(7, osg::Geometry::BIND_PER_VERTEX);  
								}
							}
						}
					}
				}
				//
			}
		}
	}

	//apply visibility state (should be a custom mask)
	if(_isVisible)
	{
		geode->setNodeMask(0xFFFFFFFF);
	}else{
		geode->setNodeMask(0x0);
	}

	return true;
}

//
//
MeshMapping::MeshMapping() 
	: osg::Object()
{
	m_visitor = new MeshMappingVisitor();
}

MeshMapping::MeshMapping(const std::string& name, HogBoxMaterial* mat, bool vis, bool checkGeoms) 
	: osg::Object()
{
	m_visitor = new MeshMappingVisitor(name, mat, vis, checkGeoms);
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
MeshMapping::MeshMapping(const MeshMapping& mesh,const osg::CopyOp& copyop)
	: osg::Object(mesh, copyop)
{
}

MeshMapping::~MeshMapping(){
	m_visitor = NULL;
}