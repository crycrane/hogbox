#include <hogboxHUD/HudRegion.h>

#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <osg/TexMat>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <hogbox/HogBoxUtils.h>
#include <hogbox/NPOTResizeCallback.h>

using namespace hogboxHUD;

#ifndef WIN32
	#define SHADER_COMPAT \ 
	"#ifndef GL_ES\n" \
	"#if (__VERSION__ <= 110)\n" \ 
	"#define lowp\n" \ 
	"#define mediump\n" \ 
	"#define highp\n" \ 
	"#endif\n" \ 
	"#endif\n" 
#else
	#define SHADER_COMPAT ""
#endif

static const char* texturedVertSource = { 
	SHADER_COMPAT 
	"attribute vec4 osg_Vertex;\n"
	"attribute vec4 osg_MultiTexCoord0;\n"
	"uniform mat4 osg_ModelViewProjectionMatrix;\n"
	"varying mediump vec2 texCoord0;\n"
	"void main(void) {\n" 
	"  gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n" 
	"  texCoord0 = osg_MultiTexCoord0.xy;\n"
	"}\n" 
}; 

static const char* texturedFragSource = { 
	SHADER_COMPAT 
	"uniform sampler2D diffuseTexture;\n"
	"varying mediump vec2 texCoord0;\n"
	"void main(void) {\n" 
	"  gl_FragColor = texture2D(diffuseTexture, texCoord0);\n" 
	"}\n" 
};

HudRegion::HudRegion(bool isProcedural) 
	: osg::Object(),
	m_isProcedural(isProcedural),
	m_corner(osg::Vec2(0.0f,0.0f)),
	m_size(osg::Vec2(1.0f, 1.0f)),
	m_rotation(0.0f),
	//by default inherit all parent transforms
	m_transformInheritMask(INHERIT_ALL_TRANSFORMS),
	m_visible(true),
	m_depth(0.0f),
	//animation
	m_isRotating(false),
	m_isTranslating(false),
	m_isSizing(false),
	m_isColoring(false),
	m_isFading(false),
	m_animateRotate(new hogbox::AnimateFloat()),
	m_animateSize(new hogbox::AnimateVec2()),
	m_animatePosition(new hogbox::AnimateVec2()),
	m_animateColor(new hogbox::AnimateVec3()),
	m_animateAlpha(new hogbox::AnimateFloat()),
	m_prevTick(0.0f),
	m_animationDisabled(false),
	m_microMemoryMode(false),
	m_assestLoaded(false),
	//Create our callbacks
	//mouse events
	m_onMouseDownEvent(new CallbackEvent(this, "OnMouseDown")),
	m_onMouseUpEvent(new CallbackEvent(this, "OnMouseUp")),
	m_onMouseMoveEvent(new CallbackEvent(this, "OnMouseMove")),
	m_onMouseDragEvent(new CallbackEvent(this, "OnMouseDrag")),
	m_onDoubleClickEvent(new CallbackEvent(this, "OnDoubleClick")),
	m_onMouseEnterEvent(new CallbackEvent(this, "OnMouseEnter")),
	m_onMouseLeaveEvent(new CallbackEvent(this, "OnMouseLeave")),
	//keyboard events
	m_onKeyDownEvent(new CallbackEvent(this, "OnKeyDown")),
	m_onKeyUpEvent(new CallbackEvent(this, "OnKeyUp"))
{

	m_root = new osg::MatrixTransform(osg::Matrix::identity());
	m_translate = new osg::MatrixTransform();
	m_rotate = new osg::MatrixTransform();
	m_scale = new osg::MatrixTransform();
	m_region = new osg::Group();
	m_childMount = new osg::MatrixTransform();
	
	//create and attach our default updatecallback
	m_updateCallback = new HudRegionUpdateCallback(this);
	this->AddUpdateCallback(m_updateCallback.get());

	//build the transform hierachy	
	m_root->addChild(m_translate);
	m_translate->addChild(m_rotate.get());
	m_rotate->addChild(m_scale.get());
	//attach region node for rendering
	m_scale->addChild(m_region.get());	

	//attach the child regions to rotate, so children
	//are affected by this regions translate and rotate
	//transforms directly, but scale is applied indepenatly
	//to allow for different resize modes etc
	m_rotate->addChild(m_childMount.get());
	
	m_stateset = new osg::StateSet(); 
	m_stateset->setDataVariance(osg::Object::DYNAMIC);
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
	m_stateset->setMode(GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
    //stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
#else
	osg::Program* program = new osg::Program; 
	program->setName("textureShader"); 
	program->addShader(new osg::Shader(osg::Shader::VERTEX, texturedVertSource)); 
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, texturedFragSource)); 
	m_stateset->setAttributeAndModes(program, osg::StateAttribute::ON); 	
