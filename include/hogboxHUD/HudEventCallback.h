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

#include <map>
#include <osg/Referenced>
#include <osg/ref_ptr>

//#include <hogboxHUD/HudRegion.h>
#include <hogbox/Callback.h>
#include <hogboxHUD/HudInputEvent.h>


namespace hogboxHUD {
	
	//
	//Base HudEvent Callback type
	//
	class HudEventCallback : public hogbox::Callback
	{
	public:	
		HudEventCallback()
		: hogbox::Callback()
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
		typedef void (C::* HudEventCallbackFunc)(osg::Object*, HudInputEvent&);  //get value function definition
								
		//
		//Contructor requires
		//T* the type sending the events
		//C* the object wanting to be calledback when the event occurs
		//HudEventCallbackFunc a member function of rObject that matches our callback template 
		HudEventObjectCallback(osg::Object* sender, C *rObject, HudEventCallbackFunc ghandler = 0)
			: HudEventCallback(),
			p_sender(sender),
			f_callbackHudFunc(ghandler),
            mp_object(rObject)
		{
		}
		
		virtual void TriggerCallback(HudInputEvent& inputEvent) 
		{
            OSG_FATAL << "HudEventObjectCallback::TriggerCallback" << std::endl;
			if (f_callbackHudFunc) 
			{
				//call our receiver objects callback function
				(mp_object->*f_callbackHudFunc)(p_sender, inputEvent);			
			} else 
			{
				OSG_FATAL << "HudEvent ERROR: No Callback function registered." << std::endl;
			}
		}
		
	protected:	

		virtual ~HudEventObjectCallback(void){}
		
	protected:
		
		osg::Object* p_sender;

		//function pointers to the callback function for this event
		HudEventCallbackFunc f_callbackHudFunc;
		
		//the pointer to the class instance
		//we are calling the callback of
		C* mp_object;	
	};
	
	
	//
	//CallbackEvent allows a hud region to register a new type of event for which external objects can register
	//HudEventCallbacKs to be called when the event type is triggered
	//
	class HudCallbackEvent : public hogbox::CallbackEvent
	{
	public:
		HudCallbackEvent(osg::Object* sender, const std::string& eventName)
			: hogbox::CallbackEvent(eventName),
			p_eventSender(sender)
		{
			
		}
		
		//
		//
		void Trigger(HudInputEvent& inputEvent)
		{
            OSG_FATAL << "HudCallbackEvent::Trigger, num callbacks '" << _callbacks.size() << "'." << std::endl;
			//call all our callback functions
			for(unsigned int i=0; i<_callbacks.size(); i++)
			{
                //cast to HudEventCallback
                HudEventCallback* asHudCallback = dynamic_cast<HudEventCallback*>(_callbacks[i].get());
                if(asHudCallback){
                    asHudCallback->TriggerCallback(inputEvent);
                }
			}
		}
		
	protected:
		virtual ~HudCallbackEvent(void){}
		
	protected:
		
		//the object sending the event (e.g. hudregion, button)
		osg::Object* p_eventSender;
    
			
	};


 };//end hogboxHUD namespace
 