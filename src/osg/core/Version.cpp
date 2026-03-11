/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query functions. Returns major, minor, patch,
 * SO version, and library name strings.
 */
#include <osg/Version>

#include <stdio.h>
#include <string>

extern "C"
{

    const char*
    osgGetVersion()
    {
        static char osg_version[256];
        static int  osg_version_init = 1;
        if( osg_version_init )
        {
            if( OSG_VERSION_REVISION == 0 )
            {
                sprintf( osg_version,
                         "%d.%d.%d",
                         OSG_VERSION_MAJOR,
                         OSG_VERSION_MINOR,
                         OSG_VERSION_RELEASE );
            }
            else
            {
                sprintf( osg_version,
                         "%d.%d.%d-%d",
                         OSG_VERSION_MAJOR,
                         OSG_VERSION_MINOR,
                         OSG_VERSION_RELEASE,
                         OSG_VERSION_REVISION );
            }

            osg_version_init = 0;
        }

        return osg_version;
    }

    const char*
    osgGetSOVersion()
    {
        static char osg_soversion[32];
        static int  osg_soversion_init = 1;
        if( osg_soversion_init )
        {
            sprintf( osg_soversion, "%d", OSGILIATH_SOVERSION );
            osg_soversion_init = 0;
        }

        return osg_soversion;
    }

    const char*
    osgGetLibraryName()
    {
        return "OpenSceneGraph Library";
    }
}
