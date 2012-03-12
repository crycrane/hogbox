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
	HogBoxMaterialXmlWrapper() 
			: hogboxDB::XmlClassWrapper("HogBoxMaterial")
	{


	}
    
    //
    virtual osg::Object* allocateClassType(){return new hogbox::HogBoxMaterial();}
    
    //
    virtual XmlClassWrapper* cloneType(){return new HogBoxMaterialXmlWrapper();} 

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
    
    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        
        hogbox::HogBoxMaterial* material = dynamic_cast<hogbox::HogBoxMaterial*>(p_wrappedObject.get());
		_xmlAttributes["AmbientColor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,osg::Vec3>
                                        ("AmbientColor", material,
                                        &hogbox::HogBoxMaterial::GetAmbient,
                                        &hogbox::HogBoxMaterial::SetAmbient);
        
		_xmlAttributes["DiffuseColor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,osg::Vec3>
                                        ("DiffuseColor", material,
                                         &hogbox::HogBoxMaterial::GetDiffuse,
                                         &hogbox::HogBoxMaterial::SetDiffuse);
        
		_xmlAttributes["SpecularColor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,osg::Vec3>
                                            ("SpecularColor", material,
                                            &hogbox::HogBoxMaterial::GetSpecular,
                                            &hogbox::HogBoxMaterial::SetSpecular);
        
		_xmlAttributes["Shine"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,double>
                                ("Shine", material,
                                &hogbox::HogBoxMaterial::GetShine,
                                &hogbox::HogBoxMaterial::SetShine);
        
		_xmlAttributes["Opacity"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,double>
                                    ("Opacity", material,
                                    &hogbox::HogBoxMaterial::GetAlphaValue,
                                    &hogbox::HogBoxMaterial::SetAlphaValue);
        
		_xmlAttributes["EnableAlpha"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,bool>
                                        ("EnableAlpha", material,
                                        &hogbox::HogBoxMaterial::GetAlphaEnabled,
                                        &hogbox::HogBoxMaterial::SetAlphaEnabled);
        
		_xmlAttributes["TwoSided"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,bool>
                                        ("TwoSided", material,
                                        &hogbox::HogBoxMaterial::GetTwoSidedLightingEnabled,
                                        &hogbox::HogBoxMaterial::SetTwoSidedLightingEnabled);
		
		_xmlAttributes["BinNumber"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,int>
                                    ("BinNumber", material,
                                    &hogbox::HogBoxMaterial::GetBinNumber,
                                    &hogbox::HogBoxMaterial::SetBinNumber);
        
		_xmlAttributes["BinHint"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,int>
                                    ("BinHint", material,
                                    &hogbox::HogBoxMaterial::GetBinHint,
                                    &hogbox::HogBoxMaterial::SetBinHint);
        
		//wrap textures
		_xmlAttributes["TextureChannels"] = new hogboxDB::CallbackXmlClassPointerMap<hogbox::HogBoxMaterial,int,osg::Texture>
                                            ("TextureChannels", material,
                                            &hogbox::HogBoxMaterial::GetTexture,
                                            &hogbox::HogBoxMaterial::SetTexture);
        
		//wrap uniforms
		_xmlAttributes["Uniforms"] = new hogboxDB::CallbackXmlClassPointerList<hogbox::HogBoxMaterial,osg::UniformPtrVector, osg::Uniform>
                                    ("Uniforms", material,
                                    &hogbox::HogBoxMaterial::GetUniformList,
                                    &hogbox::HogBoxMaterial::SetUniformList);
		//wrap shaders
		_xmlAttributes["Shaders"] = new hogboxDB::CallbackXmlClassPointerList<hogbox::HogBoxMaterial,osg::ShaderPtrVector, osg::Shader>
                                    ("Shaders", material,
                                    &hogbox::HogBoxMaterial::GetShaderList,
                                    &hogbox::HogBoxMaterial::SetShaderList);
        
		//flag that tangent space vectors are required
		_xmlAttributes["UseTangentSpace"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,bool>
                                            ("UseTangentSpace", material,
                                            &hogbox::HogBoxMaterial::IsUsingTangetSpace,
                                            &hogbox::HogBoxMaterial::UseTangentSpace);
        
		//Flag use of skinning
		_xmlAttributes["UseSkinning"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxMaterial,bool>
                                        ("UseSkinning", material,
                                        &hogbox::HogBoxMaterial::IsUsingSkinning,
                                        &hogbox::HogBoxMaterial::UseSkinning);
        
		//fallback infomation
        
		//feature level pointer
		_xmlAttributes["RequiredFeatureLevel"] = new hogboxDB::CallbackXmlClassPointer<hogbox::HogBoxMaterial, hogbox::SystemFeatureLevel>
                                                ("RequiredFeatureLevel", material,
                                                &hogbox::HogBoxMaterial::GetFeatureLevel,
                                                &hogbox::HogBoxMaterial::SetFeatureLevel);
        
		//pointer to the fallback material type
		_xmlAttributes["FallbackMaterial"] = new hogboxDB::CallbackXmlClassPointer<hogbox::HogBoxMaterial, hogbox::HogBoxMaterial>
                                            ("FallbackMaterial", material,
                                            &hogbox::HogBoxMaterial::GetFallbackMaterial,
                                            &hogbox::HogBoxMaterial::SetFallbackMaterial);

    }

};

typedef osg::ref_ptr<HogBoxMaterialXmlWrapper> HogBoxMaterialXmlWrapperPtr;


