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



#include <hogboxDB/XmlClassManager.h>
#include "OsgNodeXmlWrapper.h"

//
//Manages the loading of osg nodes
//
class OsgNodeManager : public hogboxDB::XmlClassManager
{
public:

	OsgNodeManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Node", new OsgNodeXmlWrapper());//"Xml definition of osg Node");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	OsgNodeManager(const OsgNodeManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Object(hogboxDB, OsgNodeManager)

protected:

	virtual ~OsgNodeManager(void){
	}

};

//
//Will register the xml mamager plugin with the hogbox registry automatically
//then this dll is loaded
//REGISTER_HOGBOXPLUGIN(Node, OsgNodeManager)

