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
#include <osgDB/FileNameUtils>

#include <osgUtil/IncrementalCompileOperation>

#include <hogbox/HogBoxBase.h>
#include <hogbox/Callback.h>

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
//ReadFileCallback used to read images from archive when reading from
//an osg in an archive
//
class ReadOsgImageFileFromArchiveCallback : public virtual osgDB::ReadFileCallback
{
public:
    ReadOsgImageFileFromArchiveCallback(const std::string& osgPath, osgDB::Archive* archive)
    : osgDB::ReadFileCallback(),
    _archive(archive),
    _osgPath(osgPath)
    {}
    
    virtual osgDB::ReaderWriter::ReadResult readImage(const std::string& filename, const osgDB::ReaderWriter::Options* options){
        OSG_FATAL << "READ IMAGE CALLBACK '" << filename << "', osgPath: '" << _osgPath << "'." <<std::endl;
        //
        osg::setNotifyLevel(osg::DEBUG_FP);
        if(_archive.get()){
            osgDB::ReaderWriter::ReadResult result = _archive->readImage(_osgPath+"/"+filename, options);
            osg::setNotifyLevel(osg::FATAL);
            return result;
        }
        return osgDB::ReadFileCallback::readImage(filename, options);
    }
    
protected:
    virtual ~ReadOsgImageFileFromArchiveCallback() {}
    
protected:
    osg::ref_ptr<osgDB::Archive> _archive;
    std::string _osgPath;
};
    
//
//Process a node after loading
class ProcessNodeOperation : public osg::Referenced
{
public:
    ProcessNodeOperation()
        : osg::Referenced()
    {
    }
    
    virtual bool process(osg::Node* node){
        return true;
    }
    
protected:
    virtual ~ProcessNodeOperation(){}
};
    
// 
class DatabasePagingOperation : public osg::Operation, public osgUtil::IncrementalCompileOperation::CompileCompletedCallback
{
public:
    
    DatabasePagingOperation(const std::string& filename,
                            hogbox::Callback* callback,
                            bool cache = false,
                            ProcessNodeOperation* processor = NULL,
                            osgDB::Archive* archive=NULL,
                            osgUtil::IncrementalCompileOperation* ico=NULL)
        : Operation("DatabasePaging Operation", false),
        _filename(filename),
        _modelReadyToMerge(false),
        _done(false),
        _callback(callback),
        _cache(cache),
        _processor(processor),
        _incrementalCompileOperation(ico),
        _archive(archive)
    {
    }
    
    virtual void operator () (osg::Object* object)
    {
        OSG_FATAL<<"LoadAndCompileOperation "<<_filename<<std::endl;
        
        _modelReadyToMerge = false;
        
        //read the model file, using the archive if supplied
        if(_archive.valid()){
            
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = new osgDB::ReaderWriter::Options();
            std::string folderPath = osgDB::getFilePath(_filename);
            std::string fullFolderPath = folderPath.empty() ? "/assets" : "/assets/"+folderPath;
            local_opt->setReadFileCallback(new ReadOsgImageFileFromArchiveCallback(fullFolderPath, _archive.get()));
            
            osgDB::ReaderWriter::ReadResult result = _archive->readNode("/assets/"+_filename, local_opt.get());
            _loadedModel = result.getNode();
        }else{
            _loadedModel = osgDB::readNodeFile(_filename);
        }
        
        if (_loadedModel.valid())
        {
            if(_processor.get()){
                _processor->process(_loadedModel.get());
            }
            
            if (_incrementalCompileOperation.valid())
            {
                OSG_FATAL<<"Registering with ICO "<<_filename<<std::endl;
                
                osg::ref_ptr<osgUtil::IncrementalCompileOperation::CompileSet> compileSet =
                new osgUtil::IncrementalCompileOperation::CompileSet(_loadedModel.get());
                
                compileSet->_compileCompletedCallback = this;
                
                _incrementalCompileOperation->add(compileSet.get());
            }else{
                _modelReadyToMerge = true;
            }
        }
        
        if(!_incrementalCompileOperation.get()){
            _done = true;
        }
        
        OSG_FATAL<<"done LoadAndCompileOperation "<<_filename<<std::endl;
    }
    
