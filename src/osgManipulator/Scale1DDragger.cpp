/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Single-axis scaling dragger. Constrains scale to one
 * axis via a linear handle.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/Scale1DDragger.hpp>

#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/LineWidth.hpp>
#include <osg/state/Material.hpp>
#include <osgManipulator/Command.hpp>

using namespace osgManipulator;

namespace
{

    double
    computeScale( const osg::dvec3& startProjectedPoint,
                  const osg::dvec3& projectedPoint,
                  double            scaleCenter )
    {
        double denom = startProjectedPoint[0] - scaleCenter;
        double scale = denom != 0.0 ? ( projectedPoint[0] - scaleCenter ) / denom : 1.0;
        return scale;
    }

}

Scale1DDragger::Scale1DDragger( ScaleMode scaleMode ) :
    Dragger(),
    _scaleCenter( 0.0 ),
    _minScale( 0.001 ),
    _scaleMode( scaleMode )
{
    _projector =
        new LineProjector( osg::dvec3( -0.5, 0.0, 0.0 ), osg::dvec3( 0.5, 0.0, 0.0 ) );
    setColor( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    setPickColor( osg::vec4( 1.0F, 1.0F, 0.0F, 1.0F ) );
}

Scale1DDragger::~Scale1DDragger()
{
}

bool
Scale1DDragger::handle( const PointerInfo&            pointer,
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
                    _scaleCenter = 0.0;
                    if( _scaleMode == SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT )
                    {
                        if( pointer.contains( _leftHandleNode.get() ) )
                        {
                            _scaleCenter = _projector->getLineEnd()[0];
                        }
                        else if( pointer.contains( _rightHandleNode.get() ) )
                        {
                            _scaleCenter = _projector->getLineStart()[0];
                        }
                    }

                    // Generate the motion command.
                    osg::ref_ptr<Scale1DCommand> cmd = new Scale1DCommand();
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
                    osg::ref_ptr<Scale1DCommand> cmd = new Scale1DCommand();

                    // Compute scale.
                    double scale = computeScale( _startProjectedPoint,
                                                 projectedPoint,
                                                 _scaleCenter );
                    if( scale < getMinScale() )
                    {
                        scale = getMinScale();
                    }

                    // Snap the referencePoint to the line start or line end depending on
                    // which is closer.
                    double referencePoint = _startProjectedPoint[0];
                    if( fabs( _projector->getLineStart()[0] - referencePoint ) <
                        fabs( _projector->getLineEnd()[0] - referencePoint ) )
                    {
                        referencePoint = _projector->getLineStart()[0];
                    }
                    else
                    {
                        referencePoint = _projector->getLineEnd()[0];
                    }

                    cmd->setStage( MotionCommand::MOVE );
                    cmd->setLocalToWorldAndWorldToLocal( _projector->getLocalToWorld(),
                                                         _projector->getWorldToLocal() );
                    cmd->setScale( scale );
                    cmd->setScaleCenter( _scaleCenter );
                    cmd->setReferencePoint( referencePoint );
                    cmd->setMinScale( getMinScale() );

                    // Dispatch command.
                    dispatch( *cmd );

                    aa.requestRedraw();
                }
                return true;
            }

        // Pick finish.
        case( osgGA::GUIEventAdapter::RELEASE ) :
            {
                osg::ref_ptr<Scale1DCommand> cmd = new Scale1DCommand();

                cmd->setStage( MotionCommand::FINISH );
                cmd->setLocalToWorldAndWorldToLocal( _projector->getLocalToWorld(),
                                                     _projector->getWorldToLocal() );

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
Scale1DDragger::setupDefaultGeometry()
{
    // Get the line length and direction.
    osg::dvec3  lineDirD   = _projector->getLineEnd() - _projector->getLineStart();
    float       lineLength = static_cast<float>( osg::length( lineDirD ) );

    osg::Geode* lineGeode  = new osg::Geode;
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

    // Create a left box.
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(
            new osg::Box( osg::vec3( _projector->getLineStart() ), 0.05F * lineLength )
        ) );
        addChild( geode );
        setLeftHandleNode( *geode );
    }

    // Create a right box.
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(
            new osg::Box( osg::vec3( _projector->getLineEnd() ), 0.05F * lineLength )
        ) );
        addChild( geode );
        setRightHandleNode( *geode );
    }
}
