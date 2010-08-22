/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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
#include <string>
#include <stdio.h>

extern "C" {


#define hogbox_MAJOR_VERSION    0
#define hogbox_MINOR_VERSION    2
#define hogbox_PATCH_VERSION    0
#define hogbox_SOVERSION        1

const char* hogboxGetVersion()
{
    static char hogbox_version[256];
    static int hogbox_version_init = 1;
    if (hogbox_version_init)
    {
        sprintf(hogbox_version,"%d.%d.%d",hogbox_MAJOR_VERSION,hogbox_MINOR_VERSION,hogbox_PATCH_VERSION);
        hogbox_version_init = 0;
    }
    
    return hogbox_version;
}

const char* hogboxGetSOVersion()
{
    static char hogbox_soversion[32];
    static int hogbox_soversion_init = 1;
    if (hogbox_soversion_init)
    {
        sprintf(hogbox_soversion,"%d",hogbox_SOVERSION);
        hogbox_soversion_init = 0;
    }
    
    return hogbox_soversion;
}

const char* hogboxGetLibraryName()
{
    return "HogBox Library";
}

}
