/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Planar dragger with corner/edge tabs for 2D scale and
 * translate in a single plane.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/TabPlaneDragger>

#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/quat.hpp>
#include <osg/nodes/AutoTransform.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/LineWidth.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osgManipulator/AntiSquish>

using namespace osgManipulator;

namespace
{

    osg::Node*
    createHandleNode( Scale2DDragger* cornerScaleDragger,
                      float           handleScaleFactor,
                      bool            twosided )
    {
        osg::Vec3Array* vertices = new osg::Vec3Array( 4 );
        ( *vertices )[0] =
            osg::vec3(
                static_cast<float>( cornerScaleDragger->getTopLeftHandlePosition()[0] ),
                0.0F,
                static_cast<float>( cornerScaleDragger->getTopLeftHandlePosition()[1] )
            ) *
            handleScaleFactor;
        ( *vertices )[1] =
            osg::vec3( static_cast<float>(
                           cornerScaleDragger->getBottomLeftHandlePosition()[0]
                       ),
                       0.0F,
                       static_cast<float>(
                           cornerScaleDragger->getBottomLeftHandlePosition()[1]
                       ) ) *
            handleScaleFactor;
        ( *vertices )[2] =
            osg::vec3( static_cast<float>(
                           cornerScaleDragger->getBottomRightHandlePosition()[0]
                       ),
                       0.0F,
                       static_cast<float>(
                           cornerScaleDragger->getBottomRightHandlePosition()[1]
                       ) ) *
            handleScaleFactor;
        ( *vertices )[3] =
            osg::vec3(
                static_cast<float>( cornerScaleDragger->getTopRightHandlePosition()[0] ),
                0.0F,
                static_cast<float>( cornerScaleDragger->getTopRightHandlePosition()[1] )
            ) *
            handleScaleFactor;

        osg::Geometry* geometry = new osg::Geometry();
        geometry->setVertexArray( vertices );
        {
            osg::DrawElementsUShort* elements =
                new osg::DrawElementsUShort( osg::PrimitiveSet::TRIANGLES, 6 );
            ( *elements )[0] = 0;
            ( *elements )[1] = 1;
            ( *elements )[2] = 3;
            ( *elements )[3] = 1;
            ( *elements )[4] = 2;
            ( *elements )[5] = 3;
            geometry->addPrimitiveSet( elements );
        }

        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back( osg::vec3( 0.0, 1.0, 0.0 ) );
        geometry->setNormalArray( normals, osg::Array::BIND_OVERALL );

        osg::Geode* geode = new osg::Geode;
        geode->setName( "Dragger Handle" );
        geode->addDrawable( geometry );

        if( !twosided )
        {
            osg::CullFace* cullface = new osg::CullFace;
            cullface->setMode( osg::CullFace::Mode::FRONT );
            geode->getOrCreateStateSet()->setAttribute(
                cullface,
                osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
            );
            geode->getOrCreateStateSet()->setMode( GL_CULL_FACE,
                                                   osg::StateAttribute::ON |
                                                       osg::StateAttribute::OVERRIDE );
        }

        // GL_LIGHTING removed: not in core profile

        return geode;
    }

    osg::Node*
    createHandleScene( const osg::vec3& pos,
                       osg::Node*       handleNode,
                       float            handleScaleFactor )
    {
        osg::AutoTransform* at = new osg::AutoTransform;
        at->setPosition( osg::dvec3( pos ) );
        at->setPivotPoint( osg::dvec3( pos * handleScaleFactor ) );
        at->setAutoScaleToScreen( true );
        at->addChild( handleNode );

        AntiSquish* as = new AntiSquish;
        as->setPivot( osg::dvec3( pos ) );
        as->addChild( at );

        return as;
    }

