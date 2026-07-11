/* OSGiliath -- OpenSceneGraph fork. See LICENSE.txt */
#include "SponzaGpuRayScene.hpp"
#include "SponzaOptions.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <ios>
#include <limits>
#include <osg/core/Notify.hpp>
#include <osg/core/Timer.hpp>
#include <osg/geometry/BufferObject.hpp>
#include <osg/GL>
#include <osg/nodes/Node.hpp>
#include <osg/state/StateSet.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/fstream.hpp>
#include <string>
#include <system_error>

namespace
{

    constexpr std::array<char, 8U> cacheMagic{ 'O', 'S', 'G', 'R', 'T', 'S', 'C', 'N' };
    constexpr std::uint32_t        cacheVersion         = 1U;
    constexpr std::uint32_t        exclusionListVersion = 1U;
    constexpr std::uint32_t        shaderLayoutVersion  = 1U;

    struct ModelStamp
    {
            std::uint64_t size       = 0U;
            std::int64_t  mtimeTicks = 0;
    };

    bool
    writeValue( osgDB::ofstream& stream,
                const void*      value,
                std::size_t      size )
    {
        stream.write( static_cast<const char*>( value ),
                      static_cast<std::streamsize>( size ) );
        return static_cast<bool>( stream );
    }

    template<typename T>
    bool
    writePod( osgDB::ofstream& stream,
              const T&         value )
    {
        return writeValue( stream, &value, sizeof( value ) );
    }

    bool
    readValue( osgDB::ifstream& stream,
               void*            value,
               std::size_t      size )
    {
        stream.read( static_cast<char*>( value ), static_cast<std::streamsize>( size ) );
        return static_cast<bool>( stream );
    }

    template<typename T>
    bool
    readPod( osgDB::ifstream& stream,
             T&               value )
    {
        return readValue( stream, &value, sizeof( value ) );
    }

    bool
    modelStamp( const std::filesystem::path& modelPath,
                ModelStamp&                  stamp )
    {
        std::error_code      error;
        const std::uintmax_t size = std::filesystem::file_size( modelPath, error );
        if( error )
        {
            return false;
        }
        const std::filesystem::file_time_type mtime =
            std::filesystem::last_write_time( modelPath, error );
        if( error )
        {
            return false;
        }

        stamp.size       = static_cast<std::uint64_t>( size );
        stamp.mtimeTicks = static_cast<std::int64_t>( mtime.time_since_epoch().count() );
        return true;
    }

    std::filesystem::path
    resolveModelPath( const std::string& modelPath )
    {
        const std::string resolved = osgDB::findDataFile( modelPath );
        if( !resolved.empty() )
        {
            return std::filesystem::path( resolved );
        }
        return std::filesystem::path( modelPath );
    }

    std::filesystem::path
    densifyCacheSuffix( const sponza::SponzaOptions& options )
    {
        const auto edgeMillimeters = static_cast<long>(
            std::lround( static_cast<double>( options.bakeDensifyMaxEdge ) * 1000.0 )
        );
        return std::string( ".dense-" ) +
               std::to_string( edgeMillimeters ) +
               "mm-" +
               std::to_string( options.bakeDensifyMaxSubdiv );
    }

    std::filesystem::path
    cachePathForModel( const std::filesystem::path& modelPath,
                       const sponza::SponzaOptions& options )
    {
        std::filesystem::path cachePath  = modelPath;
        cachePath                       += ".rtscene";
        if( options.bakeDensifyEnabled )
        {
            cachePath += densifyCacheSuffix( options );
        }
        return cachePath;
    }

    template<typename ArrayT>
    void
    installShaderStorageBuffer( ArrayT& array )
    {
        array.setBufferObject( new osg::ShaderStorageBufferObject );
    }

    GLsizeiptr
    bufferSize( const osg::Array& array )
    {
        return static_cast<GLsizeiptr>( array.getTotalDataSize() );
    }

    osg::vec4
    paddedVec4( const osg::vec3& value,
                float            w )
    {
        return osg::vec4( value.x, value.y, value.z, w );
    }

    osg::uivec4
    nodeMeta( const sponza::BvhNode& node )
    {
        return osg::uivec4( node.left, node.right, node.start, node.count );
    }

    float
    sceneDiagonal( const sponza::RayScene& scene )
    {
        if( scene.nodes.empty() )
        {
            return 0.0F;
        }

        const osg::vec3 extent =
            scene.nodes.front().bounds.max - scene.nodes.front().bounds.min;
        return osg::length( extent );
    }

