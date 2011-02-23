#pragma once

#include <map>
#include <osg/Referenced>
#include <osg/ref_ptr>

#include <hogboxStage/ComponentEvent.h>

//#include <hogboxStage/Emtity.h>
//#include <hogboxHUD/HudInputEvent.h>


namespace hogboxStage {

	class Component;
	
	//
	//Base Component Callback type
	//
	class ComponentEventCallback : public osg::Referenced
	{
	public:	
		ComponentEventCallback()
		: osg::Referenced()
		{}
		
		virtual void TriggerCallback( ComponentEventPtr entityEvent) 
		{
			OSG_WARN << "ComponentEventCallback ERROR: No Callback function registered." << std::endl;
		}
		
	protected:	
		
		virtual ~ComponentEventCallback(void){}
	};
	typedef osg::ref_ptr<ComponentEventCallback> ComponentEventCallbackPtr;

	//
	//ComponentEventObjectCallback
	//Used to register a single function of a class instance as a callback function
	//for ComponentEvents
	//C the type receiving events, the app layer and parent regions etc
	//
	template <class C> class ComponentEventObjectCallback : public ComponentEventCallback
	{
	public:	
	
		//
		//The template  function for all classes wanting to receive this callback event
		//takes as arguments
		//Component the sender of the events
		//ComponentEventPtr the intput event that triggered this event
		typedef void (C::* ComponentEventCallbackFunc)(Component*, ComponentEventPtr);  //get value function definition
								
		//
		//Contructor requires
		//T* the type sending the events
		//C* the object wanting to be calledback when the event occurs
		//HudEventCallbackFunc a member function of rObject that matches our callback template 
		ComponentEventObjectCallback(Component* sender, C *rObject, ComponentEventCallbackFunc ghandler = 0)
			: ComponentEventCallback(),
			p_sender(sender),
			mp_object(rObject),
			f_callbackFunc(ghandler)
		{
		}
		
		virtual void TriggerCallback(ComponentEventPtr* entityEvent) 
		{
			if (f_callbackFunc) 
			{
				//call our receiver objects callback function
				(mp_object->*f_callbackFunc)(p_sender, entityEvent);			
			} else 
			{
				OSG_WARN << "ComponentEventObjectCallback ERROR: No Callback function registered." << std::endl;
			}
		}
		
	protected:	

		virtual ~ComponentEventObjectCallback(void){}
		
	protected:
		
		Component* p_sender;

		//function pointers to the callback function for this event
		ComponentEventCallbackFunc f_callbackFunc;
		
		//the pointer to the class instance
		//we are calling the callback of
		C* mp_object;	
	};
	
	
	//
	//ComponentCallbackEvent allows a hud region to register a new type of event for which external objects can register
	//ComponentEventCallbacks to be called when the event type is triggered
	//
	class ComponentCallbackEvent : public osg::Referenced 
	{
	public:
		ComponentCallbackEvent(Component* sender, const std::string& eventName)
			: osg::Referenced(),
			p_eventSender(sender),
			m_eventName(eventName)
		{
			
		}
		
		const std::string& GetEventName(){return m_eventName;}
		
		//Register a new Callback receiver for this event
		bool AddCallbackReceiver(ComponentEventCallback* callback)
		{
			if(!callback){return false;}
			m_callbacks.push_back(callback);
			return true;
		}
		
		//
		//
		void TriggerCallback(ComponentEventPtr entityEvent)
		{
			//call all our callback functions
			for(unsigned int i=0; i<m_callbacks.size(); i++)
			{
				m_callbacks[i]->TriggerCallback(entityEvent);
			}
		}
		
	protected:
		virtual ~ComponentCallbackEvent(void){}
		
	protected:
		
		//the object sending the event (e.g. hudregion, button)
		Component* p_eventSender;
		
		//event has a name which can be used by user as a simple reference (e.g OnMouseDown)
		std::string m_eventName; 
		
		//the list of callbacks registered to receive this event when triggered
		std::vector<ComponentEventCallbackPtr> m_callbacks;
			
	};
	typedef osg::ref_ptr<ComponentCallbackEvent> ComponentCallbackEventPtr;


 };//end hogboxApp namespace
 