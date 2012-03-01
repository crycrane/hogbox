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
	MeshMappingXmlWrapper() 
			: hogboxDB::XmlClassWrapper("MeshMapping")
	{

	}
    
    //
    virtual osg::Object* allocateClassType(){return new hogbox::MeshMapping();}
    
    //
    virtual XmlClassWrapper* cloneType(){return new MeshMappingXmlWrapper();}

protected:

	virtual ~MeshMappingXmlWrapper(void){}
    
    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        hogbox::MeshMapping* mapping = dynamic_cast<hogbox::MeshMapping*>(p_wrappedObject.get());
        
		_xmlAttributes["MapToList"] = new hogboxDB::CallbackXmlAttributeList<hogbox::MeshMappingVisitor, std::vector<std::string>, std::string>
                                                                            (mapping->_visitor, &hogbox::MeshMappingVisitor::GetMapToList, 
                                                                             &hogbox::MeshMappingVisitor::SetMapToList);
        
        
		_xmlAttributes["AssignedMaterial"] = new hogboxDB::CallbackXmlClassPointer<hogbox::MeshMappingVisitor, hogbox::HogBoxMaterial>
                                                                                (mapping->_visitor, &hogbox::MeshMappingVisitor::GetMaterial,
                                                                                &hogbox::MeshMappingVisitor::SetMaterial);
        
		_xmlAttributes["Visible"] = new hogboxDB::CallbackXmlAttribute<hogbox::MeshMappingVisitor, bool>
                                                                        (mapping->_visitor,&hogbox::MeshMappingVisitor::GetVisible,
                                                                        &hogbox::MeshMappingVisitor::SetVisible);
        
		//Flag use of skinning
		_xmlAttributes["UseSkinning"] = new hogboxDB::CallbackXmlAttribute<hogbox::MeshMappingVisitor,bool>
                                                                        (mapping->_visitor, &hogbox::MeshMappingVisitor::IsUsingSkinning,
                                                                        &hogbox::MeshMappingVisitor::UseSkinning);

    }

};

typedef osg::ref_ptr<MeshMappingXmlWrapper> MeshMappingXmlWrapperPtr;

