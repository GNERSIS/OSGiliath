/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Single-axis translation dragger. Constrains movement to
 * one axis for precise linear positioning.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/Translate1DDragger.hpp>

#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/LineWidth.hpp>
#include <osg/state/Material.hpp>
#include <osgManipulator/Command.hpp>

using namespace osgManipulator;

Translate1DDragger::Translate1DDragger() :
    Dragger(),
    _checkForNodeInNodePath( true )
{
    _projector = new LineProjector;
    setColor( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    setPickColor( osg::vec4( 1.0F, 1.0F, 0.0F, 1.0F ) );
}

Translate1DDragger::Translate1DDragger( const osg::dvec3& s,
                                        const osg::dvec3& e ) :
    Dragger(),
    _checkForNodeInNodePath( true )
{
    _projector = new LineProjector( s, e );
    setColor( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    setPickColor( osg::vec4( 1.0F, 1.0F, 0.0F, 1.0F ) );
}

Translate1DDragger::~Translate1DDragger()
{
}

bool
Translate1DDragger::handle( const PointerInfo&            pointer,
                            const osgGA::GUIEventAdapter& ea,
                            osgGA::GUIActionAdapter&      aa )
{
    // Check if the dragger node is in the nodepath.
    if( _checkForNodeInNodePath )
    {
        if( !pointer.contains( this ) )
        {
            return false;
        }
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
                    osg::ref_ptr<TranslateInLineCommand> cmd =
                        new TranslateInLineCommand( _projector->getLineStart(),
                                                    _projector->getLineEnd() );
                    cmd->setStage( MotionCommand::START );
                    cmd->setLocalToWorldAndWorldToLocal( _projector->getLocalToWorld(),
                                                         _projector->getWorldToLocal() );

                    // Dispatch command.
                    dispatch( *cmd );

                    // Set color to pick color.
                    setMaterialColor( _pickColor, *this );

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
                    osg::ref_ptr<TranslateInLineCommand> cmd =
                        new TranslateInLineCommand( _projector->getLineStart(),
                                                    _projector->getLineEnd() );
                    cmd->setStage( MotionCommand::MOVE );
                    cmd->setLocalToWorldAndWorldToLocal( _projector->getLocalToWorld(),
                                                         _projector->getWorldToLocal() );
                    cmd->setTranslation( projectedPoint - _startProjectedPoint );

                    // Dispatch command.
                    dispatch( *cmd );

                    aa.requestRedraw();
                }
                return true;
            }

        // Pick finish.
        case( osgGA::GUIEventAdapter::RELEASE ) :
            {
                osg::dvec3 projectedPoint;
                if( _projector->project( pointer, projectedPoint ) )
                {
                    osg::ref_ptr<TranslateInLineCommand> cmd =
                        new TranslateInLineCommand( _projector->getLineStart(),
                                                    _projector->getLineEnd() );

                    cmd->setStage( MotionCommand::FINISH );
                    cmd->setLocalToWorldAndWorldToLocal( _projector->getLocalToWorld(),
                                                         _projector->getWorldToLocal() );

                    // Dispatch command.
                    dispatch( *cmd );

                    // Reset color.
                    setMaterialColor( _color, *this );

                    aa.requestRedraw();
                }

                return true;
            }
        default :
            return false;
    }
}

void
Translate1DDragger::setupDefaultGeometry()
{
    // Get the line length and direction.
    osg::dvec3  lineDirD   = _projector->getLineEnd() - _projector->getLineStart();
    float       lineLength = ( float )osg::length( lineDirD );
    osg::vec3   lineDir    = osg::vec3( osg::normalize( lineDirD ) );

    osg::Geode* geode      = new osg::Geode;
    // Create a left cone.
    {
        osg::Cone* cone = new osg::Cone( osg::vec3( _projector->getLineStart() ),
                                         0.025F * lineLength,
                                         0.10F * lineLength );
        osg::quat  rotation;
        rotation = osg::quat( lineDir, osg::vec3( 0.0F, 0.0F, 1.0F ) );
        cone->setRotation( rotation );

        geode->addDrawable( new osg::ShapeDrawable( cone ) );
    }

    // Create a right cone.
    {
        osg::Cone* cone = new osg::Cone( osg::vec3( _projector->getLineEnd() ),
                                         0.025F * lineLength,
                                         0.10F * lineLength );
        osg::quat  rotation;
        rotation = osg::quat( osg::vec3( 0.0F, 0.0F, 1.0F ), lineDir );
        cone->setRotation( rotation );

        geode->addDrawable( new osg::ShapeDrawable( cone ) );
    }

    // Create an invisible cylinder for picking the line.
    {
        osg::Cylinder* cylinder =
            new osg::Cylinder( osg::vec3( osg::dvec3( _projector->getLineStart() +
                                                      _projector->getLineEnd() ) *
                                          0.5 ),
                               0.015F * lineLength,
                               lineLength );
        osg::quat rotation;
        rotation = osg::quat( osg::vec3( 0.0F, 0.0F, 1.0F ), lineDir );
        cylinder->setRotation( rotation );
        osg::Drawable* cylinderGeom = new osg::ShapeDrawable( cylinder );

        setDrawableToAlwaysCull( *cylinderGeom );

        geode->addDrawable( cylinderGeom );
    }

    osg::Geode* lineGeode = new osg::Geode;
    // Create a line.
    {
        osg::Geometry*  geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array( 2 );
        ( *vertices )[0]         = _projector->getLineStart();
        ( *vertices )[1]         = _projector->getLineEnd();

        geometry->setVertexArray( vertices );
        geometry->addPrimitiveSet(
            new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 2 )
        );

        lineGeode->addDrawable( geometry );
    }

    // Turn of lighting for line and set line width.
    // GL_LIGHTING removed: not in core profile
    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth( 2.0F );
    lineGeode->getOrCreateStateSet()->setAttributeAndModes( linewidth,
                                                            osg::StateAttribute::ON );

    // Add line and cones to the scene.
    addChild( lineGeode );
    addChild( geode );
}
