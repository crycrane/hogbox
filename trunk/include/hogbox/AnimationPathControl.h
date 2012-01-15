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

#include <hogbox/Export.h>
#include <hogbox/HogBoxBase.h> 
#include <hogbox/AnimationPathEventCallback.h>

#include <osg/Timer>
#include <osg/AnimationPath>


#include <iostream>
#include <sstream>
#include <vector>


class AnimationPathEventCallback;

namespace hogbox 
{

//
// SplitAni
//
//Represents a single original animation path, and all the sub animations derived from it
//
class SplitAni : public osg::Object
{
public:
	SplitAni(void) 
		: osg::Object(),
		m_currentAnimation(-1)
	{
	}
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	SplitAni(const SplitAni& split,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(split, copyop)
	{
	}
	META_Box(hogbox,SplitAni);

	//?
	void AddSplitAni(osg::ref_ptr<osg::AnimationPath> ani){m_splitAnis.push_back(ani);}
	void AddSplitAni(){m_splitAnis.push_back(new osg::AnimationPath());}  
	
	int GetNumSplitAnis(){return m_splitAnis.size();} 
	osg::AnimationPath* GetSplitAni(unsigned int index)
	{	
		//check index is inbounds
		if( (index < 0) || (index >= m_splitAnis.size()) )
		{	return m_originalAni.get();}

		m_currentAnimation=index;
		return m_splitAnis[index].get();
	}

	//set all the paths animation modes i.e. lopp, single swing
	void SetAnimationType(osg::AnimationPath::LoopMode mode)
	{	
		int size=m_splitAnis.size();
		for(int i=0; i<size; i++)
		{m_splitAnis[i]->setLoopMode( mode );}
	}

	void SetOriginal(osg::AnimationPath* ani, int fps){m_originalAni=ani;m_fps=fps;}
	osg::AnimationPath* GetOriginal(){return m_originalAni.get();}

	const unsigned int GetTotalSplitAnimations(){return m_splitAnis.size();}
	const unsigned int GetCurrentAnimation(){return m_currentAnimation;}

	//re set the frames of the sub path subID using frames from-to of originalPath
	void ReSampleSubAni(int subID, int from, int to);

protected:

	virtual ~SplitAni(void)
	{
		//original loaded animation
		m_originalAni = NULL;
		m_fps=30;

		//original animation split into sub anis
		for(unsigned int i=0; i<m_splitAnis.size(); i++)
		{
			m_splitAnis[i] = NULL;
		}
		m_splitAnis.clear();
	}
protected:

	//original loaded animation
	osg::ref_ptr<osg::AnimationPath> m_originalAni;
	int m_fps;

	//original animation split into sub anis
	std::vector< osg::ref_ptr<osg::AnimationPath> > m_splitAnis; 

	unsigned int m_currentAnimation; //-1 == original

};

typedef osg::ref_ptr<SplitAni> SplitAniPtr;



//
//AnimationPathControl
//
//Takes an osg node finds all its animations
//then allows the simple spliting of the orignal animation into smaller sub animation
//Call init to pass a node to have it animations controled
//Call SplitAnimations to create the sub animations with the frames you require from a simple vector list
//Call AddNewSubAnimation to create a new sub animation containing the desired frames and related to a name
//Call SplitFromCfgFile to create our named sub animations from a config file
//
//Once you have called init and created your desired sub animations 
//Call AddSubAnimationToQueue to add a sub animation by name or index to our animation queue
//Each animation in the queue is played one after the other until the queue is empty
//
class HOGBOX_EXPORT AnimationPathControl : public osg::Object
{
public:
	AnimationPathControl(void);

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	AnimationPathControl(const AnimationPathControl& control,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(control, copyop)
	{
	}
	META_Box(hogbox,AnimationPathControl);

	//
	//pass the  model to have it's aniation controlled
	//this will find all animations below aniNode and create a CSplitAni
	//per path found, the CSplitAni list can then be used to create our sub animations
	bool Init(osg::Node* aniNode, int fps = 30, std::string config = "");

protected:

	virtual ~AnimationPathControl(void);

	//the find and add animation funcs are called by init

	//add an animation to the controller
	void AddAnimation(osg::Node* node, osg::AnimationPathCallback* ani);
	//finds all the nodes with animation below node and adds them to this controller
	int FindAnimationNodes(osg::Node* node);

public:

	//Add a new sub animtion to our list of animations (by default the sub animation 
	//is the same as the original
	void AddNewSubAnimation(std::string name);
	//add a new sub animation but sample frames from-to from the original animation
	void AddNewSubAnimation(std::string name, int from, int to);

	//get the total frames in the original animation
	int GetMaxFrames();
	//get the length in seconds of the original animation
	double GetAnimationLength();

	//return the number of node animations being controled
	int GetTotalAnimatedNodes(){return m_splitAnis.size();}
	//return the number of sub animations being used
	int GetNumSubAnimations(){return m_splitAnis[0]->GetNumSplitAnis();} 

	//resample the exisit subAnimation sub id to consist
	//of from-to frames of the original animation
	void ReSampleSubAni(int subID, int from, int to);


	//Helpers to split animation into sub 

	//takes the animations found during init
	//and splits into equal sub animations fpa frames long
	void SplitAnimations(int fpa);

	//takes the animations found during init
	//and splits into fpa.size() animations the length of the vectors value
	void SplitAnimations(std::vector<int> fpa);

	//creates the sub animations from frame ranges set in a cfg file
	void SplitFromCfgFile(std::string file);
	

	//set if we run as realtime update or manual frame selection
	void SetTimeMode(int mode);
	int GetTimeMode();
	//the frame to use if using manual time mode
	void SetManualFrame(double frame);
	double GetManualFrame();

	//the rate the animation plays at
	void SetMultiplier(float multi);
	float GetMultiplier();

	//
	//force swap to sub animation id, also resetin to the start
	//
	void SwapAnimation(int id);
	void SwapAnimation(std::string name);
	void NextAnimation();
	void PreviousAnimation();

	//set play type, e.g. LOOP, 
	void SetAnimationType(osg::AnimationPath::LoopMode mode);

	//full pause of all animation
	void TogglePause();
	void SetPause(bool pause);

	//reset the current animation to the start
	void ResetAnimation();
	

	//Animation queue
	//makes the animation change tot this id once the current one finishes
	void AddSubAnimationToQueue(int id);
	void AddChangeToManualToQueue(int id);
	void AddDelayedAnimationToQueue(int id, float delay);

	//by name
	void AddSubAnimationToQueue(std::string name);
	void AddChangeToManualToQueue(std::string name);
	void AddDelayedAnimationToQueue(std::string name, float delay);

	//return the number of animations in the queue
	int GetAniQueueLength();

	//clear the animation queue
	void ClearAnimationQueue();

	//find an animtion index from its friendly name
	int FindAnimationNameIndex(std::string name);

	//return true if any of our callbacks are currently playing
	bool isPlaying();

protected:

	//the node that has had the controller applied
	osg::Node* m_animatedNode;

	//list of the all the AnimationPathEventCallbacks we have attached to the nodes in m_animatedNode
	std::vector< osg::ref_ptr<AnimationPathEventCallback> > m_aniCallbacks; 

	//the list of friendly names that relate to the
	std::vector<std::string> m_animationNames;

	//the split paths for each node containing the orignal
	//paths and the new sub sampled versions
	std::vector<SplitAniPtr> m_splitAnis;

	//the frames per second of the original animations of m_animatedNode
	int m_fps;

	//is the animation paused
	bool m_paused;

};

typedef osg::ref_ptr<AnimationPathControl> AnimationPathControlPtr;

};