/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osg/core/ref_ptr.hpp>

namespace osg
{

    class Geode;
    class Geometry;

}

namespace sponza
{

    osg::ref_ptr<osg::Geometry>
    createFullscreenQuadGeometry();

    osg::ref_ptr<osg::Geode>
    makeFullscreenPassGeode( const char* fragmentShader );

}
