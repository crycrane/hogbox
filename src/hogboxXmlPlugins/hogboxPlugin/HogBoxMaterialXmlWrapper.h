#pragma once

#include <hogbox/HogBoxMaterial.h>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for HogBoxObject, used by HogBoxObjectManager
//to read write HogBoxObjects to xml
//
class HogBoxMaterialXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass HogBoxObject to be wrapped
	HogBoxMaterialXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "HogBoxMaterial")
	{
		//allocate a HogBoxMaterial, currently it has no sub 'type's
		hogbox::HogBoxMaterial* material = new hogbox::HogBoxMaterial();
		if(!material){return;}

		//add the attributes required to exposue the HogBoxMaterial members to the xml wrapper

		m_xmlAttributes["AmbientColor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,osg::Vec3>(material,
																	&hogbox::HogBoxMaterial::GetAmbient,
																	&hogbox::HogBoxMaterial::SetAmbient);
		m_xmlAttributes["DiffuseColor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,osg::Vec3>(material,
																	&hogbox::HogBoxMaterial::GetDiffuse,
																	&hogbox::HogBoxMaterial::SetDiffuse);
		m_xmlAttributes["SpecularColor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,osg::Vec3>(material,
																	&hogbox::HogBoxMaterial::GetSpecular,
																	&hogbox::HogBoxMaterial::SetSpecular);

		m_xmlAttributes["Shine"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,double>(material,
																	&hogbox::HogBoxMaterial::GetShine,
																	&hogbox::HogBoxMaterial::SetShine);

		m_xmlAttributes["Opacity"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,double>(material,
																	&hogbox::HogBoxMaterial::GetAlphaValue,
																	&hogbox::HogBoxMaterial::SetAlphaValue);

		m_xmlAttributes["EnableAlpha"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,bool>(material,
																	&hogbox::HogBoxMaterial::GetAlphaEnabled,
																	&hogbox::HogBoxMaterial::SetAlphaEnabled);

		m_xmlAttributes["TwoSided"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,bool>(material,
																	&hogbox::HogBoxMaterial::GetTwoSidedLightingEnabled,
																	&hogbox::HogBoxMaterial::SetTwoSidedLightingEnabled);
		
		m_xmlAttributes["BinNumber"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,int>(material,
																									  &hogbox::HogBoxMaterial::GetBinNumber,
																									  &hogbox::HogBoxMaterial::SetBinNumber);

		m_xmlAttributes["BinHint"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,int>(material,
																									&hogbox::HogBoxMaterial::GetBinHint,
																									&hogbox::HogBoxMaterial::SetBinHint);

		//wrap textures
		m_xmlAttributes["TextureChannels"] = new hogboxDB::CallbackXmlClassPointerMap<hogbox::HogBoxMaterial,int,osg::Texture>(material,
																						&hogbox::HogBoxMaterial::GetTexture,
																						&hogbox::HogBoxMaterial::SetTexture);

		//wrap uniforms
		m_xmlAttributes["Uniforms"] = new hogboxDB::CallbackXmlClassPointerList<hogbox::HogBoxMaterial,hogbox::UniformPtrVector, osg::Uniform>(material,
																						&hogbox::HogBoxMaterial::GetUniformList,
																						&hogbox::HogBoxMaterial::SetUniformList);
		//wrap shaders
		m_xmlAttributes["Shaders"] = new hogboxDB::CallbackXmlClassPointerList<hogbox::HogBoxMaterial,hogbox::ShaderPtrVector, osg::Shader>(material,
																						&hogbox::HogBoxMaterial::GetShaderList,
																						&hogbox::HogBoxMaterial::SetShaderList);

		//flag that tangent space vectors are required
		m_xmlAttributes["UseTangentSpace"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,bool>(material,
																						&hogbox::HogBoxMaterial::IsUsingTangetSpace,
																						&hogbox::HogBoxMaterial::UseTangentSpace);



		//fallback infomation

		//feature level pointer
		m_xmlAttributes["RequiredFeatureLevel"] = new hogboxDB::CallbackXmlClassPointer<hogbox::HogBoxMaterial, hogbox::SystemFeatureLevel>(material,
																													&hogbox::HogBoxMaterial::GetFeatureLevel,
																													&hogbox::HogBoxMaterial::SetFeatureLevel);

		//pointer to the fallback material type
		m_xmlAttributes["FallbackMaterial"] = new hogboxDB::CallbackXmlClassPointer<hogbox::HogBoxMaterial, hogbox::HogBoxMaterial>(material,
																													&hogbox::HogBoxMaterial::GetFallbackMaterial,
																													&hogbox::HogBoxMaterial::SetFallbackMaterial);
		
		//store the material as the wrapped object
		p_wrappedObject = material;

	}

	//
	//cast to material before setting name as it has it's own version
	virtual void SetObjectNameFromUniqueID(const std::string& name)
	{
		//cast to material
		hogbox::HogBoxMaterial* material = dynamic_cast<hogbox::HogBoxMaterial*>(p_wrappedObject.get());
		if(!material){return;}

		material->setName(name);
	}

protected:

	virtual ~HogBoxMaterialXmlWrapper(void){}

};

typedef osg::ref_ptr<HogBoxMaterialXmlWrapper> HogBoxMaterialXmlWrapperPtr;


