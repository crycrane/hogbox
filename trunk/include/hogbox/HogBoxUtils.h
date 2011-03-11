#pragma once

//////////////////////////////////////////////////////////
// Author:	Thomas Hogarth								//
// Date:	12/09/2007									//
//														//
// Class:												//
// Osg object helper class								//
//														//
// Description:											//
// Wraps up the functionality of loading and			//
// transforming and manipulating a geometry object in	//
// osg													//
//														//
//////////////////////////////////////////////////////////

#include <hogbox/Export.h>

#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <osg/Texture3D>
#include <osg/Texture1D>


#include <hogbox/FindStateSetVisitor.h>
#include <hogbox/Noise.h>



namespace hogbox {

	//Scenegraph helpers

	//rename all the geodes in the graph node to the passed name
	extern HOGBOX_EXPORT void RenameGeodes(osg::Node* node, const std::string name);
	//find a node in the graph node with name
	extern HOGBOX_EXPORT osg::Node*	 FindNodeByName(osg::Node* node, const std::string name);

	//
	//Image Helpers

	//get the width of image in pixels
	extern HOGBOX_EXPORT int GetImageWidth(osg::Image* image);
	extern HOGBOX_EXPORT int GetImageHeight(osg::Image* image);

	//create a new image from a sub region of the source image
	extern HOGBOX_EXPORT osg::Image* CreateSubImage(int sCol, int sRow, int width, int height, osg::Image* source);

	//load an image from disk
	extern HOGBOX_EXPORT osg::Image* LoadImage2D(std::string name);
	//load image from disk and apply to a texture
	extern HOGBOX_EXPORT osg::Texture2D* LoadTexture2D(const std::string& fileName);
	extern HOGBOX_EXPORT osg::TextureCubeMap* LoadTextureCubeCross(std::string file);


	//
	//File helpers

	extern HOGBOX_EXPORT bool RelocateTextureImage(std::string savePath, osg::Image* image, bool local);
	extern HOGBOX_EXPORT std::string ChangeFileFolder(std::string filePath, std::string newFolder);

	//Launch the platforms own open file dialog box and return the result as a string
	extern HOGBOX_EXPORT const std::string OpenPlatformFileDialog(const char* exts, const char* defaultExt = NULL, const char* defaultFolder = NULL);
	extern HOGBOX_EXPORT char* OpenSaveFileDialog(const char* exts, char* defaultFolder);	
	
	//copy a file
	extern HOGBOX_EXPORT bool CopyFileHB(std::string from, std::string to);
	
	//get set the platforms working directory
	extern HOGBOX_EXPORT void SetWorkingDirectory(const char* dir);
	extern HOGBOX_EXPORT void GetWorkingDirectory(int length, char* retDir);
	
	//launch a platform specific messagebox
	extern HOGBOX_EXPORT void MsgBox(const std::string title, const std::string message, int mode = 0);
	extern HOGBOX_EXPORT int MsgBoxYesNo(std::string title, std::string msg);
	
	//lauch programs or open files
	extern HOGBOX_EXPORT void ShellExec(std::string file, std::string verb);	
	extern HOGBOX_EXPORT void ShellExec(std::string file, std::string verb, std::string args);
	
	//conveniance load file funcs
	extern HOGBOX_EXPORT osg::Texture2D* LoadTexture2DDialog();
	extern HOGBOX_EXPORT osg::Image* LoadImageDialog();

	extern HOGBOX_EXPORT std::string GetFromFile(char* file);

	//
	//Noise helpers

	extern HOGBOX_EXPORT osg::Image*  make3DNoiseImage(int texSize);
	extern HOGBOX_EXPORT osg::Texture3D*  make3DNoiseTexture(int texSize );
	extern HOGBOX_EXPORT osg::Image*  make1DSineImage( int texSize );
	extern HOGBOX_EXPORT osg::Texture1D*  make1DSineTexture( int texSize );


	//Geom helpers
	extern HOGBOX_EXPORT osg::Geometry* BuildXYQuad(osg::Vec2 size, osg::Vec4 color, float depth);

	extern HOGBOX_EXPORT double CalcAngleBetweenVectors(osg::Vec3 u, osg::Vec3 v);

	//
	//Find all nodes in the graph that match thename passed
	class FindNodesByName : public osg::NodeVisitor
	{
	public:

		FindNodesByName(std::string searchName, bool subString=false)
			: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),        
			_searchName(searchName),
			_searchSubString(subString),
			_count(0)
		{
		}
	    
		virtual void apply(osg::Node& node)
		{
			if(_searchSubString)
			{
				size_t found=node.getName().rfind(_searchName);
				if (found!=std::string::npos)
				{
					_foundList.push_back(&node);
					_count++;
				}
			}else{
				if(node.getName() == _searchName){
					_foundList.push_back(&node);
					_count++;
				}
			}
			traverse(node);
		}
	    
		std::string _searchName;
		//search for name as substring of name
		bool _searchSubString;

		std::vector<osg::Node*> _foundList;
		int _count;
	};


};//end hogbox namespace