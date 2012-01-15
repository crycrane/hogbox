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

#include <hogboxVision/Export.h>
#include <hogboxVision/VideoFileStream.h>
#include <hogboxVision/WebCamStream.h>

#include <hogboxVision/CameraBasedTracker.h>

namespace hogboxVision {

//
//VideoFileStreamWrapper
//
class HOGBOXVIS_EXPORT VideoFileStreamWrapper : public osg::Object
{
public:
	VideoFileStreamWrapper(const std::string& name, VideoFileStream* proto)
		: osg::Object(),
		m_pluginName(name),
		m_prototype(proto)
	{
	}

	VideoFileStreamWrapper(const VideoFileStreamWrapper& wrapper,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(wrapper, copyop)
	{

	}

    virtual osg::Object* cloneType() const { return new VideoFileStreamWrapper (m_pluginName, m_prototype); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new VideoFileStreamWrapper (*this,copyop); }
    bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const VideoFileStreamWrapper*>(obj)!=NULL; }
    const char* className() const { return m_prototype->className(); }
	const char* libraryName() const { return "hogboxVision"; }
	
	const std::string& GetPluginName(){return m_pluginName;}
	VideoFileStream* GetPrototype(){return m_prototype;}
	
protected:

	virtual ~VideoFileStreamWrapper(void)
	{
		m_prototype = NULL;
	}

protected:

	VideoFileStreamPtr m_prototype;	
	std::string m_pluginName; //just base part at end of lib name e.g. dshow
};

typedef osg::ref_ptr<VideoFileStreamWrapper> VideoFileStreamWrapperPtr;


//
//WebCamStreamWrapper
//
class HOGBOXVIS_EXPORT WebCamStreamWrapper : public osg::Object
{
public:
	WebCamStreamWrapper(const std::string& name, WebCamStream* proto)
		: osg::Object(),
		m_pluginName(name),
		m_prototype(proto)
	{
	}

	WebCamStreamWrapper(const WebCamStreamWrapper& wrapper,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(wrapper, copyop)
	{

	}

    virtual osg::Object* cloneType() const { return new WebCamStreamWrapper (m_pluginName, m_prototype); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new WebCamStreamWrapper (*this,copyop); }
    bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const WebCamStreamWrapper*>(obj)!=NULL; }
    const char* className() const { return m_prototype->className(); }
	const char* libraryName() const { return "hogboxVision"; }
	
	const std::string& GetPluginName(){return m_pluginName;}
	WebCamStream* GetPrototype(){return m_prototype;}
	
protected:

	virtual ~WebCamStreamWrapper(void)
	{
		m_prototype = NULL;
	}

protected:

	WebCamStreamPtr m_prototype;
	std::string m_pluginName; //just base part at end of lib name e.g. dshow
};

typedef osg::ref_ptr<WebCamStreamWrapper> WebCamStreamWrapperPtr;


//
//CameraBasedTrackerWrapper
//
class HOGBOXVIS_EXPORT CameraBaseTrackerWrapper : public osg::Object
{
public:
	CameraBaseTrackerWrapper(CameraBasedTracker* proto)
		: osg::Object(),
		m_prototype(proto)
	{
	}

	CameraBaseTrackerWrapper(const CameraBaseTrackerWrapper& wrapper,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(wrapper, copyop)
	{

	}

    virtual osg::Object* cloneType() const { return new CameraBaseTrackerWrapper (m_prototype); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new CameraBaseTrackerWrapper (*this,copyop); }
    bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const CameraBaseTrackerWrapper*>(obj)!=NULL; }
    const char* className() const { return m_prototype->className(); }
	const char* libraryName() const { return "hogboxVision"; }
	
	CameraBasedTracker* GetPrototype(){return m_prototype;}
	
protected:

	virtual ~CameraBaseTrackerWrapper(void)
	{
		m_prototype = NULL;
	}

protected:

	CameraBasedTrackerPtr m_prototype;	
};

typedef osg::ref_ptr<CameraBaseTrackerWrapper> CameraBaseTrackerWrapperPtr;



}; //end hogboxVision namespace
	
