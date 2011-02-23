#pragma once

#include <osg/Object>
#include <osg/FrameStamp>
#include <hogbox/HogBoxBase.h>

namespace hogboxStage {
	
//
//The base container type for entity event params
//
class ComponentEvent : public osg::Object
{
public:	
	ComponentEvent()
		: osg::Object()
	{
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	ComponentEvent(const ComponentEvent& ent,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(ent, copyop)
	{
	}

	META_Box(hogboxStage, ComponentEvent);
	
protected:	
	
	virtual ~ComponentEvent(void){}

protected:

};
typedef osg::ref_ptr<ComponentEvent> ComponentEventPtr;

//
//The base update event type
//
class ComponentUpdate : public ComponentEvent
{
public:
	ComponentUpdate()
		: ComponentEvent(),
		_frameStamp()
	{
	}

	ComponentUpdate(osg::FrameStamp frameStamp)
		: ComponentEvent(),
		_frameStamp(frameStamp)
	{
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	ComponentUpdate(const ComponentUpdate& ent,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: ComponentEvent(ent, copyop),
		_frameStamp(ent._frameStamp)
	{
	}

	META_Box(hogboxStage, ComponentUpdate);

protected:
	virtual ~ComponentUpdate(){
	}

protected:

	//the current frame stamp from the viewer
	osg::FrameStamp _frameStamp;
};

};//end hogboxStage namespace
 