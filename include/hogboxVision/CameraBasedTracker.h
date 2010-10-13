#pragma once
#include "hogtracker.h"
//#include <cv.h>
#include "CameraCalibration.h"

namespace hogboxVision {

//
//CameraBasedTracker 
//Tracks objects in images using computer vision techniques
//Children of CameraBasedTracker are expected to have achild implementation
//of CameraCalibration
class HOGBOXVIS_EXPORT CameraBasedTracker : public HogTracker
{
public:
	CameraBasedTracker(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	CameraBasedTracker(const CameraBasedTracker&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogboxVision, CameraBasedTracker);


	//
	//Init camera based tracker, will set the width and height values
	//then load any camera calibration file using the child classes implementation
	bool InitTracker(int width=640, int height=480, const std::string& cameraCalibrationFile="");

	//update the tracker then call base update which calls update tracker
	virtual void Update(hogbox::ImagePtr image);
	
	//should be pure virtual but it complains?
	virtual void UpdateTracking(){}

	//
	//Load and allocate a new CameraCalibration from file
	virtual bool LoadCalibrationFromFile(const std::string& fileName){return false;}

protected:

	virtual ~CameraBasedTracker(void);

protected:

	//dimensions of image being input
	int m_videoWidth;
	int m_videoHeight;

	//pointer to the image currently being tracked
	osg::ref_ptr<osg::Image> p_image;
	
	//camera tracker keeps track of p_images previous
	//modified count so we know if we need to update the marker detection
	int m_modifiedCount;

	//all child camera based trackers are expected to allocate and initalise
	//their own implmentation of a CameraCalibration structure
	CameraCalibrationPtr m_cc;

};

typedef osg::ref_ptr<CameraBasedTracker> CameraBasedTrackerPtr;

};
