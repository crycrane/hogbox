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

#include <hogboxDB/Export.h>
#include <hogboxDB/XmlClassManager.h>

namespace hogboxDB {

//
//XmlClassManagerWrapper
//Used to wrap each type of xmlnodemanager for
//registration with the registry
//
class HOGBOXDB_EXPORT XmlClassManagerWrapper : public osg::Object
{
public:
	XmlClassManagerWrapper(hogboxDB::XmlClassManager* proto)
		: osg::Object(),
		m_prototype(proto)
	{
	}

	XmlClassManagerWrapper(const XmlClassManagerWrapper& manager,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(manager, copyop)
	{

	}

    virtual osg::Object* cloneType() const { return new XmlClassManagerWrapper (m_prototype); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new XmlClassManagerWrapper (*this,copyop); }
    bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const XmlClassManagerWrapper*>(obj)!=NULL; }
    const char* className() const { return m_prototype->className(); }
	const char* libraryName() const { return "hogboxDB"; }
	
	XmlClassManager* GetPrototype(){return m_prototype;}
	
protected:

	virtual ~XmlClassManagerWrapper(void)
	{
		m_prototype = NULL;
	}

protected:

	osg::ref_ptr<XmlClassManager> m_prototype;	
};

typedef osg::ref_ptr<XmlClassManagerWrapper> XmlClassManagerWrapperPtr;

}; //end hogbox namespace
	
