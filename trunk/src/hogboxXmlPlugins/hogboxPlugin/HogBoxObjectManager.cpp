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


#include <hogboxDB/HogBoxRegistry.h>
#include <hogboxDB/XmlClassManager.h>
#include <hogboxDB/HogBoxManager.h>
#include <hogbox/HogBoxObject.h>

#include "HogBoxObjectXmlWrapper.h"
#include "MeshMappingXmlWrapper.h"

//
//Managers HogBoxObject nodes in an xml document
//
class HogBoxObjectManager : public hogboxDB::XmlClassManager
{
public:

	HogBoxObjectManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("HogBoxObject", new HogBoxObjectXmlWrapper());//"Xml definition of HogBoxObject.");
		SupportsClassType("MeshMapping", new MeshMappingXmlWrapper());//"Xml definition of MeshMapping. For defining the The state of the meshes in a HogBoxObject.");
	}
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxObjectManager(const HogBoxObjectManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Object(hogboxDB, HogBoxObjectManager)

protected:

	virtual ~HogBoxObjectManager(void){
	}
	
};

//
//Will register the xml mamager plugin with the hogbox registry automatically
//then this dll is loaded
//REGISTER_HOGBOXPLUGIN(HogBoxObject, HogBoxObjectManager)

