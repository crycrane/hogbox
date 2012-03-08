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
	OsgTextureXmlWrapper(const std::string& classType) 
			: hogboxDB::XmlClassWrapper(classType),
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

	}
    
    //should be a inherited type e.g. texture2D
    virtual osg::Object* allocateClassType(){return NULL;}
    
    //
    virtual XmlClassWrapper* cloneType(){return new OsgTextureXmlWrapper("Texture");}


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

	virtual ~OsgTextureXmlWrapper(void){
	}
    
    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        //osg::Texture* texture = dynamic_cast<osg::Texture*>(p_wrappedObject.get());
		//use helper variables to load the texture wrap modes
		_xmlAttributes["WrapU"] = new hogboxDB::TypedXmlAttribute<std::string>("WrapU", &_wrapU);
		_xmlAttributes["WrapV"] = new hogboxDB::TypedXmlAttribute<std::string>("WrapV", &_wrapV);
		_xmlAttributes["WrapW"] = new hogboxDB::TypedXmlAttribute<std::string>("WrapW", &_wrapW);
        
		_xmlAttributes["MinFilter"] = new hogboxDB::TypedXmlAttribute<std::string>("MinFilter", &_minFilter);
		_xmlAttributes["MagFilter"] = new hogboxDB::TypedXmlAttribute<std::string>("MagFilter", &_magFilter);
        
		_xmlAttributes["GenMipMaps"] = new hogboxDB::TypedXmlAttribute<bool>("GenMipMaps", &_autoGenMipMaps);
		_xmlAttributes["ResizeNPOT"] = new hogboxDB::TypedXmlAttribute<bool>("ResizeNPOT", &_resizeNPOT);
        
		_xmlAttributes["MaxAnisotropy"] = new hogboxDB::TypedXmlAttribute<float>("MaxAnisotropy", &_maxAnisotropic);
    }

};

typedef osg::ref_ptr<OsgTextureXmlWrapper> OsgTextureXmlWrapperPtr;


