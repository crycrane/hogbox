#include <hogbox/AnimationPathEventCallBack.h>

using namespace hogbox;

void AnimationPathEventCallback::SetChangeToEvent(osg::AnimationPath* path, osg::AnimationPath::LoopMode changeToMode, double multi, int id)
{
	AniEvent aniEvent;
	aniEvent.eventType = CHANGETO;
	aniEvent.pChangeToAniPath = path;
	aniEvent.changeToMode=changeToMode;
	aniEvent.changeToMulti=multi;

	aniEvent.delayWait = 0.0f; //time to wait before switching

	aniEvent.changeID = id;

	aniEvent.startTick = osg::Timer::instance()->tick();

	isPlaying = true;

	m_aniEventQueue.push_back(aniEvent);
}

void AnimationPathEventCallback::SetChangeToManualEvent(osg::AnimationPath* path, osg::AnimationPath::LoopMode changeToMode, double multi)
{
	AniEvent aniEvent;
	aniEvent.eventType = CHANGETOMANUAL;
	aniEvent.pChangeToAniPath = path;
	aniEvent.changeToMode=changeToMode;
	aniEvent.changeToMulti=multi;

	m_aniEventQueue.push_back(aniEvent);
}

void AnimationPathEventCallback::SetDelayedAnimation(osg::AnimationPath* path, osg::AnimationPath::LoopMode changeToMode, 
												 double multi, float delay)
{
	AniEvent aniEvent;
	aniEvent.eventType = CHANGETO_DELAY;
	aniEvent.pChangeToAniPath = path;
	aniEvent.changeToMode=changeToMode;
	aniEvent.changeToMulti=multi;
	aniEvent.delayWait = delay;

	aniEvent.startTick = osg::Timer::instance()->tick();

	m_aniEventQueue.push_back(aniEvent);
}

void AnimationPathEventCallback::SetTimeMode(int mode)
{
	m_timeMode = mode;
	if(m_timeMode == 1)
	{
		SetManualFrame(_latestTime);
		_firstTime=0.0f;
	}
}
int AnimationPathEventCallback::GetTimeMode(){return m_timeMode;}

void AnimationPathEventCallback::SetManualFrame(double time){m_manualFrame=time;}

float AnimationPathEventCallback::GetManualFrame(){return m_manualFrame;}

double AnimationPathEventCallback::GetAniLength(){return _animationPath->getLastTime();}

void AnimationPathEventCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	if (_animationPath.valid() && 
		nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR && 
		nv->getFrameStamp())
	{
		//get the current event from the queue
		AniEvent* pEvent = NULL;
		if(m_aniEventQueue.size() > 0)
		{ pEvent = &m_aniEventQueue[0];}

		double time = nv->getFrameStamp()->getReferenceTime();

		if(m_timeMode==0)
		{
			_latestTime = time;
		}else{
			_latestTime = m_manualFrame;
			if(m_timeMode==1)
			{ _firstTime = 0.0f;}//_latestTime;}
		}

		if(pEvent)
		{

			//check for a delayed start
			if(pEvent->eventType == CHANGETO_DELAY)
			{
				if(pEvent->delayWait > 0.0f)
				{
					pEvent->delayWait -= osg::Timer::instance()->delta_s(pEvent->startTick,osg::Timer::instance()->tick());
					pEvent->startTick = osg::Timer::instance()->tick();
					if(pEvent->delayWait <= 0.0f) //delay ended
					{
						this->setAnimationPath(pEvent->pChangeToAniPath);
						this->reset();
						this->setTimeMultiplier(pEvent->changeToMulti); 
						_animationPath->setLoopMode(pEvent->changeToMode);
						//m_event=NONE;
						m_aniEventQueue.erase(m_aniEventQueue.begin());
					}
				}
			}
	
			//if we reach the end of the current path check for changeto or remove event from queue
			if( (getAnimationTime()>=_animationPath->getLastTime()) && (isPlaying))
			{
				isPlaying=false;

				switch(pEvent->eventType)
				{
					case CHANGETO:
					{

						this->setAnimationPath(pEvent->pChangeToAniPath);
						this->reset();
						this->setTimeMultiplier(pEvent->changeToMulti); 
						_animationPath->setLoopMode(pEvent->changeToMode); 
					
						//remove the event from the queue
						m_aniEventQueue.erase(m_aniEventQueue.begin());
						//m_event=NONE;

						break;
					}
					case CHANGETOMANUAL:
					{
						this->setAnimationPath(pEvent->pChangeToAniPath);
						this->reset();
						this->setTimeMultiplier(pEvent->changeToMulti); 
						_animationPath->setLoopMode(pEvent->changeToMode); 
						this->SetTimeMode(1); 
						
						//remove the event from the queue
						//m_event=NONE;
						m_aniEventQueue.erase(m_aniEventQueue.begin());

						break;
					}
					default :break;
				}
			}else if(getAnimationTime()<_animationPath->getLastTime())
			{
				isPlaying=true;
			}

		}else if(getAnimationTime()<_animationPath->getLastTime())
		{
				isPlaying=true;
		}

		if (!_pause)
		{
			// Only update _firstTime the first time, when its value is still DBL_MAX
			if (_firstTime==DBL_MAX) _firstTime = time;
			update(*node);
		}
	}
	
	// must call any nested node callbacks and continue subgraph traversal.
	osg::NodeCallback::traverse(node,nv);
}

