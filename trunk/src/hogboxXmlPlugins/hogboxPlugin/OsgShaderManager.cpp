#pragma once
#include <hogboxDB/XmlClassManager.h>
#include "OsgShaderXmlWrapper.h"
#include "OsgUniformXmlWrapper.h"

#include <osg/Shader>
#include <osg/Program>

//
//Manages the loading of the hogbox xml representation
//of osg Shader objects. These include Uniforms, Programs,
//and Shaders (both vertex and fragment)
//
class OsgShaderManager : public hogboxDB::XmlClassManager
{
public:

	OsgShaderManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Shader", "Xml definition of osg Shader");
		SupportsClassType("Uniform", "Xml definition of osg uniform");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	OsgShaderManager(const OsgShaderManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxDB, OsgShaderManager)

protected:

	virtual ~OsgShaderManager(void)
	{
	}


	//
	//Create a HogBoxObject and read it's data in from disk
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		if(!xmlNode){return NULL;}

		hogboxDB::XmlClassWrapperPtr xmlWrapper;

		//handle various class types
		if(xmlNode->name == "Uniform")
		{
			xmlWrapper = new OsgUniformXmlWrapper(xmlNode);

		}else if(xmlNode->name == "Shader"){

			xmlWrapper = new OsgShaderXmlWrapper(xmlNode);
		}

		if(!xmlWrapper){return NULL;}
		//did the wrapper alocate an object
		if(!xmlWrapper->getWrappedObject()){return NULL;}

		//if the wrapper was created properly then use it 
		//to deserialize our the xmlNode into it's wrapped object
		if(!xmlWrapper->deserialize(xmlNode))
		{
			//an error occured deserializing the xml node
			return NULL;
		}

		//we're done deserialising the object
		return xmlWrapper;
	}

};

REGISTER_HOGBOXPLUGIN(Shader, OsgShaderManager)

