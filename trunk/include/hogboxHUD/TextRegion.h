#pragma once

#include "HudRegion.h"

#include <osg/Group>
#include <osg/Depth>
#include <osgText/Text>

namespace hogboxHUD {

enum TEXT_ALIGN
{
	CENTER_ALIGN = 0,
	RIGHT_ALIGN,
	LEFT_ALIGN
};

//
//TextRegion
//
class HOGBOXHUD_EXPORT TextRegion : public HudRegion
{
public:
	TextRegion(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	TextRegion(const TextRegion& region,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogboxHUD,TextRegion);

	virtual bool Create(osg::Vec2 corner, osg::Vec2 size, const std::string& fileName, const std::string& label, float fontHeight = 18.0f);
	
	virtual int HandleInputEvent(HudInputEvent& hudEvent);

	//
	//Text region loads the aditional assests
	//searches for a ttf file to use as a font
	virtual bool LoadAssest(const std::string& folderName);


	//overload set position
	virtual void SetPosition(const osg::Vec2& corner);

	//overload SetSize so we can set the texts pixelheight independantly
	virtual void SetSize(const osg::Vec2& size);

	//overload set alpha so we can also adust text color alpha
	virtual void SetAlpha(const float& alpha);

	//set the actual text string,
	virtual void SetText(const std::string& str);
	//return the current text vaule
	const std::string& GetText() const;

	//set font height in pixels
	void SetFontHeight(const float& fontHeight);
	const float& GetFontHeight() const;

	//set the texts font type
	void SetFontType(const std::string& fontFile);
	const std::string& GetFontType() const;

	//set the number of pixels between
	//the edge of the text and the edge of the
	//region when in left or right alignments
	const float& GetBoarderPadding() const;
	void SetBoarderPadding(const float& padding);

	//Set the text alignment mode
	void SetAlignment(const TEXT_ALIGN& alignment);
	const TEXT_ALIGN& GetAlignment() const;

	//set the text font color
	void SetTextColor(const osg::Vec4& color);
	const osg::Vec4& GetTextColor() const;

	//set the drop shadow color
	void SetDropShadowColor(const osg::Vec4& color);
	const osg::Vec4& GetDropShadowColor() const;

	//enable/disable the texts drop shadow
	void UseDropShadow(const bool& useShadow);
	const bool& isUsingDropShadow() const;
	
public:
	
	//
	//Funcs to register event callbacks
	void AddOnTextChangedCallbackReceiver(HudEventCallback* callback);

protected:

	virtual ~TextRegion(void);

protected:

	//the string to display
	std::string m_string;

	//text displayed in the region
	osg::ref_ptr<osgText::Text> m_text;

	//heigth of font in pixels/hud space
	float m_fontHeight;

	//the name of the font being used
	std::string m_fontName;

	//the padding to left or right of text
	float m_boarderPadding;

	//the alignment mode for the text
	//relative to the local region space
	TEXT_ALIGN m_alignmentMode;

	//the color of our text, including alpha
	//which should match the region alpha
	osg::Vec4 m_textColor;

	//is the drop shadow being rendered
	bool m_usingDropShadow;

	//color of drop shadow
	osg::Vec4 m_dropShadowColor;
	
	//Callback events
	
	//on text changed called each time SetText is called to inform receivers of the change
	osg::ref_ptr<CallbackEvent> m_onTextChangedEvent;
};

typedef osg::ref_ptr<TextRegion> TextRegionPtr;

};