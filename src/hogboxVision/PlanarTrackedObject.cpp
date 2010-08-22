#include <hogboxVision/PlanarTrackedObject.h>

using namespace hogboxVision;

PlanarTrackedObject::PlanarTrackedObject(TrackedDimensions dimensions) 
		: TrackedObject(dimensions)
{
	//assign the 4 corners
	m_corners.assign(4, osg::Vec2());
}

PlanarTrackedObject::~PlanarTrackedObject()
{
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
PlanarTrackedObject::PlanarTrackedObject(const PlanarTrackedObject& object,const osg::CopyOp& copyop)
	: TrackedObject(object, copyop)
{
}
