#pragma once

#include <hogboxStage/Component.h>
#include <hogboxStage/WorldTransformComponent.h>
#include <hogbox/HogboxObject.h>

namespace hogboxStage 
{

//
//RenderableComponent
//A component which uses a hogboxObject as a renderable model
//RenderableComponent is dependant on WorldTransformComponent
//to orient the model
//
class HOGBOXSTAGE_EXPORT RenderableComponent : public Component
{
public:
	RenderableComponent(Entity* parent=NULL);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	RenderableComponent(const RenderableComponent& ent,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: Component(ent, copyop)
	{
		//add callbacks for when we show/hide the renderable
		AddCallbackEventType("OnShow");
		AddCallbackEventType("OnHide");
	}

	META_Box(hogboxStage, RenderableComponent);

	//
	//Our main update function 
	virtual bool OnUpdate(ComponentEvent* eventData){return true;}

	//
	//pure virtual get type name to be implemented by concrete types
	virtual const std::string GetTypeName(){return "RenderableComponent";}

	//Set the renderable object
	void SetRenderableObject(hogbox::HogBoxObject* renderable);

	hogbox::HogBoxObject* GetRenderableObject();

public:

	//
	//Callback triggered when an entities WorldTransformComponent is changed/moved
	void OnEntityMovedCallback(Component* sender, ComponentEventPtr moveEvent);

protected:

	virtual ~RenderableComponent(void);


protected:

	hogbox::HogBoxObjectPtr _renderObject;

};
typedef osg::ref_ptr<RenderableComponent> RenderableComponentPtr;

};