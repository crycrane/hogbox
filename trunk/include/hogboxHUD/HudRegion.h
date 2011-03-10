#pragma once

#include <hogbox/AnimateValue.h>
#include <hogboxHUD/Export.h>
#include <hogbox/HogBoxBase.h>

#include <osg/MatrixTransform>
#include <osg/Material>
#include<osg/Texture2D>

#include <hogboxHUD/HudInputEvent.h>
#include <hogboxHUD/HudEventCallback.h>



namespace hogboxHUD {

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
//forward declare our update callback
class HudRegionUpdateCallback;

//helper func to rename geodes and attach region as user data
extern HOGBOXHUD_EXPORT void MakeHudGeodes(osg::Node* graph, HudRegion* region);

	
//
//Base hud region
//
class HOGBOXHUD_EXPORT HudRegion : public osg::Object
{
public:
	//When constructing a region automatically within the create of a parent region
	//isProcedural should be flaged as true so that when hudregions are saved the
	//system knows not to save out the automaticaly created regions
	HudRegion(bool isProcedural=false);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HudRegion(const HudRegion& region,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogboxHUD,HudRegion);

	typedef std::vector<HudRegionPtr> HudRegionList;

	//load assets, set size and position, then apply the names to any geodes for picking
	virtual bool Create(osg::Vec2 corner, osg::Vec2 size, const std::string& fileName, bool microMemoryMode=false);

	//was the region auto created by a parent (i.e. we don't want to save it to disk)
	bool isProcedural(){return m_isProcedural;}

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
	void SetParent(HudRegion* parent){m_p_parent=parent;}
	HudRegion* GetParent(){return m_p_parent;}

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

	//set the size of this region in hud coords 
	//via the m_scale matrix, any children will be resize using the
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
		m_transformInheritMask = inherit;
	}
	InheritanceMask GetTransformInheritMask(){
		return m_transformInheritMask;
	}

	//visibility
	const bool& IsVisible() const;
	void SetVisible(const bool& visible);

	//set the texture used by default
	void SetBaseTexture(osg::Texture* texture);
	//set the texture used in mouse rollovers
	void SetRolloverTexture(osg::Texture* texture);

	//Apply the texture to the channel 0/diffuse
	void ApplyTexture(osg::Texture* tex);
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


	//Set the regions stateset directly
	void SetStateSet(osg::StateSet* stateSet);
	//return the stateset applied to root
	osg::StateSet* GetStateSet(); 

	//used to flag to the user that something about this region
	//has changed, i.e. the text was altered, a slider was moved etc
	bool StateChanged();

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
	HudRegionList GetChildrenList()const{return m_p_children;}
	void SetChildrenList(const HudRegionList& list);
	
	
	//
	//Animation
	
	//Rotation channel
	template <typename M>
	void AddRotationKey(float degrees, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_animateRotate->GetNumKeys() == 0){m_animateRotate->SetValue(this->GetRotation());}
		m_animateRotate->AddKey<M>(degrees, duration);
	}
	hogbox::AnimateValue<float>::KeyFrame* GetRotationKey(unsigned int index){return m_animateRotate->GetKey(index);}
	bool RemoveRotationKey(unsigned int index){return m_animateRotate->RemoveKey(index);}
	unsigned int GetNumRotationKeys(){return m_animateRotate->GetNumKeys();}
	
	//Position channel
	template <class M>
	void AddPositionKey(osg::Vec2 pos, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_animatePosition->GetNumKeys() == 0){m_animatePosition->SetValue(this->GetPosition());}
		m_animatePosition->AddKey<M>(pos, duration);
	}
	hogbox::AnimateValue<osg::Vec2>::KeyFrame* GetPositionKey(unsigned int index){return m_animatePosition->GetKey(index);}
	bool RemovePositionKey(unsigned int index){return m_animatePosition->RemoveKey(index);}
	unsigned int GetNumPositionKeys(){return m_animatePosition->GetNumKeys();}
	
	//Size channel
	template <typename M>
	void AddSizeKey(osg::Vec2 size, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_animateSize->GetNumKeys() == 0){m_animateSize->SetValue(this->GetSize());}
		m_animateSize->AddKey<M>(size, duration);
	}
	hogbox::AnimateValue<osg::Vec2>::KeyFrame* GetSizeKey(unsigned int index){return m_animateSize->GetKey(index);}
	bool RemoveSizeKey(unsigned int index){return m_animateSize->RemoveKey(index);}
	unsigned int GetNumSizeKeys(){return m_animateSize->GetNumKeys();}
	
	//Color channel
	template <typename M>
	void AddColorKey(osg::Vec3 color, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_animateColor->GetNumKeys() == 0){m_animateColor->SetValue(this->GetColor());}
		m_animateColor->AddKey<M>(color, duration);
	}
	hogbox::AnimateValue<osg::Vec3>::KeyFrame* GetColorKey(unsigned int index){return m_animateColor->GetKey(index);}
	bool RemoveColorKey(unsigned int index){return m_animateColor->RemoveKey(index);}
	unsigned int GetNumColorKeys(){return m_animateColor->GetNumKeys();}
	
	//Alpha channel
	template <typename M>
	void AddAlphaKey(float alpha, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_animateAlpha->GetNumKeys() == 0){m_animateAlpha->SetValue(this->GetAlpha());}
		m_animateAlpha->AddKey<M>(alpha, duration);
	}
	hogbox::AnimateValue<float>::KeyFrame* GetAlphaKey(unsigned int index){return m_animateAlpha->GetKey(index);}
	bool RemoveAlphaKey(unsigned int index){return m_animateAlpha->RemoveKey(index);}
	unsigned int GetNumAlphaKeys(){return m_animateAlpha->GetNumKeys();}
	
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
	void SetAnimationDisabled(bool disable){m_animationDisabled = disable;}
	
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
	//of m_region
	virtual bool UnLoadAssests();

	//used internally to alert others that a value has changed
	void TriggerStateChanged();

