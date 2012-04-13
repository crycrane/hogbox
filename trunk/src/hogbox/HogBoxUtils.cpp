#include <hogbox/HogBoxUtils.h>

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>

#ifdef WIN32
#include <windows.h>
#endif

#ifndef MAX_PATH
	#define MAX_PATH 512
#endif

//
//Camera helpers
//

osg::Matrix hogbox::ComputeLookAtMatrixForNodeBounds(osg::Node* scene, float distance, osg::Vec3 forwardAxis, osg::Vec3 upAxis)
{
    osg::Vec3 camPos, lookAt, up;
    if(!hogbox::ComputeLookAtVectorsForNodeBounds(scene, distance, forwardAxis, upAxis, 
                                                  camPos, lookAt, up)){
        return osg::Matrix::identity();
    }
    return osg::Matrix::lookAt(camPos, lookAt, up);
}

bool hogbox::ComputeLookAtVectorsForNodeBounds(osg::Node* scene, float distance, osg::Vec3 forwardAxis, osg::Vec3 upAxis,
                                               osg::Vec3& outCamPos, osg::Vec3& outLookAt, osg::Vec3& outUpAxis)
{
    if(!scene){
        return false;
    }
    forwardAxis.normalize();
    osg::BoundingSphere bounds = scene->computeBound();
    osg::Vec3 lookAt = bounds.center();
    //float radius = bounds.radius() >= 1.0f ? bounds.radius() : 1.0f;
    //OSG_FATAL << "ComputeLookAtMatrixForNodeBounds: radius " << radius << std::endl;
    osg::Vec3 camPos = lookAt + (-forwardAxis * (bounds.radius()*distance));
    osg::Vec3 up = upAxis;
    up.normalize();
    
    outCamPos = camPos;
    outLookAt = lookAt;
    outUpAxis = up;
    return true;
}

osg::Geometry* hogbox::createTexturedQuadGeometry(const float& width,const float& height, QuadArgs args)
{
    //create the default in XY plane with origin in bottom left
    osg::Vec3 corner = osg::Vec3(0.0f,0.0f,0.0f);
    osg::Vec3 widthVec = osg::Vec3(1.0f,0.0f,0.0f);
    osg::Vec3 heightVec = osg::Vec3(0.0f,1.0f,0.0f);
    float temp;
    
    switch(args._originType){
        case hogbox::QuadArgs::ORI_BOTTOM_LEFT:
            break;
        case hogbox::QuadArgs::ORI_TOP_LEFT:
            heightVec *= -1.0f;
            break;
        case hogbox::QuadArgs::ORI_CENTER:
            corner = osg::Vec3(-0.5f,-0.5f,0.0f);
            break;
        default:break;
    }
    
    //flip plane
    switch(args._planeType){
        case hogbox::QuadArgs::PLANE_XY:
            break;
        case hogbox::QuadArgs::PLANE_XZ:
            //flip y and z
            temp = corner.y();
            corner.y() = corner.z();
            corner.z() = temp;
            
            temp = widthVec.y();
            widthVec.y() = widthVec.z();
            widthVec.z() = temp;
            
            temp = heightVec.y();
            heightVec.y() = heightVec.z();
            heightVec.z() = temp;
            break;
        default:break;
    }
    
    float l = args._corners[0]._texCoord.x();
    float b = args._corners[0]._texCoord.y();
    float r = args._corners[3]._texCoord.x();
    float t = args._corners[3]._texCoord.y();
    
    if(args._corners[0]._radius == 0.0f && args._corners[1]._radius == 0.0f && args._corners[2]._radius == 0.0f && args._corners[3]._radius == 0.0f){
        return osg::createTexturedQuadGeometry(corner, widthVec, heightVec, l,b,r,t); 
    }
    
    //
    osg::Vec3 heightDir = heightVec;
    heightDir.normalize();
    
    osg::Vec3 widthDir = widthVec;
    widthDir.normalize();
    
    osg::Geometry* geom = new osg::Geometry();
    
    osg::Vec3Array* coords = new osg::Vec3Array();
    
    //first vec at center
    osg::Vec3 center = corner+(heightVec*0.5f)+(widthVec*0.5f);
    coords->push_back(center);
    
    //move to bottom left corner
    if(args._corners[0]._radius != 0.0f){
        
        osg::Vec3 arcPos = corner + (heightDir*args._corners[0]._radius);
        coords->push_back(arcPos);

        osg::Vec3 arcPivot = arcPos + (widthDir*args._corners[0]._radius);
        
        //loop and draw segments for rounded corner around arcPivot
        unsigned int segments = args._corners[0]._segments;
        for(unsigned int i=0; i<segments; i++){
            float pc = (float)(i+1)/(float)segments;
            //get radians for segment
            float rad = osg::DegreesToRadians(180.0f+(90.0f*pc));
            float c = cosf(rad);
            float s = sinf(rad);
            
            osg::Vec3 localArcPos = osg::Vec3(c,s,0.0f) * args._corners[0]._radius;
            arcPos = arcPivot + localArcPos;
            coords->push_back(arcPos);
        }
        
    }else{
        coords->push_back(corner);
    }
    
    //move to bottom right corner
    if(args._corners[1]._radius != 0.0f){
        
        osg::Vec3 arcPos = (corner+widthVec) + -(widthDir*args._corners[1]._radius);
        coords->push_back(arcPos);
        
        osg::Vec3 arcPivot = arcPos + (heightDir*args._corners[1]._radius);
        
        //loop and draw segments for rounded corner around arcPivot
        unsigned int segments = args._corners[1]._segments;
        for(unsigned int i=0; i<segments; i++){
            float pc = (float)(i+1)/(float)segments;
            //get radians for segment
            float rad = osg::DegreesToRadians(270.0f+(90.0f*pc));
            float c = cosf(rad);
            float s = sinf(rad);
            
            osg::Vec3 localArcPos = osg::Vec3(c,s,0.0f) * args._corners[1]._radius;
            arcPos = arcPivot + localArcPos;
            coords->push_back(arcPos);
        }
        
    }else{
        coords->push_back(corner+widthVec);
    }
    
    //move to top right corner
    if(args._corners[3]._radius != 0.0f){
        
        osg::Vec3 arcPos = (corner+widthVec+heightVec) + -(heightDir*args._corners[3]._radius);
        coords->push_back(arcPos);
        
        osg::Vec3 arcPivot = arcPos + (-widthDir*args._corners[3]._radius);
        
        //loop and draw segments for rounded corner around arcPivot
        unsigned int segments = args._corners[3]._segments;
        for(unsigned int i=0; i<segments; i++){
            float pc = (float)(i+1)/(float)segments;
            //get radians for segment
            float rad = osg::DegreesToRadians((90.0f*pc));
            float c = cosf(rad);
            float s = sinf(rad);
            
            osg::Vec3 localArcPos = osg::Vec3(c,s,0.0f) * args._corners[3]._radius;
            arcPos = arcPivot + localArcPos;
            coords->push_back(arcPos);
        }
        
    }else{
        coords->push_back(corner+widthVec+heightVec);
    }
    
    //move to top left
    if(args._corners[2]._radius != 0.0f){
        
        osg::Vec3 arcPos = (corner+heightVec) + (widthDir*args._corners[2]._radius);
        coords->push_back(arcPos);
        
        osg::Vec3 arcPivot = arcPos + -(heightDir*args._corners[2]._radius);
        
        //loop and draw segments for rounded corner around arcPivot
        unsigned int segments = args._corners[2]._segments;
        for(unsigned int i=0; i<segments; i++){
            float pc = (float)(i+1)/(float)segments;
            //get radians for segment
            float rad = osg::DegreesToRadians(90.0f+(90.0f*pc));
            float c = cosf(rad);
            float s = sinf(rad);
            
            osg::Vec3 localArcPos = osg::Vec3(c,s,0.0f) * args._corners[2]._radius;
            arcPos = arcPivot + localArcPos;
            coords->push_back(arcPos);
        }
        
    }else{
        coords->push_back(corner+heightVec);
    }
    
    if(args._corners[0]._radius != 0){
        coords->push_back(corner + (heightDir*args._corners[0]._radius));
    }else{
        coords->push_back(corner);
    }
    
    geom->setVertexArray(coords);
    
    osg::Vec2Array* tcoords = new osg::Vec2Array();
    for(unsigned int i=0; i<coords->size(); i++){
        tcoords->push_back(hogbox::computeQuadTexCoord((*coords)[i], width, height, l,b,r,t));
    }
    
    geom->setTexCoordArray(0,tcoords);
    
    osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
    //geom->setColorArray(colours);
    //geom->setColorBinding(Geometry::BIND_OVERALL);
    
    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0] = widthVec^heightVec;
    (*normals)[0].normalize();
    //geom->setNormalArray(normals);
    //geom->setNormalBinding(Geometry::BIND_OVERALL);
    
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN,0,coords->size()));
    
    return geom;
}

