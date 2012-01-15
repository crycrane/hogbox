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

//#include "AnimationPathControl.h"

#include <osg/Timer>
#include <osg/AnimationPath>
#include <osg/NodeVisitor>

#include <iostream>
#include <vector>

namespace hogbox
{

//forward declare the osgAnicontrol helper system
//class AnimationPathControl;

//
// CEventAnimationCallBack
//
// This is our own version of the AnimationPathCallback called on each frame update
// of an animation path
// It allows for sub animations to be queued, once an animation reaches its end the next in the queue is played
// Animation events in the queue can be
// CHANGETO = change to the new animation path and play
// CHANGETOMANUAL = change to the new animation path but then set time mode to manual so we can select frames ourselves
// CHANGETO_DELAY = change to an animation but wait for a time delay until we start it
//
class AnimationPathEventCallback : public osg::AnimationPathCallback
{
public:
	AnimationPathEventCallback(void) : AnimationPathCallback(), 
									isPlaying(true)
	{
		m_timeMode = 0;
		m_aniEventQueue.clear();
		isPlaying = true;
	}
	//~AnimationPathEventCallback(void){}

	bool isPlaying;

	//Set the callback to change to the passed path and loop mode once we reach the end of the current path
	//if a COsgAnimationControl control pointer and switch are passed, the animation will be applied to that controller
	//and the switch will be switched to switchIndex. It is assumed this switch would hold both the nodes of the current controller
	// and the change to controller.
	void SetChangeToEvent(osg::AnimationPath* path, osg::AnimationPath::LoopMode changeToMode, double multi, int id = 0);


	void SetChangeToManualEvent(osg::AnimationPath* path, osg::AnimationPath::LoopMode changeToMode, double multi);

	void SetDelayedAnimation(osg::AnimationPath* path, osg::AnimationPath::LoopMode changeToMode, double multi, float delay);

	void SetTimeMode(int mode);


	int GetTimeMode();
	void SetManualFrame(double time);
	float GetManualFrame();

	double GetAniLength();

	//are we currently playing
	bool isAniPlaying(){return isPlaying;}

	//get the length of the animation queue
	int GetAniQueueLength(){
		return m_aniEventQueue.size();
	}

	//clear all animations from the queue
	void ClearAnimationQueue(){
		m_aniEventQueue.clear();
	}

	//our update operator
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

protected:

	enum AniEventType
	{
		NONE = 0,
		CHANGETO,
		CHANGETOMANUAL,
		CHANGETO_DELAY
	};

	struct AniEvent
	{
		AniEventType eventType;
		osg::AnimationPath* pChangeToAniPath;
		osg::AnimationPath::LoopMode changeToMode; 
		double changeToMulti;

		float delayWait; //time to wait before switching

		int changeID;

		osg::Timer_t startTick;
	};

	//queue of animtion events to fire off
	std::vector<AniEvent> m_aniEventQueue;

	//manual animation
	int m_timeMode; //0 = realtime, 1 = frame request
	float m_manualFrame;

};

};