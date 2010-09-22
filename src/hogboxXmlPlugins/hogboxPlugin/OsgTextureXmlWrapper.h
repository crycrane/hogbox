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
			: hogboxDB::XmlClassWrapper(node, "Texture"),
			_wrapU("REPEAT"),
			_wrapV("REPEAT"),
			_wrapW("REPEAT"),
			//filtering
			_minFilter("LINEAR_MIPMAP_NEAREST"),
			_magFilter("LINEAR_MIPMAP_NEAREST"),
			//auto gen mipmaps, default to true
			_autoGenMipMaps(true),
			//resize nonpower of two, default to true
			_resizeNPOT(true)
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

		m_xmlAttributes["MinFilter"] = new hogboxDB::TypedXmlAttribute<std::string>(&_minFilter);
		m_xmlAttributes["MagFilter"] = new hogboxDB::TypedXmlAttribute<std::string>(&_magFilter);

		m_xmlAttributes["GenMipMaps"] = new hogboxDB::TypedXmlAttribute<bool>(&_autoGenMipMaps);
		m_xmlAttributes["ResizeNPOT"] = new hogboxDB::TypedXmlAttribute<bool>(&_resizeNPOT);

		m_xmlAttributes["MaxAnisotropy"] = new hogboxDB::TypedXmlAttribute<float>(&_maxAnisotropic);
		
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

		//apply filter mode
		texture->setFilter(osg::Texture2D::MIN_FILTER, GetFilterMode(_minFilter));
		texture->setFilter(osg::Texture2D::MAG_FILTER, GetFilterMode(_magFilter));

		texture->setResizeNonPowerOfTwoHint(_resizeNPOT);
		texture->setUseHardwareMipMapGeneration(_autoGenMipMaps);

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

	osg::Texture::FilterMode GetFilterMode(const std::string& filterStr)
	{
		if(filterStr == "LINEAR"){return osg::Texture::LINEAR;}
		if(filterStr == "LINEAR_MIPMAP_LINEAR"){return osg::Texture::LINEAR_MIPMAP_LINEAR;}
		if(filterStr == "LINEAR_MIPMAP_NEAREST"){return osg::Texture::LINEAR_MIPMAP_NEAREST;}
		if(filterStr == "NEAREST"){return osg::Texture::NEAREST;}
		if(filterStr == "NEAREST_MIPMAP_LINEAR"){return osg::Texture::NEAREST_MIPMAP_LINEAR;}
		if(filterStr == "NEAREST_MIPMAP_NEAREST"){return osg::Texture::NEAREST_MIPMAP_NEAREST;}
		return osg::Texture::LINEAR;
	}

public:

	//helper values for loading

	//tex wrap mode
	std::string _wrapU;
	std::string _wrapV;
	std::string _wrapW;

	//filtering
	std::string _minFilter;
	std::string _magFilter;

	//auto gen mipmaps, default to true
	bool _autoGenMipMaps;

	//resize nonpower of two, default to true
	bool _resizeNPOT;

	//max anisotropy
	float _maxAnisotropic;

protected:

	virtual ~OsgTextureXmlWrapper(void)
	{

	}

};

typedef osg::ref_ptr<OsgTextureXmlWrapper> OsgTextureXmlWrapperPtr;


