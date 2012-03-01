/* Written by Thomas Hogarth, (C) 2011
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 */


#pragma once

#include <hogbox/HogBoxBase.h>

#include <osg/Group>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

#include <hogboxVision/VideoLayer.h>

//Bugs
//Think there is a bug at the minute which cuases every other pass to be upside down
//probably caused by the Ortho2DLayer

namespace hogboxVision {

//
//UNIFORM HELPER
class UniformAtts
{
public:
	UniformAtts(void){}
	UniformAtts(int min, int max)
	{
		_min=min;
		_max=max;
		_uniform=NULL;
	}
	~UniformAtts(void)
	{
		_uniform=NULL;
	}

	osg::ref_ptr<osg::Uniform> _uniform; 
	int _min;
	int _max;

	osg::Uniform* GetUniform(){return _uniform.get();}
};


class RTTPass : public osg::Object
{
public:

	typedef osg::Texture2D TextureType;
	typedef osg::ref_ptr<TextureType> TextureRef;
	typedef std::map<std::string, TextureRef> SamplerToTextureMap;
	typedef std::pair<std::string, TextureRef> SamplerToTexturePair;

	struct RTTArgs {

		RTTArgs()
			: clearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f)), 
            outWidth(256),
			outHeight(256),
			requiredOutCount(1),
			requiredInCount(0),
			startChannel(0),
			rttScene(NULL),
            shaderFileIsSource(false),
            depthRender(false)
		{}
        
        osg::Vec4 clearColor;
		
		int outWidth; //output texture width
		int outHeight; //output texture height
		int requiredOutCount; //the number of output textures bound to the camera

		int requiredInCount; //the number of input textures required
		int startChannel; //the first input channel for textures to be bound (then incs by one per input unit, default is 0)

		//list of input sampler names and their associated textures if inCount > 0
		SamplerToTextureMap inputTextures;

		//if the input texture count is 0 a model can be passed instead, which
		//is rendered using the provided modelview and projection matrix
		osg::ref_ptr<osg::Node> rttScene;
		osg::Matrix projectMatrix;
		osg::Matrix modelViewMatrix;

		//the fragment shader used (can be  empty)
		std::string fragmentShaderFile;
		//the vertex shader used (can be  empty)
		std::string vertexShaderFile;
        
        //flag this to pass actual shader source in the strings above rather then loading them from disk
        bool shaderFileIsSource;
        
        //should we render a depth texture for output 0
        bool depthRender;
	};

	//
	//constructor 
    RTTPass();

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	RTTPass(const RTTPass&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Object(hogboxVision, RTTPass);


	//init the render to texture pass, 
	virtual bool Init(RTTArgs args);


    osg::ref_ptr<osg::Group> getRoot() { return _rootGroup; }
    TextureRef getOutputTexture(int i) { return _outTextures[i]; }
    
	int getOutputWidth(){return _outputWidth;}
	int getOutputHeight(){return _outputHeight;}

	//query the number of inputs and ouputs

	//
	//returns the number of input textures required for the pass
	int getRequiredInputCount();
	//
	//return the current number of textures currently bound
	int getCurrentInputCount();

	//
	//returns the number of output textures that have been created and attached
	int getOutputCount();

	//
	//set a texture to a chosen channel also setting its related uniform variable
	//
	bool setInputTexture(int channel, TextureRef tex, std::string uniformName);
	
	//
	//Add an input variable (uniform) to our
	//gl state to be passed to the shader pass
	//min max passed to set the UniformAtts struct used
	//for creating the GUI
	bool addInputUniform(osg::Uniform* uniform, int min, int max);
	
protected:

	//
	virtual ~RTTPass();
	
	//
	//create/allocate the number of required textures for 
	//the ouput textures
	void createOutputTextures();
	
	//
	//setup the a camera to do our render to texture
	void setupCamera();

    
	//
	//load fragment shader, create program
	//attach to the stateset
	void setFragmentShader(const std::string& filename, bool shaderFileIsSource);

	//
	//load vertex shader, create program
	//attach to the stateset
	void setVertexShader(const std::string& filename, bool shaderFileIsSource);

protected:
//Members
    
    osg::Vec4 _clearColor;
    
	//scenegraph nodes to setup our fullscreen prerender pass
    osg::ref_ptr<osg::Group> _rootGroup;
    osg::ref_ptr<osg::Camera> _camera;
	osg::ref_ptr<Ortho2DLayer> _videoQuad;
    
    //if its a model render flag this to render a depth texture
    bool _depthRender;
	
	//list of input textures needed for the pass
	unsigned int _requiredInputTextures;
	SamplerToTextureMap _inTextures;
	
	//channel to bind net input texture to
	//set by init args startChannel then increased
	//as each input texture is bound
	unsigned int _channelIndex;

	//list of output textures created by this pass
	unsigned int _outputTextureCount;
	std::vector<TextureRef> _outTextures;
	//the output dimensions
    int _outputWidth;
    int _outputHeight;

	//gl state and shaders
    osg::ref_ptr<osg::Program> _shaderProgram;
    osg::ref_ptr<osg::StateSet> _stateSet;

	//list of uniform variables used by the pass
	std::vector<UniformAtts> _uniformList;
};

typedef osg::ref_ptr<RTTPass> RTTPassPtr;

};