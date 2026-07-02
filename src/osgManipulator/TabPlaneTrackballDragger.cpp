/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Combined TabPlane and Trackball dragger. Provides 2D scale,
 * translate, and 3D rotation in one composite manipulator.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/TabPlaneTrackballDragger.hpp>

#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/quat.hpp>
#include <osg/nodes/AutoTransform.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/LineWidth.hpp>
#include <osg/state/PolygonMode.hpp>

using namespace osgManipulator;

TabPlaneTrackballDragger::TabPlaneTrackballDragger()
{
    _trackballDragger = new TrackballDragger( true );
    addChild( _trackballDragger.get() );
    addDragger( _trackballDragger.get() );

    _tabPlaneDragger = new TabPlaneDragger();
    addChild( _tabPlaneDragger.get() );
    addDragger( _tabPlaneDragger.get() );

    setParentDragger( getParentDragger() );
}

TabPlaneTrackballDragger::~TabPlaneTrackballDragger()
{
}

void
TabPlaneTrackballDragger::setupDefaultGeometry()
{
    _trackballDragger->setupDefaultGeometry();
    _tabPlaneDragger->setupDefaultGeometry( true );
}
