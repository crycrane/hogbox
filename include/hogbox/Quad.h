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

#include <osg/Geometry>
#include <osg/Geode>


namespace hogbox {

class HOGBOX_EXPORT Quad : public osg::Geometry
{
public:
    
    //orientation
    enum QuadPlane{
        PLANE_XY = 0,
        PLANE_XZ = 1,
        PLANE_YZ = 2
    };
    
    enum QuadOrigin{
        ORI_BOTTOM_LEFT = 0,
        ORI_TOP_LEFT = 1,
        ORI_CENTER = 2
    };
    
    //
    //args for building quads
    class QuadArgs : public osg::Referenced {
    public:
        class Corner{
        public:
            Corner()
                : _texCoord(osg::Vec2(0.0f,0.0f)),
                _color(osg::Vec4(1,1,1,1)),
                _radius(0.0f),
                _segments(3)
            {
            }
            Corner(const Corner& cnr)
                : _texCoord(cnr._texCoord),
                _color(cnr._color),
                _radius(cnr._radius),
                _segments(cnr._segments)
            {
            }
            osg::Vec2 _texCoord;
            osg::Vec4 _color;
            float _radius;
            unsigned int _segments;
        };
        
        QuadArgs()
            : osg::Referenced(),
            _planeType(PLANE_XY),
            _rotationType(PLANE_XY),
            _originType(ORI_BOTTOM_LEFT),
            _useTexcoords(true),
            _useColor(true),
            _useNormal(true)
        {
            _corners[0]._texCoord = osg::Vec2(0.0f,0.0f);//bl
            _corners[1]._texCoord = osg::Vec2(1.0f,0.0f);//br
            _corners[2]._texCoord = osg::Vec2(1.0f,1.0f);//tl
            _corners[3]._texCoord = osg::Vec2(1.0f,1.0f);//tr
        }
        
        QuadArgs(const QuadArgs& args)
            : osg::Referenced(),
            _planeType(args._planeType),
            _rotationType(args._rotationType),
            _originType(args._originType),
            _useTexcoords(args._useTexcoords),
            _useColor(args._useColor),
            _useNormal(args._useNormal)
        {
            _corners[0] = Corner(args._corners[0]);
            _corners[1] = Corner(args._corners[1]);
            _corners[2] = Corner(args._corners[2]);
            _corners[3] = Corner(args._corners[3]);
        }
        
        //
        //Sets all corners radius to the same value
        void SetAllCornersRadius(const float& radius, const unsigned int& segments=0){
            for(unsigned int i=0; i<4; i++){
                _corners[i]._radius = radius;
                if(segments>0){
                    _corners[i]._segments = segments;
                }
            }
        }
        
        Corner      _corners[4];
        QuadPlane   _planeType;
        QuadPlane   _rotationType;
        QuadOrigin  _originType;
        bool        _useTexcoords;
        bool        _useColor;
        bool        _useNormal;
        
    protected:
        virtual ~QuadArgs(){}
    
    };

    //
    //Construt geometry but requires call to SetSize
    //in order to build the quad itself
    Quad();
    
    //
    //Contruct a quad with width, height and optional args
    Quad(float width, float height, QuadArgs* args=new QuadArgs());
    
    //
    //Contruct a quad with size and optional args
    Quad(const osg::Vec2& size, QuadArgs* args=new QuadArgs());
    
    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
    Quad(const Quad& quad,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    virtual osg::Object* cloneType() const { return new Quad(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new Quad(*this,copyop); }
    virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const Quad*>(obj)!=NULL; }
    virtual const char* libraryName() const { return "hogbox"; }
    virtual const char* className() const { return "Quad"; }
    
    //
    //Set the size of the quad, rebuilding as required
    //also optionally set the quadargs
    void SetSize(const osg::Vec2& size, QuadArgs* args=NULL);
    
    //
    //Return the size of the quad
    const osg::Vec2& GetSize();
    
    //
    //Set the width, which inturn sets size
    void SetWidth(const float& width);
    
    //
    //Return width
    const float GetWidth();
  
    //
    //Set the width, which inturn sets size
    void SetHeight(const float& height);
    
    //
    //Return height
    const float GetHeight();
    
    //
    //Set the build args, optionally force 
    //a rebuild of the quad
    void SetArgs(QuadArgs* args, const bool& rebuild = true);
    
protected:
    
    virtual ~Quad();
    
    //
	//Geom helpers
    //Convenience function to be used for creating quad geometry with texture coords.
    //Tex coords go from left bottom (l,b) to right top (r,t).
    void buildQuadGeometry(const float& width,const float& height, QuadArgs* args);
    
    osg::Vec2 computeQuadTexCoord(const osg::Vec3& coord, const float& width, const float& height,
                                                       const float& l, const float& b, const float& r, const float& t);
    
protected:
    
    osg::Vec2 _size;
    
    //quad build args
    osg::ref_ptr<QuadArgs> _args;
};
    
//
//Quad geode wraps quad geometry and stateset
//e.g. exposes color, texture data etc
class HOGBOX_EXPORT QuadGeode : public osg::Geode
{
public:
    enum ShaderMode{
        COLOR_SHADER,
        TEXTURED_SHADER,
        CUSTOM_SHADER
    };
    
    //extend QuadArgs
    class QuadGeodeArgs : public Quad::QuadArgs {
    public:
        QuadGeodeArgs()
            : Quad::QuadArgs()
        {
        }
        
        QuadGeodeArgs(const QuadGeodeArgs& args)
            : Quad::QuadArgs(args)
        {
        }
    
        
    protected:
        virtual ~QuadGeodeArgs(){
        }
    };
    
    //
    //just allocate the transform graph and geode
    QuadGeode(QuadGeodeArgs* args=NULL);
    
    //
    //Contruct base quad geometry now, passing width height and
    //quad args, quad args can't be null
    QuadGeode(const float& width, const float& height, QuadGeodeArgs* args = new QuadGeodeArgs());
    
    //
    //Contruct base quad geometry now, passing width height and
    //quad args, quad args can't be null
    QuadGeode(const osg::Vec2& size, QuadGeodeArgs* args = new QuadGeodeArgs());
    
    //Apply the texture to the channel 0/diffuse
    virtual void ApplyTexture(osg::Texture* tex, const unsigned int& channel=0);
    
    //set the material color of the region
    void SetColor(const osg::Vec3& color);
    const osg::Vec3& GetColor() const;
    
    //set the regions Alpha value
    virtual void SetAlpha(const float& alpha);
    const float& GetAlpha() const;
    //get set enable alpha
    void EnableAlpha(const bool& enable);
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
    
    virtual ~QuadGeode();
    
    //
    //Init Material/stateset stuff
    virtual void InitStateSet();
    
protected:
    
    //our quad geometry
    osg::ref_ptr<Quad> _quad;
    
    //color of quad
    osg::Vec3 _color;
    //color uniform for shader, as vec4 including alpha
    osg::ref_ptr<osg::Uniform> _colorUniform;
    //alpha of quad
    float _alpha;
    bool _alphaEnabled;
    
    //
    ShaderMode _shaderMode;
};
    
};//end hogbox namespace