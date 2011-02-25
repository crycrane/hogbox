#pragma once

#include <hogboxStage/Component.h>

namespace hogboxStage 
{

//
//MovedEvent
//The update type passed to OnMoved receivers
//
class MovedEvent : public ComponentEvent
{
public:
	MovedEvent(osg::Matrix trans=osg::Matrix())
		: ComponentEvent(),
		_transform(trans)
	{
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	MovedEvent(const MovedEvent& ent,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: ComponentEvent(ent, copyop),
		_transform(ent._transform)
	{
	}

	META_Box(hogboxStage, MovedEvent);

protected:
	virtual ~MovedEvent(){
	}

public:

	osg::Matrix _transform;
};

//
//WorldTransformComponent
//Stores a transform matrix for positioning and entity
//in world coords, it is then used by RenderableComponents to
//position the model
//
class HOGBOXSTAGE_EXPORT WorldTransformComponent : public Component
{
public:
	WorldTransformComponent(Entity* parent=NULL)
		: Component(parent)
	{
		//add callback to indicate a change to the transform
		AddCallbackEventType("OnMoved");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	WorldTransformComponent(const WorldTransformComponent& ent,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: Component(ent, copyop),
		_transform(ent._transform)
	{
		//add callback to indicate a change to the transform
		AddCallbackEventType("OnMoved");
	}

	META_Box(hogboxStage, WorldTransformComponent);

	//
	//Our main update function 
	virtual bool OnUpdate(ComponentEventPtr eventData){return true;}

	//
	//pure virtual get type name to be implemented by concrete types
	virtual const std::string GetTypeName(){return "WorldTransformComponent";}

	//
	//Get the transform
	const osg::Matrix& GetTransform()const{return _transform;}
	//
	//Set the transform
	void SetTransform(const osg::Matrix& trans){
		if(trans != _transform){
			_transform = trans;
			this->TriggerEventCallback("OnMoved", new MovedEvent(_transform));
		}
	}

	//
	//Get position
	const osg::Vec3 GetPosition()const{return _transform.getTrans();}
	//
	//Set just position
	void SetPosition(const osg::Vec3& pos){
		if(pos != _transform.getTrans()){
			_transform.setTrans(pos);
			this->TriggerEventCallback("OnMoved", new MovedEvent(_transform));
		}
	}

	//
	//Set just rotation xyz in degrees
	void SetRotation(float x, float y, float z){
		osg::Matrix rot = osg::Matrix::rotate(osg::DegreesToRadians(x), osg::Vec3(1,0,0)) *
						osg::Matrix::rotate(osg::DegreesToRadians(y), osg::Vec3(0,1,0)) *
						osg::Matrix::rotate(osg::DegreesToRadians(z), osg::Vec3(0,0,1));
		SetRotation(rot.getRotate());
	}

	//
	//Get rotation as quatinion
	const osg::Quat GetRotationQuat(){return _transform.getRotate();}
	//
	//Set just rotation from quat
	void SetRotation(osg::Quat rot){
		if(rot != _transform.getRotate())
		{
			_transform.setRotate(rot);
			this->TriggerEventCallback("OnMoved", new MovedEvent(_transform));
		}
	}


protected:

	virtual ~WorldTransformComponent(void) {

	}

protected:

	osg::Matrix _transform;

};
typedef osg::ref_ptr<WorldTransformComponent> WorldTransformComponentPtr;

};