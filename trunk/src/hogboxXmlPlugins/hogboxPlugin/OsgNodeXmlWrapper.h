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

#include <osg/Node>
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
//<Node uniqueID='MyNode1'>
//	<File>MyModelFile.ive</File>
//</Node>
//
//Nodes can use several methods for loading data, but the base
//Node class has no methods to store the data (i.e. fileName)
//OsgNodeXmlWrapper provides the means of temporarily storing
//info for conveniance load functions
//
class OsgNodeXmlWrapper : public hogboxDB::XmlClassWrapper
{
public:


	//pass node representing the texture object
	OsgNodeXmlWrapper(osgDB::XmlNode* node) 
			: hogboxDB::XmlClassWrapper(node, "Node")
	{

		//allocate Group to repesent any loaded nodes
		osg::Group* group = new osg::Group();
		group->setName("OsgNodeRoot");

		//add the temporary wrapper attributes
		m_xmlAttributes["File"] = new hogboxDB::TypedXmlAttribute<std::string>(&_fileName);

		p_wrappedObject = group;
	}


	//overload deserialise
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in))
		{
			return false;
		}

		//cast wrapped to Group
		osg::Group* group = dynamic_cast<osg::Group*>(p_wrappedObject.get());
		if(!group){return false;}

		if(!_fileName.empty())
		{
			//load the file name using osgDB readNode
			osg::Node* fileNode = osgDB::readNodeFile(_fileName);

			//check it loaded ok
			if(!fileNode)
			{
				//if not try using findDataFile
				osg::notify(osg::WARN) << "XML ERROR: Loading node file '" << _fileName << "', for Node '" << group->getName() << "'." << std::endl;
				std::string tryFileName = osgDB::findDataFile( _fileName );
				
				if(!tryFileName.empty()){
					osg::notify(osg::WARN) << "           Found Alternative file'" << tryFileName << "', attempting to load," << std::endl;
					fileNode = osgDB::readNodeFile(tryFileName);
					if(!fileNode){return false;}
					_fileName = tryFileName;
				}else{
					return false;
				}
				
				//
				OSG_NOTICE << "XML Info: Loaded osg Node file '" << _fileName << "'." << std::endl;
			}
			//add the node to our base group
			group->addChild(fileNode);
		}
		return true;
	}


public:

	//public conveniance variables for loading assests
	//at deserialise time
	std::string _fileName;

protected:

	virtual ~OsgNodeXmlWrapper(void){}

};

typedef osg::ref_ptr<OsgNodeXmlWrapper> OsgNodeXmlWrapperPtr;


