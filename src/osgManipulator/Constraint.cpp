/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract constraint applied to dragger motion. Limits
 * movement to grids, planes, or custom regions.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/Constraint.hpp>

#include <math.h>
#include <osg/maths/vec2.hpp>
#include <osgManipulator/Command.hpp>
#include <osgManipulator/Dragger.hpp>

using namespace osgManipulator;

namespace
{

    double
    round_to_nearest_int( double x )
    {
        return floor( x + 0.5 );
    }

    osg::dvec3
    snap_point_to_grid( const osg::dvec3& point,
                        const osg::dvec3& origin,
                        const osg::dvec3& spacing )
    {
        osg::dvec3 scale;
        scale[0] = spacing[0] != 0.0
                     ? round_to_nearest_int( ( point[0] - origin[0] ) / spacing[0] )
                     : 1.0;
        scale[1] = spacing[1] != 0.0
                     ? round_to_nearest_int( ( point[1] - origin[1] ) / spacing[1] )
                     : 1.0;
        scale[2] = spacing[2] != 0.0
                     ? round_to_nearest_int( ( point[2] - origin[2] ) / spacing[2] )
                     : 1.0;
        osg::dvec3 snappedPoint  = origin;
        snappedPoint            += osg::dvec3( scale[0] * spacing[0],
                                               scale[1] * spacing[1],
                                               scale[2] * spacing[2] );
        return snappedPoint;
    }

}

void
Constraint::computeLocalToWorldAndWorldToLocal() const
{
    if( !_refNode )
    {
        _localToWorld = osg::dmat4();
        _worldToLocal = osg::dmat4();
        return;
    }

    osg::NodePath pathToRoot;
    computeNodePathToRoot( const_cast<osg::Node&>( getReferenceNode() ), pathToRoot );
    _localToWorld = osg::computeLocalToWorld( pathToRoot );
    _worldToLocal = osg::computeWorldToLocal( pathToRoot );
}

GridConstraint::GridConstraint( osg::Node&        refNode,
                                const osg::dvec3& origin,
                                const osg::dvec3& spacing ) :
    Constraint( refNode ),
    _origin( origin ),
    _spacing( spacing )
{
}

bool
GridConstraint::constrain( TranslateInLineCommand& command ) const
{
    if( command.getStage() == osgManipulator::MotionCommand::START )
    {
        computeLocalToWorldAndWorldToLocal();
    }
    else if( command.getStage() == osgManipulator::MotionCommand::FINISH )
    {
        return true;
    }

    osg::dvec3 translatedPoint      = command.getLineStart() + command.getTranslation();
    osg::dvec3 localTranslatedPoint = ( osg::dvec3( translatedPoint ) *
                                        command.getLocalToWorld() *
                                        getWorldToLocal() );
    osg::dvec3 newLocalTranslatedPoint =
        snap_point_to_grid( localTranslatedPoint, _origin, _spacing );
    command.setTranslation( newLocalTranslatedPoint *
                            getLocalToWorld() *
                            command.getWorldToLocal() -
                            command.getLineStart() );

    return true;
}

bool
GridConstraint::constrain( TranslateInPlaneCommand& command ) const
{
    if( command.getStage() == osgManipulator::MotionCommand::START )
    {
        computeLocalToWorldAndWorldToLocal();
    }
    else if( command.getStage() == osgManipulator::MotionCommand::FINISH )
    {
        return true;
    }

    osg::dmat4 commandToConstraint = command.getLocalToWorld() * getWorldToLocal();
    osg::dmat4 constraintToCommand = getLocalToWorld() * command.getWorldToLocal();

    // Snap the reference point to grid.
    osg::dvec3 localRefPoint = command.getReferencePoint() * commandToConstraint;
    osg::dvec3 snappedLocalRefPoint =
        snap_point_to_grid( localRefPoint, _origin, _spacing );
    osg::dvec3 snappedCmdRefPoint = snappedLocalRefPoint * constraintToCommand;

    // Snap the translated point to grid.
    osg::dvec3 translatedPoint = snappedCmdRefPoint + command.getTranslation();
    osg::dvec3 localTranslatedPoint =
        osg::dvec3( translatedPoint ) * commandToConstraint;
    osg::dvec3 newLocalTranslatedPoint =
        snap_point_to_grid( localTranslatedPoint, _origin, _spacing );

    // Set the snapped translation.
    command.setTranslation(
        newLocalTranslatedPoint * constraintToCommand - snappedCmdRefPoint
    );

    return true;
}

