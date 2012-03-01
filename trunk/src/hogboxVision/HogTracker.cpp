#include <hogboxVision/HogTracker.h>

#include <stdio.h>
#include <stdarg.h>

using namespace hogboxVision;

HogTracker::HogTracker(void)
	: osg::Object()
{
}

HogTracker::~HogTracker(void)
{
	_trackedObjects.clear();
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
HogTracker::HogTracker(const HogTracker& tracker,const osg::CopyOp& copyop)
	: osg::Object(tracker, copyop)
{
}


//init the tracker
bool HogTracker::InitTracker()
{
	return true;
}

//
//update the tracker
//
void HogTracker::Update()
{

	//update the tracking
	this->UpdateTracking();

}

//
//Add a pre created object to the tracked objects  list
//
void HogTracker::AddTrackedObject(TrackedObjectPtr object)
{
	if(!object.get()){return;}
	_trackedObjects.push_back(object);
}
