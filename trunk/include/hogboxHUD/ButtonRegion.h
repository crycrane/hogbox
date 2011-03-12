#pragma once

#include <hogboxHUD/TextRegion.h>

namespace hogboxHUD {

//
//Button region, is just a text region that responds to
// click events by flagging its isPressed flag
//
class HOGBOXHUD_EXPORT ButtonRegion : public TextRegion
{
public:
	ButtonRegion(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	ButtonRegion(const ButtonRegion& region,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogboxHUD,ButtonRegion);

	//
	//Create a region to behave like a basic button
	virtual bool Create(osg::Vec2 corner, osg::Vec2 size, const std::string& fileName, const std::string& label);

	//
	//Detects mouse down and flags is pressed state,
	//also swaps to the mouse down texture if one is present
	virtual int HandleInputEvent(HudInputEvent& hudEvent);

	//
	//Button region loads the aditional assests
	//mouseDown.png, used when the mouse is pressed down on the region
	virtual bool LoadAssest(const std::string& folderName);
	
	//
	//Funcs to register event callbacks
	void AddOnButtonClickedCallbackReceiver(HudEventCallback* callback);
	
public:
	
	//these realy need to be protected, but the callback system wil need to be made a friend of this?
	//receive the base callbacks to detect our button click event
	void OnMouseDown(osg::Object* sender, hogboxHUD::HudInputEvent& inputEvent);
	void OnMouseUp(osg::Object* sender, hogboxHUD::HudInputEvent& inputEvent);

	//detect for rollover and to disable a mouseDown when focus is lost
	void OnMouseEnter(osg::Object* sender, hogboxHUD::HudInputEvent& inputEvent);
	void OnMouseLeave(osg::Object* sender, hogboxHUD::HudInputEvent& inputEvent);
	
protected:

	virtual ~ButtonRegion(void);

protected:

	//is the button currently held down (received mouseDown Event)
	//if mouse up occurs while m_butonDown is true a ButtonClick Event is triggered
	bool m_buttonDown;

	//mouse down/pressed texture
	osg::ref_ptr<osg::Texture> m_mouseDownTexture;
	
	//sent once a mouse down followed by a mouse up event is received by this region
	osg::ref_ptr<CallbackEvent> m_onButtonClickedEvent;
};

typedef osg::ref_ptr<ButtonRegion> ButtonRegionPtr;

};