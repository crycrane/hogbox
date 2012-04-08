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
#include "OsgTextureXmlWrapper.h"
#include "OsgTexture2DXmlWrapper.h"
#include "OsgTextureRectangleXmlWrapper.h"
#include "OsgTexture3DXmlWrapper.h"
#include "OsgTextureCubeXmlWrapper.h"

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
        //should I have individual wrappers here?
		SupportsClassType("Texture", new OsgTextureXmlWrapper("Texture"));//"Xml definition of osg Texture");
		SupportsClassType("Texture2D", new OsgTexture2DXmlWrapper());//"Xml definition of osg Texture2D, inherited from Texture");
		SupportsClassType("TextureCubeMap", new OsgTextureCubeXmlWrapper());//"Xml definition of osg TextureCubeMap, inherited from Texture");
		SupportsClassType("TextureRectangle", new OsgTextureCubeXmlWrapper());//"Xml definition of osg TextureRectangle, inherited from Texture");
		SupportsClassType("Texture3D", new OsgTexture3DXmlWrapper());//"Xml definition of osg TextureRectangle, inherited from Texture");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	OsgTextureManager(const OsgTextureManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Object(hogboxDB, OsgTextureManager)

protected:

	virtual ~OsgTextureManager(void){
	}

};

//REGISTER_HOGBOXPLUGIN(Texture, OsgTextureManager)

