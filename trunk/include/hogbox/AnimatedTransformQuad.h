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


#include <hogbox/TransformQuad.h>
#include <hogbox/AnimateValue.h>


namespace hogbox {

class AnimateUpdateCallback;

//
//Adds basic animation to a transform quad
//
class HOGBOX_EXPORT AnimatedTransformQuad : public TransformQuad
{
public:
    
    friend class AnimateUpdateCallback;

    
    //
    //just allocate the transform graph and geode
    AnimatedTransformQuad(TransformQuadArgs* args=NULL);
    
    //
    //Contruct base quad geometry now, passing width height and
    //quad args, quad args can't be null
    AnimatedTransformQuad(const float& width, const float& height, TransformQuadArgs* args = new TransformQuadArgs());

    //
    //Contruct base quad geometry now, passing width height and
    //quad args, quad args can't be null
    AnimatedTransformQuad(const osg::Vec2& size, TransformQuadArgs* args = new TransformQuadArgs());
    
    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    AnimatedTransformQuad(const AnimatedTransformQuad& quad,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    META_Object(hogbox,AnimatedTransformQuad);
    
    //
    //Animation
    
    //Rotation channel
    template <typename M>
    void AddRotationKey(float degrees, float duration, hogbox::Callback* callback=NULL)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animateRotate->GetNumKeys() == 0){_animateRotate->SetValue(this->GetRotation());}
        _animateRotate->AddKey<M>(degrees, duration, callback);
    }
    hogbox::AnimateValue<float>::KeyFrame* GetRotationKey(unsigned int index){return _animateRotate->GetKey(index);}
    bool RemoveRotationKey(unsigned int index){return _animateRotate->RemoveKey(index);}
    unsigned int GetNumRotationKeys(){return _animateRotate->GetNumKeys();}
    
    //Position channel
    template <class M>
    void AddPositionKey(osg::Vec2 pos, float duration, hogbox::Callback* callback=NULL)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animatePosition->GetNumKeys() == 0){_animatePosition->SetValue(this->GetPosition());}
        _animatePosition->AddKey<M>(pos, duration, callback);
    }
    hogbox::AnimateValue<osg::Vec2>::KeyFrame* GetPositionKey(unsigned int index){return _animatePosition->GetKey(index);}
    bool RemovePositionKey(unsigned int index){return _animatePosition->RemoveKey(index);}
    unsigned int GetNumPositionKeys(){return _animatePosition->GetNumKeys();}
    
    //Size channel
    template <typename M>
    void AddSizeKey(osg::Vec2 size, float duration, hogbox::Callback* callback=NULL)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animateSize->GetNumKeys() == 0){_animateSize->SetValue(this->GetSize());}
        _animateSize->AddKey<M>(size, duration, callback);
    }
    hogbox::AnimateValue<osg::Vec2>::KeyFrame* GetSizeKey(unsigned int index){return _animateSize->GetKey(index);}
    bool RemoveSizeKey(unsigned int index){return _animateSize->RemoveKey(index);}
    unsigned int GetNumSizeKeys(){return _animateSize->GetNumKeys();}
    
    
    //are any of our channels still animating
    bool IsAnimating()
    {
        unsigned int totalKeys = GetNumPositionKeys() + GetNumSizeKeys() + GetNumRotationKeys();
        if(totalKeys > 0)
        {return true;}
        return false;
    }
    
    //
    //Enable disable any updates of animations
    void SetAnimationDisabled(bool disable){_animationDisabled = disable;}
    
    //Color channel
    template <typename M>
    void AddColorKey(osg::Vec3 color, float duration, hogbox::Callback* callback=NULL)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animateColor->GetNumKeys() == 0){_animateColor->SetValue(this->GetColor());}
        _animateColor->AddKey<M>(color, duration, callback);
    }
    hogbox::AnimateValue<osg::Vec3>::KeyFrame* GetColorKey(unsigned int index){return _animateColor->GetKey(index);}
    bool RemoveColorKey(unsigned int index){return _animateColor->RemoveKey(index);}
    unsigned int GetNumColorKeys(){return _animateColor->GetNumKeys();}
    
    //Alpha channel
    template <typename M>
    void AddAlphaKey(float alpha, float duration, hogbox::Callback* callback=NULL)
    {
        //if its going to be the first key ensure value is the regions current
        if(_animateAlpha->GetNumKeys() == 0){_animateAlpha->SetValue(this->GetAlpha());}
        _animateAlpha->AddKey<M>(alpha, duration, callback);
    }
    hogbox::AnimateValue<float>::KeyFrame* GetAlphaKey(unsigned int index){return _animateAlpha->GetKey(index);}
    bool RemoveAlphaKey(unsigned int index){return _animateAlpha->RemoveKey(index);}
    unsigned int GetNumAlphaKeys(){return _animateAlpha->GetNumKeys();}
    
protected:
    
    //destructor
    virtual ~AnimatedTransformQuad(void);
    
    //
    //set default animation values and update callback
    void setDefaultAnimationValues();
    
    //
    //general update, updates and syncs our animations etc
    //normally called by an attached updateCallback
    virtual void UpdateAnimation(float simTime);
    
protected:


    //
    //Our default update callback to ensure update is called each frame
    osg::ref_ptr<AnimateUpdateCallback> _updateCallback;
    
    //
    //Animation for each of our attributes
    hogbox::AnimateFloatPtr _animateRotate; bool _isRotating;
    hogbox::AnimateVec2Ptr _animatePosition; bool _isTranslating;
    hogbox::AnimateVec2Ptr _animateSize; bool _isSizing;
    
    hogbox::AnimateVec3Ptr _animateColor; bool _isColoring;
    hogbox::AnimateFloatPtr _animateAlpha; bool _isFading;
    
    //previous framestamp time to calc time elapsed
    float _prevTick;
    
    //used to stop any animation
    bool _animationDisabled;
};

//
//HudRegionUpdateCallback
//our default update callback which will ensure animations etc are updated
//
class AnimateUpdateCallback : public osg::NodeCallback
{
public:
    //contuct, passing the region to update
    AnimateUpdateCallback(AnimatedTransformQuad* region) 
        : osg::NodeCallback(),
        p_updateRegion(region),
        _prevTick(0.0f)
    {
    }
    
    //
    //Update operator
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if (p_updateRegion &&
            nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR && 
            nv->getFrameStamp())
        {
            //get the time passed since last update
            double time = nv->getFrameStamp()->getReferenceTime();
            if(_prevTick==0.0f){_prevTick = time;}
            //float timePassed = time - _prevTick;
            _prevTick = time;
            
            p_updateRegion->UpdateAnimation(time);
        }
        osg::NodeCallback::traverse(node,nv);
    }
    
protected:
    
    virtual~AnimateUpdateCallback(void){}
    
protected:
    
    AnimatedTransformQuad* p_updateRegion;
    float _prevTick;
};
    
}; //end hogboxhud namespace
