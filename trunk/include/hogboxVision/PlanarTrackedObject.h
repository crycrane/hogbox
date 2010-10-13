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

	//set by implementations
	std::vector<osg::Vec2> GetCorners(){return m_corners;}
	int GetDirection(){return m_direction;}

	//get set the width
	float GetWidth(){return m_width;}
	void SetWidth(float width){m_width = width;}
	
protected:

	virtual ~PlanarTrackedObject();

protected:

	std::vector<osg::Vec2> m_corners;
	int m_direction; //which side is local up/y
	
	//width of planar quad in mm
	float m_width;

};

};
