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

#include <hogbox/HogBoxBase.h>
#include <string>
#include <vector>

#include "TrackedObject.h"

namespace hogboxVision {

//
//HogTracker
//Base class for any type of object tracker
//
class HOGBOXVIS_EXPORT HogTracker : public osg::Object
{
public:
	HogTracker(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	HogTracker(const HogTracker&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Object(hogboxVision, HogTracker);

	//init the tracker 
	virtual bool InitTracker();

	//updates and then callus update tracking
	virtual void Update();

	//
	virtual void UpdateTracking(){}

	//
	//Add a pre created object to the tracked objects  list
	void AddTrackedObject(TrackedObjectPtr object);

protected:

	virtual ~HogTracker(void);

protected:

	//the list of objects being tracked by the tracker
	std::vector<TrackedObjectPtr> _trackedObjects;

};

};
