#include <hogboxHUD/ButtonRegion.h>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <hogbox/HogBoxUtils.h>

using namespace hogboxHUD;

ButtonRegion::ButtonRegion(void) : TextRegion(),
									m_buttonDown(false),
									m_mouseDownTexture(NULL),
									//callback events
									m_onButtonClickedEvent(new CallbackEvent(this, "OnButtonClicked"))
{

	//register for the base mouse down and mouse up events to detect ButtonClicked
	//set app class to receive callback when mouse is pressed on region
	this->AddOnMouseDownCallbackReceiver(new hogboxHUD::HudEventObjectCallback<ButtonRegion>(this, this,
																							 &ButtonRegion::OnMouseDown));
	this->AddOnMouseUpCallbackReceiver(new hogboxHUD::HudEventObjectCallback<ButtonRegion>(this, this,
																						   &ButtonRegion::OnMouseUp));
	this->AddOnMouseEnterCallbackReceiver(new hogboxHUD::HudEventObjectCallback<ButtonRegion>(this, this,
																						   &ButtonRegion::OnMouseEnter));
	this->AddOnMouseLeaveCallbackReceiver(new hogboxHUD::HudEventObjectCallback<ButtonRegion>(this, this,
																							  &ButtonRegion::OnMouseLeave));
	
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
ButtonRegion::ButtonRegion(const ButtonRegion& region,const osg::CopyOp& copyop)
	: TextRegion(region, copyop)
{
}

ButtonRegion::~ButtonRegion(void)
{
	m_mouseDownTexture = NULL;
}

//
// Create a button with a loaded file as backdrop
//
bool ButtonRegion::Create(osg::Vec2 corner, osg::Vec2 size, const std::string& fileName,
																const std::string& label)
{
	//load the base assets and apply names and sizes
	return TextRegion::Create(corner,size,fileName, label);
}

int ButtonRegion::HandleInputEvent(HudInputEvent& hudEvent)
{	
	osg::notify(osg::WARN) << "CLICK LICK SLICK" << std::endl;
	return TextRegion::HandleInputEvent(hudEvent); 
}

//
//
//receive the base callbacks to detect our button click event
//
void ButtonRegion::OnMouseDown(hogboxHUD::HudRegion* sender, hogboxHUD::HudInputEvent& inputEvent)
{
	m_buttonDown = true;
}
void ButtonRegion::OnMouseUp(hogboxHUD::HudRegion* sender, hogboxHUD::HudInputEvent& inputEvent)
{
	//if the button is down then it's a Click event
	if(m_buttonDown){
		//trigger the onButtonClicked Event 
		m_onButtonClickedEvent->TriggerEvent(inputEvent);
	}
	
	//reset buttonDown state
	m_buttonDown = false;
}

//detect for rollover and to disable a mouseDown when focus is lost
void ButtonRegion::OnMouseEnter(hogboxHUD::HudRegion* sender, hogboxHUD::HudInputEvent& inputEvent)
{
	//switch to rollover texture
	if(m_rollOverTexture.valid())
	{this->ApplyTexture(m_rollOverTexture.get());}
}

void ButtonRegion::OnMouseLeave(hogboxHUD::HudRegion* sender, hogboxHUD::HudInputEvent& inputEvent)
{
	//switch back to base texture
	if(m_baseTexture.valid())
	{this->ApplyTexture(m_baseTexture.get());}
	
	//reset buttonDown state
	m_buttonDown = false;
}

//
//Button region loads the aditional assests
//mouseDown.png, used when the mouse is pressed down on the region
//
bool ButtonRegion::LoadAssest(const std::string& folderName)
{
	//call base first
	bool ret = TextRegion::LoadAssest(folderName);

	//try to load the mouseDown texture
	std::string mouseDownTextureFile = folderName+"/mouseDown.png";
	if(osgDB::fileExists(mouseDownTextureFile) )
	{m_mouseDownTexture = hogbox::LoadTexture2D(mouseDownTextureFile);}

	return ret;
}

//
//Funcs to register event callbacks
void ButtonRegion::AddOnButtonClickedCallbackReceiver(HudEventCallback* callback)
{
	m_onButtonClickedEvent->AddCallbackReceiver(callback);
}


