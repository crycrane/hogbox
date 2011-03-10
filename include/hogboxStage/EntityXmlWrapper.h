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
		hogboxStage::Entity* entity = new hogboxStage::Entity();

		m_xmlAttributes["Components"] = new hogboxDB::CallbackXmlClassPointerList<hogboxStage::Entity,hogboxStage::ComponentPtrVector, hogboxStage::Component>(entity,
																																&hogboxStage::Entity::GetComponentsList,
																																&hogboxStage::Entity::SetComponentsList);

		//store the VideoFileStrem as the wrapped object
		p_wrappedObject = entity;
	}

	//overload deserialise to call create stream once our atts are loaded
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//cast wrapped to entity
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

