#include <hogbox/AssetManager.h>

#include <hogbox/SystemInfo.h>
#include <hogbox/HogBoxUtils.h>

#ifdef TARGET_OS_IPHONE
#import <Foundation/NSString.h>
#import <Foundation/NSUserDefaults.h>
#endif

using namespace hogbox;


//
//Plugin to wrap xml inputs
//
class ReaderWriterXMLObject : public osgDB::ReaderWriter
{
public:
    ReaderWriterXMLObject()
    {
        OSG_FATAL << "Construct ReaderWriterXMLObject" << std::endl;
    }
    
    virtual const char* className() const
    {
        return "XML Object wrapper Reader/Writer";
    }
    
    virtual bool acceptsExtension(const std::string& extension) const
    { 
        return osgDB::equalCaseInsensitive(extension,"xml");
    }
    
    virtual ReadResult readObject(const std::string& file,
                                  const osgDB::ReaderWriter::Options* options) const
    {
        OSG_FATAL<<"ReaderWriterXMLObject File("<<file<<")"<<std::endl;
        
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
        
        std::string fileName = file;
        
        fileName = osgDB::findDataFile( fileName, options );
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
        
        osg::ref_ptr<XmlInputObject> xmlObject = new XmlInputObject(fileName);
        return xmlObject.release();
    }
    
    virtual ReadResult readObject(std::istream& fin, const Options* options) const{
        OSG_FATAL<<"ReaderWriterXMLObject Stream()"<<std::endl;
        osg::ref_ptr<XmlInputObject> xmlObject = new XmlInputObject(fin);
        return xmlObject.release();
    }
};

// now register with Registry to instantiate the above
// reader/writer.
//REGISTER_OSGPLUGIN(xml, ReaderWriterXMLObject)



//
//
//
AssetManager::AssetManager(void)
    : osg::Referenced(),
    _archive(NULL)
{
    OSG_INFO << "Construct AssetManager" << std::endl;
    
    //hack for now we register the xml plugin here
    static osgDB::RegisterReaderWriterProxy<ReaderWriterXMLObject> g_proxy_ReaderWriterXMLObject;
    
    //create and start our database paging thread
    _databasePagingThread = new osg::OperationThread;
    _databasePagingThread->startThread();
}

AssetManager::~AssetManager(void)
{
    OSG_INFO << "Destruct AssetManager" << std::endl;
}

//
//Load an archive, if successful then all
//subsequent asset loads are loaded from the archive
//
bool AssetManager::OpenAndMountArchive(const std::string& fileName){
    _archive = osgDB::openArchive(fileName, osgDB::Archive::READ);
    if(_archive.valid()){
        /*OSG_FATAL<<"List of files in archive:"<<std::endl;
        osgDB::Archive::FileNameList fileNames;
        if (_archive->getFileNames(fileNames))
        {
            for(osgDB::Archive::FileNameList::const_iterator itr=fileNames.begin();
                itr!=fileNames.end();
                ++itr)
            {
                OSG_FATAL<<"    "<<*itr<<std::endl;
            }
        }
        
        OSG_FATAL<<std::endl;
        OSG_FATAL<<"Master file "<<_archive->getMasterFileName()<<std::endl;*/
    }
    return _archive.valid();
}

//
//Call once per frame if using paging (getOrLoad with a callback)
void AssetManager::Sync()
{
    std::vector<DatabasePagingOperationPtr>::iterator itr = _pagingOperations.begin();
    //for( ; itr!=_pagingOperations.end(); itr++){
    while(itr != _pagingOperations.end()){
        
        OSG_FATAL << "  Sync Operation" << std::endl;
        if((*itr)->Sync()){
             OSG_FATAL << "  Operation Complete" << std::endl;
            //it's done, does it require caching
            if((*itr)->CacheModel()){
                
            }
            
            //erase this operation from array
            itr = _pagingOperations.erase(itr);
            
        }else{
            itr++;
        }
    }
}

