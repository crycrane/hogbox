#pragma once

#include <hogboxHUD/Export.h>
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
class HOGBOXHUD_EXPORT HudInputEvent
{
public:

	HudInputEvent(void)
	{
		m_osgInputEvent = NULL;
		m_mouseCoords=osg::Vec2(0,0);
		m_preMouseCoords=osg::Vec2(0,0); 
		m_mouseChange=osg::Vec2(0,0);
		m_button=0;
		m_key=0;
	}
	~HudInputEvent(void){}

	//
	//Set the current state from the osgInput system
	//
	void SetEvent(HudEventType type, const osgGA::GUIEventAdapter& osgInputEvent, osg::Vec2 hudSize)
	{
		m_eventType = type;
		
		m_osgInputEvent = NULL;
		m_osgInputEvent = new osgGA::GUIEventAdapter(osgInputEvent);
		
		m_button = osgInputEvent.getButton();
		m_key = osgInputEvent.getKey();
		
		//tack previous states
		m_preMouseCoords = m_mouseCoords;
		
		//set mouse coords
		m_mouseCoords = osg::Vec2(osgInputEvent.getX(),osgInputEvent.getY());
		m_mouseChange = m_mouseCoords - m_preMouseCoords;

		//store window dimensions
		m_winDimensions = osg::Vec2(osgInputEvent.getWindowWidth(), osgInputEvent.getWindowHeight());
		m_hudDimensions = hudSize;
	}
	
	//get the event type
	HudEventType GetEventType(){return m_eventType;}
	
	//get the actual osgGA event that set this state
	osgGA::GUIEventAdapter* GetInputState(){return m_osgInputEvent.get();}

	//keyboard
	
	//
	//return the list of currently held keys
	std::deque<int> GetHeldKeys()
	{return m_vHeldKeys;}
	
	//
	//if the key is held return index of key, else return -1
	int IsKeyHeld(int key, bool checkCaps = false)
	{
		for(unsigned int i=0; i<m_vHeldKeys.size(); i++)
		{
			if(m_vHeldKeys[i] == key)
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
		
		m_vHeldKeys.push_back(key);
		return true;
	}
	
	//
	//removes the key from the held list if it exists
	bool ReleaseHeldKey(int key)
	{
		int index = IsKeyHeld(key);
		if(index>=0)
		{
			m_vHeldKeys.erase( m_vHeldKeys.begin()+index);
			return true;
		}
		return false;
	}
	
	void PressKey(int key)
	{m_vPressedKeys.push_back(key);}
		
	
	//
	//simple pressed keys
	//check if a specific key is pressed
	bool IsKeyPressed(int key)
	{
		for(int i=0; i<(int)m_vPressedKeys.size();i++)
		{
			if(m_vPressedKeys[i] == key){return true;}
		}
		return false;
	}
	
	//
	//reset the pressed keys at the end of a frame
	void ResetPressedKeys()
	{m_vPressedKeys.clear(); }
	
	//
	//return the list of currently pressed keys
	std::vector<int> GetPressedKeys()
	{return m_vPressedKeys;}
	
	
	//get event button/key
	int GetButton(){return m_button;}
	int GetKey(){return m_key;}

	//mouse atts
	osg::Vec2 GetMouseCoords(){return m_mouseCoords;}
	//get mouse coords in hudspace, bottom left being 0,0
	osg::Vec2 GetHudSpaceMouseCoords()
	{
		float sX = m_hudDimensions.x()*(m_mouseCoords.x()/m_winDimensions.x());
		float sY = m_hudDimensions.y()*(m_mouseCoords.y()/m_winDimensions.y());
		return osg::Vec2(sX,sY);
	}

	osg::Vec2 GetMouseChange(){return m_mouseChange;}
	
	
protected:
	
	//the hogboxHUD input event type
	HudEventType m_eventType;
	
	//the last received input event from osgGA
	osg::ref_ptr<osgGA::GUIEventAdapter> m_osgInputEvent;
	
	//which mouse button is being pressed
	int m_button;
	int m_key;
	
	//list of keys press, lasts a single frame, before they are reset
	std::vector<int> m_vPressedKeys;
	
	//list of key being held down, key will remain in list
	//until it is released
	std::deque<int> m_vHeldKeys;
	
	osg::Vec2 m_mouseCoords;
	osg::Vec2 m_preMouseCoords;
	osg::Vec2 m_mouseChange;
	
	//window dimensions 
	osg::Vec2 m_winDimensions;
	//hud dimensions
	osg::Vec2 m_hudDimensions;

};

};
