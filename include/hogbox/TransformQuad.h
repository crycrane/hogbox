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
//A matrixtransform with a quadgeode attached
//also forwards access to the quadgeodes stateset
//functions
//
class HOGBOX_EXPORT TransformQuad : public osg::MatrixTransform
{
public:
    
    enum SizeStyle{
        SIZE_BY_SCALEMATRIX = 0, //default, fine for hard edge quads
        SIZE_BY_GEOM = 1 //best for quads wioth rounded corners but overhead of rebuilding quad
    };
    
    //extend QuadArgs
    class TransformQuadArgs : public QuadGeode::QuadGeodeArgs {
    public:
        TransformQuadArgs()
            : QuadGeode::QuadGeodeArgs(),
            _sizeStyle(SIZE_BY_SCALEMATRIX),
            _rotatePlane(Quad::PLANE_XY)
        {
        }
        
        TransformQuadArgs(const TransformQuadArgs& args)
            : QuadGeode::QuadGeodeArgs(args),
            _sizeStyle(args._sizeStyle),
            _rotatePlane(args._rotatePlane)
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
    TransformQuad(TransformQuadArgs* args=NULL);
    
    //
    //Contruct base quad geometry now, passing width height and
    //quad args, quad args can't be null
    TransformQuad(const float& width, const float& height, TransformQuadArgs* args = new TransformQuadArgs());

    //
    //Contruct base quad geometry now, passing width height and
    //quad args, quad args can't be null
    TransformQuad(const osg::Vec2& size, TransformQuadArgs* args = new TransformQuadArgs());
    
    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    TransformQuad(const TransformQuad& quad,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    META_Object(hogbox,TransformQuad);
    
    //
    //Build the actual quad and attach to geode
    void buildQuad(const float& width, const float& height, TransformQuadArgs* args = NULL);
    
    //
    //Build the actual quad and attach to geode
    void buildQuad(const osg::Vec2& size, TransformQuadArgs* args = NULL){
        this->buildQuad(size.x(), size.y(), args);
    }
    
    
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
    virtual void SetSize(const osg::Vec2& size);
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
    //stateset for quadgeode
    
    //Apply the texture to the channel 0/diffuse
    virtual void ApplyTexture(osg::Texture* tex, const unsigned int& channel=0);
    
    //set the material color of the region
    virtual void SetColor(const osg::Vec3& color);
    const osg::Vec3& GetColor() const;
    
    //set the regions Alpha value
    virtual void SetAlpha(const float& alpha);
    const float& GetAlpha() const;
    //get set enable alpha
    virtual void EnableAlpha(const bool& enable);
    const bool& IsAlphaEnabled() const;
    
    //Hack for now, but blending doesn't work without light, but coloring doesn't work with
    void DisableLighting(){
        _stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    }
    
    //
    //Loads and applies a custom shader
    void SetCustomShader(const std::string& vertShader, const std::string& fragShader, const bool& shadersAreSource = false);
    
    //
    //Enable/Disable color writes, i.e. only write to depth buffer
    void DisableColorWrites();
    void EnableColorWrites();
    
    //
    void SetColorWriteMask(const bool& enable);
    
    //
    //Enable/Disable depth writes
    void DisableDepthWrites();
    void EnableDepthWrites();
    
    //
    void SetDepthWriteMask(const bool& enable);
    
    //
    //Enable/Disable depth testing
    void DisableDepthTest();
    void EnableDepthTest();
    
    //
    void SetDepthTestEnabled(const bool& enable);
    
    //
    //Enable fast drawm, just disables depth writes and testing
    void EnableFastDraw();
    
    //
    //Set the renderbin number
    void SetRenderBinNumber(const int& num);
    
protected:
    
    //destructor
    virtual ~TransformQuad(void);


    //
    //Build our base graph of translate, rotate
    //scale and geode
    void buildBaseGraph();
    
protected:
    
    osg::ref_ptr<TransformQuadArgs> _args;
    
    
    //translate attached to root, for hudspace translation (xy)
    osg::ref_ptr<osg::MatrixTransform> _translate;
    //rotate attached to translate, for rotation around z axis
    osg::ref_ptr<osg::MatrixTransform> _rotate;
    //scale attached to rotate, scales the region geom to
    //region size. 
    osg::ref_ptr<osg::MatrixTransform> _scale;
    
    //quad geode
    osg::ref_ptr<QuadGeode> _quadGeode;
    
    
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

};

    
}; //end hogboxhud namespace
