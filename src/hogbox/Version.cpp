#include <hogbox/Version.h>

extern "C" {


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
