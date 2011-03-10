#pragma once
#include <hogboxDB/HogBoxRegistry.h>
#include <hogboxDB/XmlClassManager.h>
#include <hogboxDB/HogBoxManager.h>
#include <hogboxStage/Component.h>

#include <hogboxStage/ComponentXmlWrapper.h>

namespace hogboxStage{

//
//Xml manager for loading component
//
class ComponentXmlManager : public hogboxDB::XmlClassManager
{
public:

	ComponentXmlManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Component", "Xml definition of Component and basic derived types");
	}
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	ComponentXmlManager(const ComponentXmlManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxStage, ComponentXmlManager)

protected:

	virtual ~ComponentXmlManager(void)
	{
	}
	
	//
	//Create an HudRegion from xml
	//
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		ComponentXmlWrapperPtr xmlWrapper;

		//create our object and it's xml wrapper.
		xmlWrapper = new ComponentXmlWrapper(xmlNode);
		
		if(!xmlWrapper.get()){return NULL;}
		//did the wrapper alocate an object
		if(!xmlWrapper->getWrappedObject()){return NULL;}

		//if the wrapper was created properly then use it 
		//to deserialize our the xmlNode into it's wrapped object
		if(!xmlWrapper->deserialize(xmlNode))
		{
			//an error occured deserializing the xml node
			return NULL;
		}

		return xmlWrapper;
	}
};

};

using namespace hogboxStage;

//
//Will register the xml mamager plugin with the hogbox registry automatically
//then this dll is loaded
REGISTER_HOGBOXPLUGIN(Component,ComponentXmlManager)

