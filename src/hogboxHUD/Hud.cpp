#include <hogboxHUD/Hud.h>

using namespace hogboxHUD;

//BUG@tom Since moving to CMake the Singleton class has been misbehaving. An app seems to
//use a different instance of the registry then the plugins seem to register too.
//Changing to the dreaded global instance below has fixed things but I need to try and correct this
osg::ref_ptr<Hud> s_hogboxHUDInstance = NULL;

Hud* Hud::Inst(bool erase)
{
	if(s_hogboxHUDInstance==NULL)
	{s_hogboxHUDInstance = new Hud();}		
	if(erase)
	{
		s_hogboxHUDInstance->destruct();
		s_hogboxHUDInstance = 0;
	}
    return s_hogboxHUDInstance.get();
}

Hud::Hud(void) 
    : osg::Referenced(), 
    _camera(NULL),
    _regionGroup(NULL)
{
    
}

Hud::~Hud(void)
{
	OSG_NOTICE << "    Deallocating HogBoxHud Instance." << std::endl;
	_regionGroup = NULL; 
    
	//delete all attached regions, each region is then responsible for 
	//deleting it own children
	for(unsigned int i=0; i<_regions.size(); i++)
	{
		_regions[i] = NULL;
	}
	_regions.clear();
}

//
//Sets up the camera for rendering the hud
//
osg::Node* Hud::Create(osg::Vec2 screenSize)
{
	//store our projection size
	_screenSize = screenSize;
    
	_camera = new osg::Camera;
    _camera->setCullMask(hogbox::MAIN_CAMERA_CULL);
    // set the projection matrix
    _camera->setProjectionMatrix(osg::Matrix::ortho2D(0,_screenSize.x(),0,_screenSize.y()));
	
	//osg::Matrix viewMat = _camera->getViewMatrix();
	//viewMat = viewMat * osg::Matrix::rotate(osg::DegreesToRadians(90.0f), osg::Vec3(0,0,1));
	//_camera->setViewMatrix(viewMat);
    
    // set the view matrix    
    _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _camera->setViewMatrix(osg::Matrix::identity());
    
    // only clear the depth buffer as this is post render
    _camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    
    // draw hud after main camera view.
    _camera->setRenderOrder(osg::Camera::POST_RENDER);
    
	//add the main group to which we attach the regions
	_regionGroup = new osg::Group();
    _regionGroup->setUpdateCallback(new HudUpdateCallback());
    
    //add the new page to the hud camera
	_camera->addChild( _regionGroup.get());
	
	//create and add our root region
	_hudRegion = new Region();
	//_hudRegion->Create(osg::Vec2(0.0f,0.0f), _screenSize, "");
	//set the root region layer far back so we have plenty of positive layers infront
	_hudRegion->SetLayer(-1.0f);
	
	//attach our root region to the hud graph
	_regionGroup->addChild(_hudRegion->GetRegion());
    
	return _camera.get();
}

//
// Pass an input event to all attached regions
//
bool Hud::HandleInputEvent(HudInputEvent& hudEvent)
{
	bool ret=false;
    
	//loop all attached
	for(unsigned int i=0; i<_regions.size(); i++)
	{
		if(_regions[i]->HandleInputEvent(hudEvent)>0)
		{ret = true;}
	}
	return false;
}

//
// Add a region to the hud
//
bool Hud::AddRegion(Region* region)
{
	//check it is not null
	if(!region)
	{return false;}
    
	_regions.push_back(region);
	_hudRegion->AddChild(region);
    
	return true;
}

bool Hud::RemoveRegion(Region* region)
{
    if(!region){return false;}
    int deleteIndex = -1;
    //get index
    for(unsigned int i=0; i<_regions.size(); i++){
        if(_regions[i]->getName() == region->getName()){
            deleteIndex = i;
            break;
        }
    }
    if(deleteIndex != -1){
        _hudRegion->RemoveChild(_regions[deleteIndex].get());
        _regions.erase(_regions.begin()+deleteIndex);
        return true;
    }
    return false;
}

//
//hide the hud
//
void Hud::SetHudVisibility(bool vis)
{
	if(vis)
	{
		_camera->setNodeMask(0xFFFFFFFF);
	}else{
		_camera->setNodeMask(0x0);
	}
}

//
//get visible state
//
bool Hud::GetHudVisibility()
{
	if(	_camera->getNodeMask() == 0xFFFFFFFF)
	{
		return false;
	}else{
		return true;
	}
	return false;
}

//
//Get the hud size (projection size onto screen/view)
//
const osg::Vec2& Hud::GetHudSize()
{
    return _screenSize;
}

//
//Set hud projection size
void Hud::SetHudProjectionSize(osg::Vec2 size)
{
    _screenSize = size;
    // set the projection matrix
    _camera->setProjectionMatrix(osg::Matrix::ortho2D(0,size.x(),0,size.y()));
}

//
//Rotate and translate the root hud region to run vertically up screen
//
void Hud::SetHudOrientation(HudOrientation ori)
{
	if(ori == HORIZONTAL_ORIENTATION){
		_hudRegion->SetRotation(0.0f);
		_hudRegion->SetPosition(osg::Vec2(0,0));
	}else{
		_hudRegion->SetRotation(-90.0f);
		_hudRegion->SetPosition(osg::Vec2(0.0f, _screenSize.y()));
	}
}

//
//Does the hud require redrawing
//
const bool Hud::RequiresRedraw()
{
    return _hudRegion->isRenderStateDirty(); 
}