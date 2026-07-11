/* OSGiliath -- OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include "SponzaRayScene.hpp"

#include <cstddef>
#include <osg/core/ref_ptr.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/state/BufferIndexBinding.hpp>

namespace osg
{

    class Node;
    class StateSet;

}

namespace sponza
{

    struct SponzaOptions;

    constexpr unsigned int rtNodeMinBinding        = 0U;
    constexpr unsigned int rtNodeMaxBinding        = 1U;
    constexpr unsigned int rtNodeMetaBinding       = 2U;
    constexpr unsigned int rtTriangleIndexBinding  = 3U;
    constexpr unsigned int rtTriangleV0Binding     = 4U;
    constexpr unsigned int rtTriangleV1Binding     = 5U;
    constexpr unsigned int rtTriangleV2Binding     = 6U;
    constexpr unsigned int rtTriangleNormalBinding = 7U;

    struct GpuRayScene
    {
            osg::ref_ptr<osg::Vec4Array>                  nodeMin;
            osg::ref_ptr<osg::Vec4Array>                  nodeMax;
            osg::ref_ptr<osg::Vec4uiArray>                nodeMeta;
            osg::ref_ptr<osg::UIntArray>                  triangleIndices;
            osg::ref_ptr<osg::Vec4Array>                  triangleV0Edge;
            osg::ref_ptr<osg::Vec4Array>                  triangleV1;
            osg::ref_ptr<osg::Vec4Array>                  triangleV2;
            osg::ref_ptr<osg::Vec4Array>                  triangleNormal;

            osg::ref_ptr<osg::ShaderStorageBufferBinding> nodeMinBinding;
            osg::ref_ptr<osg::ShaderStorageBufferBinding> nodeMaxBinding;
            osg::ref_ptr<osg::ShaderStorageBufferBinding> nodeMetaBinding;
            osg::ref_ptr<osg::ShaderStorageBufferBinding> triangleIndexBinding;
            osg::ref_ptr<osg::ShaderStorageBufferBinding> triangleV0Binding;
            osg::ref_ptr<osg::ShaderStorageBufferBinding> triangleV1Binding;
            osg::ref_ptr<osg::ShaderStorageBufferBinding> triangleV2Binding;
            osg::ref_ptr<osg::ShaderStorageBufferBinding> triangleNormalBinding;

            std::size_t                                   nodeCount          = 0U;
            std::size_t                                   triangleCount      = 0U;
            std::size_t                                   triangleIndexCount = 0U;
            float                                         sceneDiagonal      = 0.0F;

            bool
            empty() const
            {
                return nodeCount ==
                       0U ||
                       triangleCount ==
                       0U ||
                       triangleIndexCount == 0U;
            }
    };

    GpuRayScene
    createGpuRayScene( const RayScene& scene );

    GpuRayScene
    loadOrCreateGpuRayScene( osg::Node&           model,
                             const SponzaOptions& options );

    void
    applyGpuRaySceneBindings( osg::StateSet&     stateSet,
                              const GpuRayScene& scene );

}
