#pragma once

#include <osgDB/XmlParser>
#include <osgDB/FileNameUtils>

namespace hogboxDB {

	//Methods for deserialising Xml node strings into various standard and
	//osg datatypes, these include bool, int, float, Vec2, Vec3, Vec4. 
	//Can also handle moving through a list of space sperated values using
	//the stringStreamToType function. This will read in the required values for the
	//type, moving the string stream read position each time

	//
	//Return the next word in a stringstream, checking for exception
	//shifting the stringstream onto the next
	static const bool getNextStringStreamWord(std::stringstream& input, std::string& result, bool errorOnEndStream=false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask |= std::stringstream::eofbit;}
		input.exceptions ( exceptionMask );
		try {
			//read bool value
			input >> result;
		}
		catch (std::stringstream::failure e) {
			osg::notify(osg::DEBUG_INFO) << "getNextStringStreamWord: EXCEPTION: moving to next word in string stream = '" << input.str() << "'" << std::endl
										 << "The following exception message was thrown '" << e.what() << "'." << std::endl;
			return false;
		}
		return true;
	}

	//
	//convert the next word of the string stream to the result type
	//moving the position in the passed stream on to the next word
	static const bool stringStreamToType(std::stringstream& input, std::string& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask |= std::stringstream::eofbit;}
		input.exceptions ( exceptionMask );
		try {
			//read bool value
			input >> result;
		}
		catch (std::stringstream::failure e) {
			osg::notify(osg::DEBUG_INFO) << "getNextStringStreamWord: EXCEPTION: Passing string, from input string = '" << input.str() << "'" << std::endl
										 << "The following exception message was thrown '" << e.what() << "'." << std::endl;

			return false;
		}
		return true;
	}
	static const bool stringStreamToType(std::stringstream& input, bool& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask |= std::stringstream::eofbit;}
		input.exceptions ( exceptionMask );
		try {
			//read bool value
			input >> result;
		}
		catch (std::stringstream::failure e) {
			osg::notify(osg::DEBUG_INFO) << "getNextStringStreamWord: EXCEPTION: Passing bool, from input string = '" << input.str() << "'" << std::endl
										 << "The following exception message was thrown '" << e.what() << "'." << std::endl;

			return false;
		}
		return true;
	}
	static const bool stringStreamToType(std::stringstream& input, int& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask = std::stringstream::eofbit | std::stringstream::failbit | std::stringstream::badbit;}
		input.exceptions ( exceptionMask );
		try {
			//read int value
			input >> result;
		}
		catch (std::stringstream::failure e) {
			osg::notify(osg::DEBUG_INFO) << "stringStreamToType: EXCEPTION: Passing int, from input string = '" << input.str() << "'" << std::endl
										 << "The following exception message was thrown '" << e.what() << "'." << std::endl;

			return false;
		}
		return true;
	}
	static const bool stringStreamToType(std::stringstream& input, unsigned int& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask = std::stringstream::eofbit | std::stringstream::failbit | std::stringstream::badbit;}
		input.exceptions ( exceptionMask );
		try {
			//read int value
			input >> result;
		}
		catch (std::stringstream::failure e) {
			osg::notify(osg::DEBUG_INFO) << "stringStreamToType: EXCEPTION: Passing unsigned int, from input string = '" << input.str() << "'" << std::endl
										 << "The following exception message was thrown '" << e.what() << "'." << std::endl;
			return false;
		}
		return true;
	}
	static const bool stringStreamToType(std::stringstream& input, float& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask |= std::stringstream::eofbit;}
		input.exceptions ( exceptionMask );
		try {
			std::string word;
			//read word string
			input >> word;
			//convert to float with osg helper
			result = osg::asciiToFloat(word.c_str());
		}
		catch (std::stringstream::failure e) {
			osg::notify(osg::DEBUG_INFO) << "stringStreamToType: EXCEPTION: Passing float from input string input = '" << input.str() << "'."  << std::endl
										 << "                               The following exception message was thrown '" << e.what() << "'." << std::endl;
			return false;
		}
		return true;
	}
	static const bool stringStreamToType(std::stringstream& input, double& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask |= std::stringstream::eofbit;}
		input.exceptions ( exceptionMask );
		try {
			std::string word;
			//read word string
			input >> word;
			//convert to float with osg helper
			result = osg::asciiToDouble(word.c_str());
		}
		catch (std::stringstream::failure e) {
			osg::notify(osg::DEBUG_INFO) << "stringStreamToType: EXCEPTION: Passing double from input string input = '" << input.str() << "'."  << std::endl
										 << "                               The following exception message was thrown '" << e.what() << "'." << std::endl;
			return false;
		}
		return true;
	}
	static const bool stringStreamToType(std::stringstream& input, osg::Vec2& result, bool errorOnEndStream = false)
	{
		float x,y=0.0f;
		if(!hogboxDB::stringStreamToType(input, x,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, y,errorOnEndStream)){return false;}
		result.set(x,y);
		return true;
	}
	static const bool stringStreamToType(std::stringstream& input, osg::Vec3& result, bool errorOnEndStream = false)
	{
		float x,y,z=0.0f;
		if(!hogboxDB::stringStreamToType(input, x,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, y,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, z,errorOnEndStream)){return false;}
		result.set(x,y,z);
		return true;
	}
	static const bool stringStreamToType(std::stringstream& input, osg::Vec4& result, bool errorOnEndStream = false)
	{
		float x,y,z,w=0.0f;
		if(!hogboxDB::stringStreamToType(input, x,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, y,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, z,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, w,errorOnEndStream)){return false;}
		result.set(x,y,z,w);
		return true;
	}
	static const bool stringStreamToType(std::stringstream& input, osg::Matrix& result, bool errorOnEndStream = false)
	{
		float e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16=0.0f;
		if(!hogboxDB::stringStreamToType(input, e1,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e2,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e3,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e4,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e5,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e6,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e7,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e8,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e9,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e10,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e11,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e12,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e13,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e14,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e15,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, e16,errorOnEndStream)){return false;}
		result.set(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16);
		return true;
	}

	//
	//convert and return the input string as the result type
	static const bool asciiToType(const std::string& input, bool& result)
	{
		std::stringstream boolStr(input);
		return hogboxDB::stringStreamToType(boolStr, result);
	}
	static const bool asciiToType(const std::string& input, int& result)
	{
		std::stringstream intStr(input);
		return hogboxDB::stringStreamToType(intStr, result);
	}
	static const bool asciiToType(const std::string& input, unsigned int& result)
	{
		std::stringstream intStr(input);
		return hogboxDB::stringStreamToType(intStr, result);
	}
	static const bool asciiToType(const std::string& input, float& result)
	{
		std::stringstream floatStr(input);
		return hogboxDB::stringStreamToType(floatStr, result);
	}
	static const bool asciiToType(const std::string& input, double& result)
	{
		std::stringstream doubleStr(input);
		return hogboxDB::stringStreamToType(doubleStr, result);
	}

	//
	//Read each values of an ascii vec2 into a seperate string then use
	//osg::asciiToDouble to convert each value to a double to set on the
	//vec2
	static const bool asciiToType(const std::string& input, osg::Vec2& result)
	{
		std::stringstream vecStr(input);
		return hogboxDB::stringStreamToType(vecStr, result);
	}
	static const bool asciiToType(const std::string& input, osg::Vec3& result)
	{
		std::stringstream vecStr(input);
		return hogboxDB::stringStreamToType(vecStr, result);
	}
	static const bool asciiToType(const std::string& input, osg::Vec4& result)
	{
		std::stringstream vecStr(input);
		return hogboxDB::stringStreamToType(vecStr, result);
	}
	static const bool asciiToType(const std::string& input, osg::Matrix& result)
	{
		std::stringstream matrixStr(input);
		return hogboxDB::stringStreamToType(matrixStr, result);
	}


	//
	//return the value of an xml property as the passed type, returns false if propery does not exist
	//

	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, std::string& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			result = xmlNode->properties[propertyName];
			return true;
		}
		return false;
	}
	
	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, bool& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, int& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, unsigned int& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, float& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}
	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, double& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, osg::Vec2& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, osg::Vec3& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, osg::Vec4& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}
	static const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, osg::Matrix& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}


	//Read the content of a node as a certain type
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, std::string& result)
	{
		if(!xmlNode){return false;}
		result = xmlNode->contents;
		return true;
	}
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, bool& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, int& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, unsigned int& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, float& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, double& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, osg::Vec2& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, osg::Vec3& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, osg::Vec4& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static const bool getXmlContents(osgDB::XmlNode* xmlNode, osg::Matrix& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}

}; //end hogboxDB namespace