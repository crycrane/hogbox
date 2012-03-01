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
#include <hogbox/HogBoxLight.h>

#include "HogBoxLightXmlWrapper.h"

//
//Managers HogBoxLight nodes in an xml document
//
class HogBoxLightManager : public hogboxDB::XmlClassManager
{
public:

	HogBoxLightManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("HogBoxLight", new HogBoxLightXmlWrapper());//"Xml definition of HogBoxLight");
	}
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxLightManager(const HogBoxLightManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Object(hogboxDB, HogBoxLightManager)

protected:

	virtual ~HogBoxLightManager(void){
	}
	
};

//
//Will register the xml mamager plugin with the hogbox registry automatically
//then this dll is loaded
//REGISTER_HOGBOXPLUGIN( HogBoxLight, HogBoxLightManager )

