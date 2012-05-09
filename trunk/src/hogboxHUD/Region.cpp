#include <hogboxHUD/Region.h>

#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <osg/TexMat>
#include <osg/TexEnv>
#include <osg/Depth>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
//#include <hogbox/SystemInfo.h>
#include <hogbox/HogBoxUtils.h>
#include <hogbox/AssetManager.h>
#include <hogboxHUD/Hud.h>
//#include <hogbox/NPOTResizeCallback.h>

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
    "uniform mediump vec4 _color;\n"
	"varying mediump vec2 texCoord0;\n"
	"void main(void) {\n" 
    "  mediump vec4 texColor = texture2D(diffuseTexture, texCoord0);\n" 
    "  gl_FragColor = texColor;//vec4(texColor.r*_color.r,texColor.g*_color.g,texColor.b*_color.b, texColor.a*_color.a);\n"
    "  //gl_FragColor.a = gl_FragColor.a*_color.a;\n"
	"}\n" 
};

static const char* coloredVertSource = { 
	SHADER_COMPAT 
	"attribute vec4 osg_Vertex;\n"
	"uniform mat4 osg_ModelViewProjectionMatrix;\n"
	"void main(void) {\n" 
	"  gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n" 
	"}\n" 
}; 

static const char* coloredFragSource = { 
	SHADER_COMPAT 
    "uniform mediump vec4 _color;\n"
	"void main(void) {\n" 
    "  gl_FragColor = _color;\n"
	"}\n" 
};

Region::Region(RegionStyle* args) 
    : AnimatedTransformQuad(args),//give default style, can be overriden by create
    _isProcedural(false),
    _assestLoaded(false),
    //by default inherit all parent transforms
    _transformInheritMask(INHERIT_ALL_TRANSFORMS),
    _visible(true),
    _pickable(true),
    _microMemoryMode(false)
{
    this->InitEvents();
    this->InitStateSet();
    
	_childMount = new osg::MatrixTransform();
    
	//attach the child regions to rotate, so children
	//are affected by this regions translate and rotate
	//transforms directly, but scale is applied indepenatly
	//to allow for different resize modes etc
	_rotate->addChild(_childMount.get());
    
    ApplyNodeMask();
    
	//no parent by default
	p_parent=NULL;
    
	_hovering = false;
    
    static int uniqueRegionID = 0;
    std::stringstream nameStream;
    nameStream << "region_" << uniqueRegionID;
    uniqueRegionID++;
    this->setName(nameStream.str());
}