#endif
	m_region->setStateSet(m_stateset.get()); 

	m_material = new osg::Material();
	m_material->setColorMode(osg::Material::OFF);
	m_material->setDataVariance(osg::Object::DYNAMIC);
	m_stateset->setAttributeAndModes(m_material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	SetColor(osg::Vec3(1.0f, 1.0f, 1.0f));
	SetAlpha(1.0f);
	EnableAlpha(false);

	//no parent by default
	m_p_parent=NULL;

	m_hovering = false;
	
	//set default animation values
	m_animateRotate->SetValue(this->GetRotation());
	m_animateSize->SetValue(this->GetSize());
	m_animatePosition->SetValue(this->GetPosition());
	m_animateColor->SetValue(this->GetColor());
	m_animateAlpha->SetValue(this->GetAlpha());
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
HudRegion::HudRegion(const HudRegion& region,const osg::CopyOp& copyop)
	: osg::Object(region, copyop),
	m_corner(region.m_corner),
	m_size(region.m_size),
	m_rotation(region.m_rotation),
	m_visible(region.m_visible),
	m_depth(region.m_depth),
	m_color(region.m_color),
	m_alpha(region.m_alpha),
	m_alphaEnabled(region.m_alphaEnabled)
{
}

HudRegion::~HudRegion(void)
{
	//release the callback events
	m_onMouseDownEvent = NULL;
	m_onMouseUpEvent = NULL;
	m_onMouseMoveEvent = NULL;
	m_onMouseDragEvent = NULL;
	m_onDoubleClickEvent = NULL;
	m_onMouseEnterEvent = NULL;
	m_onMouseLeaveEvent = NULL;

	OSG_NOTICE << "    Deallocating HudRegion: Name '" << this->getName() << "'." << std::endl;
	for(unsigned int i=0; i<m_p_children.size(); i++)
	{
		m_childMount->removeChild(m_p_children[i]->GetRegion());
		m_p_children[i]=NULL;
	}
	m_childMount = NULL;
	m_p_children.clear();
	if(m_region.valid()){
		ClearHudGeodes(m_region.get());
		m_region = NULL;
	}
	m_root = NULL;
	m_translate = NULL;
	m_rotate = NULL;
	m_scale = NULL;

	m_material = NULL;
}

//
//general update, updates and syncs our animations etc
//normally called by an attached updateCallback
//
void HudRegion::Update(float simTime)
{
	float timePassed = simTime - m_prevTick;
	m_prevTick = simTime;
	
	if(!m_animationDisabled)
	{
		//update all our smooth values
		if(m_isRotating = m_animateRotate->Update(timePassed))
		{this->SetRotation(m_animateRotate->GetValue());}
		
		if(m_isTranslating = m_animatePosition->Update(timePassed))
		{this->SetPosition(m_animatePosition->GetValue());}
		
		if(m_isSizing = m_animateSize->Update(timePassed))
		{this->SetSize(m_animateSize->GetValue());}
		
		if(m_isColoring = m_animateColor->Update(timePassed))
		{this->SetColor(m_animateColor->GetValue());}
		
		if(m_isFading = m_animateAlpha->Update(timePassed))
		{this->SetAlpha(m_animateAlpha->GetValue());}
	}
}

//
// Create the region of size, in postion using a loaded model
//
bool HudRegion::Create(osg::Vec2 corner, osg::Vec2 size, const std::string& folderName, bool microMemoryMode)
{
	m_assetFolder = folderName;
	SetMicroMemoryMode(microMemoryMode);

	if(!m_microMemoryMode)
	{
		//load the assests that represent this region
		//i.e. geometry and textures and apply to the region node
		//for rendering
		this->LoadAssest(m_assetFolder);
	}else{
		//in micro mode hud regions defult to not visible
		SetVisible(false);
	}

	//Set the regions position, size and rotation
	SetPosition(corner);SetSize(size);
	
	return true;
}

//
// Returns the root transform matrix
//
osg::MatrixTransform* HudRegion::GetRegion()
{
	return m_root.get();
}

//
// 0 not used, 1 used by this, 2 used by child
//
int HudRegion::HandleInputEvent(HudInputEvent& hudEvent)
{
	int ret = 0;
	
	//check for basic event and inform our callbacks if detected
	osg::notify(osg::DEBUG_FP) << "hogboxHUD HudRegion: Input Event received by region '" << this->getName() << "'." << std::endl;
	
	//handle the event type
    switch(hudEvent.GetEventType())
    {
		//key is pressed down
		case(ON_KEY_DOWN):
        {
			osg::notify(osg::DEBUG_FP) << "		KEY DOWN" << std::endl;
			m_onKeyDownEvent->TriggerEvent(hudEvent);
			break;
        }
			
		//key released
		case(ON_KEY_UP):
        {
			osg::notify(osg::DEBUG_FP) << "		KEY UP" << std::endl;
			m_onKeyUpEvent->TriggerEvent(hudEvent);
			break;
		}
			
		//mouse moving
		case(ON_MOUSE_MOVE):
        {
			//trigger our onMouseDown event
			osg::notify(osg::DEBUG_FP) << "		MOUSE MOVE" << std::endl;
			m_onMouseMoveEvent->TriggerEvent(hudEvent);
			break;
		}
			
		//mouse drag (moving with button held)
		case(ON_MOUSE_DRAG):
        {
			//trigger our onMouseDrag event
			osg::notify(osg::DEBUG_FP) << "		MOUSE DRAG" << std::endl;
			m_onMouseDragEvent->TriggerEvent(hudEvent);
			break;
        } 
			
		//mouse down
		case(ON_MOUSE_DOWN):
		{
			//trigger our onMouseDown event
			osg::notify(osg::DEBUG_FP) << "		MOUSE DOWN" << std::endl;
			m_onMouseDownEvent->TriggerEvent(hudEvent);
			break;
		}
			
		//mouse up
		case(ON_MOUSE_UP):
        {
			//trigger our onMouseUp event
			osg::notify(osg::DEBUG_FP) << "		MOUSE UP" << std::endl;
			m_onMouseUpEvent->TriggerEvent(hudEvent);
			break;
        } 
			
		//double click do down and up
		case(ON_DOUBLE_CLICK):
        {
			osg::notify(osg::DEBUG_FP) << "		DOUBLE CLICK" << std::endl;
			m_onDoubleClickEvent->TriggerEvent(hudEvent);
			break;
        } 
			
		//mouse enter
		case(ON_MOUSE_ENTER):
        {
			//trigger our onMouseUp event
			osg::notify(osg::DEBUG_FP) << "		MOUSE ENTER" << std::endl;
			m_onMouseEnterEvent->TriggerEvent(hudEvent);
			break;
        } 
			
		//mouse leave
		case(ON_MOUSE_LEAVE):
        {
			//trigger our onMouseUp event
			osg::notify(osg::DEBUG_FP) << "		MOUSE LEAVE" << std::endl;
			m_onMouseEnterEvent->TriggerEvent(hudEvent);
			break;
        } 
		default:break;
	}
	//hudEvent

	return ret;
}

//
//adds a child to this region which will then be transformed relative
// to the parent
//
void HudRegion::AddChild(HudRegion* region)
{
	m_childMount->addChild(region->GetRegion());
	m_p_children.push_back(region); 

	region->SetParent(this); 
}


//
// Checks if the name equal one of or children
//
bool HudRegion::IsChild(const std::string& uniqueID)
{
	for(unsigned int i=0; i<m_p_children.size(); i++)
	{
		if(uniqueID.compare( m_p_children[i]->getName()) == 0)
		{
			return true;
		}else{
			if(m_p_children[i]->IsChild(uniqueID))
			{return true;}
		}
	}
	return false;
}

//
//remove child
//
void HudRegion::RemoveChild(HudRegion* region)
{
	
}

void HudRegion::RemoveChild(unsigned int pos, unsigned int numChildrenToRemove)
{
	if(pos >= m_p_children.size()){return;}
	HudRegionList::iterator first = m_p_children.begin();
	HudRegion* region = (*first+pos);
	m_childMount->removeChild(region->GetRegion());
	m_p_children.erase(first+pos);
}

//
// Pass the event on to all the children regions of this region
// example, a diloag region passes to it button to animate the button
// returns true if used by one of the children
//
bool HudRegion::HandleChildEvents(HudInputEvent& hudEvent)
{
	bool l_ret=false;
	//loop through all the children
	for(unsigned int i=0; i<m_p_children.size(); i++)
	{
		if( m_p_children[i]->HandleInputEvent(hudEvent) != 0)
		{l_ret = true;}
	}
	return l_ret;
}


//
//Load assest takes a folder name containing our assest
//each region type expects to find specificlty named assests in the folder
//Base implementation loads 
//geom.osg, used as render geometry (if not present then a quad is generated with bottom left corner at 0,0)
//base.png, used as the default texture if present
//rollover.png, used for mouse rollovers if present
//
bool HudRegion::LoadAssest(const std::string& folderName)
{
	if(m_assestLoaded){return true;}
	//if it has no name, then no assets are needed
	if(folderName.size() == 0)
	{return true;}

	//try to load the file as an ive file
	osg::Node* geometry = osgDB::readNodeFile(folderName+"/geom.osg");
	if(geometry==NULL)
	{
		//create an xy quad of size 1,1 in its place
		osg::Geometry* geom = osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0f), 
															osg::Vec3(1.0f,0.0f,0.0f),
															osg::Vec3(0.0f, 1.0f, 0.0f));
		geom->setColorArray(NULL);//don't use color array as we set color via gl material
		osg::Geode* geode = new osg::Geode();
		geode->addDrawable(geom);
		m_region->addChild(geode);

	}else{
		//attach the geomtry to the region node
		m_region->addChild(geometry);
	}

	//now try to load a base texture
	std::string baseTextureFile = osgDB::findDataFile( folderName+"/base.png");
	if(baseTextureFile.empty()){baseTextureFile = osgDB::findDataFile( folderName+"/base.jpg");}
	//if(osgDB::fileExists(baseTextureFile) )
	{
		m_baseTexture = hogbox::LoadTexture2D(baseTextureFile);
		if(m_baseTexture.get())
		{
            m_baseTexture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            m_baseTexture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
			m_baseTexture->setUseHardwareMipMapGeneration(false);
			m_baseTexture->setResizeNonPowerOfTwoHint(false);
			
			//apply the base as default
			this->ApplyTexture(m_baseTexture.get());
		}
	}

	//rollover
	std::string rollOverTextureFile = folderName+"/rollover.png";
	if(rollOverTextureFile.empty()){rollOverTextureFile = folderName+"/rollover.png";}
	if(osgDB::fileExists(rollOverTextureFile) )
	{m_rollOverTexture = hogbox::LoadTexture2D(rollOverTextureFile);}

	//if in micro memory mode set textures to unref image data
	if(m_microMemoryMode){
		if(m_baseTexture.get()){m_baseTexture->setUnRefImageDataAfterApply(true);}
		if(m_rollOverTexture.get()){m_rollOverTexture->setUnRefImageDataAfterApply(true);}
	}

	//make the region identifiable by the picker
	//by setting the regions geometry names to the
	//unique ID
	setName(this->getName());

	m_assestLoaded = true;

	return true;
}

