/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * GLStaticLibrary class.
 * Provides: getProcAddress.
 */
/* file:        src/osg/GLStaticLibrary.h
 * author:      Alok Priyadarshi 2010-04-27
 */

#pragma once

namespace osg
{

    class GLStaticLibrary
    {
        public:

            static void*
            getProcAddress( const char* procName );
    };

}
