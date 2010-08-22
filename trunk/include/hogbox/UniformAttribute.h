#pragma once

//////////////////////////////////////////////////////////
// Author:	Thomas Hogarth								//
// Date:	12/09/2007									//
//														//
// Class:												//
// UniformAtts											//
//														//
// Description:											//
// Wraps up the functionality of creating a uniform     //
// variable and associating min and max values with it  //
//														//
//////////////////////////////////////////////////////////

#include <hogbox/Export.h>

#include <osg/Uniform>

namespace hogbox {

class UniformAtts : public osg::Object
{
public:
	UniformAtts(void) : osg::Object() {}
	UniformAtts(osg::ref_ptr<osg::Uniform> uniform, int min=0, int max=1)
	{
		m_min=min;
		m_max=max;
		m_uniform=uniform;
	}

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	UniformAtts(const UniformAtts&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY){}
	META_Box(hogbox, UniformAtts);

	//return the uniform
	osg::Uniform* GetUniform(){return m_uniform.get();}

	const int GetMin(){return m_min;}
	const int GetMax(){return m_max;}

protected:

	~UniformAtts(void)
	{m_uniform=NULL;}

protected:
	osg::ref_ptr<osg::Uniform> m_uniform; 
	int m_min;
	int m_max;
};

typedef osg::ref_ptr<UniformAtts> UniformAttsPtr;

}; //end hogbox namespace