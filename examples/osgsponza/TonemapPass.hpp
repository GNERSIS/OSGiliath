/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osg/core/ref_ptr.hpp>

namespace osg
{

    class Camera;
    class Geode;

}

namespace sponza
{

    struct SponzaFrameContext;
    struct SponzaOptions;
    struct SponzaTargets;

    osg::ref_ptr<osg::Geode>
    createTonemapQuad( const SponzaOptions&      options,
                       SponzaTargets&            targets,
                       const SponzaFrameContext& frame );

    osg::ref_ptr<osg::Camera>
    createTonemapCamera( const SponzaOptions&      options,
                         SponzaTargets&            targets,
                         const SponzaFrameContext& frame );

}
