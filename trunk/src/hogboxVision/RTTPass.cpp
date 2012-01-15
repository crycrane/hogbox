#include <hogboxVision/RTTPass.h>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osg/Material>
#include <osg/PolygonMode>

#include <iostream>

using namespace hogboxVision;

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

static const char* depthPassVertSource = { 
	SHADER_COMPAT 
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "attribute vec4 osg_Vertex;\n"
    
    "void main(void)\n"
    "{\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "}\n"  
}; 

static const char* depthPassFragSource = { 
	SHADER_COMPAT 
    "void main(void)\n"
    "{\n"	
    "    gl_FragColor.rgba = gl_FragCoord.zzzz;\n"
    "}\n"
};




RTTPass::RTTPass() 
	: osg::Object(),
    _depthRender(false),
	_outputWidth(0),
	_outputHeight(0),
	_requiredInputTextures(0),
	_outputTextureCount(0),
	_channelIndex(0)
{
	OSG_NOTICE << "RTTPass: Constructor" << std::endl;
}

RTTPass::~RTTPass()
{
	OSG_NOTICE << "    Deallocating RTTPass: Name '" << this->getName() << "'." << std::endl;

	_uniformList.clear();
	_stateSet = NULL;
	_videoQuad = NULL;
	_camera = NULL;
	_rootGroup = NULL;

	_outTextures.clear();
	_inTextures.clear();

}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
RTTPass::RTTPass(const RTTPass& pass,const osg::CopyOp& copyop)
	: osg::Object(pass, copyop)
{
}

//
//init the render to texture pass, 
//
bool RTTPass::Init(RTTArgs args)
{
    _clearColor = args.clearColor;
    _depthRender = args.depthRender;
	_outputWidth = args.outWidth;
	_outputHeight = args.outHeight;
	_requiredInputTextures = args.requiredInCount;
	_outputTextureCount = args.requiredOutCount;
	_channelIndex = args.startChannel;

    _rootGroup = new osg::Group;
   
	//create output textures ready to be bound
	//to the camera
    createOutputTextures();

	//default camera setup for screen algined rtt
    _camera = new osg::Camera;
	_camera->setName("RttPass Camera");
    setupCamera();

	//if we have input textures create our screen alighned quad to render them full viewport
	if(_requiredInputTextures > 0)
	{
		_videoQuad = new Ortho2DLayer(_outputWidth, _outputHeight, Ortho2DLayer::ORTHO_NESTED);
		_camera->addChild(_videoQuad.get());
		//ensure camera only creates a color buffer for fullscreen rtt and doesn't perform any clears
		_camera->setImplicitBufferAttachmentMask(osg::DisplaySettings::IMPLICIT_COLOR_BUFFER_ATTACHMENT);
		_camera->setClearMask(0);
		_stateSet = _videoQuad->getOrCreateStateSet();

		//apply all the textures to the fsquads render stateset
		for(SamplerToTextureMap::iterator it=args.inputTextures.begin(); it != args.inputTextures.end(); it++)
		{
			this->setInputTexture(_channelIndex, (*it).second, (*it).first);
			_channelIndex++;
		}

	}else{

		//adjust camera for render scene to texture
		_camera->setProjectionMatrix(args.projectMatrix);
		//_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		_camera->setViewMatrix(args.modelViewMatrix);

		//attach the scene to the camera
		_camera->addChild(args.rttScene);
        
        if(_depthRender){
            //if we are doing a depth render we also want to override the shaders to render depth tp color
            osg::Program* program = new osg::Program; 
            program->setName("depthToColorShader"); 
            program->addShader(new osg::Shader(osg::Shader::VERTEX, depthPassVertSource)); 
            program->addShader(new osg::Shader(osg::Shader::FRAGMENT, depthPassFragSource)); 
            _camera->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        }
	}

	//attach the camera to the root
    _rootGroup->addChild(_camera.get());


	//apply shaders if passed in
	if(!args.vertexShaderFile.empty())
	{this->setVertexShader(args.vertexShaderFile, args.shaderFileIsSource);}
	if(!args.fragmentShaderFile.empty())
	{this->setFragmentShader(args.fragmentShaderFile, args.shaderFileIsSource);}

	return true;
}


void RTTPass::setupCamera()
{
    // clearing
    _camera->setClearColor(_clearColor);
    _camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // projection and view
	int w=_outputWidth;
	int h=_outputHeight;

    _camera->setProjectionMatrix(osg::Matrix::ortho2D(0, w, 0, h));
    _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _camera->setViewMatrix(osg::Matrix::identity());

    _camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
    
    // viewport
    _camera->setViewport(0, 0, _outputWidth, _outputHeight);

    _camera->setRenderOrder(osg::Camera::PRE_RENDER);
	_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
/*	FRAME_BUFFER_OBJECT,
	PIXEL_BUFFER_RTT,
	PIXEL_BUFFER,
	FRAME_BUFFER,
	SEPERATE_WINDOW */
	
    // attach the output textures
    for (unsigned int i=0; i<_outTextures.size(); i++) {
        //for now if we are using depthRender attach it to output 0
        if(_depthRender && i==0){
            _camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0), _outTextures[i].get());
        }else{
            _camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0+i), _outTextures[i].get());
        }
    }
}

