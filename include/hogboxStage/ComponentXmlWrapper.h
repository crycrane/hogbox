#pragma once


#include <hogboxDB/XmlClassWrapper.h>

#include <hogboxStage/Component.h>
#include <hogboxStage/WorldTransformComponent.h>
#include <hogboxStage/RenderableComponent.h>

//
//Xml wrapper for all types of component
//
class ComponentXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//init from an existing entity object
	ComponentXmlWrapper(hogboxStage::Component* component)
		: hogboxDB::XmlClassWrapper(new osgDB::XmlNode(), "Component")
	{
		p_wrappedObject = component;
		//TODO HOOKUP XML ATTRIBUTES
	}

	//init from an xml node
	ComponentXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "Component")
	{
		hogboxStage::Component* component = NULL;

		//get the Component type from the nodes 'type' property
		std::string streamTypeStr;
		if(!hogboxDB::getXmlPropertyValue(node, "type", streamTypeStr))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype 'Component' should have a 'type' property." <<std::endl 
									<< "                    i.e. <Component uniqueID='myID' type='WorldTransform'>." << std::endl;
			return;
		}

		//allocate the correct type

		//standard
		if(streamTypeStr == "WorldTransform" || streamTypeStr.empty())
		{
			hogboxStage::WorldTransformComponent* worldTransComponent = new hogboxStage::WorldTransformComponent();
			m_xmlAttributes["Matrix"] = new hogboxDB::CallbackXmlAttribute<hogboxStage::WorldTransformComponent,osg::Matrix>(worldTransComponent,
																												&hogboxStage::WorldTransformComponent::GetTransform,
																												&hogboxStage::WorldTransformComponent::SetTransform);
			
			m_xmlAttributes["Position"] = new hogboxDB::CallbackXmlAttribute<hogboxStage::WorldTransformComponent,osg::Vec3>(worldTransComponent,
																												&hogboxStage::WorldTransformComponent::GetPosition,
																												&hogboxStage::WorldTransformComponent::SetPosition);

			m_xmlAttributes["Rotation"] = new hogboxDB::CallbackXmlAttribute<hogboxStage::WorldTransformComponent,osg::Vec3>(worldTransComponent,
																												&hogboxStage::WorldTransformComponent::GetRotationDegrees,
																												&hogboxStage::WorldTransformComponent::SetRotationDegrees);
			component = worldTransComponent;
		}

		//button
		if(streamTypeStr == "Renderable")
		{
			hogboxStage::RenderableComponent* renderComponent = new hogboxStage::RenderableComponent();
			//set renderable hogboxobject pointer attribute
			m_xmlAttributes["RenderObject"] = new hogboxDB::CallbackXmlClassPointer< hogboxStage::RenderableComponent,hogbox::HogBoxObject>(renderComponent,
																										&hogboxStage::RenderableComponent::GetRenderableObject,				
																										&hogboxStage::RenderableComponent::SetRenderableObject);
			component = renderComponent;
		}

		//store the component as the wrapped object
		p_wrappedObject = component;
	}

	//overload deserialise to call create stream once our atts are loaded
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//Base hudregion
		hogboxStage::Component* comp = dynamic_cast<hogboxStage::Component*>(p_wrappedObject.get());
		if(comp)
		{
			return true;//entity->Create(region->GetPosition(), region->GetSize(), _assetDir);
		}

		return false;
	}

protected:


protected:

	virtual ~ComponentXmlWrapper(void){}

};

typedef osg::ref_ptr<ComponentXmlWrapper> ComponentXmlWrapperPtr;

