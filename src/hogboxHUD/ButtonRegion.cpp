#include <hogboxHUD/ButtonRegion.h>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <hogbox/HogBoxUtils.h>

using namespace hogboxHUD;

ButtonRegion::ButtonRegion(void) : TextRegion(),
									 m_isPressed(false),
									 m_mouseDownTexture(NULL)
{

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
	bool ret = TextRegion::Create(corner,size,fileName, label);
	m_isPressed = false;
	return ret;
}

int ButtonRegion::Event(const std::string ID, CHudEvent hudEvent)
{	
	//check if the hud event is for this region
	if(hudEvent.GetID().compare(this->getName())==0)
	{
		//if it was and it's a down mouse press and the region isn't diabled
		if(hudEvent.GetEventType()== MDOWN)
		{
			//flag to user it has been pressed
			m_isPressed=true;
			return 1;
		}

		//if it's a mouse up event 
		if(hudEvent.GetEventType()== MUP)
		{
			//if it's already pressed down
			if(m_isPressed)
			{
			}
		}

		if(hudEvent.GetEventType() == HOVER)
		{
			if(m_hovering==false)
			{
				m_hovering = true;
				//swap tex to rollover
				if(m_rollOverTexture.valid())
				{this->ApplyTexture(m_rollOverTexture.get());}
			}
		}
		return 1;
	}

	//if not for use but we were hovering, undo the hover
	if(m_hovering == true)
	{
		m_hovering = false;
		if(m_baseTexture.valid())
		{this->ApplyTexture(m_baseTexture.get());}
	}

	return TextRegion::Event(ID, hudEvent); 
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


