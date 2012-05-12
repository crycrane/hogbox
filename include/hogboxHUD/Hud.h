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

#include <osg/Camera>
#include <hogboxHUD/Region.h>

namespace hogboxHUD {

class HudUpdateCallback;
    
//
//HogBoxHud
//
//OsgHud is responsible for storing regions and passing events
//Also handles the occurance of window resizing and re positions/resizes
//attached regions accordingly
//
class HOGBOXHUD_EXPORT Hud : public osg::Referenced //, public hogbox::Singleton<HogBoxHud> 
{
public:
    
    friend class HudUpdateCallback;
    
    //friend hogbox::Singleton<HogBoxHud>;
    enum HudOrientation{
        HORIZONTAL_ORIENTATION, //default
        VERTICAL_ORIENTATION
    };
    
    static Hud* Inst(bool erase = false);
    
    //
    //Creates the ortho camera and main region group attached to the camera
    //the ortho matrix is made at screenSize
    osg::Node* Create(osg::Vec2 screenSize);
    
    bool HandleInputEvent(HudInputEvent& hudEvent);
    osg::Node* GetHudNode(){return dynamic_cast<osg::Node*> (_camera.get());}
    
    osg::Group* GetRootGroup(){return _regionGroup.get();}
    
    //add a region to our hud
    bool AddRegion(Region* region);
    bool RemoveRegion(Region* region);
    
    //hide the hud
    void SetHudVisibility(bool vis);
    bool GetHudVisibility(); 
    
    //
    //Get the hud size (projection size onto screen/view)
    const osg::Vec2& GetHudSize();
    
    //
    //Set hud projection size
    void SetHudProjectionSize(osg::Vec2 size);
    
    //
    //Rotate and translate the root hud region to run vertically up screen
    void SetHudOrientation(HudOrientation ori);
    
    //
    //get the ammount of time passed to this frame
    const float& GetTimePassed(){return _timePassed;}
    
    //
    //Does the hud require redrawing
    const bool RequiresRedraw();
    
protected:
    
    Hud(void);
    virtual ~Hud(void);
    
    virtual void destruct(){
        _regions.clear();
        _regionGroup=NULL;
        _camera=NULL;
    }
    
    void SetTimePassed(const float& timePassed){_timePassed = timePassed;}
    
protected:
    
    //track a single prev time so all hudregions can be updated 
    //with a single timePassed (to prevent issues with adding/removing)
    float _timePassed;
    
    //hud is rendered using a post render ortho camera
    osg::ref_ptr<osg::Camera> _camera;
    //region group attached to camera and has all regions attached to it
    osg::ref_ptr<osg::Group> _regionGroup;
    
    //the root region to which all regions are attached. This is
    //attach to the _regionGroup
    osg::ref_ptr<Region> _hudRegion;
    
    //list of regions attached to this hud
    Region::RegionList _regions;
    
    //store current screnn/projection size
    osg::Vec2 _screenSize;
};

//
class HudUpdateCallback : public osg::NodeCallback
{
public:
    HudUpdateCallback() 
        : osg::NodeCallback(),
        _prevTick(0.0f)
    {
    }
    
    //
    //Update operator
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if (nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR && 
            nv->getFrameStamp())
        {
            //get the time passed since last update
            double time = nv->getFrameStamp()->getReferenceTime();
            if(_prevTick==0.0f){_prevTick = time;}
            float timePassed = time - _prevTick;
            Hud::Inst()->SetTimePassed(timePassed);
            _prevTick = time;
        }
        osg::NodeCallback::traverse(node,nv);
    }
    
protected:
    
    virtual~HudUpdateCallback(void){}
    
protected:
    float _prevTick;
};
    
};