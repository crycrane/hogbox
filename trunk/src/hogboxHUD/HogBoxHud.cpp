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

HogBoxHud::HogBoxHud(void) : m_camera(NULL),
						m_regionGroup(NULL)
{

}

HogBoxHud::~HogBoxHud(void)
{
	OSG_NOTICE << "    Deallocating HogBoxHud Instance." << std::endl;
	m_regionGroup = NULL; 

	//delete all attached regions, each region is then responsible for 
	//deleting it own children
	for(unsigned int i=0; i<m_regions.size(); i++)
	{
		m_regions[i] = NULL;
	}
	m_regions.clear();
}

//
//Sets up the camera for rendering the hud
//
osg::Node* HogBoxHud::Create(osg::Vec2 screenSize)
{
	//store our projection size
	m_screenSize = screenSize;

	m_camera = new osg::CameraNode;

    // set the projection matrix
    m_camera->setProjectionMatrix(osg::Matrix::ortho2D(0,m_screenSize.x(),0,m_screenSize.y()));
	
	//osg::Matrix viewMat = m_camera->getViewMatrix();
	//viewMat = viewMat * osg::Matrix::rotate(osg::DegreesToRadians(90.0f), osg::Vec3(0,0,1));
	//m_camera->setViewMatrix(viewMat);

    // set the view matrix    
    m_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    m_camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer as this is post render
    m_camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw hud after main camera view.
    m_camera->setRenderOrder(osg::CameraNode::POST_RENDER);

	//add the main group to which we attach the regions
	m_regionGroup = new osg::Group();

    //add the new page to the hud camera
	m_camera->addChild( m_regionGroup.get());
	
	//create and add our root region
	m_hudRegion = new HudRegion(true);
	m_hudRegion->Create(osg::Vec2(0.0f,0.0f), m_screenSize, "");
	//set the root region layer far back so we have plenty of positive layers infront
	m_hudRegion->SetLayer(-1.0f);
	
	//attach our root region to the hud graph
	m_regionGroup->addChild(m_hudRegion->GetRegion());

	return m_camera.get();
}

//
// Pass an input event to all attached regions
//
bool HogBoxHud::HandleInputEvent(HudInputEvent& hudEvent)
{
	bool ret=false;

	//loop all attached
	for(unsigned int i=0; i<m_regions.size(); i++)
	{
		if(m_regions[i]->HandleInputEvent(hudEvent)>0)
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

	m_regions.push_back(region);
	m_hudRegion->AddChild(region);

	return true;
}

//
//hide the hud
//
void HogBoxHud::SetHudVisibility(bool vis)
{
	if(vis)
	{
		m_camera->setNodeMask(0xFFFFFFFF);
	}else{
		m_camera->setNodeMask(0x0);
	}
}

//
//get visible state
//
bool HogBoxHud::GetHudVisibility()
{
	if(	m_camera->getNodeMask() == 0xFFFFFFFF)
	{
		return false;
	}else{
		return true;
	}
	return false;
}

//
//Rotate and translate the root hud region to run vertically up screen
//
void HogBoxHud::SetHudOrientation(HudOrientation ori)
{
	if(ori == HORIZONTAL_ORIENTATION){
		m_hudRegion->SetRotation(0.0f);
		m_hudRegion->SetPosition(osg::Vec2(0,0));
	}else{
		m_hudRegion->SetRotation(90.0f);
		m_hudRegion->SetPosition(osg::Vec2(m_screenSize.x(), 0.0f));
	}
}
