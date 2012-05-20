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

#include <osgDB/XmlParser>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#define USE_ASSETMANAGER 1

#if USE_ASSETMANAGER
#include <hogbox/AssetManager.h>
#endif

namespace hogboxDB {
    
    //
    //Helper to open an xml file and then try to find and return the specified node
    //if the node does not exist or the file doesn't open null is returned.
    static inline osg::ref_ptr<osgDB::XmlNode> openXmlFileAndReturnNode(const std::string& fileName, const std::string rootNodeName)
    {
        //allocate the document node
        osg::ref_ptr<osgDB::XmlNode> doc = new osgDB::XmlNode;
        osgDB::XmlNode* root = 0;
        
#if USE_ASSETMANAGER
        hogbox::XmlInputObjectPtr xmlObject = hogbox::AssetManager::Inst()->GetOrLoadXmlObject(fileName);
        if(!xmlObject.get()){
            OSG_FATAL << "hogboxDB: openXmlFileAndReturnNode: ERROR: Failed to Read XML File '" << fileName << "'." << std::endl;
            return NULL;            
        }
        

        if(!xmlObject->GetInput()){
            OSG_FATAL << "hogboxDB: openXmlFileAndReturnNode: ERROR: Failed to Open XML File '" << fileName << "'." << std::endl;
            return NULL;
        }
        
        //read the file into out document
        //osg::ref_ptr<osgDB::XmlNode> root = new osgDB::XmlNode;
        doc->read(xmlObject->GetInput());
#else
        std::string foundFile = osgDB::findDataFile(fileName);
        
        if (foundFile.empty())
        {
            OSG_FATAL << "hogboxDB: openXmlFileAndReturnNode: ERROR: Could Not Find File '" << fileName << "'." << std::endl;
            return NULL;
        }
        
        //open the file with xmlinput
        osgDB::XmlNode::Input input;
        input.open(foundFile);
        input.readAllDataIntoBuffer();
        
        if(!input){
            OSG_FATAL << "hogboxDB: openXmlFileAndReturnNode: ERROR: Failed to Open XML File '" << foundFile << "'." << std::endl;
            return NULL;
        }
        
        //read the file into out document
        doc->read(input);
#endif 
        
        //iterate over the document nodes and try and find a HogBoxDatabase node to
        //use as a root
        for(osgDB::XmlNode::Children::iterator itr = doc->children.begin();
            itr != doc->children.end() && !root;
            ++itr)
        {
            //OSG_ALWAYS << "NODE NAME '" << (*itr)->name << "'" << std::endl;
            if ((*itr)->name==rootNodeName){ root = (*itr);break;}
        }
        
        if (root == NULL)
        {
            OSG_FATAL << "XML Database Error: Opened Xml file '" << fileName << "'," << std::endl
                        << "                    but failed to find node named <" << rootNodeName << "> node." << std::endl;
            return NULL;
        }
        return root;
    }

	//Methods for deserialising Xml node strings into various standard and
	//osg datatypes, these include bool, int, float, Vec2, Vec3, Vec4. 
	//Can also handle moving through a list of space sperated values using
	//the stringStreamToType function. This will read in the required values for the
	//type, moving the string stream read position each time


	//
	//convert the next word of the string stream to the result type
	//moving the position in the passed stream on to the next word
	static inline const bool stringStreamToType(std::stringstream& input, std::string& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask |= std::stringstream::eofbit;}
		input.exceptions ( exceptionMask );
		//try {
			//read bool value
			input >> result;
		//}
		//catch (std::stringstream::failure e) {
		//	osg::notify(osg::DEBUG_INFO) << "getNextStringStreamWord: EXCEPTION: Passing string, from input string = '" << input.str() << "'" << std::endl
		//								 << "The following exception message was thrown '" << e.what() << "'." << std::endl;

