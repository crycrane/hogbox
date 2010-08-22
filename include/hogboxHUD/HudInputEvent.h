#pragma once

#include <hogboxHUD/Export.h>

#include <osg/Geode>
#include <osg/MatrixTransform>

#include <iostream>
#include <string>
#include <vector>

namespace hogboxHUD {

enum EVENT_TYPE
{
	CLICK,	//MOUSE CLICK
	MDOWN,	//MOUSE BUTTON DOWN
	MUP,	//MOUSE BUTTON UP
	DRAG,	//DRAG WITH MOUSE BUTTON DOWN
	HOVER,
	PRESS,	//KEY PRESSED
	KDOWN,	//KEYBOARD KEY DOWN
	KUP,
	NEW_MARKER,
	OBJ_CHANGE,
	MARKER_CLICK,
	MARKER_SHAKE
};


class HOGBOXHUD_EXPORT CHudEvent
{
public:

	CHudEvent(void)
	{
		m_mouseCoords=osg::Vec2(0,0);
		m_preMouseCoords=osg::Vec2(0,0); 
		m_mouseChange=osg::Vec2(0,0);
		m_button=0;
		m_key=0;
	}
	~CHudEvent(void){}

	//id of the item the HUDREGION that has recived 
	//a user input
	std::string m_ID;
	EVENT_TYPE m_EventType;

	osg::Vec2 m_mouseCoords;
	osg::Vec2 m_preMouseCoords;
	osg::Vec2 m_mouseChange;

	//window dimensions 
	osg::Vec2 m_winDimensions;
	//hud dimensions
	osg::Vec2 m_hudDimensions;

	//which mouse button is being pressed
	int m_button;
	int m_key;

	void SetEvent(	const std::string id, EVENT_TYPE eT, 
					osg::Vec2 mouseCoords, osg::Vec2 mouseChange, 
					int button, int key,
					osg::Vec2 winDimensions,
					osg::Vec2 hudDimensions)
	{
		m_ID = id;
		m_EventType = eT;

		m_preMouseCoords=m_mouseCoords;
		m_mouseCoords=mouseCoords;
		m_mouseChange=mouseChange;

		m_winDimensions = winDimensions;
		m_hudDimensions = hudDimensions;

		m_key=key;
		m_button=button;
	}


//FUNCTIONS
	std::string GetID(){return m_ID;}
	EVENT_TYPE GetEventType(){return m_EventType;}

	//mouse atts
	osg::Vec2 GetMouseCoords(){return m_mouseCoords;}
	//het mouse coords in hudspace, bottom left being 0,0
	osg::Vec2 GetHudSpaceMouseCoords()
	{
		float sX = m_hudDimensions.x()*(m_mouseCoords.x()/m_winDimensions.x());
		float sY = m_hudDimensions.y()*(m_mouseCoords.y()/m_winDimensions.y());
		return osg::Vec2(sX,sY);
	}

	osg::Vec2 GetMouseChange(){return m_mouseChange;}
	int GetButton(){return m_button;}
	
	//keyboard ats
	int GetKey(){return m_key;}

};

};
