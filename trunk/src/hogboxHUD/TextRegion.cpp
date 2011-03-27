#include <hogboxHUD/TextRegion.h>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

using namespace hogboxHUD;

TextRegion::TextRegion(void) : HudRegion(),
								 m_text(NULL),
								 m_string(""),
								m_fontHeight(18.0f),
								m_fontName("fonts/arial.ttf"),
								m_boarderPadding(5.0f),
								m_alignmentMode(CENTER_ALIGN),
								m_textColor(osg::Vec4(0.1f,0.1f,0.1f,1.0f)),
								m_usingDropShadow(false),
								m_dropShadowColor(osg::Vec4(0.1f,0.1f,0.1f,0.7f)),
								//callback events
								m_onTextChangedEvent(new CallbackEvent(this, "OnTextChanged"))

{
	//create the text label to add to the button
	m_text = new osgText::Text;
	m_text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);

	m_text->setColor(m_textColor);
	this->SetFontType(m_fontName);
	this->SetFontHeight(m_fontHeight);
	this->SetAlignment(m_alignmentMode);
	this->UseDropShadow(m_usingDropShadow);
	this->SetDropShadowColor(m_dropShadowColor);

	//add the text to a geode for drawing
	osg::Geode* geode = new osg::Geode();
	geode->addDrawable(m_text);

	//add the textgeode to a transform to raise it above the background
	osg::MatrixTransform* frontLayer = new osg::MatrixTransform();
	frontLayer->setMatrix(osg::Matrix::translate(osg::Vec3(0,0,0.1)));
	frontLayer->addChild(geode);

	//attach the text to translate for now 
	m_translate->addChild(frontLayer);
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
TextRegion::TextRegion(const TextRegion& region,const osg::CopyOp& copyop)
	: HudRegion(region, copyop),
	//m_text(copyop(region.m_text.get())),
	m_fontHeight(region.m_fontHeight),
	m_fontName(region.m_fontName),
	m_boarderPadding(region.m_boarderPadding),
	m_alignmentMode(region.m_alignmentMode),
	m_textColor(region.m_textColor),
	m_usingDropShadow(region.m_usingDropShadow),
	m_dropShadowColor(region.m_dropShadowColor)
{
	//this->SetTextColor(m_textColor);
	this->SetFontType(m_fontName);
	this->SetFontHeight(m_fontHeight);
	this->SetAlignment(m_alignmentMode);
	this->UseDropShadow(m_usingDropShadow);
	this->SetDropShadowColor(m_dropShadowColor);
}

TextRegion::~TextRegion(void)
{
	OSG_NOTICE << "    Deallocating TextRegion: Name '" << this->getName() << "'." << std::endl;
	m_text = NULL;
}


//
// Create a text region using a fileName as the background
// then creating an osg text geode and attaching directly to translate
// so that the texts size can be set directly in pixels, without
//being scaled by the m_scale matrix
//
bool TextRegion::Create(osg::Vec2 corner, osg::Vec2 size, const std::string& fileName, const std::string& label, float fontHeight)
{
	//load the base assets and apply names and sizes, do this
	//after creating and adding the text so that the text geode 
	//name will also be changed
	bool ret = HudRegion::Create(corner,size,fileName);

	SetText(label);
	
	return ret;
}

//
//Text region loads the aditional assests
//searches for a ttf file to use as a font
//
bool TextRegion::LoadAssest(const std::string& folderName)
{
	//call base first
	bool ret = HudRegion::LoadAssest(folderName);

	//get the directory contents
	osgDB::DirectoryContents dirCont;
	dirCont = osgDB::getDirectoryContents(folderName);
	//loop all the files and look for ttf extension
	for(unsigned int i=0; i<dirCont.size(); i++)
	{
		if(osgDB::getFileExtension(dirCont[i]).compare("ttf") == 0)
		{
			//try to load as font
			this->SetFontType(dirCont[i]);
		}
	}
	return ret;
}

//
//overload set position
//
void TextRegion::SetPosition(const osg::Vec2& corner)
{
	HudRegion::SetPosition(corner);
}

//overload SetSize so we can set the texts pixelheight independantly
void TextRegion::SetSize(const osg::Vec2& size)
{
	HudRegion::SetSize(size);

	//set the texts max sizes to the new size
	m_text->setMaximumHeight(size.y());
	m_text->setMaximumWidth(size.x());

	//set new centered position
	this->SetAlignment(m_alignmentMode);
}

