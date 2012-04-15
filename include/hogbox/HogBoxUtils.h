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


#include <hogbox/Export.h>
#include <hogbox/HogBoxBase.h>

#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <osg/Texture3D>
#include <osg/Texture1D>

#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/AnimationManagerBase>

#include <hogbox/FindStateSetVisitor.h>
#include <hogbox/Noise.h>


namespace hogbox {

    //
    //Camera helpers
    extern HOGBOX_EXPORT osg::Matrix ComputeLookAtMatrixForNodeBounds(osg::Node* scene, float distance = 3.0f, 
                                                                      osg::Vec3 axis = osg::Y_AXIS, osg::Vec3 upAxis = osg::Z_AXIS);
    
    extern HOGBOX_EXPORT bool ComputeLookAtVectorsForNodeBounds(osg::Node* scene, float distance, osg::Vec3 forwardAxis, osg::Vec3 upAxis,
                                                                osg::Vec3& outCamPos, osg::Vec3& outLookAt, osg::Vec3& outUpAxis);
    
	//
	//Image Helpers

	//create a new image from a sub region of the source image
	extern HOGBOX_EXPORT osg::Image* CreateSubImage(int sCol, int sRow, int width, int height, osg::Image* source);

    //
	extern HOGBOX_EXPORT osg::TextureCubeMap* LoadTextureCubeCross(std::string file);

    
	extern HOGBOX_EXPORT double CalcAngleBetweenVectors(osg::Vec3 u, osg::Vec3 v);
    
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

	//
	//Applydefault nodeMask from a nodes name
	//NORENDER = do not add the default MAIN_CAMERA_CULL mask
	//PICKABLE = add PICK_MESH mask
	//COLLIDABLE = add the COLLIDE_MESH mask
	//GlOW = add GLOW_MESH mask, do not add the default MAIN_CAMERA_CULL mask
	class ApplyDefaultNodeMaskVisitor : public osg::NodeVisitor
	{
	public:

		ApplyDefaultNodeMaskVisitor()
			: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
		{
		}
	    
		virtual void apply(osg::Node& node)
		{
			if(dynamic_cast<osg::Geode*>(&node))
			{
				bool render=true;
				bool glow=false;
				osg::Node::NodeMask mask = 0;

				if(node.getName().rfind("NORENDER")!=std::string::npos)
				{render=false;}
				if(node.getName().rfind("GlOW")!=std::string::npos)
				{glow=true;render=false;}
				//add masks
				if(render){
					mask = mask | MAIN_CAMERA_CULL;
				}else if(glow){
					//mask = mask | NodeMasks
				}

				if(node.getName().rfind("PICKABLE")!=std::string::npos)
				{
					mask = mask | PICK_MESH;
				}
				if(node.getName().rfind("COLLIDABLE")!=std::string::npos)
				{
					mask = mask | COLLIDE_MESH;
				}

				//set the nodes mask
				node.setNodeMask(mask);
			}

			traverse(node);
		}
	};

	//
	//Vistor to find any statesets and enable/disable backface culling
	class ApplyBackFaceCullingVisitor : public osg::NodeVisitor
	{
	public:
		bool _cullBackFaces;

		ApplyBackFaceCullingVisitor(bool cullBackFaces=true)
			: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
			_cullBackFaces(cullBackFaces)
		{
		}
	    
		virtual void apply(osg::Node& node)
		{
			osg::StateSet* nodesState = node.getStateSet();
			if(nodesState){
				if(_cullBackFaces){
					nodesState->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
				}else{
					nodesState->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
				}
			}

			traverse(node);
		}
	};
    
    //
    //Visitor to enable VBO on all drawables
	class ApplyVBOVisitor : public osg::NodeVisitor
	{
	public:
        
		ApplyVBOVisitor()
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
		{
		}
	    
		virtual void apply(osg::Geode& geode)
		{
            for(unsigned int i=0; i<geode.getNumDrawables(); i++)
            {
                geode.getDrawable(i)->setUseDisplayList(false);
                geode.getDrawable(i)->setUseVertexBufferObjects(true);
            }
            
			traverse(geode);
		}
	};

};//end hogbox namespace