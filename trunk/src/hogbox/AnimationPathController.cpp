#include <hogbox/AnimationPathControl.h>

#include <osg/Group>

using namespace hogbox;


void SplitAni::ReSampleSubAni(int subID, int from, int to)
{
	if(subID<0 || subID>=(int)m_splitAnis.size())
	{return;}
	//check the original path is good
	if(!m_originalAni)
	{return;}
	
	//clear the current path
	m_splitAnis[subID]->getTimeControlPointMap().clear();

	//find the number of frames in the original animation
	osg::AnimationPath::TimeControlPointMap timeMap = m_originalAni->getTimeControlPointMap();

	//seconds per rame
	int totalFrames = (int)timeMap.size();
	double secsPerFrame = (double)1.0f/m_fps;

	//get start end frames as times
	double frameT = from*secsPerFrame;
	double endT = to*secsPerFrame;

	int newCount=0;

	while(frameT<endT)
	{
		osg::AnimationPath::ControlPoint cp;
		m_originalAni->getInterpolatedControlPoint(frameT, cp);   

		//insert into the subanimation starting from time 0
		m_splitAnis[subID]->insert(newCount*secsPerFrame, cp);

		//step to nxt time
		frameT+=secsPerFrame;

		//increase new frame count
		newCount++;
	}
}

//CONTROL

AnimationPathControl::AnimationPathControl(void)
{
	m_paused = false;
}

AnimationPathControl::~AnimationPathControl(void)
{
	OSG_NOTICE << "    Deallocating AnimationPathControl: Name '" << this->getName() << "'." << std::endl;
	//the group of all entities that are in the markers space
	m_animatedNode = NULL;

	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{
		m_aniCallbacks[i] = NULL;
	}
	m_aniCallbacks.clear();

	//the split paths for each node
	for(unsigned int i=0; i<m_splitAnis.size(); i++)
	{;
		m_splitAnis[i] = NULL;
	}
	m_splitAnis.clear();
}


//
// find all the animations in this node and store pointers in the subAnimation list
//
bool AnimationPathControl::Init(osg::Node* aniNode, int fps, std::string config)
{
	m_fps = fps;

	m_animatedNode = aniNode;
	int total =  FindAnimationNodes(m_animatedNode);
	if(total==0)
	{
		osg::notify(osg::WARN) << "AnimationPathControl::Init: WARN: There are no animation nodes in this MODEL, animation control will not be applied" <<std::endl;  
		return false;
	}

	//now create the sub helpers
	//loop through each callback
	for(unsigned int i=0; i<(int)m_aniCallbacks.size(); i++)
	{
		//get a pointer to the callbacks original path
		osg::AnimationPath* originalPath = m_aniCallbacks[i]->getAnimationPath();

		if(originalPath)
		{
			//find the number of frames in the animation
			osg::AnimationPath::TimeControlPointMap timeMap = originalPath->getTimeControlPointMap();

			//create a new split animation to store this callbacks split version
			SplitAniPtr l_splitAnis = SplitAniPtr(new SplitAni());
			//add the original path to the split ani
			l_splitAnis->SetOriginal(originalPath, m_fps); 

			//add the splitpth list onto our list, should be insame position a our callback index
			m_splitAnis.push_back(l_splitAnis); 
		}
	}

	//if we have a config file use it to set our sub anis
	if(config.size() > 0)
	{SplitFromCfgFile(config);}
	
	return true;
}

//
// Add a generic animation callback, also swapping it for an event based
//
void AnimationPathControl::AddAnimation(osg::Node* node, osg::AnimationPathCallback* ani)
{
	osg::ref_ptr<AnimationPathEventCallback> newCallBack = new AnimationPathEventCallback();
	newCallBack->setAnimationPath(ani->getAnimationPath());
	
	node->setUpdateCallback(newCallBack.get()); 
	
	m_aniCallbacks.push_back(newCallBack);
}

