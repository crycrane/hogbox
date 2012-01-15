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

#include <hogboxDB/XmlClassManager.h>
#include "OsgNodeXmlWrapper.h"

#include <osg/Node>

//
//Manages the loading of osg nodes
//
class OsgNodeManager : public hogboxDB::XmlClassManager
{
public:

	OsgNodeManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Node", "Xml definition of osg Node");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	OsgNodeManager(const OsgNodeManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxDB, OsgNodeManager)

protected:

	virtual ~OsgNodeManager(void)
	{
	}


	//
	//Create a HogBoxObject and read it's data in from disk
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		if(!xmlNode){return NULL;}

		hogboxDB::XmlClassWrapperPtr xmlWrapper;

		xmlWrapper = new OsgNodeXmlWrapper(xmlNode);

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

		//we're done deserialising the object
		return xmlWrapper;
	}

};

//
//Will register the xml mamager plugin with the hogbox registry automatically
//then this dll is loaded
REGISTER_HOGBOXPLUGIN(Node, OsgNodeManager)

