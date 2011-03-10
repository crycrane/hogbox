#pragma once

#include <hogboxStage/Component.h>
#include <hogboxStage/WorldTransformComponent.h>
#include <hogboxStage/CollidableComponent.h>

namespace hogboxStage 
{

//
//PhysicsComponent
//Does basic particle physics and then sets the WorldTransformComponents
//Position to reflect the particles
//
class HOGBOXSTAGE_EXPORT PhysicsComponent : public Component
{
public:
	PhysicsComponent()
		: Component()
	{
		//add callback to indicate a change to the transform
		//AddCallbackEventType("OnMoved");

		//this depends on WorldTransformComponent so it can set position
		AddComponentDependency("WorldTransformComponent");
		//also depends on/can listen out for collide events from a collideablecomponent
		AddComponentDependency("CollidableComponent");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	PhysicsComponent(const PhysicsComponent& ent,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: Component(ent, copyop),
		_position(ent._position),
		_mass(ent._mass),
		_velocity(ent._velocity)
	{
		//add callback to indicate a change to the transform
		//AddCallbackEventType("OnMoved");
		//this depends on WorldTransformComponent so it can set position
		AddComponentDependency("WorldTransformComponent");
	}

	META_Box(hogboxStage, PhysicsComponent);

	//
	//pure virtual get type name to be implemented by concrete types
	virtual const std::string GetTypeName(){return "PhysicsComponent";}

	//
	//Our main update function 
	virtual bool OnUpdate(ComponentEventPtr eventData){
		 //integrate physics
		//linear
		float timeStep = 0.033f;
		osg::Vec3 acceleration = _forces / _mass;
		_velocity += acceleration * timeStep;
		_position += _velocity * timeStep;
		_forces = osg::Vec3(0.0f,0.0f,0.0f); //clear forces

		return true;
	}


	//
	//Get position
	const osg::Vec3& GetPosition()const{ 
		return _position;
	}
	//
	//Set position
	void SetPosition(const osg::Vec3& pos){
		_position = pos;
	}

	//
	//Get mass
	const float& GetMass()const{
		return _mass;
	}
	//
	//Set mass
	void SetMass(const float& mass){
		_mass = mass;
	}

	//
	//Get current Velocity
	const osg::Vec3& GetVelocity(){
		return _velocity;
	}

	//
	//
	virtual bool HandleComponentDependency(Component* component){
		//physics handle two types, worldtrans and collidable
		WorldTransformComponent* transComp = dynamic_cast<WorldTransformComponent*>(component);
		if(transComp){
			p_worldTrans = transComp;
			return true;
		}
		//cast to collidable
		CollidableComponent* collideComp = dynamic_cast<CollidableComponent*>(component);
		if(collideComp){
			//register for the OnCollide Event
			//collideComp->RegisterCallbackForEvent(new ComponentEventObjectCallback<PhysicsComponent>(this,this,
			//														&PhysicsComponent::OnEntityCollidedCallback),
			//														"OnCollide");
			return true;
		}
		return false;
	}

protected:

	virtual ~PhysicsComponent(void) {

	}

protected:

	//we use these only so we can use them as xml attributes
	osg::Vec3 _position;
	float     _mass;

	osg::Vec3 _forces;

	osg::Vec3 _velocity;

	//store a pointer to the worldtranscomp that this will move
	WorldTransformComponentPtr p_worldTrans;

};
typedef osg::ref_ptr<PhysicsComponent> PhysicsComponentPtr;

};