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
#include <hogbox/AnimateValue.h>


#include <osg/MatrixTransform>
#include <osg/Material>
#include<osg/Texture2D>

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

class HudRegion;
typedef osg::ref_ptr<HudRegion> HudRegionPtr;
class HudRegionWrapper;
//forward declare our update callback
class HudRegionUpdateCallback;

//helper func to rename geodes and attach region as user data
extern HOGBOXHUD_EXPORT void MakeHudGeodes(osg::Node* node, osg::ref_ptr<HudRegionWrapper> region);
extern HOGBOXHUD_EXPORT void ClearHudGeodes(osg::Node* node);


//
//Base hud region
//
class HOGBOXHUD_EXPORT HudRegion : public osg::Object
{
public:
    
    //orientation
    enum RegionPlane{
        PLANE_XY = 0,
        PLANE_XZ = 1,
        PLANE_YZ = 2
    };
    
    enum RegionOrigin{
        ORI_BOTTOM_LEFT = 0,
        ORI_TOP_LEFT = 1,
        ORI_CENTER = 2
    };
    
    enum ShaderMode{
        COLOR_SHADER,
        TEXTURED_SHADER,
        CUSTOM_SHADER
    };
    
    //When constructing a region automatically within the create of a parent region
    //isProcedural should be flaged as true so that when hudregions are saved the
    //system knows not to save out the automaticaly created regions
    HudRegion(RegionPlane plane=PLANE_XY, RegionOrigin origin=ORI_BOTTOM_LEFT, bool isProcedural=false);
    
    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    HudRegion(const HudRegion& region,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    META_Object(hogboxHUD,HudRegion);
    
    typedef std::vector<HudRegionPtr> HudRegionList;
    
    //load assets, set size and position, then apply the names to any geodes for picking
    virtual bool Create(osg::Vec2 corner, osg::Vec2 size, const std::string& fileName, bool microMemoryMode=false);
    
    //was the region auto created by a parent (i.e. we don't want to save it to disk)
    bool isProcedural(){return _isProcedural;}
    
    //return the regions root osg transform node
    osg::MatrixTransform* GetRegion();
    
    //overload the setnames of osg object so that
    //the geodes etc also use the name for picking
    void setName( const std::string& name );
    
    
    //adds a child to this region which will then be transformed relative
    //to this region
    void AddChild(HudRegion* region);
    bool IsChild(const std::string& uniqueID);
    //set this regions parent, NULL if attached directly
    //to the hud
    void SetParent(HudRegion* parent){p_parent=parent;}
    HudRegion* GetParent(){return p_parent;}
    
    //remove child
    void RemoveChild(HudRegion* region);
    void RemoveChild(unsigned int pos, unsigned int numChildrenToRemove=1);
    void RemoveChildAllChildren();
    
    //get number of children
    unsigned int GetNumChildren(){
        return _children.size();
    }
    
    //Events
    
    
    //
    //general update, updates and syncs our animations etc
    //normally called by an attached updateCallback
    virtual void Update(float simTime);
    
    //Handle a hud event passed by the picker, i.e. mouse click, key press
    //geodeID is the name of the geode picked in the hud
    virtual int HandleInputEvent(HudInputEvent& hudEvent);//  = 0;
    //pass event on to child regions
    bool HandleChildEvents(HudInputEvent& hudEvent);
    
    
    //translation in hudspace, set position virtual
    //to handle special cases
    virtual void SetPosition(const osg::Vec2& corner);
    const osg::Vec2& GetPosition() const;
    
    //get set roation around the z axis in degrees
    virtual void SetRotation(const float& rotate);
    const float& GetRotation() const;
    
    //set the plane of rotation
    void SetRotationPlane(RegionPlane plane){_rotatePlane = plane;}
    const RegionPlane& GetRotationPlane() const{return _rotatePlane;}
    
    //set the size of this region in hud coords 
    //via the _scale matrix, any children will be resize using the
    //percentage difference of the passed in and previous sizes
    //resize is virtual so different type cans handle special situations
    virtual void SetSize(const osg::Vec2& size);
    //return the size of the region in hud coord
    const osg::Vec2& GetSize() const;
    
    //set the size as a percentage/scale factor of the current size
    //i.e. <1 smaller, >1 bigger
    void SetSizeFromPercentage(float xScaler, float yScaler);
    
    //set the position as a percentage/scale factor of the current position
    //i.e. <1 smaller, >1 bigger
    void SetPositionFromPercentage(float xScaler, float yScaler);
    
    //move to a new layer, z depth relative to parent
    void SetLayer(const float& depth);
    const float& GetLayer() const;
    
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
    
    //Apply the texture to the channel 0/diffuse
    virtual void ApplyTexture(osg::Texture* tex);
    void ApplyBaseTexture();
    void ApplyRollOverTexture();
    
    //set the material color of the region
    void SetColor(const osg::Vec3& color);
    const osg::Vec3& GetColor() const;
    
    //set the regions Alpha value
    virtual void SetAlpha(const float& alpha);
    const float& GetAlpha() const;
    //get set enable alpha
    void EnableAlpha(const bool& enable);
    const bool& IsAlphaEnabled() const;
    
    //Hack for now, but blending doesn't work without light, but coloring doesn't work with
    void DisableLighting(){
        _stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    }
    
    
    //Set the regions stateset directly
    void SetStateSet(osg::StateSet* stateSet);
    //return the stateset applied to root
    osg::StateSet* GetStateSet(); 
    
    //
    //Loads and applies a custom shader
    void SetCustomShader(const std::string& vertShader, const std::string& fragShader, const bool& shadersAreSource = false);
    
    //
    //disable color writes, i.e. only write to depth buffer
    void DisableColorWrites();
    //
    //Reenable color writes
    void EnableColorWrites();
    
    //
    void SetColorWriteMask(const bool& enable);
    
    //
    //Disable depth writes
    void DisableDepthWrites();
    //
    //Enable depth writes
    void EnableDepthWrites();
    
    //
    void SetDepthWriteMask(const bool& enable);
    
    //
    //Disable depth testing
    void DisableDepthTest();
    
    //
    //Enable depth testing
    void EnableDepthTest();
    
    //
    void SetDepthTestEnabled(const bool& enable);
    
    //
    //Enable fast drawm, just disables depth writes and testing
    void EnableFastDraw();
    
    //
    //Set the renderbin number
    void SetRenderBinNumber(const int& num);
    
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
    HudRegionList GetChildrenList()const{return _children;}
    void SetChildrenList(const HudRegionList& list);
    
    
    //
    //Animation
    
    //Rotation channel
    template <typename M>
    void AddRotationKey(float degrees, float duration)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animateRotate->GetNumKeys() == 0){_animateRotate->SetValue(this->GetRotation());}
        _animateRotate->AddKey<M>(degrees, duration);
    }
    hogbox::AnimateValue<float>::KeyFrame* GetRotationKey(unsigned int index){return _animateRotate->GetKey(index);}
    bool RemoveRotationKey(unsigned int index){return _animateRotate->RemoveKey(index);}
    unsigned int GetNumRotationKeys(){return _animateRotate->GetNumKeys();}
    
    //Position channel
    template <class M>
    void AddPositionKey(osg::Vec2 pos, float duration)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animatePosition->GetNumKeys() == 0){_animatePosition->SetValue(this->GetPosition());}
        _animatePosition->AddKey<M>(pos, duration);
    }
    hogbox::AnimateValue<osg::Vec2>::KeyFrame* GetPositionKey(unsigned int index){return _animatePosition->GetKey(index);}
    bool RemovePositionKey(unsigned int index){return _animatePosition->RemoveKey(index);}
    unsigned int GetNumPositionKeys(){return _animatePosition->GetNumKeys();}
    
    //Size channel
    template <typename M>
    void AddSizeKey(osg::Vec2 size, float duration)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animateSize->GetNumKeys() == 0){_animateSize->SetValue(this->GetSize());}
        _animateSize->AddKey<M>(size, duration);
    }
    hogbox::AnimateValue<osg::Vec2>::KeyFrame* GetSizeKey(unsigned int index){return _animateSize->GetKey(index);}
    bool RemoveSizeKey(unsigned int index){return _animateSize->RemoveKey(index);}
    unsigned int GetNumSizeKeys(){return _animateSize->GetNumKeys();}
    
    //Color channel
    template <typename M>
    void AddColorKey(osg::Vec3 color, float duration)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animateColor->GetNumKeys() == 0){_animateColor->SetValue(this->GetColor());}
        _animateColor->AddKey<M>(color, duration);
    }
    hogbox::AnimateValue<osg::Vec3>::KeyFrame* GetColorKey(unsigned int index){return _animateColor->GetKey(index);}
    bool RemoveColorKey(unsigned int index){return _animateColor->RemoveKey(index);}
    unsigned int GetNumColorKeys(){return _animateColor->GetNumKeys();}
    
    //Alpha channel
    template <typename M>
    void AddAlphaKey(float alpha, float duration)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animateAlpha->GetNumKeys() == 0){_animateAlpha->SetValue(this->GetAlpha());}
        _animateAlpha->AddKey<M>(alpha, duration);
    }
    hogbox::AnimateValue<float>::KeyFrame* GetAlphaKey(unsigned int index){return _animateAlpha->GetKey(index);}
    bool RemoveAlphaKey(unsigned int index){return _animateAlpha->RemoveKey(index);}
    unsigned int GetNumAlphaKeys(){return _animateAlpha->GetNumKeys();}
    
    //are any of our channels still animating
    bool IsAnimating()
    {
        unsigned int totalKeys = GetNumPositionKeys() + GetNumSizeKeys() + GetNumColorKeys() + GetNumAlphaKeys();
        if(totalKeys > 0)
        {return true;}
        return false;
    }
    
    //
    //Enable disable any updates of animations
    void SetAnimationDisabled(bool disable){_animationDisabled = disable;}
    
    //
    //Get/Set use of microMemory mode
    const bool& GetMicroMemoryMode()const;
    void SetMicroMemoryMode(const bool& on);
    
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
    virtual ~HudRegion(void);
    
    //
    //Load assest takes a folder name containing our assest
    //each region type expects to find specificlty named assests in the folder
    //Base implementation loads 
    //geom.osg, used as render geometry (if not present then a quad is generated with bottom left corner at 0,0)
    //base.png, used as the default texture if present
    //rollover.png, used for mouse rollovers if present
    virtual bool LoadAssest(const std::string& folderName);
    
    //
    //Unload assest, deleting bae/rollover textures and any children
    //of _region
    virtual bool UnLoadAssests();
    
    //
    //compute and apply the current node mask for the region geode
    void ApplyNodeMask();
    
