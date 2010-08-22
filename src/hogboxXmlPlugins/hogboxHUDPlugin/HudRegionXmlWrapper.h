#pragma once

#include <hogboxHUD/HudRegion.h>
#include <hogboxHUD/TextRegion.h>
#include <hogboxHUD/ButtonRegion.h>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for Base HudRegion
//
class HudRegionXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass HogBoxObject to be wrapped
	HudRegionXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "HudRegion")
	{
		hogboxHUD::HudRegion* region = NULL;

		//get the Texture type from the nodes 'type' property
		std::string streamTypeStr;
		if(!hogboxDB::getXmlPropertyValue(node, "type", streamTypeStr))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype 'HudRegion' should have a 'type' property." <<std::endl 
									<< "                    i.e. <HudRegion uniqueID='myID' type='Button'>. Defaulting to base type" << std::endl;
			return;
		}

		
		//allocate the correct type

		//standard
		if(streamTypeStr == "Base" || streamTypeStr.empty())
		{
			region = new hogboxHUD::HudRegion();
		}

		//button
		if(streamTypeStr == "Button")
		{
			region = new hogboxHUD::ButtonRegion();
		}

		//text
		if(streamTypeStr == "Text")
		{
			region = new hogboxHUD::TextRegion();
		}


		//add the base attributes

		//Src should be a folder/directory containing the regions assests, which needs to be passed
		//to create after deserialising
		m_xmlAttributes["Src"] = new hogboxDB::TypedXmlAttribute<std::string>(&_assetDir);

		m_xmlAttributes["Position"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,osg::Vec2>(region,
																	&hogboxHUD::HudRegion::GetPosition,
																	&hogboxHUD::HudRegion::SetPosition);

		m_xmlAttributes["Size"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,osg::Vec2>(region,
																	&hogboxHUD::HudRegion::GetSize,
																	&hogboxHUD::HudRegion::SetSize);

		m_xmlAttributes["Rotation"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,float>(region,
																	&hogboxHUD::HudRegion::GetRotation,
																	&hogboxHUD::HudRegion::SetRotation);

		m_xmlAttributes["Color"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,osg::Vec3>(region,
																	&hogboxHUD::HudRegion::GetColor,
																	&hogboxHUD::HudRegion::SetColor);

		m_xmlAttributes["Alpha"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,float>(region,
																	&hogboxHUD::HudRegion::GetAlpha,
																	&hogboxHUD::HudRegion::SetAlpha);


		m_xmlAttributes["UseAlpha"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,bool>(region,
																	&hogboxHUD::HudRegion::IsAlphaEnabled,
																	&hogboxHUD::HudRegion::EnableAlpha);

		m_xmlAttributes["Layer"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,float>(region,
																	&hogboxHUD::HudRegion::GetLayer,
																	&hogboxHUD::HudRegion::SetLayer);

		//children
		m_xmlAttributes["Children"] = new hogboxDB::CallbackXmlClassPointerList<hogboxHUD::HudRegion,hogboxHUD::HudRegion::HudRegionList, hogboxHUD::HudRegion>(region,
																																		&hogboxHUD::HudRegion::GetChildrenList,
																																		&hogboxHUD::HudRegion::SetChildrenList);

		//add type specific atts

		//add text atts (and to any inherited (button etc)
		if(streamTypeStr == "Button" || streamTypeStr == "Text")
		{
			//cast to text
			hogboxHUD::TextRegion* text = dynamic_cast<hogboxHUD::TextRegion*>(region);
			if(text)
			{

				m_xmlAttributes["Text"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,std::string>(text,
																							&hogboxHUD::TextRegion::GetText,
																							&hogboxHUD::TextRegion::SetText);
				
				m_xmlAttributes["Font"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,std::string>(text,
																		&hogboxHUD::TextRegion::GetFontType,
																		&hogboxHUD::TextRegion::SetFontType);

				m_xmlAttributes["FontHeight"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,float>(text,
																		&hogboxHUD::TextRegion::GetFontHeight,
																		&hogboxHUD::TextRegion::SetFontHeight);

				m_xmlAttributes["Boarder"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,float>(text,
																		&hogboxHUD::TextRegion::GetBoarderPadding,
																		&hogboxHUD::TextRegion::SetBoarderPadding);

				m_xmlAttributes["TextColor"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,osg::Vec4>(text,
																		&hogboxHUD::TextRegion::GetTextColor,
																		&hogboxHUD::TextRegion::SetTextColor);

				//drop shadow
				m_xmlAttributes["UseShadow"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,bool>(text,
																		&hogboxHUD::TextRegion::isUsingDropShadow,
																		&hogboxHUD::TextRegion::UseDropShadow);

				m_xmlAttributes["ShadowColor"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,osg::Vec4>(text,
																		&hogboxHUD::TextRegion::GetDropShadowColor,
																		&hogboxHUD::TextRegion::SetDropShadowColor);

			}
		}


		//store the VideoFileStrem as the wrapped object
		p_wrappedObject = region;
	}

	//overload deserialise to call create stream once our atts are loaded
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//cast wrapped to Highest type first

		//Button
		hogboxHUD::ButtonRegion* button = dynamic_cast<hogboxHUD::ButtonRegion*>(p_wrappedObject.get());
		if(button)
		{
			return button->Create(button->GetPosition(), button->GetSize(), _assetDir, button->GetText());
		}

		//Text
		hogboxHUD::TextRegion* text = dynamic_cast<hogboxHUD::TextRegion*>(p_wrappedObject.get());
		if(text)
		{
			return text->Create(text->GetPosition(), text->GetSize(), _assetDir, text->GetText(), text->GetFontHeight());
		}

		//Base hudregion
		hogboxHUD::HudRegion* region = dynamic_cast<hogboxHUD::HudRegion*>(p_wrappedObject.get());
		if(region)
		{
			return region->Create(region->GetPosition(), region->GetSize(), _assetDir);
		}

		return false;
	}

protected:

	//helper loading variables
	std::string _assetDir;


protected:

	virtual ~HudRegionXmlWrapper(void){}

};

typedef osg::ref_ptr<HudRegionXmlWrapper> HudRegionXmlWrapperPtr;