//
//get or load a new osg node
//
osg::NodePtr AssetManager::GetOrLoadNode(const std::string& fileName, ReadOptions* readOptions)
{
    osg::ref_ptr<ReadOptions> defaultOptions;
    //if read options is null allocate a default
    if(!readOptions){
        defaultOptions = new ReadOptions();
        readOptions = defaultOptions.get();
    }
    //check if name is already in the map
    if(readOptions->cache){
        FileToNodeMap::iterator found = _fileCache.find(fileName);
        if(found != _fileCache.end()){
            
            //if there is a callback trigger now as the node is already loaded
            if(readOptions->loadCompleteCallback.get()){
                readOptions->loadCompleteCallback->TriggerCallback((*found).second);
            }
            
            //return existing
            return (*found).second;
        }
    }
    
    //if there is a callback allocate a DatabasePagingOperation and add to our queue
    if(readOptions->loadCompleteCallback.get()){
        DatabasePagingOperationPtr operation = new DatabasePagingOperation(fileName,
                                                                           readOptions->loadCompleteCallback.get(),
                                                                           readOptions->cache,
                                                                           _archive.get());
        _databasePagingThread->add(operation.get());
        _pagingOperations.push_back(operation);
        return NULL;
    }
    
    //not found so load
    osg::NodePtr node = NULL;
    if(_archive.valid()){
        
        osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = new osgDB::ReaderWriter::Options();
        std::string folderPath = osgDB::getFilePath(fileName);
        std::string fullFolderPath = folderPath.empty() ? "/assets" : "/assets/"+folderPath;
        local_opt->setReadFileCallback(new ReadOsgImageFileFromArchiveCallback(fullFolderPath, _archive.get()));
        
        osgDB::ReaderWriter::ReadResult result = _archive->readNode("/assets/"+fileName, local_opt.get());
        node = result.getNode();
    }else{
        node = osgDB::readNodeFile(fileName);
    }
    
    //check we have a valid node
    if(node.get()){
        //make sure we are using vertex buffer objects
        ApplyVBOVisitor vboVisitor;
        node->accept(vboVisitor);
        
        if(readOptions->cache){
            //apply the defaults visitor
            //ApplyIOSOptVisitor visitor;
            //node->accept(visitor);
            //add to the cache
            _fileCache[fileName] = node;
        }
    }else{
        OSG_FATAL << "AssetManager::GetOrLoadNode: Failed to read file '" << fileName << "'." << std::endl;
    }

    return node;
}

//
//load node then return a cloned version, cloning everything bar primatives, textures and arrays
//
osg::Node* AssetManager::InstanceNode(const std::string& fileName, ReadOptions* readOptions)
{
    //load to cache
    osg::NodePtr node = GetOrLoadNode(fileName);
    
    //clone the original and return
    osg::Node* instance = osg::clone(node.get(), osg::CopyOp::DEEP_COPY_ALL & 
                                                ~osg::CopyOp::DEEP_COPY_PRIMITIVES & 
                                                ~osg::CopyOp::DEEP_COPY_ARRAYS &
                                                ~osg::CopyOp::DEEP_COPY_IMAGES &
                                                ~osg::CopyOp::DEEP_COPY_TEXTURES);
    return instance;
}

//
//get or load a new osg node
//
osg::Tex2DPtr AssetManager::GetOrLoadTex2D(const std::string& fileName, ReadOptions* readOptions)
{
    //check if name is already in the map
    FileToTex2DMap::iterator found = _textureCache.find(fileName);
    if(found != _textureCache.end()){
        //return existing
        return (*found).second;
    }    

    osg::Tex2DPtr tex = NULL;
    
    std::string deviceFileName = GetImagePathForDevice(fileName);
    osg::ref_ptr<osg::Image> image = this->GetOrLoadImage(fileName);
    
	if(image.valid())
	{
		image->setFileName(fileName);
		tex = new osg::Texture2D(image.get());
		tex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_LINEAR);
		tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR_MIPMAP_LINEAR);
		tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
		tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
        
        //OSG_INFO << "OsgModelCache::getOrLoadTex2D: INFO: Lodeded image file '" << deviceFileName << "'." << std::endl;
        
	}else{
        //OSG_FATAL << "OsgModelCache::getOrLoadTex2D: ERROR: Failed to read image file '" << fileName << "'." << std::endl;
        return NULL;
    }
    
    if(tex.get()){
        _textureCache[fileName] = tex;
    }
    return tex;
}

