#include <hogbox/HogBoxUtils.h>

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>

#ifndef MAX_PATH
	#define MAX_PATH 512
#endif

//Search all nodes in the tree
//
osg::Node* hogbox::FindNodeByName(osg::Node* node, const std::string name)
{

	if(node->getName().compare(name) == 0)
	{
		//CMsgLog::Inst()->WriteToLog("found node");
		return node;
	}else{
		//CMsgLog::Inst()->WriteToLog("Check Children of "+node->getName());
	}

	// Traverse any group node
	if(dynamic_cast<osg::Group*> (node))
	{
		osg::Group* group = static_cast<osg::Group*> (node);

		osg::Node* found = NULL;
		for(unsigned int i=0; i < group->getNumChildren(); i++)
		{
			found =  FindNodeByName(group->getChild(i), name);
			if(found!=NULL)
			{return found;}
        }

	}

	return NULL;
}


osg::Image* hogbox::CreateSubImage(int sCol, int sRow, int width, int height, osg::Image* source)
{
	//check source is safe
	if(source==NULL)
	{return NULL;}

	//get the number of elements in a pixel
	int px = source->getPixelSizeInBits()/8;

	//create the pixel buufer for the new image
	unsigned char* subBuf = new unsigned char[(width*height)*px];

	int eCol = sCol+width;
	int eRow = sRow+height;

	//keep trag of the current index into the new buffer
	int index=0;

	//loop all the rows  or y dir of the source image
	for(int row=sRow; row<eRow; row++)
	{
		//loop the cols or x dir
		for(int col=sCol; col<eCol; col++)
		{

			//get the pixel at the curretn pixel coord
			unsigned char* pixel = source->data(col, row, 0);
			//loop the elements of the pixel and add to the ne buffer
			for(int p=0; p<px; p++)
			{
				subBuf[index] = pixel[p];
				index++;
			}
		}
	}

	//create the new image
	osg::Image* subImage = new osg::Image();

	subImage->allocateImage(width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, 1);
	subImage->setImage( width,height ,1, GL_RGB, GL_RGB,
						GL_UNSIGNED_BYTE, subBuf, osg::Image::NO_DELETE, 1);

	return subImage;
}

//
// get the width in pixels of the passed image
//
int hogbox::GetImageWidth(osg::Image* image)
{
	int px = image->getPixelSizeInBits()/8;
	//get the dimensions of each sub images
	int width = image->getRowSizeInBytes()/px;

	return width;
}

//
// get the height in pixels of the current image
//
int hogbox::GetImageHeight(osg::Image* image)
{
	int height = image->getImageSizeInBytes()/image->getRowSizeInBytes();
	return height;
}

//
// Load an image file and apply to a new texture 2d
//
osg::Image* hogbox::LoadImage2D(std::string name)
{
	osg::ref_ptr<osg::Image> image = NULL;
	image = osgDB::readImageFile(name);
	if(image == NULL)
	{
//		CMsgLog::Inst()->WriteToLog( "Error could not load image file " + name );
	}else{
		image->setFileName(name);
		return image.get();
	}

	return NULL;
}

//
// Load an image file and apply to a new texture 2d
//
osg::Texture2D* hogbox::LoadTexture2D(const std::string& fileName)
{
	osg::Image* image = osgDB::readImageFile(fileName);

	if(image == NULL)
	{
//		cout<< "Error could not load image file " << name << endl;
	}else{
		image->setFileName(fileName);

		osg::Texture2D* tex;
		tex = new osg::Texture2D();
		tex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_LINEAR);
		tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR_MIPMAP_LINEAR);
		tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
		tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
		tex->setImage(image);
		//m_imageDataBase.push_back(image);
		return tex;
	}

	return NULL;
}

