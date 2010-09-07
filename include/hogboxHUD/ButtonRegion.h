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

	bool IsPressed()
	{	bool ret=m_isPressed;
		m_isPressed=false;
		return ret;
	}

protected:

	virtual ~ButtonRegion(void);

protected:

	bool m_isPressed;

	//mouse down/pressed texture
	osg::ref_ptr<osg::Texture> m_mouseDownTexture;
};

typedef osg::ref_ptr<ButtonRegion> ButtonRegionPtr;

};