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

	typedef std::map<std::string, bool> ComponentDependencyMap;
	typedef std::pair<std::string, bool> ComponentDependencyPair;

	Component();

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	Component(const Component& ent,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogboxStage, Component);

	//
	//Called when a component is added to an entity
	virtual bool OnAttach(Entity* parent){
		p_entity = parent;
		return true;
	}


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
	bool RegisterCallbackForEvent(ComponentEventCallback* callback, const std::string& eventName);

	//
	//returns the index of the callback if it exists else -1
	int GetCallbackEventIndex(const std::string& eventName);

	//
	//Returns true if this component depends on the passed type
	bool DependsOnType(const std::string& typeName);

	//
	//When a new component is attached to our parent component all components are checked
	//for un resolved dependancies. If this component depends on the newly added component type
	//then it is passed to this components HandleComponentDependency function
	virtual bool HandleComponentDependency(Component* component){
		return true;
	}

protected:

	virtual ~Component(void);
	//
	//Adds a new ComponentCallbackEvent to our list ensuring the name is unique, this should be used
	//during a types constuctor to register any new event types, returns false if name already used
	bool AddCallbackEventType(const std::string& eventName);

	//
	//Trigger an event callback
	bool TriggerEventCallback(const std::string& eventName, ComponentEventPtr entityEvent);

	//add a dependency on a component type
	void AddComponentDependency(const std::string& typeName);

protected:

	//pointer to the parent/owning entity
	Entity* p_entity;

	//the list of callback events this etity has registered
	std::vector<ComponentCallbackEventPtr> _callbackEvents;

	//Components can register dependency on other components
	//have all dependencies been resolved
	bool _dependsResolved;
	//map of component type name to a bool indicating if the dependency has been handled
	ComponentDependencyMap _dependComponents;

};
typedef osg::ref_ptr<Component> ComponentPtr;
typedef std::vector<ComponentPtr> ComponentPtrVector;

};