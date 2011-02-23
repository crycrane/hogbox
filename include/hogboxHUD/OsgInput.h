#pragma once

#include <hogboxHUD/Export.h>
#include <hogboxHUD/HudInputEvent.h>
#include <hogboxHUD/HudRegion.h>

#include <osgViewer/Viewer>
#include <osg/observer_ptr>

#include <osgUtil/IntersectVisitor>

#include <osgGA/EventVisitor>
#include <osgGA/GUIEventHandler>


namespace hogboxHUD {


struct UnPickableNode
{
	osg::Node*	_node;
	int			_prevMask;
};

//
//The HudInputHandler handles picking of geodes from the osg graph
//representing HudRegions, the HudInputHandler is also responsible for keeping track of
//the current HudRegion i.e. the region in focus
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
    void pick(const osgGA::GUIEventAdapter& ea, bool hudPick = true); //mode 0 = push, 1=release, 2 = double click
  

	//
	//
	HudInputEvent* GetCurrentEvent()
	{return m_inputState.get();}

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

	//
	//Set a new focus object, this will also inform the previous focus object
	//of the mouseLeave event, and the new object of the mouseEnter event
	void SetFocusRegion(HudRegion* focusRegion);
	
protected:

	//handle to our viewer so we can grab the scene for picking
    osg::observer_ptr<osgViewer::Viewer> _sceneView;
    
	//dimensions of hud
	osg::Vec2 m_hudDimensions;

	//current input event state
	osg::ref_ptr<HudInputEvent> m_inputState;
	
	//the current hud region with focus (need to implement default root region, for now null till mouse click)
	HudRegion* p_focusRegion;

	//store a pointer to the last node to be clicked
	//i.e. might not be a hud region
	osg::ref_ptr<osg::Node>  m_clickObject;

	//the list of unpickable nodes, disabled during picking
	std::vector<UnPickableNode> m_vUnPickableNodes;
};

};