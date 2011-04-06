#pragma once

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/TimelineAnimationManager>

#include <osgAnimation/ActionStripAnimation>
#include <osgAnimation/ActionBlendIn>
#include <osgAnimation/ActionBlendOut>
#include <osgAnimation/ActionAnimation>

namespace hogbox
{

	//
	//HogBoxAnimationManager
	//Adds a few extra features to BasicAnimation Manager
	class HogBoxAnimationManager : public osgAnimation::TimelineAnimationManager
	{
	public:
		HogBoxAnimationManager() 
			: osgAnimation::TimelineAnimationManager()
		{
		}
		HogBoxAnimationManager(const osgAnimation::AnimationManagerBase& b, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY)
			: osgAnimation::TimelineAnimationManager(b,copyop)
		{
		}

		META_Object(hogbox, HogBoxAnimationManager);
  
		void playAnimationNow(osgAnimation::Animation* animation, float blendIn, float blendOut, int priority = 0){
			animation->setDuration(1.0); // set this animation duration to 10 seconds
			osgAnimation::ActionStripAnimation* animationStrip = new osgAnimation::ActionStripAnimation(animation,blendIn,blendOut);
			animationStrip->setLoop(1); // one time

            osgAnimation::Timeline* tml = this->getTimeline();
			// dont play if already playing
			//if (!tml->isActive(_scratchNose.get()))
			//{
			// add this animation on top of two other
			// we add one to evaluate the animation at the next frame, else we
			// will miss the current frame
			tml->addActionAt(tml->getCurrentFrame() + 1, animationStrip, priority);
		}

		void playAnimationByNameNow(const std::string& name, float blendIn=0.0, float blendOut=0.0, int priority = 0){
			for( osgAnimation::AnimationList::const_iterator iterAnim = _animations.begin(); 
				iterAnim != _animations.end(); 
				++iterAnim ) 
			{
				if ( (*iterAnim)->getName() == name ){
					
					playAnimationNow((*iterAnim), blendIn, blendOut, priority);
					//playAnimation((*iterAnim).get(), priority, weight);
				}
			}
		}

		osgAnimation::Animation* GetAnimationByName(const std::string& name){
			for( osgAnimation::AnimationList::const_iterator iterAnim = _animations.begin(); 
				iterAnim != _animations.end(); 
				++iterAnim ) 
			{
				if ( (*iterAnim)->getName() == name ){return (*iterAnim);}
			}
			return NULL;
		}

	protected:
		virtual ~HogBoxAnimationManager(){
		}
	protected:

	};

	//
	//finds and returns the first AnimationManagerBase in the sub graph
	//converts it to a BasicAnimationManager and returns
	struct FindAnimationManagerBaseVisitor : public osg::NodeVisitor
	{
		osg::ref_ptr<osgAnimation::AnimationManagerBase> _foundManagerBase;
		osg::ref_ptr<osg::Node> _foundInNode;

		FindAnimationManagerBaseVisitor() 
			: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) 
		{
		}

		//base version just stores them 
		virtual void onFoundManagerBase(osgAnimation::AnimationManagerBase* manager, osg::Node* sourceNode)
		{
			_foundManagerBase = manager;
			_foundInNode = sourceNode;
		}