protected:
    
    //a value of true indicates that this region was
    //auto created by a parent (a silder making the inc and dec buttons for example)
    //these type of region do not want to be saved out
    bool _isProcedural;
    
    RegionPlane _plane;
    RegionPlane _rotatePlane;
    RegionOrigin _origin;
    
    //
    //Folder containing region render assests
    std::string _assetFolder;
    //tracks if assets have been loaded
    bool _assestLoaded;
    
    //main root attached to hud
    osg::ref_ptr<osg::MatrixTransform> _root;
    //translate attached to root, for hudspace translation (xy)
    osg::ref_ptr<osg::MatrixTransform> _translate;
    //rotate attached to translate, for rotation around z axis
    osg::ref_ptr<osg::MatrixTransform> _rotate;
    //scale attached to rotate, scales the region geom to
    //region size. 
    osg::ref_ptr<osg::MatrixTransform> _scale;
    
    //child mount attached to rotate, child nodes are resized
    //individually so _scale matrix does not affect them
    osg::ref_ptr<osg::MatrixTransform> _childMount;
    
    //region node attached to scale, contains the actual geometry
    //or drawable representation of the region which should be 1x1
    //_scale will then make it our desired size
    osg::ref_ptr<osg::Geode> _region; 
    
    //size in hud coords
    osg::Vec2 _size;
    //bottom left corner in screen or parent space
    //i.e. if the region is attached as child to another
    //the regions origin becomes the parents bottom left corner
    osg::Vec2 _corner;
    //rotation in degrees arond the z axis
    float _rotation;
    //depth/layer of the region i.e. z coord relative to parent
    //the root region held by the hogboxHud sets its depth/layer to -1
    float _depth;
    
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
    HudRegion* p_parent;
    
    //regions attached to this one
    HudRegionList _children; 
    
    //the material applied to the region (should I just use HogBoxMaterial?)
    osg::ref_ptr<osg::StateSet> _stateset; 
    osg::ref_ptr<osg::Material> _material;
    osg::Vec3 _color;
    //color uniform for shader, as vec4 including alpha
    osg::ref_ptr<osg::Uniform> _colorUniform;
    float _alpha;
    bool _alphaEnabled;
    
    //
    ShaderMode _shaderMode;
    
    //
    //microMemoryMode=true, indicates that we want to unref our textures image data
    //and we want to full delete the textures when hidden, the textures are reloaded
    //when the region is shown again. Default is false/off
    bool _microMemoryMode;
    
    
    
    //
    //Our default update callback to ensure update is called each frame
    osg::ref_ptr<HudRegionUpdateCallback> _updateCallback;
    
    //
    //Animation for each of our attributes
    hogbox::AnimateFloatPtr _animateRotate; bool _isRotating;
    hogbox::AnimateVec2Ptr _animatePosition; bool _isTranslating;
    hogbox::AnimateVec2Ptr _animateSize; bool _isSizing;
    hogbox::AnimateVec3Ptr _animateColor; bool _isColoring;
    hogbox::AnimateFloatPtr _animateAlpha; bool _isFading;
    
    //previous framestamp time to calc time elapsed
    float _prevTick;
    
    //used to stop any animation
    bool _animationDisabled;
    
    //Callback system
    
    //base hud region handles a few basic hud callbacks
    //OnMouseDown, called when mouse down/press event occurs in the region
    //OnMouseUp, called when mouse up/release event occurs in the region
    //OnMouseMove, called if mouse moves in the regions area
    //OnMouseDrag, same as mouseMove only a mouse button is held down
    //OnMouseEnter, called when mouse first enters the regions area
    //OnMouseLeave, called when mouse leaves the area (input model will currently make this difficult to detect)
    
    //mouse events
    osg::ref_ptr<CallbackEvent> _onMouseDownEvent;
    osg::ref_ptr<CallbackEvent> _onMouseUpEvent;
    osg::ref_ptr<CallbackEvent> _onMouseMoveEvent;
    osg::ref_ptr<CallbackEvent> _onMouseDragEvent;
    osg::ref_ptr<CallbackEvent> _onDoubleClickEvent;
    osg::ref_ptr<CallbackEvent> _onMouseEnterEvent;
    osg::ref_ptr<CallbackEvent> _onMouseLeaveEvent;
    
    //keyboard events
    osg::ref_ptr<CallbackEvent> _onKeyDownEvent;
    osg::ref_ptr<CallbackEvent> _onKeyUpEvent;
};

