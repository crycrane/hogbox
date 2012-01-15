#include <hogboxHUD/ButtonRegion.h>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <hogbox/HogBoxUtils.h>

using namespace hogboxHUD;

ButtonRegion::ButtonRegion(RegionPlane plane, RegionOrigin origin, bool isProcedural) 
    : TextRegion(plane, origin, isProcedural),
    _buttonDown(false),
    _mouseDownTexture(NULL),
    //callback events
    _onButtonClickedEvent(new CallbackEvent(this, "OnButtonClicked"))
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
	//OSG_NOTICE << "    Deallocating ButtonRegion: Name '" << this->getName() << "'." << std::endl;
	_onButtonClickedEvent = NULL;
    _mouseDownTexture = NULL;
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
	return TextRegion::HandleInputEvent(hudEvent); 
}

//
//
//receive the base callbacks to detect our button click event
//
void ButtonRegion::OnMouseDown(osg::Object* sender, hogboxHUD::HudInputEvent& inputEvent)
{
	_buttonDown = true;
}
void ButtonRegion::OnMouseUp(osg::Object* sender, hogboxHUD::HudInputEvent& inputEvent)
{
	//if the button is down then it's a Click event
	if(_buttonDown){
		//trigger the onButtonClicked Event 
		_onButtonClickedEvent->TriggerEvent(inputEvent);
	}
	
	//reset buttonDown state
	_buttonDown = false;
}

//detect for rollover and to disable a mouseDown when focus is lost
void ButtonRegion::OnMouseEnter(osg::Object* sender, hogboxHUD::HudInputEvent& inputEvent)
{
	//switch to rollover texture
	if(_rollOverTexture.valid())
	{this->ApplyTexture(_rollOverTexture.get());}
}

void ButtonRegion::OnMouseLeave(osg::Object* sender, hogboxHUD::HudInputEvent& inputEvent)
{
	//switch back to base texture
	if(_baseTexture.valid())
	{this->ApplyTexture(_baseTexture.get());}
	
	//reset buttonDown state
	_buttonDown = false;
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
	{_mouseDownTexture = hogbox::LoadTexture2D(mouseDownTextureFile);}

	return ret;
}

//
//Funcs to register event callbacks
void ButtonRegion::AddOnButtonClickedCallbackReceiver(HudEventCallback* callback)
{
	_onButtonClickedEvent->AddCallbackReceiver(callback);
}


