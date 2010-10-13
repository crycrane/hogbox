#pragma once

#include <hogbox/Export.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osgUtil/TangentSpaceGenerator>

#include <list>

#include <hogbox/HogBoxBase.h>
#include <hogbox/HogBoxMaterial.h>

namespace hogbox {


//
//MeshMapping Visitor is used to apply a group of states and parameters to a group of named
//geodes. The MeshMapping applies Material/StateSet and visibility (via node mask)
//to all nodes that meet the required mapTo parameter
//
class MeshMappingVisitor : public osg::NodeVisitor
{
public:
	MeshMappingVisitor(std::string name="", hogbox::HogBoxMaterial* mat = NULL, bool vis = true, bool checkGeoms=false);

	//Get set the material used in the mapping
	HogBoxMaterial* GetMaterial(){return _material;}
	void SetMaterial(HogBoxMaterial* mat){_material = mat;}

	//Add mapto name
	void AddMapToString(const std::string& mapTo){_mapToList.push_back(mapTo);}
	//get get the entire mapto list
	const std::vector<std::string>& GetMapToList()const{return _mapToList;}
	void SetMapToList(const std::vector<std::string>& list){_mapToList = list;}

	//Get Set the mapping visibility
	const bool& GetVisible()const{return _isVisible;}
	void SetVisible(const bool& vis){_isVisible = vis;}

	
    virtual void apply(osg::Node& node);


	//
	//check if the passed string matches any of our matTo names
	bool compareWithMapToList( const std::string& name);
	//
	//Apply the mapping paramters to the geode
	bool ApplyMappingParams(osg::Geode* geode);

protected:
    
	virtual ~MeshMappingVisitor();

	//check the drawable names else just geodes
	bool _checkGeoms;

	//the list of names the mapping is looking for
	//A name starting with a * indicates to check the
	//string as just part of the geodes name (a wild card)
	std::vector<std::string> _mapToList;

	
	//the mapping parameters

	//material (stateset) to apply to any found geode
	hogbox::HogBoxMaterialPtr _material;

	//the visibility state to apply to the geodes
	bool _isVisible;

};

typedef osg::ref_ptr<MeshMappingVisitor> MeshMappingVisitorPtr;


//
//MeshMapping is merely an osg::Object based wrapper for the MeshMappingVisitor
//This is required mainly due to the fact the xml system needs osg::Objects as they 
//have a name member
class HOGBOX_EXPORT MeshMapping : public osg::Object
{
public:
	MeshMapping();
	MeshMapping(const std::string& name, HogBoxMaterial* mat, bool vis, bool checkGeoms);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	MeshMapping(const MeshMapping& mesh,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogbox,MeshMapping);

public:

	osg::ref_ptr<MeshMappingVisitor> m_visitor;

protected:
	virtual ~MeshMapping();
};
typedef osg::ref_ptr<MeshMapping> MeshMappingPtr;
typedef std::vector<MeshMappingPtr> MeshMappingPtrVector;

};//end hogbox namespace