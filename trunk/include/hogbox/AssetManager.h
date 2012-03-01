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

#include <map>
#include <iomanip>

#include <osgDB/ReadFile>
#include <osgDB/Archive>
#include <osgDB/XmlParser>

#include <hogbox/HogBoxBase.h>

#ifdef ANDROID
#include <jni.h>
#endif

namespace hogbox {
    

//create osg::Object wrapper around xmlinput to allow loading through plugin system
class XmlInputObject : public osg::Object 
{
public:
    XmlInputObject() 
    : osg::Object()
    {}

    XmlInputObject(const std::string& fileName) 
    : osg::Object()
    {
        _input.open(fileName);
        _input.readAllDataIntoBuffer();
    }
    
    XmlInputObject(std::istream& fin) 
    : osg::Object()
    {
        _input.attach(fin);
        _input.readAllDataIntoBuffer();
    }
    
    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	XmlInputObject(const XmlInputObject& object,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
    : osg::Object(object, copyop)
    {
    }

    META_Object(osg, XmlInputObject);
    
    osgDB::XmlNode::Input& GetInput(){
        return _input;
    }
    
protected:
    virtual ~XmlInputObject(){
    }
    
protected:
    osgDB::XmlNode::Input _input;
};

typedef osg::ref_ptr<XmlInputObject> XmlInputObjectPtr;


//
//Handle reading and writing of files
//
class AssetManager : public osg::Referenced
{
public:
    
    static AssetManager* Inst(){
        static osg::ref_ptr<AssetManager> g_AssetManagerInstance = new AssetManager();
        return g_AssetManagerInstance.get();
    }
    
    //
    //Load an archive, if successful then all
    //subsequent asset loads are loaded from the archive
    bool OpenAndMountArchive(const std::string& fileName);
    
    //get or load a new osg node
    osg::NodePtr GetOrLoadNode(std::string fileName);
    
    //load node then return a cloned version, cloning everything bar primatives, textures and arrays
    osg::Node* InstanceNode(std::string fileName);
    
    //
    //get or load an image then add to a texture
    osg::Tex2DPtr GetOrLoadTex2D(std::string fileName);
    
    //
    //get or load an image
    osg::ImagePtr GetOrLoadImage(std::string fileName);
    
    //
    //get or load a font
    osgText::FontPtr GetOrLoadFont(std::string fileName);
    
    //
    //Get or load an xml object
    XmlInputObjectPtr GetOrLoadXmlObject(std::string fileName);
    
    
    //
    //Get or load a shader from file
    osg::ShaderPtr GetOrLoadShader(std::string fileName, osg::Shader::Type shaderType);
    
    //
    //Get or load a program based on the two shaders used to create it
    osg::ProgramPtr GetOrLoadProgram(std::string vertShaderFile, std::string fragShaderFile);
    
    //release all objects from the cache
    void ReleaseAssets();
    
    //directory helpers
    
    //
    //return true is file exists, if archive is mounted that is checked
    bool FileExists(const std::string& file);

    //
    //return filepath if found, if not returns empty string
    const std::string FindFile(const std::string& file);
    
    //
    //Find the full path for a texture including localisation
    //and screen size versions e.g. @2x if they exist
    const std::string GetImagePathForDevice(const std::string& fileName);
    
    
    //Android asset managment 
#ifdef ANDROID
    void setAndroidAssetEnv(JNIEnv* env, jobject androidAssetManger){
        g_androidAssetManger = androidAssetManger;
        g_androidEnv = env;
    }
    
    JNIEnv* getAndroidEnv(){return g_androidEnv;}
    jobject getAndroidAssetManger(){return g_androidAssetManger;}
#endif
    
    

    
protected:
    AssetManager(void);
    virtual ~AssetManager(void);
    
protected:
    
    //optional archive used to load assets from
    osg::ref_ptr<osgDB::Archive> _archive;
    
    //the map of file names to cached osg nodes
    typedef std::pair<std::string, osg::NodePtr> FileToNodePair;
    typedef std::map<std::string, osg::NodePtr> FileToNodeMap;
    
    FileToNodeMap _fileCache;
    
    typedef std::pair<std::string, osg::Tex2DPtr> FileToTex2DPair;
    typedef std::map<std::string, osg::Tex2DPtr> FileToTex2DMap;
    
    FileToTex2DMap _textureCache;
    
    typedef std::pair<std::string, osg::ImagePtr> FileToImagePair;
    typedef std::map<std::string, osg::ImagePtr> FileToImageMap;
    
    FileToImageMap _imageCache;
    
    typedef std::pair<std::string, osgText::FontPtr> FileToFontPair;
    typedef std::map<std::string, osgText::FontPtr> FileToFontMap;
    
    FileToFontMap _fontCache;
    
    typedef std::pair<std::string, XmlInputObjectPtr> FileToXmlObjectPair;
    typedef std::map<std::string, XmlInputObjectPtr> FileToXmlObjectMap;
    
    FileToXmlObjectMap _xmlObjectCache;

    typedef std::pair<std::string, osg::ShaderPtr> FileToShaderPair;
    typedef std::map<std::string, osg::ShaderPtr> FileToShaderMap;
    
    FileToShaderMap _shaderCache;
    
    typedef std::pair<std::string, osg::ProgramPtr> FileToProgramPair;
    typedef std::map<std::string, osg::ProgramPtr> FileToProgramMap;
    
    FileToProgramMap _programCache;
    
    
    //Android asset managment 
#ifdef ANDROID
    jobject g_androidAssetManger;
    JNIEnv* g_androidEnv;
#endif

};
    
}

