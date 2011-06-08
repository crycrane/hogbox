#pragma once

#include <hogbox/HogBoxViewer.h>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for HogBoxViewer
//
class HogBoxViewerXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass HogBoxObject to be wrapped
	HogBoxViewerXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "HogBoxViewer")
	{
		//allocate the HogBoxObject
		hogbox::HogBoxViewer* hogboxViewer = new hogbox::HogBoxViewer();
		if(!hogboxViewer){return;}

		//add the attributes required to exposue the HogBoxObjects members to the xml wrapper

		//Position attribute Vec3
		m_xmlAttributes["ScreenID"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetScreenID,
																	&hogbox::HogBoxViewer::SetScreenID);

		//screen name
		m_xmlAttributes["Title"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,std::string>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetWindowName,
																	&hogbox::HogBoxViewer::SetWindowName);

		//window stuff
		m_xmlAttributes["Size"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,osg::Vec2>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetWindowSize,
																	&hogbox::HogBoxViewer::SetWindowSize);
		m_xmlAttributes["Corner"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,osg::Vec2>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetWindowCorner,
																	&hogbox::HogBoxViewer::SetWindowCorner);

		m_xmlAttributes["DoubleBuffer"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetDoubleBuffered,
																	&hogbox::HogBoxViewer::SetDoubleBuffered);

		m_xmlAttributes["VerticalSync"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetVSync,
																	&hogbox::HogBoxViewer::SetVSync);

		m_xmlAttributes["ColorBits"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetColorBits,
																	&hogbox::HogBoxViewer::SetColorBits);
		m_xmlAttributes["DepthBits"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetDepthBits,
																	&hogbox::HogBoxViewer::SetDepthBits);
		m_xmlAttributes["AlphaBits"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetAlphaBits,
																	&hogbox::HogBoxViewer::SetAlphaBits);
		m_xmlAttributes["StencilBits"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetStencilBits,
																	&hogbox::HogBoxViewer::SetStencilBits);

		m_xmlAttributes["FullScreen"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>(hogboxViewer,
																	&hogbox::HogBoxViewer::isFullScreen,
																	&hogbox::HogBoxViewer::SetFullScreen);
		m_xmlAttributes["Boarder"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetWindowDecoration,
																	&hogbox::HogBoxViewer::SetWindowDecoration);
		m_xmlAttributes["ShowCursor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>(hogboxViewer,
																	&hogbox::HogBoxViewer::isCursorVisible,
																	&hogbox::HogBoxViewer::SetCursorVisible);

		//
		m_xmlAttributes["ClearColor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,osg::Vec4>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetClearColor,
																	&hogbox::HogBoxViewer::SetClearColor);
		m_xmlAttributes["AASamples"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,int>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetAASamples,
																	&hogbox::HogBoxViewer::SetAASamples);

		//stereo stuff
		m_xmlAttributes["UseStereo"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>(hogboxViewer,
																	&hogbox::HogBoxViewer::isUsingStereo,
																	&hogbox::HogBoxViewer::SetUseStereo);

		m_xmlAttributes["ConvergenceDistance"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,float>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetStereoConvDistance,
																	&hogbox::HogBoxViewer::SetStereoConvDistance);
		m_xmlAttributes["EyeSeperation"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,float>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetStereoEyeSeperation,
																	&hogbox::HogBoxViewer::SetStereoEyeSeperation);
		m_xmlAttributes["SwapEyes"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetSwapEyes,
																	&hogbox::HogBoxViewer::SetSwapEyes);
		m_xmlAttributes["StereoMode"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,int>(hogboxViewer,
																	&hogbox::HogBoxViewer::GetStereoMode,
																	&hogbox::HogBoxViewer::SetStereoMode);
		
		//m_xmlAttributes["AutoRotateView"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>(hogboxViewer,
		//															&hogbox::HogBoxViewer::GetAutoRotateView,
		//															&hogbox::HogBoxViewer::SetAutoRotateView);
        m_xmlAttributes["DeviceOrientations"] = new hogboxDB::TypedXmlAttributeList<std::vector<std::string>, std::string>(&_orientationStrings);

		//store the hogboxobject as the wrapped object
		p_wrappedObject = hogboxViewer;
	}

	//overload deserialise so we can call CreateApp window after read
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}
		//cast to light
		hogbox::HogBoxViewer* viewer = dynamic_cast<hogbox::HogBoxViewer*>(p_wrappedObject.get());
		if(viewer)
		{
            //get the orientation flags from our string list
            hogbox::HogBoxViewer::DeviceOrientationFlags flags = GetFlagsFromStringList(_orientationStrings);
            viewer->SetDeviceOrientationFlags(flags);
			return viewer->CreateAppWindow();
		}

		return false;
	}


protected:

	virtual ~HogBoxViewerXmlWrapper(void){}
    
    hogbox::HogBoxViewer::DeviceOrientationFlags GetFlagsFromStringList(std::vector<std::string> list)
    {  
        if(list.size() == 0){
            return hogbox::HogBoxViewer::ALL_ORIENTATIONS;
        }
        hogbox::HogBoxViewer::DeviceOrientationFlags flags = 0;
        for(unsigned int i=0; i<list.size(); i++)
        {
            if(list[i] == "PORTRAIT"){
                flags |= hogbox::HogBoxViewer::PORTRAIT_ORIENTATION;
            }else if(list[i] == "PORTRAIT_UPSIDEDOWN"){
                flags |= hogbox::HogBoxViewer::PORTRAIT_UPSIDEDOWN_ORIENTATION;
            }else if(list[i] == "LANDSCAPE_LEFT"){
                flags |= hogbox::HogBoxViewer::LANDSCAPE_LEFT_ORIENTATION;
            }else if(list[i] == "LANDSCAPE_RIGHT"){
                flags |= hogbox::HogBoxViewer::LANDSCAPE_RIGHT_ORIENTATION;
            }else if(list[i] == "ALL"){
                flags |= hogbox::HogBoxViewer::ALL_ORIENTATIONS;
            }
        }
        return flags;
    }
    
protected:
    
    //local list of strings representing device orientation flags
    std::vector<std::string> _orientationStrings;

};

typedef osg::ref_ptr<HogBoxViewerXmlWrapper> HogBoxViewerXmlWrapperPtr;


