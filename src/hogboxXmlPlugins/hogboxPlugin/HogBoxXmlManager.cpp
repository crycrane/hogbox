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


#include "HogBoxViewerXmlWrapper.h"
#include "HogBoxObjectXmlWrapper.h"
#include "MeshMappingXmlWrapper.h"
#include "HogBoxMaterialXmlWrapper.h"
#include "FeatureLevelXmlWrapper.h"
#include "HogBoxLightXmlWrapper.h"
#include "OsgTextureXmlWrapper.h"
#include "OsgTexture2DXmlWrapper.h"
#include "OsgTextureRectangleXmlWrapper.h"
#include "OsgTexture3DXmlWrapper.h"
#include "OsgTextureCubeXmlWrapper.h"
#include "OsgImageXmlWrapper.h"
#include "OsgNodeXmlWrapper.h"
#include "OsgShaderXmlWrapper.h"
#include "OsgUniformXmlWrapper.h"

//
//Manages the loading of the hogbox xml representation
//of osg Textures
//
class HogBoxXmlManager : public hogboxDB::XmlClassManager
{
public:

	HogBoxXmlManager(void) : hogboxDB::XmlClassManager()
	{
        OSG_FATAL << "Register HogBoxXmlManager" << std::endl;
		SupportsClassType("HogBoxViewer", new HogBoxViewerXmlWrapper());//"Xml definition of HogBoxViewer");
		SupportsClassType("HogBoxObject", new HogBoxObjectXmlWrapper());//"Xml definition of HogBoxObject.");
		SupportsClassType("MeshMapping", new MeshMappingXmlWrapper());//"Xml definition of MeshMapping. For defining the The state of the meshes in a HogBoxObject.");
        SupportsClassType("HogBoxMaterial", new HogBoxMaterialXmlWrapper());//"Xml definition of HogBoxMaterial");
		SupportsClassType("FeatureLevel", new FeatureLevelXmlWrapper());//"Xml definition of SystemFeatureLevel");
        SupportsClassType("HogBoxLight", new HogBoxLightXmlWrapper());//"Xml definition of HogBoxLight");
        SupportsClassType("Texture", new OsgTextureXmlWrapper("Texture"));//"Xml definition of osg Texture");
        SupportsClassType("Texture2D", new OsgTexture2DXmlWrapper());//"Xml definition of osg Texture2D, inherited from Texture");
        SupportsClassType("TextureCubeMap", new OsgTextureCubeXmlWrapper());//"Xml definition of osg TextureCubeMap, inherited from Texture");
        SupportsClassType("TextureRectangle", new OsgTextureCubeXmlWrapper());//"Xml definition of osg TextureRectangle, inherited from Texture");
        SupportsClassType("Texture3D", new OsgTexture3DXmlWrapper());//"Xml definition of osg TextureRectangle, inherited from Texture");
        SupportsClassType("Image", new OsgImageXmlWrapper());//"Xml definition of osg Image");
        SupportsClassType("Node", new OsgNodeXmlWrapper());//"Xml definition of osg Node");
        SupportsClassType("Shader", new OsgShaderXmlWrapper());//"Xml definition of osg Shader");
		SupportsClassType("Uniform", new OsgUniformXmlWrapper());//"Xml definition of osg uniform");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogBoxXmlManager(const HogBoxXmlManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Object(hogboxDB, HogBoxXmlManager)

protected:

	virtual ~HogBoxXmlManager(void){
	}

	//
	//
	virtual hogboxDB::XmlClassWrapperPtr readObjectFromXmlNode(osgDB::XmlNode* xmlNode)
	{
		hogboxDB::XmlClassWrapperPtr xmlWrapper = XmlClassManager::readObjectFromXmlNode(xmlNode);
        
		//hack pass the loaded featurelevel to the system info
		if(xmlNode->name == "FeatureLevel"){
			hogbox::SystemFeatureLevel* featureLevel = dynamic_cast<hogbox::SystemFeatureLevel*>(xmlWrapper->getWrappedObject());
			if(featureLevel){hogbox::SystemInfo::Inst()->SetFeatureLevel(featureLevel->getName(), featureLevel);}
		}
		return xmlWrapper;
	}
};

REGISTER_HOGBOXPLUGIN(hogbox, HogBoxXmlManager)