//
//Unload assest, deleting bae/rollover textures and any children
//of m_region
//
bool HudRegion::UnLoadAssests()
{
	if(!m_assestLoaded){return true;}

	//remove children of m_region
	m_region->removeChildren(0,m_region->getNumChildren());

	if(m_baseTexture.get()){m_stateset->removeAssociatedTextureModes(0,m_baseTexture.get());}
	m_baseTexture = NULL;
	
	if(m_rollOverTexture.get()){m_stateset->removeAssociatedTextureModes(0,m_rollOverTexture.get());}
	m_rollOverTexture = NULL;

	m_assestLoaded = false;

	return true;
}

//
//Set the id and rename any asset geodes
//
void HudRegion::setName(const std::string& name)
{
	m_region->setName(name); 
	osg::Object::setName(name);
	MakeHudGeodes(m_region.get(), new HudRegionWrapper(this));
}

//positioning

void HudRegion::SetPosition(const osg::Vec2& corner)
{
	m_corner = corner;
	m_translate->setMatrix( osg::Matrix::translate(osg::Vec3(	corner.x(),
																corner.y(), 
																m_depth
															))); 
}

const osg::Vec2& HudRegion::GetPosition() const
{
	return m_corner;
}

//
//get set roation around the z axis in degrees
//
void HudRegion::SetRotation(const float& rotate)
{
	m_rotation = rotate;
	m_rotate->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(m_rotation), osg::Vec3(0.0f,0.0f,1.0f)));
}

