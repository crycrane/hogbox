// HogSandBox.cpp : Defines the entry point for the console application.
//

#include <hogbox/HogBoxViewer.h>
#include <hogbox/HogBoxObject.h>
#include <hogbox/HogBoxLight.h>
#include <hogbox/HogBoxUtils.h>

#include <hogboxDB/HogBoxManager.h>
#include <hogboxDB/HogBoxRegistry.h>

#include <hogboxVision/VisionRegistry.h>

#include <hogbox/HogBoxNotifyHandler.h>

#include <hogboxHUD/HogBoxHud.h>
#include <hogboxHUD/ButtonRegion.h>
#include <hogboxHUD/OsgInput.h>

#include <hogboxStage/EntityManager.h>
#include <hogboxStage/RenderableComponent.h>

#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/AnimationManagerBase>

//
//finds and returns the fist AnimationManagerBase in the subgraph
struct AnimationManagerFinder : public osg::NodeVisitor
{
    osg::ref_ptr<osgAnimation::BasicAnimationManager> _am;
    AnimationManagerFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    void apply(osg::Node& node) {
        if (_am.valid())
            return;
        if (node.getUpdateCallback()) {
            osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
            if (b) {
                _am = new osgAnimation::BasicAnimationManager(*b);
				node.setUpdateCallback(_am.get());
                return;
            }
        }
        traverse(node);
    }
};


int main( int argc, const char* argv[] )
{
	
	//if we're on mac working directory returns the users root by default
	//but the the working directory argument argc[0] returns what I am used to on windows 
#ifndef WIN32
	
	char* appRootPath;
	//get the command arg
	std::string str = std::string(argv[0]);
	osg::notify(osg::WARN) << "Start " << str << std::endl;
	
	//move up in the bundle struncture to the Content folder from MacOS folder  
	size_t endpos = str.find_last_of("/"); // remove after last slash 
	str = str.substr( 0, endpos ); 
	
	//and again
	//endpos = str.find_last_of("/");  
	//str = str.substr( 0, endpos ); 	
	
	//move into resources
	//str = str + "/Resources";
	
	//set resources as the current dirrectory
	osg::notify(osg::WARN) << "try " << str << std::endl;
	hogbox::SetWorkingDirectory(str.c_str());
	
	//accuire the apps path, to check it worked and mimic windows
	appRootPath = new char[512];
	hogbox::GetWorkingDirectory(1024, appRootPath);
	
	osg::notify(osg::WARN) << "Mac OS X Current Directory " << std::string(appRootPath) << std::endl;

#endif	
	
	osg::setNotifyHandler(new hogbox::HogBoxNotifyHandler("./Data/MessageLog.html"));
	osg::setNotifyLevel(osg::INFO);

	//AnimationSplitter spiltter("./splitConfig.xml");
	//return 0;

	hogboxDB::HogBoxManager* manager = hogboxDB::HogBoxManager::Instance();
	manager->ReadDataBaseFile("./Data/hogboxDB.xml");

	//instance the entityManager to add its xml system to the hogboxDB Registry
	hogboxStage::EntitytManager* entityManager = hogboxStage::EntitytManager::Instance();


	//load the main window
	hogbox::HogBoxViewerPtr viewer = manager->ReadNodeByIDTyped<hogbox::HogBoxViewer>("MainWindow");

	osg::MatrixTransform* root = new osg::MatrixTransform();
	viewer->SetSceneNode(root);

	//load the main light
	hogbox::HogBoxLightPtr light1 = manager->ReadNodeByIDTyped<hogbox::HogBoxLight>("MainLight");

	//light1->ApplyLightToGraph(root);//(Don't like this)
	//root->addChild(light1->GetLight());

	//load our main object
	hogbox::HogBoxObjectPtr hogboxObject = manager->ReadNodeByIDTyped<hogbox::HogBoxObject>("Terrorist.Object");
	//root->addChild(osgDB::readNodeFile("./Data/Models/Terrorist/terrorist.FBX"));
	root->addChild(hogboxObject->GetRootNode());

	hogboxStage::EntityPtr entity = manager->ReadNodeByIDTyped<hogboxStage::Entity>("Terrorist.Entity");
	//add a renderable component to the entity
	entity->AddComponent(new hogboxStage::RenderableComponent(entity.get()));
//entityManager->

	//play the animation
	AnimationManagerFinder aniFinder;
	root->accept(aniFinder);
	osgAnimation::BasicAnimationManager* anim = aniFinder._am;
	//root->setUpdateCallback(anim);
	const osgAnimation::AnimationList& list = anim->getAnimationList();
    int v = 0;//getRandomValueinRange(list.size());
        
    anim->playAnimation(list[v].get());
	//anim->playAnimation(list[v].get());


	//load the webcam
	//hogboxVision::WebCamStreamPtr webcam = manager->ReadNodeByIDTyped<hogboxVision::WebCamStream>("MainWebCam");

	//add hud
	hogboxHUD::HogBoxHud::Instance()->Create(osg::Vec2(800,600));
	//root->addChild(hogboxHUD::HogBoxHud::Instance()->GetHudNode());
	
	//add input handler for hud
	hogboxHUD::HudInputHandler* input = new hogboxHUD::HudInputHandler(viewer->GetViewer(),osg::Vec2(800,600));
	viewer->addEventHandler(input);

	//add a region to the hud
	osg::ref_ptr<hogboxHUD::ButtonRegion> region = manager->ReadNodeByIDTyped<hogboxHUD::ButtonRegion>("Button.SwapCamera");
	hogboxHUD::HogBoxHud::Instance()->AddRegion(region);

	//add a camera manipulator to control camera
	osg::ref_ptr<osgGA::TrackballManipulator> cameraManipulator;
    cameraManipulator = new osgGA::TrackballManipulator; 
	//cameraManipulator->setHomePosition(osg::Vec3(0,-100,0), osg::Vec3(0,0,0),osg::Vec3(0,0,1));
	
	//add the cameraManipulator to the hogviewer
	viewer->GetViewer()->setCameraManipulator(cameraManipulator);

	while(!viewer->done())
	{
		viewer->frame();
		//if(region->IsPressed())
		{
		//	printf("click\n");
		}
	}

	viewer = NULL;

	return 0;
}
