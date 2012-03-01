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

#include "hogtracker.h"
#include "CameraCalibration.h"

namespace hogboxVision {

//
//CameraBasedTracker 
//Tracks objects in images using computer vision techniques
//Children of CameraBasedTracker are expected to have a child implementation
//of CameraCalibration
//
class HOGBOXVIS_EXPORT CameraBasedTracker : public HogTracker
{
public:
	CameraBasedTracker(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	CameraBasedTracker(const CameraBasedTracker&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Object(hogboxVision, CameraBasedTracker);


	//
	//Init camera based tracker, will set the width and height values
	//then load any camera calibration file using the child classes implementation
	bool InitTracker(int width=640, int height=480, const std::string& cameraCalibrationFile="");

	//update the tracker then call base update which calls update tracker
	//if the image modified count is different to ours
	virtual void Update(osg::ImagePtr image);
	
	//should be pure virtual but it complains?
	virtual void UpdateTracking(){}

	//
	//Load and allocate a new CameraCalibration from file
	virtual bool LoadCalibrationFromFile(const std::string& fileName){return false;}

protected:

	virtual ~CameraBasedTracker(void);

protected:

	//dimensions of image being input
	int _videoWidth;
	int _videoHeight;

	//pointer to the image from the camera currently being tracked
	osg::ref_ptr<osg::Image> p_image;
	
	//camera tracker keeps track of p_images previous
	//modified count so we know if we need to update the marker detection
	int _modifiedCount;

	//all child camera based trackers are expected to allocate and initalise
	//their own implmentation of a CameraCalibration structure
	CameraCalibrationPtr _cc;

};

typedef osg::ref_ptr<CameraBasedTracker> CameraBasedTrackerPtr;

};
