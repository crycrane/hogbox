/* Written by Thomas Hogarth, (C) 2011
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 */

#pragma once

//#include <hogbox/Singleton.h>
#include <osg/Camera>

#include "HudRegion.h"

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
    enum HudOrientation{
        HORIZONTAL_ORIENTATION, //default
        VERTICAL_ORIENTATION
    };
    
    static HogBoxHud* Instance(bool erase = false);
    
    //
    //Creates the ortho camera and main region group attached to the camera
    //the ortho matrix is made at screenSize
    osg::Node* Create(osg::Vec2 screenSize);
    
    bool HandleInputEvent(HudInputEvent& hudEvent);
    osg::Node* GetHudNode(){return dynamic_cast<osg::Node*> (m_camera.get());}
    
    osg::Group* GetRootGroup(){return m_regionGroup.get();}
    
    //add a region to our hud
    bool AddRegion(HudRegion* region);
    bool RemoveRegion(HudRegion* region);
    
    //hide the hud
    void SetHudVisibility(bool vis);
    bool GetHudVisibility(); 
    
    //
    //Set hud projection size
    void SetHudProjectionSize(osg::Vec2 size);
    
    //
    //Rotate and translate the root hud region to run vertically up screen
    void SetHudOrientation(HudOrientation ori);
    
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
    osg::ref_ptr<osg::Camera> m_camera;
    //region group attached to camera and has all regions attached to it
    osg::ref_ptr<osg::Group> m_regionGroup;
    
    //the root region to which all regions are attached. This is
    //attach to the m_regionGroup
    osg::ref_ptr<HudRegion> m_hudRegion;
    
    //list of regions attached to this hud
    HudRegion::HudRegionList m_regions;
    
    //store current screnn/projection size
    osg::Vec2 m_screenSize;
};
    
};