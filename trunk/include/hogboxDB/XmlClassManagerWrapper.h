/*
 *  untitled.h
 *  hogboxIPhone
 *
 *  Created by Thomas Hogarth on 01/10/2009.
 *  Copyright 2009 HogBox. All rights reserved.
 *
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
	