Region::Region(osg::Vec2 corner, osg::Vec2 size, RegionStyle* style) 
    : AnimatedTransformQuad(size, style),
    _isProcedural(false),
    _assestLoaded(false),
    //by default inherit all parent transforms
    _transformInheritMask(INHERIT_ALL_TRANSFORMS),
    _visible(true),
    _pickable(true),
    _microMemoryMode(false)
{
    
    this->InitEvents();
    this->InitStateSet();
    
	_childMount = new osg::MatrixTransform();
	 
	//attach the child regions to rotate, so children
	//are affected by this regions translate and rotate
	//transforms directly, but scale is applied indepenatly
	//to allow for different resize modes etc
	_rotate->addChild(_childMount.get());
    
    ApplyNodeMask();
    
	//no parent by default
	p_parent=NULL;
    
	_hovering = false;
	
	//set default animation values
	_animateColor->SetValue(this->GetColor());
	_animateAlpha->SetValue(this->GetAlpha());
    
    static int uniqueRegionID = 0;
    std::stringstream nameStream;
    nameStream << "region_" << uniqueRegionID;
    uniqueRegionID++;
    this->setName(nameStream.str());
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
Region::Region(const Region& region,const osg::CopyOp& copyop)
    : AnimatedTransformQuad(region, copyop),
    _visible(region._visible)
{
}

Region::~Region(void)
{
	//release the callback events
	_onMouseDownEvent = NULL;
	_onMouseUpEvent = NULL;
	_onMouseMoveEvent = NULL;
	_onMouseDragEvent = NULL;
	_onDoubleClickEvent = NULL;
	_onMouseEnterEvent = NULL;
	_onMouseLeaveEvent = NULL;
    
	OSG_INFO << "    Deallocating " << this->className() << ": named '" << this->getName() << "'." << std::endl;
	for(unsigned int i=0; i<_children.size(); i++)
	{
		_childMount->removeChild(_children[i]->GetRegion());
		_children[i]=NULL;
	}
	_childMount = NULL;
	_children.clear();
	if(_quadGeode.valid()){
		ClearHudGeodes(_quadGeode.get());
		_quadGeode = NULL;
	}
    
	//_material = NULL;
}

//
//Init Callback Events
//
void Region::InitEvents()
{
    //Create our callbacks
    //mouse events
    _onMouseDownEvent = new HudCallbackEvent(this, "OnMouseDown");
    _onMouseUpEvent = new HudCallbackEvent(this, "OnMouseUp");
    _onMouseMoveEvent = new HudCallbackEvent(this, "OnMouseMove");
    _onMouseDragEvent = new HudCallbackEvent(this, "OnMouseDrag");
    _onDoubleClickEvent = new HudCallbackEvent(this, "OnDoubleClick");
    _onMouseEnterEvent = new HudCallbackEvent(this, "OnMouseEnter");
    _onMouseLeaveEvent = new HudCallbackEvent(this, "OnMouseLeave");
                       
    //keyboard events
    _onKeyDownEvent = new HudCallbackEvent(this, "OnKeyDown");
    _onKeyUpEvent = new HudCallbackEvent(this, "OnKeyUp");
}

//
//Init Material/stateset stuff
//
void Region::InitStateSet()
{
/*	_stateset = new osg::StateSet(); 
	_stateset->setDataVariance(osg::Object::DYNAMIC);
    //_material = new osg::Material();
	//_material->setColorMode(osg::Material::OFF);
	//_material->setDataVariance(osg::Object::DYNAMIC);
    
    //add the color uniform
    //_colorUniform = new osg::Uniform("_color", osg::Vec4(0.0f,1.0f,1.0f,1.0f));
    //_stateset->addUniform(_colorUniform.get());
    
#ifdef TARGET_OS_IPHONE
    _stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
#endif
    
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    
    _stateset->setAttributeAndModes(_material, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    
#else
	osg::Program* program = new osg::Program; 
	program->setName("hudColorShader"); 
	program->addShader(new osg::Shader(osg::Shader::VERTEX, coloredVertSource)); 
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, coloredFragSource)); 
	_stateset->setAttributeAndModes(program, osg::StateAttribute::ON); 	
    
    _shaderMode = COLOR_SHADER;
    
#endif
	//_root->setStateSet(_stateset.get()); 
    _quadGeode->setStateSet(_stateset.get()); 
    
    
	SetColor(osg::Vec3(0.0f, 1.0f, 1.0f));
	SetAlpha(1.0f);
	EnableAlpha(false);*/
}

//
//general update, updates and syncs our animations etc
//normally called by an attached updateCallback
//
void Region::UpdateAnimation(float simTime)
{
    //use global hud timepassed
	float timePassed = Hud::Inst()->GetTimePassed();
	_prevTick = simTime;
    
    AnimatedTransformQuad::UpdateAnimation(timePassed);
    
	if(!_animationDisabled)
	{

	}
}

//
// Create the region of size, in postion using a loaded model
//
bool Region::Create(osg::Vec2 corner, osg::Vec2 size, RegionStyle* style)
{
	SetMicroMemoryMode(_microMemoryMode);

	//Set the regions position, size and rotation
	SetPosition(corner);
    
    //if(folderName.empty()){
    SetSize(size);
	
	if(!_microMemoryMode)
	{
		//load the assests that represent this region
		//i.e. geometry and textures and apply to the region node
		//for rendering
        if(style){
            _args = style;
        }    
        this->LoadAssest(this->ArgsAsRegionStyle());
	}else{
		//in micro mode hud regions defult to not visible
		SetVisible(false);
	}

	return true;
}

//
//Convenience method to create a RegionStyle with provided args
//then pass to base Create
//
bool Region::CreateWithAsset(osg::Vec2 corner, osg::Vec2 size, const std::string& asset)
{
    RegionStyle* asRegionStyle = this->ArgsAsRegionStyle();
    if(!asRegionStyle){
        _args = this->allocateStyleType();
        asRegionStyle = this->ArgsAsRegionStyle();
    }
    asRegionStyle->_assets = asset;
    return this->Create(corner, size, this->ArgsAsRegionStyle());
}

//
// Returns the root transform matrix
//
osg::MatrixTransform* Region::GetRegion()
{
	return this;
}

//
// 0 not used, 1 used by this, 2 used by child
//
int Region::HandleInputEvent(HudInputEvent& hudEvent)
{
	int ret = 0;
	
    //prevent input while animating or unpickable
    if(IsAnimating() || !_pickable){return 0;}
    
	//check for basic event and inform our callbacks if detected
	OSG_DEBUG_FP << "hogboxHUD Region: Input Event received by region '" << this->getName() << "'." << std::endl;
	
	//handle the event type
    switch(hudEvent.GetEventType())
    {
            //key is pressed down
		case(ON_KEY_DOWN):
        {
			OSG_DEBUG_FP << "		KEY DOWN" << std::endl;
			_onKeyDownEvent->Trigger(hudEvent);
			break;
        }
			
            //key released
		case(ON_KEY_UP):
        {
			OSG_DEBUG_FP << "		KEY UP" << std::endl;
			_onKeyUpEvent->Trigger(hudEvent);
			break;
		}
			
            //mouse moving
		case(ON_MOUSE_MOVE):
        {
			//trigger our onMouseDown event
			OSG_DEBUG_FP << "		MOUSE MOVE" << std::endl;
			_onMouseMoveEvent->Trigger(hudEvent);
			break;
		}
			
            //mouse drag (moving with button held)
		case(ON_MOUSE_DRAG):
        {
			//trigger our onMouseDrag event
			OSG_DEBUG_FP << "		MOUSE DRAG" << std::endl;
            _onMouseDragEvent->Trigger(hudEvent);
			break;
        } 
			
            //mouse down
		case(ON_MOUSE_DOWN):
		{
			//trigger our onMouseDown event
			OSG_DEBUG_FP << "		MOUSE DOWN" << std::endl;
			_onMouseDownEvent->Trigger(hudEvent);
			break;
		}
			
            //mouse up
		case(ON_MOUSE_UP):
        {
			//trigger our onMouseUp event
			OSG_DEBUG_FP << "		MOUSE UP" << std::endl;
			//on mouse up check for multi touch and tap count to mimic double click behavior
            osgGA::GUIEventAdapter* ea = hudEvent.GetInputState();
            if(!ea){return 0;}
            
            if (ea->isMultiTouchEvent()) 
            {
                //get touch data
                osgGA::GUIEventAdapter::TouchData* data = ea->getTouchData();
                
                if(data->getNumTouchPoints() == 1){
                    //check tap count
                    if(data->get(0).tapCount == 2){
                        //change the event type to a double click
                        hudEvent.SetEventType(ON_DOUBLE_CLICK);
                        osg::notify(osg::DEBUG_FP) << "		DOUBLE CLICK, FROM MULTI TOUCH" << std::endl;
                        _onDoubleClickEvent->Trigger(hudEvent);
                        break;
                    }
                }
            }
            _onMouseUpEvent->Trigger(hudEvent);
			break;
        } 
			
            //double click do down and up
		case(ON_DOUBLE_CLICK):
        {
			OSG_DEBUG_FP << "		DOUBLE CLICK" << std::endl;
			_onDoubleClickEvent->Trigger(hudEvent);
			break;
        } 
			
            //mouse enter
		case(ON_MOUSE_ENTER):
        {
			//trigger our onMouseUp event
			OSG_DEBUG_FP << "		MOUSE ENTER" << std::endl;
			_onMouseEnterEvent->Trigger(hudEvent);
			break;
        } 
			
            //mouse leave
		case(ON_MOUSE_LEAVE):
        {
			//trigger our onMouseUp event
			OSG_DEBUG_FP << "		MOUSE LEAVE" << std::endl;
			_onMouseLeaveEvent->Trigger(hudEvent);
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
void Region::AddChild(Region* region)
{
	_childMount->addChild(region->GetRegion());
	_children.push_back(region); 
    
	region->SetParent(this); 
}


//
// Checks if the name equal one of or children
//
bool Region::IsChild(const std::string& uniqueID)
{
	for(unsigned int i=0; i<_children.size(); i++)
	{
		if(uniqueID.compare( _children[i]->getName()) == 0)
		{
			return true;
		}else{
			if(_children[i]->IsChild(uniqueID))
			{return true;}
		}
	}
	return false;
}

//
//remove child
//
bool Region::RemoveChild(Region* region)
{
    if(!region){return false;}
    int deleteIndex = -1;
	for(unsigned int i=0; i<_children.size(); i++){
        if(_children[i]->getName() == region->getName()){
            deleteIndex = i;
            break;
        }
    }
    if(deleteIndex != -1){
        return this->RemoveChild(deleteIndex,1);
    }
    return false;
}

bool Region::RemoveChild(unsigned int pos, unsigned int numChildrenToRemove)
{
	if(pos >= _children.size()){return false;}
	RegionList::iterator first = _children.begin();
	Region* region = (*first+pos);
	_childMount->removeChild(region->GetRegion());
	_children.erase(first+pos);
    return true;
}

void Region::RemoveAllChildren()
{
    unsigned int num = _children.size();
    for(unsigned int i=0; i<num; i++){
        this->RemoveChild(0, 1);
    }
}

//
// Pass the event on to all the children regions of this region
// example, a diloag region passes to it button to animate the button
// returns true if used by one of the children
//
bool Region::HandleChildEvents(HudInputEvent& hudEvent)
{
	bool l_ret=false;
	//loop through all the children
	for(unsigned int i=0; i<_children.size(); i++)
	{
		if( _children[i]->HandleInputEvent(hudEvent) != 0)
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
bool Region::LoadAssest(RegionStyle* style)
{
	if(_assestLoaded){return true;}
    
    if(!style){return false;}
    
	//if it has no name, then assets aren't needed
	if(style->_assets.empty())
	{return true;}
    
    std::string assetName = style->_assets;
    
    //if we have any kind of asset name we build the quad
    if(!assetName.empty()){
        
        this->buildQuad(_size, style);
    }
        
    
    //load background texture
    if(assetName != "Quad")//use Quad as special name to have just solid colored quad with no texture
    {
        //now try to load a base texture
        std::string baseTextureFile = assetName+".png";
        {
            _baseTexture = hogbox::AssetManager::Inst()->GetOrLoadTex2D(baseTextureFile);
            if(_baseTexture.get())
            {
                //apply the base as default
                this->ApplyTexture(_baseTexture.get());
                
                //see if we want to size by texture dimensions
                if(style->_sizeByImage){
                    osg::Vec2 imageSize = osg::Vec2(_baseTexture->getImage()->s(),_baseTexture->getImage()->t());
                    this->SetSize(imageSize);
                }
            }
        }
        
        //rollover
        std::string rollOverTextureFile = assetName+"_over.png";
        //if(rollOverTextureFile.empty()){rollOverTextureFile = folderName+"/rollover.png";}
        //if(osgDB::fileExists(rollOverTextureFile) )
        _rollOverTexture = NULL;//OsgModelCache::Inst()->getOrLoadTex2D(rollOverTextureFile);
        
        //if in micro memory mode set textures to unref image data
        if(_microMemoryMode){
            if(_baseTexture.get()){_baseTexture->setUnRefImageDataAfterApply(true);}
            if(_rollOverTexture.get()){_rollOverTexture->setUnRefImageDataAfterApply(true);}
        }
    }
    
	//make the region identifiable by the picker
	//by setting the regions geometry names to the
	//unique ID
	setName(this->getName());
    _args = style;
    _assetFolder = style->_assets;
	_assestLoaded = true;
    
	return true;
}

//
//Unload assest, deleting bae/rollover textures and any children
//of _region
//
bool Region::UnLoadAssests()
{
	if(!_assestLoaded){return true;}
    
	//remove children of _region
	//_region = NULL;
    
	if(_baseTexture.get()){_stateset->removeAssociatedTextureModes(0,_baseTexture.get());}
	_baseTexture = NULL;
	
	if(_rollOverTexture.get()){_stateset->removeAssociatedTextureModes(0,_rollOverTexture.get());}
	_rollOverTexture = NULL;
    
	_assestLoaded = false;
    
	return true;
}

//
//Set the id and rename any asset geodes
//
void Region::setName(const std::string& name)
{
	if(_quadGeode.get()){_quadGeode->setName(name);}
	osg::Object::setName(name);
	MakeHudGeodes(_quadGeode.get(), new RegionWrapper(this));
}

const bool& Region::IsVisible() const
{
	return _visible;
}

void Region::SetVisible(const bool& visible)
{
	//set hud to invisable as default
	if(visible)
	{
		//load assests if micoMemory mode
		//if(_microMemoryMode){this->LoadAssest(_assetFolder);}
        SetPickable(true);
		//_root->setNodeMask(0xFFFFFFFF);
        _prevTick = 0.0f; //reset prev tick
	}else{
		if(_microMemoryMode){this->UnLoadAssests();}
        SetPickable(false);
		//_root->setNodeMask(0x0);
        _prevTick = 0.0f; //reset prev tick
	}
    _visible=visible;
    ApplyNodeMask();
    
    //loop through all the children
	for(unsigned int i=0; i<_children.size(); i++)
	{
        //_p_children[i]->SetVisible(visible);
	}
}

//pickable by hudinput
const bool& Region::IsPickable() const
{
    return _pickable;
}
void Region::SetPickable(const bool& pickable)
{
    _pickable = pickable;
    ApplyNodeMask();
    //loop through all the children
	for(unsigned int i=0; i<_children.size(); i++)
	{
        //  _p_children[i]->SetPickable(pickable);
	}
}

//
//compute and apply the current node mask for the region geode
void Region::ApplyNodeMask()
{
    unsigned int nodeMask = 0x0;
    if(_visible){
        nodeMask |= hogbox::MAIN_CAMERA_CULL;
    }
    if(_pickable){
        nodeMask |= hogbox::PICK_MESH;
    }
    if(_quadGeode.get())
    {
        _quadGeode->setNodeMask(nodeMask);
        this->setNodeMask(nodeMask);
    }
}

//
//set the texture used by default
//
void Region::SetBaseTexture(osg::Texture2D* texture)
{
	_baseTexture = NULL;
	_baseTexture = texture;
}
//
//Set the texture used as rollover
//
void Region::SetRolloverTexture(osg::Texture2D* texture)
{
	_baseTexture = NULL;
	_baseTexture = texture;
}
void Region::ApplyBaseTexture()
{
	this->ApplyTexture(this->_baseTexture);
}

void Region::ApplyRollOverTexture()
{
	this->ApplyTexture(this->_rollOverTexture);
}

//
//override setAlpha to set alpha of children
//
void Region::SetAlpha(const float& alpha)
{
    hogbox::TransformQuad::SetAlpha(alpha);
    
    for(unsigned int i=0; i<_children.size(); i++){
        Region* asRegion = dynamic_cast<Region*>(_children[i].get());
        if(asRegion){
            asRegion->SetAlpha(alpha);
        }
    }
}

//
//convert corrds into the regions local system with corner at the origin
//
osg::Vec2 Region::GetRegionSpaceCoords(osg::Vec2 spCoords)
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
osg::Vec2 Region::GetAbsoluteCorner()
{
	osg::Vec2 ret = _corner;
	if(p_parent != NULL)
	{
		//now rotate our corner by the parents rotation
		float rot = osg::DegreesToRadians(p_parent->GetRotation());
		osg::Vec2 oldCoord = ret;
		ret.x() = cos(rot)*oldCoord.x() - sin(rot)*oldCoord.y();
		ret.y() = sin(rot)*oldCoord.x() + cos(rot)*oldCoord.y();
		ret+=p_parent->GetAbsoluteCorner();
	}
	return ret;
}

//recurse through parents and find the regions absolute rotation
float Region::GetAbsoluteRotation()
{
	float ret = _rotation;
	if(p_parent != NULL)
	{
		//now rotate our corner by the parents rotation
		ret+=p_parent->GetAbsoluteRotation();
	}
	return ret;
}

//
//add an updatecallback, this is done by creating a new node
//attaching the update callback to it, then attaching the new node
//to the root for updating
//
bool Region::AddUpdateCallback(osg::NodeCallback* callback)
{
	//create the new node to handle the callback
	osg::Node* callbackNode = NULL;
	callbackNode = new osg::Node();
	if(callbackNode)
	{
		callbackNode->setUpdateCallback(callback);
		this->addChild(callbackNode);
	}else{
		return false;
	}
	return true;	
}

void Region::SetChildrenList(const RegionList& list)
{
	for(unsigned int i=0; i<list.size(); i++)
	{
		this->AddChild(list[i]);
	}
}

//
//Get/Set use of microMemory mode
//
const bool& Region::GetMicroMemoryMode()const
{
	return _microMemoryMode; 
}

void Region::SetMicroMemoryMode(const bool& on)
{
	_microMemoryMode = on;
	if(_microMemoryMode){
		//if we already have some textures set them to unref image data
		if(_baseTexture.get()){_baseTexture->setUnRefImageDataAfterApply(true);}
		if(_rollOverTexture.get()){_rollOverTexture->setUnRefImageDataAfterApply(true);}
	}else{
		if(_baseTexture.get()){_baseTexture->setUnRefImageDataAfterApply(false);}
		if(_rollOverTexture.get()){_rollOverTexture->setUnRefImageDataAfterApply(false);}
	}
}

//
//Funcs to register event callbacks

//mouse
void Region::AddOnMouseDownCallbackReceiver(HudEventCallback* callback)
{
	_onMouseDownEvent->AddCallbackReceiver(callback);
}

void Region::AddOnMouseUpCallbackReceiver(HudEventCallback* callback)
{
	_onMouseUpEvent->AddCallbackReceiver(callback);
}

void Region::AddOnMouseMoveCallbackReceiver(HudEventCallback* callback)
{
	_onMouseMoveEvent->AddCallbackReceiver(callback);
}

void Region::AddOnMouseDragCallbackReceiver(HudEventCallback* callback)
{
	_onMouseDragEvent->AddCallbackReceiver(callback);
}

void Region::AddOnDoubleClickCallbackReceiver(HudEventCallback* callback)
{
	_onDoubleClickEvent->AddCallbackReceiver(callback);
}

void Region::AddOnMouseEnterCallbackReceiver(HudEventCallback* callback)
{
	_onMouseEnterEvent->AddCallbackReceiver(callback);
}

void Region::AddOnMouseLeaveCallbackReceiver(HudEventCallback* callback)
{
	_onMouseLeaveEvent->AddCallbackReceiver(callback);
}

//keyboard
void Region::AddOnKeyDownCallbackReceiver(HudEventCallback* callback)
{
	_onKeyDownEvent->AddCallbackReceiver(callback);
}

void Region::AddOnKeyUpCallbackReceiver(HudEventCallback* callback)
{
	_onKeyUpEvent->AddCallbackReceiver(callback);
}


//helper func to rename geodes and attach region as user data
void hogboxHUD::MakeHudGeodes(osg::Node* node, osg::ref_ptr<RegionWrapper> region)
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
