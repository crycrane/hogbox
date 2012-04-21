#include <hogbox/AnimatedTransformQuad.h>


using namespace hogbox;

AnimatedTransformQuad::AnimatedTransformQuad(TransformQuadArgs* args) 
    : TransformQuad(args),
    //animation
    _animateRotate(new hogbox::AnimateFloat()),
    _isRotating(false),
    _animatePosition(new hogbox::AnimateVec2()),
    _isTranslating(false),
    _animateSize(new hogbox::AnimateVec2()),
    _isSizing(false),
    _prevTick(0.0f),
    _animationDisabled(false),
    _animateColor(new hogbox::AnimateVec3()),
    _isColoring(false),
    _animateAlpha(new hogbox::AnimateFloat()),
    _isFading(false)
{
	this->setDefaultAnimationValues();
}

AnimatedTransformQuad::AnimatedTransformQuad(const float& width, const float& height, TransformQuadArgs* args) 
    : TransformQuad(width, height, args),
    //animation
    _animateRotate(new hogbox::AnimateFloat()),
    _isRotating(false),
    _animatePosition(new hogbox::AnimateVec2()),
    _isTranslating(false),
    _animateSize(new hogbox::AnimateVec2()),
    _isSizing(false),
    _prevTick(0.0f),
    _animationDisabled(false),
    _animateColor(new hogbox::AnimateVec3()),
    _isColoring(false),
    _animateAlpha(new hogbox::AnimateFloat()),
    _isFading(false)
{
	this->setDefaultAnimationValues();
}

//
//Contruct base quad geometry now, passing width height and
//quad args, quad args can't be null
AnimatedTransformQuad::AnimatedTransformQuad(const osg::Vec2& size, TransformQuadArgs* args)
    : TransformQuad(size, args),
    //animation
    _animateRotate(new hogbox::AnimateFloat()),
    _isRotating(false),
    _animatePosition(new hogbox::AnimateVec2()),
    _isTranslating(false),
    _animateSize(new hogbox::AnimateVec2()),
    _isSizing(false),
    _prevTick(0.0f),
    _animationDisabled(false),
    _animateColor(new hogbox::AnimateVec3()),
    _isColoring(false),
    _animateAlpha(new hogbox::AnimateFloat()),
    _isFading(false)
{
	this->setDefaultAnimationValues();
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
AnimatedTransformQuad::AnimatedTransformQuad(const AnimatedTransformQuad& quad,const osg::CopyOp& copyop)
    : TransformQuad(quad, copyop)
{

}

AnimatedTransformQuad::~AnimatedTransformQuad(void)
{

}

//
//set default animation values and update callback
//
void AnimatedTransformQuad::setDefaultAnimationValues()
{
    //create and attach our default updatecallback
	_updateCallback = new AnimateUpdateCallback(this);
	this->setUpdateCallback(_updateCallback.get());
    
    //set default animation values
    _animateRotate->SetValue(this->GetRotation());
	_animateSize->SetValue(this->GetSize());
	_animatePosition->SetValue(this->GetPosition());
	_animateColor->SetValue(this->GetColor());
	_animateAlpha->SetValue(this->GetAlpha());
}

//
//general update, updates and syncs our animations etc
//normally called by an attached updateCallback
//
void AnimatedTransformQuad::UpdateAnimation(float simTime)
{
    //use global hud timepassed
	float timePassed = simTime;//Hud::Inst()->GetTimePassed();
	_prevTick = simTime;
    
	if(!_animationDisabled)
	{
		//update all our smooth values
		if((_isRotating = _animateRotate->Update(timePassed)))
		{this->SetRotation(_animateRotate->GetValue());}
		
		if((_isTranslating = _animatePosition->Update(timePassed)))
		{this->SetPosition(_animatePosition->GetValue());}
		
		if((_isSizing = _animateSize->Update(timePassed)))
		{this->SetSize(_animateSize->GetValue());}
        
        if((_isColoring = _animateColor->Update(timePassed)))
		{this->SetColor(_animateColor->GetValue());}
		
		if((_isFading = _animateAlpha->Update(timePassed)))
		{this->SetAlpha(_animateAlpha->GetValue());}
	}
}