const float& HudRegion::GetRotation() const
{
	return m_rotation;
}

void HudRegion::SetSize(const osg::Vec2& size)
{
	//resize this regions geometry
	m_scale->setMatrix( osg::Matrix::scale(osg::Vec3(size.x(), size.y(), 1.0f)));
	
	//resize children using the relative scale difference
	//between old and new size
	float xScaler = size.x()/m_size.x();
	float yScaler = size.y()/m_size.y();

	//then scale the relative position of any children
	for(unsigned int i=0; i<m_p_children.size(); i++)
	{
		//check if the child want to inherit
		if((m_p_children[i]->GetTransformInheritMask() & INHERIT_SIZE))
		{m_p_children[i]->SetSizeFromPercentage(xScaler, yScaler);}

		if((m_p_children[i]->GetTransformInheritMask() & INHERIT_POSITION))
		{m_p_children[i]->SetPositionFromPercentage(xScaler, yScaler);}
	}

	//store new size
	m_size = size;
}

const osg::Vec2& HudRegion::GetSize() const
{
	return m_size;
}

//
//set the size as a percentage/scale factor of the current size 
//i.e. <1 smaller, >1 bigger
//
void HudRegion::SetSizeFromPercentage(float xScaler, float yScaler)
{
	osg::Vec2 newSize;
	newSize.x() = m_size.x()*xScaler;
	newSize.y() = m_size.y()*yScaler;
	this->SetSize(newSize);
}