		//	return false;
		//}
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, bool& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask |= std::stringstream::eofbit;}
		input.exceptions ( exceptionMask );
		//try {
			//read bool value
			input >> result;
		//}
		//catch (std::stringstream::failure e) {
		//	osg::notify(osg::DEBUG_INFO) << "getNextStringStreamWord: EXCEPTION: Passing bool, from input string = '" << input.str() << "'" << std::endl
		//								 << "The following exception message was thrown '" << e.what() << "'." << std::endl;

		//	return false;
		//}
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, int& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask = std::stringstream::eofbit | std::stringstream::failbit | std::stringstream::badbit;}
		input.exceptions ( exceptionMask );
		//try {
			//read int value
			input >> result;
		//}
		//catch (std::stringstream::failure e) {
		//	osg::notify(osg::DEBUG_INFO) << "stringStreamToType: EXCEPTION: Passing int, from input string = '" << input.str() << "'" << std::endl
		//								 << "The following exception message was thrown '" << e.what() << "'." << std::endl;

		//	return false;
		//}
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, unsigned int& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask = std::stringstream::eofbit | std::stringstream::failbit | std::stringstream::badbit;}
		input.exceptions ( exceptionMask );
		//try {
			//read int value
			input >> result;
		//}
		//catch (std::stringstream::failure e) {
		//	osg::notify(osg::DEBUG_INFO) << "stringStreamToType: EXCEPTION: Passing unsigned int, from input string = '" << input.str() << "'" << std::endl
		//								 << "The following exception message was thrown '" << e.what() << "'." << std::endl;
		//	return false;
		//}
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, float& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask |= std::stringstream::eofbit;}
		input.exceptions ( exceptionMask );
		//try {
            std::string word;
			//read word string
            input >> word;
			//convert to float with osg helper
			result = osg::asciiToFloat(word.c_str());
		//}
		//catch (std::stringstream::failure e) {
		//	osg::notify(osg::DEBUG_INFO) << "stringStreamToType: EXCEPTION: Passing float from input string input = '" << input.str() << "'."  << std::endl
		//								 << "                               The following exception message was thrown '" << e.what() << "'." << std::endl;
		//	return false;
		//}
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, double& result, bool errorOnEndStream = false)
	{
		std::ios_base::iostate exceptionMask = std::stringstream::failbit | std::stringstream::badbit;
		if(errorOnEndStream){exceptionMask |= std::stringstream::eofbit;}
		input.exceptions ( exceptionMask );
		//try {
			std::string word;
			//read word string
			input >> word;
			//convert to float with osg helper
			result = osg::asciiToDouble(word.c_str());
		//}
		//catch (std::stringstream::failure e) {
		//	osg::notify(osg::DEBUG_INFO) << "stringStreamToType: EXCEPTION: Passing double from input string input = '" << input.str() << "'."  << std::endl
		//								 << "                               The following exception message was thrown '" << e.what() << "'." << std::endl;
		//	return false;
		//}
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, osg::Vec2& result, bool errorOnEndStream = false)
	{
		float x,y=0.0f;
		if(!hogboxDB::stringStreamToType(input, x,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, y,errorOnEndStream)){return false;}
		result.set(x,y);
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, osg::Vec3& result, bool errorOnEndStream = false)
	{
		float x,y,z=0.0f;
		if(!hogboxDB::stringStreamToType(input, x,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, y,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, z,errorOnEndStream)){return false;}
		result.set(x,y,z);
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, osg::Vec4& result, bool errorOnEndStream = false)
	{
		float x,y,z,w=0.0f;
		if(!hogboxDB::stringStreamToType(input, x,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, y,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, z,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, w,errorOnEndStream)){return false;}
		result.set(x,y,z,w);
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, osg::Quat& result, bool errorOnEndStream = false)
	{
		float x,y,z,w=0.0f;
		if(!hogboxDB::stringStreamToType(input, x,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, y,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, z,true)){return false;}
		if(!hogboxDB::stringStreamToType(input, w,errorOnEndStream)){return false;}
		result.set(x,y,z,w);
		return true;
	}
	static inline const bool stringStreamToType(std::stringstream& input, osg::Matrix& result, bool errorOnEndStream = false)
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
	static inline const bool asciiToType(const std::string& input, bool& result)
	{
		std::stringstream boolStr(input);
		return hogboxDB::stringStreamToType(boolStr, result);
	}
	static inline const bool asciiToType(const std::string& input, int& result)
	{
		std::stringstream intStr(input);
		return hogboxDB::stringStreamToType(intStr, result);
	}
	static inline const bool asciiToType(const std::string& input, unsigned int& result)
	{
		std::stringstream intStr(input);
		return hogboxDB::stringStreamToType(intStr, result);
	}
	static inline const bool asciiToType(const std::string& input, float& result)
	{
		std::stringstream floatStr(input);
		return hogboxDB::stringStreamToType(floatStr, result);
	}
	static inline const bool asciiToType(const std::string& input, double& result)
	{
		std::stringstream doubleStr(input);
		return hogboxDB::stringStreamToType(doubleStr, result);
	}

	//
	//Read each values of an ascii vec2 into a seperate string then use
	//osg::asciiToDouble to convert each value to a double to set on the
	//vec2
	static inline const bool asciiToType(const std::string& input, osg::Vec2& result)
	{
		std::stringstream vecStr(input);
		return hogboxDB::stringStreamToType(vecStr, result);
	}
	static inline const bool asciiToType(const std::string& input, osg::Vec3& result)
	{
		std::stringstream vecStr(input);
		return hogboxDB::stringStreamToType(vecStr, result);
	}
	static inline const bool asciiToType(const std::string& input, osg::Vec4& result)
	{
		std::stringstream vecStr(input);
		return hogboxDB::stringStreamToType(vecStr, result);
	}
	static inline const bool asciiToType(const std::string& input, osg::Quat& result)
	{
		std::stringstream vecStr(input);
		return hogboxDB::stringStreamToType(vecStr, result);
	}
	static inline const bool asciiToType(const std::string& input, osg::Matrix& result)
	{
		std::stringstream matrixStr(input);
		return hogboxDB::stringStreamToType(matrixStr, result);
	}
    
    


	//
	//return the value of an xml property as the passed type, returns false if propery does not exist
	//

	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, std::string& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			result = xmlNode->properties[propertyName];
			return true;
		}
		return false;
	}
	
	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, bool& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, int& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, unsigned int& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, float& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}
	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, double& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, osg::Vec2& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, osg::Vec3& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}

	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, osg::Vec4& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}
    
	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, osg::Quat& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}
    
	static inline const bool getXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, osg::Matrix& result)
	{
		if(!xmlNode){return false;}
		if(xmlNode->properties.count(propertyName) > 0)
		{
			return hogboxDB::asciiToType(xmlNode->properties[propertyName], result);
		}
		return false;
	}


	//Read the content of a node as a certain type
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, std::string& result)
	{
		if(!xmlNode){return false;}
		result = xmlNode->contents;
		return true;
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, bool& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, int& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, unsigned int& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, float& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, double& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, osg::Vec2& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, osg::Vec3& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, osg::Vec4& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, osg::Quat& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
	static inline const bool getXmlContents(osgDB::XmlNode* xmlNode, osg::Matrix& result)
	{
		if(!xmlNode){return false;}
		return hogboxDB::asciiToType(xmlNode->contents, result);
	}
    
    //
    //Write funcs
    
    static inline const bool typeToStringStream(const std::string& input, std::stringstream& result)
	{
        result << input;
        return true;
    }
    static inline const bool typeToStringStream(const bool& input, std::stringstream& result)
	{
        result << input;
        return true;
    }
    static inline const bool typeToStringStream(const int& input, std::stringstream& result)
	{
        result << input;
        return true;
    }
    static inline const bool typeToStringStream(const unsigned int& input, std::stringstream& result)
	{
        result << input;
        return true;
    }
    static inline const bool typeToStringStream(const float& input, std::stringstream& result)
	{
        result << input;
        return true;
    }
    static inline const bool typeToStringStream(const double& input, std::stringstream& result)
	{
        result << input;
        return true;
    }
    static inline const bool typeToStringStream(const osg::Vec2& input, std::stringstream& result)
	{
        if(!typeToStringStream(input.x(), result)){return false;}result << " ";
        if(!typeToStringStream(input.y(), result)){return false;}
        return true;
    }
    static inline const bool typeToStringStream(const osg::Vec3& input, std::stringstream& result)
	{
        if(!typeToStringStream(input.x(), result)){return false;}result << " ";
        if(!typeToStringStream(input.y(), result)){return false;}result << " ";
        if(!typeToStringStream(input.z(), result)){return false;}
        return true;
    }
    static inline const bool typeToStringStream(const osg::Vec4& input, std::stringstream& result)
	{
        if(!typeToStringStream(input.x(), result)){return false;}result << " ";
        if(!typeToStringStream(input.y(), result)){return false;}result << " ";
        if(!typeToStringStream(input.z(), result)){return false;}result << " ";
        if(!typeToStringStream(input.w(), result)){return false;}
        return true;
    }
    static inline const bool typeToStringStream(const osg::Quat& input, std::stringstream& result)
	{
        if(!typeToStringStream(input.x(), result)){return false;}result << " ";
        if(!typeToStringStream(input.y(), result)){return false;}result << " ";
        if(!typeToStringStream(input.z(), result)){return false;}result << " ";
        if(!typeToStringStream(input.w(), result)){return false;}
        return true;
    }
    static inline const bool typeToStringStream(const osg::Matrix& input, std::stringstream& result)
	{
        if(!typeToStringStream(input(0,0), result)){return false;}result << " ";
        if(!typeToStringStream(input(0,1), result)){return false;}result << " ";
        if(!typeToStringStream(input(0,2), result)){return false;}result << " ";
        if(!typeToStringStream(input(0,3), result)){return false;}result << " ";
        
        if(!typeToStringStream(input(1,0), result)){return false;}result << " ";
        if(!typeToStringStream(input(1,1), result)){return false;}result << " ";
        if(!typeToStringStream(input(1,2), result)){return false;}result << " ";
        if(!typeToStringStream(input(1,3), result)){return false;}result << " ";
        
        if(!typeToStringStream(input(2,0), result)){return false;}result << " ";
        if(!typeToStringStream(input(2,1), result)){return false;}result << " ";
        if(!typeToStringStream(input(2,2), result)){return false;}result << " ";
        if(!typeToStringStream(input(2,3), result)){return false;}result << " ";
        
        if(!typeToStringStream(input(3,0), result)){return false;}result << " ";
        if(!typeToStringStream(input(3,1), result)){return false;}result << " ";
        if(!typeToStringStream(input(3,2), result)){return false;}result << " ";
        if(!typeToStringStream(input(3,3), result)){return false;}
        return true;
    }

    static inline const std::string typeToAscii(const std::string& input)
	{
		std::stringstream ss("");
		hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const bool& input)
	{
		std::stringstream ss;
		hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const int& input)
	{
		std::stringstream ss;
		hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const unsigned int& input)
	{
		std::stringstream ss;
        hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const float& input)
	{
		std::stringstream ss;
		hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const double& input)
	{
		std::stringstream ss;
		hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const osg::Vec2& input)
	{
		std::stringstream ss;
		hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const osg::Vec3& input)
	{
		std::stringstream ss;
		hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const osg::Vec4& input)
	{
		std::stringstream ss;
		hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const osg::Quat& input)
	{
		std::stringstream ss;
		hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    static inline const std::string typeToAscii(const osg::Matrix& input)
	{
		std::stringstream ss;
        hogboxDB::typeToStringStream(input, ss);
        return ss.str();
	}
    
	//
	//set am xml nodes property to ascii representation of passed value
	//
    
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const std::string& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const bool& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const int& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const unsigned int& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const float& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const double& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const osg::Vec2& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const osg::Vec3& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const osg::Vec4& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const osg::Quat& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlPropertyValue(osgDB::XmlNode* xmlNode, const std::string& propertyName, const osg::Matrix& input)
	{
		if(!xmlNode){return false;}
		xmlNode->properties[propertyName] = hogboxDB::typeToAscii(input);
		return true;
	}
    
    //write xml content to type
    
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const std::string& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const bool& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const int& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const unsigned int& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const float& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const double& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const osg::Vec2& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const osg::Vec3& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const osg::Vec4& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const osg::Quat& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}
	static inline const bool setXmlContents(osgDB::XmlNode* xmlNode, const osg::Matrix& input)
	{
		if(!xmlNode){return false;}
		xmlNode->contents = hogboxDB::typeToAscii(input);
		return true;
	}

}; //end hogboxDB namespace