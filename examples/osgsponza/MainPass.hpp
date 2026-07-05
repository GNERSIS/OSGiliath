/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osg/core/ref_ptr.hpp>

namespace osg
{

    class Camera;
    class Node;

}

namespace sponza
{

    struct SponzaFrameContext;
    struct SponzaOptions;
    struct SponzaTargets;

    osg::ref_ptr<osg::Camera>
    createRttCamera( osg::Node*                model,
                     const SponzaOptions&      options,
                     SponzaTargets&            targets,
                     const SponzaFrameContext& frame );

}