//
// Find any animation callbacks in the node subgraph and add to our list
//
int AnimationPathControl::FindAnimationNodes(osg::Node* node)
{
	// Find animations
	if(dynamic_cast<osg::AnimationPathCallback*> (node->getUpdateCallback()) )
	{
		//animation path callback found
		osg::AnimationPathCallback* nodeCallBack = dynamic_cast<osg::AnimationPathCallback*> (node->getUpdateCallback());
		
		//check the length as 0 length can be exported
		
		//find the number of frames in the animation
		osg::AnimationPath::TimeControlPointMap timeMap = nodeCallBack->getAnimationPath()->getTimeControlPointMap();
		int length = (int)timeMap.size();

		if(length>0)
		{
			//create an ani event callback
			AnimationPathEventCallback* newCallBack = new AnimationPathEventCallback();
			//copy original paths 
			newCallBack->setAnimationPath(nodeCallBack->getAnimationPath());

			//set the new call back to the node
			node->setUpdateCallback(newCallBack); 
			m_aniCallbacks.push_back(newCallBack);//dynamic_cast<osg::AnimationPathCallback*> (node->getUpdateCallback()) );
		
		}else{

			osg::notify(osg::WARN) << "AnimationPathControl: FindAnimationNodes: Warning Node '" << node->getName().c_str() << "' has zero frames, ensure no redundant key frames were included" << std::endl;
		}
	}

	// Traverse any group node
	if(dynamic_cast<osg::Group*> (node))
	{
		osg::Group* group = static_cast<osg::Group*> (node);
		for(unsigned int i=0; i<group->getNumChildren(); i++)
		{
			FindAnimationNodes(group->getChild(i));
		}
	}

	return m_aniCallbacks.size();
}

//
// Add new sub animation for every node
//
void AnimationPathControl::AddNewSubAnimation(std::string name)
{
	for(int i=0; i<(int)m_splitAnis.size(); i++)
	{
		m_splitAnis[i]->AddSplitAni();
	}
	m_animationNames.push_back(name);
}

void AnimationPathControl::AddNewSubAnimation(std::string name, int from, int to)
{
	for(int i=0; i<(int)m_splitAnis.size(); i++)
	{
		m_splitAnis[i]->AddSplitAni();
		int newIndex = m_splitAnis[i]->GetTotalSplitAnimations();
		m_splitAnis[i]->ReSampleSubAni(newIndex-1, from, to);
	}
	m_animationNames.push_back(name);
}

void AnimationPathControl::ReSampleSubAni(int subID, int from, int to)
{
	for(int i=0; i<(int)m_splitAnis.size(); i++)
	{
		m_splitAnis[i]->ReSampleSubAni(subID, from, to);
	}
}

int AnimationPathControl::GetMaxFrames()
{
	if(m_aniCallbacks.size() == 0)
	{return 0;}

	//get a pointer to the callbacks original path
	osg::AnimationPath* originalPath = m_aniCallbacks[0]->getAnimationPath();

	if(originalPath==NULL){return -1;}

	//find the number of frames in the animation
	osg::AnimationPath::TimeControlPointMap timeMap = originalPath->getTimeControlPointMap();

	return (int)timeMap.size();
}

double AnimationPathControl::GetAnimationLength()
{
	if(m_aniCallbacks.size()==0)
	{return 0.0f;}
	return m_aniCallbacks[0]->GetAniLength(); 
}

//
// Split any found animations into subanimations of frames per animation
//
void AnimationPathControl::SplitAnimations(int fpa)
{
	int totalFrames = this->GetMaxFrames();
	double secsPerFrame = (double)1.0f/m_fps;
	
	//the number of frames we have used in the original animation
	int offSet=0;
	int count=0;
	//check if we just want the original animation
	if(fpa != totalFrames)
	{
		//loop adding split frames until we have reached the end of the 
		//original animation
		while(offSet<totalFrames)
		{
			std::ostringstream stream(std::ostringstream::out);
			stream << "Ani-"<<count;
			this->AddNewSubAnimation(stream.str(), offSet, offSet+fpa);

			//increase the offset into the original path
			offSet+=fpa;
			//check it didnt run over
			if(offSet>=totalFrames){break;}
			count++;
		}
	}			
}

