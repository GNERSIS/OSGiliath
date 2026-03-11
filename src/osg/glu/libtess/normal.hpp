/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osg: __gl_projectPolygon.
 */
/*
** Author: Eric Veach, July 1994.
**
*/

#pragma once

#include "tess.hpp"

/* __gl_projectPolygon( tess ) determines the polygon normal
 * and project vertices onto the plane of the polygon.
 */
void
__gl_projectPolygon( GLUtesselator* tess );
