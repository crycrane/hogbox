#pragma once

//////////////////////////////////////////////////////////
// Author:	Thomas Hogarth								//
// Date:	12/09/2007									//
//														//
// Class:												//
// Osg StateSet helper class				            //
//														//
// Description:											//
// Wraps up and simplifies the setup of osg statesets	//
// Making the behaviour similar to 3DS Max materials	//
// Also provides a fallback system for materials that	//
// require features that may not exist on all systems   //
//////////////////////////////////////////////////////////

#include <hogbox/Export.h>
#include <hogbox/HogBoxBase.h>
#include <hogbox/SystemInfo.h>

#include <osg/Material>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/TexEnvCombine>

#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/HighlightMapGenerator>

#include <hogbox/HogBoxLight.h>


namespace hogbox {

//channels used for basic mode of operation
#define DIFFUSE_CHANNEL		0
#define REFLECT_CHANNEL		1
#define SPECULAR_CHANNEL	2
#define SHADOW_CHANNEL		3


//MATERIAL

class HOGBOX_EXPORT HogBoxMaterial : public osg::Object
{
public:
	HogBoxMaterial(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxMaterial(const HogBoxMaterial&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogbox, HogBoxMaterial);

	//
	//Return the materials stateset
	osg::StateSet* GetStateSet(){return m_stateset.get();}

	//
	//overload set name to also set the name of the stateset
	void setName( const std::string& name );
	void setName( const char* name );

//gl material

	//Set and apply all the values of the material
	void SetMaterial(osg::Vec3 ambi, osg::Vec3 diffuse, osg::Vec3 spec, double exp);
	void SetDefaultMaterial();

	void SetAmbient(const osg::Vec3& ambient){
		SetMaterial(ambient, m_diffuseColor, m_specularColour, m_shininess);
	}
	const osg::Vec3& GetAmbient() const{return m_ambientColour;}
	void SetDiffuse(const osg::Vec3& diffuse){
		SetMaterial(m_ambientColour, diffuse, m_specularColour, m_shininess);
	}
	const osg::Vec3& GetDiffuse() const{return m_diffuseColor;}
	void SetSpecular(const osg::Vec3& specular){
		SetMaterial(m_ambientColour, m_diffuseColor, specular, m_shininess);
	}
	const osg::Vec3& GetSpecular() const{return m_specularColour;}
	void SetShine(const double& shine){
		SetMaterial(m_ambientColour, m_diffuseColor, m_specularColour, shine);
	}
	const double& GetShine() const{return m_shininess;}

	//Get set the alpha gl material
	void SetAlphaValue(const double& blend);
	const double& GetAlphaValue() const{return m_alpha;}
	//enable/disable gl bending
	void SetAlphaEnabled(const bool& enabled);
	const bool& GetAlphaEnabled() const;

	//enable disable 2 sided lighting
	void SetTwoSidedLightingEnabled(const bool& val);
	const bool& GetTwoSidedLightingEnabled()const{return m_backFaceLit;}

//textures

	//set a texture to a specified channel, also creating a shader
	//sampler for the texture. The samplers name is taken from the
	//textures name
	void SetTexture(const int& channel,osg::Texture* tex);

	//return the texture if any in the channel, if
	//there is no texture in the channel NULL is returned
	osg::Texture* GetTexture(const int& channel);

	//is there an enabled texture in the channel
	bool IsTextureEnabled(const int& channel);

	//enable/diable the texture in channel
	void EnableTexture(const int& channel, bool on);

	//return the number of channels containing a valid texture
	unsigned int GetNumTextures()const;

//uniforms

	bool AddUniform(osg::Uniform* uni); 
	bool RemoveUniform(osg::Uniform* uni); 
	osg::Uniform* GetUniform(const unsigned int index);
	osg::Uniform* GetUniform(const std::string& name);	
	const unsigned int GetNumUniforms(){return m_uniforms.size();}

	//get set the entire list of uniforms for easier xmlwrapping, setUniformList
	//list perform AddUniform on each of the passed list
	UniformPtrVector GetUniformList()const{return m_uniforms;}
	void SetUniformList(const UniformPtrVector& list);

//shaders

	//true if there is currently any active shader program
	//attached to the materials stateset
	bool isShaderMaterial(){return m_isShaderMaterial;}

	//Add/Remove shaders from the materials shader program
	//if this is the first shader added the shader program is created
	//and attached to the materials stateset (also flaging m_isShaderMaterial
	bool AddShader(osg::Shader* shader);
	//if this is the last shader removed, the program is also detached
	//and destroyed
	bool RemoveShader(osg::Shader* shader);

	//return a shader from it's index in the list
	//return null if out of range
	osg::Shader* GetShader(const unsigned int index);

	//get set the entire list of shaders for easier xmlwrapping, setShaderList
	//will perform AddShader on each of the passed list
	ShaderPtrVector GetShaderList()const{return m_shaders;}
	void SetShaderList(const ShaderPtrVector& list);


	void UseTangentSpace(const bool& useTangent);
	const bool& IsUsingTangetSpace()const{return m_useTangentSpace;}


	//Load a shader from a file into the shader passed in
	static void LoadShaderSource( osg::Shader* shader, const std::string& fileName );


	//add source to top of main source
	void CombineShaders(osg::Shader* main, const std::string & addFile); 

	//Feature level

	void SetFeatureLevel(SystemFeatureLevel* level){m_featureLevel = level;}
	SystemFeatureLevel* GetFeatureLevel(){return m_featureLevel;}

	void SetFallbackMaterial(HogBoxMaterial* material){p_fallbackMaterial = material;}
	HogBoxMaterial* GetFallbackMaterial(){return p_fallbackMaterial;}

	//
	//Returns this if a m_featureLevelName is "" or is matched by SystemInfo's current FeatureLevelName
	//if m_featureLevelName is provided but not matched then p_fallbackMaterial is tried, until a functional
	//material is found or no fallback is provided
	HogBoxMaterial* GetFunctionalMaterial();

protected:

	virtual ~HogBoxMaterial(void);

	//the osg stateset this material is wrapping
	osg::ref_ptr<osg::StateSet> m_stateset; 

	//basic gl material
	osg::ref_ptr<osg::Material> m_material; 
	osg::Vec3 m_ambientColour;
	osg::Vec3 m_diffuseColor;
	osg::Vec3 m_specularColour;
	double m_shininess;

	//Alpha
	bool m_alphaUsed;
	double m_alpha;

	//Back face lighting
	bool m_backFaceLit;
	osg::Material::Face m_lightFace;  
	

//Textures
	//map a texture to a sampler uniform
	class TextureUnit : public osg::Referenced {
	public:
		TextureUnit():osg::Referenced(){}
		bool enabled;
		osg::ref_ptr<osg::Texture> texture;
		osg::ref_ptr<osg::Uniform> sampler;
	protected:
		virtual ~TextureUnit(){}
	};
	typedef osg::ref_ptr<TextureUnit> TextureUnitPtr;
	typedef std::map<int, TextureUnitPtr> TextureChannelMap; 
	TextureChannelMap m_textureList;


//shaders

	//list of uniforms used by the material
	UniformPtrVector m_uniforms;
	
	//Is there a shader applied and currently active
	bool m_isShaderMaterial;

	//The program is created as soon as the first shader
	//is attached to our material
	osg::ref_ptr<osg::Program> m_program;

	//the list of shaders attached to the material
	ShaderPtrVector m_shaders;

	//flags if the shaders require tangent and binormal vectors
	bool m_useTangentSpace;


//Fallback system

	//HogBox Materials can provide a fallback material for if particular gl features
	//are't supported by the driver. A common example being a fallback to a fixed function
	//material from a shader based one if shaders aren't supported

	//the SystemFeatureLevel required for the material
	SystemFeatureLevelPtr m_featureLevel;

	//pointer to a material to fallback on if this materials m_featureLevelName is higher then
	//the sytems feature level
	osg::ref_ptr<HogBoxMaterial> p_fallbackMaterial;

};

typedef osg::ref_ptr<HogBoxMaterial> HogBoxMaterialPtr;

}; //end hogbox namespace