//
// Loads a cube map cross file and splits it into the 6 images required
// then loads into a texture object and returns
//
osg::TextureCubeMap* hogbox::LoadTextureCubeCross(std::string file)
{
	osg::Image* image;
	image = osgDB::readImageFile(file);

	if(image==NULL)
	{
//		cout<< "Error could not load cube cross image file " << file << endl;
	}else{

		//get the dimensions of each sub images
		int width = hogbox::GetImageWidth(image);
		int height = hogbox::GetImageHeight(image);

		int subW = width/3;
		int subH = height/4;

		osg::Image* negZ = hogbox::CreateSubImage(subW, 0, subW,subH, image);
		negZ->flipHorizontal();

		osg::Image* negY = hogbox::CreateSubImage(subW, subH, subW,subH, image);
		negY->flipVertical();

		osg::Image* negX = hogbox::CreateSubImage(0, subH*2, subW,subH, image);
		negX->flipVertical();

		osg::Image* posZ = hogbox::CreateSubImage(subW, subH*2, subW,subH, image);
		posZ->flipVertical();

		osg::Image* posX = hogbox::CreateSubImage(subW*2, subH*2, subW,subH, image);
		posX->flipVertical();

		osg::Image* posY = hogbox::CreateSubImage(subW, subH*3, subW,subH, image);
		posY->flipVertical();


	/*	osgDB::writeImageFile(*posX, "c:/posX.bmp");
		osgDB::writeImageFile(*posY, "c:/posY.bmp");
		osgDB::writeImageFile(*posZ, "c:/posZ.bmp");
		osgDB::writeImageFile(*negX, "c:/negX.bmp");
		osgDB::writeImageFile(*negY, "c:/negY.bmp");
		osgDB::writeImageFile(*negZ, "c:/negZ.bmp"); */

		//create the cube map
		osg::TextureCubeMap* cubeMap = new osg::TextureCubeMap;

		cubeMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		cubeMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		cubeMap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
		cubeMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
		cubeMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);


		// assign the six images to the texture object
		cubeMap->setImage(osg::TextureCubeMap::POSITIVE_X, posX);
		cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_X, negX);
		cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Y, posY);
		cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Y, negY);
		cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Z, posZ);
		cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Z, negZ);

		return cubeMap;
	}

	return NULL;
}

#ifndef WIN32 //we need to import the Cocoa objective c funcs from the windowsCompact.mm
bool GetOpenFilenameOSX(const char *extensions, const char *defaultFolder, char *selectedFilename){return true;}
#endif

//
// returns a file path from the open file common dialog box
//
const std::string hogbox::OpenPlatformFileDialog(const char* exts, const char* defaultExt, const char* defaultFolder)
{
	char szFileName[MAX_PATH]="";
#ifdef WIN32

	OPENFILENAME ofn;
	
	ZeroMemory(&ofn, sizeof(ofn));
	
	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = GetActiveWindow();
	ofn.nFileOffset=1;
	ofn.Flags=OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrFilter = (LPSTR)exts;//"Text Files (*.mp3)\0*.mp3\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = (LPSTR)szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = (LPCSTR)"mp3";
	
	if(GetOpenFileName(&ofn) == 0)
	{
		return NULL;
	}
	
	//char* v = new char[MAX_PATH];
	//sprintf(v,"%s", ofn.lpstrFile);
	return std::string(ofn.lpstrFile);//ofn.lpstrFile; 
	
	//return file;//ofn.lpstrFile;
#else
	if (GetOpenFilenameOSX(exts, defaultFolder, szFileName))
	{
		char* v = new char[MAX_PATH];
		sprintf(v,"%s",szFileName);
		return v;//ofn.lpstrFile; 		
	}
	
	return NULL;
#endif
}

#ifndef WIN32
bool GetSaveFileNameOSX(const char* exts, const char* defaultDirectory, char* saveFileName){return true;}
#endif

//
// open a save file dialog and return the selected file path
//
char* hogbox::OpenSaveFileDialog(const char* exts, char* defaultFolder)
{
#ifdef WIN32
	OPENFILENAME ofn;
	char szFileName[MAX_PATH]="";
	
	ZeroMemory(&ofn, sizeof(ofn));
	
	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = GetActiveWindow();
	ofn.nFileOffset=1;
	ofn.lpstrFilter = (LPCSTR)exts;//"Text Files (*.mp3)\0*.mp3\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = (LPSTR)szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = (LPCSTR)"hog";
	
	if(GetSaveFileName(&ofn))
	{
		
	}
	char* v = new char[MAX_PATH];
	sprintf(v,"%s", ofn.lpstrFile);
	return v;//ofn.lpstrFile;
	//LEAKING
#else
	char szFileName[MAX_PATH]="";
	if(GetSaveFileNameOSX(exts, defaultFolder, szFileName))
	{
		char* v = new char[MAX_PATH];
		sprintf(v,"%s",szFileName);
		return v;//ofn.lpstrFile; 
	}
	return NULL;
	
#endif
	//return file;//ofn.lpstrFile;
}

