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
    class ButtonRegionStyle : public TextRegionStyle{
    public:
        ButtonRegionStyle()
            : TextRegionStyle()
        {
        }
        
        ButtonRegionStyle(const ButtonRegionStyle& args)
            : TextRegionStyle(args)
        {
        }
        
    protected:
        virtual ~ButtonRegionStyle(){
        }
    };
    
    //
    ButtonRegion(ButtonRegionStyle* style = new ButtonRegionStyle());
    
    //
	ButtonRegion(osg::Vec2 corner, osg::Vec2 size, ButtonRegionStyle* style = new ButtonRegionStyle());

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	ButtonRegion(const ButtonRegion& region,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Object(hogboxHUD,ButtonRegion);
    virtual RegionStyle* allocateStyleType(){return new ButtonRegionStyle();}

	//
	//Create a region to behave like a basic button
	virtual bool Create(osg::Vec2 corner, osg::Vec2 size, RegionStyle* style = new ButtonRegionStyle());

    //
    //Get Set the highlight color
    const osg::Vec3& GetHighlightColor()const;
    void SetHighlightColor(const osg::Vec3& color);
    
    //
    //set the enabled state to false to prevent clicks
    void SetEnabled(const bool& enable);
    const bool& GetEnabled()const;
    
	//
	//Detects mouse down and flags is pressed state,
	//also swaps to the mouse down texture if one is present
	virtual int HandleInputEvent(HudInputEvent& hudEvent);
	
	//
	//Funcs to register event callbacks
	void AddOnButtonClickedCallbackReceiver(HudEventCallback* callback);
    
    //
    //In special cases, external object may need to break the buttonDown state,
    //e.g. if the button is being dragged as part of a list
    void CancelButtonDown();
	
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
    
    //
	//Button region loads the aditional assests
	//mouseDown.png, used when the mouse is pressed down on the region
	virtual bool LoadAssest(RegionStyle* args);

protected:

    //
    //enable/disable pressing of the button
    bool _enabled;
    
	//is the button currently held down (received mouseDown Event)
	//if mouse up occurs while _butonDown is true a ButtonClick Event is triggered
	bool _buttonDown;

	//mouse down/pressed texture
	osg::ref_ptr<osg::Texture> _mouseDownTexture;
    
    //
    //Highlight color
    osg::Vec3 _highlightColor;
    //cache the original color when highlighting
    osg::Vec3 _oriColor;
	
	//sent once a mouse down followed by a mouse up event is received by this region
	osg::ref_ptr<HudCallbackEvent> _onButtonClickedEvent;
};

typedef osg::ref_ptr<ButtonRegion> ButtonRegionPtr;

};