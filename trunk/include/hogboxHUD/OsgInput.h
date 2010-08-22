#pragma once

#include <hogboxHUD/Export.h>
#include <hogboxHUD/HudInputEvent.h>

#include <osgViewer/Viewer>

#include <osg/Timer>
#include <osg/io_utils>
#include <osg/observer_ptr>

#include <osgUtil/IntersectVisitor>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/EventVisitor>
#include <osgGA/GUIEventHandler>

#include <deque>

namespace hogboxHUD {


struct UnPickableNode
{
	osg::Node*	_node;
	int			_prevMask;
};

//
//Class to handle keyboard and mouse events
//then pass them down to the basic hud
//
class HOGBOXHUD_EXPORT HudInputHandler : public osgGA::GUIEventHandler 
{
public: 

	HudInputHandler(osgViewer::Viewer* sceneView, osg::Vec2 hudDimensions);
    ~HudInputHandler();

	//
	//override the GUIEventHandler handle function to process user input
    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);

	//
	//Pick items using intersection visitor
	//mouse coords stored in ea,
	//mode is mouse click mode 0 MUP, 1 MDOWN, 2 HOVER
    void pick(const osgGA::GUIEventAdapter& ea, int mode, bool hudPick = true); //mode 0 = push, 1=release, 2 = double click
  
	//
	//Helper to pass events down to the basic hud
	bool PushBasicHudEvent(int osgType, std::string nodeName, osg::Vec2 mCoords, 
							osg::Vec2 mChange, int mButton, int key,
							const osgGA::GUIEventAdapter& ea);

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

	//
	//
	hogboxHUD::EVENT_TYPE GetEventType()
	{return m_currentEventType;}

	//
	//normal mesh picked (i,e, model not part of the basic hud)
	osg::Node* GetClickedObject(){
		return m_clickObject.get();
	}
	//
	//reset the clicked object pointer
	void ResetClickedObject(){
		m_clickObject = NULL;
	}

	//mouse
	
	//
	//return the last known mouse button pressed
	int mbutton(){return _mbutton;}
	
	//
	//return last known mouse coords
	osg::Vec2 GetMouseCoords(){return osg::Vec2(_mx,_my);}



	//unpickable nodes
	//we need to switch off some nodes when picking so they dont get
	//in the way of our hud, (should be replaced by node masks

	//
	//add a node to be switched off during picking
	void AddUnPickableNode(osg::Node* node)
	{
		UnPickableNode unPickableNode;
		unPickableNode._node = node;
		unPickableNode._prevMask = node->getNodeMask();
		m_vUnPickableNodes.push_back(unPickableNode);
	}

	//
	//remove all nodes from the unpickable list
	void RemoveUnPickableNodes()
	{
		m_vUnPickableNodes.clear();
	}


protected:

	//handle to our viewer so we can grab the scene for picking
    osg::observer_ptr<osgViewer::Viewer> _sceneView;
    
	//dimensions of hud
	osg::Vec2 m_hudDimensions;

	//mouse coords
    float _mx,_my;
	//mouse button
	unsigned int _mbutton;

	//list of keys press, lasts a single frame, before they are reset
	std::vector<int> m_vPressedKeys;

	//list of key being held down, key will remain in list
	//until it is released
	std::deque<int> m_vHeldKeys;

	//the current event type
	EVENT_TYPE m_currentEventType;

	//store a pointer to the last normal node to be clicked
	//i.e. not a basic hud region
	osg::ref_ptr<osg::Node>  m_clickObject;

	//is ctrl key being held
	bool m_ctrl;

	//the list of unpickable nodes, disabled during picking
	std::vector<UnPickableNode> m_vUnPickableNodes;

	//indicates a request to exit app
	bool _done;
};

};