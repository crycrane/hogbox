// HogSandBox.cpp : Defines the entry point for the console application.
//

#include <tchar.h>
#include <hogbox/HogBoxViewer.h>
#include <hogbox/HogBoxObject.h>

#include <hogboxDB/HogBoxManager.h>
#include <hogboxDB/HogBoxRegistry.h>

#include <hogbox/HogBoxNotifyHandler.h>

#include <hogboxHUD/HogBoxHud.h>
#include <hogboxHUD/HudRegion.h>
#include <hogboxHUD/OsgInput.h>

#include <hogboxVision/VisionRegistry.h>

//
//Create a viewProjection matrix to represent our projector and
//generate our procedural texture coords
//The function calulates the correct perspective projection to fill an object of screenHeight*aspect X screenHeight
//at the screenDistance provided
//
osg::Node* CreateProjectorNode(const osg::Vec3& position, const osg::Vec3& direction, double screenHeight, 
							   double aspect, double screenDistance, unsigned int lightNum, unsigned int textureUnit)
{
    osg::Group* group = new osg::Group;
    
    // create light source.
    osg::LightSource* lightsource = new osg::LightSource;
    osg::Light* light = lightsource->getLight();
    light->setLightNum(lightNum);
    light->setPosition(osg::Vec4(position,1.0f));
    light->setAmbient(osg::Vec4(0.00f,0.00f,0.05f,1.0f));
    light->setDiffuse(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    group->addChild(lightsource);
    
    // create tex gen.
    
    osg::Vec3 up(0.0f,0.0f,1.0f);
    up = (direction ^ up) ^ direction;
    up.normalize();

	 // calculate the appropriate left, right etc.

    float zNear = screenDistance; //mm
    float zFar  = screenDistance*2; //mm
    double right  =  (screenHeight*aspect)*0.5f;
    double left   = -right;
    double top    =  screenHeight*0.5f;
    double bottom =  -top;

    osg::TexGenNode* texgenNode = new osg::TexGenNode;
    texgenNode->setTextureUnit(textureUnit);
    osg::TexGen* texgen = texgenNode->getTexGen();
    texgen->setMode(osg::TexGen::EYE_LINEAR);
    texgen->setPlanesFromMatrix(osg::Matrixd::lookAt(position, position+direction, up)*
								//osg::Matrixd::perspective(fovy,aspect,zNear,zFar)*
								osg::Matrixd::frustum(left,right,bottom,top,zNear,zFar) *
								osg::Matrix::scale(0.5f,0.5f,0.5f) *
								osg::Matrix::translate(0.5,0.5,0.5));

    group->addChild(texgenNode);
    
    return group;
    
}


bool ApplyReceiveProjectionState(osg::StateSet* state, osg::Group* world, int channel)
{
	state->setMode(GL_LIGHT0, osg::StateAttribute::ON);

	//add procedural texture coords
	state->setTextureMode(channel,GL_TEXTURE_GEN_S,osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
    state->setTextureMode(channel,GL_TEXTURE_GEN_T,osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
    state->setTextureMode(channel,GL_TEXTURE_GEN_R,osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
    state->setTextureMode(channel,GL_TEXTURE_GEN_Q,osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

	return true;
}


int _tmain(int argc, _TCHAR* argv[])
{
	osg::setNotifyHandler(new hogbox::HogBoxNotifyHandler("./Data/OsgSandBoxMessageLog.html"));
	osg::setNotifyLevel(osg::NOTICE);

	//load our xml databse
	hogboxDB::HogBoxManager* manager = hogboxDB::HogBoxManager::Instance();
	manager->ReadDataBaseFile("Data/projectionSceneHogBoxDB.xml");

	hogboxVision::VideoFileStreamPtr video = manager->ReadNodeByIDTyped<hogboxVision::VideoFileStream>("TestVideo");
	video->play();

	//master root node attached to the viewer for rendering
	osg::ref_ptr<osg::Group> _root = new osg::Group();

	//The texture to project
	osg::ref_ptr<osg::Texture2D> _projectionTexture = manager->ReadNodeByIDTyped<osg::Texture2D>("ProjectionMap");

	//the root node for the scene we are rendering to texture to be projected
	osg::ref_ptr<osg::MatrixTransform> _projectionScene = new osg::MatrixTransform();

	//the root node for the scene we project onto
	osg::ref_ptr<osg::MatrixTransform> _receiveGroup = new osg::MatrixTransform();

	//assemble base graph
	_root->addChild(_receiveGroup);

	//create the viewer then init with our root node 
	//from here on in the viewer will render anything attached to root
	//when we call frame()
	hogbox::HogBoxViewerPtr _viewer = new hogbox::HogBoxViewer();
	_viewer->Init(_root);


	//create and add the projection node
	_receiveGroup->addChild(CreateProjectorNode(osg::Vec3(2000.0f,0.0f,1000.0f), osg::Vec3(-1,0,0),1500.0f,1.33,2000.0f,0,0));

	//read the world floor object to project onto 
	hogbox::HogBoxObjectPtr _floorObject = manager->ReadNodeByIDTyped<hogbox::HogBoxObject>("World.Floor");
	if(!_floorObject){

	}
	_receiveGroup->addChild(_floorObject->GetRootNode());

	//the stateset containing the projection texture and procedurally generated
	//linear eye mapping coords. This state is applied to any object that want to
	//be projected onto (i.e. the cinema screen)
	hogbox::HogBoxMaterialPtr _receiveProjectionMat = manager->ReadNodeByIDTyped<hogbox::HogBoxMaterial>("ReceiveProjectionMat");


	//apply the projection coords etc to our reciviceProjection material
	ApplyReceiveProjectionState(_receiveProjectionMat->GetStateSet(), _receiveGroup, 0);

	//apply the receiveProjection mat to the entire floor model (which also has the wall etc in it)
	_floorObject->GetRootNode()->setStateSet(_receiveProjectionMat->GetStateSet());

	//add hud
	hogboxHUD::HogBoxHud::Instance()->Create(osg::Vec2(640,480));
	_root->addChild(hogboxHUD::HogBoxHud::Instance()->GetHudNode());
	
	//add input handler for hud
	hogboxHUD::HudInputHandler* input = new hogboxHUD::HudInputHandler(_viewer->GetViewer(),osg::Vec2(640,480));
	//viewer->addEventHandler(input);

	//add a region to the hud
	osg::ref_ptr<hogboxHUD::HudRegion> region = manager->ReadNodeByIDTyped<hogboxHUD::HudRegion>("Hud.Base.Background");
		
	//region->Create(osg::Vec2(20,20),osg::Vec2(50,50), "Quad");
	region->setName(std::string("MyHudRegion"));

	hogboxHUD::HogBoxHud::Instance()->AddRegion(region);

	//add the cameraManipulator to the hogviewer
	_viewer->GetViewer()->setCameraManipulator(new osgGA::TrackballManipulator());

	while(!_viewer->done())
	{
		_viewer->frame();
	}

	return 0;
}

