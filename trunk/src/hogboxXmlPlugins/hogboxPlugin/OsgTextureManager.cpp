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
#include "OsgTextureXmlWrapper.h"

#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>

//
//Manages the loading of the hogbox xml representation
//of osg Textures
//
class OsgTextureManager : public hogboxDB::XmlClassManager
{
public:

	OsgTextureManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Texture", "Xml definition of osg Texture");
		SupportsClassType("Texture2D", "Xml definition of osg Texture2D, inherited from Texture");
		SupportsClassType("TextureCubeMap", "Xml definition of osg TextureCubeMap, inherited from Texture");
		SupportsClassType("TextureRectangle", "Xml definition of osg TextureRectangle, inherited from Texture");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	OsgTextureManager(const OsgTextureManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Box(hogboxDB, OsgTextureManager)

protected:

	virtual ~OsgTextureManager(void)
	{
	}

	//
	//Create a HogBoxObject and read it's data in from disk
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
	{
		hogbox::ObjectPtr object;
		OsgTextureXmlWrapperPtr xmlWrapper;

		xmlWrapper = new OsgTextureXmlWrapper(xmlNode);

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

REGISTER_HOGBOXPLUGIN(Texture, OsgTextureManager)

