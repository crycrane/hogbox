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

#include "rttpass.h"


namespace hogboxVision {

/*
enum HVNResult{
	TRUE,
	FALSE,
	ERROR,
	SUCCESS,
	ERROR_IN_
};
*/
//
//HogVisionNode
//
//Inherits from RTTPass
//Addeds extra cosmetic functionality to the rttpass
//loading and saving of state to file
//Can create and handle a gui for interacting with the pass
//Aid in the process of attaching passes together
//
//Not impelemented yet
//
class HogVisionNode :
	public RTTPass
{
public:
	HogVisionNode(void);
	virtual ~HogVisionNode(void);

	

};

}