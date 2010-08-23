// HogSandBox.cpp : Defines the entry point for the console application.
//

#include <tchar.h>
#include <hogbox/HogBoxViewer.h>
#include <hogbox/HogBoxObject.h>
#include <hogbox/HogBoxLight.h>

#include <hogboxDB/HogBoxManager.h>
#include <hogboxDB/HogBoxRegistry.h>

#include <hogbox/HogBoxNotifyHandler.h>

#include <hogboxHUD/HogBoxHud.h>
#include <hogboxHUD/ButtonRegion.h>
#include <hogboxHUD/OsgInput.h>

int _tmain(int argc, _TCHAR* argv[])
{
	osg::setNotifyHandler(new hogbox::HogBoxNotifyHandler("./Data/MessageLog.html"));
	osg::setNotifyLevel(osg::NOTICE);

	hogboxDB::HogBoxManager* manager = hogboxDB::HogBoxManager::Instance();
	manager->ReadDataBaseFile("Data/hogboxDB.xml");

	hogbox::HogBoxViewerPtr viewer = manager->ReadNodeByIDTyped<hogbox::HogBoxViewer>("MainWindow");

	osg::MatrixTransform* root = new osg::MatrixTransform();
	viewer->SetSceneNode(root);


	hogbox::HogBoxLightPtr light1 = manager->ReadNodeByIDTyped<hogbox::HogBoxLight>("MainLight");

	light1->ApplyLightToGraph(root);//(Don't like this)
	root->addChild(light1->GetLight());

	hogbox::HogBoxObjectPtr hogboxObject = manager->ReadNodeByIDTyped<hogbox::HogBoxObject>("BoxMan.Object");
	root->addChild(hogboxObject->GetRootNode());

	//add hud
	hogboxHUD::HogBoxHud::Instance()->Create(osg::Vec2(800,600));
	root->addChild(hogboxHUD::HogBoxHud::Instance()->GetHudNode());
	
	//add input handler for hud
	hogboxHUD::HudInputHandler* input = new hogboxHUD::HudInputHandler(viewer->GetViewer(),osg::Vec2(800,600));
	viewer->addEventHandler(input);

	//add a region to the hud
	osg::ref_ptr<hogboxHUD::ButtonRegion> region = new hogboxHUD::ButtonRegion();
	region->Create(osg::Vec2(20,20),osg::Vec2(50,50), "Quad", "Click");
	region->setName(std::string("MyHudRegion"));

	hogboxHUD::HogBoxHud::Instance()->AddRegion(region);

	//region->SetBaseTexture(mat->GetTexture(0));
	//region->ApplyBaseTexture();

	//add a camera manipulator to control camera
	osg::ref_ptr<osgGA::TrackballManipulator> cameraManipulator;
    cameraManipulator = new osgGA::TrackballManipulator; 
	cameraManipulator->setHomePosition(osg::Vec3(0,-100,0), osg::Vec3(0,0,0),osg::Vec3(0,0,1));
	
	//add the cameraManipulator to the hogviewer
	//viewer->addEventHandler(cameraManipulator.get());
	viewer->GetViewer()->setCameraManipulator(cameraManipulator);

	while(!viewer->done())
	{
		//viewer->SetCameraViewMatrix(cameraManipulator->getInverseMatrix());
		//viewer->SetCameraViewDistance( cameraManipulator->getDistance() );
		//root->setMatrix(cameraManipulator->getMatrix());
		viewer->frame();

		if(region->IsPressed())
		{
			printf("click\n");
		}
	}

	viewer = NULL;

	return 0;
}

