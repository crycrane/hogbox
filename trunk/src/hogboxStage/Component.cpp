#include <hogboxStage/Component.h>

using namespace hogboxStage;

Component::Component()
	: osg::Object(),
	p_entity(NULL),
	_dependsResolved(false)
{
	//add the on desturct message
	AddCallbackEventType("OnDestruct");
	//AddCallbackEventType("OnUpdate");
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
Component::Component(const Component& ent,const osg::CopyOp& copyop)
	: osg::Object(ent, copyop)
{
	//add the on desturct message
	AddCallbackEventType("OnDestruct");
	//AddCallbackEventType("OnUpdate");
}

Component::~Component(void) 
{
	//inform callback receivers of destuct (TEST)
	TriggerEventCallback("OnDestruct", NULL);
}

//
//register a callback to one of our events, returns false if the event does not exist
//
bool Component::RegisterCallbackForEvent(ComponentEventCallback* callback, const std::string& eventName)
{
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
//
int Component::GetCallbackEventIndex(const std::string& eventName)
{
	for(unsigned int i=0; i<_callbackEvents.size(); i++){
		if(_callbackEvents[i]->GetEventName() == eventName){
			return (int)i;
		}
	}
	return -1;
}

//
//Returns true if this component depends on the passed type
//
bool Component::DependsOnType(const std::string& typeName)
{
	ComponentDependencyMap::iterator itr = _dependComponents.find(typeName);
	if(itr == _dependComponents.end()){return false;}
	return true;
}



//
//Adds a new ComponentCallbackEvent to our list ensuring the name is unique, this should be used
//during a types constuctor to register any new event types, returns false if name already used
//
bool Component::AddCallbackEventType(const std::string& eventName)
{
	int existingIndex = GetCallbackEventIndex(eventName);
	if(existingIndex == -1){return false;}
	_callbackEvents.push_back(new ComponentCallbackEvent(this, eventName));
	return true;
}

//
//Trigger an event callback
//
bool Component::TriggerEventCallback(const std::string& eventName, ComponentEventPtr entityEvent)
{
	int eventIndex = GetCallbackEventIndex(eventName);
	if(eventIndex == -1){return false;}
	_callbackEvents[eventIndex]->TriggerCallback(entityEvent);
	return true;
}

//
//add a dependency on a component type
//
void Component::AddComponentDependency(const std::string& typeName)
{
	//check if its already listed
	ComponentDependencyMap::iterator itr = _dependComponents.find(typeName);
	if(itr != _dependComponents.end()){return;}
	_dependComponents.insert(ComponentDependencyPair(typeName, false));
}