//
//Copy file from to
//
bool hogbox::CopyFileHB(std::string from, std::string to)
{
#ifdef WIN32
	CopyFile(from.c_str(), to.c_str(), FALSE);
	return false;
#else
	//printf("Copy from %s to %s\n", from.c_str(), to.c_str());
	//Copy(from.c_str(), to.c_str());
	std::string op = "cp \"" + from + "\" \"" + to + "\"";
	printf("%s\n", op.c_str());
	system(op.c_str());
	return true;
#endif
}


//
//Set the current system directory
//
void hogbox::SetWorkingDirectory(const char* dir)
{
#ifdef WIN32
	SetCurrentDirectory(dir);
#else
	chdir(dir);
#endif
}

//
//Get the current system directory
//
void hogbox::GetWorkingDirectory(int length, char* retDir)
{
#ifdef WIN32
	DWORD dw = static_cast<DWORD>(length);
	GetCurrentDirectory(dw, retDir);
#else//think the below works on win32 but for now we know the above is fine
	getcwd(retDir, length);
#endif
}

//we need to import the Cocoa objective c funcs from the windowsCompact.mm
#ifndef WIN32
void MessageBoxOSX(const char *title, const char *message){}
#endif

void hogbox::MsgBox(const std::string title, const std::string message, int mode)
{
#ifdef WIN32
	MessageBox (HWND_DESKTOP, message.c_str(), title.c_str(), MB_OK | MB_ICONEXCLAMATION);
#else
	MessageBoxOSX(title.c_str(), message.c_str());
#endif
}

#ifndef WIN32
int MsgBoxYesNoOSX(const char* title, const char* message){}
#endif

//
//Show an OS Yes no based message box
// returns -1 if cancel, 0 if no, 1 if yess
//
int hogbox::MsgBoxYesNo(std::string title, std::string msg)
{
#ifdef WIN32 
	int ret = MessageBox (HWND_DESKTOP, msg.c_str(), title.c_str(), MB_YESNO | MB_ICONEXCLAMATION);
	if(ret == IDYES)
	{return 1;}
	return 0;
#else
	return MsgBoxYesNoOSX(title.c_str(), msg.c_str());
#endif
}



//
//
//
//
//returns a texture2d loaded from a openfile dialog path
//
osg::Texture2D* hogbox::LoadTexture2DDialog()
{
	char* appRoot = new char[MAX_PATH];
	//accuire the apps path
	GetWorkingDirectory(MAX_PATH, appRoot);
	
	osg::Image* image = NULL;//osgDB::readImageFile(OpenPlatformFileDialog(OPEN_IMAGE_STRING_WIN, "All"));
	if(image==NULL)
	{
//	cout<< "Error could not load image file from dialog" << endl;
	}else{
		osg::Texture2D* tex = new osg::Texture2D();
		tex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
		tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
		tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
		tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
		tex->setImage(image);
		return tex;
	}
	
	return NULL;
}

//
// returns a requested image if the load is successful
//
osg::Image* hogbox::LoadImageDialog()
{
	char* appRoot = new char[MAX_PATH];
	//accuire the apps path
	GetWorkingDirectory(MAX_PATH, appRoot);
	
	osg::Image* image = NULL;//osgDB::readImageFile(OpenPlatformFileDialog(OPEN_IMAGE_STRING_WIN, "All"));
	
	SetWorkingDirectory(appRoot);
	delete [] appRoot;
	
	return image;
}


//
//
//
bool hogbox::RelocateTextureImage(std::string savePath, osg::Image* image, bool local)
{
	if( (image == NULL) )
	{return false;}
	//osg::Image* scaledImg = hogbox::CreateSubImage(0,0, GetImageWidth(image), GetImageHeight(image), image);

	//scaledImg->scaleImage(
	std::string file = osgDB::getSimpleFileName(image->getFileName());
	//osgDB::writeImageFile(*image, savePath+file);
	hogbox::CopyFileHB(image->getFileName().c_str(), std::string(savePath+file).c_str());

	if(local)
	{image->setFileName("images/"+file);
	}else{
		image->setFileName(savePath+file);
	}

	return true;
}

