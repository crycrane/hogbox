#pragma once

#include <hogboxStage/Entity.h>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for Base HudRegion
//
class EntityXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//init from an existing entity object
	EntityXmlWrapper(hogboxStage::Entity* entity)
		: hogboxDB::XmlClassWrapper(new osgDB::XmlNode(), "Entity")
	{
		p_wrappedObject = entity;
		//TODO HOOKUP XML ATTRIBUTES
	}

	//init from an xml node
	EntityXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "Entity")
	{
		hogboxStage::Entity* entity = NULL;

		//get the entity type from the nodes 'type' property
		std::string streamTypeStr;
		if(!hogboxDB::getXmlPropertyValue(node, "type", streamTypeStr))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype 'Entity' should have a 'type' property." <<std::endl 
									<< "                    i.e. <Entity uniqueID='myID' type='StaticModel'>. Defaulting to 'StaticModel' type" << std::endl;
			return;
		}

		//allocate the correct type

		//standard
		if(streamTypeStr == "Base" || streamTypeStr.empty())
		{
			entity = new hogboxStage::Entity();
		}

		//button
		if(streamTypeStr == "Button")
		{
			//region = new hogboxHUD::ButtonRegion();
		}

		//add type specific atts

		//add text atts (and to any inherited (button etc)
		if(streamTypeStr == "Button" || streamTypeStr == "Text")
		{
			//cast to text
			/*hogboxHUD::TextRegion* text = dynamic_cast<hogboxHUD::TextRegion*>(region);
			if(text)
			{

				m_xmlAttributes["Text"] = new hogboxDB::CallbackXmlAttribute<hogboxHUD::TextRegion,std::string>(text,
																							&hogboxHUD::TextRegion::GetText,
																							&hogboxHUD::TextRegion::SetText);
			
			}*/
		}

		//store the VideoFileStrem as the wrapped object
		p_wrappedObject = entity;
	}

	//overload deserialise to call create stream once our atts are loaded
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//cast wrapped to Highest type first

		//Button
		/*hogboxHUD::ButtonRegion* button = dynamic_cast<hogboxHUD::ButtonRegion*>(p_wrappedObject.get());
		if(button)
		{
			return button->Create(button->GetPosition(), button->GetSize(), _assetDir, button->GetText());
		}*/

		//Base hudregion
		hogboxStage::Entity* entity = dynamic_cast<hogboxStage::Entity*>(p_wrappedObject.get());
		if(entity)
		{
			return true;//entity->Create(region->GetPosition(), region->GetSize(), _assetDir);
		}

		return false;
	}

protected:


protected:

	virtual ~EntityXmlWrapper(void){}

};

typedef osg::ref_ptr<EntityXmlWrapper> EntityXmlWrapperPtr;