//
//get or load an image
//
osg::ImagePtr AssetManager::GetOrLoadImage(const std::string& fileName, ReadOptions* readOptions)
{
    //check if name is already in the map
    FileToImageMap::iterator found = _imageCache.find(fileName);
    if(found != _imageCache.end()){
        //return existing
        return (*found).second;
    }    
    
    std::string deviceFileName = GetImagePathForDevice(fileName);
    osg::ImagePtr image = NULL;
    
    if(_archive.valid()){
        osgDB::ReaderWriter::ReadResult result = _archive->readImage(deviceFileName);
        image = result.getImage();
    }else{
        image = osgDB::readImageFile(deviceFileName);
    }
    
    if(image.get()){
        _imageCache[fileName] = image;
        OSG_INFO << "OsgModelCache::GetOrLoadImage: INFO: Lodeded image file '" << deviceFileName << "'." << std::endl;
        
	}else{
        OSG_FATAL << "OsgModelCache::GetOrLoadImage: ERROR: Failed to read image file '" << fileName << "'." << std::endl;
    }
    return image;
    
}

//
//get or load a font
//
osgText::FontPtr AssetManager::GetOrLoadFont(const std::string& fileName, ReadOptions* readOptions)
{
    //check if name is already in the map
    FileToFontMap::iterator found = _fontCache.find(fileName);
    if(found != _fontCache.end()){
        //return existing
        return (*found).second;
    }
    
    //not found so load
    
    osg::ref_ptr<osgDB::ReaderWriter::Options> localOptions;
    localOptions = new osgDB::ReaderWriter::Options;
    localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);
    
    osg::ObjectPtr obj = NULL;
    if(_archive.valid()){
        osgDB::ReaderWriter::ReadResult result = _archive->readObject("/assets/"+fileName,localOptions);
        obj = result.getObject();
    }else{
        obj = osgDB::readObjectFile(fileName,localOptions);
    }   
    
    if(obj.get()){
        osgText::Font* font = dynamic_cast<osgText::Font*>(obj.get());
        if(font){
            OSG_FATAL << "ReadFont from archive '" << fileName << "'." << std::endl;
            _fontCache[fileName] = font;
            return font;
        }
    }
    return NULL;
}

//
//Get or load an xml object
//
XmlInputObjectPtr AssetManager::GetOrLoadXmlObject(const std::string& fileName, ReadOptions* readOptions)
{
    //check if name is already in the map
    FileToXmlObjectMap::iterator found = _xmlObjectCache.find(fileName);
    if(found != _xmlObjectCache.end()){
        //return existing
        return (*found).second;
    }
    osg::ref_ptr<osg::Object> obj = NULL;
    if(_archive.valid()){
        osgDB::ReaderWriter::ReadResult result = _archive->readObject("/assets/"+fileName);
        obj = result.getObject();
    }else{
        obj = osgDB::readObjectFile(fileName);
    }   
    
    if(obj.get()){
        XmlInputObject* xmlObject = dynamic_cast<XmlInputObject*>(obj.get());
        if(xmlObject){
            OSG_FATAL << "ReadXml from archive '" << fileName << "'." << std::endl;
            //_xmlObjectCache[fileName] = xmlObject;//caching doesn't seem to work on xml input, think node read destroys it (maybe make xmlinputobject read it and cache an xml root node)
            return xmlObject;
        }
    }else{
        OSG_FATAL << "Error Reading XmlFile '" << fileName << "'." << std::endl;
    }
    return NULL;
}