    void
    createCornerScaleDraggerGeometry( Scale2DDragger* cornerScaleDragger,
                                      osg::Node*      handleNode,
                                      float           handleScaleFactor )
    {
        // Create a top left box.
        {
            osg::Node* handleScene = createHandleScene(
                osg::vec3( static_cast<float>(
                               cornerScaleDragger->getTopLeftHandlePosition()[0]
                           ),
                           0.0F,
                           static_cast<float>(
                               cornerScaleDragger->getTopLeftHandlePosition()[1]
                           ) ),
                handleNode,
                handleScaleFactor
            );
            cornerScaleDragger->addChild( handleScene );
            cornerScaleDragger->setTopLeftHandleNode( *handleScene );
        }

        // Create a bottom left box.
        {
            osg::Node* handleScene = createHandleScene(
                osg::vec3( static_cast<float>(
                               cornerScaleDragger->getBottomLeftHandlePosition()[0]
                           ),
                           0.0F,
                           static_cast<float>(
                               cornerScaleDragger->getBottomLeftHandlePosition()[1]
                           ) ),
                handleNode,
                handleScaleFactor
            );
            cornerScaleDragger->addChild( handleScene );
            cornerScaleDragger->setBottomLeftHandleNode( *handleScene );
        }

        // Create a bottom right box.
        {
            osg::Node* handleScene = createHandleScene(
                osg::vec3( static_cast<float>(
                               cornerScaleDragger->getBottomRightHandlePosition()[0]
                           ),
                           0.0F,
                           static_cast<float>(
                               cornerScaleDragger->getBottomRightHandlePosition()[1]
                           ) ),
                handleNode,
                handleScaleFactor
            );
            cornerScaleDragger->addChild( handleScene );
            cornerScaleDragger->setBottomRightHandleNode( *handleScene );
        }

        // Create a top right box.
        {
            osg::Node* handleScene = createHandleScene(
                osg::vec3( static_cast<float>(
                               cornerScaleDragger->getTopRightHandlePosition()[0]
                           ),
                           0.0F,
                           static_cast<float>(
                               cornerScaleDragger->getTopRightHandlePosition()[1]
                           ) ),
                handleNode,
                handleScaleFactor
            );
            cornerScaleDragger->addChild( handleScene );
            cornerScaleDragger->setTopRightHandleNode( *handleScene );
        }
    }

    void
    createEdgeScaleDraggerGeometry( Scale1DDragger* horzEdgeScaleDragger,
                                    Scale1DDragger* vertEdgeScaleDragger,
                                    osg::Node*      handleNode,
                                    float           handleScaleFactor )
    {
        // Create a left box.
        {
            osg::Node* handleScene = createHandleScene(
                osg::vec3(
                    static_cast<float>( horzEdgeScaleDragger->getLeftHandlePosition() ),
                    0.0F,
                    0.0F
                ),
                handleNode,
                handleScaleFactor
            );
            horzEdgeScaleDragger->addChild( handleScene );
            horzEdgeScaleDragger->setLeftHandleNode( *handleScene );
        }

        // Create a right box.
        {
            osg::Node* handleScene = createHandleScene(
                osg::vec3(
                    static_cast<float>( horzEdgeScaleDragger->getRightHandlePosition() ),
                    0.0F,
                    0.0F
                ),
                handleNode,
                handleScaleFactor
            );
            horzEdgeScaleDragger->addChild( handleScene );
            horzEdgeScaleDragger->setRightHandleNode( *handleScene );
        }

        // Create a top box.
        {
            osg::Node* handleScene = createHandleScene(
                osg::vec3(
                    static_cast<float>( vertEdgeScaleDragger->getLeftHandlePosition() ),
                    0.0F,
                    0.0F
                ),
                handleNode,
                handleScaleFactor
            );
            vertEdgeScaleDragger->addChild( handleScene );
            vertEdgeScaleDragger->setLeftHandleNode( *handleScene );
        }

        // Create a bottom box.
        {
            osg::Node* handleScene = createHandleScene(
                osg::vec3(
                    static_cast<float>( vertEdgeScaleDragger->getRightHandlePosition() ),
                    0.0F,
                    0.0F
                ),
                handleNode,
                handleScaleFactor
            );
            vertEdgeScaleDragger->addChild( handleScene );
            vertEdgeScaleDragger->setRightHandleNode( *handleScene );
        }

        osg::quat rotation( osg::vec3( 0.0F, 0.0F, 1.0F ),
                            osg::vec3( 1.0F, 0.0F, 0.0F ) );
        vertEdgeScaleDragger->setMatrix( osg::rotate( osg::dquat( rotation ) ) );
    }

