#include <hogboxHUD/HudRegion.h>

#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <osg/TexMat>
#include <osg/TexEnv>
#include <osg/Depth>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
//#include <hogbox/SystemInfo.h>
#include <hogbox/HogBoxUtils.h>
//#include "OsgModelCache.h"
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

HudRegion::HudRegion(RegionPlane plane, RegionOrigin origin, bool isProcedural) 
    : osg::Object(),
    _plane(plane),
    _rotatePlane(PLANE_XY),
    _origin(origin),
    _isProcedural(isProcedural),
    _corner(osg::Vec2(0.0f,0.0f)),
    _size(osg::Vec2(1.0f, 1.0f)),
    _rotation(0.0f),
    //by default inherit all parent transforms
    _transformInheritMask(INHERIT_ALL_TRANSFORMS),
    _visible(true),
    _pickable(true),
    _depth(0.0f),
    //animation
    _isRotating(false),
    _isTranslating(false),
    _isSizing(false),
    _isColoring(false),
    _isFading(false),
    _animateRotate(new hogbox::AnimateFloat()),
    _animateSize(new hogbox::AnimateVec2()),
    _animatePosition(new hogbox::AnimateVec2()),
    _animateColor(new hogbox::AnimateVec3()),
    _animateAlpha(new hogbox::AnimateFloat()),
    _prevTick(0.0f),
    _animationDisabled(false),
    _microMemoryMode(false),
    _assestLoaded(false),
    //Create our callbacks
    //mouse events
    _onMouseDownEvent(new CallbackEvent(this, "OnMouseDown")),
    _onMouseUpEvent(new CallbackEvent(this, "OnMouseUp")),
    _onMouseMoveEvent(new CallbackEvent(this, "OnMouseMove")),
    _onMouseDragEvent(new CallbackEvent(this, "OnMouseDrag")),
    _onDoubleClickEvent(new CallbackEvent(this, "OnDoubleClick")),
    _onMouseEnterEvent(new CallbackEvent(this, "OnMouseEnter")),
    _onMouseLeaveEvent(new CallbackEvent(this, "OnMouseLeave")),
    //keyboard events
    _onKeyDownEvent(new CallbackEvent(this, "OnKeyDown")),
    _onKeyUpEvent(new CallbackEvent(this, "OnKeyUp"))
{
    
	_root = new osg::MatrixTransform(osg::Matrix::identity());
	_translate = new osg::MatrixTransform();
	_rotate = new osg::MatrixTransform();
	_scale = new osg::MatrixTransform();
	_region = new osg::Geode();
	_childMount = new osg::MatrixTransform();
	
	//create and attach our default updatecallback
	_updateCallback = new HudRegionUpdateCallback(this);
	this->AddUpdateCallback(_updateCallback.get());
    
	//build the transform hierachy	
	_root->addChild(_translate);
	_translate->addChild(_rotate.get());
	_rotate->addChild(_scale.get());
	//attach region node for rendering
	_scale->addChild(_region.get());	
    
	//attach the child regions to rotate, so children
	//are affected by this regions translate and rotate
	//transforms directly, but scale is applied indepenatly
	//to allow for different resize modes etc
	_rotate->addChild(_childMount.get());
	
	_stateset = new osg::StateSet(); 
	_stateset->setDataVariance(osg::Object::DYNAMIC);
    _material = new osg::Material();
	_material->setColorMode(osg::Material::OFF);
	_material->setDataVariance(osg::Object::DYNAMIC);
    
    //add the color uniform
    _colorUniform = new osg::Uniform("_color", osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    _stateset->addUniform(_colorUniform.get());
    
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
    _region->setStateSet(_stateset.get()); 
    
    
	SetColor(osg::Vec3(1.0f, 1.0f, 1.0f));
	SetAlpha(1.0f);
	EnableAlpha(false);
    
    ApplyNodeMask();
    
	//no parent by default
	p_parent=NULL;
    
	_hovering = false;
	
	//set default animation values
	_animateRotate->SetValue(this->GetRotation());
	_animateSize->SetValue(this->GetSize());
	_animatePosition->SetValue(this->GetPosition());
	_animateColor->SetValue(this->GetColor());
	_animateAlpha->SetValue(this->GetAlpha());
    
    static int uniqueRegionID = 0;
    std::stringstream nameStream;
    nameStream << "region_" << uniqueRegionID;
    uniqueRegionID++;
    this->setName(nameStream.str());
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
HudRegion::HudRegion(const HudRegion& region,const osg::CopyOp& copyop)
: osg::Object(region, copyop),
_corner(region._corner),
_size(region._size),
_rotation(region._rotation),
_visible(region._visible),
_depth(region._depth),
_color(region._color),
_alpha(region._alpha),
_alphaEnabled(region._alphaEnabled)
{
}

HudRegion::~HudRegion(void)
{
	//release the callback events
	_onMouseDownEvent = NULL;
	_onMouseUpEvent = NULL;
	_onMouseMoveEvent = NULL;
	_onMouseDragEvent = NULL;
	_onDoubleClickEvent = NULL;
	_onMouseEnterEvent = NULL;
	_onMouseLeaveEvent = NULL;
    
	OSG_FATAL << "    Deallocating " << this->className() << ": named '" << this->getName() << "'." << std::endl;
	for(unsigned int i=0; i<_children.size(); i++)
	{
		_childMount->removeChild(_children[i]->GetRegion());
		_children[i]=NULL;
	}
	_childMount = NULL;
	_children.clear();
	if(_region.valid()){
		ClearHudGeodes(_region.get());
		_region = NULL;
	}
	_root = NULL;
	_translate = NULL;
	_rotate = NULL;
	_scale = NULL;
    
	_material = NULL;
}

//
//general update, updates and syncs our animations etc
//normally called by an attached updateCallback
//
void HudRegion::Update(float simTime)
{
    if(_prevTick==0.0f){_prevTick = simTime;}
	float timePassed = simTime - _prevTick;
	_prevTick = simTime;
    
	if(!_animationDisabled)
	{
		//update all our smooth values
		if(_isRotating = _animateRotate->Update(timePassed))
		{this->SetRotation(_animateRotate->GetValue());}
		
		if(_isTranslating = _animatePosition->Update(timePassed))
		{this->SetPosition(_animatePosition->GetValue());}
		
		if(_isSizing = _animateSize->Update(timePassed))
		{this->SetSize(_animateSize->GetValue());}
		
		if(_isColoring = _animateColor->Update(timePassed))
		{this->SetColor(_animateColor->GetValue());}
		
		if(_isFading = _animateAlpha->Update(timePassed))
		{this->SetAlpha(_animateAlpha->GetValue());}
	}
}

//
// Create the region of size, in postion using a loaded model
//
bool HudRegion::Create(osg::Vec2 corner, osg::Vec2 size, const std::string& folderName, bool microMemoryMode)
{
	_assetFolder = folderName;
	SetMicroMemoryMode(microMemoryMode);
    
	if(!_microMemoryMode)
	{
		//load the assests that represent this region
		//i.e. geometry and textures and apply to the region node
		//for rendering
		this->LoadAssest(_assetFolder);
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
	return _root.get();
}

//
// 0 not used, 1 used by this, 2 used by child
//
int HudRegion::HandleInputEvent(HudInputEvent& hudEvent)
{
	int ret = 0;
	
    //prevent input while animating or unpickable
    if(IsAnimating() || !_pickable){return 0;}
    
	//check for basic event and inform our callbacks if detected
	osg::notify(osg::DEBUG_FP) << "hogboxHUD HudRegion: Input Event received by region '" << this->getName() << "'." << std::endl;
	
	//handle the event type
    switch(hudEvent.GetEventType())
    {
            //key is pressed down
		case(ON_KEY_DOWN):
        {
			osg::notify(osg::DEBUG_FP) << "		KEY DOWN" << std::endl;
			_onKeyDownEvent->TriggerEvent(hudEvent);
			break;
        }
			
            //key released
		case(ON_KEY_UP):
        {
			osg::notify(osg::DEBUG_FP) << "		KEY UP" << std::endl;
			_onKeyUpEvent->TriggerEvent(hudEvent);
			break;
		}
			
            //mouse moving
		case(ON_MOUSE_MOVE):
        {
			//trigger our onMouseDown event
			osg::notify(osg::DEBUG_FP) << "		MOUSE MOVE" << std::endl;
			_onMouseMoveEvent->TriggerEvent(hudEvent);
			break;
		}
			
            //mouse drag (moving with button held)
		case(ON_MOUSE_DRAG):
        {
			//trigger our onMouseDrag event
			osg::notify(osg::DEBUG_FP) << "		MOUSE DRAG" << std::endl;
            _onMouseDragEvent->TriggerEvent(hudEvent);
			break;
        } 
			
            //mouse down
		case(ON_MOUSE_DOWN):
		{
			//trigger our onMouseDown event
			osg::notify(osg::DEBUG_FP) << "		MOUSE DOWN" << std::endl;
			_onMouseDownEvent->TriggerEvent(hudEvent);
			break;
		}
			
            //mouse up
		case(ON_MOUSE_UP):
        {
			//trigger our onMouseUp event
			osg::notify(osg::DEBUG_FP) << "		MOUSE UP" << std::endl;
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
                        _onDoubleClickEvent->TriggerEvent(hudEvent);
                        break;
                    }
                }
            }
            _onMouseUpEvent->TriggerEvent(hudEvent);
			break;
        } 
			
            //double click do down and up
		case(ON_DOUBLE_CLICK):
        {
			osg::notify(osg::DEBUG_FP) << "		DOUBLE CLICK" << std::endl;
			_onDoubleClickEvent->TriggerEvent(hudEvent);
			break;
        } 
			
            //mouse enter
		case(ON_MOUSE_ENTER):
        {
			//trigger our onMouseUp event
			osg::notify(osg::DEBUG_FP) << "		MOUSE ENTER" << std::endl;
			_onMouseEnterEvent->TriggerEvent(hudEvent);
			break;
        } 
			
            //mouse leave
		case(ON_MOUSE_LEAVE):
        {
			//trigger our onMouseUp event
			osg::notify(osg::DEBUG_FP) << "		MOUSE LEAVE" << std::endl;
			_onMouseEnterEvent->TriggerEvent(hudEvent);
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
	_childMount->addChild(region->GetRegion());
	_children.push_back(region); 
    
	region->SetParent(this); 
}


//
// Checks if the name equal one of or children
//
bool HudRegion::IsChild(const std::string& uniqueID)
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
void HudRegion::RemoveChild(HudRegion* region)
{
    if(!region){return;}
    int deleteIndex = -1;
	for(unsigned int i=0; i<_children.size(); i++){
        if(_children[i]->getName() == region->getName()){
            deleteIndex = i;
            break;
        }
    }
    if(deleteIndex != -1){
        this->RemoveChild(deleteIndex,1);
    }
}

void HudRegion::RemoveChild(unsigned int pos, unsigned int numChildrenToRemove)
{
	if(pos >= _children.size()){return;}
	HudRegionList::iterator first = _children.begin();
	HudRegion* region = (*first+pos);
	_childMount->removeChild(region->GetRegion());
	_children.erase(first+pos);
}

void HudRegion::RemoveChildAllChildren()
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
bool HudRegion::HandleChildEvents(HudInputEvent& hudEvent)
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
bool HudRegion::LoadAssest(const std::string& folderName)
{
	if(_assestLoaded){return true;}
	//if it has no name, then no assets aren't needed
	if(folderName.size() == 0)
	{return true;}
    
	{
        //create the default in XY plane with origin in bottom left
        osg::Vec3 corner = osg::Vec3(0.0f,0.0f,0.0f);
        osg::Vec3 width = osg::Vec3(1.0f,0.0f,0.0f);
        osg::Vec3 height = osg::Vec3(0.0f,1.0f,0.0f);
        float temp = 0.0f;
		//create an xy quad of size 1,1 in its place
		osg::Geometry* geom = NULL;
        
        switch(_origin){
            case ORI_BOTTOM_LEFT:
                break;
            case ORI_TOP_LEFT:
                height *= -1.0f;
                break;
            case ORI_CENTER:
                corner = osg::Vec3(-0.5f,-0.5f,0.0f);
                //width =  osg::Vec3(0.5f,0.0f,0.0f);
                //height =  osg::Vec3(0.0f,0.5f,0.0f);
                break;
            default:break;
        }
        
        //flip plane
        switch(_plane){
            case PLANE_XY:
                break;
            case PLANE_XZ:
                //flip y and z
                temp = corner.y();
                corner.y() = corner.z();
                corner.z() = temp;
                
                temp = width.y();
                width.y() = width.z();
                width.z() = temp;
                
                temp = height.y();
                height.y() = height.z();
                height.z() = temp;
                
                break;
            default:break;
        }
        
        geom = osg::createTexturedQuadGeometry(corner, width, height);
        geom->setColorArray(NULL);//don't use color array as we set color via gl material
#ifdef TARGET_OS_IPHONE
        geom->setUseDisplayList(false);
        geom->setUseVertexBufferObjects(true);
#endif
		
		_region->addDrawable(geom);
		//_region = geode;
        
	}
    
    //load background texture
    if(folderName != "Quad")//use Quad as special name to have just solid colored quad with no texture
    {
        //now try to load a base texture
        std::string baseTextureFile = folderName+".png";
        {
            _baseTexture = NULL;//OsgModelCache::Inst()->getOrLoadTex2D(baseTextureFile);
            if(_baseTexture.get())
            {
                //apply the base as default
                this->ApplyTexture(_baseTexture.get());
            }
        }
        
        //rollover
        std::string rollOverTextureFile = folderName+"_over.png";
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
    
	_assestLoaded = true;
    
	return true;
}

//
//Unload assest, deleting bae/rollover textures and any children
//of _region
//
bool HudRegion::UnLoadAssests()
{
	if(!_assestLoaded){return true;}
    
	//remove children of _region
	_region = NULL;
    
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
void HudRegion::setName(const std::string& name)
{
	_region->setName(name); 
	osg::Object::setName(name);
	MakeHudGeodes(_region.get(), new HudRegionWrapper(this));
}

//positioning

void HudRegion::SetPosition(const osg::Vec2& corner)
{
	_corner = corner;
    
    //flip plane
    float temp = 0.0f;
    osg::Vec3 offset = osg::Vec3(corner.x(),corner.y(),_depth);
    switch(_plane){
        case PLANE_XY:
            break;
        case PLANE_XZ:
            //flip x and z
            temp = offset.y();
            offset.y() = offset.z();
            offset.z() = temp;
            break;
        default:break;
    }
	_translate->setMatrix( osg::Matrix::translate(offset)); 
}

const osg::Vec2& HudRegion::GetPosition() const
{
	return _corner;
}

//
//get set roation around the z axis in degrees
//
void HudRegion::SetRotation(const float& rotate)
{
	_rotation = rotate;
    
    //flip plane
    float temp = 0.0f;
    osg::Vec3 axis = osg::Vec3(0.0f,0.0f,1.0f);
    switch(_rotatePlane){
        case PLANE_XY:
            break;
        case PLANE_XZ:
            //flip x and z (as its rotation)
            temp = axis.y();
            axis.y() = axis.z();
            axis.z() = temp;
            break;
        case PLANE_YZ:
            //flip x and z (as its rotation)
            temp = axis.x();
            axis.x() = axis.z();
            axis.z() = temp;
            break;
        default:break;
    }
    
	_rotate->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(_rotation), axis));
}

const float& HudRegion::GetRotation() const
{
	return _rotation;
}

void HudRegion::SetSize(const osg::Vec2& size)
{
	//resize this regions geometry
    float temp = 0.0f;
    osg::Vec3 sizeAxis = osg::Vec3(size.x(), size.y(), 1.0f);
    switch(_plane){
        case PLANE_XY:
            break;
        case PLANE_XZ:
            //flip x and z
            temp = sizeAxis.y();
            sizeAxis.y() = sizeAxis.z();
            sizeAxis.z() = temp;
            break;
        default:break;
    }
	_scale->setMatrix( osg::Matrix::scale(sizeAxis));
	
	//resize children using the relative scale difference
	//between old and new size
	float xScaler = size.x()/_size.x();
	float yScaler = size.y()/_size.y();
    
	//then scale the relative position of any children
	for(unsigned int i=0; i<_children.size(); i++)
	{
		//check if the child want to inherit
		if((_children[i]->GetTransformInheritMask() & INHERIT_SIZE))
		{_children[i]->SetSizeFromPercentage(xScaler, yScaler);}
        
		if((_children[i]->GetTransformInheritMask() & INHERIT_POSITION))
		{_children[i]->SetPositionFromPercentage(xScaler, yScaler);}
	}
    
	//store new size
	_size = size;
}

const osg::Vec2& HudRegion::GetSize() const
{
	return _size;
}

//
//set the size as a percentage/scale factor of the current size 
//i.e. <1 smaller, >1 bigger
//
void HudRegion::SetSizeFromPercentage(float xScaler, float yScaler)
{
	osg::Vec2 newSize;
	newSize.x() = _size.x()*xScaler;
	newSize.y() = _size.y()*yScaler;
	this->SetSize(newSize);
}

//
//set the position as a percentage/scale factor of the current position
//i.e. <1 smaller, >1 bigger
//
void HudRegion::SetPositionFromPercentage(float xScaler, float yScaler)
{
	osg::Vec2 newPos;
	newPos.x() = _corner.x()*xScaler;
	newPos.y() = _corner.y()*yScaler;
	this->SetPosition(newPos);
}

//
//move to a new layer
//

void HudRegion::SetLayer(const float& depth)
{
    _depth = depth;
    
    //flip plane
    float temp = 0.0f;
    osg::Vec3 offset = osg::Vec3(_corner.x(),_corner.y(),depth);
    switch(_plane){
        case PLANE_XY:
            break;
        case PLANE_XZ:
            //flip x and z
            temp = offset.y();
            offset.y() = offset.z();
            offset.z() = temp;
            break;
        default:break;
    }
	_translate->setMatrix( osg::Matrix::translate(offset)); 
}

const float& HudRegion::GetLayer() const
{
	return _depth;
}


const bool& HudRegion::IsVisible() const
{
	return _visible;
}

void HudRegion::SetVisible(const bool& visible)
{
	//set hud to invisable as default
	if(visible)
	{
		//load assests if micoMemory mode
		if(_microMemoryMode){this->LoadAssest(_assetFolder);}
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
const bool& HudRegion::IsPickable() const
{
    return _pickable;
}
void HudRegion::SetPickable(const bool& pickable)
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
void HudRegion::ApplyNodeMask()
{
    unsigned int nodeMask = 0x0;
    if(_visible){
        nodeMask |= hogbox::MAIN_CAMERA_CULL;
    }
    if(_pickable){
        nodeMask |= hogbox::PICK_MESH;
    }
    if(_region.get())
    {
        _region->setNodeMask(nodeMask);
        _root->setNodeMask(nodeMask);
    }
}

//
//set the texture used by default
//
void HudRegion::SetBaseTexture(osg::Texture* texture)
{
	_baseTexture = NULL;
	_baseTexture = texture;
}
//
//Set the texture used as rollover
//
void HudRegion::SetRolloverTexture(osg::Texture* texture)
{
	_baseTexture = NULL;
	_baseTexture = texture;
}

//
//Apply the texture to the channel 0/diffuse
//
void HudRegion::ApplyTexture(osg::Texture* tex)
{
	if(!_stateset.get()){return;}
	
	_stateset->setTextureAttributeAndModes(0,tex,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    _stateset->addUniform(new osg::Uniform("diffuseTexture", 0));
    
    //
    if(_shaderMode == COLOR_SHADER && tex != NULL){
        osg::Program* program = new osg::Program; 
        program->setName("hudTexturedShader"); 
        program->addShader(new osg::Shader(osg::Shader::VERTEX, texturedVertSource)); 
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, texturedFragSource)); 
        _stateset->setAttributeAndModes(program, osg::StateAttribute::ON); 	
        _shaderMode = TEXTURED_SHADER;
        
    }else if(_shaderMode == TEXTURED_SHADER && tex == NULL){
        osg::Program* program = new osg::Program; 
        program->setName("hudColorShader"); 
        program->addShader(new osg::Shader(osg::Shader::VERTEX, coloredVertSource)); 
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, coloredFragSource)); 
        _stateset->setAttributeAndModes(program, osg::StateAttribute::ON); 	
        _shaderMode = COLOR_SHADER;
    }
	
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
    
    /*if(!hogbox::SystemInfo::Instance()->npotTextureSupported())
     {
     osg::ref_ptr<hogbox::NPOTResizeCallback> resizer = new hogbox::NPOTResizeCallback(tex, 0, _stateset.get());
     //if the texture casts as a rect apply the tex rect scaling to texture coords
     osg::Texture2D* tex2D = dynamic_cast<osg::Texture2D*> (tex); 
     if(tex2D){
     if(resizer->useAsCallBack()){tex2D->setSubloadCallback(resizer.get());}
     }
     }else*/{
         
         //clamp to edge required for IPhone NPOT support
         tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
         tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
         
         
         //if 2d set filtering to nearest
         osg::Texture2D* tex2D = dynamic_cast<osg::Texture2D*> (tex); 
         if(tex2D){
             //set to linear to disable mipmap generation
             tex2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
             tex2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
             tex2D->setUseHardwareMipMapGeneration(false);
             tex2D->setResizeNonPowerOfTwoHint(false);
             tex2D->setUnRefImageDataAfterApply(true);
         }
         
     }
}

void HudRegion::ApplyBaseTexture()
{
	this->ApplyTexture(this->_baseTexture);
}

void HudRegion::ApplyRollOverTexture()
{
	this->ApplyTexture(this->_rollOverTexture);
}

//
//set the material color of the region
//
void HudRegion::SetColor(const osg::Vec3& color)
{
	_color = color;
    
	osg::Vec4 vec4Color = osg::Vec4(color, _alpha);
    
	//set the materials color
	_material->setAmbient(osg::Material::FRONT_AND_BACK, vec4Color ); 
	_material->setDiffuse(osg::Material::FRONT_AND_BACK, vec4Color );
	_material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1.0) );
	_material->setShininess(osg::Material::FRONT_AND_BACK, 1.0);
	_material->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(color,_alpha));
    
    //update color uniform
    _colorUniform->set(vec4Color);
    
	//call for children
	for(unsigned int i=0; i<_children.size(); i++)
	{
		_children[i]->SetColor(color); 
	}
}

