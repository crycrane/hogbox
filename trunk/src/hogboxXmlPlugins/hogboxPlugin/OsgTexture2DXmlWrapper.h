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

#include <osg/Texture2D>
#include "OsgTextureXmlWrapper.h"


//
//Xml wrapper for osg::Texture2D, used by OsgTextureManager
//to read write osg::Texture objects to xml (well there attributes)
//
//Xml Definition: 
//<Texture uniqueID='MyTexture1' type='Texture2D'>
//	<TextureImage>
//		<Image useID='MyImage1'/>
//	</TextureImage>
//</Texture>
//
//class 'type's supported are Texture2D, TextureRectangle, TextureCubeMap
//TextureImage is a pointer to an image node
//
class OsgTexture2DXmlWrapper : public OsgTextureXmlWrapper
{
public:

	//pass node representing the texture object
	OsgTexture2DXmlWrapper() 
        : OsgTextureXmlWrapper("Texture2D")
	{

	}

    //
    virtual osg::Object* allocateClassType(){return new osg::Texture2D();}
    
    //
    virtual XmlClassWrapper* cloneType(){return new OsgTexture2DXmlWrapper();}

protected:

	virtual ~OsgTexture2DXmlWrapper(void){

	}

    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        
        OsgTextureXmlWrapper::bindXmlAttributes();
        
        osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(p_wrappedObject.get());
        
        //The image used by the texture2D
        _xmlAttributes["TextureImage"] = new hogboxDB::CallbackXmlClassPointer<osg::Texture2D,osg::Image>(texture,
                                                                                                          &osg::Texture2D::getImage,
                                                                                                          &osg::Texture2D::setImage);
    }
};

typedef osg::ref_ptr<OsgTextureXmlWrapper> OsgTextureXmlWrapperPtr;