//
// Split animations into variable size chunks based on the passed array
// will cut off once the original length is reached
//
void AnimationPathControl::SplitAnimations(std::vector<int> fpa)
{
	//Add a in sub for each
	int pFrame = 0;
	for(unsigned int i=0; i<fpa.size(); i++)
	{
		std::ostringstream stream(std::ostringstream::out);
		stream << "Ani-"<<i;

		this->AddNewSubAnimation(stream.str(), pFrame, pFrame+(fpa[i]-1));
		pFrame += fpa[i];
	}
}

//
//creates the sub animations from frame ranges set in a cfg file
//
void AnimationPathControl::SplitFromCfgFile(std::string file)
{
/*	COsgSettings settings;
	if(settings.LoadSettings(file.c_str()))
	{
		if(settings.FindFirstIndexOf("<ANI_CFG>"))
		{
			while(settings.MoveToNextBefore("</ANI_CFG>"))
			{
				std::string aniName = settings.GetNextAttribute();
				int start = settings.GetNextIntAttribute();
				int end = settings.GetNextIntAttribute();
				this->AddNewSubAnimation(aniName, start, end);
			}
		}
	}*/
}

void AnimationPathControl::SetMultiplier(float multi)
{

	//loop through callbacks and set the muliplies
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{
		m_aniCallbacks[i]->setTimeMultiplier(multi);
	}

}

void AnimationPathControl::SetTimeMode(int mode)
{
	//loop through callbacks and set the time mode
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{
		m_aniCallbacks[i]->SetTimeMode(mode);
	}
}

int AnimationPathControl::GetTimeMode()
{
	if(m_aniCallbacks.size()==0)
	{return 0;}
	return m_aniCallbacks[0]->GetTimeMode();
}

//
//Set the current manual frame time if using manual time mode
//
void AnimationPathControl::SetManualFrame(double frame)
{
	//loop through callbacks and set the manualFrame
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{
		m_aniCallbacks[i]->SetManualFrame(frame);
	}
}

double AnimationPathControl::GetManualFrame()
{
	if(m_aniCallbacks.size()==0)
	{return 0.0f;}
	return	m_aniCallbacks[0]->GetManualFrame();
}

float AnimationPathControl::GetMultiplier()
{
	if(m_aniCallbacks.size()==0)
	{return 0.0f;}
	//just return the first one as they should be the same
	return	m_aniCallbacks[0]->getTimeMultiplier();
}

//
// Change all the callbacks to animation ID,
//
void AnimationPathControl::SwapAnimation(int id)
{
	if((int)m_splitAnis.size()>0)
	{
		if( (id<0) || (id>=(int)m_splitAnis[0]->GetTotalSplitAnimations()))
		{
			return;
		}
	}else{return;}

	//loop all the callbacks and swap their paths for one of the
	// split animations
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{
		m_aniCallbacks[i]->setAnimationPath(m_splitAnis[i]->GetSplitAni(id));
	}

	//reset current to ensure it starts at the begining
	this->ResetAnimation();
}

void AnimationPathControl::SwapAnimation(std::string name)
{
	int index = this->FindAnimationNameIndex(name);
	if(index!=-1)
	{SwapAnimation(index);}
}

//
// Move to next animation
//
void AnimationPathControl::NextAnimation()
{
	//check we wont go over the end
	if(m_splitAnis[0]->GetCurrentAnimation()+1 < m_splitAnis[0]->GetTotalSplitAnimations())
	{SwapAnimation(m_splitAnis[0]->GetCurrentAnimation()+1);}
	else{SwapAnimation(0);}
}

//
//Move to the previous sub animation
//
void AnimationPathControl::PreviousAnimation()
{
	//check we wont go over the end
	if(m_splitAnis[0]->GetCurrentAnimation()-1 >= 0)
	{SwapAnimation(m_splitAnis[0]->GetCurrentAnimation()-1);}
	else{SwapAnimation(m_splitAnis[0]->GetTotalSplitAnimations()-1);}
}

//
//Change the loop mode of all paths in all split animatins, aswell as the original
//
void AnimationPathControl::SetAnimationType(osg::AnimationPath::LoopMode mode)
{
	for(unsigned int i=0; i<m_splitAnis.size(); i++)
	{	m_splitAnis[i]->SetAnimationType(mode); }
}


