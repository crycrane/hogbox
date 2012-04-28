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

#include <hogboxHUD/Export.h>
#include <hogbox/HogBoxBase.h>
#include <hogbox/Quad.h>
#include <hogbox/AnimatedTransformQuad.h>

#include <osg/ValueObject>
#include <osg/UserDataContainer>

#include "HudInputEvent.h"
#include "HudEventCallback.h"



namespace hogboxHUD {

//what is all this doing here, sort it    

//InheritanceMask masks,
//Currently only adding/removing the INHERIT_SIZE has any effect
typedef unsigned int InheritanceMask;
#define	INHERIT_NONE			0x00000000
#define	INHERIT_POSITION		0x00000001
#define	INHERIT_ROTATION		0x00000002
#define	INHERIT_SIZE			0x00000004
#define INHERIT_ALL_TRANSFORMS (INHERIT_POSITION|INHERIT_ROTATION|INHERIT_SIZE)

class Region;
typedef osg::ref_ptr<Region> RegionPtr;
class RegionWrapper;
//forward declare our update callback
class RegionUpdateCallback;

//helper func to rename geodes and attach region as user data
extern HOGBOXHUD_EXPORT void MakeHudGeodes(osg::Node* node, osg::ref_ptr<RegionWrapper> region);
extern HOGBOXHUD_EXPORT void ClearHudGeodes(osg::Node* node);


//
//Base hud region
//
class HOGBOXHUD_EXPORT Region : public hogbox::AnimatedTransformQuad
{
public:
    
    class RegionStyle : public TransformQuadArgs{
    public:
        RegionStyle()
            : TransformQuadArgs(),
            _color(osg::Vec3(-1.0f,-1.0f,-1.0f)),//-1 indicates to not use the color arg
            _alpha(-1.0f), //-1 indicates to not use the alpha
            _assets("")
        {
        }
        
        RegionStyle(const RegionStyle& args)
            : TransformQuadArgs(args),
            _color(args._color),
            _alpha(args._alpha),
            _assets(args._assets)
        {
        }
        
        osg::Vec3 _color;
        float _alpha;
        
        std::string _assets;
        
    protected:
        virtual ~RegionStyle(){
        }
    };
    
    //
    Region(RegionStyle* args = new RegionStyle());
    