    virtual bool compileCompleted(osgUtil::IncrementalCompileOperation::CompileSet* compileSet)
    {
        OSG_NOTICE<<"compileCompleted"<<std::endl;
        _modelReadyToMerge = true;
        _done = true;
        return true;
    }
    
    //
    //called by assetmanager on the main thread once per frame to perform callback etc 
    //once load is compelte,
    //returns true sync is compelte and PagingOperation can be deleted
    //returns false if still loading
    bool Sync(){
        if(_modelReadyToMerge){
            if(_callback.get()){
                _callback->TriggerCallback(_loadedModel.get());
            }
            return true;
        }else if(_done){
            //should we still call the callback and just pass NULL?
            return true;
        }
        return false;
    }
    
    //
    //does the loaded model require caching
    bool CacheModel(){return _cache;}
    
    //
    //return the loaded model
    osg::Node* GetLoadedModel(){
        return _loadedModel;
    }
    
    //
    //get model ready sate
    bool ModelReady(){
        return _modelReadyToMerge;
    }
    
    //
    //is it done, i.e. can be done but the model didn't load
    //and thus isn't ready to merge
    bool Done(){
        return _done;
    }
    
protected:
    
    std::string                                         _filename;
    osg::ref_ptr<osg::Node>                             _loadedModel;
    bool                                                _modelReadyToMerge;
    bool                                                _done;
    hogbox::CallbackPtr                                 _callback;
    bool                                                _cache;
    osg::ref_ptr<osgUtil::IncrementalCompileOperation>  _incrementalCompileOperation;
    
    //optional processor applied once loaded but still in paging thread
    osg::ref_ptr<ProcessNodeOperation> _processor;
    
    //optional archive used to load assets from
    osg::ref_ptr<osgDB::Archive> _archive;
};
typedef osg::ref_ptr<DatabasePagingOperation> DatabasePagingOperationPtr;

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
    
    //
    //Call once per frame if using paging (getOrLoad with a callback)
    void Sync();
    
    //options for reading
    class ReadOptions : public osg::Referenced{
    public:
        ReadOptions()
            : osg::Referenced(),
            cache(false),
            loadCompleteCallback(NULL),
            processor(NULL)
        {
        }
        bool cache;
        hogbox::CallbackPtr loadCompleteCallback;
        ProcessNodeOperation* processor;
    };
    
    //get or load a new osg node
    osg::NodePtr GetOrLoadNode(const std::string& fileName, ReadOptions* readOptions=NULL);
    
    //load node then return a cloned version, cloning everything bar primatives, textures and arrays
    osg::Node* InstanceNode(const std::string& fileName, ReadOptions* readOptions=NULL);
    
    //
    //get or load an image then add to a texture
    osg::Tex2DPtr GetOrLoadTex2D(const std::string& fileName, ReadOptions* readOptions=NULL);
    
    //
    //get or load an image
    osg::ImagePtr GetOrLoadImage(const std::string& fileName, ReadOptions* readOptions=NULL);
    
    //
    //get or load a font
    osgText::FontPtr GetOrLoadFont(const std::string& fileName, ReadOptions* readOptions=NULL);
    
    //
    //Get or load an xml object
    XmlInputObjectPtr GetOrLoadXmlObject(const std::string& fileName, ReadOptions* readOptions=NULL);
    
    
    //
    //Get or load a shader from file
    osg::ShaderPtr GetOrLoadShader(const std::string& fileName, osg::Shader::Type shaderType, ReadOptions* readOptions=NULL);
    
    //
    //Get or load a program based on the two shaders used to create it
    osg::ProgramPtr GetOrLoadProgram(const std::string& vertShaderFile, const std::string& fragShaderFile, ReadOptions* readOptions=NULL);
    
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
    
    //the database paging thread
    osg::ref_ptr<osg::OperationThread> _databasePagingThread;
    //our array of paging operations running on the paging thread
    //need to call asset manager Sync function to handle mergeing etc
    std::vector<DatabasePagingOperationPtr> _pagingOperations;
    
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

