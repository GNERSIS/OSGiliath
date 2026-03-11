/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ShapeToGeometry example application
 */
#include "ShapeToGeometry.hpp"

#include <osg/maths/mat4.hpp>

osg::Geode*
convertShapeToGeode( const osg::Shape&             shape,
                     const osg::TessellationHints* hints )
{
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( osg::convertShapeToGeometry( shape, hints ) );
    return geode;
}

osg::Geode*
convertShapeToGeode( const osg::Shape&             shape,
                     const osg::TessellationHints* hints,
                     const osg::vec4&              color )
{
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( osg::convertShapeToGeometry( shape, hints, color ) );
    return geode;
}
