#pragma once
//Created by T.Hogarth, HogBox


#include <hogbox/HogBoxBase.h>
#include <string>

namespace hogboxVision {

//CameraCalibration
//base type for any type of camera calibration structure
//i.e. opencv cam matrix, artk arparam structure
//the intension is to allow access to any common variables
//
class CameraCalibration : public osg::Object
{
public:
	CameraCalibration(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	CameraCalibration(const CameraCalibration& calib,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogboxVision,CameraCalibration);

	virtual void ChangeSize(int width, int height);

	//all camera calibrations should try to support
	//a load from a single file
	virtual bool LoadCalibrationFromFile(const std::string& fileName){return false;}

protected:

	virtual ~CameraCalibration(void);

protected:

	//common variables
	int m_cameraWidth; 
	int m_cameraHeight;

	double m_focalLength;

	//matrix, distortion (what format?)
};

typedef osg::ref_ptr<CameraCalibration> CameraCalibrationPtr;

};