//
//Get or load a shader from file
//
osg::ShaderPtr AssetManager::GetOrLoadShader(const std::string& fileName, osg::Shader::Type shaderType, ReadOptions* readOptions)
{
    //check if name is already in the map
    FileToShaderMap::iterator found = _shaderCache.find(fileName);
    if(found != _shaderCache.end()){
        //return existing
        return (*found).second;
    }
    
    osg::ShaderPtr shader = new osg::Shader(shaderType == osg::Shader::VERTEX ? osg::Shader::VERTEX : osg::Shader::FRAGMENT);
    std::string vertFile = osgDB::findDataFile(fileName);
    
    if(shader->loadShaderSourceFromFile(vertFile)){
        _shaderCache[fileName] = shader;
        return shader;
    }
    
    return NULL;
    
    //_shaderProgram->addShader(_vertShader.get());
}

//
//Get or load a program based on the two shaders used to create it
//
osg::ProgramPtr AssetManager::GetOrLoadProgram(const std::string& vertShaderFile, const std::string& fragShaderFile, ReadOptions* readOptions)
{
    //programs are labeled using both filenames combined
    std::string programName = vertShaderFile + fragShaderFile;
    
    //check if name is already in the map
    FileToProgramMap::iterator found = _programCache.find(programName);
    if(found != _programCache.end()){
        //return existing
        return (*found).second;
    }
    
    //if we dont find it then create one by loading the shaders
    osg::ProgramPtr program = new osg::Program();
    osg::ShaderPtr vertShader = AssetManager::Inst()->GetOrLoadShader(vertShaderFile, osg::Shader::VERTEX);
    osg::ShaderPtr fragShader = AssetManager::Inst()->GetOrLoadShader(fragShaderFile, osg::Shader::FRAGMENT);
    if(vertShader.valid() && fragShader.valid()){
        program->addShader(vertShader.get());
        program->addShader(fragShader.get());
        
        _programCache[programName] = program;
        return program;
    }
    
    return NULL;
    
}

void AssetManager::ReleaseAssets()
{
    _fileCache.clear();
    _textureCache.clear();
    _fontCache.clear();
    _xmlObjectCache.clear();
    _shaderCache.clear();
    _programCache.clear();
}

//
//return true is file exists, if archive is mounted
//that is checked
//
bool AssetManager::FileExists(const std::string& file)
{
    if(_archive.valid()){
        return _archive->fileExists("/assets/"+file);
    }
    
    std::string foundPath = osgDB::findDataFile(file);
    return !foundPath.empty();
}

//
//Find the full path for a texture including localisation
//and screen size versions e.g. @2x if they exist
//
const std::string AssetManager::GetImagePathForDevice(const std::string& fileName)
{
    //new method, just see if its retina and force user to provide both types
    SystemInfo::ScreenDensity screenType = SystemInfo::Inst()->getScreenDensity();
    
    //full path to normal res
    std::string path = osgDB::getFilePath(fileName);
    std::string name = osgDB::getStrippedName(fileName);
    std::string ext = osgDB::getFileExtensionIncludingDot(fileName);
    std::string fullFileName = FindFile(fileName);
    
    if(screenType != SystemInfo::LOW_DENSITY){ //add @2x for higher resolutions
        //see if a @2x version exists        
        std::string at2X = path.empty() ? name+"@2x"+ext : path+"/"+name+"@2x"+ext;
        std::string retinaFullFileName = FindFile(at2X);
        if (!retinaFullFileName.empty()) {
            fullFileName = retinaFullFileName;
        }
    }
    
    //if fullfilename is empty now, try a localised folder
    if(fullFileName.empty()) {
#ifdef TARGET_OS_IPHONE
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
        NSString *currentLanguage = [languages objectAtIndex:0];
        
        //create localised path
        std::string localFolder = std::string([currentLanguage UTF8String])+".lproj";
        
        if(screenType != SystemInfo::LOW_DENSITY){
            fullFileName = FindFile(localFolder+"/"+name+"@2x"+ext);
        }else{
            fullFileName = FindFile(localFolder+"/"+name+ext);
        }
#endif
    }
    return fullFileName;
}

//
//return filepath if found, if not returns empty string
//
const std::string AssetManager::FindFile(const std::string& file)
{
    if(_archive.valid()){
        if(_archive->fileExists("/assets/"+file)){
            return "/assets/"+file;
        }else{
            return "";
        }
    }
    return osgDB::findDataFile(file);
}
