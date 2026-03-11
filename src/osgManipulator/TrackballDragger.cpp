/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Rotation dragger using a virtual trackball interface.
 * Provides unconstrained 3D rotation via arc segments.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/TrackballDragger>

#include <osg/geometry/Geometry.hpp>
#include <osg/maths/quat.hpp>
#include <osg/nodes/AutoTransform.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osgManipulator/AntiSquish>

using namespace osgManipulator;

namespace
{

    osg::Geometry*
    createCircleGeometry( float        radius,
                          unsigned int numSegments )
    {
        const float angleDelta =
            static_cast<float>( 2.0 * osg::PI / double( numSegments ) );
        const float     r           = radius;
        float           angle       = 0.0F;
        osg::Vec3Array* vertexArray = new osg::Vec3Array( numSegments );
        osg::Vec3Array* normalArray = new osg::Vec3Array( numSegments );
        for( unsigned int i = 0; i < numSegments; ++i, angle += angleDelta )
        {
            float c = cosf( angle );
            float s = sinf( angle );
            ( *vertexArray )[i].set( c * r, s * r, 0.0F );
            ( *normalArray )[i].set( c, s, 0.0F );
        }
        osg::Geometry* geometry = new osg::Geometry();
        geometry->setVertexArray( vertexArray );
        geometry->setNormalArray( normalArray, osg::Array::BIND_PER_VERTEX );
        geometry->addPrimitiveSet(
            new osg::DrawArrays( osg::PrimitiveSet::LINE_LOOP,
                                 0,
                                 static_cast<GLsizei>( vertexArray->size() ) )
        );
        return geometry;
    }

}

TrackballDragger::TrackballDragger( bool useAutoTransform )
{
    if( useAutoTransform )
    {
        float                 pixelSize = 50.0F;
        osg::MatrixTransform* scaler    = new osg::MatrixTransform;
        scaler->setMatrix(
            osg::scale( ( double )pixelSize, ( double )pixelSize, ( double )pixelSize )
        );

        osg::AutoTransform* at = new osg::AutoTransform;
        at->setAutoScaleToScreen( true );
        at->addChild( scaler );

        AntiSquish* as = new AntiSquish;
        as->addChild( at );
        addChild( as );

        _xDragger = new RotateCylinderDragger();
        scaler->addChild( _xDragger.get() );
        addDragger( _xDragger.get() );

        _yDragger = new RotateCylinderDragger();
        scaler->addChild( _yDragger.get() );
        addDragger( _yDragger.get() );

        _zDragger = new RotateCylinderDragger();
        scaler->addChild( _zDragger.get() );
        addDragger( _zDragger.get() );

        _xyzDragger = new RotateSphereDragger();
        scaler->addChild( _xyzDragger.get() );
        addDragger( _xyzDragger.get() );
    }
    else
    {
        _xDragger = new RotateCylinderDragger();
        addChild( _xDragger.get() );
        addDragger( _xDragger.get() );

        _yDragger = new RotateCylinderDragger();
        addChild( _yDragger.get() );
        addDragger( _yDragger.get() );

        _zDragger = new RotateCylinderDragger();
        addChild( _zDragger.get() );
        addDragger( _zDragger.get() );

        _xyzDragger = new RotateSphereDragger();
        addChild( _xyzDragger.get() );
        addDragger( _xyzDragger.get() );
    }

    _axisLineWidth      = 2.0F;
    _pickCylinderHeight = 0.15F;

    setParentDragger( getParentDragger() );
}

TrackballDragger::~TrackballDragger()
{
}

void
TrackballDragger::setupDefaultGeometry()
{
    _geode = new osg::Geode;
    {
        osg::TessellationHints* hints = new osg::TessellationHints;
        hints->setCreateTop( false );
        hints->setCreateBottom( false );
        hints->setCreateBackFace( false );

        _cylinder = new osg::Cylinder;
        _cylinder->setHeight( _pickCylinderHeight );
        osg::ShapeDrawable* cylinderDrawable =
            new osg::ShapeDrawable( _cylinder.get(), hints );
        _geode->addDrawable( cylinderDrawable );
        setDrawableToAlwaysCull( *cylinderDrawable );
        _geode->addDrawable( createCircleGeometry( 1.0F, 100 ) );
    }

    // Draw in line mode.
    {
        osg::PolygonMode* polymode = new osg::PolygonMode;
        polymode->setMode( osg::PolygonMode::Face::FRONT_AND_BACK,
                           osg::PolygonMode::Mode::LINE );
        _geode->getOrCreateStateSet()->setAttributeAndModes(
            polymode,
            osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON
        );
        _lineWidth = new osg::LineWidth( _axisLineWidth );
        _geode->getOrCreateStateSet()->setAttributeAndModes( _lineWidth.get(),
                                                             osg::StateAttribute::ON );
    }

    // Add line to all the individual 1D draggers.
    _xDragger->addChild( _geode.get() );
    _yDragger->addChild( _geode.get() );
    _zDragger->addChild( _geode.get() );

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

    // Add invisible sphere for pick the spherical dragger.
    {
        osg::Drawable* sphereDrawable = new osg::ShapeDrawable( new osg::Sphere() );
        setDrawableToAlwaysCull( *sphereDrawable );
        osg::Geode* sphereGeode = new osg::Geode;
        sphereGeode->addDrawable( sphereDrawable );

        _xyzDragger->addChild( sphereGeode );
    }
}

void
TrackballDragger::setAxisLineWidth( float linePixelWidth )
{
    _axisLineWidth = linePixelWidth;
    if( _lineWidth.valid() )
    {
        _lineWidth->setWidth( linePixelWidth );
    }
}

void
TrackballDragger::setPickCylinderHeight( float pickCylinderHeight )
{
    _pickCylinderHeight = pickCylinderHeight;
    if( _cylinder.valid() )
    {
        _cylinder->setHeight( pickCylinderHeight );
    }
}
