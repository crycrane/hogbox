#include <hogboxVision/PlanarTrackedObject.h>

using namespace hogboxVision;

PlanarTrackedObject::PlanarTrackedObject(TrackedDimensions dimensions) 
	: TrackedObject(dimensions),
	_direction(-1),
	_width(80.0f)
{
	//assign the 4 corners
	_corners.assign(4, osg::Vec2());
}

PlanarTrackedObject::~PlanarTrackedObject()
{
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
PlanarTrackedObject::PlanarTrackedObject(const PlanarTrackedObject& object,const osg::CopyOp& copyop)
	: TrackedObject(object, copyop)
{
}
