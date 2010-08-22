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
class HogVisionNode :
	public RTTPass
{
public:
	HogVisionNode(void);
	virtual ~HogVisionNode(void);

	

};

}