#pragma once

#include <osg/Node>
#include <osg/Geode>
#include <osg/NodeVisitor>

#include <osg/StateSet>
#include <string>
#include <vector>

namespace hogbox
{

//
//Tracks the current state until the named geode is found
// once found the current state is taken to be that of the geode
//
class FindGeodeStateSetVisitor : public osg::NodeVisitor
{
public:

	FindGeodeStateSetVisitor(std::string geodeName, bool checkGeoms=false):
	  osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)        
    {
		_currentState =	NULL;

		//the name of the geode whos state we are interested in
		_searchName = geodeName;
		//the state of the search for geode
		_foundState = NULL;

		_checkGeoms = checkGeoms;

    }
    
    virtual void apply(osg::Node& node)
    {
		//capture any new state set and set as current
		osg::StateSet* state = node.getStateSet();
		if(state != NULL)
		{
			printf("StateFound %s\n", state->getName().c_str());
			_currentState = state;
		}

		//if it cast as geode, see if it's our geode 
		osg::Geode* geo = dynamic_cast<osg::Geode*>(&node);
		if (geo)
        {
			if(!_checkGeoms)
			{
				if(geo->getName().compare(_searchName) == 0)
				{
					_foundState = _currentState;
				}
			}else{
				//check geom names
				for(unsigned int i=0;i<geo->getNumDrawables(); i++)
				{
					osg::StateSet* geomState = geo->getDrawable(i)->getStateSet();
					if(geomState != NULL)
					{
						_currentState = geomState;
					}

					if(geo->getDrawable(i)->getName().compare(_searchName) == 0)
					{
						_foundState = _currentState;
					}
				}
			}
        }
        else
        {

           
        }
		traverse(node);
    }
    
	//check the drawable nodes names else just geodes
	bool _checkGeoms;

	//current state as we transverse tree
	osg::ref_ptr<osg::StateSet> _currentState;

	//the name of the geode whos state we are interested in
	std::string  _searchName;
	//the state of the search for geode
	osg::ref_ptr<osg::StateSet> _foundState;

};

}; //end hogbox namespace