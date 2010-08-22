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
		//allocate the HogBoxObject
		hogbox::HogBoxLight* hogboxLight = new hogbox::HogBoxLight();
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

		//All lights require a unique lightID propertey
		int lightID;
		if(!hogboxDB::getXmlPropertyValue(in, "lightID", lightID))
		{
			osg::notify(osg::WARN)	<< "XML ERROR: Nodes of classtype '" << in->name << "' should have a lightID property." <<std::endl 
									<< "                    i.e. <" << in->name << " lightID='0'>" << std::endl
									<< "                          The lightID is expected to be unique and start from 0" << std::endl;
			return false;
		}

		//cast to light
		hogbox::HogBoxLight* light = dynamic_cast<hogbox::HogBoxLight*>(p_wrappedObject.get());
		if(light)
		{
			light->SetGLID(lightID);
		}

		return true;
	}


protected:

	virtual ~HogBoxLightXmlWrapper(void){}

};

typedef osg::ref_ptr<HogBoxLightXmlWrapper> HogBoxObjectXmlWrapperPtr;


