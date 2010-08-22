#pragma once

#include <hogbox/HogBoxMesh.h>
#include <hogboxDB/XmlClassWrapper.h>

//
//MeshMappingXmlWrapper
//
class MeshMappingXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass node representing the texture object
	MeshMappingXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "MeshMapping")
	{

		hogbox::MeshMapping* mapping = new hogbox::MeshMapping();

		//register common attributes

		//mapto list
		m_xmlAttributes["MapToList"] = new hogboxDB::CallbackXmlAttributeList<hogbox::MeshMappingVisitor, std::vector<std::string>, std::string>(mapping->m_visitor,
																														&hogbox::MeshMappingVisitor::GetMapToList,
																														&hogbox::MeshMappingVisitor::SetMapToList);


		m_xmlAttributes["AssignedMaterial"] = new hogboxDB::CallbackXmlClassPointer<hogbox::MeshMappingVisitor, hogbox::HogBoxMaterial>(mapping->m_visitor,
																			&hogbox::MeshMappingVisitor::GetMaterial,
																			&hogbox::MeshMappingVisitor::SetMaterial);

		m_xmlAttributes["Visible"] = new hogboxDB::CallbackXmlAttribute<hogbox::MeshMappingVisitor, bool>(mapping->m_visitor,
																			&hogbox::MeshMappingVisitor::GetVisible,
																			&hogbox::MeshMappingVisitor::SetVisible);

		//store mesh
		p_wrappedObject = mapping;
	}

protected:

	virtual ~MeshMappingXmlWrapper(void){}

};

typedef osg::ref_ptr<MeshMappingXmlWrapper> MeshMappingXmlWrapperPtr;

