/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Operating system utility functions. Provides environment variable
 * access, temporary file paths, and platform detection.
 */
#include <osg/core/os_utils.hpp>

extern "C"
{

#ifdef __APPLE__
    #define USE_POSIX_SPAWN 1
#endif

#ifdef USE_POSIX_SPAWN

    #include <spawn.h>
    #include <sys/wait.h>

    int
    osg_system( const char* command )
    {
        pid_t pid;
        posix_spawn( &pid, command, NULL, NULL, NULL, NULL );
        return waitpid( pid, NULL, 0 );
    }

#else    // use tranditional C sysmtem call for osg_system implementation

    #include <stdlib.h>

    int
    osg_system( const char* command )
    {
        return system( command );
    }

#endif
}
