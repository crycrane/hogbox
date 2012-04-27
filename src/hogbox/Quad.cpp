#include <hogbox/Quad.h>

#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osgDB/FileUtils>

using namespace hogbox;

#ifndef WIN32
#define SHADER_COMPAT \
"#ifndef GL_ES\n" \
"#if (__VERSION__ <= 110)\n" \
"#define lowp\n" \
"#define mediump\n" \
"#define highp\n" \
"#endif\n" \
"#endif\n"
#else
#define SHADER_COMPAT ""
#endif

static const char* texturedVertSource = { 
	SHADER_COMPAT 
	"attribute vec4 osg_Vertex;\n"
	"attribute vec4 osg_MultiTexCoord0;\n"
	"uniform mat4 osg_ModelViewProjectionMatrix;\n"
	"varying mediump vec2 texCoord0;\n"
	"void main(void) {\n" 
	"  gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n" 
	"  texCoord0 = osg_MultiTexCoord0.xy;\n"
	"}\n" 
}; 

static const char* texturedFragSource = { 
	SHADER_COMPAT 
	"uniform sampler2D diffuseTexture;\n"
    "uniform mediump vec4 _color;\n"
	"varying mediump vec2 texCoord0;\n"
	"void main(void) {\n" 
    "  mediump vec4 texColor = texture2D(diffuseTexture, texCoord0);\n" 
    "  gl_FragColor = texColor;//vec4(texColor.r*_color.r,texColor.g*_color.g,texColor.b*_color.b, texColor.a*_color.a);\n"
    "  //gl_FragColor.a = gl_FragColor.a*_color.a;\n"
	"}\n" 
};

static const char* coloredVertSource = { 
	SHADER_COMPAT 
	"attribute vec4 osg_Vertex;\n"
	"uniform mat4 osg_ModelViewProjectionMatrix;\n"
	"void main(void) {\n" 
	"  gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n" 
	"}\n" 
}; 

static const char* coloredFragSource = { 
	SHADER_COMPAT 
    "uniform mediump vec4 _color;\n"
	"void main(void) {\n" 
    "  gl_FragColor = _color;\n"
	"}\n" 
};

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
Quad::Quad(const osg::Vec2& size, QuadArgs* args)
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
//is the quad equal, based on size and args
//
const bool Quad::isEqual(osg::Vec2 size, QuadArgs* args)
{
    if(_size == size && 
       _args->isEqual(args)){
        return true;
    }
    return false;
}

//
static std::vector<QuadPtr> g_quadCache;

//
//get or create a new quad with matching args and size
//
Quad* Quad::getOrCreateQuad(const osg::Vec2& size, QuadArgs* args)
{
    for(unsigned int i=0; i<g_quadCache.size(); i++){
        if(g_quadCache[i]->isEqual(size, args)){
            return g_quadCache[i].get();
        }
    }
    
    Quad* quad = new Quad(size, args);
    g_quadCache.push_back(quad);
    return quad;
}

void Quad::clearCache()
{
    g_quadCache.clear();
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

//
//just allocate the transform graph and geode
//
QuadGeode::QuadGeode(QuadGeodeArgs* args)
    : osg::Geode(),
    _quad(new Quad())
{
    this->InitStateSet();
    this->addDrawable(_quad.get());
}

//
//Contruct base quad geometry now, passing width height and
//quad args, quad args can't be null
//
QuadGeode::QuadGeode(const float& width, const float& height, QuadGeodeArgs* args)
    : osg::Geode(),
    _quad(NULL)
{
    _quad = Quad::getOrCreateQuad(osg::Vec2(width, height), args);
    this->InitStateSet();
    this->addDrawable(_quad.get());
}

//
//Contruct base quad geometry now, passing width height and
//quad args, quad args can't be null
//
QuadGeode::QuadGeode(const osg::Vec2& size, QuadGeodeArgs* args)
    : osg::Geode(),
    _quad(NULL)
{
    _quad = Quad::getOrCreateQuad(size, args);
    this->InitStateSet();
    this->addDrawable(_quad.get());
}

QuadGeode::~QuadGeode()
{
    
}


//
//Init Material/stateset stuff
//
void QuadGeode::InitStateSet()
{
    _stateset = new osg::StateSet();
    _stateset->setDataVariance(osg::Object::DYNAMIC);
    //_material = new osg::Material();
    //_material->setColorMode(osg::Material::OFF);
    //_material->setDataVariance(osg::Object::DYNAMIC);

    //add the color uniform
    _colorUniform = new osg::Uniform("_color", osg::Vec4(0.0f,1.0f,1.0f,1.0f));
    _stateset->addUniform(_colorUniform.get());

    #ifdef TARGET_OS_IPHONE
    _stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
    #endif

    #ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE

    //_stateset->setAttributeAndModes(_material, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    #else
    osg::Program* program = new osg::Program; 
    program->setName("coloredQuadShader"); 
    program->addShader(new osg::Shader(osg::Shader::VERTEX, coloredVertSource)); 
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, coloredFragSource)); 
    _stateset->setAttributeAndModes(program, osg::StateAttribute::ON); 	

    _shaderMode = COLOR_SHADER;

    #endif
    //_root->setStateSet(_stateset.get()); 
    //_quadGeode->setStateSet(_stateset.get()); 

    this->SetColor(osg::Vec3(1.0f, 1.0f, 1.0f));
    this->SetAlpha(1.0f);
    this->EnableAlpha(false);
}

