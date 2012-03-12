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

#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for osg::Shader, used by OsgTextureManager
//to read write osg::Texture objects to xml (well there attributes)
//
//Xml Definition: 
//<Shader uniqueID='MyVertShader1' type='VERTEX'>
//	
//</Shader>
//
//class 'type's supported are Vertex, Fragment and Geometry
//
class OsgShaderXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass node representing the texture object
	OsgShaderXmlWrapper() 
			: hogboxDB::XmlClassWrapper("Shader")
	{

	}

    //
    virtual osg::Object* allocateClassType(){return new osg::Shader();}
    
    //
    virtual XmlClassWrapper* cloneType(){return new OsgShaderXmlWrapper();}
    
	//overload deserialise
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//Shaders can provide a File attribute which is used as the shaders
		//source code file if it exists

		//cast wrapped to shader
		osg::Shader* shader = dynamic_cast<osg::Shader*>(p_wrappedObject.get());
		if(!shader){return false;}
        
        
		//get the Texture type from the nodes 'type' property
		std::string shaderTypeStr;
		if(!hogboxDB::getXmlPropertyValue(in, "shaderType", shaderTypeStr))
		{
			OSG_WARN << "XML ERROR: Nodes of classtype 'Shader' should have a 'shaderType' property." <<std::endl 
            << "                    i.e. <Shader uniqueID='myID' shaderType='Vertex'>" << std::endl;
			return false;
		}
        
		//create our object and it's xml wrapper.
		//handle various types
        osg::Shader::Type shaderType;
		if(shaderTypeStr == "Vertex")
		{
			shaderType = osg::Shader::VERTEX;
            
		}else if(shaderTypeStr == "Fragment"){
            
			shaderType = osg::Shader::FRAGMENT;
            
		}else if(shaderTypeStr == "Geometry"){
            
			shaderType = osg::Shader::GEOMETRY;
            
		}else{
            
			OSG_WARN << "XML ERROR: Parsing node '" << in->name << "', with type '" << shaderTypeStr << "'." << std::endl
            << "                      It is not a valid Shader type." << std::endl;
			return false;
		}
        

		if(!shader->getFileName().empty())
		{
			OSG_INFO << "XML INFO: Loading shader source file '" << shader->getFileName() << "', for Shader node '" << shader->getName() << "'." << std::endl;

            //read shader from AssetManager
            osg::ShaderPtr readShader = hogbox::AssetManager::Inst()->GetOrLoadShader(shader->getFileName(), shaderType);
            
			//load the shader source
			if(!readShader.get())
			{
				p_wrappedObject = NULL;
                return false;
			}
            
			OSG_INFO << "          Success loading shader source file '" << shader->getFileName() << "', for Shader node '" << shader->getName() << "'." << std::endl;
            p_wrappedObject = readShader;
		}
		return true;
	}

protected:

	virtual ~OsgShaderXmlWrapper(void){}
    
    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        osg::Shader* shader = dynamic_cast<osg::Shader*>(p_wrappedObject.get());
		//File name for the source code of the shader. Gets read into the shaders fileName variable
		_xmlAttributes["File"] = new hogboxDB::CallbackXmlAttribute<osg::Shader,std::string>
                                ("File", shader,
                                &osg::Shader::getFileName,
                                &osg::Shader::setFileName);
    }

};

typedef osg::ref_ptr<OsgShaderXmlWrapper> OsgShaderXmlWrapperPtr;


