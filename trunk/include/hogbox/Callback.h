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

#include <osg/Referenced>
#include <osg/ref_ptr>

namespace hogbox {

	//
	//Base Callback type
	//
	class Callback : public osg::Referenced
	{
	public:	
		Callback()
            : osg::Referenced()
		{}
		
		virtual void TriggerCallback(void* object) 
		{
			OSG_WARN << "Callback ERROR: No Callback function registered." << std::endl;
		}
		
	protected:	
		
		virtual ~Callback(void){}
	};
	typedef osg::ref_ptr<Callback> CallbackPtr;

	//
	//ObjectCallback
	//Used to register a single function of a class instance as a callback function
	//C the type receiving events
	//
	template <class C> class ObjectCallback : public Callback
	{
	public:	
	
		//
		//The template  function for all classes wanting to receive this callback event
		//can pass only a void pointer to be cast to correct type
		typedef void (C::* ObjectCallbackFunc)(void*);  //get value function definition
								
		//
		//Contructor requires
		//T* the type sending the events
		//C* the object wanting to be calledback when the event occurs
		//HudEventCallbackFunc a member function of rObject that matches our callback template 
		ObjectCallback(C *rObject, ObjectCallbackFunc ghandler = 0)
			: Callback(),
			f_callbackFunc(ghandler),
            mp_object(rObject)
		{
		}
		
		virtual void TriggerCallback(void* object) 
		{
			if (f_callbackFunc) 
			{
				//call our receiver objects callback function
				(mp_object->*f_callbackFunc)(object);			
			} else 
			{
				OSG_WARN << "HudEvent ERROR: No Callback function registered." << std::endl;
			}
		}
		
	protected:	

		virtual ~ObjectCallback(void){}
		
	protected:

		//function pointers to the callback function for this event
		ObjectCallbackFunc f_callbackFunc;
		
		//the pointer to the class instance
		//we are calling the callback of
		C* mp_object;	
	};
	
	
	//
	//CallbackEvent allows an object to register a list of Callbacks 
    //To be triggered when require
	//
	class CallbackEvent : public osg::Referenced 
	{
	public:
		CallbackEvent(const std::string& eventName)
			: osg::Referenced(),
			_eventName(eventName)
		{
			
		}
		
		const std::string& GetName(){return _eventName;}
		
		//Register a new Callback receiver for this event
		void AddCallbackReceiver(Callback* callback)
		{
			_callbacks.push_back(callback);
		}
		
		//
		//
		void Trigger(void* object)
		{
			//call all our callback functions
			for(unsigned int i=0; i<_callbacks.size(); i++)
			{
				_callbacks[i]->TriggerCallback(object);
			}
		}
		
	protected:
		virtual ~CallbackEvent(void){}
		
	protected:

		//event has a name which can be used by user as a simple reference (e.g OnMouseDown)
		std::string _eventName; 
		
		//the list of callbacks registered to receive this event when triggered
		std::vector<CallbackPtr> _callbacks;
			
	};
    typedef osg::ref_ptr<CallbackEvent> CallbackEventPtr;


 };//end hogboxHUD namespace
 