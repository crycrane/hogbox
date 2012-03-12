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
	HogBoxObjectXmlWrapper() 
			: hogboxDB::XmlClassWrapper("HogBoxObject")
	{
        OSG_FATAL << "Allocate HogBoxObjectXmlWrapper" << std::endl;
	}
    
    //
    virtual osg::Object* allocateClassType(){
        OSG_FATAL << "HogBoxObjectXmlWrapper Allocating HogBoxObject" << std::endl;
        return new hogbox::HogBoxObject();
    }
    
    //
    virtual XmlClassWrapper* cloneType(){return new HogBoxObjectXmlWrapper();} 

protected:

	virtual ~HogBoxObjectXmlWrapper(void){}
    
    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        
        hogbox::HogBoxObject* hogboxObject = dynamic_cast<hogbox::HogBoxObject*>(p_wrappedObject.get());
        
		//Position attribute Vec3
		_xmlAttributes["Position"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxObject,osg::Vec3>
                                    ("Position", hogboxObject,
                                    &hogbox::HogBoxObject::GetLocalTranslation,
                                    &hogbox::HogBoxObject::SetLocalTranslation);
		//RotationRadians attribute Vec3 in radians
		//_xmlAttributes["RotationRadians"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxObject,osg::Vec3>(hogboxObject,
		//															&hogbox::HogBoxObject::GetRotationRadians,
		//															&hogbox::HogBoxObject::SetRotationRadians);
		//RotationDegrees attribute Vec3 in degrees
		_xmlAttributes["Rotation"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxObject,osg::Vec3>
                                    ("Rotation", hogboxObject,
                                    &hogbox::HogBoxObject::GetLocalRotation,
                                     &hogbox::HogBoxObject::SetLocalRotation);
		//scale Vec3
		_xmlAttributes["Scale"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxObject,osg::Vec3>
                                ("Scale",hogboxObject,
                                &hogbox::HogBoxObject::GetLocalScale,
                                &hogbox::HogBoxObject::SetLocalScale);
        
		//hogbox objects wrap a set of osg nodes which need to be added to the object
		//via AddNodeToObject. The xml reader will read the list of nodes, directly
		//into _wrappededNodes. Once reading is complete the list is passed trough 
		//AddNodeToObject so that child nodes etc can be wrapped
		_xmlAttributes["ModelNodes"] = new hogboxDB::CallbackXmlClassPointerList<hogbox::HogBoxObject,osg::NodePtrVector, osg::Node>
                                        ("ModelNodes", hogboxObject,
                                         &hogbox::HogBoxObject::GetWrappedNodes,
                                         &hogbox::HogBoxObject::SetWrappedNodes);
        
		//the list of meshmappings
		_xmlAttributes["MeshMappings"] = new hogboxDB::CallbackXmlClassPointerList<hogbox::HogBoxObject,
                                                                                    hogbox::MeshMappingPtrVector, 
                                                                                    hogbox::MeshMapping>
                                        ("MeshMappings", hogboxObject,
                                        &hogbox::HogBoxObject::GetMeshMappings,
                                        &hogbox::HogBoxObject::SetMeshMappings);
    }

};

typedef osg::ref_ptr<HogBoxObjectXmlWrapper> HogBoxObjectXmlWrapperPtr;


