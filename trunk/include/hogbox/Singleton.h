#pragma once

#include <assert.h>

namespace hogbox
{

//! The Singleton class is a template class for creating singleton objects.
/*!
    When the static Instance() method is called for the first time, the singleton 
    object is created. Every sequential call returns a reference to this instance.
    The class instance can be destroyed by calling the DestroyInstance() method.
*/
template <typename T> 
class Singleton
{
public:
    
    //! Gets a reference to the instance of the singleton class.
    /*!
        \return A reference to the instance of the singleton class.
        If there is no instance of the class yet, one will be created.
    */
    static T* Instance(bool erase = false)
    {
		static osg::ref_ptr<T> s_instance = new T; //!< singleton class instance
		if(erase)
		{
			s_instance->destruct();
			s_instance = 0;
		}
        return s_instance.get();
    }


protected:

    // shield the constructor and destructor to prevent outside sources
    // from creating or destroying a Singleton instance.

    //! Default constructor.
    Singleton()
    {
    }


    //! Destructor.
    virtual ~Singleton()
    {
    }

	virtual void destruct()=0;

private:

    //! Copy constructor.
    Singleton(const Singleton& source)
    {
    }

};


}; //end hogbox namespace