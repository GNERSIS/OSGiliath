/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osg: __gl_memInit.
 */
/*
** Author: Eric Veach, July 1994.
**
*/

#pragma once

#include <stdlib.h>

#define memRealloc realloc
#define memFree    free

#define memInit    __gl_memInit
extern int
__gl_memInit( size_t );

#define memAlloc malloc
