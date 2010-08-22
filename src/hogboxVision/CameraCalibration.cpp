#include <hogboxVision/CameraCalibration.h>

using namespace hogboxVision;

CameraCalibration::CameraCalibration(void) 
		: osg::Object(),
		m_cameraWidth(0),
		m_cameraHeight(0),
		m_focalLength(0.0f)
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
	m_cameraWidth = width;
	m_cameraHeight = height;
}
