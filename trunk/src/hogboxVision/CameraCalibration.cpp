#include <hogboxVision/CameraCalibration.h>

using namespace hogboxVision;

CameraCalibration::CameraCalibration(void) 
		: osg::Object(),
		_cameraWidth(0),
		_cameraHeight(0),
		_focalLength(0.0f)
{
}

CameraCalibration::~CameraCalibration(void)
{
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
CameraCalibration::CameraCalibration(const CameraCalibration& calib,const osg::CopyOp& copyop)
		: osg::Object(calib, copyop)
{

}


void CameraCalibration::ChangeSize(int width, int height)
{
	_cameraWidth = width;
	_cameraHeight = height;
}
