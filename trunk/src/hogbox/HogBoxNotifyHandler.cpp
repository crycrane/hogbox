#pragma once

#include <hogbox/HogBoxNotifyHandler.h>
#include <sstream>
#include <iomanip>

using namespace hogbox;

HogBoxNotifyHandler::HogBoxNotifyHandler(const std::string& outputFileName)
		: PlatformNotifyHandler()
{
	m_outputFile.open(outputFileName.c_str());
	if(m_outputFile.is_open())
	{
		//create our html styles for each severity
		HtmlStyle fatalStyle(osg::Vec3(0.9f,0.1f,0.1f));
		m_levelToHtmlStyleList[osg::FATAL] = fatalStyle;

		HtmlStyle warnStyle(osg::Vec3(0.1f,0.1f,0.9f));
		m_levelToHtmlStyleList[osg::WARN] = warnStyle;

		HtmlStyle noticeStyle(osg::Vec3(0.1f,0.9f,0.1f));
		m_levelToHtmlStyleList[osg::NOTICE] = noticeStyle;

		HtmlStyle infoStyle(osg::Vec3(0.0f,0.0f,0.0f));
		m_levelToHtmlStyleList[osg::INFO] = infoStyle;

		HtmlStyle debugStyle(osg::Vec3(0.3f,0.3f,0.3f));
		m_levelToHtmlStyleList[osg::DEBUG_INFO] = debugStyle;

		HtmlStyle debugFPStyle(osg::Vec3(0.6f,0.6f,0.6f));
		m_levelToHtmlStyleList[osg::DEBUG_FP] = debugFPStyle;

		//write our html header
		m_outputFile << "<html>" << std::endl << "<body>";
	}else{
		osg::notify(osg::WARN) << "HogBoxNotifyHandler::HogBoxNotifyHandler: ERROR: Failed to create Message log file '" << outputFileName << "'. The standard log output will be used." << std::endl;
	}
}

HogBoxNotifyHandler::~HogBoxNotifyHandler()
{
	//write our html closing tags
	if(m_outputFile.is_open())
	{
		//write our html closing tags
		m_outputFile << "</body>" << std::endl << "</html>" << std::endl;
		m_outputFile.close();
	}
}

//
//Write the notify message into a html file
//
void HogBoxNotifyHandler::notify(osg::NotifySeverity severity, const char *message)
{
	//place the line into html paragraph
	if(m_outputFile.is_open())
	{
		m_outputFile << "<PRE " << CreateHtmlStyleString(severity) << ">" << message << "</PRE>" << std::endl;
	
		//if file failed to open, then use the standard handler
		PlatformNotifyHandler::notify(severity, message);
	}else{
		//if file failed to open, then use the standard handler
		PlatformNotifyHandler::notify(severity, message);
	}
}

//
//returns a html style string based on severity to be inserted into <p> tag
//
const std::string HogBoxNotifyHandler::CreateHtmlStyleString(osg::NotifySeverity severity)
{
	//get style
	if(m_levelToHtmlStyleList.count(severity) > 0)
	{
		HtmlStyle htmlStyle = m_levelToHtmlStyleList[severity];

		//make our hex color string
		std::ostringstream colorStr(std::ostringstream::out);
		colorStr << "#" << std::setw( 2 ) << std::setfill( '0' ) << std::hex << std::uppercase << (int)(htmlStyle.fontColor.x()*255) 
						<< std::setw( 2 ) << std::setfill( '0' ) << std::hex << std::uppercase << (int)(htmlStyle.fontColor.y()*255) 
						<< std::setw( 2 ) << std::setfill( '0' ) << std::hex << std::uppercase << (int)(htmlStyle.fontColor.z()*255);
		std::ostringstream styleStr(std::ostringstream::out);
		styleStr << "style=\"font-family:" << htmlStyle.fontFamily << ";color:" << colorStr.str() << ";font-size:" << htmlStyle.fontSize << "px;\"";
		return styleStr.str();
	}

	return "";
}