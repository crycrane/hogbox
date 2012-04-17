#include <hogbox/Quad.h>

using namespace hogbox;

//
//Construt geometry but requires call to SetSize
//in order to build the quad itself
//
Quad::Quad()
    : osg::Geometry(),
    _args(NULL)
{
    
}

//
//Contruct a quad with width, height and optional args
//
Quad::Quad(float width, float height, QuadArgs* args)
    : osg::Geometry(),
    _size(osg::Vec2(width, height)),
    _args(args)
{
    this->buildQuadGeometry(width, height, args);
}

//
//Contruct a quad with width, height and optional args
//
Quad::Quad(osg::Vec2 size, QuadArgs* args)
    : osg::Geometry(),
    _size(size),
    _args(args)
{
    this->buildQuadGeometry(size.x(), size.y(), args);
}

//
//Copy constructor using CopyOp to manage deep vs shallow copy.
//
Quad::Quad(const Quad& quad,const osg::CopyOp& copyop)
    : osg::Geometry(quad, copyop),
    _size(quad._size)
{
    
}

Quad::~Quad()
{
    
}

//
//Set the size of the quad, rebuilding as required
//
void Quad::SetSize(const osg::Vec2& size, QuadArgs* args)
{
    if(size == _size){return;}
    
    _size = size;
    
    if(args){
        _args = args;
    }
    
    //if still no build args, then allocate defaults
    if(!_args.valid()){
        _args = new QuadArgs();
    }
    
    this->buildQuadGeometry(_size.x(),_size.y(), _args.get());
}

//
//Return the size of the quad
//
const osg::Vec2& Quad::GetSize()
{
    return _size;
}

//
//Set the width, which inturn sets size
//
void Quad::SetWidth(const float& width)
{
    SetSize(osg::Vec2(width, _size.y()));
}

//
//Return width
//
const float Quad::GetWidth()
{
    return _size.x();
}

//
//Set the width, which inturn sets size
//
void Quad::SetHeight(const float& height)
{
    SetSize(osg::Vec2(_size.x(), height));
}

//
//Return height
//
const float Quad::GetHeight()
{
    return _size.y();
}

//
//Set the build args, optionally force 
//a rebuild of the quad
//
void Quad::SetArgs(QuadArgs* args, const bool& rebuild)
{
    //don't allow setting of null args
    if(!args){return;}
    _args = args;
    if(rebuild){
        this->buildQuadGeometry(_size.x(), _size.y(), _args.get());
    }
}

