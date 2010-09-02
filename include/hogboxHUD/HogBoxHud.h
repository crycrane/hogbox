#pragma once

//#include <hogbox/Singleton.h>
#include <osg/CameraNode>

#include <hogboxHUD/HudRegion.h>

namespace hogboxHUD {

//
//HogBoxHud
//
//OsgHud is responsible for storing regions and passing events
//Also handles the occurance of window resizing and re positions/resizes
//attached regions accordingly
//
class HOGBOXHUD_EXPORT HogBoxHud : public osg::Referenced //, public hogbox::Singleton<HogBoxHud> 
{
public:

	//friend hogbox::Singleton<HogBoxHud>;

	static HogBoxHud* Instance(bool erase = false);

	//
	//Creates the ortho camera and main region group attached to the camera
	//the ortho matrix is made at screenSize
	osg::Node* Create(osg::Vec2 screenSize);

	bool Event(const std::string ID, CHudEvent hudEvent);
	osg::Node* GetHudNode(){return dynamic_cast<osg::Node*> (m_camera.get());}

	//add a region to our hud
	bool AddRegion(HudRegion* region);

	//hide the hud
	void SetHudVisibility(bool vis);
	bool GetHudVisibility(); 

protected:
	
	HogBoxHud(void);
	virtual ~HogBoxHud(void);

	virtual void destruct(){
		m_regions.clear();
		m_regionGroup=NULL;
		m_camera=NULL;
	}

protected:

	//hud is rendered using a post render ortho camera
	osg::ref_ptr<osg::CameraNode> m_camera;
	//region group attached to camera and has all regions attached to it
	osg::ref_ptr<osg::Group> m_regionGroup;

	//info regions attached to this camera
	HudRegion::HudRegionList m_regions;

	//store current screnn/projection size
	osg::Vec2 m_screenSize;
};

};