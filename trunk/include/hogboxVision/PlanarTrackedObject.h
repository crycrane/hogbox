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

#include "trackedobject.h"
#include <vector>
#include <osg/Vec2>

namespace hogboxVision {

//
//Planar tracked objects store a list 2d coords
//representing the corners of the planar object
//
class HOGBOXVIS_EXPORT PlanarTrackedObject : public TrackedObject
{
public:
	PlanarTrackedObject(TrackedDimensions dimensions=TRACKING_BOTH);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	PlanarTrackedObject(const PlanarTrackedObject&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Object(hogboxVision, PlanarTrackedObject);

	//set by implementations
	std::vector<osg::Vec2> GetCorners(){return _corners;}
	int GetDirection(){return _direction;}

	//get set the width
	float GetWidth(){return _width;}
	void SetWidth(float width){_width = width;}
	
protected:

	virtual ~PlanarTrackedObject();

protected:

	std::vector<osg::Vec2> _corners;
	int _direction; //which side is local up/y
	
	//width of planar quad in mm
	float _width;

};

};
