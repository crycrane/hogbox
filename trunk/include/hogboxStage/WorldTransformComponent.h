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

	META_Object(hogboxStage, MovedEvent);

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
	WorldTransformComponent()
		: Component()
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
	//pure virtual get type name to be implemented by concrete types
	virtual const std::string GetTypeName(){return "WorldTransformComponent";}

	//
	//Our main update function 
	virtual bool OnUpdate(ComponentEventPtr eventData){return true;}


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
	const osg::Vec3& GetPosition()const{ 
		return _translate;
	}
	//
	//Set just position
	void SetPosition(const osg::Vec3& pos){
		_translate = pos;
		if(pos != _transform.getTrans()){
			_transform.setTrans(pos);
			this->TriggerEventCallback("OnMoved", new MovedEvent(_transform));
		}
	}

	//
	//Get rotation as quatinion
	const osg::Quat GetRotationQuat(){return _transform.getRotate();}
	//get rotate as vec3 of angles in degrees, currently won't catch any sets from
	//quats so only reflects the last set from degrees
	const osg::Vec3& GetRotationDegrees()const{
		return _rotate;
	}

	//
	//Set just rotation xyz in degrees
	void SetRotationDegrees(const float& x, const float& y, const float& z){
		SetRotationDegrees(osg::Vec3(x,y,z));
	}
	void SetRotationDegrees(const osg::Vec3& rotDegrees){
		_rotate = rotDegrees;
		osg::Matrix rot = osg::Matrix::rotate(osg::DegreesToRadians(rotDegrees.x()), osg::Vec3(1,0,0)) *
						osg::Matrix::rotate(osg::DegreesToRadians(rotDegrees.y()), osg::Vec3(0,1,0)) *
						osg::Matrix::rotate(osg::DegreesToRadians(rotDegrees.z()), osg::Vec3(0,0,1));
		SetRotation(rot.getRotate());
	}
	//
	//Set just rotation from quat
	void SetRotation(const osg::Quat& rot){
		if(rot != _transform.getRotate()){
			_transform.setRotate(rot);
			this->TriggerEventCallback("OnMoved", new MovedEvent(_transform));
		}
	}


protected:

	virtual ~WorldTransformComponent(void) {

	}

protected:

	osg::Matrix _transform;

	//we use these only so we can use them as xml attributes
	osg::Vec3 _translate;
	osg::Vec3 _rotate;
	osg::Vec3 _scale;

};
typedef osg::ref_ptr<WorldTransformComponent> WorldTransformComponentPtr;

};