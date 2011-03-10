
#include <hogboxStage/RenderableComponent.h>
#include <hogboxStage/Entity.h>
using namespace hogboxStage;


RenderableComponent::RenderableComponent()
	: Component(),
	_renderObject(NULL)
{
	//add callbacks for when we show/hide the renderable
	AddCallbackEventType("OnShow");
	AddCallbackEventType("OnHide");

	//this depends on WorldTransformComponent
	AddComponentDependency("WorldTransformComponent");
}

RenderableComponent::~RenderableComponent(void) 
{
}

//
//Called when a component is added to an entity
//
bool RenderableComponent::OnAttach(Entity* parent)
{
	Component::OnAttach(parent);

	return true;
}

//
//When a new component is attached to our parent component all components are checked
//for un resolved dependancies. If this component depends on the newly added component type
//then it is passed to this components HandleComponentDependency function
//
bool RenderableComponent::HandleComponentDependency(Component* component)
{
	//try to cast to WorldTransformComponent
	WorldTransformComponent* transComp = dynamic_cast<WorldTransformComponent*>(component);
	if(transComp){
		transComp->RegisterCallbackForEvent(new ComponentEventObjectCallback<RenderableComponent>(this,this,
																&RenderableComponent::OnEntityMovedCallback),
																"OnMoved");
		return true;
	}
	
	return false;
}

//
//Set the renderable object
//
void RenderableComponent::SetRenderableObject(hogbox::HogBoxObject* renderable)
{
	_renderObject=NULL;
	_renderObject=renderable;
}

hogbox::HogBoxObject* RenderableComponent::GetRenderableObject()
{
	return _renderObject.get();
}

//
//Callback triggered when an entities WorldTransformComponent is changed/moved
//
void RenderableComponent::OnEntityMovedCallback(Component* sender, ComponentEventPtr moveEvent)
{
	//cast the event to a moved event
	MovedEvent* movedEvent = dynamic_cast<MovedEvent*>(moveEvent.get());
	if(movedEvent){
		//if we have a renderable object then update its transform with the received transform
		if(_renderObject.get()){
			_renderObject->SetWorldTransform(movedEvent->_transform);
		}
	}
}

