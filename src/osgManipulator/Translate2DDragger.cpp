/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Planar translation dragger. Constrains movement to a
 * 2D plane for planar positioning.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/Translate2DDragger.hpp>

#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/LineWidth.hpp>
#include <osg/state/Material.hpp>
#include <osgManipulator/Command.hpp>

using namespace osgManipulator;

Translate2DDragger::Translate2DDragger()
{
    _projector     = new PlaneProjector( osg::Plane( 0.0, 1.0, 0.0, 0.0 ) );
    _polygonOffset = new osg::PolygonOffset( -1.0F, -1.0F );
    setColor( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    setPickColor( osg::vec4( 1.0F, 1.0F, 0.0F, 1.0F ) );
}

Translate2DDragger::Translate2DDragger( const osg::Plane& plane )
{
    _projector     = new PlaneProjector( plane );
    _polygonOffset = new osg::PolygonOffset( -1.0F, -1.0F );
    setColor( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    setPickColor( osg::vec4( 1.0F, 1.0F, 0.0F, 1.0F ) );
}

Translate2DDragger::~Translate2DDragger()
{
}

bool
Translate2DDragger::handle( const PointerInfo&            pointer,
                            const osgGA::GUIEventAdapter& ea,
                            osgGA::GUIActionAdapter&      aa )
{
    // Check if the dragger node is in the nodepath.
    if( !pointer.contains( this ) )
    {
        return false;
    }

    switch( ea.getEventType() )
    {
        // Pick start.
        case( osgGA::GUIEventAdapter::PUSH ) :
            {
                // Get the LocalToWorld matrix for this node and set it for the
                // projector.
                osg::NodePath nodePathToRoot;
                computeNodePathToRoot( *this, nodePathToRoot );
                osg::dmat4 localToWorld = osg::computeLocalToWorld( nodePathToRoot );
                _projector->setLocalToWorld( localToWorld );

                if( _projector->project( pointer, _startProjectedPoint ) )
                {
                    // Generate the motion command.
                    osg::ref_ptr<TranslateInPlaneCommand> cmd =
                        new TranslateInPlaneCommand( _projector->getPlane() );

                    cmd->setStage( MotionCommand::START );
                    cmd->setReferencePoint( _startProjectedPoint );
                    cmd->setLocalToWorldAndWorldToLocal( _projector->getLocalToWorld(),
                                                         _projector->getWorldToLocal() );

                    // Dispatch command.
                    dispatch( *cmd );

                    // Set color to pick color.
                    setMaterialColor( _pickColor, *this );
                    getOrCreateStateSet()->setAttributeAndModes(
                        _polygonOffset.get(),
                        osg::StateAttribute::ON
                    );

                    aa.requestRedraw();
                }
                return true;
            }

        // Pick move.
        case( osgGA::GUIEventAdapter::DRAG ) :
            {
                osg::dvec3 projectedPoint;
                if( _projector->project( pointer, projectedPoint ) )
                {
                    // Generate the motion command.
                    osg::ref_ptr<TranslateInPlaneCommand> cmd =
                        new TranslateInPlaneCommand( _projector->getPlane() );

                    cmd->setStage( MotionCommand::MOVE );
                    cmd->setLocalToWorldAndWorldToLocal( _projector->getLocalToWorld(),
                                                         _projector->getWorldToLocal() );
                    cmd->setTranslation( projectedPoint - _startProjectedPoint );
                    cmd->setReferencePoint( _startProjectedPoint );

                    // Dispatch command.
                    dispatch( *cmd );

                    aa.requestRedraw();
                }
                return true;
            }

        // Pick finish.
        case( osgGA::GUIEventAdapter::RELEASE ) :
            {
                osg::ref_ptr<TranslateInPlaneCommand> cmd =
                    new TranslateInPlaneCommand( _projector->getPlane() );

                cmd->setStage( MotionCommand::FINISH );
                cmd->setReferencePoint( _startProjectedPoint );
                cmd->setLocalToWorldAndWorldToLocal( _projector->getLocalToWorld(),
                                                     _projector->getWorldToLocal() );

                // Dispatch command.
                dispatch( *cmd );

                // Reset color.
                setMaterialColor( _color, *this );
                getOrCreateStateSet()->removeAttribute( _polygonOffset.get() );

                aa.requestRedraw();

                return true;
            }
        default :
            return false;
    }
}

void
Translate2DDragger::setupDefaultGeometry()
{
    // Create a line.
    osg::Geode* lineGeode = new osg::Geode;
    {
        osg::Geometry*  geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array( 2 );
        ( *vertices )[0]         = osg::vec3( 0.0F, 0.0F, -0.5F );
        ( *vertices )[1]         = osg::vec3( 0.0F, 0.0F, 0.5F );

        geometry->setVertexArray( vertices );
        geometry->addPrimitiveSet(
            new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 2 )
        );

        lineGeode->addDrawable( geometry );
    }

    // Turn of lighting for line and set line width.
    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth( 2.0F );
    lineGeode->getOrCreateStateSet()->setAttributeAndModes( linewidth,
                                                            osg::StateAttribute::ON );
    // GL_LIGHTING removed: not in core profile

    osg::Geode* geode = new osg::Geode;

    // Create left cone.
    {
        osg::Cone* cone = new osg::Cone( osg::vec3( 0.0F, 0.0F, -0.5F ), 0.025F, 0.10F );
        osg::quat  rotation( osg::vec3( 0.0F, 0.0F, -1.0F ),
                             osg::vec3( 0.0F, 0.0F, 1.0F ) );
        cone->setRotation( rotation );
        geode->addDrawable( new osg::ShapeDrawable( cone ) );
    }

    // Create right cone.
    {
        osg::Cone* cone = new osg::Cone( osg::vec3( 0.0F, 0.0F, 0.5F ), 0.025F, 0.10F );
        geode->addDrawable( new osg::ShapeDrawable( cone ) );
    }

    // Create an invisible cylinder for picking the line.
    {
        osg::Cylinder* cylinder =
            new osg::Cylinder( osg::vec3( 0.0F, 0.0F, 0.0F ), 0.015F, 1.0F );
        osg::Drawable* drawable = new osg::ShapeDrawable( cylinder );
        setDrawableToAlwaysCull( *drawable );
        geode->addDrawable( drawable );
    }

    // MatrixTransform to rotate the geometry according to the normal of the plane.
    osg::MatrixTransform* xform = new osg::MatrixTransform;

    // Create an arrow in the X axis.
    {
        osg::MatrixTransform* arrow = new osg::MatrixTransform;
        arrow->addChild( lineGeode );
        arrow->addChild( geode );

        // Rotate X-axis arrow appropriately.
        osg::quat rotation( osg::vec3( 1.0F, 0.0F, 0.0F ),
                            osg::vec3( 0.0F, 0.0F, 1.0F ) );
        arrow->setMatrix( osg::rotate( osg::dquat( rotation ) ) );

        xform->addChild( arrow );
    }

    // Create an arrow in the Z axis.
    {
        osg::Group* arrow = new osg::Group;
        arrow->addChild( lineGeode );
        arrow->addChild( geode );

        xform->addChild( arrow );
    }

    // Rotate the xform so that the geometry lies on the plane.
    {
        osg::dvec3 normalD = _projector->getPlane().getNormal();
        normalD            = osg::normalize( normalD );
        osg::vec3 normal   = osg::vec3( normalD );
        osg::quat rotation( osg::vec3( 0.0F, 1.0F, 0.0F ), normal );
        xform->setMatrix( osg::rotate( osg::dquat( rotation ) ) );
    }

    addChild( xform );
}
