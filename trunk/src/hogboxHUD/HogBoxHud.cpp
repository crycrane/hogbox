#include <hogboxHUD/HogBoxHud.h>

using namespace hogboxHUD;

//BUG@tom Since moving to CMake the Singleton class has been misbehaving. An app seems to
//use a different instance of the registry then the plugins seem to register too.
//Changing to the dreaded global instance below has fixed things but I need to try and correct this
osg::ref_ptr<HogBoxHud> s_hogboxHUDInstance = NULL;

HogBoxHud* HogBoxHud::Instance(bool erase)
{
	if(s_hogboxHUDInstance==NULL)
	{s_hogboxHUDInstance = new HogBoxHud();}		
	if(erase)
	{
		s_hogboxHUDInstance->destruct();
		s_hogboxHUDInstance = 0;
	}
    return s_hogboxHUDInstance.get();
}

HogBoxHud::HogBoxHud(void) 
: osg::Referenced(), 
_camera(NULL),
_regionGroup(NULL)
{
    
}

HogBoxHud::~HogBoxHud(void)
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
osg::Node* HogBoxHud::Create(osg::Vec2 screenSize)
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
    
    //add the new page to the hud camera
	_camera->addChild( _regionGroup.get());
	
	//create and add our root region
	_hudRegion = new HudRegion(hogboxHUD::HudRegion::PLANE_XY, hogboxHUD::HudRegion::ORI_BOTTOM_LEFT, true);
	_hudRegion->Create(osg::Vec2(0.0f,0.0f), _screenSize, "");
	//set the root region layer far back so we have plenty of positive layers infront
	_hudRegion->SetLayer(-1.0f);
	
	//attach our root region to the hud graph
	_regionGroup->addChild(_hudRegion->GetRegion());
    
	return _camera.get();
}

//
// Pass an input event to all attached regions
//
bool HogBoxHud::HandleInputEvent(HudInputEvent& hudEvent)
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
bool HogBoxHud::AddRegion(HudRegion* region)
{
	//check it is not null
	if(!region)
	{return false;}
    
	_regions.push_back(region);
	_hudRegion->AddChild(region);
    
	return true;
}

bool HogBoxHud::RemoveRegion(HudRegion* region)
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
void HogBoxHud::SetHudVisibility(bool vis)
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
bool HogBoxHud::GetHudVisibility()
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
//Set hud projection size
void HogBoxHud::SetHudProjectionSize(osg::Vec2 size)
{
    _screenSize = size;
    // set the projection matrix
    _camera->setProjectionMatrix(osg::Matrix::ortho2D(0,size.x(),0,size.y()));
}

//
//Rotate and translate the root hud region to run vertically up screen
//
void HogBoxHud::SetHudOrientation(HudOrientation ori)
{
	if(ori == HORIZONTAL_ORIENTATION){
		_hudRegion->SetRotation(0.0f);
		_hudRegion->SetPosition(osg::Vec2(0,0));
	}else{
		_hudRegion->SetRotation(-90.0f);
		_hudRegion->SetPosition(osg::Vec2(0.0f, _screenSize.y()));
	}
}