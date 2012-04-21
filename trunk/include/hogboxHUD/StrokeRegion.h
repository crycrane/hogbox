
#pragma once

#include <hogboxHUD/Region.h>

namespace hogboxHUD {

//
//
//
class HOGBOXHUD_EXPORT StrokeRegion : public Region
{
public:
    class StrokeRegionStyle : public RegionStyle{
    public:
        StrokeRegionStyle()
            : RegionStyle(),
            _strokeSize(0.0f),
            _strokeColor(osg::Vec3(0.0f,0.0f,0.0f))
        {
        }
        
        StrokeRegionStyle(const StrokeRegionStyle& args)
            : RegionStyle(args),
            _strokeSize(args._strokeSize),
            _strokeColor(args._strokeColor)
        {
        }
        
        //percentage of size used for stoke
        float _strokeSize;
        
        //color of stroke
        osg::Vec3 _strokeColor;
        
    protected:
        virtual ~StrokeRegionStyle(){
        }
    };
    
    //
    StrokeRegion(StrokeRegionStyle* style = new StrokeRegionStyle());
    
    //
    //When calling create, this region will be main region and be slightly
    //shrunk to allow the _stroke region to 
	StrokeRegion(osg::Vec2 corner, osg::Vec2 size, StrokeRegionStyle* style = new StrokeRegionStyle());

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	StrokeRegion(const StrokeRegion& region,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	META_Object(hogboxHUD,StrokeRegion);
    virtual RegionStyle* allocateStyleType(){return new StrokeRegionStyle();}

	//
	//override to provide stroke args by default
	virtual bool Create(osg::Vec2 corner, osg::Vec2 size, RegionStyle* args = NULL);//new StrokeRegionStyle() );
    
    //
    //set the regions Alpha value
    virtual void SetAlpha(const float& alpha);
    virtual void EnableAlpha(const bool& enable);

protected:

	virtual ~StrokeRegion(void);
    

	//
    //Creates an aditional region positioned, scaled and
    //coloured to appear as a stroke around this region
	virtual bool LoadAssest(RegionStyle* args);

protected:

    //
    RegionPtr _stroke;
	
};

typedef osg::ref_ptr<StrokeRegion> StrokeRegionPtr;

};