bool
GridConstraint::constrain( Scale1DCommand& command ) const
{
    if( command.getStage() == osgManipulator::MotionCommand::START )
    {
        computeLocalToWorldAndWorldToLocal();
    }
    else if( command.getStage() == osgManipulator::MotionCommand::FINISH )
    {
        return true;
    }

    double     scaledPoint = ( command.getReferencePoint() - command.getScaleCenter() ) *
                             command.getScale() +
                             command.getScaleCenter();

    osg::dmat4 constraintToCommand = getLocalToWorld() * command.getWorldToLocal();
    osg::dvec3 commandOrigin       = _origin * constraintToCommand;
    osg::dvec3 commandSpacing =
        ( _origin + _spacing ) * constraintToCommand - commandOrigin;

    double spacingFactor = commandSpacing[0] != 0.0
                             ? round_to_nearest_int( ( scaledPoint - commandOrigin[0] ) /
                                                     commandSpacing[0] )
                             : 1.0;

    double snappedScaledPoint = commandOrigin[0] + commandSpacing[0] * spacingFactor;

    double denom        = ( command.getReferencePoint() - command.getScaleCenter() );
    double snappedScale = ( denom != 0.0 )
                            ? ( snappedScaledPoint - command.getScaleCenter() ) / denom
                            : 1.0;
    if( snappedScale < command.getMinScale() )
    {
        snappedScale = command.getMinScale();
    }

    command.setScale( snappedScale );
    return true;
}

bool
GridConstraint::constrain( Scale2DCommand& command ) const
{
    if( command.getStage() == osgManipulator::MotionCommand::START )
    {
        computeLocalToWorldAndWorldToLocal();
    }
    else if( command.getStage() == osgManipulator::MotionCommand::FINISH )
    {
        return true;
    }

    osg::dvec2 scaledPoint  = command.getReferencePoint() - command.getScaleCenter();
    scaledPoint[0]         *= command.getScale()[0];
    scaledPoint[1]         *= command.getScale()[1];
    scaledPoint            += command.getScaleCenter();

    osg::dmat4 constraintToCommand = getLocalToWorld() * command.getWorldToLocal();
    osg::dvec3 commandOrigin       = _origin * constraintToCommand;
    osg::dvec3 commandSpacing =
        ( _origin + _spacing ) * constraintToCommand - commandOrigin;

    osg::dvec2 spacingFactor;
    spacingFactor[0] = commandSpacing[0] != 0.0
                         ? round_to_nearest_int( ( scaledPoint[0] - commandOrigin[0] ) /
                                                 commandSpacing[0] )
                         : 1.0;
    spacingFactor[1] = commandSpacing[2] != 0.0
                         ? round_to_nearest_int( ( scaledPoint[1] - commandOrigin[2] ) /
                                                 commandSpacing[2] )
                         : 1.0;

    osg::dvec2 snappedScaledPoint =
        ( osg::dvec2( commandOrigin[0], commandOrigin[2] ) +
          osg::dvec2( commandSpacing[0] * spacingFactor[0],
                      commandSpacing[2] * spacingFactor[1] ) );

    osg::dvec2 denom = command.getReferencePoint() - command.getScaleCenter();
    osg::dvec2 snappedScale;
    snappedScale[0] =
        denom[0] != 0.0
            ? ( snappedScaledPoint[0] - command.getScaleCenter()[0] ) / denom[0]
            : 1.0;
    snappedScale[1] =
        denom[1] != 0.0
            ? ( snappedScaledPoint[1] - command.getScaleCenter()[1] ) / denom[1]
            : 1.0;

    if( snappedScale[0] < command.getMinScale()[0] )
    {
        snappedScale[0] = command.getMinScale()[0];
    }
    if( snappedScale[1] < command.getMinScale()[1] )
    {
        snappedScale[1] = command.getMinScale()[1];
    }

    command.setScale( snappedScale );
    return true;
}

bool
GridConstraint::constrain( ScaleUniformCommand& ) const
{
    // Can you correctly snap a ScaleUniformCommand using a Grid constraint that has
    // different spacings in the three axis??
    return false;
}
