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


#include <hogbox/Quad.h>
#include <hogbox/AnimateValue.h>
#include <osg/MatrixTransform>


namespace hogbox {

class AnimateUpdateCallback;

//
//A matrixtransform with a quad attached to a geode
//
class HOGBOX_EXPORT TransformQuad : public osg::MatrixTransform
{
public:
    
    enum SizeStyle{
        SIZE_BY_SCALEMATRIX = 0, //default, fine for hard edge quads
        SIZE_BY_GEOM = 1 //best for quads wioth rounded corners but overhead of rebuilding quad
    };
    
    //extend QuadArgs
    class TransformQuadArgs : public Quad::QuadArgs {
    public:
        TransformQuadArgs()
            : Quad::QuadArgs(),
            _sizeStyle(SIZE_BY_SCALEMATRIX),
            _rotatePlane(Quad::PLANE_XY)
        {
        }
        
        SizeStyle _sizeStyle;
        Quad::QuadPlane _rotatePlane;
        
    protected:
        virtual ~TransformQuadArgs(){
        }
    };
    
    //
    //just allocate the transform graph and geode
    TransformQuad();
    
    //
    //Contruct base quad geometry now, passing width height and
    //quad args, quad args can't be null
    TransformQuad(const float& width, const float& height, TransformQuadArgs* args = new TransformQuadArgs());
    
    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    TransformQuad(const TransformQuad& quad,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    META_Object(hogbox,TransformQuad);
      
    //
    //general update, updates and syncs our animations etc
    //normally called by an attached updateCallback
    virtual void Update(float simTime);
    
    //
    //translation in hudspace, set position virtual
    //to handle special cases
    virtual void SetPosition(const osg::Vec2& corner);
    const osg::Vec2& GetPosition() const;
    
    //get set roation around the z axis in degrees
    virtual void SetRotation(const float& rotate);
    const float& GetRotation() const;
    
    //set the plane of rotation
    void SetRotationPlane(Quad::QuadPlane plane){_args->_rotatePlane = plane;}
    const Quad::QuadPlane& GetRotationPlane() const{return _args->_rotatePlane;}
    
    //there are two styles of setsize, 
    //SIZE_BY_SCALEMATRIX will scale the unit sized quad with the scale matrix
    //SIZE_BY_GEOM will recreate the quad at the new size leaving scale matrix at 1,1,1
    //optionally set new build args
    virtual void SetSize(const osg::Vec2& size, TransformQuadArgs* args=NULL);
    //return the size of the region in hud coord
    const osg::Vec2& GetSize() const;
    
    //set the size as a percentage/scale factor of the current size
    //i.e. <1 smaller, >1 bigger
    void SetSizeFromPercentage(float xScaler, float yScaler);
    
    //set the position as a percentage/scale factor of the current position
    //i.e. <1 smaller, >1 bigger
    void SetPositionFromPercentage(float xScaler, float yScaler);
    
    //move to a new layer, z depth relative to parent
    void SetLayer(const float& depth);
    const float& GetLayer() const;

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
    
    
    
protected:
    
    //destructor
    virtual ~TransformQuad(void);


    //
    //Build our base graph of translate, rotate
    //scale and geode
    void buildBaseGraph();
    
    //
    //Build the actual quad and attach to geode
    void buildQuad(const float& width, const float& height, Quad::QuadArgs* args = new Quad::QuadArgs());
    
protected:
    
    osg::ref_ptr<TransformQuadArgs> _args;
    
    
    //translate attached to root, for hudspace translation (xy)
    osg::ref_ptr<osg::MatrixTransform> _translate;
    //rotate attached to translate, for rotation around z axis
    osg::ref_ptr<osg::MatrixTransform> _rotate;
    //scale attached to rotate, scales the region geom to
    //region size. 
    osg::ref_ptr<osg::MatrixTransform> _scale;
    
    //geode for the quad
    osg::ref_ptr<osg::Geode> _quadGeode;
    
    //quad geometry
    osg::ref_ptr<Quad> _quad;
    
    
    //size in hud coords
    osg::Vec2 _size;
    //bottom left corner in screen or parent space
    //i.e. if the region is attached as child to another
    //the regions origin becomes the parents bottom left corner
    osg::Vec2 _corner;
    //rotation in degrees arond the z axis
    float _rotation;
    //depth/layer of the region i.e. z coord relative to parent
    //the root region held by the hogboxHud sets its depth/layer to -1
    float _depth;

    //
    //Our default update callback to ensure update is called each frame
    osg::ref_ptr<AnimateUpdateCallback> _updateCallback;
    
    //
    //Animation for each of our attributes
    hogbox::AnimateFloatPtr _animateRotate; bool _isRotating;
    hogbox::AnimateVec2Ptr _animatePosition; bool _isTranslating;
    hogbox::AnimateVec2Ptr _animateSize; bool _isSizing;
    
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
    AnimateUpdateCallback(TransformQuad* region) 
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
            
            p_updateRegion->Update(time);
        }
        osg::NodeCallback::traverse(node,nv);
    }
    
protected:
    
    virtual~AnimateUpdateCallback(void){}
    
protected:
    
    TransformQuad* p_updateRegion;
    float _prevTick;
};
    
}; //end hogboxhud namespace
