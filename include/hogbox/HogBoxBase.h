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

#include <assert.h>
#include <osg/Object>
#include <osg/Node>
#include <osg/Image>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/Uniform>
#include <osg/Shader>
#include <osg/Program>
#include <osgDB/XmlParser>
#include <osgText/Font>



namespace hogbox
{

enum NodeMasks{
	MAIN_CAMERA_CULL = 0x00000001,//render to the default viewer camera
	MAIN_CAMERA_LEFT_CULL = 0x00000002,//for default cameras stereo pair
	MAIN_CAMERA_RIGHT_CULL = 0x00000003,
	PICK_MESH = 0x00000004, //object will be tested during intersection visitors
	COLLIDE_MESH = 0x00000005 //object will be used in collision detection
};


}; //end hogbox namespace

namespace osg
{

//convinience type defs for osg::ref_ptrs of basic types
typedef osg::ref_ptr<osg::Object> ObjectPtr;
typedef std::vector<ObjectPtr> ObjectPtrVector;
typedef osg::ref_ptr<osg::Node> NodePtr;
typedef std::vector<NodePtr> NodePtrVector;
typedef osg::ref_ptr<osg::Image> ImagePtr;
typedef std::vector<ImagePtr> ImagePtrVector;
typedef osg::ref_ptr<osg::Texture> TexPtr;
typedef std::vector<TexPtr> TexPtrVector;
typedef osg::ref_ptr<osg::Texture2D> Tex2DPtr;
typedef std::vector<Tex2DPtr> Tex2DPtrVector;
typedef osg::ref_ptr<osg::Uniform> UniformPtr;
typedef std::vector<UniformPtr> UniformPtrVector;
typedef osg::ref_ptr<osg::Shader> ShaderPtr;
typedef std::vector<ShaderPtr> ShaderPtrVector;
typedef osg::ref_ptr<osg::Program> ProgramPtr;
};

namespace osgDB {
typedef osg::ref_ptr<osgDB::XmlNode> XmlNodePtr;
};

namespace osgText
{
//convinience type defs for osg::ref_ptrs of basic types
typedef osg::ref_ptr<osgText::Font> FontPtr;
};