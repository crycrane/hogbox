#pragma once

#include <hogbox/HogBoxObject.h>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for HogBoxObject, used by HogBoxObjectManager
//to read write HogBoxObjects to xml
//HogBoxObject must have loaded it's nodes/models before trying to
//set the parameters of any submeshes. To allow this meshes mappings
//read from from xml are loaded into a temp list. Then passed to hogboxobject
//once it is fully constructed
//
class HogBoxObjectXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass HogBoxObject to be wrapped
	HogBoxObjectXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "HogBoxObject")
	{
		//allocate the HogBoxObject
		hogbox::HogBoxObject* hogboxObject = new hogbox::HogBoxObject();
		if(!hogboxObject){return;}

		//add the attributes required to exposue the HogBoxObjects members to the xml wrapper

		//Position attribute Vec3
		m_xmlAttributes["Position"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxObject,osg::Vec3>(hogboxObject,
																	&hogbox::HogBoxObject::GetTranslation,
																	&hogbox::HogBoxObject::SetTranslation);
		//RotationRadians attribute Vec3 in radians
		//m_xmlAttributes["RotationRadians"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxObject,osg::Vec3>(hogboxObject,
		//															&hogbox::HogBoxObject::GetRotationRadians,
		//															&hogbox::HogBoxObject::SetRotationRadians);
		//RotationDegrees attribute Vec3 in degrees
		m_xmlAttributes["Rotation"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxObject,osg::Vec3>(hogboxObject,
																	&hogbox::HogBoxObject::GetRotation,
																	&hogbox::HogBoxObject::SetRotation);
		//scale Vec3
		m_xmlAttributes["Scale"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxObject,osg::Vec3>(hogboxObject,
															&hogbox::HogBoxObject::GetScale,
															&hogbox::HogBoxObject::SetScale);

		//hogbox objects wrap a set of osg nodes which need to be added to the object
		//via AddNodeToObject. The xml reader will read the list of nodes, directly
		//into m_wrappededNodes. Once reading is complete the list is passed trough 
		//AddNodeToObject so that child nodes etc can be wrapped
		m_xmlAttributes["ModelNodes"] = new hogboxDB::CallbackXmlClassPointerList<hogbox::HogBoxObject,hogbox::NodePtrVector, osg::Node>(hogboxObject,
																										&hogbox::HogBoxObject::GetWrappedNodes,
																										&hogbox::HogBoxObject::SetWrappedNodes);

		//the list of meshmappings
		m_xmlAttributes["MeshMappings"] = new hogboxDB::CallbackXmlClassPointerList<hogbox::HogBoxObject,hogbox::MeshMappingPtrVector, hogbox::MeshMapping>(hogboxObject,
																										&hogbox::HogBoxObject::GetMeshMappings,
																										&hogbox::HogBoxObject::SetMeshMappings);

		//store the hogboxobject as the wrapped object
		p_wrappedObject = hogboxObject;
	}

protected:

	virtual ~HogBoxObjectXmlWrapper(void){}

};

typedef osg::ref_ptr<HogBoxObjectXmlWrapper> HogBoxObjectXmlWrapperPtr;