void AnimationPathControl::SetPause(bool pause)
{
	m_paused = pause;
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{ 
		m_aniCallbacks[i]->setPause(m_paused);
	}
}

void AnimationPathControl::TogglePause()
{
	m_paused = m_paused ? false : true;
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{ 
		m_aniCallbacks[i]->setPause(m_paused);
	}
}


//Animation Queue


//
//return the number of animations in the queue
//
int AnimationPathControl::GetAniQueueLength()
{
	if(m_aniCallbacks.size() == 0)
	{return 0;}
	return m_aniCallbacks[0]->GetAniQueueLength();
}

//
//clear the animation queue
//
void AnimationPathControl::ClearAnimationQueue()
{
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{
		m_aniCallbacks[i]->ClearAnimationQueue();
	}
}

//
// find the index of a sub animation by its friendly name
//
int AnimationPathControl::FindAnimationNameIndex(std::string name)
{
	for(unsigned int i=0; i<m_animationNames.size(); i++)
	{
		if(m_animationNames[i].compare(name) == 0)
		{return i;}
	}
	return -1;
}

//
// makes the animation change to this id once the current one finishes
//
void AnimationPathControl::AddSubAnimationToQueue(int id)
{
	//TTT 0006 see if we need to use stop loop mode for any non subID change tos
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{ 
		AnimationPathEventCallback* call = dynamic_cast<AnimationPathEventCallback*> (m_aniCallbacks[i].get());
		call->SetChangeToEvent(m_splitAnis[i]->GetSplitAni(id), m_aniCallbacks[i]->getAnimationPath()->getLoopMode(), 1.0f);  
	}
}

//
// Add an animation to the queue that wont start until a delay time is reached
//
void AnimationPathControl::AddDelayedAnimationToQueue(int id, float delay)
{
	if((int)m_splitAnis.size()>0)
	{
		if( (id<0) || (id>=(int)m_splitAnis[0]->GetTotalSplitAnimations()))
		{
			return;
		}
	}else{return;}

	//loop all the callbacks and swap their paths for one of the
	// split animations
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{
		AnimationPathEventCallback* call = dynamic_cast<AnimationPathEventCallback*> (m_aniCallbacks[i].get());
		call->SetDelayedAnimation(m_splitAnis[i]->GetSplitAni(id) ,m_aniCallbacks[i]->getAnimationPath()->getLoopMode(), 1.0f, delay); 
	}
}

//by name
void AnimationPathControl::AddSubAnimationToQueue(std::string name)
{
	int index = this->FindAnimationNameIndex(name);
	if(index!=-1)
	{AddSubAnimationToQueue(index);}
}

void AnimationPathControl::AddChangeToManualToQueue(std::string name)
{
	int index = this->FindAnimationNameIndex(name);
	if(index!=-1)
	{AddChangeToManualToQueue(index);}
}

void AnimationPathControl::AddDelayedAnimationToQueue(std::string name, float delay)
{
	int index = this->FindAnimationNameIndex(name);
	if(index!=-1)
	{AddDelayedAnimationToQueue(index, delay);}
}

//
// chasnges to this animation, but also makes animation mode manual
//
void AnimationPathControl::AddChangeToManualToQueue(int id)
{
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{ 
		AnimationPathEventCallback* call = dynamic_cast<AnimationPathEventCallback*> (m_aniCallbacks[i].get());
		call->SetChangeToManualEvent(m_splitAnis[i]->GetSplitAni(id), m_aniCallbacks[i]->getAnimationPath()->getLoopMode(), 1.0f );  
	}
}

//
//Reset the current animation to its start
//
void AnimationPathControl::ResetAnimation()
{
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{ 
		m_aniCallbacks[i]->reset();
	}
}

bool AnimationPathControl::isPlaying()
{
	for(unsigned int i=0; i<m_aniCallbacks.size(); i++)
	{ 
		if(m_aniCallbacks[i]->isAniPlaying())
		{return true;}
	}
	return false;
}