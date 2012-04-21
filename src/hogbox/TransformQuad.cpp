#include <hogbox/TransformQuad.h>


using namespace hogbox;

TransformQuad::TransformQuad(TransformQuadArgs* args) 
    : osg::MatrixTransform(),
    _args(args),
    _size(osg::Vec2(1.0f, 1.0f)),
    _corner(osg::Vec2(0.0f,0.0f)),
    _rotation(0.0f),
    _depth(0.0f)
{
    this->buildBaseGraph();
}

TransformQuad::TransformQuad(const float& width, const float& height, TransformQuadArgs* args) 
    : osg::MatrixTransform(),
    _args(args),
    _size(osg::Vec2(width, height)),
    _corner(osg::Vec2(0.0f,0.0f)),
    _rotation(0.0f),
    _depth(0.0f)
{
    this->buildBaseGraph();
    this->buildQuad(width, height, args);
}

//
//Contruct base quad geometry now, passing width height and
//quad args, quad args can't be null
TransformQuad::TransformQuad(const osg::Vec2& size, TransformQuadArgs* args)
    : osg::MatrixTransform(),
    _args(args),
    _size(size),
    _corner(osg::Vec2(0.0f,0.0f)),
    _rotation(0.0f),
    _depth(0.0f)
{
    this->buildBaseGraph();
    this->buildQuad(size.x(), size.y(), args);    
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

    //allocate a quad, but don't attach, this
    //will store state set info for types not using
    //thw quad but attaching their own geom
    _quadGeode = new QuadGeode();
    
    //_quadGeode = new osg::Geode();
    
	//build the transform hierachy	
	this->addChild(_translate);
	_translate->addChild(_rotate.get());
	_rotate->addChild(_scale.get());
	//attach region node for rendering
	_scale->addChild(_quadGeode.get());	
}

//
//Build the actual quad and attach to geode
//
void TransformQuad::buildQuad(const float& width, const float& height, TransformQuadArgs* args)
{
    //if a quad already exists, then remove it first
    if(_quadGeode.get()){
        _scale->removeChild(_quadGeode.get());
        _quadGeode = NULL;
    }
    
    if(args){
        _args = args;
    }
    
    if(_args->_sizeStyle == SIZE_BY_SCALEMATRIX){
        
        //scale by
        float scale = (width+height)*0.5f;
        
        //clone the quad args and scale any conrer radius to unit size
        osg::ref_ptr<TransformQuadArgs> scaledArgs = new TransformQuadArgs(*_args.get());
        for(unsigned int i=0; i<4; i++){
            if(scaledArgs->_corners[i]._radius > 0.0f){
                scaledArgs->_corners[i]._radius = scaledArgs->_corners[i]._radius/scale;
            }
        }

        _quadGeode = new QuadGeode(1.0f, 1.0f, scaledArgs.get());
    }else{
        _quadGeode = new QuadGeode(width, height, _args.get());
    }
    _scale->addChild(_quadGeode.get());
    //_quadGeode->addDrawable(_quad.get());
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

void TransformQuad::SetSize(const osg::Vec2& size)
{
    if(size == _size){return;}
    
    //store new size
	_size = size;
    
    //if(!_quad.get()){return;}
    
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
        
        //if(!_quad.get()){
            //this->buildQuad(_size.x(), _size.y(), _args.get());
        //}
        
    }else{
        //if(!_quad.get()){
            this->buildQuad(_size.x(), _size.y(), _args.get());
       //}
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

//
//Apply the texture to the channel 0/diffuse
//
void TransformQuad::ApplyTexture(osg::Texture* tex, const unsigned int& channel)
{
	if(_quadGeode.get()){
        _quadGeode->ApplyTexture(tex, channel);
    }
}

//
//set the material color of the region
//
void TransformQuad::SetColor(const osg::Vec3& color)
{
    if(_quadGeode.get()){
        _quadGeode->SetColor(color);
    }
}

const osg::Vec3& TransformQuad::GetColor() const
{
    if(_quadGeode.get()){
        return _quadGeode->GetColor();
    }
    return osg::Vec3(0,0,0);
}


void TransformQuad::SetAlpha(const float& alpha)
{
    if(_quadGeode.get()){
        _quadGeode->SetAlpha(alpha);
    }
}

const float& TransformQuad::GetAlpha() const
{
    if(_quadGeode.get()){
        return _quadGeode->GetAlpha();
    }
    return 1.0f;
}

//
//get set enable alpha
//
void TransformQuad::EnableAlpha(const bool& enable)
{
    if(_quadGeode.get()){
        _quadGeode->EnableAlpha(enable);
    }else{
        OSG_ALWAYS << "TransformQuad EnableAlpha No Geode for region '" << this->getName() << "'" << std::endl;
    }
}

const bool& TransformQuad::IsAlphaEnabled() const
{
    if(_quadGeode.get()){
        return _quadGeode->IsAlphaEnabled();
    }
    OSG_ALWAYS << "TransformQuad IsAlphaEnabled No Geode, alpha defaults to no for region '" << this->getName() << "'" << std::endl;
    return false;
}

//
//Loads and applies a custom shader
//
void TransformQuad::SetCustomShader(const std::string& vertFile, const std::string& fragFile, const bool& shadersAreSource)
{
    if(_quadGeode.get()){
        _quadGeode->SetCustomShader(vertFile, fragFile, shadersAreSource);
    }
}

//
//disable color writes, i.e. only write to depth buffer
//
void TransformQuad::DisableColorWrites()
{
    this->SetColorWriteMask(false);
}
//
//Reenable color writes
//
void TransformQuad::EnableColorWrites()
{
    this->SetColorWriteMask(true);
}

//
void TransformQuad::SetColorWriteMask(const bool& enable)
{
    if(_quadGeode.get()){
        _quadGeode->SetColorWriteMask(enable);
    } 
}

//
//Disable depth writes
//
void TransformQuad::DisableDepthWrites()
{
    this->SetDepthWriteMask(false);
}
//
//Enable depth writes
//
void TransformQuad::EnableDepthWrites()
{
    this->SetDepthWriteMask(true);
}

//
void TransformQuad::SetDepthWriteMask(const bool& enable)
{
    if(_quadGeode.get()){
        _quadGeode->SetDepthWriteMask(enable);
    }    
}

//
//Disable depth testing
//
void TransformQuad::DisableDepthTest()
{
    this->SetDepthTestEnabled(false);
}

//
//Enable depth testing
//
void TransformQuad::EnableDepthTest()
{
    this->SetDepthTestEnabled(true);
}

//
void TransformQuad::SetDepthTestEnabled(const bool& enable)
{
    if(_quadGeode.get()){
        _quadGeode->SetDepthTestEnabled(enable);
    } 
}

//
//Enable fast drawm, just disables depth writes and testing
//
void TransformQuad::EnableFastDraw()
{
    //this->DisableLighting();
    this->DisableDepthTest();
    this->DisableDepthWrites();
}

//
//Set the renderbin number
void TransformQuad::SetRenderBinNumber(const int& num)
{
    if(_quadGeode.get()){
        _quadGeode->SetRenderBinNumber(num);
    } 
}
