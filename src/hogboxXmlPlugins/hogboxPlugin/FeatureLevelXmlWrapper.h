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

#include <hogbox/SystemInfo.h>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for osg::Node, used by OsgNodeManager
//Nodes in this context are entire sub graphs representing
//a 3d model, probably exported from 3ds max etc.
//Becuase they are always groups the contructor allocates
//and osg::Group to store the node.
//
//Xml Definition: 
//<Node uniqueID='MyNode1'>
//	<File>MyModelFile.ive</File>
//</Node>
//
//Nodes can use several methods for loading data, but the base
//Node class has no methods to store the data (i.e. fileName)
//OsgNodeXmlWrapper provides the means of temporarily storing
//info for conveniance load functions
//
class FeatureLevelXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:


	//pass node representing the texture object
	FeatureLevelXmlWrapper() 
			: hogboxDB::XmlClassWrapper("FeatureLevel")
	{
	}
    
    //
    virtual osg::Object* allocateClassType(){return new hogbox::SystemFeatureLevel();}
    
    //
    virtual XmlClassWrapper* cloneType(){return new FeatureLevelXmlWrapper();} 

protected:

	virtual ~FeatureLevelXmlWrapper(void){}

    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        hogbox::SystemFeatureLevel* featureLevel = dynamic_cast<hogbox::SystemFeatureLevel*>(p_wrappedObject.get());
        
		_xmlAttributes["GLVersion"] = new hogboxDB::TypedXmlAttribute<float>(&featureLevel->glVersion);
        
		_xmlAttributes["GLSLVersion"] = new hogboxDB::TypedXmlAttribute<float>(&featureLevel->glslVersion);
        
		_xmlAttributes["TextureUnits"] = new hogboxDB::TypedXmlAttribute<int>(&featureLevel->textureUnits);
        
		_xmlAttributes["TextureCoordUnits"] = new hogboxDB::TypedXmlAttribute<int>(&featureLevel->textureCoordUnits);
        
		_xmlAttributes["VertexAndFragmentShader"] = new hogboxDB::TypedXmlAttribute<bool>(&featureLevel->vertexAndFragmentShaders);
        
		_xmlAttributes["GeometryShader"] = new hogboxDB::TypedXmlAttribute<bool>(&featureLevel->geometryShaders);
        
		_xmlAttributes["ScreenResolution"] = new hogboxDB::TypedXmlAttribute<osg::Vec2>(&featureLevel->screenRes);
    }
};

typedef osg::ref_ptr<FeatureLevelXmlWrapper> FeatureLevelXmlWrapperPtr;