//
//Apply the texture to the channel 0/diffuse
//
void QuadGeode::ApplyTexture(osg::Texture* tex, const unsigned int& channel)
{
	if(!_stateset.get()){
        OSG_ALWAYS << "Region::ApplyTexture Failed, NULL texture." << std::endl;
        return;
    }
	
	_stateset->setTextureAttributeAndModes(channel,tex,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    _stateset->addUniform(new osg::Uniform("diffuseTexture", channel));
    
    //
    if(_shaderMode == COLOR_SHADER && tex != NULL){
        osg::Program* program = new osg::Program; 
        program->setName("texturedQuadShader"); 
        program->addShader(new osg::Shader(osg::Shader::VERTEX, texturedVertSource)); 
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, texturedFragSource)); 
        _stateset->setAttributeAndModes(program, osg::StateAttribute::ON); 	
        _shaderMode = TEXTURED_SHADER;
        
    }else if(_shaderMode == TEXTURED_SHADER && tex == NULL){
        osg::Program* program = new osg::Program; 
        program->setName("coloredQuadShader"); 
        program->addShader(new osg::Shader(osg::Shader::VERTEX, coloredVertSource)); 
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, coloredFragSource)); 
        _stateset->setAttributeAndModes(program, osg::StateAttribute::ON); 	
        _shaderMode = COLOR_SHADER;
    }
	
	//if the textures image includes alpha enable alpha/blending
	if(tex)
	{
		if(tex->getImage(0))
		{
			bool isImageTranslucent = tex->getImage(0)->getPixelFormat()==GL_RGBA || tex->getImage(0)->getPixelFormat()==GL_BGRA;
			this->EnableAlpha(isImageTranslucent);
		}
	}
	
	//NOTE@tom, below isn't needed on platforms supporting glu
	//apply a non power of two rezie callback if required
    
    /*if(!hogbox::SystemInfo::Instance()->npotTextureSupported())
     {
     osg::ref_ptr<hogbox::NPOTResizeCallback> resizer = new hogbox::NPOTResizeCallback(tex, 0, _stateset.get());
     //if the texture casts as a rect apply the tex rect scaling to texture coords
     osg::Texture2D* tex2D = dynamic_cast<osg::Texture2D*> (tex); 
     if(tex2D){
     if(resizer->useAsCallBack()){tex2D->setSubloadCallback(resizer.get());}
     }
     }else*/{
         
         //clamp to edge required for IPhone NPOT support
         tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
         tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
         
         
         //if 2d set filtering to nearest
         osg::Texture2D* tex2D = dynamic_cast<osg::Texture2D*> (tex); 
         if(tex2D){
             //set to linear to disable mipmap generation
             tex2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
             tex2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
             tex2D->setUseHardwareMipMapGeneration(false);
             tex2D->setResizeNonPowerOfTwoHint(false);
             tex2D->setUnRefImageDataAfterApply(true);
         }
         
     }
}

//
//set the material color of the region
//
void QuadGeode::SetColor(const osg::Vec3& color)
{
	_color = color;
    
	osg::Vec4 vec4Color = osg::Vec4(color, _alpha);
    
	//set the materials color
	//_material->setAmbient(osg::Material::FRONT_AND_BACK, vec4Color ); 
	//_material->setDiffuse(osg::Material::FRONT_AND_BACK, vec4Color );
	//_material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1.0) );
	//_material->setShininess(osg::Material::FRONT_AND_BACK, 1.0);
	//_material->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(color,_alpha));
    
    //update color uniform
    _colorUniform->set(vec4Color);
}

