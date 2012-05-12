#include <hogboxHUD/ButtonRegion.h>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <hogbox/HogBoxUtils.h>

using namespace hogboxHUD;

const float highlightTime = 0.2f;

ButtonRegion::ButtonRegion(ButtonRegionStyle* style)
    : TextRegion(style),
    _buttonDown(false),
    _mouseDownTexture(NULL),
    _highlightColor(osg::Vec3(0,0,1)),
    _oriColor(osg::Vec3(0,0,0)),
    //callback events
    _onButtonClickedEvent(new HudCallbackEvent(this, "OnButtonClicked"))
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

ButtonRegion::ButtonRegion(osg::Vec2 corner, osg::Vec2 size, ButtonRegionStyle* style) 
    : TextRegion(corner, size, style),
    _buttonDown(false),
    _mouseDownTexture(NULL),
    //callback events
    _onButtonClickedEvent(new HudCallbackEvent(this, "OnButtonClicked"))
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
bool ButtonRegion::Create(osg::Vec2 corner, osg::Vec2 size, RegionStyle* style )
{
	//load the base assets and apply names and sizes
	return TextRegion::Create(corner,size,style);
}

//
//Get Set the highlight color
//
const osg::Vec3& ButtonRegion::GetHighlightColor()const
{
    return _highlightColor;
}
void ButtonRegion::SetHighlightColor(const osg::Vec3& color)
{
    _highlightColor = color;
}

int ButtonRegion::HandleInputEvent(HudInputEvent& hudEvent)
{	
	return TextRegion::HandleInputEvent(hudEvent); 
}

//
//In special cases, external object may need to break the buttonDown state,
//e.g. if the button is being dragged as part of a list
//
void ButtonRegion::CancelButtonDown()
{
    if(_buttonDown){
        //animate to back to base color
        //this->AddColorKey<osgAnimation::LinearMotion>(_oriColor, highlightTime);
    }
    
    _buttonDown = false; 
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
		_onButtonClickedEvent->Trigger(inputEvent);
        
        //animate to the highlight color
        if(this->GetNumColorKeys() == 0){
            _oriColor = this->GetColor();
        }
        this->AddColorKey<osgAnimation::LinearMotion>(_highlightColor, highlightTime);
        //animate to back to base color
        this->AddColorKey<osgAnimation::LinearMotion>(_oriColor, highlightTime);
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
    
    if(_buttonDown){
        //animate to back to base color
        //this->AddColorKey<osgAnimation::LinearMotion>(_oriColor, highlightTime);
    }
    
	//reset buttonDown state
	_buttonDown = false;
    
}

//
//Button region loads the aditional assests
//mouseDown.png, used when the mouse is pressed down on the region
//
bool ButtonRegion::LoadAssest(RegionStyle* args)
{
	//call base first
	bool ret = TextRegion::LoadAssest(args);

    ButtonRegionStyle* asButtonStyle = dynamic_cast<ButtonRegionStyle*>(args);
    if(asButtonStyle){
        
    }
	//try to load the mouseDown texture
	//std::string mouseDownTextureFile = folderName+"/mouseDown.png";
	//if(osgDB::fileExists(mouseDownTextureFile) )
	//{_mouseDownTexture = hogbox::LoadTexture2D(mouseDownTextureFile);}

	return ret;
}

//
//Funcs to register event callbacks
void ButtonRegion::AddOnButtonClickedCallbackReceiver(HudEventCallback* callback)
{
	_onButtonClickedEvent->AddCallbackReceiver(callback);
}


