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

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>

#include <hogbox/HogBoxMaterial.h>

namespace hogbox {


//
//HogBoxHardwareRigTransform
//Is used by a mesh to implement the required matrixPallete uniforms and bind the boneWeight attributes to this geom
//The uniforms are applied to the RigGeoms stateset so as to be usable by the material
//applied to the geode above 
//
class HOGBOX_EXPORT HogBoxHardwareRigTransform : public osgAnimation::RigTransformHardware
{
public:
	HogBoxHardwareRigTransform(void)
		: osgAnimation::RigTransformHardware()
	{
	}

	//When the RigTransform is applied to a rig geometry the below method
	//is invoked
    void operator()(osgAnimation::RigGeometry& geom)
    {
        if (_needInit)
            if (!Init(geom))
                return;
        computeMatrixPaletteUniform(geom.getMatrixFromSkeletonToGeometry(), geom.getInvMatrixFromSkeletonToGeometry());
    }

	//
	//First of all check the geom is valid for the hardware skinning.
	//Bind the boneWeight attributes to the geom and material program. 
	//We also apply the matrixPalete uniform to the geom
	//
    bool Init(osgAnimation::RigGeometry& geom)
    {
		//Check there is a valid vertex array
        osg::Vec3Array* pos = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
        if (!pos) {
			OSG_WARN << "HogBoxHardwareRigTransform: Init: WARN: No Vertex array in the geometry '" << geom.getName() << "', Harware Skinning will not be applied." << std::endl;
            return false;
        }

        if (!geom.getSkeleton()) {
			osg::notify(osg::WARN) << "HogBoxHardwareRigTransform: Init: WARN: No osgAnimation Skeleton set in geometry '" << geom.getName() << "', Harware Skinning will not be applied."  << std::endl;
            return false;
        }

		//Get a list of the bones in the skeleton
        osgAnimation::BoneMapVisitor mapVisitor;
        geom.getSkeleton()->accept(mapVisitor);
        osgAnimation::BoneMap bm = mapVisitor.getBoneMap();

		//Create our matrix palete representing our bones transforms
        if (!createPalette(pos->size(),bm, geom.getVertexInfluenceSet().getVertexToBoneList()))
            return false;

        int attribIndex = 9;//6 and 7 used by tangent binormal, 8 is texcoord0 on nvidia cards
        int nbAttribs = getNumVertexAttrib();

		OSG_WARN << "MATRIX PALETE SIZE: " << getMatrixPaletteUniform()->getNumElements() << std::endl;
		//bind the bone weight attribute arrays to the geometry
        for (int i = 0; i < nbAttribs; i++)
        {
            std::stringstream ss;
            ss << "boneWeight" << i;
            geom.setVertexAttribData(attribIndex + i, osg::Geometry::ArrayData(getVertexAttrib(i),osg::Geometry::BIND_PER_VERTEX));
        }

		//apply our matrix palette to the geoms stateset
		osg::ref_ptr<osg::StateSet> ss = geom.getOrCreateStateSet();//new osg::StateSet;
        ss->addUniform(getMatrixPaletteUniform());
		int nbpv = getNumBonesPerVertex();
        ss->addUniform(new osg::Uniform("nbBonesPerVertex", nbpv));

		_needInit = false;
        
		return true;
    }

protected:

	virtual ~HogBoxHardwareRigTransform(){
	}

};


//
//ApplyHardwareRigTransformToRigGeomVisitor
//Visitor to apply the HardRigTransform to any RigGeometries encountered
//
struct ApplyHardwareRigTransformToRigGeomVisitor : public osg::NodeVisitor
{
    ApplyHardwareRigTransformToRigGeomVisitor( bool hardware = true) 
		: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), 
		_hardware(hardware) 
	{
	}
    
    void apply(osg::Geode& geode)
    {
        for (unsigned int i = 0; i < geode.getNumDrawables(); i++)
            apply(*geode.getDrawable(i));
    }
    void apply(osg::Drawable& geom)
    {
        if (_hardware) {
			//if its a riggeom
            osgAnimation::RigGeometry* rig = dynamic_cast<osgAnimation::RigGeometry*>(&geom);
			if (rig){
				//apply the HardwareRigTransform
                rig->setRigTransformImplementation(new HogBoxHardwareRigTransform);
			}
        }
#if 0
        if (geom.getName() != std::string("BoundingBox")) // we disable compute of bounding box for all geometry except our bounding box
            geom.setComputeBoundingBoxCallback(new osg::Drawable::ComputeBoundingBoxCallback);
//            geom.setInitialBound(new osg::Drawable::ComputeBoundingBoxCallback);
#endif
    }
	bool _hardware;
};


}; //end hogbox namespace