//
//
//
std::string hogbox::ChangeFileFolder(std::string filePath, std::string newFolder)
{
	//get the file name
	std::string fileName = osgDB::getSimpleFileName(filePath);

	return (newFolder+fileName);
}

//
// iterate through the sub nodes and rename any geodes to name string
//
void hogbox::RenameGeodes(osg::Node* node, const std::string name)
{
	//find geomtry in geodes
	if(dynamic_cast<osg::Geode*> (node))
	{
		//loop all geometry nodes
		osg::Geode* geode = static_cast<osg::Geode*> (node);
		geode->setName(name) ;
	}

	// Traverse any group node
	if(dynamic_cast<osg::Group*> (node))
	{
		osg::Group* group = static_cast<osg::Group*> (node);
		for(unsigned int i=0; i < group->getNumChildren(); i++)
		{	RenameGeodes(group->getChild(i), name);}
	}
}


//
// Procedural texture generation helper funcs
//
osg::Image*  hogbox::make3DNoiseImage(int texSize)
{
    osg::Image* image = new osg::Image;
    image->setImage(texSize, texSize, texSize,
            4, GL_RGBA, GL_UNSIGNED_BYTE,
            new unsigned char[4 * texSize * texSize * texSize],
            osg::Image::USE_NEW_DELETE);

    const int startFrequency = 4;
    const int numOctaves = 4;

    int f, i, j, k, inc;
    double ni[3];
    double inci, incj, inck;
    int frequency = startFrequency;
    GLubyte *ptr;
    double amp = 0.5;

    osg::notify(osg::INFO) << "creating 3D noise texture... ";

    for (f = 0, inc = 0; f < numOctaves; ++f, frequency *= 2, ++inc, amp *= 0.5)
    {
        SetNoiseFrequency(frequency);
        ptr = image->data();
        ni[0] = ni[1] = ni[2] = 0;

        inci = 1.0 / (texSize / frequency);
        for (i = 0; i < texSize; ++i, ni[0] += inci)
        {
            incj = 1.0 / (texSize / frequency);
            for (j = 0; j < texSize; ++j, ni[1] += incj)
            {
                inck = 1.0 / (texSize / frequency);
                for (k = 0; k < texSize; ++k, ni[2] += inck, ptr += 4)
                {
                    *(ptr+inc) = (GLubyte) (((noise3(ni) + 1.0) * amp) * 128.0);
                }
            }
        }
    }

    osg::notify(osg::INFO) << "DONE" << std::endl;
    return image;
}

osg::Texture3D*  hogbox::make3DNoiseTexture(int texSize )
{
    osg::Texture3D* noiseTexture = new osg::Texture3D;
    noiseTexture->setFilter(osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR);
    noiseTexture->setFilter(osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR);
    noiseTexture->setWrap(osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT);
    noiseTexture->setWrap(osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT);
    noiseTexture->setWrap(osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT);
    noiseTexture->setImage( make3DNoiseImage(texSize) );
    return noiseTexture;
}

///////////////////////////////////////////////////////////////////////////

osg::Image*  hogbox::make1DSineImage( int texSize )
{
    const float PI = 3.1415927;

    osg::Image* image = new osg::Image;
    image->setImage(texSize, 1, 1,
            4, GL_RGBA, GL_UNSIGNED_BYTE,
            new unsigned char[4 * texSize],
            osg::Image::USE_NEW_DELETE);

    GLubyte* ptr = image->data();
    float inc = 2. * PI / (float)texSize;
    for(int i = 0; i < texSize; i++)
    {
        *ptr++ = (GLubyte)((sinf(i * inc) * 0.5 + 0.5) * 255.);
        *ptr++ = 0;
        *ptr++ = 0;
        *ptr++ = 1;
    }
    return image;
}

