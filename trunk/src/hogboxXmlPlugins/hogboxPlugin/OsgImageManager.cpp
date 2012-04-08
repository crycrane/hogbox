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
#include "OsgImageXmlWrapper.h"


//
//Manages the loading of osg Image files using osgDB
//
class OsgImageManager : public hogboxDB::XmlClassManager
{
public:
	OsgImageManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Image", new OsgImageXmlWrapper());//"Xml definition of osg Image");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	OsgImageManager(const OsgImageManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}
	
	META_Object(hogboxDB, OsgImageManager)

protected:

	virtual ~OsgImageManager(void){
	}

};

//
//Will register the xml mamager plugin with the hogbox registry automatically
//then this dll is loaded
//REGISTER_HOGBOXPLUGIN(Image, OsgImageManager)