//
//set the position as a percentage/scale factor of the current position
//i.e. <1 smaller, >1 bigger
//
void HudRegion::SetPositionFromPercentage(float xScaler, float yScaler)
{
	osg::Vec2 newPos;
	newPos.x() = m_corner.x()*xScaler;
	newPos.y() = m_corner.y()*yScaler;
	this->SetPosition(newPos);
}

//
//move to a new layer
//

void HudRegion::SetLayer(const float& depth)
{
	osg::Vec2 pos = GetPosition();
	m_translate->setMatrix( osg::Matrix::translate( osg::Vec3(pos.x(),pos.y(),depth)));
	m_depth = depth;
}

const float& HudRegion::GetLayer() const
{
	return m_depth;
}


const bool& HudRegion::IsVisible() const
{
	return m_visible;
}

void HudRegion::SetVisible(const bool& visible)
{
	//set hud to invisable as default
	if(visible)
	{
		//load assests if micoMemory mode
		if(m_microMemoryMode){this->LoadAssest(m_assetFolder);}
		m_root->setNodeMask(0xFFFFFFFF);
	}else{
		if(m_microMemoryMode){this->UnLoadAssests();}
		m_root->setNodeMask(0x0);
	}
	m_visible=visible;
}

//
//set the texture used by default
//
void HudRegion::SetBaseTexture(osg::Texture* texture)
{
	m_baseTexture = NULL;
	m_baseTexture = texture;
}
//
//Set the texture used as rollover
//
void HudRegion::SetRolloverTexture(osg::Texture* texture)
{
	m_baseTexture = NULL;
	m_baseTexture = texture;
}

