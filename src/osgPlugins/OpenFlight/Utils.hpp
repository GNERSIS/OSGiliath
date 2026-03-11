/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: defined, FLTEXP_DELETEFILE.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#pragma once

// FLTEXP_DELETEFILE macro is used to delete temp files created during file export.
// (Too bad OSG doesn't use Boost.)

#if defined( _WIN32 )

    #include <windows.h>
    #define FLTEXP_DELETEFILE( file ) DeleteFile( ( file ) )

#else    // Unix

    #include <stdio.h>
    #define FLTEXP_DELETEFILE( file ) remove( ( file ) )

#endif
