#include <hogboxVision/TrackedObject.h>

using namespace hogboxVision;

TrackedObject::TrackedObject(TrackedDimensions dimensions) 
    : _trackingDimensions( dimensions ),
    _poseMatrix(),
    _isDetected(false),
    _isEnabled(true),
    _confidence(0.0f)
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
	_isEnabled=enabled;
}

const bool& TrackedObject::GetEnabled()const
{
	return _isEnabled;
}

//
//base update, called by a tracker
//isDetected, indicates if the object was detected by the tracker calling update
//pose, the pose matrix returned by the tracker if any
//
bool TrackedObject::UpdateMarker(bool isDetected, osg::Matrix pose)
{
	_isDetected = isDetected;
	_poseMatrix = pose;
	return true;
}
