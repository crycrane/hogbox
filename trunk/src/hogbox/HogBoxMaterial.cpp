#include <hogbox/HogBoxMaterial.h>

#include <hogbox/HogBoxUtils.h>
#include <hogbox/SystemInfo.h>
#include <osg/BlendEquation>
#include <hogbox/NPOTResizeCallback.h>

using namespace hogbox;

////////////////////////////////////MATERIAL/////////////////////////////

HogBoxMaterial::HogBoxMaterial(void)
	: osg::Object(),
	m_stateset(new osg::StateSet()),
	m_material(new osg::Material()),
	m_backFaceLit(false),
	m_lightFace(osg::Material::FRONT),
	m_alphaUsed(false),
	m_alpha(1.0f),
	m_isShaderMaterial(false),
	m_program(NULL),
	m_useTangentSpace(false),
	m_featureLevel(NULL),
	p_fallbackMaterial(NULL)
{
	SetDefaultMaterial();
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
HogBoxMaterial::HogBoxMaterial(const HogBoxMaterial& material,const osg::CopyOp& copyop)
	: osg::Object(material, copyop)
{

}


HogBoxMaterial::~HogBoxMaterial(void)
{
	for(unsigned int i = 0; i<m_uniforms.size(); i++)
	{
		m_stateset->removeUniform(m_uniforms[i]);
		m_uniforms[i] = NULL;
	}
	m_uniforms.clear();

	for(unsigned int i = 0; i<m_shaders.size(); i++)
	{
		m_program->removeShader(m_shaders[i]);
		m_shaders[i] = NULL;
	}
	m_shaders.clear();

	//iterate texture channels
	TextureChannelMap::iterator texItr = m_textureList.begin();
	for(; texItr != m_textureList.end(); texItr++)
	{
		//remove any textures from the stateset
		if(texItr->second){
			if(texItr->second->texture){
				//m_stateset->removeTextureAttribute(texItr->first, osg::StateAttribute::TEXTURE);
				texItr->second->texture = NULL;
			}
			texItr->second = NULL;
		}
	}
	m_textureList.clear();

	
	m_program = NULL;
	m_stateset = NULL; //used to apply this material to an object
	m_material = NULL;
	
}

//
//overload set name to also set the name of the stateset
void HogBoxMaterial::setName( const std::string& name ){
	if(m_stateset){m_stateset->setName(name);}
	osg::Object::setName(name);
}

void HogBoxMaterial::setName( const char* name ){
	setName(std::string(name));
}

//
// Set the lighting material attributes for this osgmaterial
//
void HogBoxMaterial::SetMaterial(osg::Vec3 ambi, osg::Vec3 diffuse, osg::Vec3 spec, double exp)
{
	//store the material values
	m_ambientColour = ambi;
	osg::Vec4 ambi4 = osg::Vec4(ambi.x(), ambi.y(), ambi.z(), m_alpha);
	m_diffuseColor = diffuse;
	osg::Vec4 diffuse4 = osg::Vec4(diffuse.x(), diffuse.y(), diffuse.z(), m_alpha);
	m_specularColour = spec;
	osg::Vec4 spec4 = osg::Vec4(spec.x(), spec.y(), spec.z(), m_alpha);
	m_shininess = exp;

	//set the ambient colour
	m_material->setAmbient(m_lightFace, ambi4);
	//set the diffuse colour
	m_material->setDiffuse(m_lightFace, diffuse4);
	//set the specular colour
	m_material->setSpecular(m_lightFace, spec4);
	//set the shininess
	m_material->setShininess(m_lightFace, m_shininess);

	m_material->setColorMode(osg::Material::OFF);

	//apply the material to the stateset
	m_stateset->setAttributeAndModes(m_material.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

	this->SetTwoSidedLightingEnabled(m_backFaceLit);

	//normalise
	m_stateset->setMode( GL_NORMALIZE, osg::StateAttribute::ON);

}

//
//Set various alpha blending modes
//
void HogBoxMaterial::SetAlphaValue(const double& blend)
{
	m_alpha = blend;
	m_material->setAlpha(m_lightFace, blend);
}

void HogBoxMaterial::SetAlphaEnabled(const bool& enabled)
{
	if(enabled)
	{
		m_alphaUsed = true;

		osg::BlendEquation* blendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
		m_stateset->setAttributeAndModes(blendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
		//tell to sort the mesh before displaying it
		m_stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		m_stateset->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		SetAlphaValue(m_alpha);
	}else{
		//osg::BlendEquation* blendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
		//m_stateset->setAttributeAndModes(blendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
		//tell to sort the mesh before displaying it
		m_stateset->setRenderingHint(osg::StateSet::OPAQUE_BIN);
		m_stateset->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

		m_alphaUsed=false;
	}
}


const bool& HogBoxMaterial::GetAlphaEnabled() const
{
	return m_alphaUsed;
}

//BACK FACE LIGHTING

//
// Set back face light on or off
//
void HogBoxMaterial::SetTwoSidedLightingEnabled(const bool& val)
{
	if(val)
	{
		m_lightFace = osg::Material::FRONT_AND_BACK;
		m_stateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		m_backFaceLit = true;
	}else{
		m_lightFace = osg::Material::FRONT;
		m_stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		m_backFaceLit = false;
	}
}

//
// Apply some basic materials so we always get something
//
void HogBoxMaterial::SetDefaultMaterial()
{
	SetMaterial(	osg::Vec3(0.4f,0.4f,0.4f),
					osg::Vec3(0.8f,0.8f,0.8f),
					osg::Vec3(1.0f,1.0f,1.0f),
					32.0f);
	SetAlphaEnabled(false);
}


//
//Set a texture on a given channel, if the channel already ha a texture
//it is removed before adding the new texture
//
void HogBoxMaterial::SetTexture(const int& channel, osg::Texture* tex)
{
	TextureUnit* unit = NULL;
	//see if the channel is already occupied
	if(m_textureList.count(channel) > 0)
	{
		//remove the existing texture from the material
		unit = m_textureList[channel];

		m_stateset->removeTextureAttribute(channel, osg::StateAttribute::TEXTURE);
		unit->texture = NULL;

		//remove the sampler uniform
		m_stateset->removeUniform(unit->sampler);
		unit->sampler = NULL;

		//flag the channel as off
		unit->enabled = false;
	}

	//if new texture is null then return and leave the channel empty
	if(tex == NULL){return;}

	//if the unit is still null, allocate a new one and add it to the map
	if(!unit){
		unit = new TextureUnit();
		m_textureList[channel] = unit;
	}

	//create the sampler uniform based on the the textures name (uniforms have to match shader names
	//so we do the same last word chack as uniforms
	std::string uniformName = tex->getName();

	//if a dot seperation sequence is found
	//use the word following the last dot
	size_t found;
	found = tex->getName().find_last_of(".");
	if(found+1 != std::string::npos)
	{
		uniformName = tex->getName().substr(found+1,tex->getName().size()-1);
	}
	unit->sampler = new osg::Uniform(uniformName.c_str(), channel);

	//store texture
	unit->texture = tex;

	//apply the texture to the materials stateset
	m_stateset->setTextureAttributeAndModes(channel, unit->texture, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	m_stateset->addUniform(unit->sampler, osg::StateAttribute::ON);

	osg::Texture2D* tex2D = dynamic_cast<osg::Texture2D*>(tex); 	
	if(tex2D){
		//if we don't want to resize the texture to power of two
		//and the hardware dows not support npot textures. The apply
		//a NPOTResizeCallback
		if(tex2D->getResizeNonPowerOfTwoHint() == false){
			osg::ref_ptr<hogbox::NPOTResizeCallback> resizer = new hogbox::NPOTResizeCallback(tex2D, channel, m_stateset.get());
			if(resizer->useAsCallBack()){
				tex2D->setSubloadCallback(resizer.get());
			}
		}
	}


	//flag the texture as on
	unit->enabled = true;

	return;
}

//
//Get a texture by the channel it is applied to
//
osg::Texture* HogBoxMaterial::GetTexture(const int& channel)
{
	//see if we can find a texture in channel
	if(m_textureList.count(channel) > 0)
	{
		return m_textureList[channel]->texture;
	}
	return NULL;
}

bool HogBoxMaterial::IsTextureEnabled(const int& channel)
{
	//see if we can find a texture in channel
	if(m_textureList.count(channel) > 0)
	{
		return m_textureList[channel]->enabled;
	}
	return false;
}


//
// Toggle the on off state of the materials
// multi texture channels
//
void HogBoxMaterial::EnableTexture(const int& channel, bool on)
{
	//see if we can find a texture in channel
	if(m_textureList.count(channel) > 0)
	{
		if( m_textureList[channel]->enabled != on)
		{
			m_textureList[channel]->enabled = on;
			if(on){
				m_stateset->setTextureAttributeAndModes(channel, m_textureList[channel]->texture, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
			}else{
				m_stateset->setTextureAttributeAndModes(channel, m_textureList[channel]->texture, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);
			}
		}
	}
}

//
//Return the number of valid textures in our list
//
unsigned int HogBoxMaterial::GetNumTextures()const
{
	unsigned int count = 0;
	hogbox::HogBoxMaterial::TextureChannelMap::const_iterator itr = m_textureList.begin();
	for(; itr != m_textureList.end(); itr++)
	{
		if((*itr).second != NULL)
		{
			if((*itr).second->texture != NULL)
			{
				count++;
			}
		}
	}
	return count;
}


//Uniforms

bool HogBoxMaterial::AddUniform(osg::Uniform* uni)
{
	if(!uni){return false;}
	if(!m_stateset){return false;}
	
	//apply to the stateset
	m_stateset->addUniform(uni, osg::StateAttribute::ON);
	//add to the pointer list
	m_uniforms.push_back(uni);

	return true;
}

bool HogBoxMaterial::RemoveUniform(osg::Uniform* uni)
{
	if(!uni){return false;}
	if(!m_stateset){return false;}
	
	//remove from state
	m_stateset->removeUniform(uni);

	//remove from pointer list
	hogbox::UniformPtrVector::iterator itr=m_uniforms.begin();
	for(; itr != m_uniforms.end(); itr++)
	{
		if((*itr) == uni){m_uniforms.erase(itr);return true;}
	}	

	return false;
}

//
// get a uniform by the index
//
osg::Uniform* HogBoxMaterial::GetUniform(const unsigned int index)
{
	//check bounds
	if( (index < 0) || (index>=m_uniforms.size()) )
 	{return NULL;}
	return m_uniforms[index];
}

//
//Get a uniform in the list by its name and return
//
osg::Uniform* HogBoxMaterial::GetUniform(const std::string& name)
{
	//loop all the uniforms in the list
	for(unsigned int i = 0; i<m_uniforms.size(); i++)
	{
		//compare to our desired name
		if(m_uniforms[i]->getName().compare(name) == 0)
		{
			return m_uniforms[i];
		}
	}
	return NULL;
}

//
//Set the whole list of uniforms (probably from xml)
//will pass over the new uniforms add each item individually
//
void HogBoxMaterial::SetUniformList(const UniformPtrVector& list)
{
	for(unsigned int i=0; i<list.size(); i++)
	{
		this->AddUniform(list[i]);
	}
}


//Shaders

//
//Add/Remove shaders from the materials shader program
//if this is the first shader added the shader program is created
//and attached to the materials stateset (also flaging m_isShaderMaterial
//
bool HogBoxMaterial::AddShader(osg::Shader* shader)
{
	if(!shader){return false;}
	if(!m_stateset){return false;}

	//if the program is null, create it
	if(m_program == NULL)
	{
		m_program = new osg::Program();
		m_program->setName( this->getName()+"_ShaderProgram" );
		//add the shader program to this materials stateset
		m_stateset->setAttributeAndModes(m_program.get(), osg::StateAttribute::ON);
		//UseTangentSpace();
	}

	//add the shader to the program and store
	if(!m_program->addShader(shader)){return false;}

	m_shaders.push_back(shader);
	return true;
}

//
//if this is the last shader removed, the program is also detached
//and destroyed
//
bool HogBoxMaterial::RemoveShader(osg::Shader* shader)
{
	if(!shader){return false;}
	if(!m_stateset){return false;}
	if(!m_program){return false;}
	
	//remove from shader
	if(!m_program->removeShader(shader)){return false;}

	//remove from pointer list
	hogbox::ShaderPtrVector::iterator itr=m_shaders.begin();
	for(; itr != m_shaders.end(); itr++)
	{
		if((*itr) == shader){m_shaders.erase(itr);return true;}
	}	
	return false;
}

//
//return a shader from it's index in the list
//return null if out of range
//
osg::Shader* HogBoxMaterial::GetShader(const unsigned int index)
{
	if(index < 0 || index >= m_shaders.size()){return NULL;}
	return m_shaders[index];
}

//
//Set the entire list of Shaders by passing through AddShader
//
void HogBoxMaterial::SetShaderList(const ShaderPtrVector& list)
{
	for(unsigned int i=0; i<list.size(); i++)
	{
		this->AddShader(list[i]);
	}
}


//
//tell the shader to where to find tangent space vectors
//
void HogBoxMaterial::UseTangentSpace(const bool& useTangentSpace )
{
	m_useTangentSpace = useTangentSpace;
	//bind the tangent space attributes
	if(m_useTangentSpace)
	{
		m_program->addBindAttribLocation ("tangent", 6);
		m_program->addBindAttribLocation ("binormal", 7);
	}else{
		m_program->removeBindAttribLocation("tangent");
		m_program->removeBindAttribLocation("binormal");
	}
}

//
//
//
void HogBoxMaterial::CombineShaders(osg::Shader* main, const std::string & addFile)
{
	//check main is there
	if(!main)
	{return;}

	osg::ref_ptr<osg::Shader> addShader = new osg::Shader(main->getType());
	this->LoadShaderSource(addShader.get(), addFile);

	//get the source loaded to be added
	std::string addSource = addShader->getShaderSource();
	//get the original source
	std::string originalSource = main->getShaderSource();

	//combine the two sources and add to the main
	std::string newMainSource = addSource + originalSource;

	main->setShaderSource(newMainSource);
}

//
// Load a shader source file into the passed in shader object
// load source from a file.
//
void HogBoxMaterial::LoadShaderSource( osg::Shader* shader, const std::string& fileName )
{
    std::string fqFileName = osgDB::findDataFile(fileName);
    if( fqFileName.length() != 0 )
    {
        if(!shader->loadShaderSourceFromFile( fqFileName.c_str() ))
		{
			std::cout << "Could not Load shader file \""<<fileName.c_str() << std::endl;
		}
    }
    else
    {
		std::cout << "Shader file \"" << fileName.c_str() << "\" not found." << std::endl;
    }
}


//
//Returns this if a m_featureLevelName is "" or is matched by SystemInfo's current FeatureLevelName
//if m_featureLevelName is provided but not matched then p_fallbackMaterial is tried, until a functional
//material is found or no fallback is provided in which case NULL is returned
//
HogBoxMaterial* HogBoxMaterial::GetFunctionalMaterial()
{
	if(!m_featureLevel){return this;}

	//compare to system info levels
	if(SystemInfo::Instance()->IsFeatureLevelSupported(m_featureLevel)) //features are avaliable to support the material
	{
		return this;	
	}else{ //try fallback
		if(p_fallbackMaterial)
		{
			osg::notify(osg::NOTICE) << "HogBoxMaterial NOTICE: Material '" << this->getName() << "' uses FeatureLevel '" << m_featureLevel->getName() << "' which is not supported," << std::endl
			<< "                                                                               Fallingback onto Material '" << p_fallbackMaterial->getName() << "'." << std::endl;
			return p_fallbackMaterial->GetFunctionalMaterial();
		}
	}
	//not supported, warn user and return null
	osg::notify(osg::WARN) << "HogBoxMaterial WARN: Material '" << this->getName() << "' uses FeatureLevel '" << m_featureLevel->getName() << "' which is not supported," << std::endl
	<< "                                                                               No fallback is provided so the material will not function as expected if at all." << std::endl;
	return NULL;
}