extern HOGBOX_EXPORT osg::Vec2 hogbox::computeQuadTexCoord(const osg::Vec3& coord, const float& width, const float& height,
                                                           const float& l, const float& b, const float& r, const float& t)
{
    //get unit coord
    float unitX,unitY;
    if(coord.x() == 0.0f){
        unitX=0.0f;
    }else{
        unitX = coord.x()/width;
    }
    if(coord.y() == 0.0f){
        unitY=0.0f;
    }else{
        unitY = coord.y()/height;
    }

    //get coord difference
    float xDif = r-l;
    float yDif = t-b;
    
    float u = l+(xDif*unitX);
    float v = b+(yDif*unitY);
    return osg::Vec2(u,v);
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
		int width = image->s();
		int height = image->t();

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
int MsgBoxYesNoOSX(const char* title, const char* message){return 0;}
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
	if(image == NULL){return false;}
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
       // SetNoiseFrequency(frequency);
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
                   // *(ptr+inc) = (GLubyte) (((noise3(ni) + 1.0) * amp) * 128.0);
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
	osg::Geometry* _baseLayer = new osg::Geometry();
	osg::Vec3Array* vertices = new osg::Vec3Array;
	vertices->push_back(osg::Vec3(0.0f,		0.0f,		depth));
	vertices->push_back(osg::Vec3(size.x(),	0.0f,		depth));
	vertices->push_back(osg::Vec3(size.x(),	size.y(),	depth));
	vertices->push_back(osg::Vec3(0.0f,		size.y(),	depth));
	_baseLayer->setVertexArray(vertices);

	osg::Vec3Array* normals = new osg::Vec3Array;
	normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
	_baseLayer->setNormalArray(normals);
	_baseLayer->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::Vec4Array* _baseColors = new osg::Vec4Array;
	_baseColors->push_back(osg::Vec4(1,1,1,1));
	_baseLayer->setColorArray(_baseColors);
	_baseLayer->setColorBinding(osg::Geometry::BIND_OVERALL);

	osg::Vec2Array* texCoords = new osg::Vec2Array;
	texCoords->push_back(osg::Vec2(0.0f,0.0f));
	texCoords->push_back(osg::Vec2(1.0f,0.0f));
	texCoords->push_back(osg::Vec2(1.0f,1.0f));
	texCoords->push_back(osg::Vec2(0.0f,1.0f));
	_baseLayer->setTexCoordArray(0, texCoords);

	_baseLayer->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	return _baseLayer;
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



