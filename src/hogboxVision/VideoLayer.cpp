#include <hogboxVision/VideoLayer.h>


#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Depth>
#include <osg/Geometry>
#include <osg/BlendFunc>
#include <osg/Notify>
#include <osg/Image>
#include <osg/TexMat>

#include <hogbox/NPOTResizeCallback.h>

using namespace hogboxVision;

// FSVideoLayer

FSVideoLayer::FSVideoLayer(int width, int height) : osg::Group(),
							m_width(width), m_height(height)
{
	// build the layer as a child of this group
	this->addChild(buildLayer().get());
}


FSVideoLayer::FSVideoLayer(const FSVideoLayer& FSVideoLayer,
	const osg::CopyOp& copyop)
{

}


FSVideoLayer::~FSVideoLayer()
{	

}

//
// Build the gemo and contruct the orth projectipon matrices
//
osg::ref_ptr<osg::Projection> FSVideoLayer::buildLayer() 
{

	m_layerProjectionMatrix = new osg::Projection(osg::Matrix::ortho2D(0, m_width, 0, m_height));

	m_layerModelViewMatrix = new osg::MatrixTransform();
	m_layerModelViewMatrix->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	m_layerProjectionMatrix->addChild(m_layerModelViewMatrix.get());

	osg::Group* layerGroup = new osg::Group();
	m_layerModelViewMatrix->addChild(layerGroup);

	layerGroup->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::ALWAYS, 1.0f, 1.0f));
	layerGroup->addChild(buildLayerGeometry().get());

	return m_layerProjectionMatrix;
}


osg::ref_ptr<osg::Geode> FSVideoLayer::buildLayerGeometry() 
{
	float minU = 0.0f;
	float maxU = 1.0f;//m_width;//1.0f
	float minV = 1.0f;
	float maxV = 0.0f;//m_height;//1.0f;

	m_layerGeode = new osg::Geode();

	m_geometry = new osg::Geometry();
	
	osg::Vec3Array* coords = new osg::Vec3Array();
	m_geometry->setVertexArray(coords);

	osg::Vec2Array* tcoords = new osg::Vec2Array();
	m_geometry->setTexCoordArray(0, tcoords);


	coords->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
	coords->push_back(osg::Vec3(m_width, 0.0f, 0.0f));
	coords->push_back(osg::Vec3(m_width, m_height, 0.0f));
	coords->push_back(osg::Vec3(0.0f, m_height, 0.0f));

	tcoords->push_back(osg::Vec2(minU, minV));
	tcoords->push_back(osg::Vec2(maxU, minV));
	tcoords->push_back(osg::Vec2(maxU, maxV));
	tcoords->push_back(osg::Vec2(minU, maxV));
	

	m_geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    osg::ref_ptr<osg::Vec4Array> quad_colors = new osg::Vec4Array;
    quad_colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    m_geometry->setColorArray(quad_colors.get());
    m_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	osg::StateSet* geoState = this->getOrCreateStateSet();// m_geometry->getOrCreateStateSet();

	geoState->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	geoState->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	
	m_layerGeode->addDrawable(m_geometry.get());

	return m_layerGeode;

}

//apply a different texture to our fs quad
void FSVideoLayer::setTextureFromVideoStream(VideoStream* videoStream, int channel)
{
	//apply the video stream to the geom state set using hogbox helper
	//CHogBox::ApplyVideoTextureToState(this->getOrCreateStateSet(), videoStream, channel); 
}

void FSVideoLayer::setTextureFromTex2D(osg::Texture2D* texture, int channel)
{
    this->getOrCreateStateSet()->setTextureAttributeAndModes(channel, texture, osg::StateAttribute::ON);
	
	//NOTE@tom, below isn't needed on platforms supporting glu
	//apply a non power of two rezie callback if required
	osg::ref_ptr<hogbox::NPOTResizeCallback> resizer = new hogbox::NPOTResizeCallback(texture, channel, this->getOrCreateStateSet());
	if(resizer->useAsCallBack()){texture->setSubloadCallback(resizer.get());}
	
}

void FSVideoLayer::setTextureFromTexRect(osg::TextureRectangle* texture, int channel)
{
	this->getOrCreateStateSet()->setTextureAttributeAndModes(channel, texture, osg::StateAttribute::ON);
	//requires coord scaling
	osg::ref_ptr<osg::TexMat> texMat = new osg::TexMat();
	//texMat->setScaleByTextureRectangleSize(true);
	//apply the texture to the quads stateset
 	this->getOrCreateStateSet()->setTextureAttributeAndModes(channel, texMat.get(), osg::StateAttribute::ON);
	
}

void FSVideoLayer::SetRotation(float x, float y, float z, int vFlip, int hFlip)
{

	m_layerModelViewMatrix->setMatrix(//	osg::Matrix::scale(vFlip, hFlip, 1) * 
										osg::Matrix::rotate(x, 1,0,0)*
										osg::Matrix::rotate(y, 0,1,0)*
										osg::Matrix::rotate(z, 0,0,1)); 
}

