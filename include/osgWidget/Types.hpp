/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Common type definitions for osgWidget. Typedefs for
 * point, size, and coordinate types used throughout.
 */
// Code by: Jeremy Moles (cubicool) 2007-2008

#pragma once

#include <numeric>
#include <osg/geometry/Geometry.hpp>

namespace osgWidget
{

    typedef osg::Vec2Array            TexCoordArray;
    typedef osg::Vec3Array            PointArray;
    typedef osg::Vec4Array            ColorArray;

    typedef TexCoordArray::value_type TexCoord;
    typedef PointArray::value_type    Point;
    typedef ColorArray::value_type    Color;

    typedef TexCoord::value_type      texcoord_type;
    typedef Point::value_type         point_type;
    typedef Color::value_type         color_type;

    typedef osg::vec2                 XYCoord;
    typedef osg::vec4                 Quad;

    typedef osg::dmat4::value_type    matrix_type;

    // This is multiplied by a normalized Z value [0.0f, -1.0f] to create a RenderBin
    // number to set the state of the Window/Widget with. Perhaps at some later time this
    // should be configurable.
    const int                         OSGWIDGET_RENDERBIN_MOD = 5'000;

}
