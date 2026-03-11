/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Box with handle tabs for scaling and translation.
 * Provides face/edge/corner handles for 3D manipulation.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/TabBoxDragger>

#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/quat.hpp>
#include <osg/state/LineWidth.hpp>

using namespace osgManipulator;

TabBoxDragger::TabBoxDragger()
{
    for( std::size_t i = 0; i < 6; ++i )
    {
        _planeDraggers.push_back( new TabPlaneDragger() );
        addChild( _planeDraggers[i].get() );
        addDragger( _planeDraggers[i].get() );
    }

    {
        _planeDraggers[0]->setMatrix( osg::translate( osg::dvec3( 0.0, 0.5, 0.0 ) ) );
    }
    {
        osg::quat rotation( osg::vec3( 0.0F, -1.0F, 0.0F ),
                            osg::vec3( 0.0F, 1.0F, 0.0F ) );
        _planeDraggers[1]->setMatrix( osg::rotate( osg::dquat( rotation ) ) *
                                      osg::translate( osg::dvec3( 0.0, -0.5, 0.0 ) ) );
    }
    {
        osg::quat rotation( osg::vec3( 0.0F, 0.0F, 1.0F ),
                            osg::vec3( 0.0F, 1.0F, 0.0F ) );
        _planeDraggers[2]->setMatrix( osg::rotate( osg::dquat( rotation ) ) *
                                      osg::translate( osg::dvec3( 0.0, 0.0, -0.5 ) ) );
    }

    {
        osg::quat rotation( osg::vec3( 0.0F, 1.0F, 0.0F ),
                            osg::vec3( 0.0F, 0.0F, 1.0F ) );
        _planeDraggers[3]->setMatrix( osg::rotate( osg::dquat( rotation ) ) *
                                      osg::translate( osg::dvec3( 0.0, 0.0, 0.5 ) ) );
    }

    {
        osg::quat rotation( osg::vec3( 1.0F, 0.0F, 0.0F ),
                            osg::vec3( 0.0F, 1.0F, 0.0F ) );
        _planeDraggers[4]->setMatrix( osg::rotate( osg::dquat( rotation ) ) *
                                      osg::translate( osg::dvec3( -0.5, 0.0, 0.0 ) ) );
    }

    {
        osg::quat rotation( osg::vec3( 0.0F, 1.0F, 0.0F ),
                            osg::vec3( 1.0F, 0.0F, 0.0F ) );
        _planeDraggers[5]->setMatrix( osg::rotate( osg::dquat( rotation ) ) *
                                      osg::translate( osg::dvec3( 0.5, 0.0, 0.0 ) ) );
    }

    setParentDragger( getParentDragger() );
}

TabBoxDragger::~TabBoxDragger()
{
}

void
TabBoxDragger::setupDefaultGeometry()
{
    for( unsigned int i = 0; i < _planeDraggers.size(); ++i )
    {
        _planeDraggers[i]->setupDefaultGeometry( false );
    }
}

void
TabBoxDragger::setPlaneColor( const osg::vec4& color )
{
    for( unsigned int i = 0; i < _planeDraggers.size(); ++i )
    {
        _planeDraggers[i]->setPlaneColor( color );
    }
}
