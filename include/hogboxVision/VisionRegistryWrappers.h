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
		_pluginName(name),
		_prototype(proto)
	{
	}

	VideoFileStreamWrapper(const VideoFileStreamWrapper& wrapper,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(wrapper, copyop)
	{

	}

    virtual osg::Object* cloneType() const { return new VideoFileStreamWrapper (_pluginName, _prototype); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new VideoFileStreamWrapper (*this,copyop); }
    bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const VideoFileStreamWrapper*>(obj)!=NULL; }
    const char* className() const { return _prototype->className(); }
	const char* libraryName() const { return "hogboxVision"; }
	
	const std::string& GetPluginName(){return _pluginName;}
	VideoFileStream* GetPrototype(){return _prototype;}
	
protected:

	virtual ~VideoFileStreamWrapper(void)
	{
		_prototype = NULL;
	}

protected:

	VideoFileStreamPtr _prototype;	
	std::string _pluginName; //just base part at end of lib name e.g. dshow
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
		_pluginName(name),
		_prototype(proto)
	{
	}

	WebCamStreamWrapper(const WebCamStreamWrapper& wrapper,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(wrapper, copyop)
	{

	}

    virtual osg::Object* cloneType() const { return new WebCamStreamWrapper (_pluginName, _prototype); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new WebCamStreamWrapper (*this,copyop); }
    bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const WebCamStreamWrapper*>(obj)!=NULL; }
    const char* className() const { return _prototype->className(); }
	const char* libraryName() const { return "hogboxVision"; }
	
	const std::string& GetPluginName(){return _pluginName;}
	WebCamStream* GetPrototype(){return _prototype;}
	
protected:

	virtual ~WebCamStreamWrapper(void)
	{
		_prototype = NULL;
	}

protected:

	WebCamStreamPtr _prototype;
	std::string _pluginName; //just base part at end of lib name e.g. dshow
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
		_prototype(proto)
	{
	}

	CameraBaseTrackerWrapper(const CameraBaseTrackerWrapper& wrapper,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
		: osg::Object(wrapper, copyop)
	{

	}

    virtual osg::Object* cloneType() const { return new CameraBaseTrackerWrapper (_prototype); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new CameraBaseTrackerWrapper (*this,copyop); }
    bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const CameraBaseTrackerWrapper*>(obj)!=NULL; }
    const char* className() const { return _prototype->className(); }
	const char* libraryName() const { return "hogboxVision"; }
	
	CameraBasedTracker* GetPrototype(){return _prototype;}
	
protected:

	virtual ~CameraBaseTrackerWrapper(void)
	{
		_prototype = NULL;
	}

protected:

	CameraBasedTrackerPtr _prototype;	
};

typedef osg::ref_ptr<CameraBaseTrackerWrapper> CameraBaseTrackerWrapperPtr;



}; //end hogboxVision namespace
	
