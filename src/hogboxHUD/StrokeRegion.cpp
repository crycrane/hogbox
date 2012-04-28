#include <hogboxHUD/StrokeRegion.h>


using namespace hogboxHUD;

//
StrokeRegion::StrokeRegion(StrokeRegionStyle* args)
    : Region(args)
{
    StrokeRegionStyle* asStrokeStyle = dynamic_cast<StrokeRegionStyle*>(args);
    if(asStrokeStyle){

    }
}

StrokeRegion::StrokeRegion(osg::Vec2 corner, osg::Vec2 size, StrokeRegionStyle* style) 
    : Region(corner, size, style)
{
	
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
StrokeRegion::StrokeRegion(const StrokeRegion& region,const osg::CopyOp& copyop)
	: Region(region, copyop)
{
}

StrokeRegion::~StrokeRegion(void)
{
	//OSG_NOTICE << "    Deallocating ListRegion: Name '" << this->getName() << "'." << std::endl;
}

//
// Create a button with a loaded file as backdrop
//
bool StrokeRegion::Create(osg::Vec2 corner, osg::Vec2 size, RegionStyle* args)
{
    osg::Vec2 smallSize = size;
    osg::Vec2 smallCorner = corner;
    
    if(args){
        _args=args;
    }
    
    //args need to cast to StrokeRegionStyle
    StrokeRegionStyle* asStrokeStyle = dynamic_cast<StrokeRegionStyle*>(args);
    if(asStrokeStyle){
        
        if(asStrokeStyle->_strokeSize > 0.0f){
            //compute the smaller size
            //smallSize = osg::Vec2(size.x()-asStrokeStyle->_strokeSize,size.y()-asStrokeStyle->_strokeSize);

            if(args->_originType == hogbox::Quad::ORI_CENTER){
                //corner stays  the same
            }else if(args->_originType == hogbox::Quad::ORI_BOTTOM_LEFT){
                //offset corner 
                osg::Vec2 sizeDiff = smallSize - size;
                //smallCorner =  corner + -(sizeDiff*0.5f);
            }
        }
    }
    
    //load the base assets and apply names and sizes
	bool ret = Region::Create(smallCorner,smallSize,args);
    

    
  /*  _itemsRoot = new Region();
    _itemsRoot->Create(osg::Vec2(0,0), osg::Vec2(1,1));
    this->AddChild(_itemsRoot.get());
   */
    
    return ret;
}

//
//
//
bool StrokeRegion::LoadAssest(RegionStyle* args)
{
	//call base first
	bool ret = Region::LoadAssest(args);

    StrokeRegionStyle* asStrokeStyle = dynamic_cast<StrokeRegionStyle*>(args);
    if(asStrokeStyle){
        
        if(asStrokeStyle->_strokeSize > 0.0f){
            //clone args and use to init stroke region
            osg::ref_ptr<RegionStyle> strokeArgs = new RegionStyle(*args);
            _stroke = new Region(strokeArgs.get());
            
            osg::Vec2 stokeSize = osg::Vec2(_size.x()+asStrokeStyle->_strokeSize,_size.y()+asStrokeStyle->_strokeSize);
            osg::Vec2 strokeCorner = osg::Vec2(0,0);
            
            if(args->_originType == hogbox::Quad::ORI_CENTER){
                    //corner stays the same
            }else if(args->_originType == hogbox::Quad::ORI_BOTTOM_LEFT){
                //offset corner 
                osg::Vec2 sizeDiff = _size - stokeSize;
                strokeCorner =  strokeCorner + (sizeDiff*0.5f);
            }
            
            float sizeScale = stokeSize.x()/_size.x();
            
            //calc scaled corner radius if any
            for(unsigned int i=0; i<4; i++){
                if(strokeArgs->_corners[i]._radius > 0.0f){
                    strokeArgs->_corners[i]._radius *= sizeScale;  
                }
            }

            // OSG_ALWAYS << "StrokeColor: " << stokeSize.x() << ", " << stokeSize.y() << std::endl;
            _stroke->Create(strokeCorner, stokeSize);
            _stroke->SetColor(asStrokeStyle->_strokeColor);
            
            
            _stroke->SetLayer(-0.001f);
            this->AddChild(_stroke.get());
        }
    }
    
	return ret;
}

//
//set the regions Alpha value
//
void StrokeRegion::SetAlpha(const float& alpha)
{
    if(_stroke.get()){
        _stroke->SetAlpha(alpha);
    }
    Region::SetAlpha(alpha);
}

//
void StrokeRegion::EnableAlpha(const bool& enable)
{
    if(_stroke.get()){
        _stroke->EnableAlpha(enable);
    }
    Region::EnableAlpha(enable);    
}
