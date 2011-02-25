#pragma once

#include <hogboxStage/Export.h>
#include <hogbox/HogBoxBase.h>
#include <hogboxStage/ComponentEventCallback.h>

namespace hogboxStage 
{

//forward declare entity
class Entity;

//
//Component
//Base class for any components that can be used by an Component
//
class HOGBOXSTAGE_EXPORT Component : public osg::Object
{
public:
	Component(Entity* parent=NULL)
		: osg::Object(),
		p_entity(parent)
	{
		//add the on desturct message
		AddCallbackEventType("OnDestruct");
		//AddCallbackEventType("OnUpdate");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	Component(const Component& ent,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(ent, copyop)
	{
		//add the on desturct message
		AddCallbackEventType("OnDestruct");
		//AddCallbackEventType("OnUpdate");
	}

	META_Box(hogboxStage, Component);


	//
	//Our main update function to be implemented by all components
	virtual bool OnUpdate(ComponentEventPtr eventData){return true;}

	//
	//pure virtual get type name to be implemented by concrete types
	virtual const std::string GetTypeName(){return "BaseComponent";}

public:
	//callback system so other components/objects can be informed when certain things
	//occur within a component.

	//
	//register a callback to one of our events, returns false if the event does not exist
	bool RegisterCallbackForEvent(ComponentEventCallback* callback, const std::string& eventName){
		if(!callback){return false;}
		int eventIndex = GetCallbackEventIndex(eventName);
		if(eventIndex == -1){return false;}

		if(!_callbackEvents[eventIndex]->AddCallbackReceiver(callback)){
			return false;
		}
		return true;
	}

	//
	//returns the index of the callback if it exists else -1
	int GetCallbackEventIndex(const std::string& eventName){
		for(unsigned int i=0; i<_callbackEvents.size(); i++){
			if(_callbackEvents[i]->GetEventName() == eventName){
				return (int)i;
			}
		}
		return -1;
	}

protected:

	virtual ~Component(void) {
		//inform callback receivers of destuct (TEST)
		TriggerEventCallback("OnDestruct", NULL);
	}

	//
	//Adds a new ComponentCallbackEvent to our list ensuring the name is unique, this should be used
	//during a types constuctor to register any new event types, returns false if name already used
	bool AddCallbackEventType(const std::string& eventName){
		int existingIndex = GetCallbackEventIndex(eventName);
		if(existingIndex == -1){return false;}
		_callbackEvents.push_back(new ComponentCallbackEvent(this, eventName));
		return true;
	}

	//
	//Trigger an event callback
	bool TriggerEventCallback(const std::string& eventName, ComponentEventPtr entityEvent){
		int eventIndex = GetCallbackEventIndex(eventName);
		if(eventIndex == -1){return false;}
		_callbackEvents[eventIndex]->TriggerCallback(entityEvent);
		return true;
	}

protected:

	//pointer to the parent/owning entity
	Entity* p_entity;

	//the list of callback events this etity has registered
	std::vector<ComponentCallbackEventPtr> _callbackEvents;

};
typedef osg::ref_ptr<Component> ComponentPtr;

};