    void
    finalizeGpuRayScene( sponza::GpuRayScene& gpuScene )
    {
        installShaderStorageBuffer( *gpuScene.nodeMin );
        installShaderStorageBuffer( *gpuScene.nodeMax );
        installShaderStorageBuffer( *gpuScene.nodeMeta );
        installShaderStorageBuffer( *gpuScene.triangleIndices );
        installShaderStorageBuffer( *gpuScene.triangleV0Edge );
        installShaderStorageBuffer( *gpuScene.triangleV1 );
        installShaderStorageBuffer( *gpuScene.triangleV2 );
        installShaderStorageBuffer( *gpuScene.triangleNormal );

        gpuScene.nodeMinBinding =
            new osg::ShaderStorageBufferBinding( sponza::rtNodeMinBinding,
                                                 gpuScene.nodeMin.get(),
                                                 0,
                                                 bufferSize( *gpuScene.nodeMin ) );
        gpuScene.nodeMaxBinding =
            new osg::ShaderStorageBufferBinding( sponza::rtNodeMaxBinding,
                                                 gpuScene.nodeMax.get(),
                                                 0,
                                                 bufferSize( *gpuScene.nodeMax ) );
        gpuScene.nodeMetaBinding =
            new osg::ShaderStorageBufferBinding( sponza::rtNodeMetaBinding,
                                                 gpuScene.nodeMeta.get(),
                                                 0,
                                                 bufferSize( *gpuScene.nodeMeta ) );
        gpuScene.triangleIndexBinding = new osg::ShaderStorageBufferBinding(
            sponza::rtTriangleIndexBinding,
            gpuScene.triangleIndices.get(),
            0,
            bufferSize( *gpuScene.triangleIndices )
        );
        gpuScene.triangleV0Binding = new osg::ShaderStorageBufferBinding(
            sponza::rtTriangleV0Binding,
            gpuScene.triangleV0Edge.get(),
            0,
            bufferSize( *gpuScene.triangleV0Edge )
        );
        gpuScene.triangleV1Binding =
            new osg::ShaderStorageBufferBinding( sponza::rtTriangleV1Binding,
                                                 gpuScene.triangleV1.get(),
                                                 0,
                                                 bufferSize( *gpuScene.triangleV1 ) );
        gpuScene.triangleV2Binding =
            new osg::ShaderStorageBufferBinding( sponza::rtTriangleV2Binding,
                                                 gpuScene.triangleV2.get(),
                                                 0,
                                                 bufferSize( *gpuScene.triangleV2 ) );
        gpuScene.triangleNormalBinding = new osg::ShaderStorageBufferBinding(
            sponza::rtTriangleNormalBinding,
            gpuScene.triangleNormal.get(),
            0,
            bufferSize( *gpuScene.triangleNormal )
        );
    }

    template<typename ArrayT>
    bool
    writeArray( osgDB::ofstream& stream,
                const ArrayT&    array )
    {
        const std::uint64_t count = static_cast<std::uint64_t>( array.size() );
        if( !writePod( stream, count ) )
        {
            return false;
        }
        if( count == 0U )
        {
            return true;
        }

        const std::size_t bytes = static_cast<std::size_t>( count ) *
                                  sizeof( typename ArrayT::ElementDataType );
        return writeValue( stream, &( array[0] ), bytes );
    }

    template<typename ArrayT>
    bool
    readArray( osgDB::ifstream&      stream,
               osg::ref_ptr<ArrayT>& array )
    {
        std::uint64_t count = 0U;
        if( !readPod( stream, count ) )
        {
            return false;
        }
        if( count >
            static_cast<std::uint64_t>( std::numeric_limits<unsigned int>::max() ) )
        {
            return false;
        }

        array = new ArrayT( static_cast<unsigned int>( count ) );
        if( count == 0U )
        {
            return true;
        }

        const std::size_t bytes = static_cast<std::size_t>( count ) *
                                  sizeof( typename ArrayT::ElementDataType );
        return readValue( stream, &( ( *array )[0] ), bytes );
    }

