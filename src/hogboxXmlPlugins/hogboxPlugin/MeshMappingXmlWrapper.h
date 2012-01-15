/* Written by Thomas Hogarth, (C) 2011
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 */

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

		//Flag use of skinning
		m_xmlAttributes["UseSkinning"] = new hogboxDB::CallbackXmlAttribute<hogbox::MeshMappingVisitor,bool>(mapping->m_visitor,
																						&hogbox::MeshMappingVisitor::IsUsingSkinning,
																						&hogbox::MeshMappingVisitor::UseSkinning);

		//store mesh
		p_wrappedObject = mapping;
	}

protected:

	virtual ~MeshMappingXmlWrapper(void){}

};

typedef osg::ref_ptr<MeshMappingXmlWrapper> MeshMappingXmlWrapperPtr;