protected:

	//a value of true indicates that this region was
	//auto created by a parent (a silder making the inc and dec buttons for example)
	//these type of region do not want to be saved out
	bool m_isProcedural;

	//
	//Folder containing region render assests
	std::string m_assetFolder;
	//tracks if assets have been loaded
	bool m_assestLoaded;

	//main root attached to hud
	osg::ref_ptr<osg::MatrixTransform> m_root;
	//translate attached to root, for hudspace translation (xy)
	osg::ref_ptr<osg::MatrixTransform> m_translate;
	//rotate attached to translate, for rotation around z axis
	osg::ref_ptr<osg::MatrixTransform> m_rotate;
	//scale attached to rotate, scales the region geom to
	//region size. 
	osg::ref_ptr<osg::MatrixTransform> m_scale;

	//child mount attached to rotate, child nodes are resized
	//individually so m_scale matrix does not affect them
	osg::ref_ptr<osg::MatrixTransform> m_childMount;

	//region node attached to scale, contains the actual geometry
	//or drawable representation of the region which should be 1x1
	//m_scale will then make it our desired size
	osg::ref_ptr<osg::Group> m_region; 

	//size in hud coords
	osg::Vec2 m_size;
	//bottom left corner in screen or parent space
	//i.e. if the region is attached as child to another
	//the regions origin becomes the parents bottom left corner
	osg::Vec2 m_corner;
	//rotation in degrees arond the z axis
	float m_rotation;
	//depth/layer of the region i.e. z coord
	float m_depth;

	//the inherit mask, telling us which
	//transform features to inherit from parent
	InheritanceMask m_transformInheritMask;

	//visibility of the region
	bool m_visible;

	//has the region changed since last time
	bool m_stateChanged;
	bool m_hovering; //is it in a hover state

	//standard texture 
	osg::ref_ptr<osg::Texture> m_baseTexture;


	//texture used on rollover
	osg::ref_ptr<osg::Texture> m_rollOverTexture;

	//nodes parent region, null if no parent (should this be shared or weak)
	HudRegion* m_p_parent;

	//regions attached to this one
	HudRegionList m_p_children; 

	//the material applied to the region (should I just use HogBoxMaterial?)
	osg::ref_ptr<osg::StateSet> m_stateset; 
	osg::ref_ptr<osg::Material> m_material;
	osg::Vec3 m_color;
	float m_alpha;
	bool m_alphaEnabled;

	//
	//microMemoryMode=true, indicates that we want to unref our textures image data
	//and we want to full delete the textures when hidden, the textures are reloaded
	//when the region is shown again. Default is false/off
	bool m_microMemoryMode;

	
	
	//
	//Our default update callback to ensure update is called each frame
	osg::ref_ptr<HudRegionUpdateCallback> m_updateCallback;
	
	//
	//Animation for each of our attributes
	hogbox::AnimateFloatPtr m_animateRotate; bool m_isRotating;
	hogbox::AnimateVec2Ptr m_animatePosition; bool m_isTranslating;
	hogbox::AnimateVec2Ptr m_animateSize; bool m_isSizing;
	hogbox::AnimateVec3Ptr m_animateColor; bool m_isColoring;
	hogbox::AnimateFloatPtr m_animateAlpha; bool m_isFading;
	
	//previous framestamp time to calc time elapsed
	float m_prevTick;
	
	//used to stop any animation
	bool m_animationDisabled;
	
	//Callback system
	
	//base hud region handles a few basic hud callbacks
	//OnMouseDown, called when mouse down/press event occurs in the region
	//OnMouseUp, called when mouse up/release event occurs in the region
	//OnMouseMove, called if mouse moves in the regions area
	//OnMouseDrag, same as mouseMove only a mouse button is held down
	//OnMouseEnter, called when mouse first enters the regions area
	//OnMouseLeave, called when mouse leaves the area (input model will currently make this difficult to detect)
	
	//mouse events
	osg::ref_ptr<CallbackEvent> m_onMouseDownEvent;
	osg::ref_ptr<CallbackEvent> m_onMouseUpEvent;
	osg::ref_ptr<CallbackEvent> m_onMouseMoveEvent;
	osg::ref_ptr<CallbackEvent> m_onMouseDragEvent;
	osg::ref_ptr<CallbackEvent> m_onDoubleClickEvent;
	osg::ref_ptr<CallbackEvent> m_onMouseEnterEvent;
	osg::ref_ptr<CallbackEvent> m_onMouseLeaveEvent;
	
	//keyboard events
	osg::ref_ptr<CallbackEvent> m_onKeyDownEvent;
	osg::ref_ptr<CallbackEvent> m_onKeyUpEvent;
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
		m_prevTick(0.0f)
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
			if(m_prevTick==0.0f){m_prevTick = time;}
			float timePassed = time - m_prevTick;
			m_prevTick = time;
			
			p_updateRegion->Update(time);
		}
		osg::NodeCallback::traverse(node,nv);
	}
	
protected:
	
	virtual~HudRegionUpdateCallback(void){}
	
protected:

	HudRegion* p_updateRegion;
	float m_prevTick;
};

}; //end hogboxhud namespace