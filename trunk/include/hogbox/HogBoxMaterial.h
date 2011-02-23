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
	
	//mapping mask, all can be combined to make uber shader :)
	enum MappingMask
    {
        BLEND = 1,
        LIGHTING = 2,
        FOG = 4,
        DIFFUSE_MAP = 8, //< Texture in unit 0
		//REFLECTION_MAP = 16,  //< Texture in unit 1 
        NORMAL_MAP = 16,  //< Texture in unit 2 and vertex attribute array 12
		//MASK_MAP = 64, //< Texture unit 4, r = mask specular, g = mask reflection, b = ? 
		//PARALLAX_MAP = 128,  //< Texture in unit 4
		//GLOW_MAP = 256, //<Texture unit 5 buffer 1 (all above go to default color buffer 0)
    };
	
	//a single lighting style can be set
	enum LightingMode
	{
		PER_VERTEX,
		PER_PIXEL,
		IMAGE_BASED
	};
	
	//shader can have a precision level set
	//this effects the precision level given to varyings in shaders
	enum ShaderDetail
	{
		LOW,
		MEDIUM,
		HIGH
	};

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
	
	//enable disable lighting
	void SetLightingEnabled(const bool& val);
	const bool& GetLightingEnabled()const{return m_isLit;}

	//enable disable 2 sided lighting
	void SetTwoSidedLightingEnabled(const bool& val);
	const bool& GetTwoSidedLightingEnabled()const{return m_backFaceLit;}
	
	//render bins
	void SetBinNumber(const int& num);
	const int& GetBinNumber()const;

	//0=default, 1=opaque, 2=trasparent
	void SetBinHint(const int& hint);
	const int& GetBinHint()const;
//textures

	//set a texture to a specified channel, also creating a shader
	//sampler for the texture. The samplers name is taken from the
	//textures name, to allow unique texture names but the correct sampler
	//name the sampler name portion of the texture name can be seperated
	//at the end by a dot for example 'mytexture.diffuseMap' would provide
	//the shader with sampler name 'diffuseMap'.
	void SetTexture(const int& channel,osg::Texture* tex);

	//return the texture if any in the channel, if
	//there is no texture in the channel NULL is returned
	osg::Texture* GetTexture(const int& channel);
	std::string GetTextureSamplerName(const int& channel);

	//is there an enabled texture in the channel
	bool IsTextureEnabled(const int& channel);

	//enable/disable the texture in channel
	void EnableTexture(const int& channel, bool on);

	//return the number of channels containing a valid texture
	unsigned int GetNumTextures()const;
	
	
	//texture matrix
	//apply a texture matrix to this stateset also setting the hb_texmax uniform to represent it
	void ApplyTextureMatrix(const int& channel, osg::TexMat* texMat);

//uniforms

	//add a unform to the state and to our list
	bool AddUniform(osg::Uniform* uni); 
	bool RemoveUniform(osg::Uniform* uni); 
	osg::Uniform* GetUniform(const unsigned int index);
	osg::Uniform* GetUniform(const std::string& name);	
	const unsigned int GetNumUniforms(){return m_uniforms.size();}

	//get set the entire list of uniforms for easier xmlwrapping, setUniformList
	//performa AddUniform on each of the passed list
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
	
	//Add shader of type from file
	bool AddShaderFromFile(const std::string file, osg::Shader::Type type=osg::Shader::VERTEX); 

	//Get or create the materials shader program, will also apply to material
	//stateset if it is created
	osg::Program* GetOrCreateProgram();

	//binds the hb_tangent anf hb_binormal attributes to the material program
	void UseTangentSpace(const bool& useTangent);
	const bool& IsUsingTangetSpace()const{return m_useTangentSpace;}
	
	//
	//bind the hb_boneWeight 0-x(4?) attributes to the material program
	//Will also add the default computeSkinning function to the vertex shader
	//user must ensure the function is forward declared and used in there own 
	//material shader
	//note that the geode we apply the material to, must contain a rig etc
	void UseSkinning(const bool& useSkinning);
	const bool& IsUsingSkinning()const{return m_useSkinning;}
	//
	//Shader composer
	
	//Create a shader based on the materials current state, 
	//overideExiting, optionally overwrite existing shaders
	void ComposeShaderFromMaterialState(ShaderDetail detail = HIGH, LightingMode lightingMode = PER_VERTEX, bool overrideExisting = false);


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
	
	//the mapping mask for the material to describe to auto shader gen how to handle the textures etc
	MappingMask m_mappingMask;
	
	//the light style for shader gen
	LightingMode m_lightingMode;
	
	//the detail/precision level of the shader gen
	ShaderDetail m_shaderDetailLevel;

	//hogbox material handles to the various osg stateset values used to describe a hogboxmaterial
	
	//is the material affected by lights
	bool m_isLit;

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
	
	int m_binNumber;
	int m_binMode;
	

//Textures
	//map a texture to a sampler uniform
	class TextureUnit : public osg::Referenced {
	public:
		TextureUnit():osg::Referenced(){}
		bool enabled;
		osg::ref_ptr<osg::Texture> texture;
		osg::ref_ptr<osg::Uniform> sampler;
		//texture matrix for the uni
		osg::ref_ptr<osg::TexMat> texmat;
		osg::ref_ptr<osg::Uniform> matUniform;
	protected:
		virtual ~TextureUnit(){}
	};
	typedef osg::ref_ptr<TextureUnit> TextureUnitPtr;
	typedef std::map<int, TextureUnitPtr> TextureChannelMap; 
	
	//the list of texture units applied to the material mapped
	//by their channel
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

	//flags if the shaders requires tangent and binormal vectors, these are currently bound to channels 12 and 13
	bool m_useTangentSpace;

	//flags if we are trying to use skinning by binding the expected boneWeight attributes and adding the default
	//vertex shader functions used for skinning
	bool m_useSkinning;

	//the built in uniforms provided by hogbox material to replace gl_material etc in gles2 shaders
	osg::ref_ptr<osg::Uniform> m_ambientColourUni;
	osg::ref_ptr<osg::Uniform> m_diffuseColourUni;
	osg::ref_ptr<osg::Uniform> m_specularColourUni;
	osg::ref_ptr<osg::Uniform> m_specularExpUni;
	
	osg::ref_ptr<osg::Uniform> m_alphaUni;
	
	osg::ref_ptr<osg::Uniform> m_twoSidedUni;

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