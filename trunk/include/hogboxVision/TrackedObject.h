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


	//init passing the name of a config/marker file
	virtual bool Init(const std::string& config);

	//the the dimensions this marker can track
	TrackedDimensions GetTrackedDimensions(){return m_trackingDimensions;}

	//get pose
	osg::Matrix GetTransform(){return m_poseMatrix;}
	
	//our we tracking this object
	void SetEnabled(const bool& enabled);
	const bool& GetEnabled()const;

	//is the object currently visible/being tracked
	bool IsObjectDetected(){return m_isDetected;}
	
	float GetConfidence(){return m_confidence;}
	
	//base update, called by a tracker
	//isDetected, indicates if the object was detected by the tracker calling update
	//pose, the pose matrix returned by the tracker if any
	bool UpdateMarker(bool isDetected, osg::Matrix pose);

protected:

	//
	virtual ~TrackedObject(void);


	//the dimensions being tracked
	TrackedDimensions m_trackingDimensions;

	//the 4x4 pose matrix for the marker, relative
	//to the trackers camera
	osg::Matrix m_poseMatrix;
	
	//our we trying to detect this object
	bool m_isEnabled;

	//was the object detected in the last update of its tracker
	bool m_isDetected;
	
	//the confidence of the tracked object i.e. are we getting a good signal/match, 0-1 1 being total confidence
	float m_confidence;

};

typedef osg::ref_ptr<TrackedObject> TrackedObjectPtr;

};