//
//Apply the texture to the channel 0/diffuse
//
void HudRegion::ApplyTexture(osg::Texture* tex)
{
	if(!m_stateset.get()){return;}
	
	m_stateset->setTextureAttributeAndModes(0,tex,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	
	//if the textures image includes alpha enable alpha/blending
	if(tex)
	{
		if(tex->getImage(0))
		{
			bool isImageTranslucent = tex->getImage(0)->getPixelFormat()==GL_RGBA || tex->getImage(0)->getPixelFormat()==GL_BGRA;
			this->EnableAlpha(isImageTranslucent);
		}
	}
	
	//NOTE@tom, below isn't needed on platforms supporting glu
	//apply a non power of two rezie callback if required
	osg::ref_ptr<hogbox::NPOTResizeCallback> resizer = new hogbox::NPOTResizeCallback(tex, 0, m_stateset.get());
	//if the texture casts as a rect apply the tex rect scaling to texture coords
	osg::Texture2D* tex2D = dynamic_cast<osg::Texture2D*> (tex); 
	if(tex2D){if(resizer->useAsCallBack()){tex2D->setSubloadCallback(resizer.get());}}
}

void HudRegion::ApplyBaseTexture()
{
	this->ApplyTexture(this->m_baseTexture);
}

void HudRegion::ApplyRollOverTexture()
{
	this->ApplyTexture(this->m_rollOverTexture);
}

//
//set the material color of the region
//
void HudRegion::SetColor(const osg::Vec3& color)
{
	m_color = color;

	osg::Vec4 vec4Color = osg::Vec4(color, m_alpha);

	//set the materials color
	m_material->setAmbient(osg::Material::FRONT_AND_BACK, vec4Color ); 
	m_material->setDiffuse(osg::Material::FRONT_AND_BACK, vec4Color );
	m_material->setSpecular(osg::Material::FRONT_AND_BACK, vec4Color );
	m_material->setShininess(osg::Material::FRONT_AND_BACK, 128);
	m_material->setEmission(osg::Material::FRONT_AND_BACK, vec4Color);
	 

	//call for children
	for(unsigned int i=0; i<m_p_children.size(); i++)
	{
		m_p_children[i]->SetColor(color); 
	}
}

const osg::Vec3& HudRegion::GetColor() const
{
	return m_color;
}


void HudRegion::SetAlpha(const float& alpha)
{
	m_alpha = alpha;

	//set the materials alpha
	m_material->setAlpha(osg::Material::FRONT_AND_BACK, m_alpha);

	//if the alpha is zero and we are in microMemory mode
	//we also full hide the region
	if(m_microMemoryMode){
		if(m_alpha>0.0f){
			SetVisible(true);
		}else{
			SetVisible(false);
		}
	}

	//call for children
	for(unsigned int i=0; i<m_p_children.size(); i++)
	{
		m_p_children[i]->SetAlpha(alpha); 
	}
}

const float& HudRegion::GetAlpha() const
{
	return m_alpha;
}

//
//get set enable alpha
//
void HudRegion::EnableAlpha(const bool& enable)
{
	if(enable)
	{
		osg::BlendEquation* blendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
		m_stateset->setAttributeAndModes(blendEquation, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		osg::BlendFunc* blendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
		m_stateset->setAttributeAndModes(blendFunc, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		//tell to sort the mesh before displaying it
		m_stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		m_stateset->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 
		m_stateset->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	}else{
		m_stateset->setRenderingHint(osg::StateSet::OPAQUE_BIN);
		m_stateset->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	}
	m_alphaEnabled = enable;
}

const bool& HudRegion::IsAlphaEnabled() const
{
	return m_alphaEnabled;
}

void HudRegion::SetStateSet(osg::StateSet* stateSet)
{
	if(m_region.get()!=NULL)
	{
		m_region->setStateSet(stateSet);
	}
}

osg::StateSet* HudRegion::GetStateSet()
{
	if(!m_stateset.get())
	{return NULL;}

	return m_stateset.get();
}




//
//convert corrds into the regions local system with corner at the origin
//
osg::Vec2 HudRegion::GetRegionSpaceCoords(osg::Vec2 spCoords)
{
	//get the vector from corner to spCoord
	osg::Vec2 coordVec = spCoords-GetAbsoluteCorner();
	//rotate the coord vec into local space
	float invRot = -osg::DegreesToRadians(GetAbsoluteRotation());
	osg::Vec2 regionSpaceCoord;
	regionSpaceCoord.x() = cos(invRot)*coordVec.x() - sin(invRot)*coordVec.y();
	regionSpaceCoord.y() = sin(invRot)*coordVec.x() + cos(invRot)*coordVec.y();
	return regionSpaceCoord;
}

//
//recersive through parents to get screen space corner
//
osg::Vec2 HudRegion::GetAbsoluteCorner()
{
	osg::Vec2 ret = m_corner;
	if(m_p_parent != NULL)
	{
		//now rotate our corner by the parents rotation
		float rot = osg::DegreesToRadians(m_p_parent->GetRotation());
		osg::Vec2 oldCoord = ret;
		ret.x() = cos(rot)*oldCoord.x() - sin(rot)*oldCoord.y();
		ret.y() = sin(rot)*oldCoord.x() + cos(rot)*oldCoord.y();
		ret+=m_p_parent->GetAbsoluteCorner();
	}
	return ret;
}

//recurse through parents and find the regions absolute rotation
float HudRegion::GetAbsoluteRotation()
{
	float ret = m_rotation;
	if(m_p_parent != NULL)
	{
		//now rotate our corner by the parents rotation
		ret+=m_p_parent->GetAbsoluteRotation();
	}
	return ret;
}

//
//add an updatecallback, this is done by creating a new node
//attaching the update callback to it, then attaching the new node
//to the root for updating
//
bool HudRegion::AddUpdateCallback(osg::NodeCallback* callback)
{
	//create the new node to handle the callback
	osg::Node* callbackNode = NULL;
	callbackNode = new osg::Node();
	if(callbackNode)
	{
		callbackNode->setUpdateCallback(callback);
		if(m_root){m_root->addChild(callbackNode);}
	}else{
		return false;
	}
	return true;	
}

void HudRegion::SetChildrenList(const HudRegionList& list)
{
	for(unsigned int i=0; i<list.size(); i++)
	{
		this->AddChild(list[i]);
	}
}

//
//Get/Set use of microMemory mode
//
const bool& HudRegion::GetMicroMemoryMode()const
{
	return m_microMemoryMode; 
}

void HudRegion::SetMicroMemoryMode(const bool& on)
{
	m_microMemoryMode = on;
	if(m_microMemoryMode){
		//if we already have some textures set them to unref image data
		if(m_baseTexture.get()){m_baseTexture->setUnRefImageDataAfterApply(true);}
		if(m_rollOverTexture.get()){m_rollOverTexture->setUnRefImageDataAfterApply(true);}
	}else{
		if(m_baseTexture.get()){m_baseTexture->setUnRefImageDataAfterApply(false);}
		if(m_rollOverTexture.get()){m_rollOverTexture->setUnRefImageDataAfterApply(false);}
	}
}

//
//Funcs to register event callbacks

//mouse
void HudRegion::AddOnMouseDownCallbackReceiver(HudEventCallback* callback)
{
	m_onMouseDownEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseUpCallbackReceiver(HudEventCallback* callback)
{
	m_onMouseUpEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseMoveCallbackReceiver(HudEventCallback* callback)
{
	m_onMouseMoveEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseDragCallbackReceiver(HudEventCallback* callback)
{
	m_onMouseDragEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnDoubleClickCallbackReceiver(HudEventCallback* callback)
{
	m_onDoubleClickEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseEnterCallbackReceiver(HudEventCallback* callback)
{
	m_onMouseEnterEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseLeaveCallbackReceiver(HudEventCallback* callback)
{
	m_onMouseLeaveEvent->AddCallbackReceiver(callback);
}

//keyboard
void HudRegion::AddOnKeyDownCallbackReceiver(HudEventCallback* callback)
{
	m_onKeyDownEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnKeyUpCallbackReceiver(HudEventCallback* callback)
{
	m_onKeyUpEvent->AddCallbackReceiver(callback);
}


//helper func to rename geodes and attach region as user data
void hogboxHUD::MakeHudGeodes(osg::Node* node, osg::ref_ptr<HudRegionWrapper> region)
{
	//find geomtry in geodes
	if(dynamic_cast<osg::Geode*> (node))
	{
		//loop all geometry nodes
		osg::Geode* geode = static_cast<osg::Geode*> (node);
		geode->setName(region->GetRegion()->getName()) ;
		geode->setUserData(region);
	}
	
	// Traverse any group node
	if(dynamic_cast<osg::Group*> (node))
	{
		osg::Group* group = static_cast<osg::Group*> (node);
		for(unsigned int i=0; i < group->getNumChildren(); i++)
		{	hogboxHUD::MakeHudGeodes(group->getChild(i), region);}
	}
}

void hogboxHUD::ClearHudGeodes(osg::Node* node)
{
	//find geomtry in geodes
	if(dynamic_cast<osg::Geode*> (node))
	{
		//loop all geometry nodes
		osg::Geode* geode = static_cast<osg::Geode*> (node);;
		geode->setUserData(NULL);
	}
	
	// Traverse any group node
	if(dynamic_cast<osg::Group*> (node))
	{
		osg::Group* group = static_cast<osg::Group*> (node);
		for(unsigned int i=0; i < group->getNumChildren(); i++)
		{	hogboxHUD::ClearHudGeodes(group->getChild(i));}
	}
}