void Quad::buildQuadGeometry(const float& width, const float& height, QuadArgs* args)
{
    //remove any existing vert arrays etc
    this->setVertexArray(NULL);
    this->setTexCoordArray(0,NULL);
    this->setColorArray(NULL);
    this->setNormalArray(NULL);
    
    this->removePrimitiveSet(0);

    
    //create the default in XY plane with origin in bottom left
    osg::Vec3 corner = osg::Vec3(0.0f,0.0f,0.0f);
    osg::Vec3 widthVec = osg::Vec3(width,0.0f,0.0f);
    osg::Vec3 heightVec = osg::Vec3(0.0f,height,0.0f);
    float temp;
    
    switch(args->_originType){
        case ORI_BOTTOM_LEFT:
            break;
        case ORI_TOP_LEFT:
            heightVec *= -1.0f;
            break;
        case ORI_CENTER:
            corner = osg::Vec3(-(width*0.5f),-(height*0.5f),0.0f);
            break;
        default:break;
    }
    
    //flip plane
    switch(args->_planeType){
        case PLANE_XY:
            break;
        case PLANE_XZ:
            //flip y and z
            temp = corner.y();
            corner.y() = corner.z();
            corner.z() = temp;
            
            temp = widthVec.y();
            widthVec.y() = widthVec.z();
            widthVec.z() = temp;
            
            temp = heightVec.y();
            heightVec.y() = heightVec.z();
            heightVec.z() = temp;
            break;
        default:break;
    }
    
    float l = args->_corners[0]._texCoord.x();
    float b = args->_corners[0]._texCoord.y();
    float r = args->_corners[3]._texCoord.x();
    float t = args->_corners[3]._texCoord.y();
    
    //if(args._corners[0]._radius == 0.0f && args._corners[1]._radius == 0.0f && args._corners[2]._radius == 0.0f && args._corners[3]._radius == 0.0f){
    //    osg::createTexturedQuadGeometry(corner, widthVec, heightVec, l,b,r,t); 
    //}
    
    //
    osg::Vec3 heightDir = heightVec;
    heightDir.normalize();
    
    osg::Vec3 widthDir = widthVec;
    widthDir.normalize();
    
    osg::Vec3Array* coords = new osg::Vec3Array();
    
    //first vec at center
    osg::Vec3 center = corner+(heightVec*0.5f)+(widthVec*0.5f);
    coords->push_back(center);
    
    //move to bottom left corner
    if(args->_corners[0]._radius != 0.0f){
        
        osg::Vec3 arcPos = corner + (heightDir*args->_corners[0]._radius);
        coords->push_back(arcPos);

        osg::Vec3 arcPivot = arcPos + (widthDir*args->_corners[0]._radius);
        
        //loop and draw segments for rounded corner around arcPivot
        unsigned int segments = args->_corners[0]._segments;
        for(unsigned int i=0; i<segments; i++){
            float pc = (float)(i+1)/(float)segments;
            //get radians for segment
            float rad = osg::DegreesToRadians(180.0f+(90.0f*pc));
            float c = cosf(rad);
            float s = sinf(rad);
            
            osg::Vec3 localArcPos = osg::Vec3(c,s,0.0f) * args->_corners[0]._radius;
            arcPos = arcPivot + localArcPos;
            coords->push_back(arcPos);
        }
        
    }else{
        coords->push_back(corner);
    }
    
    //move to bottom right corner
    if(args->_corners[1]._radius != 0.0f){
        
        osg::Vec3 arcPos = (corner+widthVec) + -(widthDir*args->_corners[1]._radius);
        coords->push_back(arcPos);
        
        osg::Vec3 arcPivot = arcPos + (heightDir*args->_corners[1]._radius);
        
        //loop and draw segments for rounded corner around arcPivot
        unsigned int segments = args->_corners[1]._segments;
        for(unsigned int i=0; i<segments; i++){
            float pc = (float)(i+1)/(float)segments;
            //get radians for segment
            float rad = osg::DegreesToRadians(270.0f+(90.0f*pc));
            float c = cosf(rad);
            float s = sinf(rad);
            
            osg::Vec3 localArcPos = osg::Vec3(c,s,0.0f) * args->_corners[1]._radius;
            arcPos = arcPivot + localArcPos;
            coords->push_back(arcPos);
        }
        
    }else{
        coords->push_back(corner+widthVec);
    }
    
    //move to top right corner
    if(args->_corners[3]._radius != 0.0f){
        
        osg::Vec3 arcPos = (corner+widthVec+heightVec) + -(heightDir*args->_corners[3]._radius);
        coords->push_back(arcPos);
        
        osg::Vec3 arcPivot = arcPos + (-widthDir*args->_corners[3]._radius);
        
        //loop and draw segments for rounded corner around arcPivot
        unsigned int segments = args->_corners[3]._segments;
        for(unsigned int i=0; i<segments; i++){
            float pc = (float)(i+1)/(float)segments;
            //get radians for segment
            float rad = osg::DegreesToRadians((90.0f*pc));
            float c = cosf(rad);
            float s = sinf(rad);
            
            osg::Vec3 localArcPos = osg::Vec3(c,s,0.0f) * args->_corners[3]._radius;
            arcPos = arcPivot + localArcPos;
            coords->push_back(arcPos);
        }
        
    }else{
        coords->push_back(corner+widthVec+heightVec);
    }
    
    //move to top left
    if(args->_corners[2]._radius != 0.0f){
        
        osg::Vec3 arcPos = (corner+heightVec) + (widthDir*args->_corners[2]._radius);
        coords->push_back(arcPos);
        
        osg::Vec3 arcPivot = arcPos + -(heightDir*args->_corners[2]._radius);
        
        //loop and draw segments for rounded corner around arcPivot
        unsigned int segments = args->_corners[2]._segments;
        for(unsigned int i=0; i<segments; i++){
            float pc = (float)(i+1)/(float)segments;
            //get radians for segment
            float rad = osg::DegreesToRadians(90.0f+(90.0f*pc));
            float c = cosf(rad);
            float s = sinf(rad);
            
            osg::Vec3 localArcPos = osg::Vec3(c,s,0.0f) * args->_corners[2]._radius;
            arcPos = arcPivot + localArcPos;
            coords->push_back(arcPos);
        }
        
    }else{
        coords->push_back(corner+heightVec);
    }
    
    if(args->_corners[0]._radius != 0){
        coords->push_back(corner + (heightDir*args->_corners[0]._radius));
    }else{
        coords->push_back(corner);
    }
    
    this->setVertexArray(coords);
    
    osg::Vec2Array* tcoords = new osg::Vec2Array();
    for(unsigned int i=0; i<coords->size(); i++){
        tcoords->push_back(this->computeQuadTexCoord((*coords)[i], width, height, l,b,r,t));
    }
    
    this->setTexCoordArray(0,tcoords);
    
    osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
    //this->setColorArray(colours);
    //this->setColorBinding(Geometry::BIND_OVERALL);
    
    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0] = widthVec^heightVec;
    (*normals)[0].normalize();
    //this->setNormalArray(normals);
    //this->setNormalBinding(Geometry::BIND_OVERALL);
    
    this->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN,0,coords->size()));
    this->setUseDisplayList(false);
    this->setUseVertexBufferObjects(true);

}

osg::Vec2 Quad::computeQuadTexCoord(const osg::Vec3& coord, const float& width, const float& height,
                                                           const float& l, const float& b, const float& r, const float& t)
{
    //get unit coord
    float unitX,unitY;
    if(coord.x() == 0.0f){
        unitX=0.0f;
    }else{
        unitX = coord.x()/width;
    }
    if(coord.y() == 0.0f){
        unitY=0.0f;
    }else{
        unitY = coord.y()/height;
    }

    //get coord difference
    float xDif = r-l;
    float yDif = t-b;
    
    float u = l+(xDif*unitX);
    float v = b+(yDif*unitY);
    return osg::Vec2(u,v);
}