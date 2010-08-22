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

		//add the temporary wrapper attributes
		m_xmlAttributes["File"] = new hogboxDB::TypedXmlAttribute<std::string>(&_fileName);

		p_wrappedObject = group;
	}


	//overload deserialise
	virtual bool deserialize(osgDB::XmlNode* in)  
	{
		if(!XmlClassWrapper::deserialize(in)){return false;}

		//cast wrapped to Group
		osg::Group* group = dynamic_cast<osg::Group*>(p_wrappedObject.get());
		if(!group){return false;}

		if(!_fileName.empty())
		{
			//load the file name using osgDB readNode
			osg::Node* fileNode = osgDB::readNodeFile(_fileName);

			//check it loaded ok
			if(fileNode){
				//add the node to our base group
				group->addChild(fileNode);
			}else{
				osg::notify(osg::WARN) << "XML ERROR: Loading node file '" << _fileName << "', for Node '" << group->getName() << "'." << std::endl;
			}
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


