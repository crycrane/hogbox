#include <hogboxVision/TrackedObject.h>

using namespace hogboxVision;

TrackedObject::TrackedObject(TrackedDimensions dimensions) 
			: m_trackingDimensions( dimensions ),
			m_isEnabled(true), m_poseMatrix(), m_isDetected(false),
			m_confidence(0.0f)
{
}

TrackedObject::~TrackedObject(void)
{
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
TrackedObject::TrackedObject(const TrackedObject& object,const osg::CopyOp& copyop)
	: osg::Object(object, copyop)
{
}

bool TrackedObject::Init(const std::string& config)
{
	this->setName(config);
	return true;
}

//
//our we tracking this object
//
void TrackedObject::SetEnabled(const bool& enabled)
{
	m_isEnabled=enabled;
}

const bool& TrackedObject::GetEnabled()const
{
	return m_isEnabled;
}

//
//base update, called by a tracker
//isDetected, indicates if the object was detected by the tracker calling update
//pose, the pose matrix returned by the tracker if any
//
bool TrackedObject::UpdateMarker(bool isDetected, osg::Matrix pose)
{
	m_isDetected = isDetected;
	m_poseMatrix = pose;
	return true;
}