const osg::Vec3& QuadGeode::GetColor() const
{
	return _color;
}


void QuadGeode::SetAlpha(const float& alpha)
{
	_alpha = alpha;
    
    //call set color to update the coloruniform with the new alpha
    this->SetColor(_color);
    
	//set the materials alpha
	//_material->setAlpha(osg::Material::FRONT_AND_BACK, _alpha);
}

const float& QuadGeode::GetAlpha() const
{
	return _alpha;
}

//
//get set enable alpha
//
void QuadGeode::EnableAlpha(const bool& enable)
{
	if(enable)
	{
		osg::BlendEquation* blendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
		_stateset->setAttributeAndModes(blendEquation, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		osg::BlendFunc* blendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
		_stateset->setAttributeAndModes(blendFunc, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		//tell to sort the mesh before displaying it
		_stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		_stateset->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 
		//_stateset->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        
	}else{
		_stateset->setRenderingHint(osg::StateSet::OPAQUE_BIN);
		_stateset->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	}
	_alphaEnabled = enable;
}

const bool& QuadGeode::IsAlphaEnabled() const
{
	return _alphaEnabled;
}

//
//Loads and applies a custom shader
//
void QuadGeode::SetCustomShader(const std::string& vertFile, const std::string& fragFile, const bool& shadersAreSource)
{
    osg::Shader* fragShader = NULL;
    if(shadersAreSource){
        fragShader = new osg::Shader(osg::Shader::FRAGMENT, fragFile.c_str());
    }else{
        fragShader = new osg::Shader(osg::Shader::FRAGMENT);
        std::string fullFragFile = osgDB::findDataFile(fragFile);
        fragShader->loadShaderSourceFromFile(fullFragFile); 
    }
    
    osg::Shader* vertShader = NULL;
    if(shadersAreSource){
        vertShader = new osg::Shader(osg::Shader::VERTEX, vertFile.c_str());
    }else{
        vertShader = new osg::Shader(osg::Shader::VERTEX);
        std::string fullVertFile = osgDB::findDataFile(vertFile);
        vertShader->loadShaderSourceFromFile(fullVertFile); 
    }
    
    osg::Program* program = new osg::Program(); 
    program->setName("customShader"); 
    
    program->addShader(fragShader); 
    program->addShader(vertShader); 
    _stateset->setAttributeAndModes(program, osg::StateAttribute::ON); 	
    _shaderMode = CUSTOM_SHADER;
}

//
//disable color writes, i.e. only write to depth buffer
//
void QuadGeode::DisableColorWrites()
{
    this->SetColorWriteMask(false);
}
//
//Reenable color writes
//
void QuadGeode::EnableColorWrites()
{
    this->SetColorWriteMask(true);
}

//
void QuadGeode::SetColorWriteMask(const bool& enable)
{
    osg::ColorMask* colorMask = new osg::ColorMask();
    colorMask->setMask(enable, enable, enable, enable);
    _stateset->setAttributeAndModes(colorMask, osg::StateAttribute::ON);  
}

//
//Disable depth writes
//
void QuadGeode::DisableDepthWrites()
{
    this->SetDepthWriteMask(false);
}
//
//Enable depth writes
//
void QuadGeode::EnableDepthWrites()
{
    this->SetDepthWriteMask(true);
}

//
void QuadGeode::SetDepthWriteMask(const bool& enable)
{
    osg::Depth* depth = new osg::Depth();
    depth->setWriteMask(false);
    _stateset->setAttributeAndModes(depth, osg::StateAttribute::ON);     
}

//
//Disable depth testing
//
void QuadGeode::DisableDepthTest()
{
    this->SetDepthTestEnabled(false);
}

//
//Enable depth testing
//
void QuadGeode::EnableDepthTest()
{
    this->SetDepthTestEnabled(true);
}

//
void QuadGeode::SetDepthTestEnabled(const bool& enable)
{
    _stateset->setMode(GL_DEPTH_TEST,enable ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
}

//
//Enable fast drawm, just disables depth writes and testing
//
void QuadGeode::EnableFastDraw()
{
    //this->DisableLighting();
    this->DisableDepthTest();
    this->DisableDepthWrites();
}

//
//Set the renderbin number
void QuadGeode::SetRenderBinNumber(const int& num)
{
    _stateset->setRenderBinDetails(num, "RenderBin");
}
