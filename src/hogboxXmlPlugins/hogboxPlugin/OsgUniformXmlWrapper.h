#pragma once

#include <osg/Uniform>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for osg::Uniform, used by OsgShaderManager
//to read write osg::Uniform objects to xml (well there attributes)
//Because uniforms can be many different types OsgUniformXmlWrapper
//overloads the virtual de/serialise functions so that the type can
//be deter
//
//Xml Definition: 
//<Uniform uniqueID='MyUniform1' type='int'>
//	2
//</Uniform>
//
//Classtypes supported are Uniform only
//'type' property indicates the type of the uniform, supported types are
//all string types supported by osg::Uniform::getTypeId( const std::string& tname )
//examples being 'int', 'float', 'vec2', 'vec3', 'vec4'
//
class OsgUniformXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass node representing the uniform 
	OsgUniformXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "Uniform")
	{
		//allocate the uniform
		osg::Uniform* wrappedUniform = new osg::Uniform();
		if(!wrappedUniform){return;}

		//get the uniform type from the nodes 'type' property
		//uniform nodes should have a 'type' property 
		std::string uniformTypeStr;
		if(!hogboxDB::getXmlPropertyValue(node, "type", uniformTypeStr))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype 'Uniform' should have a 'type' property." <<std::endl 
									<< "                    i.e. <Uniform uniqueID='myID' type='int'>" << std::endl;
			return;
		}

		//use the string representation of the uniform to find the uniform type
		osg::Uniform::Type uniformType = osg::Uniform::getTypeId(uniformTypeStr.c_str());

		//set the uniform type (this can only happen once)
		if(!wrappedUniform->setType(uniformType))
		{
			osg::notify(osg::WARN)	<< "XML WRAP ERROR: OsgUniformXmlWrapper can not set the type of the uniform to '" << uniformTypeStr << "'," << std::endl
									<< "                               The type is already set to '" << osg::Uniform::getTypename(wrappedUniform->getType()) << "'." << std::endl;
			return;
		}

		//allocated a uniform of the correct type, store it as the wrapped object
		p_wrappedObject = wrappedUniform;
	}

	//
	//Read the in xmlNode into our wrapped object via the
	//registered XmlAtributes
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		//the variable name in glsl is taken from the name of the uniform, which is set in the base 
		//deserialize to reflect the objects uniqueID
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//cast the wrapped object to an osg::Uniform
		osg::Uniform* wrappedUniform = dynamic_cast<osg::Uniform*>(p_wrappedObject.get());

		//check it is a uniform
		if(!wrappedUniform)
		{
			osg::notify(osg::WARN)	<< "XML WRAP ERROR: OsgUniformXmlWrapper expects an object of type 'osg::Uniform', it got '" << p_wrappedObject->className() << "'." <<std::endl; 
			return false;
		}


		//now we have a uniform of the correct type we need to read the approptiate variable
		//type from the xmlnodes contents. We're gonna just create a value of every type at the 
		//top of the switch and then pass it to the relevant hogboxDB::getXmlContents

		bool boolValue;
		int intValue;
		float floatValue;
		osg::Vec2 vec2Value;osg::Vec3 vec3Value;osg::Vec4 vec4Value;

		//track if it was set succssesfully
		bool isSet = false;
		osg::Uniform::Type uniformType = wrappedUniform->getType();
		switch( uniformType )
		{
		case osg::Uniform::BOOL:
			if(hogboxDB::getXmlContents(in, boolValue))
			{break;}
			isSet = wrappedUniform->set(boolValue);
			break;
		case osg::Uniform::INT:             
			if(!hogboxDB::getXmlContents(in, intValue))
			{break;}
			isSet = wrappedUniform->set(intValue);
			break;
		case osg::Uniform::FLOAT:             
			if(!hogboxDB::getXmlContents(in, floatValue))
			{break;}
			isSet = wrappedUniform->set(floatValue);
			break;
		case osg::Uniform::FLOAT_VEC2:
			if(!hogboxDB::getXmlContents(in, vec2Value))
			{break;}
			isSet = wrappedUniform->set(vec2Value);
			break;
		case osg::Uniform::FLOAT_VEC3:
			if(!hogboxDB::getXmlContents(in, vec3Value))
			{break;}
			isSet = wrappedUniform->set(vec3Value);
			break;
		case osg::Uniform::FLOAT_VEC4:
			if(!hogboxDB::getXmlContents(in, vec4Value))
			{break;}
			isSet = wrappedUniform->set(vec4Value);
			break;
		case osg::Uniform::SAMPLER_1D:
		case osg::Uniform::SAMPLER_2D:
		case osg::Uniform::SAMPLER_1D_ARRAY:
		case osg::Uniform::SAMPLER_2D_ARRAY:
		case osg::Uniform::SAMPLER_3D:
		case osg::Uniform::SAMPLER_CUBE:
		case osg::Uniform::SAMPLER_1D_SHADOW:
		case osg::Uniform::SAMPLER_2D_SHADOW:
		case osg::Uniform::SAMPLER_1D_ARRAY_SHADOW:
		case osg::Uniform::SAMPLER_2D_ARRAY_SHADOW:
			if(!hogboxDB::getXmlContents(in, intValue))
			{break;}
			isSet = wrappedUniform->set(intValue);
			break;
		case osg::Uniform::FLOAT_MAT2x3:break;
		case osg::Uniform::FLOAT_MAT2x4:break;
		case osg::Uniform::FLOAT_MAT3x2:break;
		case osg::Uniform::FLOAT_MAT3x4:break;
		case osg::Uniform::FLOAT_MAT4x2:break;
		case osg::Uniform::FLOAT_MAT4x3:break;
		case osg::Uniform::SAMPLER_BUFFER:break;
		case osg::Uniform::SAMPLER_CUBE_SHADOW:break;
		case osg::Uniform::UNSIGNED_INT:break;
		case osg::Uniform::UNSIGNED_INT_VEC2:break;
		case osg::Uniform::UNSIGNED_INT_VEC3:break;
		case osg::Uniform::UNSIGNED_INT_VEC4:break;
		case osg::Uniform::INT_SAMPLER_1D:break;
		case osg::Uniform::INT_SAMPLER_2D:break;
		case osg::Uniform::INT_SAMPLER_3D: break;
		case osg::Uniform::INT_SAMPLER_CUBE:break;
		case osg::Uniform::INT_SAMPLER_2D_RECT:break;
		case osg::Uniform::INT_SAMPLER_1D_ARRAY:break;
		case osg::Uniform::INT_SAMPLER_2D_ARRAY:break;
		case osg::Uniform::INT_SAMPLER_BUFFER:break;
		case osg::Uniform::UNSIGNED_INT_SAMPLER_1D:break;
		case osg::Uniform::UNSIGNED_INT_SAMPLER_2D:break;
		case osg::Uniform::UNSIGNED_INT_SAMPLER_3D:break;
		case osg::Uniform::UNSIGNED_INT_SAMPLER_CUBE:break;
		case osg::Uniform::UNSIGNED_INT_SAMPLER_2D_RECT:break;
		case osg::Uniform::UNSIGNED_INT_SAMPLER_1D_ARRAY:break;
		case osg::Uniform::UNSIGNED_INT_SAMPLER_2D_ARRAY:break;
		case osg::Uniform::UNSIGNED_INT_SAMPLER_BUFFER:break;
		default: break;
		}

		if(!isSet)
		{
			//failed to read the contents as the corrent type
			return false;
		}

		return true;
	}

	//
	//Uniforms names need to reflect the variables of a shader file
	//to allow this Uniforms objects will strip all but the final word
	//from a . sperated string, then use this as the uniform objects name
	//i.e. name = 'MyMaterial.diffuseColor' will be set as 'diffuseColor'.
	//The nodes it's will still be indetifiable by it's XmlClassWrapper node
	//which will retain the full ID
	virtual void SetObjectNameFromUniqueID(const std::string& name)
	{
		//cast to material
		osg::Uniform* uniform = dynamic_cast<osg::Uniform*>(p_wrappedObject.get());
		if(!uniform){return;}

		//default name to whole uniqueID
		std::string uniformName = name;

		//if a dot seperation sequence is found
		//use the wrod following the last dot
		size_t found;
		found = name.find_last_of(".");
		if(found+1 != std::string::npos)
		{
			uniformName = name.substr(found+1,name.size()-1);
		}
		uniform->setName(uniformName);
	}

protected:

	virtual ~OsgUniformXmlWrapper(void){}

};

typedef osg::ref_ptr<OsgUniformXmlWrapper> OsgUniformXmlWrapperPtr;


