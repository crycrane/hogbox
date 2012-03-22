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

#include <osg/MatrixTransform>
#include <osgAnimation/EaseMotion>
#include <hogbox/Callback.h>
#include <deque>

namespace hogbox 
{
    
    //
    //Animates a value based on a queue of keys, each key
    //has a start value, end value, duration in secs and 
    //an ease motion controler
    //
    template <class T>
    class AnimateValue : public osg::Object
    {
    public:
        
        AnimateValue()
            : osg::Object(),
            _keyFrameQueue(new KeyFrameQueue())
        {
        }
        AnimateValue(const T& value)
            : osg::Object(),
            _value(value),
            _start(value),
            _keyFrameQueue(new KeyFrameQueue())
        {
        }
        
        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        AnimateValue(const AnimateValue& value,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
            : osg::Object(value, copyop),
            _value(value._value),
            _start(value._start)
        {
        }
        
        META_Object(hogbox,AnimateValue);
        
        class KeyFrame{
        public:
            T end;
            float duration;
            osg::ref_ptr<osgAnimation::Motion> motion;
            hogbox::CallbackEventPtr event;
            
            KeyFrame(){
                event = new hogbox::CallbackEvent("KeyFrameEndEvent");
            }
            KeyFrame(const KeyFrame& key)
            : end(key.end),
            duration(key.duration),
            motion(key.motion.get())
            {
                event = new hogbox::CallbackEvent("KeyFrameEndEvent");
            }
        };
        
        //
        //KeyFrameQueue 
        //Wraps a dequeue of KeyFrames and handles selecting current frame as animation
        //updates. Callbacks can also be registered to receive notice when certain event occur
        //in this animation queue e.g. when the end of the queue is reached.
        //
        class KeyFrameQueue : public osg::Object 
        {
        public:
            KeyFrameQueue()
			: osg::Object(),
			_isPlaying(true),
			_currentKeyIndex(0)
            {
                //OSG_FATAL << "Contruct KeyFrameQueue" << std::endl;
            }
            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            KeyFrameQueue(const KeyFrameQueue& queue,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
			: osg::Object(queue, copyop),
			_isPlaying(queue._isPlaying),
			_keyFrameQueue(queue._keyFrameQueue),
			_currentKeyIndex(queue._currentKeyIndex)
            {
            }
            
            META_Object(hogbox,KeyFrameQueue);
            
            //adds a key to the queue, once the the key is reached
            //we smooth from current value to pos over duration seconds,
            //the template M is used to define the osgAnimation motion type
            //returns the new number of keys
            template <typename M>
            bool AddKey(const T& pos, const float& duration, Callback* callback=NULL){
                KeyFrame frame;
                _keyFrameQueue.push_back(frame);
                KeyFrame* framePtr = &_keyFrameQueue[_keyFrameQueue.size()-1];
                framePtr->end = pos;
                framePtr->duration = duration;
                framePtr->motion = new M(0.0f, duration, 1.0f, osgAnimation::Motion::CLAMP);
                if(callback){
                    //OSG_FATAL << "KeyFrameQueue: AddKey with Callback" << std::endl;
                    framePtr->event->AddCallbackReceiver(callback);
                }
                
                
                //if this is the first key ensure _start is set to current value
                return this->GetNumKeys()>0;
            }
            
            //return a pointer to a specific key in the queue,
            //if the key does not exist then NULL is returned
            KeyFrame* GetKey(const unsigned int& index){
                if(index<0 || index >= _keyFrameQueue.size()){return NULL;}
                typename std::deque<KeyFrame>::iterator itr = _keyFrameQueue.begin();
                itr += index;
                return &(*itr);
            }
            //return the current front of the queue
            KeyFrame* GetCurrentKey(){
                return GetKey(_currentKeyIndex);
            }
            //return the current number of keys in the queue
            const unsigned int GetNumKeys(){ 
                return _keyFrameQueue.size();
            }
            
            //remove a key from the queue
            bool RemoveKey(const unsigned int& index){
                //check it's in range
                if(index < 0 || index >= _keyFrameQueue.size()){return false;}
                if(_keyFrameQueue.size() == 1){
                    _finalFrame = KeyFrame(*this->GetKey(0));
                }
                _keyFrameQueue.erase(_keyFrameQueue.begin()+index);
                return true;
            }
            
            KeyFrame* GetFinalKey(){
                return &_finalFrame;
            }
            
            //
            //Update the current keys osgAnimation motion time,
            //also handle moving to the next key if the motion reaches it's
            //duration time.
            //Returns 
            //0=if not animating
            //1=if animating
            //2=movekey
            //3=end of queue
            int Update(const float& timePassed)
            {
                if(_isPlaying)
                {
                    //get current key frame
                    KeyFrame* key = this->GetCurrentKey();
                    if(key)
                    {	
                        //update the motions time
                        key->motion->update(timePassed);
                        
                        //check if our motion has completed
                        if(key->motion->getTime() >= key->motion->getDuration())
                        {
                            //move to next key if we have one
                            if(this->MoveToNextKey()){
                                //if movetonext returns true then we moved to another key
                                return 2;
                            }
                            //otherwise we reached the end so return 3
                            return 3;
                        }
                        return 1;
                    }else{
                        return 0;
                    }
                }
                return 0;
            }
            
        protected:
            
            virtual ~KeyFrameQueue(){}
            
            //pops the current key from the front, returns false
            //if there are no more keys in the queue 
            bool MoveToNextKey(){
                
                //ensure we reset the current key
                KeyFrame* key = GetCurrentKey();
                
                //trigger the current keys end callback
                if(key->event.valid()){
                    key->event->Trigger();
                }
                //if(key){key->motion->reset();}
                
                //
                this->RemoveKey(_currentKeyIndex);
                
                //move forward a key (need transport direction here)
                //_currentKeyIndex++;
                unsigned int size = _keyFrameQueue.size();
                //if we reach the end reset _currentKey to 0 and return false
                if(_currentKeyIndex >= size){_currentKeyIndex = 0; return false;}
                return true;
            }
            
        protected:
            
            //play state of the queue
            bool _isPlaying;
            
            std::deque<KeyFrame> _keyFrameQueue;
            unsigned int _currentKeyIndex;
            
            //copy the final key (before it empties)
            KeyFrame _finalFrame;
        };
        typedef osg::ref_ptr<KeyFrameQueue> KeyFrameQueuePtr;
        typedef std::map<std::string, KeyFrameQueuePtr> AnimationMap;
        
        
        //
        //adds a key to one of our animation queues (by animationName), once the the key is reached
        //we smooth from current _value to pos over duration seconds,
        //the template M is used to define the osgAnimation motion type
        template <typename M>
        void AddKey(const T& pos, const float& duration, Callback* callback=NULL, const std::string& animationName="DEFAULT"){
            //KeyFrame frame;
            //frame.end = pos;
            //frame.duration = duration;
            //frame.motion = new M(0.0f, duration, 1.0f, osgAnimation::Motion::CLAMP);
            _keyFrameQueue->AddKey<M>(pos,duration,callback);
            
            //if this is the first key ensure _start is set to current value
            if(this->GetNumKeys() == 1)
            {_start = _value;}
        }
        
        //return a pointer to a specific key in the queue,
        //if the key does not exist then NULL is returned
        KeyFrame* GetKey(const unsigned int& index, const std::string& animationName="DEFAULT"){
            return _keyFrameQueue->GetKey(index);
        }
        //return the current front of the queue
        KeyFrame* GetCurrentKey(const std::string& animationName="DEFAULT"){
            return this->GetKey(0, animationName);
        }
        //return the current number of keys in the queue
        const unsigned int GetNumKeys(const std::string& animationName="DEFAULT"){ 
            return _keyFrameQueue->GetNumKeys();
        }
        
        //remove a key from the queue
        bool RemoveKey(const unsigned int& index, const std::string& animationName="DEFAULT"){
            return _keyFrameQueue->RemoveKey(index);
        }
        
        //return the current actual value
        T GetValue(){return _value;}
        //directly set the current value
        void SetValue(T value){_value = value;_start = value;}
        
        //
        //update toward the current keys end value by updating the osgAnimation
        //motion and using the value to interpolate between current start value 
        //and the keys end value. If we have reached the end of the keyframe queue
        //false is returned,
        bool Update(const float& timePassed){
            
            int aniStatus = _keyFrameQueue->Update(timePassed);
            
            //if we changed key or reached the end
            if(aniStatus == 2){// || aniStatus == 3){
                _start = _value;
            }
            
            
            //get current key frame
            KeyFrame* key = _keyFrameQueue->GetCurrentKey();
            //little hackey, but if it is the fianl frame get final key
            if(aniStatus == 3){key = _keyFrameQueue->GetFinalKey();}
            if(key)
            {	
                //update the motions time
                //key->motion->update(timePassed);
                //get the interpolation value
                float t = key->motion->getValue();
                
                //interpolate from current start value to keys end value based on t
                T between = key->end - _start;
                _value = _start + (between * t);
                
            }else{
                return false;
            }
            return true;//aniStatus!=3;
        }
        
    protected:
        
        virtual ~AnimateValue(void){}
        
        //pops the current key from the front, returns false
        //if there are no more keys in the queue 
        bool ChangeToNextKey(){
            /*_keyFrameQueue->ChangeToNextKey();//pop_front();
             //set the new start value to the current value
             _start = _value;
             //check the new size
             unsigned int size = _keyFrameQueue->GetNumKeys();
             if(size > 0){return true;}*/
            return false;
        }
        
    protected:
        
        //a map of pointers to the currently used animations/KeyFrameQueues
        //and their current weighting in te final Update value. 
        osg::ref_ptr<KeyFrameQueue> _keyFrameQueue;
        
        //the lists of animation KeyFrameQueues index by a name string
        //the animation name "DEFAULT" is always added in the contructor to ensure
        //we have at least on animation to play on Update
        AnimationMap _animations;
        
        //the value at the start of the key
        T _start;
        
        //the current value
        T _value;
    };
	
    //define all the common animate value types
    typedef hogbox::AnimateValue<int> AnimateInt;
    typedef osg::ref_ptr<AnimateInt> AnimateIntPtr;
    typedef hogbox::AnimateValue<float> AnimateFloat;
    typedef osg::ref_ptr<AnimateFloat> AnimateFloatPtr;
    typedef hogbox::AnimateValue<osg::Vec2> AnimateVec2;
    typedef osg::ref_ptr<AnimateVec2> AnimateVec2Ptr;
    typedef hogbox::AnimateValue<osg::Vec3> AnimateVec3;
    typedef osg::ref_ptr<AnimateVec3> AnimateVec3Ptr;
    typedef hogbox::AnimateValue<osg::Vec4> AnimateVec4;
    typedef osg::ref_ptr<AnimateVec4> AnimateVec4Ptr;
	
};
