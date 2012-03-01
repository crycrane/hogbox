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

#include <hogboxDB/XmlClassManager.h>
#include "OsgShaderXmlWrapper.h"
#include "OsgUniformXmlWrapper.h"


//
//Manages the loading of the hogbox xml representation
//of osg Shader objects. These include Uniforms, Programs,
//and Shaders (both vertex and fragment)
//
class OsgShaderManager : public hogboxDB::XmlClassManager
{
public:

	OsgShaderManager(void) : hogboxDB::XmlClassManager()
	{
		SupportsClassType("Shader", new OsgShaderXmlWrapper());//"Xml definition of osg Shader");
		SupportsClassType("Uniform", new OsgUniformXmlWrapper());//"Xml definition of osg uniform");
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	OsgShaderManager(const OsgShaderManager& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: hogboxDB::XmlClassManager(manager, copyop)
	{
	}

	META_Object(hogboxDB, OsgShaderManager)

protected:

	virtual ~OsgShaderManager(void){
	}

};

//REGISTER_HOGBOXPLUGIN(Shader, OsgShaderManager)