//
//overload set alpha so we can also adust text color alpha
//
void TextRegion::SetAlpha(const float& alpha)
{
	m_textColor.a() = alpha;
	if(!m_alphaEnabled)
	{
		m_textColor.a() = 1.0f;
	}
	
	m_text->setColor(m_textColor);
	HudRegion::SetAlpha(alpha);
}


//
// Set the text for this region
//
void TextRegion::SetText(const std::string& str)
{
	m_string = str;
	m_text->setText(str);
	OSG_FATAL << "Set Text to '" << str << "'." << std::endl;
	osg::ref_ptr<HudInputEvent> dummyEvent;
	m_onTextChangedEvent->TriggerEvent(*dummyEvent.get());
}

//
//return the current text vaule
//
const std::string& TextRegion::GetText()const
{
	return m_string;
}


//
//set font height in pixels
//
void TextRegion::SetFontHeight(const float& fontHeight)
{
	m_fontHeight = fontHeight;
	m_text->setCharacterSize(fontHeight, 1.0f);
}

//
//Return current font height
//
const float& TextRegion::GetFontHeight()const
{
	return m_fontHeight;
}

//
//set the texts font type
//
void TextRegion::SetFontType(const std::string& fontFile)
{
	m_fontName = fontFile;
	m_text->setFont(fontFile);
}

//
//Return the name of the font type
//being used
//
const std::string& TextRegion::GetFontType()const
{
	return m_fontName;
}

//
//set the number of pixels between
//the edge of the text and the edge of the
//region when in left or right alignments
//
const float& TextRegion::GetBoarderPadding()const
{
	return m_boarderPadding;
}

void TextRegion::SetBoarderPadding(const float& padding)
{
	m_boarderPadding = padding;
}

//
//Set the text alignment mode
//
void TextRegion::SetAlignment(const TEXT_ALIGN& alignment)
{
	m_alignmentMode = alignment;

	switch(alignment)
	{
		case CENTER_ALIGN:
		{
			m_text->setAlignment(osgText::Text::CENTER_CENTER ); 
			m_text->setPosition(osg::Vec3(m_size.x()*0.5f, m_size.y()*0.5f, 0.0f));
			break;
		}
		case RIGHT_ALIGN:
		{
			m_text->setAlignment(osgText::Text::RIGHT_CENTER); 
			m_text->setPosition(osg::Vec3(m_size.x()-m_boarderPadding, m_size.y()*0.5f, 0));
			break;
		}
		case LEFT_ALIGN:
		{
			m_text->setAlignment(osgText::Text::LEFT_CENTER); 
			m_text->setPosition(osg::Vec3(m_boarderPadding, m_size.y()*0.5f, 0));
			break;
		}
		default:
		{
			m_text->setAlignment(osgText::Text::CENTER_CENTER ); 
			break;
		}
	}
}

//
//Return the text alignment mode
//
const TEXT_ALIGN& TextRegion::GetAlignment()const
{
	return m_alignmentMode;
}

//
//Set the color of the text
//
void TextRegion::SetTextColor(const osg::Vec4& color)
{
	m_textColor.r() = color.x();
	m_textColor.g() = color.y();
	m_textColor.b() = color.z();
	m_text->setColor(m_textColor);
}

//
//Return the color of the text
//
const osg::Vec4& TextRegion::GetTextColor()const
{
	return m_text->getColor();
}

//
//set the drop shadow color
//
void TextRegion::SetDropShadowColor(const osg::Vec4 &color)
{
	m_dropShadowColor = color;
	m_text->setBackdropColor(color);
}

//
//return color of drop show
//
const osg::Vec4& TextRegion::GetDropShadowColor()const
{
	return m_dropShadowColor;
}


void TextRegion::UseDropShadow(const bool& useShadow)
{
	m_usingDropShadow = useShadow;

	if(useShadow)
	{
		m_text->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_LEFT);
		m_text->setBackdropImplementation(osgText::Text::DEPTH_RANGE );
	}else{
		m_text->setBackdropType(osgText::Text::NONE);
	}
}

const bool& TextRegion::isUsingDropShadow() const
{
	return m_usingDropShadow;
}


int TextRegion::HandleInputEvent(HudInputEvent& hudEvent)
{
	return HudRegion::HandleInputEvent(hudEvent); 
}

//
//Funcs to register event callbacks
void TextRegion::AddOnTextChangedCallbackReceiver(HudEventCallback* callback)
{
	m_onTextChangedEvent->AddCallbackReceiver(callback);
}