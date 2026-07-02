/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Axis-aligned translation dragger with colored arrows.
 * Provides constrained movement along X, Y, or Z axes.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/TranslateAxisDragger.hpp>

#include <osg/maths/quat.hpp>

using namespace osgManipulator;

TranslateAxisDragger::TranslateAxisDragger()
{
    _xDragger = new Translate1DDragger( osg::dvec3( 0.0, 0.0, 0.0 ),
                                        osg::dvec3( 0.0, 0.0, 1.0 ) );
    addChild( _xDragger.get() );
    addDragger( _xDragger.get() );

    _yDragger = new Translate1DDragger( osg::dvec3( 0.0, 0.0, 0.0 ),
                                        osg::dvec3( 0.0, 0.0, 1.0 ) );
    addChild( _yDragger.get() );
    addDragger( _yDragger.get() );

    _zDragger = new Translate1DDragger( osg::dvec3( 0.0, 0.0, 0.0 ),
                                        osg::dvec3( 0.0, 0.0, 1.0 ) );
    addChild( _zDragger.get() );
    addDragger( _zDragger.get() );

    _axisLineWidth      = 2.0F;
    _pickCylinderRadius = 0.015F;
    _coneHeight         = 0.1F;

    setParentDragger( getParentDragger() );
}

TranslateAxisDragger::~TranslateAxisDragger()
{
}

void
TranslateAxisDragger::setupDefaultGeometry()
{
    // Create a line.
    _lineGeode = new osg::Geode;
    {
        osg::Geometry*  geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array( 2 );
        ( *vertices )[0]         = osg::vec3( 0.0F, 0.0F, 0.0F );
        ( *vertices )[1]         = osg::vec3( 0.0F, 0.0F, 1.0F );

        geometry->setVertexArray( vertices );
        geometry->addPrimitiveSet(
            new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 2 )
        );

        _lineGeode->addDrawable( geometry );
    }

    // Turn of lighting for line and set line width.
    {
        _lineWidth = new osg::LineWidth();
        _lineWidth->setWidth( _axisLineWidth );
        _lineGeode->getOrCreateStateSet()->setAttributeAndModes(
            _lineWidth.get(),
            osg::StateAttribute::ON
        );
        // GL_LIGHTING removed: not in core profile
    }

    // Add line to all the individual 1D draggers.
    _xDragger->addChild( _lineGeode.get() );
    _yDragger->addChild( _lineGeode.get() );
    _zDragger->addChild( _lineGeode.get() );

    osg::Geode* geode = new osg::Geode;

    // Create a cone.
    {
        _cone                            = new osg::Cone( osg::vec3( 0.0F, 0.0F, 1.0F ),
                                                          _coneHeight * 0.25F,
                                                          _coneHeight );
        osg::ShapeDrawable* coneDrawable = new osg::ShapeDrawable( _cone.get() );
        // coneDrawable->setColor(osg::vec4(0.0f,0.0f,1.0f,1.0f));
        geode->addDrawable( coneDrawable );
    }

    // Create an invisible cylinder for picking the line.
    {
        _cylinder               = new osg::Cylinder( osg::vec3( 0.0F, 0.0F, 0.5F ),
                                                     _pickCylinderRadius,
                                                     1.0F );
        osg::Drawable* geometry = new osg::ShapeDrawable( _cylinder.get() );
        setDrawableToAlwaysCull( *geometry );
        geode->addDrawable( geometry );
    }

    // Add geode to all 1D draggers.
    _xDragger->addChild( geode );
    _yDragger->addChild( geode );
    _zDragger->addChild( geode );

    // Rotate X-axis dragger appropriately.
    {
        osg::quat rotation( osg::vec3( 0.0F, 0.0F, 1.0F ),
                            osg::vec3( 1.0F, 0.0F, 0.0F ) );
        _xDragger->setMatrix( osg::rotate( osg::dquat( rotation ) ) );
    }

    // Rotate Y-axis dragger appropriately.
    {
        osg::quat rotation( osg::vec3( 0.0F, 0.0F, 1.0F ),
                            osg::vec3( 0.0F, 1.0F, 0.0F ) );
        _yDragger->setMatrix( osg::rotate( osg::dquat( rotation ) ) );
    }

    // Send different colors for each dragger.
    _xDragger->setColor( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    _yDragger->setColor( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    _zDragger->setColor( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );
}

void
TranslateAxisDragger::setAxisLineWidth( float linePixelWidth )
{
    _axisLineWidth = linePixelWidth;
    if( _lineWidth.valid() )
    {
        _lineWidth->setWidth( linePixelWidth );
    }
}

void
TranslateAxisDragger::setPickCylinderRadius( float pickCylinderRadius )
{
    _pickCylinderRadius = pickCylinderRadius;
    if( _cylinder.valid() )
    {
        _cylinder->setRadius( pickCylinderRadius );
    }
}

void
TranslateAxisDragger::setConeHeight( float height )
{
    _coneHeight = height;
    if( _cone.valid() )
    {
        _cone->setRadius( 0.25F * height );
        _cone->setHeight( height );
    }
}
