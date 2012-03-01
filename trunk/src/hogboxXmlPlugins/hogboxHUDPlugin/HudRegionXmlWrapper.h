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
		_xmlAttributes["Src"] = new hogboxDB::TypedXmlAttribute<std::string>(&_assetDir);

		_xmlAttributes["Position"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,osg::Vec2>(region,
																	&hogboxHUD::HudRegion::GetPosition,
																	&hogboxHUD::HudRegion::SetPosition);

		_xmlAttributes["Size"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,osg::Vec2>(region,
																	&hogboxHUD::HudRegion::GetSize,
																	&hogboxHUD::HudRegion::SetSize);

		_xmlAttributes["Rotation"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,float>(region,
																	&hogboxHUD::HudRegion::GetRotation,
																	&hogboxHUD::HudRegion::SetRotation);

		_xmlAttributes["Color"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,osg::Vec3>(region,
																	&hogboxHUD::HudRegion::GetColor,
																	&hogboxHUD::HudRegion::SetColor);

		_xmlAttributes["Alpha"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,float>(region,
																	&hogboxHUD::HudRegion::GetAlpha,
																	&hogboxHUD::HudRegion::SetAlpha);


		_xmlAttributes["UseAlpha"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,bool>(region,
																	&hogboxHUD::HudRegion::IsAlphaEnabled,
																	&hogboxHUD::HudRegion::EnableAlpha);

		_xmlAttributes["Layer"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::HudRegion,float>(region,
																	&hogboxHUD::HudRegion::GetLayer,
																	&hogboxHUD::HudRegion::SetLayer);

		//children
		_xmlAttributes["Children"] = new hogboxDB::CallbackXmlClassPointerList<hogboxHUD::HudRegion,hogboxHUD::HudRegion::HudRegionList, hogboxHUD::HudRegion>(region,
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

				_xmlAttributes["Text"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,std::string>(text,
																							&hogboxHUD::TextRegion::GetText,
																							&hogboxHUD::TextRegion::SetText);
				
				_xmlAttributes["Font"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,std::string>(text,
																		&hogboxHUD::TextRegion::GetFontType,
																		&hogboxHUD::TextRegion::SetFontType);

				_xmlAttributes["FontHeight"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,float>(text,
																		&hogboxHUD::TextRegion::GetFontHeight,
																		&hogboxHUD::TextRegion::SetFontHeight);

				_xmlAttributes["Boarder"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,float>(text,
																		&hogboxHUD::TextRegion::GetBoarderPadding,
																		&hogboxHUD::TextRegion::SetBoarderPadding);

				_xmlAttributes["TextColor"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,osg::Vec4>(text,
																		&hogboxHUD::TextRegion::GetTextColor,
																		&hogboxHUD::TextRegion::SetTextColor);

				//drop shadow
				_xmlAttributes["TextBackDropType"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,int>(text,
																		&hogboxHUD::TextRegion::GetBackDropTypeInt,
																		&hogboxHUD::TextRegion::SetBackDropTypeInt);

				_xmlAttributes["TextBackDropColor"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,osg::Vec4>(text,
																		&hogboxHUD::TextRegion::GetBackDropColor,
																		&hogboxHUD::TextRegion::SetBackDropColor);

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