    bool
    arraySizesMatch( const sponza::GpuRayScene& gpuScene )
    {
        return gpuScene.nodeMin.valid() &&
               gpuScene.nodeMax.valid() &&
               gpuScene.nodeMeta.valid() &&
               gpuScene.triangleIndices.valid() &&
               gpuScene.triangleV0Edge.valid() &&
               gpuScene.triangleV1.valid() &&
               gpuScene.triangleV2.valid() &&
               gpuScene.triangleNormal.valid() &&
               gpuScene.nodeMin->size() ==
               gpuScene.nodeCount &&
               gpuScene.nodeMax->size() ==
               gpuScene.nodeCount &&
               gpuScene.nodeMeta->size() ==
               gpuScene.nodeCount &&
               gpuScene.triangleIndices->size() ==
               gpuScene.triangleIndexCount &&
               gpuScene.triangleV0Edge->size() ==
               gpuScene.triangleCount &&
               gpuScene.triangleV1->size() ==
               gpuScene.triangleCount &&
               gpuScene.triangleV2->size() ==
               gpuScene.triangleCount &&
               gpuScene.triangleNormal->size() == gpuScene.triangleCount;
    }

    bool
    loadGpuRaySceneCache( const std::filesystem::path& cachePath,
                          const ModelStamp&            stamp,
                          const sponza::SponzaOptions& options,
                          sponza::GpuRayScene&         gpuScene )
    {
        osgDB::ifstream stream( cachePath.string().c_str(),
                                std::ios::in | std::ios::binary );
        if( !stream )
        {
            return false;
        }

        std::array<char, 8U> magic{};
        std::uint32_t        version            = 0U;
        std::uint32_t        cachedExclusions   = 0U;
        std::uint32_t        cachedLayout       = 0U;
        std::uint64_t        cachedSize         = 0U;
        std::int64_t         cachedMtime        = 0;
        std::uint8_t         cachedDensify      = 0U;
        float                cachedDenseEdge    = 0.0F;
        std::int32_t         cachedDenseSubdiv  = 0;
        std::uint64_t        nodeCount          = 0U;
        std::uint64_t        triangleCount      = 0U;
        std::uint64_t        triangleIndexCount = 0U;
        if( !readValue( stream, magic.data(), magic.size() ) ||
            !readPod( stream, version ) ||
            !readPod( stream, cachedExclusions ) ||
            !readPod( stream, cachedLayout ) ||
            !readPod( stream, cachedSize ) ||
            !readPod( stream, cachedMtime ) ||
            !readPod( stream, cachedDensify ) ||
            !readPod( stream, cachedDenseEdge ) ||
            !readPod( stream, cachedDenseSubdiv ) ||
            !readPod( stream, gpuScene.sceneDiagonal ) ||
            !readPod( stream, nodeCount ) ||
            !readPod( stream, triangleCount ) ||
            !readPod( stream, triangleIndexCount ) )
        {
            return false;
        }

        if( magic !=
            cacheMagic ||
            version !=
            cacheVersion ||
            cachedExclusions !=
            exclusionListVersion ||
            cachedLayout !=
            shaderLayoutVersion ||
            cachedSize !=
            stamp.size ||
            cachedMtime !=
            stamp.mtimeTicks ||
            cachedDensify !=
            static_cast<std::uint8_t>( options.bakeDensifyEnabled ? 1U : 0U ) ||
            cachedDenseEdge !=
            options.bakeDensifyMaxEdge ||
            cachedDenseSubdiv !=
            static_cast<std::int32_t>( options.bakeDensifyMaxSubdiv ) ||
            nodeCount >
            static_cast<std::uint64_t>( std::numeric_limits<std::size_t>::max() ) ||
            triangleCount >
            static_cast<std::uint64_t>( std::numeric_limits<std::size_t>::max() ) ||
            triangleIndexCount >
            static_cast<std::uint64_t>( std::numeric_limits<std::size_t>::max() ) )
        {
            return false;
        }

        gpuScene.nodeCount          = static_cast<std::size_t>( nodeCount );
        gpuScene.triangleCount      = static_cast<std::size_t>( triangleCount );
        gpuScene.triangleIndexCount = static_cast<std::size_t>( triangleIndexCount );

        if( !readArray( stream, gpuScene.nodeMin ) ||
            !readArray( stream, gpuScene.nodeMax ) ||
            !readArray( stream, gpuScene.nodeMeta ) ||
            !readArray( stream, gpuScene.triangleIndices ) ||
            !readArray( stream, gpuScene.triangleV0Edge ) ||
            !readArray( stream, gpuScene.triangleV1 ) ||
            !readArray( stream, gpuScene.triangleV2 ) ||
            !readArray( stream, gpuScene.triangleNormal ) ||
            !arraySizesMatch( gpuScene ) )
        {
            return false;
        }

        finalizeGpuRayScene( gpuScene );
        return true;
    }

