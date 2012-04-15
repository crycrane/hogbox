#include <hogbox/TransformQuad.h>


using namespace hogbox;

TransformQuad::TransformQuad() 
    : osg::MatrixTransform(),
    _args(new TransformQuadArgs()),
    _size(osg::Vec2(1.0f, 1.0f)),
    _corner(osg::Vec2(0.0f,0.0f)),
    _rotation(0.0f),
    _depth(0.0f),
    //animation
    _animateRotate(new hogbox::AnimateFloat()),
    _isRotating(false),
    _animatePosition(new hogbox::AnimateVec2()),
    _isTranslating(false),
    _animateSize(new hogbox::AnimateVec2()),
    _isSizing(false),
    _prevTick(0.0f),
    _animationDisabled(false)
{
    this->buildBaseGraph();
}

TransformQuad::TransformQuad(const float& width, const float& height, TransformQuadArgs* args) 
    : osg::MatrixTransform(),
    _args(args),
    _size(osg::Vec2(width, height)),
    _corner(osg::Vec2(0.0f,0.0f)),
    _rotation(0.0f),
    _depth(0.0f),
    //animation
    _animateRotate(new hogbox::AnimateFloat()),
    _isRotating(false),
    _animatePosition(new hogbox::AnimateVec2()),
    _isTranslating(false),
    _animateSize(new hogbox::AnimateVec2()),
    _isSizing(false),
    _prevTick(0.0f),
    _animationDisabled(false)
{
    this->buildBaseGraph();
    this->buildQuad(width, height, args);
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
TransformQuad::TransformQuad(const TransformQuad& quad,const osg::CopyOp& copyop)
    : osg::MatrixTransform(quad, copyop),
    _size(quad._size),
    _corner(quad._corner),
    _rotation(quad._rotation),
    _depth(quad._depth)
{
    this->buildBaseGraph();
    //this->buildQuad(_size.x(), _size., args);
}

TransformQuad::~TransformQuad(void)
{
	_translate = NULL;
	_rotate = NULL;
	_scale = NULL;
}

//
//Build our base graph of translate, rotate
//scale and geode
//
void TransformQuad::buildBaseGraph()
{
	_translate = new osg::MatrixTransform();
	_rotate = new osg::MatrixTransform();
	_scale = new osg::MatrixTransform();

    _quadGeode = new osg::Geode();
	
	//create and attach our default updatecallback
	_updateCallback = new AnimateUpdateCallback(this);
	this->setUpdateCallback(_updateCallback.get());
    
	//build the transform hierachy	
	this->addChild(_translate);
	_translate->addChild(_rotate.get());
	_rotate->addChild(_scale.get());
	//attach region node for rendering
	_scale->addChild(_quadGeode.get());	
    
	//attach the child regions to rotate, so children
	//are affected by this regions translate and rotate
	//transforms directly, but scale is applied indepenatly
	//to allow for different resize modes etc
	//_rotate->addChild(_childMount.get());
	
	//set default animation values
	_animateRotate->SetValue(this->GetRotation());
	_animateSize->SetValue(this->GetSize());
	_animatePosition->SetValue(this->GetPosition());
}

//
//Build the actual quad and attach to geode
//
void TransformQuad::buildQuad(const float& width, const float& height, Quad::QuadArgs* args)
{
    //if a quad already exists, then remove it first
    if(_quad.get()){
        _quadGeode->removeDrawable(_quad.get());
        _quad = NULL;
    }
    
    if(_args->_sizeStyle == SIZE_BY_SCALEMATRIX){
        _quad = new Quad(1.0f, 1.0f, args);
    }else{
        _quad = new Quad(width, height, args);
    }
    _quadGeode->addDrawable(_quad.get());
}

//
//general update, updates and syncs our animations etc
//normally called by an attached updateCallback
//
void TransformQuad::Update(float simTime)
{
    //use global hud timepassed
	float timePassed = 0.033f;//Hud::Inst()->GetTimePassed();
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
	}
}

//positioning

