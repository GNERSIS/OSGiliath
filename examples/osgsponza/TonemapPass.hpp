/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osg/core/ref_ptr.hpp>

namespace osg
{

    class Camera;
    class Geode;
    class Texture2D;

}

namespace sponza
{

    struct SponzaFrameContext;
    struct SponzaOptions;
    struct SponzaTargets;

    struct TonemapPassResult
    {
            osg::ref_ptr<osg::Camera>    resolveCamera;
            osg::ref_ptr<osg::Camera>    outputCamera;
            osg::ref_ptr<osg::Texture2D> resolvedColor;
            bool                         resolvedFxaa = false;
    };

    osg::ref_ptr<osg::Geode>
    createTonemapQuad( const SponzaOptions&      options,
                       SponzaTargets&            targets,
                       const SponzaFrameContext& frame );

    TonemapPassResult
    createTonemapPass( const SponzaOptions&      options,
                       SponzaTargets&            targets,
                       const SponzaFrameContext& frame );

    osg::ref_ptr<osg::Camera>
    createTonemapCamera( const SponzaOptions&      options,
                         SponzaTargets&            targets,
                         const SponzaFrameContext& frame );

}
