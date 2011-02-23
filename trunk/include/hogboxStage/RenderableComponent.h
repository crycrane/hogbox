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
	RenderableComponent(Entity* parent=NULL)
		: Component(parent)
	{
		//add callbacks for when we show/hide the renderable
		AddCallbackEventType("OnShow");
		AddCallbackEventType("OnHide");

		//check the entity for a worldtransform component, creating one if it doesn't exist
		//we then hook into the on Moved event to update our models transform when a change occurs
		if(p_entity){
			WorldTransformComponent* transComp = p_entity->GetOrCreateComponentOfType<WorldTransformComponent>();
			if(transComp)
			{

		}
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	RenderableComponent(const RenderableComponent& ent,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: Component(ent, copyop)
	{
		//add callbacks for when we show/hide the renderable
		AddCallbackEventType("OnShow");
		AddCallbackEventType("OnHide");
	}

	META_Box(hogboxStage, RenderableComponent);

public:

	//
	//Callback triggered when an entities WorldTransformComponent is changed/moved
	void OnEntityMovedCallback(Component* sender, ComponentEventPtr moveEvent){
		//cast the event to a moved event
		MovedEvent* movedEvent = dynamic_cast<MovedEvent*>(moveEvent.get());
		if(movedEvent){

			//if we have a renderable object then update its transform with the received
			if(_renderObject.get()){
				osg::Vec3 trans = movedEvent->_transform.getTrans();
				osg::Vec3 scale = movedEvent->_transform.getScale();
				osg::Quat rot = movedEvent->_transform.getRotate();
				_renderObject->
			}

		}
	}

protected:

	virtual ~RenderableComponent(void) {

	}

	//
	//Our main update function 
	virtual bool OnUpdate(ComponentEvent* eventData){return true;}


protected:

	HogBoxObjectPtr _renderObject;

};
typedef osg::ref_ptr<RenderableComponent> RenderableComponentPtr;

};