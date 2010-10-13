#include <hogboxVision/CameraBasedTracker.h>

using namespace hogboxVision;

CameraBasedTracker::CameraBasedTracker(void) 
	: HogTracker(),
	m_modifiedCount(-1)
{
}

CameraBasedTracker::~CameraBasedTracker(void)
{
	m_cc = NULL;
	p_image = NULL;
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
CameraBasedTracker::CameraBasedTracker(const CameraBasedTracker& tracker,const osg::CopyOp& copyop)
	: HogTracker(tracker, copyop)
{
}

//
//init the tracker passing the dimensions of the
//image used as input
bool CameraBasedTracker::InitTracker(int width, int height, const std::string& cameraCalibrationFile)
{
	m_videoWidth = width;
	m_videoHeight = height;

	if(!LoadCalibrationFromFile(cameraCalibrationFile))
	{return false;}

	return true;
}

//
//update the tracker then call base update which calls update tracker
//
void CameraBasedTracker::Update(hogbox::ImagePtr image)
{
	//clear our old image pointer
	p_image = NULL;
	//set new
	p_image = image;

	if(m_modifiedCount != p_image->getModifiedCount())
	{
		//call base to trigger detection and tracking of new image
		HogTracker::Update();
		m_modifiedCount = p_image->getModifiedCount();
	}
}
