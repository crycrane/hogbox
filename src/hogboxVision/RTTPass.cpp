#include <hogboxVision/RTTPass.h>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osg/Material>
#include <osg/PolygonMode>

#include <iostream>

using namespace hogboxVision;

RTTPass::RTTPass() 
	: osg::Object(),
	_outputWidth(0),
	_outputHeight(0),
	_requiredInputTextures(0),
	_outputTextureCount(0)
{
	osg::notify(osg::WARN) << "RTTPass: Constructor" << std::endl;
}

RTTPass::~RTTPass()
{
	osg::notify(osg::WARN) << "RTTPass: Destructor" << std::endl;

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
	_outputWidth = args.outWidth;
	_outputHeight = args.outHeight;
	_requiredInputTextures = args.requiredInCount;
	_outputTextureCount = args.requiredOutCount;


    _rootGroup = new osg::Group;
   
	//create output textures ready to be bound
	//to the camera
    createOutputTextures();

	//default camera setup for screen algined rtt
    _camera = new osg::Camera;
    setupCamera();

	//if we have input textures create our screen alighned quad to render them full viewport
	if(_requiredInputTextures > 0)
	{
		_videoQuad = new FSVideoLayer(_outputWidth, _outputHeight);
		_camera->addChild(_videoQuad.get());
		_stateSet = _videoQuad->getOrCreateStateSet();

		int channel = 0;
		//apply all the textures to the fsquads render stateset
		for(SamplerToTextureMap::iterator it=args.inputTextures.begin(); it != args.inputTextures.end(); it++)
		{
			this->setInputTexture(channel, (*it).second, (*it).first);
			channel++;
		}

	}else{

		//adjust camera for render scene to texture
		_camera->setProjectionMatrix(args.projectMatrix->getMatrix());
		_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		_camera->setViewMatrix(args.modelViewMatrix->getMatrix());

		//attach the scene to the camera
		_camera->addChild(args.rttScene);
	}

	//attach the camera to the root
    _rootGroup->addChild(_camera.get());


	//apply shaders if passed in
	if(!args.vertexShaderFile.empty())
	{this->setVertexShader(args.vertexShaderFile);}
	if(!args.fragmentShaderFile.empty())
	{this->setFragmentShader(args.fragmentShaderFile);}

	return true;
}


void RTTPass::setupCamera()
{
    // clearing
    _camera->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
    _camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // projection and view
	int w=_outputWidth;
	int h=_outputHeight;

    _camera->setProjectionMatrix(osg::Matrix::ortho2D(0, w, 0, h));
    _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _camera->setViewMatrix(osg::Matrix::identity());

    // viewport
    _camera->setViewport(0, 0, _outputWidth, _outputHeight);

    _camera->setRenderOrder(osg::Camera::PRE_RENDER);
	_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    
    // attach the 4 textures
    for (int i=0; i<_outTextures.size(); i++) {
		_camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0+i), _outTextures[i].get());
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
    for (int i=0; i<_outputTextureCount; i++) 
	{	
		osg::ref_ptr<osg::TextureRectangle> newTex = new osg::TextureRectangle();
		_outTextures.push_back(newTex);

		_outTextures[i]->setTextureSize(_outputWidth, _outputHeight);
		//_outTextures[i]->setInternalFormat(GL_RGBA);
		_outTextures[i]->setInternalFormat(GL_RGBA32F_ARB);
	    _outTextures[i]->setSourceFormat(GL_RGBA);
		//_outTextures[i]->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
		//_outTextures[i]->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
		_outTextures[i]->setResizeNonPowerOfTwoHint(false);
    }
}

void RTTPass::setFragmentShader(const std::string& filename)
{
    osg::ref_ptr<osg::Shader> fshader = new osg::Shader( osg::Shader::FRAGMENT ); 
    fshader->loadShaderSourceFromFile(osgDB::findDataFile(filename));

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
void RTTPass::setVertexShader(const std::string& filename)
{
	osg::ref_ptr<osg::Shader> vshader = new osg::Shader( osg::Shader::VERTEX ); 
    vshader->loadShaderSourceFromFile(osgDB::findDataFile(filename));

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
	//check the channel bounds against required input size
	if(channel < 0 || channel >= _requiredInputTextures)
	{return false;}

	SamplerToTexturePair textureSampler(uniformName, tex);
	//_inTextures[channel] = tex;
	_inTextures.insert(textureSampler);

    //_stateSet->setTextureAttributeAndModes(channel, _inTextures[channel].get(), osg::StateAttribute::ON);

	//if we are using screen aligned render
	if(_videoQuad){_videoQuad->setTextureFromTexRect(tex, channel);}
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
