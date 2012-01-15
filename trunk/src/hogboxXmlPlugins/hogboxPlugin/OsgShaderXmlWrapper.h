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
	OsgShaderXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "Shader")
	{

		osg::Shader* shader = NULL;

		//get the Texture type from the nodes 'type' property
		std::string shaderTypeStr;
		if(!hogboxDB::getXmlPropertyValue(node, "type", shaderTypeStr))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype 'Shader' should have a 'type' property." <<std::endl 
									<< "                    i.e. <Shader uniqueID='myID' type='Vertex'>" << std::endl;
			return;
		}

		//create our object and it's xml wrapper.
		//handle various types
		if(shaderTypeStr == "Vertex")
		{
			shader = new osg::Shader(osg::Shader::VERTEX);

		}else if(shaderTypeStr == "Fragment"){

			shader = new osg::Shader(osg::Shader::FRAGMENT);

		}else if(shaderTypeStr == "Geometry"){

			shader = new osg::Shader(osg::Shader::GEOMETRY);

		}else{

			osg::notify(osg::WARN) << "XML ERROR: Parsing node '" << node->name << "', with type '" << shaderTypeStr << "'." << std::endl
								   << "                      It is not a valid Shader type." << std::endl;
			return;
		}

		//add the common attributes
		
		//File name for the source code of the shader. Gets read into the shaders fileName variable
		m_xmlAttributes["File"] = new hogboxDB::CallbackXmlAttribute<osg::Shader,std::string>(shader,
																						&osg::Shader::getFileName,
																						&osg::Shader::setFileName);



		//store the shader in the object
		p_wrappedObject = shader;

	}

	//overload deserialise
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//Shaders can provide a File attribute which is used as the shaders
		//source code file if it exists

		//cast wrapped to shader
		osg::Shader* shader = dynamic_cast<osg::Shader*>(p_wrappedObject.get());
		if(!shader){return false;}

		if(!shader->getFileName().empty())
		{
			osg::notify(osg::INFO) << "XML INFO: Loading shader source file '" << shader->getFileName() << "', for Shader node '" << shader->getName() << "'." << std::endl;

			//load the shader source
			if(!shader->loadShaderSourceFromFile(shader->getFileName()))
			{
				//if not try using findDataFile
				std::string tryFileName = osgDB::findDataFile( shader->getFileName() );
				osg::notify(osg::WARN) << "           Trying '" << tryFileName << "'." << std::endl;
				if(!tryFileName.empty()){
					osg::notify(osg::WARN) << "           Found Alternative file'" << tryFileName << "', attempting to load," << std::endl;
					if(!shader->loadShaderSourceFromFile(tryFileName)){return false;}
					shader->setFileName(tryFileName);
				}else{
					osg::notify(osg::WARN) << "          Failed Loading shader source file '" << shader->getFileName() << "', for Shader node '" << shader->getName() << "'." << std::endl;
					return false;
				}
			}
			osg::notify(osg::INFO) << "          Success loading shader source file '" << shader->getFileName() << "', for Shader node '" << shader->getName() << "'." << std::endl;
		}
		return true;
	}

protected:

	virtual ~OsgShaderXmlWrapper(void){}

};

typedef osg::ref_ptr<OsgShaderXmlWrapper> OsgShaderXmlWrapperPtr;