//
//createOutputTextures
//Create a texture of the required output dimensions
//for as many output Textures as we require and add to
// the list _outTextures[]
//
void RTTPass::createOutputTextures()
{
    for (unsigned int i=0; i<_outputTextureCount; i++) 
	{	
		TextureRef newTex = new TextureType();
		_outTextures.push_back(newTex);

		std::ostringstream samplerName;
		samplerName << "rttTexture" << i;
		_outTextures[i]->setName(samplerName.str());
		
		_outTextures[i]->setTextureSize(_outputWidth, _outputHeight);
        //for now if we are using depthRender attach it to output 0
        if(_depthRender && i==0){
            //_outTextures[i]->setInternalFormat(GL_LUMINANCE);
            _outTextures[i]->setInternalFormat(GL_RGBA);
            //_outTextures[i]->setInternalFormat(GL_RGBA32F_ARB);
            //_outTextures[i]->setSourceFormat(GL_RGBA);
        }else{
            _outTextures[i]->setInternalFormat(GL_RGBA);
            //_outTextures[i]->setInternalFormat(GL_RGBA32F_ARB);
            _outTextures[i]->setSourceFormat(GL_RGBA);            
        }
		_outTextures[i]->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
		_outTextures[i]->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
		_outTextures[i]->setResizeNonPowerOfTwoHint(false);
    }
}

void RTTPass::setFragmentShader(const std::string& filename, bool shaderFileIsSource)
{
    osg::ref_ptr<osg::Shader> fshader = NULL;
    
    if(shaderFileIsSource){
        fshader = new osg::Shader( osg::Shader::FRAGMENT, filename.c_str()); 
    }else{
        fshader = new osg::Shader( osg::Shader::FRAGMENT ); 
        fshader->loadShaderSourceFromFile(osgDB::findDataFile(filename));
    }

	//ensure our program has been created
	if(!_shaderProgram)
	{
		_shaderProgram = 0;
		_shaderProgram = new osg::Program();
	}

	//add the frag shader to the program
    _shaderProgram->addShader(fshader.get());
	//ensure program is attached to target statest
    _stateSet->setAttributeAndModes(_shaderProgram.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
}

//
//load vertex shader, create program
//attach to the stateset
//
void RTTPass::setVertexShader(const std::string& filename, bool shaderFileIsSource)
{
	osg::ref_ptr<osg::Shader> vshader = NULL;
    if(shaderFileIsSource){
        vshader = new osg::Shader( osg::Shader::VERTEX, filename.c_str()); 
    }else{
        vshader = new osg::Shader( osg::Shader::VERTEX ); 
        vshader->loadShaderSourceFromFile(osgDB::findDataFile(filename));
    }

	//ensure our program has been created
	if(!_shaderProgram)
	{
		_shaderProgram = 0;
		_shaderProgram = new osg::Program;
	}

	//add the frag shader to the program
    _shaderProgram->addShader(vshader.get());
	//ensure program is attached to target statest
    _stateSet->setAttributeAndModes(_shaderProgram.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

}

//
//returns the number of input textures required for the pass
//
int RTTPass::getRequiredInputCount()
{
	return _requiredInputTextures;
}

//
//return the current number of input textures that have been set
// via the setInputTextureFunction
//
int RTTPass::getCurrentInputCount()
{
	int validCount = 0;
	//loop the inTextures array and see how many our now valid
	/*for(unsigned int i=0; i<_requiredInputTextures; i++)
	{
		if(_inTextures[i].valid())
		{validCount++;}
	}*/
	validCount = _inTextures.size();
	return validCount;
}

//returns the number of output textures that have been created and attached
int RTTPass::getOutputCount()
{
	return _outputTextureCount;
}

//
//add a texture to our input list, 
//attaching it to our stateset as well as creating a uniform to 
//inform the shader which channel it is bound to
//	
bool RTTPass::setInputTexture(int channel, TextureRef tex, std::string uniformName)
{
	//check the channel bounds
	if(channel < 0)
	{return false;}

	SamplerToTexturePair textureSampler(uniformName, tex);
	//_inTextures[channel] = tex;
	_inTextures.insert(textureSampler);

	if(_videoQuad.get())
	{
		_videoQuad->getOrCreateStateSet()->setTextureAttributeAndModes(channel, tex, osg::StateAttribute::ON);
	}

	//if we are using screen aligned render
	//if(_videoQuad){_videoQuad->setTextureFromTexRect(tex, channel);}
	_stateSet->addUniform(new osg::Uniform(uniformName.c_str(), channel));

	return true;
}

//
//Add an input variable (uniform) to our
//gl state to be passed to the shader pass
//min max passed to set the UniformAtts struct used
//for creating the GUI
//
bool RTTPass::addInputUniform(osg::Uniform* uniform, int min, int max)
{
	UniformAtts atts = UniformAtts(min, max);
	//atts.m_uniform = uniform;
	_uniformList.push_back(atts);

	_stateSet->addUniform(uniform);
	return true;
}
