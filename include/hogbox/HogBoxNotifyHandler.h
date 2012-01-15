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

#include <hogbox/Export.h>
#include <osg/Notify>
#include <osg/Vec3>

#include <map>
#include <fstream>



namespace hogbox {

#ifdef _WIN32
	typedef osg::WinDebugNotifyHandler PlatformNotifyHandler;
#else
	typedef osg::StandardNotifyHandler PlatformNotifyHandler;
#endif
	
//
//Redirects notify stream to a html file adding html color formating etc
//for each notify level. If we fail to create the m_outputFile stream then
//the StandardNotifyHandler handler is used
class HOGBOX_EXPORT HogBoxNotifyHandler : public PlatformNotifyHandler
{
public:
	HogBoxNotifyHandler(const std::string& outputFileName="./Data/MessageLog.html");

    void notify(osg::NotifySeverity severity, const char *message);

protected:

	~HogBoxNotifyHandler();

protected:

	//returns a html style string based on severity to be inserted into <p> tag
	const std::string CreateHtmlStyleString(osg::NotifySeverity severity);

protected:
	struct HtmlStyle{
		osg::Vec3 fontColor;
		std::string fontFamily;
		int fontSize;

		HtmlStyle(osg::Vec3 color=osg::Vec3(0.0,0.0,0.0), int size=15, std::string family="arial")
		{fontColor=color; fontFamily=family; fontSize=size;}
	};

	//map of notify levels to html styles
	typedef std::map<osg::NotifySeverity, HtmlStyle> NotifySeverityToHtmlStyleMap;
	typedef std::pair<osg::NotifySeverity, HtmlStyle> NotifySeverityToHtmlStylePair;

	NotifySeverityToHtmlStyleMap m_levelToHtmlStyleList;

	//our file output stream
	std::ofstream m_outputFile;
};

}; //end hogbox namespace