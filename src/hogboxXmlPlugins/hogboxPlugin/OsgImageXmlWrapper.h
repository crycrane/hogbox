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

#include <osg/Image>
#include <osgDB/ReadFile>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for osg::Node, used by OsgNodeManager
//Nodes in this context are entire sub graphs representing
//a 3d model, probably exported from 3ds max etc.
//Becuase they are always groups the contructor allocates
//and osg::Group to store the node.
//
//Xml Definition: 
//<Image uniqueID='MyImage1'>
//	<File>MyModelFile.png</File>
//</Image>
//
//Nodes can use several methods for loading data, but the base
//Node class has no methods to store the data (i.e. fileName)
//OsgNodeXmlWrapper provides the means of temporarily storing
//info for conveniance load functions
//
class OsgImageXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:


	//pass node representing the texture object
	OsgImageXmlWrapper() 
			: hogboxDB::XmlClassWrapper("Image")
	{

	}

    //
    virtual osg::Object* allocateClassType(){return new osg::Image();}
    
    //
    virtual XmlClassWrapper* cloneType(){return new OsgImageXmlWrapper();}

	//overload deserialise
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//cast wrapped to Group
		osg::Image* image = dynamic_cast<osg::Image*>(p_wrappedObject.get());
		if(!image){return false;}

		if(!image->getFileName().empty())
		{
			//load the image file into a temp image,
			osg::ref_ptr<osg::Image> fileImage = hogbox::AssetManager::Inst()->GetOrLoadImage(image->getFileName());

			if(fileImage)
			{
				p_wrappedObject = fileImage;

			}else{
				osg::notify(osg::WARN) << "XML ERROR: Loading Image file '" << image->getFileName() << "', for Image Node '" << image->getName() << "'." << std::endl;
			}

			//release temp image
			fileImage = NULL;
		}
		return true;
	}


protected:

	virtual ~OsgImageXmlWrapper(void){}
    
    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        osg::Image* image = dynamic_cast<osg::Image*>(p_wrappedObject.get());
		_xmlAttributes["File"] = new hogboxDB::CallbackXmlAttribute<osg::Image,std::string>
                                ("File", image,
                                &osg::Image::getFileName,
                                &osg::Image::setFileName);
    }

};

typedef osg::ref_ptr<OsgImageXmlWrapper> OsgImageXmlWrapperPtr;