		void apply(osg::Node& node) {
			if (_foundManagerBase.valid())
				return;
			if (node.getUpdateCallback()) {
				osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
				if (b) {
					onFoundManagerBase(b, &node);
					//_am = new osgAnimation::BasicAnimationManager(*b);
					//node.setUpdateCallback(_am.get());
					return;
				}
			}
			traverse(node);
		}
	};

	//
	//Inherits from AnimationManagerBaseFinder, only this
	//version converts any found animationManagerBase to a 
	//BasicAnimationManager, applys to the sourceNode and returns
	struct AnimationManagerBaseToBasicAnimationManagerVisitor : public FindAnimationManagerBaseVisitor
	{
		//created to replace an existing
		osg::ref_ptr<osgAnimation::BasicAnimationManager> _basicManager;
		
		//base version just stores them 
		virtual void onFoundManagerBase(osgAnimation::AnimationManagerBase* manager, osg::Node* sourceNode)
		{
			if(manager){
				_basicManager = new osgAnimation::BasicAnimationManager(*manager);
				if(sourceNode){sourceNode->setUpdateCallback(_basicManager.get());}
			}
			FindAnimationManagerBaseVisitor::onFoundManagerBase(_basicManager.get(), sourceNode);
		}
	};

	//
	//Inherits from AnimationManagerBaseFinder, only this
	//version converts any found animationManagerBase to a 
	//BasicAnimationManager, applys to the sourceNode and returns
	struct AnimationManagerBaseToTimelineAnimationManagerVisitor : public FindAnimationManagerBaseVisitor
	{
		//created to replace an existing
		osg::ref_ptr<osgAnimation::TimelineAnimationManager> _timelineManager;
		
		//base version just stores them 
		virtual void onFoundManagerBase(osgAnimation::AnimationManagerBase* manager, osg::Node* sourceNode)
		{
			if(manager){
				_timelineManager = new osgAnimation::TimelineAnimationManager(*manager);
				if(sourceNode){sourceNode->setUpdateCallback(_timelineManager.get());}
			}
			FindAnimationManagerBaseVisitor::onFoundManagerBase(_timelineManager.get(), sourceNode);
		}
	};

	//
	//Inherits from AnimationManagerBaseFinder, only this
	//version converts any found animationManagerBase to a 
	//HogBoxAnimationManager, applys to the sourceNode and returns
	struct AnimationManagerBaseToHogBoxAnimationManagerVisitor : public FindAnimationManagerBaseVisitor
	{
		//created to replace an existing
		osg::ref_ptr<hogbox::HogBoxAnimationManager> _hogboxManager;
		
		//base version just stores them 
		virtual void onFoundManagerBase(osgAnimation::AnimationManagerBase* manager, osg::Node* sourceNode)
		{
			if(manager){
                //check it's not already a hogbox animation manager
                hogbox::HogBoxAnimationManager* asHogBoxManger = dynamic_cast<hogbox::HogBoxAnimationManager*>(manager);
                if(asHogBoxManger){
                    _hogboxManager=asHogBoxManger;
                }else{
                    _hogboxManager = new hogbox::HogBoxAnimationManager(*manager);
                    if(sourceNode){sourceNode->setUpdateCallback(_hogboxManager.get());}         
                }
			}
			FindAnimationManagerBaseVisitor::onFoundManagerBase(_hogboxManager.get(), sourceNode);
		}
	};

	//returns the index of the keyframe closest to the passed time
	//returns -1 if the time is out of range of the key container
	template <typename ContainerType>
	static int GetNearestKeyFrameIndex(ContainerType* keyContainer, double time)
	{
		if(!keyContainer){return -1;}

		int closestFrame = -1;
		double closestDiff = 99999.99f;

		//loop all the keys
		for(unsigned int i=0; i<keyContainer->size(); i++)
		{
			double diff = fabs(time - (*keyContainer)[i].getTime());
			if( diff < closestDiff){
				closestFrame = i;
				closestDiff = diff;
			}
		}
		return closestFrame;
	}

	//// Helper method for resampling channels
	template <typename ChannelType, typename ContainerType>
	static osg::ref_ptr<ChannelType> ResampleChannel(ChannelType* sourceChannel, unsigned int startFrame, unsigned int endFrame, int fps)
	{
		osg::ref_ptr<ChannelType> newChannel = NULL;
		if(!sourceChannel){
			return newChannel;
		}

		//get the key frame container from the source channel
		ContainerType* sourceKeyCont  = sourceChannel->getSamplerTyped()->getKeyframeContainerTyped();

		if (sourceKeyCont)
		{
			OSG_INFO << "OsgAnimationTools ResampleChannel INFO: Resampling source channel '" << sourceChannel->getName() 
					 << "', from startFrame '" << startFrame << "' to endFrame '" << endFrame << ". Total frames in source channel '" << sourceKeyCont->size() << "'." << std::endl;

			//determine the copy direction, i.e. lets see if we can copy frames in reverse as could come in handy
			unsigned int from=startFrame;
			unsigned int to=endFrame;
			if(startFrame > endFrame){
				from = endFrame;
				to = startFrame;
			}

			//get our frames as a times 
			double fromTime = from == 0 ? 0.0f : (float)from/(float)fps;
			double toTime = to == 0 ? 0.0f : (float)to/(float)fps;

			//needs to be if time is too big
			float firstTime = (*sourceKeyCont)[0].getTime();
			float lastTime = (*sourceKeyCont)[sourceKeyCont->size()-1].getTime();
			if(fromTime < firstTime || toTime > lastTime)
			{
				OSG_WARN << "OsgAnimationTools ResampleChannel ERROR: startTime or endTime is out of range," << std::endl
						 << "                                         startTime '" << fromTime << "', endFrame '" << toTime << ". Max time in channel '" << lastTime << "'." << std::endl;

				return newChannel;
			}

			//find the true key frame numbers as some animation channels contain more then the actual fps
			from = GetNearestKeyFrameIndex<ContainerType>(sourceKeyCont, fromTime);
			if(from == -1){return newChannel;}
			to = GetNearestKeyFrameIndex<ContainerType>(sourceKeyCont, toTime);
			if(to == -1){return newChannel;}

			//get the offset time form the keys
			double offsetToZero = (*sourceKeyCont)[from].getTime();

			//create our new channel with same type, name and target name
			newChannel = new ChannelType();
			newChannel->setName(sourceChannel->getName());
			newChannel->setTargetName(sourceChannel->getTargetName());

			//get the new channels key containter, also creating the sampler and container if required
			ContainerType* destKeyCont = newChannel->getOrCreateSampler()->getOrCreateKeyframeContainer();

			//now copy all the frames between fromTime and toTime
			for (unsigned int k=from; k<=to; k++) 
			{
				//push source frame onto destination container
				destKeyCont->push_back((*sourceKeyCont)[k]);
				//adjust the new frames time so animation starts from 0.0
				double sourceTime = (*sourceKeyCont)[k].getTime();
				(*destKeyCont)[destKeyCont->size()-1].setTime(sourceTime-offsetToZero);
			}

			return newChannel;

		}else{
			OSG_WARN << "OsgAnimationTools ResampleChannel ERROR: source channel contains no key frame container," << std::endl;
			return newChannel;
		}
		return newChannel;
	}

	//
	//Create and return a new animation based on the start end frames of the source animation
	//creating all the relevant channels etc
	static osg::ref_ptr<osgAnimation::Animation> ResampleAnimation(osgAnimation::Animation* source, 
																	int startFrame, int endFrame, int fps, 
																	std::string newName)
	{
		osg::ref_ptr<osgAnimation::Animation> newAnimation = NULL;
		if(!source){
			return newAnimation;
		}

		newAnimation = new osgAnimation::Animation();
		newAnimation->setName(newName);

		//loop all the source channels
		osgAnimation::ChannelList sourceChannels = source->getChannels();
		for(unsigned int i=0; i<sourceChannels.size(); i++)
		{
			//clone the channel type, name and target
			//this is a dumb method but I cant find a way to just clone the channel type,
			//so instead we copy the whole channel, keys and all, then delete the ones we don't want :(
			osgAnimation::Channel* pChannel = sourceChannels[i].get();
			osg::ref_ptr<osgAnimation::Channel> resampledChannel = NULL;

			        //osgAnimation::Channel* pChannel = anim.getChannels()[i].get();

			osgAnimation::DoubleLinearChannel* pDlc = dynamic_cast<osgAnimation::DoubleLinearChannel*>(pChannel);
			if (pDlc)
			{
				resampledChannel = ResampleChannel<osgAnimation::DoubleLinearChannel, osgAnimation::DoubleKeyframeContainer>(pDlc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::FloatLinearChannel* pFlc = dynamic_cast<osgAnimation::FloatLinearChannel*>(pChannel);
			if (pFlc)
			{
				resampledChannel = ResampleChannel<osgAnimation::FloatLinearChannel, osgAnimation::FloatKeyframeContainer>(pFlc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::Vec2LinearChannel* pV2lc = dynamic_cast<osgAnimation::Vec2LinearChannel*>(pChannel);
			if (pV2lc)
			{
				resampledChannel = ResampleChannel<osgAnimation::Vec2LinearChannel, osgAnimation::Vec2KeyframeContainer>(pV2lc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::Vec3LinearChannel* pV3lc = dynamic_cast<osgAnimation::Vec3LinearChannel*>(pChannel);
			if (pV3lc)
			{
				resampledChannel = ResampleChannel<osgAnimation::Vec3LinearChannel, osgAnimation::Vec3KeyframeContainer>(pV3lc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::Vec4LinearChannel* pV4lc = dynamic_cast<osgAnimation::Vec4LinearChannel*>(pChannel);
			if (pV4lc)
			{
				resampledChannel = ResampleChannel<osgAnimation::Vec4LinearChannel, osgAnimation::Vec4KeyframeContainer>(pV4lc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::QuatSphericalLinearChannel* pQslc = dynamic_cast<osgAnimation::QuatSphericalLinearChannel*>(pChannel);
			if (pQslc)
			{
				resampledChannel = ResampleChannel<osgAnimation::QuatSphericalLinearChannel, osgAnimation::QuatKeyframeContainer>(pQslc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::FloatCubicBezierChannel* pFcbc = dynamic_cast<osgAnimation::FloatCubicBezierChannel*>(pChannel);
			if (pFcbc)
			{
				resampledChannel = ResampleChannel<osgAnimation::FloatCubicBezierChannel, osgAnimation::FloatCubicBezierKeyframeContainer>(pFcbc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::DoubleCubicBezierChannel* pDcbc = dynamic_cast<osgAnimation::DoubleCubicBezierChannel*>(pChannel);
			if (pDcbc)
			{
				resampledChannel = ResampleChannel<osgAnimation::DoubleCubicBezierChannel, osgAnimation::DoubleCubicBezierKeyframeContainer>(pDcbc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::Vec2CubicBezierChannel* pV2cbc = dynamic_cast<osgAnimation::Vec2CubicBezierChannel*>(pChannel);
			if (pV2cbc)
			{
				resampledChannel = ResampleChannel<osgAnimation::Vec2CubicBezierChannel, osgAnimation::Vec2CubicBezierKeyframeContainer>(pV2cbc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::Vec3CubicBezierChannel* pV3cbc = dynamic_cast<osgAnimation::Vec3CubicBezierChannel*>(pChannel);
			if (pV3cbc)
			{
				resampledChannel = ResampleChannel<osgAnimation::Vec3CubicBezierChannel, osgAnimation::Vec3CubicBezierKeyframeContainer>(pV3cbc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
			osgAnimation::Vec4CubicBezierChannel* pV4cbc = dynamic_cast<osgAnimation::Vec4CubicBezierChannel*>(pChannel);
			if (pV4cbc)
			{
				resampledChannel = ResampleChannel<osgAnimation::Vec4CubicBezierChannel, osgAnimation::Vec4CubicBezierKeyframeContainer>(pV4cbc, startFrame, endFrame, fps);
				if(resampledChannel){newAnimation->addChannel(resampledChannel.get());}
				continue;
			}
		}//loop channel

		return newAnimation;
	}

};//end hogbox namespace