/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osg: __gl_memInit.
 */
/*
** Author: Eric Veach, July 1994.
**
*/

#include "memalloc.hpp"
#include "string.h"

int
__gl_memInit( size_t /*maxFast*/ )
{
#ifndef NO_MALLOPT
/*  mallopt( M_MXFAST, maxFast );*/
#endif
    return 1;
}
