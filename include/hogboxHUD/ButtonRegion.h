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

#include <hogboxHUD/TextRegion.h>

namespace hogboxHUD {

//
//Button region, is just a text region that responds to
// click events by flagging its isPressed flag
//
class HOGBOXHUD_EXPORT ButtonRegion : public TextRegion
{
public:
	ButtonRegion(RegionPlane plane=PLANE_XY, RegionOrigin origin=ORI_BOTTOM_LEFT, bool isProcedural=false);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	ButtonRegion(const ButtonRegion& region,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Object(hogboxHUD,ButtonRegion);

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
	//if mouse up occurs while _butonDown is true a ButtonClick Event is triggered
	bool _buttonDown;

	//mouse down/pressed texture
	osg::ref_ptr<osg::Texture> _mouseDownTexture;
	
	//sent once a mouse down followed by a mouse up event is received by this region
	osg::ref_ptr<CallbackEvent> _onButtonClickedEvent;
};

typedef osg::ref_ptr<ButtonRegion> ButtonRegionPtr;

};