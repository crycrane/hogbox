#pragma once

#include <map>
#include <osg/Referenced>
#include <osg/ref_ptr>

//#include <hogboxHUD/HudRegion.h>
#include <hogboxHUD/HudInputEvent.h>


namespace hogboxHUD {

	class HudRegion;
	
	//
	//Base HudEvent Callback type
	//
	class HudEventCallback : public osg::Referenced
	{
	public:	
		HudEventCallback()
		: osg::Referenced()
		{}
		
		virtual void TriggerCallback( HudInputEvent& inputEvent) 
		{
			osg::notify(osg::WARN) << "HudEvent ERROR: No Callback function registered." << std::endl;
		}
		
	protected:	
		
		virtual ~HudEventCallback(void){}
	};
	typedef osg::ref_ptr<HudEventCallback> HudEventCallbackPtr;

	//
	//HudEventObjectCallback
	//Used to register a single function of a class instance as a callback function
	//for hudinputevents
	//T the type sending events, the hudregions etc
	//C the type receiving events, the app layer and parent regions etc
	//
	template <class C> class HudEventObjectCallback : public HudEventCallback
	{
	public:	
	
		//
		//The template  function for all classes wanting to receive this callback event
		//takes as arguments
		//HudRegion the sender of the events
		//HudInputEvent& the intput event that triggered this hudevent
		typedef void (C::* HudEventCallbackFunc)(HudRegion*, HudInputEvent&);  //get value function definition
								
		//
		//Contructor requires
		//T* the type sending the events
		//C* the object wanting to be calledback when the event occurs
		//HudEventCallbackFunc a member function of rObject that matches our callback template 
		HudEventObjectCallback(HudRegion* sender, C *rObject, HudEventCallbackFunc ghandler = 0)
			: HudEventCallback(),
			p_sender(sender),
			mp_object(rObject),
			f_callbackFunc(ghandler)
		{
		}
		
		virtual void TriggerCallback(HudInputEvent& inputEvent) 
		{
			if (f_callbackFunc) 
			{
				//call our receiver objects callback function
				(mp_object->*f_callbackFunc)(p_sender, inputEvent);			
			} else 
			{
				osg::notify(osg::WARN) << "HudEvent ERROR: No Callback function registered." << std::endl;
			}
		}
		
	protected:	

		virtual ~HudEventObjectCallback(void){}
		
	protected:
		
		HudRegion* p_sender;

		//function pointers to the callback function for this event
		HudEventCallbackFunc f_callbackFunc;
		
		//the pointer to the class instance
		//we are calling the callback of
		C* mp_object;	
	};
	
	
	//
	//CallbackEvent allows a hud region to register a new type of event for which external objects can register
	//HudEventCallbacKs to be called when the event type is triggered
	//
	class CallbackEvent : public osg::Referenced 
	{
	public:
		CallbackEvent(HudRegion* sender, const std::string& eventName)
			: osg::Referenced(),
			p_eventSender(sender),
			m_eventName(eventName)
		{
			
		}
		
		const std::string& GetEventName(){return m_eventName;}
		
		//Register a new Callback receiver for this event
		void AddCallbackReceiver(HudEventCallback* callback)
		{
			m_callbacks.push_back(callback);
		}
		
		//
		//
		void TriggerEvent(HudInputEvent& inputEvent)
		{
			//call all our callback functions
			for(unsigned int i=0; i<m_callbacks.size(); i++)
			{
				m_callbacks[i]->TriggerCallback(inputEvent);
			}
		}
		
	protected:
		virtual ~CallbackEvent(void){}
		
	protected:
		
		//event has a name which can be used by user as a simple reference (e.g OnMouseDown)
		std::string m_eventName; 
		
		//the object sending the event (e.g. hudregion, button)
		HudRegion* p_eventSender;
		
		//the list of callbacks registered to receive this event when triggered
		std::vector<HudEventCallbackPtr> m_callbacks;
			
	};


 };//end hogboxHUD namespace
 