#pragma once
#include "trackedobject.h"
#include <vector>
#include <osg/Vec2>

namespace hogboxVision {

//
//Planar tracked objects store a list 2d coords
//representing the corners of the planar object
//
class HOGBOXVIS_EXPORT PlanarTrackedObject : public TrackedObject
{
public:
	PlanarTrackedObject(TrackedDimensions dimensions=TRACKING_BOTH);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	PlanarTrackedObject(const PlanarTrackedObject&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Box(hogboxVision, PlanarTrackedObject);

	std::vector<osg::Vec2> GetCorners(){return m_corners;}

protected:

	virtual ~PlanarTrackedObject();

protected:

	std::vector<osg::Vec2> m_corners;
	int direction; //which side is local up

};

};
