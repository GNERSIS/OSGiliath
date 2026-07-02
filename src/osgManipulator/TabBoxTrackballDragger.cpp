/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Combined TabBox and Trackball dragger. Provides scale,
 * translate, and rotation in one composite manipulator.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/TabBoxTrackballDragger.hpp>

#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/quat.hpp>
#include <osg/nodes/AutoTransform.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/LineWidth.hpp>
#include <osg/state/PolygonMode.hpp>

using namespace osgManipulator;

TabBoxTrackballDragger::TabBoxTrackballDragger()
{
    _trackballDragger = new TrackballDragger( true );
    addChild( _trackballDragger.get() );
    addDragger( _trackballDragger.get() );

    _tabBoxDragger = new TabBoxDragger();
    addChild( _tabBoxDragger.get() );
    addDragger( _tabBoxDragger.get() );

    setParentDragger( getParentDragger() );
}

TabBoxTrackballDragger::~TabBoxTrackballDragger()
{
}

void
TabBoxTrackballDragger::setupDefaultGeometry()
{
    _trackballDragger->setupDefaultGeometry();
    _tabBoxDragger->setupDefaultGeometry();
}
