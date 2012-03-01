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

#include <osg/Texture3D>
#include "OsgTextureXmlWrapper.h"


//
//Xml wrapper for osg::Texture3D, used by OsgTextureManager
//to read write osg::Texture objects to xml (well there attributes)
//
//Xml Definition: 
//<Texture uniqueID='MyTexture1' type='Texture3D'>
//	<TextureImages count='1'>
//		<Image useID='MyImage1'/>
//	</TextureImages>
//</Texture>
//
//class 'type's supported are Texture2D, TextureRectangle, TextureCubeMap
//TextureImage is a pointer to an image node
//
class OsgTexture3DXmlWrapper : public OsgTextureXmlWrapper
{
public:
    
	//pass node representing the texture object
	OsgTexture3DXmlWrapper() 
    : OsgTextureXmlWrapper("Texture3D")
	{
        
	}
    
    //
    virtual osg::Object* allocateClassType(){return new osg::Texture3D();}
    
    //
    virtual XmlClassWrapper* cloneType(){return new OsgTexture3DXmlWrapper();}
    
protected:
    
	virtual ~OsgTexture3DXmlWrapper(void){
    
	}
    
    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        
        OsgTextureXmlWrapper::bindXmlAttributes();
        
        //osg::Texture3D* texture = dynamic_cast<osg::Texture3D*>(p_wrappedObject.get());
        
        //The image used by the texture2D
        /*_xmlAttributes["TextureImage"] = new hogboxDB::CallbackXmlClassPointer<osg::Texture2D,osg::Image>(texture,
                                                                                                          &osg::Texture2D::getImage,
                                                                                                          &osg::Texture2D::setImage);*/
    }
};

typedef osg::ref_ptr<OsgTexture3DXmlWrapper> OsgTexture3DXmlWrapperPtr;



