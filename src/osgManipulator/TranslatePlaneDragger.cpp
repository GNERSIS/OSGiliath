/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Plane translation dragger with a rectangular handle.
 * Constrains movement to the plane containing the handle.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/TranslatePlaneDragger.hpp>

#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/quat.hpp>
#include <osg/nodes/AutoTransform.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/LineWidth.hpp>
#include <osg/state/PolygonMode.hpp>

using namespace osgManipulator;

TranslatePlaneDragger::TranslatePlaneDragger() :
    _usingTranslate1DDragger( false )
{
    _translate2DDragger = new Translate2DDragger();
    _translate2DDragger->setColor( osg::vec4( 0.7F, 0.7F, 0.7F, 1.0F ) );
    addChild( _translate2DDragger.get() );
    addDragger( _translate2DDragger.get() );

    _translate1DDragger = new Translate1DDragger( osg::dvec3( 0.0, 0.0, 0.0 ),
                                                  osg::dvec3( 0.0, 1.0, 0.0 ) );
    _translate1DDragger->setCheckForNodeInNodePath( false );
    addChild( _translate1DDragger.get() );
    addDragger( _translate1DDragger.get() );

    setParentDragger( getParentDragger() );
}

TranslatePlaneDragger::~TranslatePlaneDragger()
{
}

bool
TranslatePlaneDragger::handle( const PointerInfo&            pointer,
                               const osgGA::GUIEventAdapter& ea,
                               osgGA::GUIActionAdapter&      aa )
{
    // Check if the dragger node is in the nodepath.
    if( !pointer.contains( this ) )
    {
        return false;
    }

    if( ( ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON ) &&
        ea.getEventType() == osgGA::GUIEventAdapter::PUSH )
    {
        _usingTranslate1DDragger = true;
    }

    bool handled = false;
    if( _usingTranslate1DDragger )
    {
        if( _translate1DDragger->handle( pointer, ea, aa ) )
        {
            handled = true;
        }
    }
    else
    {
        if( _translate2DDragger->handle( pointer, ea, aa ) )
        {
            handled = true;
        }
    }

    if( ea.getEventType() == osgGA::GUIEventAdapter::RELEASE )
    {
        _usingTranslate1DDragger = false;
    }

    return handled;
}

void
TranslatePlaneDragger::setupDefaultGeometry()
{
    // Create a polygon.
    {
        osg::Geode*     geode    = new osg::Geode;
        osg::Geometry*  geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array( 4 );
        ( *vertices )[0]         = osg::vec3( -0.5, 0.0, 0.5 );
        ( *vertices )[1]         = osg::vec3( -0.5, 0.0, -0.5 );
        ( *vertices )[2]         = osg::vec3( 0.5, 0.0, -0.5 );
        ( *vertices )[3]         = osg::vec3( 0.5, 0.0, 0.5 );

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

        _translate2DDragger->addChild( geode );
    }
}
