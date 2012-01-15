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

#include <osg/Image>
#include <time.h>

namespace hogbox {
	
	static void SetSeed(const unsigned int& seed=-1)
	{
		if(seed>=0)
		{
			srand(seed);
		}else{
			srand ( time(NULL) );
		}		
	}
	
	//generates a psuedo-random int b
	static int RandInt()
	{
		return rand();
	} 
	
	//generates a psuedo-random float 
	static float RandFloat()
	{
		return rand()/(float(RAND_MAX)+1);
	} 
	
	//generates a psuedo-random int between min and max
	static float RandInt(int min, int max)
	{
		if (min>max)
		{
			return rand()%((min-max)+1)+max ; 
		}
		else
		{
			return rand()%((max-min)+1)+min;
		}    
	}
	
	//generates a psuedo-random float between min and max
	static float RandFloat(float min, float max)
	{
		if (min>max)
		{
			return RandFloat()*(min-max)+max;    
		}
		else
		{
			return RandFloat()*(max-min)+min;
		}    
	}
	
	static osg::Vec2 RandVec2(float min, float max)
	{
		float disX = hogbox::RandFloat(min, max);
		float disY = hogbox::RandFloat(min, max);
		
		return osg::Vec2(disX,disY);
	}
	
	static osg::Vec3 RandNormalisedVec3()
	{
		float disX = hogbox::RandFloat(-1.0, 1.0);
		float disY = hogbox::RandFloat(-1.0, 1.0);
		float disZ = hogbox::RandFloat(-1.0, 1.0);
		osg::Vec3 vec = osg::Vec3(disX,disY,disZ);
		vec.normalize();
		return vec;
	}
	
	static osg::Vec3 RandVec3(float min, float max)
	{
		/* generate secret number: */
		float disX = hogbox::RandFloat(min, max);
		float disY = hogbox::RandFloat(min, max);
		float disZ = hogbox::RandFloat(min, max);
		
		return osg::Vec3(disX,disY,disZ);
	}

	static osg::Vec4 RandVec4(float min, float max)
	{
		/* generate secret number: */
		float disX = hogbox::RandFloat(min, max);
		float disY = hogbox::RandFloat(min, max);
		float disZ = hogbox::RandFloat(min, max);
		float disW = hogbox::RandFloat(min, max);
		
		return osg::Vec4(disX,disY,disZ,disW);
	}
	
	//binary noise funcs
	extern osg::ref_ptr<osg::Image> CreateGreyScaleBinaryNoiseImage2D(const int& width, const int& height, 
																	  const float& color1= 0.0f, const float& color2=1.0f,
																	  const unsigned int& seed = -1);
	extern osg::ref_ptr<osg::Image> CreateRGBBinaryNoiseImage2D(const int& width, const int& height, 
																const osg::Vec3& color1 = osg::Vec3(0,0,0), const osg::Vec3& color2=osg::Vec3(1,1,1),
																const unsigned int& seed = -1);
	extern osg::ref_ptr<osg::Image> CreateRGBABinaryNoiseImage2D(const int& width, const int& height, 
																const osg::Vec4& color1 = osg::Vec4(0,0,0,0), const osg::Vec4& color2=osg::Vec4(1,1,1,1),
																const unsigned int& seed = -1);
	
	//binary noise funcs
	//returns false if image is not gl_luminance
	extern bool FillGreyScaleBinaryNoiseImage2D(osg::Image* image,
												const float& color1= 0.0f, const float& color2=1.0f,
												const unsigned int& seed = -1);
	extern bool FillRGBBinaryNoiseImage2D(osg::Image* image, 
										const osg::Vec3& color1 = osg::Vec3(0,0,0), const osg::Vec3& color2=osg::Vec3(1,1,1),
										const unsigned int& seed = -1);
	extern bool FillRGBABinaryNoiseImage2D(osg::Image* image, 
											const osg::Vec4& color1 = osg::Vec4(0,0,0,0), const osg::Vec4& color2=osg::Vec4(1,1,1,1),
											const unsigned int& seed = -1);

};//end hogbox namespace