    bool
    saveGpuRaySceneCache( const std::filesystem::path& cachePath,
                          const ModelStamp&            stamp,
                          const sponza::SponzaOptions& options,
                          const sponza::GpuRayScene&   gpuScene )
    {
        osgDB::ofstream stream( cachePath.string().c_str(),
                                std::ios::out | std::ios::binary | std::ios::trunc );
        if( !stream )
        {
            return false;
        }

        const std::uint8_t densify =
            static_cast<std::uint8_t>( options.bakeDensifyEnabled ? 1U : 0U );
        const std::int32_t denseSubdiv =
            static_cast<std::int32_t>( options.bakeDensifyMaxSubdiv );
        const std::uint64_t nodeCount = static_cast<std::uint64_t>( gpuScene.nodeCount );
        const std::uint64_t triangleCount =
            static_cast<std::uint64_t>( gpuScene.triangleCount );
        const std::uint64_t triangleIndexCount =
            static_cast<std::uint64_t>( gpuScene.triangleIndexCount );

        return writeValue( stream, cacheMagic.data(), cacheMagic.size() ) &&
               writePod( stream, cacheVersion ) &&
               writePod( stream, exclusionListVersion ) &&
               writePod( stream, shaderLayoutVersion ) &&
               writePod( stream, stamp.size ) &&
               writePod( stream, stamp.mtimeTicks ) &&
               writePod( stream, densify ) &&
               writePod( stream, options.bakeDensifyMaxEdge ) &&
               writePod( stream, denseSubdiv ) &&
               writePod( stream, gpuScene.sceneDiagonal ) &&
               writePod( stream, nodeCount ) &&
               writePod( stream, triangleCount ) &&
               writePod( stream, triangleIndexCount ) &&
               writeArray( stream, *gpuScene.nodeMin ) &&
               writeArray( stream, *gpuScene.nodeMax ) &&
               writeArray( stream, *gpuScene.nodeMeta ) &&
               writeArray( stream, *gpuScene.triangleIndices ) &&
               writeArray( stream, *gpuScene.triangleV0Edge ) &&
               writeArray( stream, *gpuScene.triangleV1 ) &&
               writeArray( stream, *gpuScene.triangleV2 ) &&
               writeArray( stream, *gpuScene.triangleNormal );
    }

}

namespace sponza
{

    GpuRayScene
    createGpuRayScene( const RayScene& scene )
    {
        GpuRayScene gpuScene;
        gpuScene.nodeCount           = scene.nodes.size();
        gpuScene.triangleCount       = scene.triangles.size();
        gpuScene.triangleIndexCount  = scene.triangleIndices.size();
        gpuScene.sceneDiagonal       = sceneDiagonal( scene );

        gpuScene.nodeMin             = new osg::Vec4Array;
        gpuScene.nodeMax             = new osg::Vec4Array;
        gpuScene.nodeMeta            = new osg::Vec4uiArray;
        gpuScene.triangleIndices     = new osg::UIntArray;
        gpuScene.triangleV0Edge      = new osg::Vec4Array;
        gpuScene.triangleV1          = new osg::Vec4Array;
        gpuScene.triangleV2          = new osg::Vec4Array;
        gpuScene.triangleNormal      = new osg::Vec4Array;

        const unsigned int nodeCount = static_cast<unsigned int>( std::min<std::size_t>(
            scene.nodes.size(),
            static_cast<std::size_t>( std::numeric_limits<unsigned int>::max() )
        ) );
        gpuScene.nodeMin->reserve( nodeCount );
        gpuScene.nodeMax->reserve( nodeCount );
        gpuScene.nodeMeta->reserve( nodeCount );
        for( const BvhNode& node : scene.nodes )
        {
            gpuScene.nodeMin->push_back( paddedVec4( node.bounds.min, 0.0F ) );
            gpuScene.nodeMax->push_back( paddedVec4( node.bounds.max, 0.0F ) );
            gpuScene.nodeMeta->push_back( nodeMeta( node ) );
        }

        const unsigned int triangleIndexCount =
            static_cast<unsigned int>( std::min<std::size_t>(
                scene.triangleIndices.size(),
                static_cast<std::size_t>( std::numeric_limits<unsigned int>::max() )
            ) );
        gpuScene.triangleIndices->reserve( triangleIndexCount );
        for( std::uint32_t triangleIndex : scene.triangleIndices )
        {
            gpuScene.triangleIndices->push_back( static_cast<GLuint>( triangleIndex ) );
        }

        const unsigned int triangleCount =
            static_cast<unsigned int>( std::min<std::size_t>(
                scene.triangles.size(),
                static_cast<std::size_t>( std::numeric_limits<unsigned int>::max() )
            ) );
        gpuScene.triangleV0Edge->reserve( triangleCount );
        gpuScene.triangleV1->reserve( triangleCount );
        gpuScene.triangleV2->reserve( triangleCount );
        gpuScene.triangleNormal->reserve( triangleCount );
        for( const Triangle& triangle : scene.triangles )
        {
            gpuScene.triangleV0Edge->push_back( paddedVec4( triangle.v0,
                                                            triangle.longestEdge ) );
            gpuScene.triangleV1->push_back( paddedVec4( triangle.v1, 0.0F ) );
            gpuScene.triangleV2->push_back( paddedVec4( triangle.v2, 0.0F ) );
            gpuScene.triangleNormal->push_back( paddedVec4( triangle.normal, 0.0F ) );
        }

        finalizeGpuRayScene( gpuScene );

        return gpuScene;
    }

