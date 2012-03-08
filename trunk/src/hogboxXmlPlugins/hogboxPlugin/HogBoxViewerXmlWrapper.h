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

#include <hogbox/HogBoxViewer.h>
#include <hogboxDB/XmlClassWrapper.h>


//
//Xml wrapper for HogBoxViewer
//
class HogBoxViewerXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:

	//pass HogBoxObject to be wrapped
	HogBoxViewerXmlWrapper() 
			: hogboxDB::XmlClassWrapper("HogBoxViewer")
	{

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

    //
    virtual osg::Object* allocateClassType(){return new hogbox::HogBoxViewer();}
    
    //
    virtual XmlClassWrapper* cloneType(){return new HogBoxViewerXmlWrapper();} 

protected:

	virtual ~HogBoxViewerXmlWrapper(void){}
    
    //
    //Bind the xml attributes for the wrapped object
    virtual void bindXmlAttributes(){
        hogbox::HogBoxViewer* hogboxViewer = dynamic_cast<hogbox::HogBoxViewer*>(p_wrappedObject.get());
        
		//Position attribute Vec3
		_xmlAttributes["ScreenID"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>("ScreenID", hogboxViewer,
                                                                                                           &hogbox::HogBoxViewer::GetScreenID,
                                                                                                           &hogbox::HogBoxViewer::SetScreenID);
        
		//screen name
		_xmlAttributes["Title"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,std::string>("Title", hogboxViewer,
                                                                                                       &hogbox::HogBoxViewer::GetWindowName,
                                                                                                       &hogbox::HogBoxViewer::SetWindowName);
        
		//window stuff
		_xmlAttributes["Size"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,osg::Vec2>("Size", hogboxViewer,
                                                                                                    &hogbox::HogBoxViewer::GetWindowSize,
                                                                                                    &hogbox::HogBoxViewer::SetWindowSize);
		_xmlAttributes["Corner"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,osg::Vec2>("Corner", hogboxViewer,
                                                                                                      &hogbox::HogBoxViewer::GetWindowCorner,
                                                                                                      &hogbox::HogBoxViewer::SetWindowCorner);
        
		_xmlAttributes["DoubleBuffer"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>("DoubleBuffer", hogboxViewer,
                                                                                                       &hogbox::HogBoxViewer::GetDoubleBuffered,
                                                                                                       &hogbox::HogBoxViewer::SetDoubleBuffered);
        
		_xmlAttributes["VerticalSync"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>("VerticalSync", hogboxViewer,
                                                                                                       &hogbox::HogBoxViewer::GetVSync,
                                                                                                       &hogbox::HogBoxViewer::SetVSync);
        
		_xmlAttributes["ColorBits"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>("ColorBits", hogboxViewer,
                                                                                                            &hogbox::HogBoxViewer::GetColorBits,
                                                                                                            &hogbox::HogBoxViewer::SetColorBits);
		_xmlAttributes["DepthBits"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>("DepthBits", hogboxViewer,
                                                                                                            &hogbox::HogBoxViewer::GetDepthBits,
                                                                                                            &hogbox::HogBoxViewer::SetDepthBits);
		_xmlAttributes["AlphaBits"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>("DepthBits", hogboxViewer,
                                                                                                            &hogbox::HogBoxViewer::GetAlphaBits,
                                                                                                            &hogbox::HogBoxViewer::SetAlphaBits);
		_xmlAttributes["StencilBits"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,unsigned int>("StencilBits", hogboxViewer,
                                                                                                              &hogbox::HogBoxViewer::GetStencilBits,
                                                                                                              &hogbox::HogBoxViewer::SetStencilBits);
        
		_xmlAttributes["FullScreen"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>("FullScreen", hogboxViewer,
                                                                                                     &hogbox::HogBoxViewer::isFullScreen,
                                                                                                     &hogbox::HogBoxViewer::SetFullScreen);
		_xmlAttributes["Boarder"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>("Boarder", hogboxViewer,
                                                                                                  &hogbox::HogBoxViewer::GetWindowDecoration,
                                                                                                  &hogbox::HogBoxViewer::SetWindowDecoration);
		_xmlAttributes["ShowCursor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>("ShowCursor", hogboxViewer,
                                                                                                     &hogbox::HogBoxViewer::isCursorVisible,
                                                                                                     &hogbox::HogBoxViewer::SetCursorVisible);
        
		//
		_xmlAttributes["ClearColor"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,osg::Vec4>("ClearColor", hogboxViewer,
                                                                                                          &hogbox::HogBoxViewer::GetClearColor,
                                                                                                          &hogbox::HogBoxViewer::SetClearColor);
		_xmlAttributes["AASamples"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,int>("AASamples", hogboxViewer,
                                                                                                   &hogbox::HogBoxViewer::GetAASamples,
                                                                                                   &hogbox::HogBoxViewer::SetAASamples);
        
		//stereo stuff
		_xmlAttributes["UseStereo"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>("UseStereo", hogboxViewer,
                                                                                                    &hogbox::HogBoxViewer::isUsingStereo,
                                                                                                    &hogbox::HogBoxViewer::SetUseStereo);
        
		_xmlAttributes["ConvergenceDistance"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,float>("ConvergenceDistance", hogboxViewer,
                                                                                                               &hogbox::HogBoxViewer::GetStereoConvDistance,
                                                                                                               &hogbox::HogBoxViewer::SetStereoConvDistance);
		_xmlAttributes["EyeSeperation"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,float>("EyeSeperation", hogboxViewer,
                                                                                                         &hogbox::HogBoxViewer::GetStereoEyeSeperation,
                                                                                                         &hogbox::HogBoxViewer::SetStereoEyeSeperation);
		_xmlAttributes["SwapEyes"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>("SwapEyes", hogboxViewer,
                                                                                                   &hogbox::HogBoxViewer::GetSwapEyes,
                                                                                                   &hogbox::HogBoxViewer::SetSwapEyes);
		_xmlAttributes["StereoMode"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,int>("StereoMode", hogboxViewer,
                                                                                                    &hogbox::HogBoxViewer::GetStereoMode,
                                                                                                    &hogbox::HogBoxViewer::SetStereoMode);
		
		//_xmlAttributes["AutoRotateView"] = new hogboxDB::CallbackXmlAttribute<hogbox::HogBoxViewer,bool>(hogboxViewer,
		//															&hogbox::HogBoxViewer::GetAutoRotateView,
		//															&hogbox::HogBoxViewer::SetAutoRotateView);
        _xmlAttributes["DeviceOrientations"] = new hogboxDB::TypedXmlAttributeList<std::vector<std::string>, std::string>("DeviceOrientations", &_orientationStrings);
    }
    
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


