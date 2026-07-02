/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Axis-aligned scaling dragger. Provides constrained scale
 * along X, Y, or Z axes via box-shaped handles.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/ScaleAxisDragger.hpp>

#include <osg/geometry/Geometry.hpp>
#include <osg/maths/quat.hpp>

using namespace osgManipulator;

ScaleAxisDragger::ScaleAxisDragger()
{
    _xDragger = new osgManipulator::Scale1DDragger();
    addChild( _xDragger.get() );
    addDragger( _xDragger.get() );

    _yDragger = new osgManipulator::Scale1DDragger();
    addChild( _yDragger.get() );
    addDragger( _yDragger.get() );

    _zDragger = new osgManipulator::Scale1DDragger();
    addChild( _zDragger.get() );
    addDragger( _zDragger.get() );

    _axisLineWidth = 2.0F;
    _boxSize       = 0.05F;

    setParentDragger( getParentDragger() );
}

ScaleAxisDragger::~ScaleAxisDragger()
{
}

void
ScaleAxisDragger::setupDefaultGeometry()
{
    // Create a line.
    _lineGeode = new osg::Geode;
    {
        osg::Geometry*  geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array( 2 );
        ( *vertices )[0]         = osg::vec3( 0.0F, 0.0F, 0.0F );
        ( *vertices )[1]         = osg::vec3( 1.0F, 0.0F, 0.0F );

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

    // Create a box.
    _box = new osg::Box( osg::vec3( 1.0F, 0.0F, 0.0F ), _boxSize );
    geode->addDrawable( new osg::ShapeDrawable( _box.get() ) );

    // Add geode to all 1D draggers.
    _xDragger->addChild( geode );
    _yDragger->addChild( geode );
    _zDragger->addChild( geode );

    // Rotate Z-axis dragger appropriately.
    {
        osg::quat rotation( osg::vec3( 1.0F, 0.0F, 0.0F ),
                            osg::vec3( 0.0F, 0.0F, 1.0F ) );
        _zDragger->setMatrix( osg::rotate( osg::dquat( rotation ) ) );
    }

    // Rotate Y-axis dragger appropriately.
    {
        osg::quat rotation( osg::vec3( 1.0F, 0.0F, 0.0F ),
                            osg::vec3( 0.0F, 1.0F, 0.0F ) );
        _yDragger->setMatrix( osg::rotate( osg::dquat( rotation ) ) );
    }

    // Send different colors for each dragger.
    _xDragger->setColor( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    _yDragger->setColor( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    _zDragger->setColor( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );
}

void
ScaleAxisDragger::setAxisLineWidth( float linePixelWidth )
{
    _axisLineWidth = linePixelWidth;
    if( _lineWidth.valid() )
    {
        _lineWidth->setWidth( linePixelWidth );
    }
}

void
ScaleAxisDragger::setBoxSize( float size )
{
    _boxSize = size;
    if( _box.valid() )
    {
        _box->setHalfLengths( osg::vec3( size * 0.5F, size * 0.5F, size * 0.5F ) );
    }
}
