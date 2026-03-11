/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ShapeToGeometry example application
 */
#ifndef SHAPE_TO_GEOMETRY
    #define SHAPE_TO_GEOMETRY 1
    #include <osg/geometry/Geometry.hpp>
    #include <osg/maths/compat.hpp>
    #include <osg/nodes/Geode.hpp>
    #include <osg/traversal/NodeVisitor.hpp>

osg::Geode*
convertShapeToGeode( const osg::Shape&             shape,
                     const osg::TessellationHints* hints );

osg::Geode*
convertShapeToGeode( const osg::Shape&             shape,
                     const osg::TessellationHints* hints,
                     const osg::vec4&              color );

#endif
