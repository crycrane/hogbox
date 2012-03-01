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

#include <hogboxHUD/Export.h>
#include <hogbox/HogBoxBase.h>
#include <osgGA/GUIEventHandler>

#include <deque>

namespace hogboxHUD {

//
//Enums for our basic hud input event types, 
//these are mainly the osgGA event types, plus the extra
//hud specific events like MouseEnter, MouseLeave etc

enum HudEventType
{
	//mouse events
	ON_MOUSE_DOWN = 1,
	ON_MOUSE_UP = 2,
	ON_MOUSE_MOVE = 4,
	ON_MOUSE_DRAG = 8,
	ON_DOUBLE_CLICK = 16,
	ON_MOUSE_ENTER = 32,
	ON_MOUSE_LEAVE = 64,
	//key event
	ON_KEY_DOWN = 128,
	ON_KEY_UP = 256
};
	
//
//A HudInputEvent contains the input evnet type, osgGA input state
//as well as the state of the HogBoxHud/Window etc
//Can also be used to track the state over time to get mouse velocities etc
//
class HOGBOXHUD_EXPORT HudInputEvent : public osg::Object
{
public:

	HudInputEvent(void) 
		: osg::Object(),
		_osgInputEvent(new osgGA::GUIEventAdapter()),
        _button(0),
        _key(0),
		_mouseCoords(osg::Vec2(0,0)),
		_preMouseCoords(osg::Vec2(0,0)), 
		_mouseChange(osg::Vec2(0,0)),
		_prevTick(0.0f),
		_timePassed(0.0f)
	{
	}
	
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HudInputEvent(const HudInputEvent& event,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(event, copyop),
		_eventType(event._eventType),
        _button(event._button),
        _key(event._key),
        _vPressedKeys(event._vPressedKeys),
        _vHeldKeys(event._vHeldKeys),
		_mouseCoords(event._mouseCoords),
		_preMouseCoords(event._preMouseCoords), 
		_mouseChange(event._mouseChange),
        _winDimensions(event._winDimensions),
        _hudDimensions(event._hudDimensions),
		_prevTick(event._prevTick),
		_timePassed(event._timePassed)
	{
		//if(event.GetInputState()){
		_osgInputEvent = new osgGA::GUIEventAdapter();// event._osgInputEvent->clone(copyop);
		//}
	}
	
	META_Object(hogboxHUD,HudInputEvent);

	//
	//Set the current state from the osgInput system
	//
	void SetEvent(HudEventType type, const osgGA::GUIEventAdapter& osgInputEvent, osg::Vec2 hudSize)
	{
		_eventType = type;
		
		_osgInputEvent = NULL;
		_osgInputEvent = new osgGA::GUIEventAdapter(osgInputEvent, osg::CopyOp::DEEP_COPY_ALL);
		
		_button = osgInputEvent.getButton();
		_key = osgInputEvent.getKey();
		
		//tack previous states
		_preMouseCoords = _mouseCoords;
		
		//set mouse coords
		_mouseCoords = osg::Vec2(osgInputEvent.getX(),osgInputEvent.getY());
		_mouseChange = _mouseCoords - _preMouseCoords;

		//store window dimensions
		_winDimensions = osg::Vec2(osgInputEvent.getWindowWidth(), osgInputEvent.getWindowHeight());
		_hudDimensions = hudSize;
		
		//store time and calc time passed since last event
		if(_prevTick == 0.0f){_prevTick=osgInputEvent.getTime();}
		_timePassed = osgInputEvent.getTime() - _prevTick;
		_prevTick = osgInputEvent.getTime();
	}
	
	//get the event type
	const HudEventType& GetEventType(){return _eventType;}
	void SetEventType(HudEventType type){_eventType = type;}
	
	
	//get the actual osgGA event that set this state
	osgGA::GUIEventAdapter* GetInputState(){return _osgInputEvent.get();}

	//keyboard
	
	//
	//return the list of currently held keys
	std::deque<int> GetHeldKeys()
	{return _vHeldKeys;}
	
	//
	//if the key is held return index of key, else return -1
	int IsKeyHeld(int key, bool checkCaps = false)
	{
		for(unsigned int i=0; i<_vHeldKeys.size(); i++)
		{
			if(_vHeldKeys[i] == key)
			{return i;}
		}
		//also check for upper/lower case keys
		if(checkCaps)
		{
			//looking for A-Z
			if( (key >= (int)65) && (key <= (int)90) )
			{
				//try lower
				int lowerKey = key + 32;
				return IsKeyHeld(lowerKey);
			}else if( (key >= 97) && (key<= 122))//looking a-z
			{
				//try upper
				int upperKey = key - 32;
				return IsKeyHeld(upperKey);
			}else{
				
			}
		}
		return-1;
	}
	
	//
	// Press down a held key, adding to the list if it doesn't already exist
	// returns true if is a new press
	bool PressHeldKey(int key)
	{
		int index =  IsKeyHeld(key);
		if(index>=0)
		{return false;}
		
		_vHeldKeys.push_back(key);
		return true;
	}
	
	//
	//removes the key from the held list if it exists
	bool ReleaseHeldKey(int key)
	{
		int index = IsKeyHeld(key);
		if(index>=0)
		{
			_vHeldKeys.erase( _vHeldKeys.begin()+index);
			return true;
		}
		return false;
	}
	
	void PressKey(int key)
	{_vPressedKeys.push_back(key);}
		
	
	//
	//simple pressed keys
	//check if a specific key is pressed
	bool IsKeyPressed(int key)
	{
		for(int i=0; i<(int)_vPressedKeys.size();i++)
		{
			if(_vPressedKeys[i] == key){return true;}
		}
		return false;
	}
	
	//
	//reset the pressed keys at the end of a frame
	void ResetPressedKeys()
	{_vPressedKeys.clear(); }
	
	//
	//return the list of currently pressed keys
	std::vector<int> GetPressedKeys()
	{return _vPressedKeys;}
	
	
	//get event button/key
	int GetButton(){return _button;}
	int GetKey(){return _key;}

	//mouse atts
	osg::Vec2 GetMouseCoords(){return _mouseCoords;}
	//get mouse coords in hudspace, bottom left being 0,0
	osg::Vec2 GetHudSpaceMouseCoords()
	{
		float sX = _hudDimensions.x()*(_mouseCoords.x()/_winDimensions.x());
		float sY = _hudDimensions.y()*(_mouseCoords.y()/_winDimensions.y());
		return osg::Vec2(sX,sY);
	}

	//Vector between this and the previous mouse coord
	osg::Vec2 GetMouseChange(){return _mouseChange;}
	
	//get the mouse movement as velocity vector
	osg::Vec2 GetMouseVelocityVec(){return GetMouseChange()*GetTimePassed();}
	
	//get mouse veocity as a speed/magnitude
	float GetMouseVelocity(){return GetMouseVelocityVec().length();}
	
	
	//the current sim time
	double GetTime(){
		if(_osgInputEvent.get()){return _osgInputEvent->getTime();}
		return 1.0f;
	}
	
	//get the time passed since the last event
	double GetTimePassed(){return _timePassed;}
	
	
protected:
	
	virtual ~HudInputEvent(void){}
	
protected:
	
	//the hogboxHUD input event type
	HudEventType _eventType;
	
	//the last received input event from osgGA
	osg::ref_ptr<osgGA::GUIEventAdapter> _osgInputEvent;
	
	//which mouse button is being pressed
	int _button;
	int _key;
	
	//list of keys press, lasts a single frame, before they are reset
	std::vector<int> _vPressedKeys;
	
	//list of key being held down, key will remain in list
	//until it is released
	std::deque<int> _vHeldKeys;
	
	osg::Vec2 _mouseCoords;
	osg::Vec2 _preMouseCoords;
	osg::Vec2 _mouseChange;
	
	//window dimensions 
	osg::Vec2 _winDimensions;
	//hud dimensions
	osg::Vec2 _hudDimensions;
	
	//time 
	
	float _prevTick;
	float _timePassed;


};

};