    void
    createTranslateDraggerGeometry( Scale2DDragger*        cornerScaleDragger,
                                    TranslatePlaneDragger* translateDragger )
    {
        // Create a polygon.
        {
            osg::Geode*     geode    = new osg::Geode;
            osg::Geometry*  geometry = new osg::Geometry();

            osg::Vec3Array* vertices = new osg::Vec3Array( 4 );
            ( *vertices )[0]         = osg::vec3(
                static_cast<float>( cornerScaleDragger->getTopLeftHandlePosition()[0] ),
                0.0F,
                static_cast<float>( cornerScaleDragger->getTopLeftHandlePosition()[1] )
            );
            ( *vertices )[1] =
                osg::vec3( static_cast<float>(
                               cornerScaleDragger->getBottomLeftHandlePosition()[0]
                           ),
                           0.0F,
                           static_cast<float>(
                               cornerScaleDragger->getBottomLeftHandlePosition()[1]
                           ) );
            ( *vertices )[2] =
                osg::vec3( static_cast<float>(
                               cornerScaleDragger->getBottomRightHandlePosition()[0]
                           ),
                           0.0F,
                           static_cast<float>(
                               cornerScaleDragger->getBottomRightHandlePosition()[1]
                           ) );
            ( *vertices )[3] = osg::vec3(
                static_cast<float>( cornerScaleDragger->getTopRightHandlePosition()[0] ),
                0.0F,
                static_cast<float>( cornerScaleDragger->getTopRightHandlePosition()[1] )
            );

            geometry->setVertexArray( vertices );
            {
                osg::DrawElementsUShort* elements =
                    new osg::DrawElementsUShort( osg::PrimitiveSet::TRIANGLES, 6 );
                ( *elements )[0] = 0;
                ( *elements )[1] = 1;
                ( *elements )[2] = 3;
                ( *elements )[3] = 1;
                ( *elements )[4] = 2;
                ( *elements )[5] = 3;
                geometry->addPrimitiveSet( elements );
            }

            osg::Vec3Array* normals = new osg::Vec3Array;
            normals->push_back( osg::vec3( 0.0, 1.0, 0.0 ) );
            geometry->setNormalArray( normals, osg::Array::BIND_OVERALL );

            geode->addDrawable( geometry );

            osg::PolygonMode* polymode = new osg::PolygonMode;
            polymode->setMode( osg::PolygonMode::Face::FRONT_AND_BACK,
                               osg::PolygonMode::Mode::LINE );
            geode->getOrCreateStateSet()->setAttributeAndModes(
                polymode,
                osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON
            );

            // GL_LIGHTING removed: not in core profile

            translateDragger->getTranslate2DDragger()->addChild( geode );
        }
    }

}

TabPlaneDragger::TabPlaneDragger( float handleScaleFactor ) :
    _handleScaleFactor( handleScaleFactor )
{
    _cornerScaleDragger =
        new Scale2DDragger( Scale2DDragger::SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT );
    addChild( _cornerScaleDragger.get() );
    addDragger( _cornerScaleDragger.get() );

    _horzEdgeScaleDragger =
        new Scale1DDragger( Scale1DDragger::SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT );
    addChild( _horzEdgeScaleDragger.get() );
    addDragger( _horzEdgeScaleDragger.get() );

    _vertEdgeScaleDragger =
        new Scale1DDragger( Scale1DDragger::SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT );
    addChild( _vertEdgeScaleDragger.get() );
    addDragger( _vertEdgeScaleDragger.get() );

    _translateDragger = new TranslatePlaneDragger();
    _translateDragger->setColor( osg::vec4( 0.7F, 0.7F, 0.7F, 1.0F ) );
    addChild( _translateDragger.get() );
    addDragger( _translateDragger.get() );

    setParentDragger( getParentDragger() );
}

TabPlaneDragger::~TabPlaneDragger()
{
}

bool
TabPlaneDragger::handle( const PointerInfo&            pointer,
                         const osgGA::GUIEventAdapter& ea,
                         osgGA::GUIActionAdapter&      aa )
{
    // Check if the dragger node is in the nodepath.
    if( !pointer.contains( this ) )
    {
        return false;
    }

    // Since the translate plane and the handleNode lie on the same plane the hit
    // could've been on either one. But we need to handle the scaling draggers before the
    // translation. Check if the node path has the scaling nodes else check for the
    // scaling nodes in next hit.
    if( _cornerScaleDragger->handle( pointer, ea, aa ) )
    {
        return true;
    }
    if( _horzEdgeScaleDragger->handle( pointer, ea, aa ) )
    {
        return true;
    }
    if( _vertEdgeScaleDragger->handle( pointer, ea, aa ) )
    {
        return true;
    }

    PointerInfo nextPointer( pointer );
    nextPointer.next();

    while( !nextPointer.completed() )
    {
        if( _cornerScaleDragger->handle( nextPointer, ea, aa ) )
        {
            return true;
        }
        if( _horzEdgeScaleDragger->handle( nextPointer, ea, aa ) )
        {
            return true;
        }
        if( _vertEdgeScaleDragger->handle( nextPointer, ea, aa ) )
        {
            return true;
        }

        nextPointer.next();
    }

    if( _translateDragger->handle( pointer, ea, aa ) )
    {
        return true;
    }

    return false;
}

void
TabPlaneDragger::setupDefaultGeometry( bool twoSidedHandle )
{
    osg::ref_ptr<osg::Node> handleNode = createHandleNode( _cornerScaleDragger.get(),
                                                           _handleScaleFactor,
                                                           twoSidedHandle );

    createCornerScaleDraggerGeometry( _cornerScaleDragger.get(),
                                      handleNode.get(),
                                      _handleScaleFactor );
    createEdgeScaleDraggerGeometry( _horzEdgeScaleDragger.get(),
                                    _vertEdgeScaleDragger.get(),
                                    handleNode.get(),
                                    _handleScaleFactor );
    createTranslateDraggerGeometry( _cornerScaleDragger.get(), _translateDragger.get() );
}
