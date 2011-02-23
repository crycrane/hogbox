#include <hogbox/HogBoxMaterial.h>

#include <hogbox/HogBoxUtils.h>
#include <hogbox/SystemInfo.h>
#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <hogbox/NPOTResizeCallback.h>

using namespace hogbox;

//static default skinning shader function
static const char* computeSkinningVertSource = {  
"in vec4 boneWeight0;\n"
"in vec4 boneWeight1;\n"
"in vec4 boneWeight2;\n"
"in vec4 boneWeight3;\n"
"uniform int nbBonesPerVertex;\n"
"uniform mat4 matrixPalette[19];//MAX_MATRIX];\n"
"vec4 position;\n"
"vec3 normal;\n"
"\n"
"void ComputeSkinnedNormalAndPosition(vec4 boneWeight)\n"
"{\n"
"    int matrixIndex;\n"
"    float matrixWeight;\n"
"    for (int i = 0; i < 2; i++)\n"
"    {\n"
"        matrixIndex =  int(boneWeight[0]);\n"
"        matrixWeight = boneWeight[1];\n"
"        mat4 matrix = matrixPalette[matrixIndex];\n"
"        // correct for normal if no scale in bone\n"
"        mat3 matrixNormal = mat3(matrix);\n"
"        position += matrixWeight * (matrix * gl_Vertex );\n"
"        normal += matrixWeight * (matrixNormal * gl_Normal );\n"
"        boneWeight = boneWeight.zwxy;\n"
"    }\n"
"}\n"
"\n"
"void DoSkinning()\n"
"{\n"
"// there is 2 bone data per attributes\n"
"    if (nbBonesPerVertex > 0)\n"
"        ComputeSkinnedNormalAndPosition(boneWeight0);\n"
"    if (nbBonesPerVertex > 2)\n"
"        ComputeSkinnedNormalAndPosition(boneWeight1);\n"
"    if (nbBonesPerVertex > 4)\n"
"        ComputeSkinnedNormalAndPosition(boneWeight2);\n"
"    if (nbBonesPerVertex > 6)\n"
"        ComputeSkinnedNormalAndPosition(boneWeight3);\n"
"//position = gl_Vertex;\n"
"//normal = gl_Normal;\n"
"}\n"
}; 

////////////////////////////////////MATERIAL/////////////////////////////