void TransformQuad::SetPosition(const osg::Vec2& corner)
{
	_corner = corner;
    
    //flip plane
    float temp = 0.0f;
    osg::Vec3 offset = osg::Vec3(corner.x(),corner.y(),_depth);
    switch(_args->_planeType){
        case Quad::PLANE_XY:
            break;
        case Quad::PLANE_XZ:
            //flip x and z
            temp = offset.y();
            offset.y() = offset.z();
            offset.z() = temp;
            break;
        default:break;
    }
	_translate->setMatrix( osg::Matrix::translate(offset)); 
}

const osg::Vec2& TransformQuad::GetPosition() const
{
	return _corner;
}

//
//get set roation around the z axis in degrees
//
void TransformQuad::SetRotation(const float& rotate)
{
	_rotation = rotate;
    
    //flip plane
    float temp = 0.0f;
    osg::Vec3 axis = osg::Vec3(0.0f,0.0f,1.0f);
    switch(_args->_rotatePlane){
        case Quad::PLANE_XY:
            break;
        case Quad::PLANE_XZ:
            //flip x and z (as its rotation)
            temp = axis.y();
            axis.y() = axis.z();
            axis.z() = temp;
            break;
        case Quad::PLANE_YZ:
            //flip x and z (as its rotation)
            temp = axis.x();
            axis.x() = axis.z();
            axis.z() = temp;
            break;
        default:break;
    }
    
	_rotate->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(_rotation), axis));
}

const float& TransformQuad::GetRotation() const
{
	return _rotation;
}

void TransformQuad::SetSize(const osg::Vec2& size, TransformQuadArgs* args)
{
    if(size == _size){return;}
    
    //store new size
	_size = size;
    
    if(args){
        _args = args;
    }
    
    if(_args->_sizeStyle == SIZE_BY_SCALEMATRIX)
    {
        //resize this regions geometry
        float temp = 0.0f;
        osg::Vec3 sizeAxis = osg::Vec3(size.x(), size.y(), 1.0f);
        switch(_args->_planeType){
            case Quad::PLANE_XY:
                break;
            case Quad::PLANE_XZ:
                //flip x and z
                temp = sizeAxis.y();
                sizeAxis.y() = sizeAxis.z();
                sizeAxis.z() = temp;
                break;
            default:break;
        }
        _scale->setMatrix( osg::Matrix::scale(sizeAxis));
        
        if(args){
            this->buildQuad(_size.x(), _size.y(), _args.get());
        }
        
    }else{
        if(!_quad.get()){
            this->buildQuad(_size.x(), _size.y(), _args.get());
        }
    }
}

const osg::Vec2& TransformQuad::GetSize() const
{
	return _size;
}

//
//set the size as a percentage/scale factor of the current size 
//i.e. <1 smaller, >1 bigger
//
void TransformQuad::SetSizeFromPercentage(float xScaler, float yScaler)
{
	osg::Vec2 newSize;
	newSize.x() = _size.x()*xScaler;
	newSize.y() = _size.y()*yScaler;
	this->SetSize(newSize);
}

//
//set the position as a percentage/scale factor of the current position
//i.e. <1 smaller, >1 bigger
//
void TransformQuad::SetPositionFromPercentage(float xScaler, float yScaler)
{
	osg::Vec2 newPos;
	newPos.x() = _corner.x()*xScaler;
	newPos.y() = _corner.y()*yScaler;
	this->SetPosition(newPos);
}

//
//move to a new layer
//

void TransformQuad::SetLayer(const float& depth)
{
    _depth = depth;
    
    //flip plane
    float temp = 0.0f;
    osg::Vec3 offset = osg::Vec3(_corner.x(),_corner.y(),depth);
    switch(_args->_planeType){
        case Quad::PLANE_XY:
            break;
        case Quad::PLANE_XZ:
            //flip x and z
            temp = offset.y();
            offset.y() = offset.z();
            offset.z() = temp;
            break;
        default:break;
    }
	_translate->setMatrix( osg::Matrix::translate(offset)); 
}

const float& TransformQuad::GetLayer() const
{
	return _depth;
}
