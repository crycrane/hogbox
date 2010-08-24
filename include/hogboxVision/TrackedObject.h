#pragma once

#include <hogboxVision/Export.h>
#include <hogbox/HogboxBase.h>

#include <string>
#include <osg/Matrix>

namespace hogboxVision {

class HogTracker;
class CARTKTracker;
class COpenCVPlanarPoseTracker;

//
//Base class for any object tracked by a tracker
//All tracked objects store 6 dof info, but have
//a flag indicating the which dimensions are being tracked
//i.e. pos only, rot only, both
class HOGBOXVIS_EXPORT TrackedObject : public osg::Object
{
public:

	//tracker is friend as only it can call a markers update (is this dumb? yes)
	friend class HogTracker;
	friend class CARTKTracker;
	friend class COpenCVPlanarPoseTracker;

	enum TrackedDimensions{
		TRACKING_POSITION,
		TRACKING_ROTATION,
		TRACKING_BOTH
	};

	TrackedObject(TrackedDimensions dimensions = TRACKING_BOTH);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	TrackedObject(const TrackedObject&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogboxVision, TrackedObject);


	virtual bool Init();

	//the the dimensions this marker can track
	TrackedDimensions GetTrackedDimensions(){return m_trackingDimensions;}

	//get pose
	osg::Matrix GetTransform(){return m_poseMatrix;}

	//is the object currently visible/being tracked
	bool IsObjectDetected(){return m_isDetected;}

protected:

	//
	virtual ~TrackedObject(void);

	//base update, called by a tracker
	//isDetected, indicates if the object was detected by the tracker calling update
	//pose, the pose matrix returned by the tracker if any
	bool UpdateMarker(bool isDetected, osg::Matrix pose);

	//the dimensions being tracked
	TrackedDimensions m_trackingDimensions;

	//the 4x4 pose matrix for the marker, relative
	//to the trackers camera
	osg::Matrix m_poseMatrix;

	//was the object detected in the last update of its tracker
	bool m_isDetected;

};

typedef osg::ref_ptr<TrackedObject> TrackedObjectPtr;

};