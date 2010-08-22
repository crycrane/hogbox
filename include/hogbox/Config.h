#pragma once

#define HOGBOX_DISABLE_MSVC_WARNINGS

const unsigned int MAX_CHARS = 1024;


#ifdef WIN32

	//#define WIN32_LEAN_AND_MEAN
	//#define VC_EXTRALEAN

	#include <windows.h>

	#define X86
	#define SAVE_PROJECT_FORMAT_STRING "HOGBOX (*.hog)\0*.hog\0"
	#define OPEN_MODEL_STRING_WIN "HOGBOX (*.hog)\0*.hog\0OpenSceneGraph (*.ive)\0*.ive\0OpenSceneGraph (*.osg)\0*.osg\0 3DS (*.3ds)\0*.3ds\0Alias Wavefront (*.obj)\0*.obj\0Stereolithography (*.stl)\0*.stl\0Openflight (*.flt)\0*.flt\0Quake MD2 (*.md2)\0*.md2\0Direct X (*.x)\0*.x\0Geo(*.geo)\0*.geo\0All Files (*.*)\0*.*\0"
	#define EXPORT_MODEL_STRING_WIN "OpenSceneGraph (*.ive)\0*.ive\0OpenSceneGraph (*.osg)\0*.osg\0Alias Wavefront (*.obj)\0*.obj\0Openflight (*.flt)\0*.flt\0"
    #define OPEN_IMAGE_STRING_WIN "PNG (*.png)\0*.png\0Bitmap (*.bmp)\0*.bmp\0JPEG (*.jpg)\0*.jpg\0DirectX image format (*.dds)\0*.dds\0Tiff (*.tiff)\0*.tiff\0TGA (*.tga)\0*.tga\0gif (*.gif)\0*.gif\0High Dynamic Range image (*.hdr)\0*.hdr\0All Files (*.*)\0*.*\0"

#else


	#define HOGBOX_EXPORT //xcode doesn't need the dll export 

	#define MAX_PATH 2048

	#define SAVE_PROJECT_FORMAT_STRING "hog"
	
	#define OPEN_MODEL_STRING_WIN "hog, ive, osg, 3ds, obj, stl, flt, md2, x, geo" //need an all file type

	#define EXPORT_MODEL_STRING_WIN "ive, osg, obj, flt, 3ds"

	#define OPEN_IMAGE_STRING_WIN "png, bmp, jpg, dds, tiff, tga, gif, hdr"


	//declare handlers
	#define Sleep(a) (usleep(a)) //pass sleep comands
	//#define hogbox::SetWorkingDirectory(a) (chdir(a))
	//#define hogbox::GetWorkingDirectory(a, b) (chdir(a, b))

#endif
