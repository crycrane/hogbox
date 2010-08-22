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

	META_Box(hogboxVision, HogTracker);

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
	std::vector<TrackedObjectPtr> m_trackedObjects;

};

};