    //
    //
    Region(osg::Vec2 corner, osg::Vec2 size, RegionStyle* args = new RegionStyle());
    
    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    Region(const Region& region,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    META_Object(hogboxHUD,Region);
    virtual RegionStyle* allocateStyleType(){return new RegionStyle();}
    
    typedef std::vector<RegionPtr> RegionList;
    
    //
    //Create will load our assets and call setposition, setsize set
    virtual bool Create(osg::Vec2 corner, osg::Vec2 size, RegionStyle* style = NULL);
    
    //
    //Convenience method to create a RegionStyle with provided args
    //then pass to base Create
    virtual bool CreateWithAsset(osg::Vec2 corner, osg::Vec2 size, const std::string& asset);
    
    //was the region auto created by a parent (i.e. we don't want to save it to disk)
    bool isProcedural(){return _isProcedural;}
    
    //return the regions root osg transform node
    osg::MatrixTransform* GetRegion();
    
    //overload the setnames of osg object so that
    //the geodes etc also use the name for picking
    void setName( const std::string& name );
    
    
    //adds a child to this region which will then be transformed relative
    //to this region
    void AddChild(Region* region);
    bool IsChild(const std::string& uniqueID);
    //set this regions parent, NULL if attached directly
    //to the hud
    void SetParent(Region* parent){p_parent=parent;}
    Region* GetParent(){return p_parent;}
    
    //remove child
    bool RemoveChild(Region* region);
    bool RemoveChild(unsigned int pos, unsigned int numChildrenToRemove=1);
    void RemoveAllChildren();
    
    //get number of children
    unsigned int GetNumChildren(){
        return _children.size();
    }
    
    //Events
    
    //Handle a hud event passed by the picker, i.e. mouse click, key press
    //geodeID is the name of the geode picked in the hud
    virtual int HandleInputEvent(HudInputEvent& hudEvent);//  = 0;
    //pass event on to child regions
    bool HandleChildEvents(HudInputEvent& hudEvent);

    
    //Get set the transform inheritance mask
    void SetTransformInheritMask(InheritanceMask inherit){
        _transformInheritMask = inherit;
    }
    InheritanceMask GetTransformInheritMask(){
        return _transformInheritMask;
    }
    
    //visibility
    const bool& IsVisible() const;
    void SetVisible(const bool& visible);
    
    //pickable by hudinput
    const bool& IsPickable() const;
    void SetPickable(const bool& pickable);
    
    const std::string& GetAssetFolder()const{
        return _assetFolder;
    }
    
    //set the texture used by default
    void SetBaseTexture(osg::Texture* texture);
    osg::Texture* GetBaseTexture(){return _baseTexture.get();}
    //set the texture used in mouse rollovers
    void SetRolloverTexture(osg::Texture* texture);
    
    //
    void ApplyBaseTexture();
    void ApplyRollOverTexture();
    
    //
    //override setAlpha to set alpha of children
    virtual void SetAlpha(const float& alpha);
    
    
    //convert coords into the regions local system with this corner as the origin
    osg::Vec2 GetRegionSpaceCoords(osg::Vec2 spCoords);
    //recerse through parents to get screen space corner
    osg::Vec2 GetAbsoluteCorner();
    //recurse through parents and find the regions absolute rotation
    float GetAbsoluteRotation();
    
    //
    //add an updatecallback, this is done by creating a new node
    //attaching the update callback to it, then attaching the new node
    //to the root for updating
    bool AddUpdateCallback(osg::NodeCallback* callback);
    
    
    //
    //Helpers to get set the entire list of children (used by hudregion xml plugin)
    //Set will pass each list item through add child
    RegionList GetChildrenList()const{return _children;}
    void SetChildrenList(const RegionList& list);
    
    
    //
    //Get/Set use of microMemory mode
    const bool& GetMicroMemoryMode()const;
    void SetMicroMemoryMode(const bool& on);
    
    //
    //return the build args as a RegionStyle
    RegionStyle* ArgsAsRegionStyle(){
        return dynamic_cast<RegionStyle*>(_args.get());
    }
    
    //
    //Funcs to register event callbacks
    //mouse events
    void AddOnMouseDownCallbackReceiver(HudEventCallback* callback);
    void AddOnMouseUpCallbackReceiver(HudEventCallback* callback);
    void AddOnMouseMoveCallbackReceiver(HudEventCallback* callback);
    void AddOnMouseDragCallbackReceiver(HudEventCallback* callback);
    void AddOnDoubleClickCallbackReceiver(HudEventCallback* callback);
    void AddOnMouseEnterCallbackReceiver(HudEventCallback* callback);
    void AddOnMouseLeaveCallbackReceiver(HudEventCallback* callback);
    //keyboard
    void AddOnKeyDownCallbackReceiver(HudEventCallback* callback);
    void AddOnKeyUpCallbackReceiver(HudEventCallback* callback);
    
protected:
    
    //destructor
    virtual ~Region(void);
    
    //
    //Init Callback Events
    virtual void InitEvents();
    
    //
    //Init Material/stateset stuff
    virtual void InitStateSet();
    
    //
    //Uses style to loaded required assets if any
    //_assets string is used to indicate 
    //a. that a region want a rendered quad (e.g. _assets = "Quad"
    //b. the file name for a diffuse texture excluding extension e.g. Images/button (for now)
    virtual bool LoadAssest(RegionStyle* style);
    
    //
    //Unload assest, deleting bae/rollover textures and any children
    //of _region
    virtual bool UnLoadAssests();

    
    //
    //compute and apply the current node mask for the region geode
    void ApplyNodeMask();
    
    //
    //general update, updates and syncs our animations etc
    //normally called by an attached updateCallback
    virtual void UpdateAnimation(float simTime);
    
protected:
    
    //a value of true indicates that this region was
    //auto created by a parent (a silder making the inc and dec buttons for example)
    //these type of region do not want to be saved out
    bool _isProcedural;
    
    //
    //Folder containing region render assests
    std::string _assetFolder;
    //tracks if assets have been loaded
    bool _assestLoaded;
    
    //child mount attached to rotate, child nodes are resized
    //individually so _scale matrix does not affect them
    osg::ref_ptr<osg::MatrixTransform> _childMount;
    
    

    //the inherit mask, telling us which
    //transform features to inherit from parent
    InheritanceMask _transformInheritMask;
    
    //visibility of the region, renderable mask
    bool _visible;
    
    //pickable mask applied
    bool _pickable;
    
    //has the region changed since last time
    bool _hovering; //is it in a hover state
    
    //standard texture 
    osg::ref_ptr<osg::Texture> _baseTexture;
    
    
    //texture used on rollover
    osg::ref_ptr<osg::Texture> _rollOverTexture;
    
    //nodes parent region, null if no parent (should this be shared or weak)
    Region* p_parent;
    
    //regions attached to this one
    RegionList _children; 

    
    //
    //microMemoryMode=true, indicates that we want to unref our textures image data
    //and we want to full delete the textures when hidden, the textures are reloaded
    //when the region is shown again. Default is false/off
    bool _microMemoryMode;
    
    //
    //regions can have a stroke which is just another region positioned behind
    osg::ref_ptr<Region> _strokeRegion;

    
    //Callback system
    
    //base hud region handles a few basic hud callbacks
    //OnMouseDown, called when mouse down/press event occurs in the region
    //OnMouseUp, called when mouse up/release event occurs in the region
    //OnMouseMove, called if mouse moves in the regions area
    //OnMouseDrag, same as mouseMove only a mouse button is held down
    //OnMouseEnter, called when mouse first enters the regions area
    //OnMouseLeave, called when mouse leaves the area (input model will currently make this difficult to detect)
    
    //mouse events
    osg::ref_ptr<HudCallbackEvent> _onMouseDownEvent;
    osg::ref_ptr<HudCallbackEvent> _onMouseUpEvent;
    osg::ref_ptr<HudCallbackEvent> _onMouseMoveEvent;
    osg::ref_ptr<HudCallbackEvent> _onMouseDragEvent;
    osg::ref_ptr<HudCallbackEvent> _onDoubleClickEvent;
    osg::ref_ptr<HudCallbackEvent> _onMouseEnterEvent;
    osg::ref_ptr<HudCallbackEvent> _onMouseLeaveEvent;
    
    //keyboard events
    osg::ref_ptr<HudCallbackEvent> _onKeyDownEvent;
    osg::ref_ptr<HudCallbackEvent> _onKeyUpEvent;
};

//
//Wrapper to store a normal pointer to a hudregion
//within an osg referenced object.
class RegionWrapper : public osg::Referenced
{
public:
    RegionWrapper(Region* region)
    : osg::Referenced(),
    p_region(region)
    {
    }
    
    Region* GetRegion(){
        return p_region;
    }
    void SetRegion(Region* region){
        p_region=region;
    }
    
protected:
    
    virtual ~RegionWrapper(){
        p_region=NULL;
    }
    
protected:
    Region* p_region;
};
    
}; //end hogboxhud namespace