osg::Texture1D*  hogbox::make1DSineTexture( int texSize )
{
    osg::Texture1D* sineTexture = new osg::Texture1D;
    sineTexture->setWrap(osg::Texture1D::WRAP_S, osg::Texture1D::REPEAT);
    sineTexture->setFilter(osg::Texture1D::MIN_FILTER, osg::Texture1D::LINEAR);
    sineTexture->setFilter(osg::Texture1D::MAG_FILTER, osg::Texture1D::LINEAR);
    sineTexture->setImage( make1DSineImage(texSize) );
    return sineTexture;
}

//GEOM HELPERS

//
// Bulid a rectangular geometry of size using color
// this layer is then altered in runtime using baselayer functions
//
osg::Geometry* hogbox::BuildXYQuad(osg::Vec2 size, osg::Vec4 color, float depth)
{
	osg::Geometry* m_baseLayer = new osg::Geometry();
	osg::Vec3Array* vertices = new osg::Vec3Array;
	vertices->push_back(osg::Vec3(0.0f,		0.0f,		depth));
	vertices->push_back(osg::Vec3(size.x(),	0.0f,		depth));
	vertices->push_back(osg::Vec3(size.x(),	size.y(),	depth));
	vertices->push_back(osg::Vec3(0.0f,		size.y(),	depth));
	m_baseLayer->setVertexArray(vertices);

	osg::Vec3Array* normals = new osg::Vec3Array;
	normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
	m_baseLayer->setNormalArray(normals);
	m_baseLayer->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::Vec4Array* m_baseColors = new osg::Vec4Array;
	m_baseColors->push_back(osg::Vec4(1,1,1,1));
	m_baseLayer->setColorArray(m_baseColors);
	m_baseLayer->setColorBinding(osg::Geometry::BIND_OVERALL);

	osg::Vec2Array* texCoords = new osg::Vec2Array;
	texCoords->push_back(osg::Vec2(0.0f,0.0f));
	texCoords->push_back(osg::Vec2(1.0f,0.0f));
	texCoords->push_back(osg::Vec2(1.0f,1.0f));
	texCoords->push_back(osg::Vec2(0.0f,1.0f));
	m_baseLayer->setTexCoordArray(0, texCoords);

	m_baseLayer->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	return m_baseLayer;
}
/*
//
// Load a video stream and applt to a texture
// then bind texture and offset texture matrix to passed stateset
// Currently not included as only windows is implemented
CVideoStream* hogbox::ApplyVideoTextureToState(osg::StateSet* state, std::string file, int channel )
{
	bool isRect = SystemInfo::Inst()->textureRectangleSupported();
	
	//first of all open the file
	osg::ref_ptr<CVideoStream> videoStream = NULL;

	//select the correct video for platform
#ifdef WIN32
	videoStream = new CVideoStream(false);//CMJVidStream(file.c_str(), isRect);
#else

#endif

	//check it worked
	if(!videoStream->isValid())
	{return NULL;}

	osg::ref_ptr< osg::Texture > videoTexture = NULL;
	if(isRect)
	{
		videoTexture = new osg::TextureRectangle(videoStream.get());
	}else{
		videoTexture = new osg::Texture2D(videoStream.get());
	}

	videoTexture->setDataVariance(osg::Object::DYNAMIC);
	videoTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	videoTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	videoTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
	videoTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);
	videoTexture->setResizeNonPowerOfTwoHint(false);

	osg::TexMat* texMat = NULL;
	//if 2d add the sub load callback to resize to power of two
	if(!isRect)
	{
		VideoTex2DCallBack* texCB = new VideoTex2DCallBack(dynamic_cast<osg::Texture2D*> (videoTexture.get()));
		dynamic_cast<osg::Texture2D*> (videoTexture.get())->setSubloadCallback(texCB); 

		//apply the crop scaling to the streams shift matrix
		videoStream->SetEdgeOffSet(texCB->getScaledTexCoordX(), texCB->getScaledTexCoordY());
		texMat = (osg::TexMat*) videoStream->getTexMat();
	}else{//texture rect
		texMat = new osg::TexMat();
		texMat->setScaleByTextureRectangleSize(true);
	}

	//apply the texture to the quads stateset
 	state->setTextureAttributeAndModes(channel, videoTexture.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);



	return videoStream.get();

}

//VIDEO HELPERS


//
//Create a Texture with the video stream
//apply the tex to the passed state
//apply any texture matrices for coord scaling
//
void hogbox::ApplyVideoTextureToState(osg::StateSet* state, CVideoStream* videoStream, int channel)
{
	//check it worked
	if(!videoStream->isValid())
	{return;}

	bool isRect = SystemInfo::Inst()->textureRectangleSupported();
	
	osg::ref_ptr<osg::Texture> videoTexture = NULL; 
	if(isRect)
	{
		videoTexture = new osg::TextureRectangle(videoStream);
	}else{
		videoTexture = new osg::Texture2D(videoStream);
	}

	//set the texture state
	videoTexture->setDataVariance(osg::Object::DYNAMIC);
	videoTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	videoTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	videoTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
	videoTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);
	videoTexture->setResizeNonPowerOfTwoHint(false);

	osg::TexMat* texMat = NULL;
	//if 2d add the sub load callback to resize to power of two
	if(!isRect)
	{
		VideoTex2DCallBack* texCB = new VideoTex2DCallBack(dynamic_cast<osg::Texture2D*> (videoTexture.get()));
		dynamic_cast<osg::Texture2D*> (videoTexture.get())->setSubloadCallback(texCB); 

		//apply the crop scaling to the streams shift matrix
		videoStream->SetEdgeOffSet(texCB->getScaledTexCoordX(), texCB->getScaledTexCoordY());
		texMat = (osg::TexMat*) videoStream->getTexMat();
	}else{//texture rect
		texMat = new osg::TexMat();
		texMat->setScaleByTextureRectangleSize(true);
	}

	//apply the texture to the quads stateset
 	state->setTextureAttributeAndModes(channel, videoTexture.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	
	//apply the tex matrix for the image, if it has one
	if (texMat)
	{   
		state->setTextureAttributeAndModes(channel, texMat, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 
	}
}

osg::Texture2D* hogbox::CreateVideoTexture2D(CVideoStream* videoStream)
{
	//check it worked
	if(!videoStream->isValid())
	{return NULL;}

	bool isRect = SystemInfo::Inst()->textureRectangleSupported();
	
	osg::Texture2D* videoTexture = NULL; 
	videoTexture = new osg::Texture2D(videoStream);
	//set the texture state
	videoTexture->setDataVariance(osg::Object::DYNAMIC);
	videoTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	videoTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	videoTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
	videoTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);
	//videoTexture->setResizeNonPowerOfTwoHint(false);

	return videoTexture;
}

osg::TextureRectangle* hogbox::CreateVideoTextureRect(CVideoStream* videoStream)
{
	//check it worked
	if(!videoStream)
	{return NULL;}

	bool isRect = SystemInfo::Inst()->textureRectangleSupported();
	
	osg::TextureRectangle* videoTexture = NULL; 
	videoTexture = new osg::TextureRectangle(videoStream);
	//set the texture state
	videoTexture->setDataVariance(osg::Object::DYNAMIC);
	videoTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	videoTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	videoTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
	videoTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);
	videoTexture->setResizeNonPowerOfTwoHint(false);
	
	return videoTexture;
}
*/

//
//
//
//
// return angle between in degrees
//
double hogbox::CalcAngleBetweenVectors(osg::Vec3 u, osg::Vec3 v)
{
	/*
	//dot our two vecs and convert based on direction
	double dot = u * v;

	//scale to 90
	return osg::DegreesToRadians(90.0f * (1-dot));
	*/
	double ang=0.0f;

	double uDotv = u * v;

	double uL = u.length();
	double vL = v.length();

	if(uDotv == 0.0f)
	{
		ang = osg::DegreesToRadians(90.0);
	}else if( uDotv == 1.0f){
		ang = 0.0f;
	}else if(uDotv == -1.0f){
		ang = osg::DegreesToRadians(180.0);
	}else{
		ang = acos(uDotv/uL*vL);
	}
	return osg::RadiansToDegrees(ang);

	/*	double angle = (u * v ) / ( (u.length()) * (v.length()) );
	angle = acos(angle);
	angle = osg::RadiansToDegrees(angle);
	return angle;*/
}



