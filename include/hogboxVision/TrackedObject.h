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

	META_Object(hogboxVision, TrackedObject);


	//init passing the name of a config/marker file
	virtual bool Init(const std::string& config);

	//the the dimensions this marker can track
	TrackedDimensions GetTrackedDimensions(){return _trackingDimensions;}

	//get pose
	osg::Matrix GetTransform(){return _poseMatrix;}
	
	//our we tracking this object
	void SetEnabled(const bool& enabled);
	const bool& GetEnabled()const;

	//is the object currently visible/being tracked
	bool IsObjectDetected(){return _isDetected;}
	
	float GetConfidence(){return _confidence;}
	
	//base update, called by a tracker
	//isDetected, indicates if the object was detected by the tracker calling update
	//pose, the pose matrix returned by the tracker if any
	virtual bool UpdateMarker(bool isDetected, osg::Matrix pose);

protected:

	//
	virtual ~TrackedObject(void);


	//the dimensions being tracked
	TrackedDimensions _trackingDimensions;

	//the 4x4 pose matrix for the marker, relative
	//to the trackers camera
	osg::Matrix _poseMatrix;
	
	//our we trying to detect this object
	bool _isEnabled;

	//was the object detected in the last update of its tracker
	bool _isDetected;
	
	//the confidence of the tracked object i.e. are we getting a good signal/match, 0-1 1 being total confidence
	float _confidence;

};

typedef osg::ref_ptr<TrackedObject> TrackedObjectPtr;

};