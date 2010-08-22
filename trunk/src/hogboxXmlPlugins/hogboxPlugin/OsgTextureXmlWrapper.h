#pragma once

#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for osg::Texture, used by OsgTextureManager
//to read write osg::Texture objects to xml (well there attributes)
//
//Xml Definition: 
//<Texture uniqueID='MyTexture1' type='Texture2D'>
//	<TextureImage>
//		<Image useID='MyImage1'/>
//	</TextureImage>
//</Texture>
//
//class 'type's supported are Texture2D, TextureRectangle, TextureCubeMap
//TextureImage is a pointer to an image node
//
class OsgTextureXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass node representing the texture object
	OsgTextureXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "Texture")
	{

		osg::Texture* texture = NULL;

		//get the Texture type from the nodes 'type' property
		std::string texTypeStr;
		if(!hogboxDB::getXmlPropertyValue(node, "type", texTypeStr))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype 'Texture' should have a 'type' property." <<std::endl 
									<< "                    i.e. <Texture uniqueID='myID' type='Texture2D'>" << std::endl;
			return;
		}

		//create our object and it's xml wrapper.
		//handle various types
		if(texTypeStr == "Texture2D")
		{
			osg::Texture2D* tex2D = new osg::Texture2D();

			//The image used by the texture2D
			m_xmlAttributes["TextureImage"] = new hogboxDB::CallbackXmlClassPointer<osg::Texture2D,osg::Image>(tex2D,
																							&osg::Texture2D::getImage,
																							&osg::Texture2D::setImage);

			//store the object
			texture = tex2D;

		}else if(texTypeStr == "TextureRectangle"){

			osg::TextureRectangle* texRect = new osg::TextureRectangle();

			//The image used by the texture2D
			m_xmlAttributes["TextureImage"] = new hogboxDB::CallbackXmlClassPointer<osg::TextureRectangle,osg::Image>(texRect,
																							&osg::TextureRectangle::getImage,
																							&osg::TextureRectangle::setImage);

			//store the object
			texture = texRect;

		}else if(texTypeStr == "TextureCubeMap"){

			osg::TextureCubeMap* cubeMap = new osg::TextureCubeMap();

			//store the object
			texture = cubeMap;

		}else{

			osg::notify(osg::WARN) << "XML ERROR: Parsing node '" << node->name << "'," << std::endl
								   << "                      It is not a valid texture type." << std::endl;
			return;
		}

		//register common attributes

		//use helper variables to load the texture wrap modes
		m_xmlAttributes["WrapU"] = new hogboxDB::TypedXmlAttribute<std::string>(&_wrapU);
		m_xmlAttributes["WrapV"] = new hogboxDB::TypedXmlAttribute<std::string>(&_wrapV);
		m_xmlAttributes["WrapW"] = new hogboxDB::TypedXmlAttribute<std::string>(&_wrapW);

		//store texture
		p_wrappedObject = texture;
	}


	//overload deserialise
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//cast wrapped to Texture
		osg::Texture* texture = dynamic_cast<osg::Texture*>(p_wrappedObject.get());
		if(!texture){return false;}


		//apply the texture wrap modes
		texture->setWrap(osg::Texture::WRAP_S, GetWrapMode(_wrapU));
		texture->setWrap(osg::Texture::WRAP_T, GetWrapMode(_wrapV));
		texture->setWrap(osg::Texture::WRAP_R, GetWrapMode(_wrapW));

		return true;
	}

	osg::Texture::WrapMode GetWrapMode(const std::string wrapStr)
	{
		if(wrapStr == "CLAMP"){return osg::Texture::CLAMP;}
		if(wrapStr == "CLAMP_TO_EDGE"){return osg::Texture::CLAMP_TO_EDGE;}
		if(wrapStr == "CLAMP_TO_BORDER"){return osg::Texture::CLAMP_TO_BORDER;}
		if(wrapStr == "REPEAT"){return osg::Texture::REPEAT;}
		if(wrapStr == "MIRROR"){return osg::Texture::MIRROR;}
		return osg::Texture::REPEAT;
	}
public:

	//helper values for loading
	std::string _wrapU;
	std::string _wrapV;
	std::string _wrapW;

protected:

	virtual ~OsgTextureXmlWrapper(void)
	{

	}

};

typedef osg::ref_ptr<OsgTextureXmlWrapper> OsgTextureXmlWrapperPtr;


