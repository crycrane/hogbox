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

#include <hogboxDB/HogBoxRegistry.h>
#include <hogboxDB/XmlClassManager.h>
#include <hogboxDB/HogBoxManager.h>

#include <hogbox/HogBoxMaterial.h>
#include "HogBoxMaterialXmlWrapper.h"

//for now material will load featurelevels and add them to the SystemInfo instance
//as they're the only object thats uses them
#include "FeatureLevelXmlWrapper.h"

//
//Managers osgobject nodes in an xml document
//
class HogBoxMaterialManager : public hogboxDB::XmlClassManager
{
public:

	HogBoxMaterialManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("HogBoxMaterial", "Xml definition of HogBoxMaterial");
		SupportsClassType("FeatureLevel", "Xml definition of SystemFeatureLevel");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxMaterialManager(const HogBoxMaterialManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxDB, HogBoxMaterialManager)


protected:

	virtual ~HogBoxMaterialManager(void)
	{
	}
	
	//
	//Create an HogBoxObject from xml
	//
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		hogboxDB::XmlClassWrapperPtr xmlWrapper;

		//allocate the correct wrapper type
		if(xmlNode->name == "HogBoxMaterial")
		{
			//create our object and it's xml wrapper.
			xmlWrapper = new HogBoxMaterialXmlWrapper(xmlNode);
		}else if(xmlNode->name == "FeatureLevel"){
			xmlWrapper = new FeatureLevelXmlWrapper(xmlNode);
		}

		if(!xmlWrapper){return NULL;}
		//did the wrapper alocate an object
		if(!xmlWrapper->getWrappedObject()){return NULL;}

		//if the wrapper was created properly then use it 
		//to deserialize our the xmlNode into it's wrapped object
		if(!xmlWrapper->deserialize(xmlNode))
		{
			//an error occured deserializing the xml node
			return NULL;
		}

		//hack pass the loaded featurelevel to the system info
		if(xmlNode->name == "FeatureLevel"){
			hogbox::SystemFeatureLevel* featureLevel = dynamic_cast<hogbox::SystemFeatureLevel*>(xmlWrapper->getWrappedObject());
			if(featureLevel){hogbox::SystemInfo::Instance()->SetFeatureLevel(featureLevel->getName(), featureLevel);}
		}
		return xmlWrapper;
	}
};

REGISTER_HOGBOXPLUGIN(HogBoxMaterial, HogBoxMaterialManager)