//
//Wrapper to store a normal pointer to a hudregion
//within an osg referenced object.
class HudRegionWrapper : public osg::Referenced
{
public:
    HudRegionWrapper(HudRegion* region)
    : osg::Referenced(),
    p_region(region)
    {
    }
    
    HudRegion* GetRegion(){
        return p_region;
    }
    void SetRegion(HudRegion* region){
        p_region=region;
    }
    
protected:
    
    virtual ~HudRegionWrapper(){
        p_region=NULL;
    }
    
protected:
    HudRegion* p_region;
};

//
//HudRegionUpdateCallback
//our default update callback which will ensure animations etc are updated
//
class HudRegionUpdateCallback : public osg::NodeCallback
{
public:
    //contuct, passing the region to update
    HudRegionUpdateCallback(HudRegion* region) 
    : osg::NodeCallback(),
    p_updateRegion(region),
    _prevTick(0.0f)
    {
    }
    
    //
    //Update operator
    //Here we smooth to targets if value is not equal
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if (p_updateRegion &&
            nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR && 
            nv->getFrameStamp())
        {
            //get the time passed since last update
            double time = nv->getFrameStamp()->getReferenceTime();
            if(_prevTick==0.0f){_prevTick = time;}
            //float timePassed = time - _prevTick;
            _prevTick = time;
            
            p_updateRegion->Update(time);
        }
        osg::NodeCallback::traverse(node,nv);
    }
    
protected:
    
    virtual~HudRegionUpdateCallback(void){}
    
protected:
    
    HudRegion* p_updateRegion;
    float _prevTick;
};
    
}; //end hogboxhud namespace
