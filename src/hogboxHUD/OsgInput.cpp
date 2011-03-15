#include <hogboxHUD/OsgInput.h>

#include <hogboxHUD/HogBoxHud.h>

using namespace hogboxHUD;

HudInputHandler::HudInputHandler(osgViewer::Viewer* sceneView, osg::Vec2 hudDimensions)
	: m_inputState(new HudInputEvent()),
	_sceneView(sceneView),
	p_focusRegion(NULL),
	m_hudDimensions(hudDimensions)
{
	
}

HudInputHandler::~HudInputHandler()
{

}

//
//override the GUIEventHandler handle function to process user input
//
bool HudInputHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
	//handle the event type
    switch(ea.getEventType())
    {

		//key is pressed down
		case(osgGA::GUIEventAdapter::KEYDOWN):
        {
			//add the key to our list of pressed keys for the frame
			m_inputState->PressKey(ea.getKey());			
			//add to held keys
			m_inputState->PressHeldKey(ea.getKey()); 

			m_inputState->SetEvent(ON_KEY_DOWN, ea, m_hudDimensions);
			break;
        }

		//key released
		case(osgGA::GUIEventAdapter::KEYUP):
        {
			//remove key from held list
			m_inputState->ReleaseHeldKey(ea.getKey());
			
			m_inputState->SetEvent(ON_KEY_UP, ea, m_hudDimensions);
			break;
		}

		//mouse moving
		case(osgGA::GUIEventAdapter::MOVE):
        {
			pick(ea,true); //hover check
			
			m_inputState->SetEvent(ON_MOUSE_MOVE, ea, m_hudDimensions);
			break;
		}

		//mouse drag (moving with button held)
		case(osgGA::GUIEventAdapter::DRAG):
        {
			//pass drag to our infoucs region
			m_inputState->SetEvent(ON_MOUSE_DRAG, ea, m_hudDimensions);
			break;
        } 

		//mouse down
		case(osgGA::GUIEventAdapter::PUSH):
		{
			pick(ea,true);//MDOWN
			pick(ea,false);//MDOWN
			//pass mouse down to our infoucs region
			m_inputState->SetEvent(ON_MOUSE_DOWN, ea, m_hudDimensions);
			break;
		}
	
		//mouse up
		case(osgGA::GUIEventAdapter::RELEASE):
        {
			pick(ea,true);//MDOWN
			pick(ea,false);//MDOWN
			//pass mouse up to our infoucs region
			m_inputState->SetEvent(ON_MOUSE_UP, ea, m_hudDimensions);
			break;
        } 

		//double click do down and up
		case(osgGA::GUIEventAdapter::DOUBLECLICK):
        {
			pick(ea,true);//MDOWN
			pick(ea,false);//MDOWN
			//pass double click to our infoucs region
			m_inputState->SetEvent(ON_DOUBLE_CLICK, ea, m_hudDimensions);
			break;
        } 

        default:break;
    }

	if(p_focusRegion)
	{
		p_focusRegion->HandleInputEvent(*m_inputState.get());
	}else{
		if(p_clickObject){
			osg::Referenced* userData = p_clickObject->getUserData();
			CallbackEvent* callback = dynamic_cast<CallbackEvent*>(userData);
			if(callback){
				callback->TriggerEvent(*m_inputState.get());
			}
			//clear
			p_clickObject=NULL;
		}
	}
	return false;
}

//
//Pick is called by mouse up/down and mouse move to select a new focus region
//the focus region then receives all other inputs i.e. keys, mouseDrags etc
//
void HudInputHandler::pick(const osgGA::GUIEventAdapter& ea, bool hudPick) //mode 0 = push, 1=release, 2 = double click
{
	//select the sub graph to pick from based on hudPick
	osg::Node* scene = NULL;
	if(hudPick){
		//get our hud node
		scene = HogBoxHud::Instance()->GetHudNode();
	}else{
		//get entire scene
		scene = _sceneView->getCamera();
	}
	//if no scene then no point continuing
    if (scene == NULL) return;

	//when we pick disable any unpickable nodes and store their current mask as prev
	for(unsigned int i = 0; i<m_vUnPickableNodes.size(); i++)
	{
		m_vUnPickableNodes[i]._prevMask = m_vUnPickableNodes[i]._node->getNodeMask();
		m_vUnPickableNodes[i]._node->setNodeMask(0x0);
	}

	osg::Node* node = 0;
	osg::Group* parent = 0;

	//create our intersector to pick mouse coord projected into scene
    osgUtil::LineSegmentIntersector* picker;
    picker = new osgUtil::LineSegmentIntersector( osgUtil::Intersector::PROJECTION, ea.getXnormalized(),ea.getYnormalized() );

	//pass intersector through scene find the intersections
    osgUtil::IntersectionVisitor iv(picker);
	if(!hudPick){
		iv.setTraversalMask(hogbox::NodeMasks::PICK_MESH);
	}
	scene->accept(iv);

	//toggle unpickable nodes back to previous
	for(unsigned int i = 0; i<m_vUnPickableNodes.size(); i++)
	{
		m_vUnPickableNodes[i]._node->setNodeMask(m_vUnPickableNodes[i]._prevMask);
	}

	//get simplified mouse coord
	osg::Vec2 mCoords = osg::Vec2(ea.getX(),ea.getY());
	osg::Vec2 mChange = osg::Vec2(0,0);

	//picker hit somthing
    if (picker->containsIntersections())
    {
		int numInterSects = picker->getIntersections().size();
		osgUtil::LineSegmentIntersector::Intersection intersection = picker->getFirstIntersection();

		//get first valid node
        osg::NodePath& nodePath = intersection.nodePath;
        node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
        parent = (nodePath.size()>=2)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]):0;

		OSG_INFO << "hogboxHUD HudInputHandler: Picked node '" << node->getName() << "'." << std::endl;
		
		//did we pick a node
        if (node)// && (node->getName().size() != 0) )
		{
			//successfully picked a node so store it then pass its name down to the basic hud system
			p_clickObject = NULL;
			//check if the node has user data
			HudRegion* geodeRegion = dynamic_cast<HudRegion*> (node->getUserData());
			SetFocusRegion(geodeRegion);
			if(!geodeRegion){
				p_clickObject = node;
			}

		}else{

			p_clickObject=NULL;
			SetFocusRegion(NULL);
		}

	
	}else{ //no node picked

		SetFocusRegion(NULL);
	}
}

//
//Set a new focus object, this will also inform the previous focus object
//of the mouseLeave event, and the new object of the mouseEnter event
//
void HudInputHandler::SetFocusRegion(HudRegion* focusRegion)
{
	if(focusRegion)
	{
		osg::notify(osg::DEBUG_FP) << "hogboxHUD HudInputHandler: Switching focus to region '" << focusRegion->getName() << "'."  << std::endl;
	}
	p_focusRegion = focusRegion;
}