    GpuRayScene
    loadOrCreateGpuRayScene( osg::Node&           model,
                             const SponzaOptions& options )
    {
        const std::filesystem::path modelPath = resolveModelPath( options.modelPath );
        const std::filesystem::path cachePath = cachePathForModel( modelPath, options );
        ModelStamp                  stamp;
        const bool                  hasStamp = modelStamp( modelPath, stamp );

        if( hasStamp )
        {
            GpuRayScene        cachedScene;
            const osg::Timer_t startTick = osg::Timer::instance()->tick();
            if( loadGpuRaySceneCache( cachePath, stamp, options, cachedScene ) )
            {
                const osg::Timer_t endTick = osg::Timer::instance()->tick();
                OSG_NOTICE << "Sponza RT shadow scene cache loaded "
                           << cachedScene.triangleCount << " triangles, "
                           << cachedScene.nodeCount << " BVH nodes from "
                           << cachePath.string() << " in "
                           << osg::Timer::instance()->delta_s( startTick, endTick )
                           << " s" << std::endl;
                return cachedScene;
            }
        }

        const osg::Timer_t buildStart = osg::Timer::instance()->tick();
        RayScene           rayScene   = buildRayScene( model );
        GpuRayScene        gpuScene   = createGpuRayScene( rayScene );
        const osg::Timer_t buildEnd   = osg::Timer::instance()->tick();
        OSG_NOTICE << "Sponza RT shadow scene built " << gpuScene.triangleCount
                   << " triangles, " << gpuScene.nodeCount << " BVH nodes in "
                   << osg::Timer::instance()->delta_s( buildStart, buildEnd ) << " s"
                   << std::endl;

        if( hasStamp && !gpuScene.empty() )
        {
            std::error_code             error;
            const std::filesystem::path parent = cachePath.parent_path();
            if( !parent.empty() )
            {
                std::filesystem::create_directories( parent, error );
            }
            if( !saveGpuRaySceneCache( cachePath, stamp, options, gpuScene ) )
            {
                OSG_WARN << "Sponza RT shadow scene could not write cache "
                         << cachePath.string() << std::endl;
            }
        }
        else if( !hasStamp )
        {
            OSG_WARN << "Sponza RT shadow scene could not stat model file "
                     << modelPath.string() << "; cache disabled" << std::endl;
        }

        return gpuScene;
    }

    void
    applyGpuRaySceneBindings( osg::StateSet&     stateSet,
                              const GpuRayScene& scene )
    {
        if( scene.empty() )
        {
            return;
        }

        stateSet.setAttributeAndModes( scene.nodeMinBinding.get(),
                                       osg::StateAttribute::ON );
        stateSet.setAttributeAndModes( scene.nodeMaxBinding.get(),
                                       osg::StateAttribute::ON );
        stateSet.setAttributeAndModes( scene.nodeMetaBinding.get(),
                                       osg::StateAttribute::ON );
        stateSet.setAttributeAndModes( scene.triangleIndexBinding.get(),
                                       osg::StateAttribute::ON );
        stateSet.setAttributeAndModes( scene.triangleV0Binding.get(),
                                       osg::StateAttribute::ON );
        stateSet.setAttributeAndModes( scene.triangleV1Binding.get(),
                                       osg::StateAttribute::ON );
        stateSet.setAttributeAndModes( scene.triangleV2Binding.get(),
                                       osg::StateAttribute::ON );
        stateSet.setAttributeAndModes( scene.triangleNormalBinding.get(),
                                       osg::StateAttribute::ON );
    }

}
