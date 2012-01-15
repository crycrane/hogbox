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

#include <hogbox/HogBoxLight.h>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for HogBoxObject, used by HogBoxObjectManager
//to read write HogBoxObjects to xml
//HogBoxObject must have loaded it's nodes/models before trying to
//set the parameters of any submeshes. To allow this meshes mappings
//read from from xml are loaded into a temp list. Then passed to hogboxobject
//once it is fully constructed
//
class HogBoxLightXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass HogBoxObject to be wrapped
	HogBoxLightXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "HogBoxLight")
	{
		
		//All lights require a unique lightID propertey to be passed to the contructor
		int lightID;
		if(!hogboxDB::getXmlPropertyValue(node, "lightID", lightID))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype '" << node->name << "' should have a lightID property." <<std::endl 
			<< "                    i.e. <" << node->name << " lightID='0'>" << std::endl
			<< "                          The lightID is expected to be unique and start from 0" << std::endl;
			//return;
		}
		
		//allocate the HogBoxObject
		hogbox::HogBoxLight* hogboxLight = new hogbox::HogBoxLight(lightID);
		if(!hogboxLight){return;}

		//add the attributes required to exposue the HogBoxObjects members to the xml wrapper

		//Position attribute Vec3
		m_xmlAttributes["Position"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxLight,osg::Vec4>(hogboxLight,
																	&hogbox::HogBoxLight::GetPosition,
																	&hogbox::HogBoxLight::SetPosition);

		//Colors
		m_xmlAttributes["Diffuse"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxLight,osg::Vec4>(hogboxLight,
																	&hogbox::HogBoxLight::GetDiffuse,
																	&hogbox::HogBoxLight::SetDiffuse);
		m_xmlAttributes["Ambient"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxLight,osg::Vec4>(hogboxLight,
																	&hogbox::HogBoxLight::GetAmbient,
																	&hogbox::HogBoxLight::SetAmbient);
		m_xmlAttributes["Specular"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxLight,osg::Vec4>(hogboxLight,
																	&hogbox::HogBoxLight::GetSpecular,
																	&hogbox::HogBoxLight::SetSpecular);

		//store the hogboxobject as the wrapped object
		p_wrappedObject = hogboxLight;
	}

	//overload deserialise to look for the required lightID property
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//cast to light
		hogbox::HogBoxLight* light = dynamic_cast<hogbox::HogBoxLight*>(p_wrappedObject.get());
		if(light)
		{
			//light->SetGLID(lightID);
		}

		return true;
	}


protected:

	virtual ~HogBoxLightXmlWrapper(void){}

};

typedef osg::ref_ptr<HogBoxLightXmlWrapper> HogBoxObjectXmlWrapperPtr;


