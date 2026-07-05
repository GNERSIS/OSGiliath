/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <cstddef>
#include <string>

namespace osg
{

    class Node;

}

namespace sponza
{

    struct SponzaOptions;

    struct VisibilityBakeResult
    {
            bool        enabled           = false;
            bool        loadedFromCache   = false;
            std::size_t geometryCount     = 0U;
            std::size_t vertexCount       = 0U;
            std::size_t occluderTriangles = 0U;
            double      wallTimeSeconds   = 0.0;
            std::string cachePath;
    };

    VisibilityBakeResult
    applyVisibilityBake( osg::Node*           model,
                         const SponzaOptions& options );

}