HogBoxMaterial::HogBoxMaterial(void)
	: osg::Object(),
	m_stateset(new osg::StateSet()),
	m_material(new osg::Material()),
	m_isLit(true),
	m_backFaceLit(false),
	m_lightFace(osg::Material::FRONT),
	m_alphaUsed(false),
	m_alpha(1.0f),
	m_binNumber(0),
	m_binMode(0),
	m_isShaderMaterial(false),
	m_program(NULL),
	m_useTangentSpace(false),
	m_useSkinning(false),
	m_featureLevel(NULL),
	p_fallbackMaterial(NULL)
{	
	
	//set this as the user data of our stateset so it can be retreived while transversing the graph
	m_stateset->setUserData(this);
	
	//the built in uniforms provided by hogbox material to replace gl_material etc in gles2 shaders
	m_ambientColourUni = new osg::Uniform("hb_ambientColor", m_ambientColour);
	this->AddUniform(m_ambientColourUni.get());
	m_diffuseColourUni = new osg::Uniform("hb_diffuseColor", m_diffuseColor);
	this->AddUniform(m_diffuseColourUni.get());
	m_specularColourUni = new osg::Uniform("hb_specularColor", m_specularColour);
	this->AddUniform(m_specularColourUni.get());
	m_specularExpUni = new osg::Uniform("hb_shine", (float)m_shininess);
	this->AddUniform(m_specularExpUni.get());
	
	m_alphaUni = new osg::Uniform("hb_alpha", (float)m_alpha);
	this->AddUniform(m_alphaUni.get());
	
	m_twoSidedUni = new osg::Uniform("hb_isTwoSided", (int)m_backFaceLit);
	this->AddUniform(m_twoSidedUni.get());
	
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
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
	m_stateset->setAttributeAndModes(m_material.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	//normalise
	m_stateset->setMode( GL_NORMALIZE, osg::StateAttribute::ON);
#endif
	
	this->SetTwoSidedLightingEnabled(m_backFaceLit);


	
	//set hogbox material uniforms
	m_ambientColourUni->set(m_ambientColour);
	m_diffuseColourUni->set(m_diffuseColor);
	m_specularColourUni->set(m_specularColour);
	m_specularExpUni->set((float)m_shininess);
	
	m_alphaUni->set((float)m_alpha);
	
	m_twoSidedUni->set((int)m_backFaceLit);

}

//
//Set various alpha blending modes
//
void HogBoxMaterial::SetAlphaValue(const double& blend)
{
	m_alpha = blend;
	m_material->setAlpha(m_lightFace, blend);
	m_alphaUni->set((float)m_alpha);
}

void HogBoxMaterial::SetAlphaEnabled(const bool& enabled)
{
	if(enabled)
	{
		m_alphaUsed = true;

		osg::BlendEquation* blendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
		m_stateset->setAttributeAndModes(blendEquation,osg::StateAttribute::ON);
		osg::BlendFunc* blendFunc = new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE);//GL_ONE_MINUS_SRC_ALPHA);
		m_stateset->setAttributeAndModes(blendFunc,osg::StateAttribute::ON);
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

//enable disable lighting
void HogBoxMaterial::SetLightingEnabled(const bool& val)
{
	m_isLit = val;
	if(m_isLit)
	{
		//enable lighting
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
		m_stateset->setMode(GL_LIGHTING,osg::StateAttribute::ON);
#endif
	}else{
		//diable lighting	
		m_stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);		
	}
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
	m_twoSidedUni->set((int)m_backFaceLit);
}

void HogBoxMaterial::SetBinNumber(const int& num)
{
	m_binNumber = num;
	m_stateset->setBinNumber(num);
}

const int& HogBoxMaterial::GetBinNumber()const
{
	return m_binNumber;
}

//0=default, 1=opaque, 2=trasparent
void HogBoxMaterial::SetBinHint(const int& hint)
{
	m_binMode = hint;
	m_stateset->setRenderingHint(m_binMode);
}

const int& HogBoxMaterial::GetBinHint()const
{
	return m_binMode;
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
	if(found != std::string::npos)
	{
		if(found+1 < uniformName.size()-1)
		{
			uniformName = tex->getName().substr(found+1,tex->getName().size()-1);
		}
	}

	unit->sampler = new osg::Uniform(uniformName.c_str(), channel);

	//store texture
	unit->texture = tex;

	//apply the texture to the materials stateset
	m_stateset->setTextureAttributeAndModes(channel, unit->texture, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	m_stateset->addUniform(unit->sampler, osg::StateAttribute::ON);

	osg::Texture2D* tex2D = dynamic_cast<osg::Texture2D*>(tex);
	osg::TexMat* texMat = NULL;
	if(tex2D){
		//if we don't want to resize
		if(tex2D->getResizeNonPowerOfTwoHint() == false)
		{
			//if npot isn't supported
			if(!SystemInfo::Instance()->npotTextureSupported())
			{
				//if we don't want to resize the texture to power of two
				//and the hardware does not support npot textures. Then apply
				//a NPOTResizeCallback				
				osg::ref_ptr<hogbox::NPOTResizeCallback> resizer = new hogbox::NPOTResizeCallback(tex2D, channel, NULL);
				if(resizer->useAsCallBack()){
					tex2D->setSubloadCallback(resizer.get());
				}
				texMat = resizer->GetScaleMatrix();
			}
		}
	}

	//make sure we have a default tex matrix
	if(!texMat){texMat = new osg::TexMat();}
	
	this->ApplyTextureMatrix(channel, texMat);

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

std::string HogBoxMaterial::GetTextureSamplerName(const int& channel)
{
	//see if we can find a texture in channel
	if(m_textureList.count(channel) > 0)
	{
		return m_textureList[channel]->sampler->getName();
	}
	return "";
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

//
//apply a texture matrix to this stateset also setting the hb_texmax uniform to represent it
//
void HogBoxMaterial::ApplyTextureMatrix(const int& channel, osg::TexMat* texMat)
{
	
	TextureUnit* unit = NULL;
	//see if the channel is already occupied
	if(m_textureList.count(channel) > 0)
	{
		//remove the existing texture from the material
		unit = m_textureList[channel];
	}
	
	//if the unit is still null, allocate a new one and add it to the map
	if(!unit){
		unit = new TextureUnit();
		m_textureList[channel] = unit;
	}
	
	if(!unit->matUniform.get()){
		//create the hogbox uniform to represent the tex matrix based on the the channel
		std::ostringstream uniformName;
		uniformName << "hb_texmat" << channel;
		unit->matUniform = new osg::Uniform(uniformName.str().c_str(), texMat->getMatrix());
		m_stateset->addUniform(unit->matUniform, osg::StateAttribute::ON);		
	}
	
	unit->texmat = texMat;
	unit->matUniform->set(texMat->getMatrix());
	
	//apply the texture matrix to the stateset
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
	m_stateset->setTextureAttributeAndModes(channel, texMat, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
#endif
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
	if(!m_stateset.get()){return false;}

	GetOrCreateProgram();

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
//Add shader of type from file
//
bool HogBoxMaterial::AddShaderFromFile(const std::string file, osg::Shader::Type type)
{
	osg::Shader* shader = new osg::Shader(type);
	if(!shader){return false;}
	
	//load the source file
	if(!shader->loadShaderSourceFromFile(file))
	{
		//if it fails once then try using osgDB::findFile which is needed on IPhone at mo
		std::string tryFileName = osgDB::findDataFile(shader->getFileName());
		if(!tryFileName.empty())
		{
			if(!shader->loadShaderSourceFromFile(file))
			{return false;}
		}
		
		shader->setFileName(tryFileName);
			
	}else{
		shader->setFileName(file);
	}
	return AddShader(shader);
}

//
//Get or create the materials shader program, will also apply to material
//stateset if it is created
//
osg::Program* HogBoxMaterial::GetOrCreateProgram()
{
	//if the program is null, create it
	if(!m_program.get())
	{
		m_program = new osg::Program();
		m_program->setName( this->getName()+"_ShaderProgram" );
		//add the shader program to this materials stateset
		m_stateset->setAttributeAndModes(m_program.get(), osg::StateAttribute::ON);
	}
	return m_program.get();
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
		//if the program is null, create it
		GetOrCreateProgram();
		
		m_program->addBindAttribLocation ("hb_tangent", 6);
		//m_program->addBindAttribLocation ("hb_binormal", 7);
	}else{
		if(m_program.get()){m_program->removeBindAttribLocation("hb_tangent");}
		//m_program->removeBindAttribLocation("hb_binormal");
	}
}

//
//bind the hb_boneWeight 0-x(4?) attributes to the material program
//Will also add the default computeSkinning function to the vertex shader
//user must ensure the function is forward declared and used in there own 
//material shader
//note that the geode we apply the material to, must contain a rig etc
//
void HogBoxMaterial::UseSkinning(const bool& useSkinning)
{
	m_useSkinning = useSkinning;
	unsigned int nbAttribs = 2;
	if(m_useSkinning)
	{
		GetOrCreateProgram();
		//bind the boneWeight attribte location to the material program
		//for now asume there are 4
		unsigned int attribIndex = 9;
		for (unsigned int i = 0; i < nbAttribs; i++)
		{
			std::stringstream ss;
			ss << "boneWeight" << i;
			m_program->addBindAttribLocation(ss.str(), attribIndex + i);
			osg::notify(osg::INFO) << "set vertex attrib " << ss.str() << std::endl;
		}

		//now load the default computeSkinning function into a vertex shader to apply to the program
		osg::Shader* skinningSource = new osg::Shader(osg::Shader::VERTEX, computeSkinningVertSource);
		m_program->addShader(skinningSource);

	}else{
		//Remove hb_boneWeight attributes if any
		if(m_program.get())
		{
			for (unsigned int i = 0; i < nbAttribs; i++)
			{
				std::stringstream ss;
				ss << "boneWeight" << i;
				m_program->removeBindAttribLocation(ss.str());
			}
		}
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
//Shader composer

//Create a shader based on the materials current state, 
//overideExiting, optionally overwrite existing shaders
//
void HogBoxMaterial::ComposeShaderFromMaterialState(ShaderDetail detail, LightingMode lightingMode, bool overrideExisting)
{
	
	std::string diffuseName, normalName, reflectionName = "";
	//create our mapping mask
	int stateMask;
	bool dif=false;
	bool norm=false;
	
	if(m_alphaUsed)
	{
		OSG_INFO << "HOGBOX ShaderGen: Using Alpha." << std::endl;
		stateMask |= BLEND;
	}
	
	if(m_isLit)
	{
		OSG_INFO << "HOGBOX ShaderGen: Using Lighting." << std::endl;
		stateMask |= LIGHTING;
	}
	
	//is diffuse map present
	if(GetTexture(0))
	{
		dif=true;
		OSG_INFO << "HOGBOX ShaderGen: Using Diffuse Map." << std::endl;
		stateMask |= DIFFUSE_MAP;diffuseName=GetTextureSamplerName(0);
	}

	if(GetTexture(1))
	{
		norm=true;
		OSG_INFO << "HOGBOX ShaderGen: Using Reflection Map." << std::endl;
		//stateMask |= REFLECTION_MAP;
	}
	
	//if(GetTexture(2))
	//{stateMask |= NORMAL_MAP;}
	
	if(GetTexture(3))
	{
		OSG_INFO << "HOGBOX ShaderGen: Using Mask Map." << std::endl;
		//stateMask |= MASK_MAP;
	}
	
	if(GetTexture(4))
	{
		//stateMask |= PARALLAX_MAP;
	}
	
	if(GetTexture(5))
	{
		//stateMask |= GLOW_MAP;
	}
	
	//m_mappingMask = stateMask;
	m_lightingMode = lightingMode;
	m_shaderDetailLevel = detail;
	
	//set the precision string from our detail level
	std::string plevel;
	if(detail == LOW)
	{
		plevel = "lowp";
	}else if(detail == MEDIUM){
		plevel = "mediump";
	}else {
		plevel = "highp";
	}

	
	
	//common defines form vert and frag
	std::ostringstream vert;
	std::ostringstream frag;
	
	//define precision if not avaliable
	vert <<	"#ifndef GL_ES" << std::endl <<
			"	#if (__VERSION__ <= 110)" << std::endl << 
			"		#define lowp" << std::endl <<
			"		#define mediump" << std::endl <<
			"		#define highp" << std::endl <<
			"	#endif" << std::endl <<
			"#endif" << std::endl;
	
	//varying variables
	
	vert << "varying " << plevel << " vec4 vertColor;\n";
	
    // write varyings
    if (m_isLit && !norm)
    {
        vert << "varying " << plevel << " vec3 normalDir;\n";
    }
	
    if (m_isLit || norm)
    {
        vert << "varying " << plevel << " vec3 lightDir;\n";
        vert << "varying " << plevel << " vec3 viewDir;\n";
    }
	
	//add tex coords channel one if any texture mappings are used
	if (dif || norm) 
	{
		vert << "varying " << plevel << " vec2 texCoord" << 0 << ";" << std::endl;
	}
	
	vert << std::endl;
	
	//copy header and varying into fragment shader
    frag << vert.str();

	
	//vertex attributes
	
	//vetex
	vert << "attribute vec4 osg_Vertex;" << std::endl;
	
	//only use normal for lighting
	if (m_isLit || norm)
    {vert << "attribute vec3 osg_Normal;" << std::endl;}
	
		
	//add tex coords channel one if any texture mappings are used
	if (dif || norm) 
	{
		vert << "attribute vec4 osg_MultiTexCoord" << 0 << ";" << std::endl;
		vert << "" << " uniform mat4 hb_texmat" << 0 << ";" << std::endl;
	}
	
	vert << std::endl;
	
	//osg built in uniforms
	vert << "" << " uniform mat3 osg_NormalMatrix;" << std::endl <<
			"" << " uniform mat4 osg_ModelViewProjectionMatrix;" << std::endl <<
			"" << " uniform mat4 osg_ModelViewMatrix;" << std::endl;

	
	vert << std::endl;
	
	//hogbox built in uniforms
	//the hogbox material uniforms are written to either the vertex or fragment shader
	//depending on if we are vertex or pixel lighting
	if (m_isLit || norm)
	{
		frag << "" << " uniform vec3 hb_ambientColor;" << std::endl <<
				"" << " uniform vec3 hb_diffuseColor;" << std::endl <<
				"" << " uniform vec3 hb_specularColor;" << std::endl <<
				"" << " uniform float hb_shine;" << std::endl <<
				"" << " uniform float hb_alpha;" << std::endl <<
				"" << " uniform float hb_isTwoSided;" << std::endl;
		frag << std::endl;
	}else{
		vert << "uniform vec3 hb_ambientColor;" << std::endl <<
		"" << " uniform vec3 hb_diffuseColor;" << std::endl <<
		"" << " uniform vec3 hb_specularColor;" << std::endl <<
		"" << " uniform float hb_shine;" << std::endl <<
		"" << " uniform float hb_alpha;" << std::endl <<
		"" << " uniform float hb_isTwoSided;" << std::endl;
		vert << std::endl;		
	}
	
	
	//decalre vertex main
	
	vert << "\n"\
	"void main()\n"\
	"{\n"\
	"  gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n";
	
	if (dif || norm) 
	{
		vert << "  " << plevel << " vec4 tc0 = osg_MultiTexCoord0* hb_texmat0;\n";// * hb_texmat0;
		//vert << "  tc0.x *= hb_texmat0[0][0];\n";// tc0.y*=hb_texmat0[1][1];\n";
		vert << "  texCoord0 = tc0.xy;\n";
	}

	if (norm) 
	{
		vert << 
		"  " << plevel << " vec3 n = osg_NormalMatrix * osg_Normal;\n"\
		"  " << plevel << " vec3 t = osg_NormalMatrix * tangent;\n"\
		"  " << plevel << " vec3 b = cross(n, t);\n"\
		"  " << plevel << " vec3 dir = -vec3(osg_ModelViewMatrix * osg_Vertex);\n"\
		"  viewDir.x = dot(dir, t);\n"\
		"  viewDir.y = dot(dir, b);\n"\
		"  viewDir.z = dot(dir, n);\n"\
		"  " << plevel << " vec4 lpos = vec4(100.0,100.0,100.0,1.0);\n"\
		"  if (lpos.w == 0.0)\n"\
		"    dir = lpos.xyz;\n"\
		"  else\n"\
		"    dir += lpos.xyz;\n"\
		"  lightDir.x = dot(dir, t);\n"\
		"  lightDir.y = dot(dir, b);\n"\
		"  lightDir.z = dot(dir, n);\n";
	}
	else if (m_isLit)
	{
		vert << 
		"  normalDir = osg_NormalMatrix * osg_Normal;\n"\
		"  " << plevel << " vec3 dir = -vec3(osg_ModelViewMatrix * osg_Vertex);\n"\
		"  viewDir = dir;\n"\
		"  " << plevel << " vec4 lpos = vec4(100.0,100.0,100.0,1.0);\n"\
		"  if (lpos.w == 0.0)\n"\
		"    lightDir = lpos.xyz;\n"\
		"  else\n"\
		"    lightDir = lpos.xyz + dir;\n";
	}
	else
	{
		vert << "  vertColor.xyz = hb_diffuseColor;vertColor.a=1.0;\n";
	}
	
	vert << "}\n";
		
		
	//
	//fragment shader
	
	//decalre any samplers
	hogbox::HogBoxMaterial::TextureChannelMap::const_iterator itr = m_textureList.begin();
	for(; itr != m_textureList.end(); itr++)
	{
		//check we have a valid sampler first
		if((*itr).second->sampler)
		{
			//add tex sampler (needs to handle rects)
			frag << "uniform sampler2D " << diffuseName << ";" << std::endl;
		}
	}
	
	frag << std::endl;
	
	frag << "\n"\
	"void main()\n"\
	"{\n";
	
	if (dif)
	{
		frag << "  " << plevel << " vec4 base = texture2D(" << diffuseName << ", texCoord0.xy);\n";
	}
	else
	{
		frag << "  " << plevel << " vec4 base = vec4(1.0);\n";
	}
	
	if (norm)
	{
		//frag << "  vec3 normalDir = texture2D(normalMap, texCoord.xy).xyz*2.0-1.0;\n";
	}
	
	if (m_isLit || norm)
	{
		frag << 
		"  " << plevel << " vec3 nd = normalize(normalDir);\n"\
		"  " << plevel << " vec3 ld = normalize(lightDir);\n"\
		"  " << plevel << " vec3 vd = normalize(viewDir);\n"\
		"  " << plevel << " vec4 color = vec4(0.0,0.0,0.0,1.0);\n"\
		"  color += hb_ambientColor;\n"\
		"  float diff = max(dot(ld, nd), 0.0);\n"\
		"  color += hb_diffuseColor * diff;\n"\
		"  color *= base;\n"\
		"  if (diff > 0.0)\n"\
		"  {\n"\
		"    " << plevel << " vec3 halfDir = normalize(ld+vd);\n"\
		"    color.rgb += base.a * hb_specularColor.rgb * \n"\
		"      pow(max(dot(halfDir, nd), 0.0), hb_shine);\n"\
		"  }\n";
	}
	else
	{
		frag << "  " << plevel << " vec4 color = base;\n";
	}
	
	if (!m_isLit)
	{
		//frag << "  color *= vertColor;\n";
	}
	
	
	frag << "  gl_FragColor = color;\n";
	frag << "}\n";
	
	std::string vertstr = vert.str();
	std::string fragstr = frag.str();
	
	OSG_INFO << "HogBox ShaderGen Vertex shader:\n" << vertstr << std::endl;
	OSG_INFO << "HogBox ShaderGen Fragment shader:\n" << fragstr << std::endl;
	
	this->AddShader(new osg::Shader(osg::Shader::VERTEX, vertstr));
	this->AddShader(new osg::Shader(osg::Shader::FRAGMENT, fragstr));
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
			OSG_NOTICE << "HogBoxMaterial NOTICE: Material '" << this->getName() << "' uses FeatureLevel '" << m_featureLevel->getName() << "' which is not supported," << std::endl
			<< "                                                                               Fallingback onto Material '" << p_fallbackMaterial->getName() << "'." << std::endl;
			return p_fallbackMaterial->GetFunctionalMaterial();
		}
	}
	//not supported, warn user and return null
	osg::notify(osg::WARN) << "HogBoxMaterial WARN: Material '" << this->getName() << "' uses FeatureLevel '" << m_featureLevel->getName() << "' which is not supported," << std::endl
	<< "                                                                               No fallback is provided so the material will not function as expected if at all." << std::endl;
	return NULL;
}