const osg::Vec3& HudRegion::GetColor() const
{
	return _color;
}


void HudRegion::SetAlpha(const float& alpha)
{
	_alpha = alpha;
    
	//set the materials alpha
	_material->setAlpha(osg::Material::FRONT_AND_BACK, _alpha);
    
	//if the alpha is zero and we are in microMemory mode
	//we also full hide the region
	if(_microMemoryMode){
		if(_alpha>0.0f){
			SetVisible(true);
		}else{
			SetVisible(false);
		}
	}
    
	//call for children
	for(unsigned int i=0; i<_children.size(); i++)
	{
		_children[i]->SetAlpha(alpha); 
	}
}

const float& HudRegion::GetAlpha() const
{
	return _alpha;
}

//
//get set enable alpha
//
void HudRegion::EnableAlpha(const bool& enable)
{
	if(enable)
	{
		osg::BlendEquation* blendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
		_stateset->setAttributeAndModes(blendEquation, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		osg::BlendFunc* blendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
		_stateset->setAttributeAndModes(blendFunc, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		//tell to sort the mesh before displaying it
		_stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		_stateset->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 
		//_stateset->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        
	}else{
		_stateset->setRenderingHint(osg::StateSet::OPAQUE_BIN);
		_stateset->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	}
	_alphaEnabled = enable;
}

const bool& HudRegion::IsAlphaEnabled() const
{
	return _alphaEnabled;
}

void HudRegion::SetStateSet(osg::StateSet* stateSet)
{
	if(_region.get()!=NULL)
	{
		_region->setStateSet(stateSet);
	}
}

osg::StateSet* HudRegion::GetStateSet()
{
	if(!_stateset.get())
	{return NULL;}
    
	return _stateset.get();
}

//
//Loads and applies a custom shader
//
void HudRegion::SetCustomShader(const std::string& vertFile, const std::string& fragFile, const bool& shadersAreSource)
{
    osg::Shader* fragShader = NULL;
    if(shadersAreSource){
        fragShader = new osg::Shader(osg::Shader::FRAGMENT, fragFile.c_str());
    }else{
        fragShader = new osg::Shader(osg::Shader::FRAGMENT);
        std::string fullFragFile = osgDB::findDataFile(fragFile);
        fragShader->loadShaderSourceFromFile(fullFragFile); 
    }
    
    osg::Shader* vertShader = NULL;
    if(shadersAreSource){
        vertShader = new osg::Shader(osg::Shader::VERTEX, vertFile.c_str());
    }else{
        vertShader = new osg::Shader(osg::Shader::VERTEX);
        std::string fullVertFile = osgDB::findDataFile(vertFile);
        vertShader->loadShaderSourceFromFile(fullVertFile); 
    }
    
    osg::Program* program = new osg::Program(); 
    program->setName("hudCustomShader"); 
    
    program->addShader(fragShader); 
    program->addShader(vertShader); 
    _stateset->setAttributeAndModes(program, osg::StateAttribute::ON); 	
    _shaderMode = CUSTOM_SHADER;
}

//
//disable color writes, i.e. only write to depth buffer
//
void HudRegion::DisableColorWrites()
{
    this->SetColorWriteMask(false);
}
//
//Reenable color writes
//
void HudRegion::EnableColorWrites()
{
    this->SetColorWriteMask(true);
}

//
void HudRegion::SetColorWriteMask(const bool& enable)
{
    osg::ColorMask* colorMask = new osg::ColorMask();
    colorMask->setMask(enable, enable, enable, enable);
    _stateset->setAttributeAndModes(colorMask, osg::StateAttribute::ON);  
}

//
//Disable depth writes
//
void HudRegion::DisableDepthWrites()
{
    this->SetDepthWriteMask(false);
}
//
//Enable depth writes
//
void HudRegion::EnableDepthWrites()
{
    this->SetDepthWriteMask(true);
}

//
void HudRegion::SetDepthWriteMask(const bool& enable)
{
    osg::Depth* depth = new osg::Depth();
    depth->setWriteMask(false);
    _stateset->setAttributeAndModes(depth, osg::StateAttribute::ON);     
}

//
//Disable depth testing
//
void HudRegion::DisableDepthTest()
{
    this->SetDepthTestEnabled(false);
}

//
//Enable depth testing
//
void HudRegion::EnableDepthTest()
{
    this->SetDepthTestEnabled(true);
}

//
void HudRegion::SetDepthTestEnabled(const bool& enable)
{
    _stateset->setMode(GL_DEPTH_TEST,enable ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
}

//
//Enable fast drawm, just disables depth writes and testing
//
void HudRegion::EnableFastDraw()
{
    //this->DisableLighting();
    this->DisableDepthTest();
    this->DisableDepthWrites();
}

//
//Set the renderbin number
void HudRegion::SetRenderBinNumber(const int& num)
{
    _stateset->setRenderBinDetails(num, "RenderBin");
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
float HudRegion::GetAbsoluteRotation()
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
bool HudRegion::AddUpdateCallback(osg::NodeCallback* callback)
{
	//create the new node to handle the callback
	osg::Node* callbackNode = NULL;
	callbackNode = new osg::Node();
	if(callbackNode)
	{
		callbackNode->setUpdateCallback(callback);
		if(_root){_root->addChild(callbackNode);}
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
	return _microMemoryMode; 
}

void HudRegion::SetMicroMemoryMode(const bool& on)
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
void HudRegion::AddOnMouseDownCallbackReceiver(HudEventCallback* callback)
{
	_onMouseDownEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseUpCallbackReceiver(HudEventCallback* callback)
{
	_onMouseUpEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseMoveCallbackReceiver(HudEventCallback* callback)
{
	_onMouseMoveEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseDragCallbackReceiver(HudEventCallback* callback)
{
	_onMouseDragEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnDoubleClickCallbackReceiver(HudEventCallback* callback)
{
	_onDoubleClickEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseEnterCallbackReceiver(HudEventCallback* callback)
{
	_onMouseEnterEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnMouseLeaveCallbackReceiver(HudEventCallback* callback)
{
	_onMouseLeaveEvent->AddCallbackReceiver(callback);
}

//keyboard
void HudRegion::AddOnKeyDownCallbackReceiver(HudEventCallback* callback)
{
	_onKeyDownEvent->AddCallbackReceiver(callback);
}

void HudRegion::AddOnKeyUpCallbackReceiver(HudEventCallback* callback)
{
	_onKeyUpEvent->AddCallbackReceiver(callback);
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
