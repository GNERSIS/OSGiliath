/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Rotation dragger using a sphere interface. Provides
 * unconstrained rotation via a virtual sphere surface.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/RotateSphereDragger>

#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/Material.hpp>
#include <osgManipulator/Command>

using namespace osgManipulator;

RotateSphereDragger::RotateSphereDragger() :
    _prevPtOnSphere( true )
{
    _projector = new SpherePlaneProjector();
    setColor( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    setPickColor( osg::vec4( 1.0F, 1.0F, 0.0F, 1.0F ) );
}

RotateSphereDragger::~RotateSphereDragger()
{
}

bool
RotateSphereDragger::handle( const PointerInfo&            pointer,
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

                _startLocalToWorld = _projector->getLocalToWorld();
                _startWorldToLocal = _projector->getWorldToLocal();

                if( _projector->isPointInFront( pointer, _startLocalToWorld ) )
                {
                    _projector->setFront( true );
                }
                else
                {
                    _projector->setFront( false );
                }

                osg::dvec3 projectedPoint;
                if( _projector->project( pointer, projectedPoint ) )
                {
                    // Generate the motion command.
                    osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();
                    cmd->setStage( MotionCommand::START );
                    cmd->setLocalToWorldAndWorldToLocal( _startLocalToWorld,
                                                         _startWorldToLocal );

                    // Dispatch command.
                    dispatch( *cmd );

                    // Set color to pick color.
                    setMaterialColor( _pickColor, *this );

                    _prevRotation    = osg::quat();
                    _prevWorldProjPt = projectedPoint * _projector->getLocalToWorld();
                    _prevPtOnSphere  = _projector->isProjectionOnSphere();

                    aa.requestRedraw();
                }

                return true;
            }

        // Pick move.
        case( osgGA::GUIEventAdapter::DRAG ) :
            {
                // Get the LocalToWorld matrix for this node and set it for the
                // projector.
                osg::dmat4 localToWorld =
                    osg::rotate( osg::dquat( _prevRotation ) ) * _startLocalToWorld;
                _projector->setLocalToWorld( localToWorld );

                osg::dvec3 projectedPoint;
                if( _projector->project( pointer, projectedPoint ) )
                {
                    osg::dvec3 prevProjectedPoint =
                        _prevWorldProjPt * _projector->getWorldToLocal();
                    osg::quat deltaRotation =
                        _projector->getRotation( prevProjectedPoint,
                                                 _prevPtOnSphere,
                                                 projectedPoint,
                                                 _projector->isProjectionOnSphere(),
                                                 1.0F );
                    osg::quat rotation = deltaRotation * _prevRotation;

                    // Generate the motion command.
                    osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();
                    cmd->setStage( MotionCommand::MOVE );
                    cmd->setLocalToWorldAndWorldToLocal( _startLocalToWorld,
                                                         _startWorldToLocal );
                    cmd->setRotation( rotation );

                    // Dispatch command.
                    dispatch( *cmd );

                    _prevWorldProjPt = projectedPoint * _projector->getLocalToWorld();
                    _prevRotation    = rotation;
                    _prevPtOnSphere  = _projector->isProjectionOnSphere();
                    aa.requestRedraw();
                }
                return true;
            }

        // Pick finish.
        case( osgGA::GUIEventAdapter::RELEASE ) :
            {
                osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();

                cmd->setStage( MotionCommand::FINISH );
                cmd->setLocalToWorldAndWorldToLocal( _startLocalToWorld,
                                                     _startWorldToLocal );

                // Dispatch command.
                dispatch( *cmd );

                // Reset color.
                setMaterialColor( _color, *this );

                aa.requestRedraw();

                return true;
            }
        default :
            return false;
    }
}

void
RotateSphereDragger::setupDefaultGeometry()
{
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(
        new osg::ShapeDrawable( const_cast<osg::Sphere*>( _projector->getSphere() ) )
    );
    addChild( geode );
}
