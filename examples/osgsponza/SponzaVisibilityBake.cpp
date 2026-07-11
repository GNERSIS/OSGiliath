/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "SponzaLighting.hpp"
#include "SponzaOptions.hpp"
#include "SponzaVisibilityBake.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <ios>
#include <limits>
#include <map>
#include <numeric>
#include <osg/core/Notify.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Timer.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/GL>
#include <osg/images/Image.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/transform.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/fstream.hpp>
#include <string>
#include <thread>
#include <vector>

namespace
{

    constexpr unsigned int          visibilityAttribLocation = 7U;
    constexpr unsigned int          radianceAttribLocation   = 1U;
    constexpr unsigned int          tangentAttribLocation    = 6U;
    constexpr std::array<char, 8U>  cacheMagic{ 'O', 'S', 'G', 'V', 'I', 'S', 'B', 'K' };
    // v6 radiance hit gathering includes sky-lit bounce via interpolated visibility.
    constexpr std::uint32_t         cacheVersion         = 6U;
    constexpr std::size_t           bvhLeafSize          = 8U;
    constexpr float                 rayOriginOffset      = 0.02F;
    constexpr float                 adaptiveOffsetScale  = 0.005F;
    constexpr float                 bakeMaxEdgeLength    = 0.5F;
    constexpr float                 facePlaneNudge       = 0.03F;
    constexpr float                 rayHitEpsilon        = 1.0E-4F;
    constexpr float                 minNormalLength2     = 1.0E-10F;
    constexpr float                 pi                   = 3.14159265358979323846F;
    constexpr double                twoPi                = 6.28318530717958647692;
    constexpr double                sunDiscLuminance     = 500.0;
    constexpr int                   albedoTargetSamples  = 4'096;
    constexpr unsigned int          bakeMaxSubdivisions  = 6U;
    constexpr std::size_t           maxExtraSamplePoints = 6U;
    constexpr float                 minMultibounceDenom  = 0.35F;
    constexpr std::array<float, 2U> faceSampleFractions{ 0.35F, 0.70F };
    constexpr std::uint32_t         invalidRecordIndex =
        std::numeric_limits<std::uint32_t>::max();

    struct AdjacentFace
    {
            osg::vec3 centroid{ 0.0F, 0.0F, 0.0F };
            osg::vec3 normal{ 0.0F, 1.0F, 0.0F };
            float     longestEdge = 0.0F;
    };

    struct GeometryRecord
    {
            osg::Geometry*                         geometry = nullptr;
            const osg::Vec3Array*                  vertices = nullptr;
            const osg::Vec3Array*                  normals  = nullptr;
            osg::dmat4                             localToWorld;
            osg::dmat4                             normalMatrix;
            std::string                            name;
            std::vector<std::vector<AdjacentFace>> adjacentFaces;
            std::uint32_t                          materialIndex    = 0U;
            std::size_t                            vertexCount      = 0U;
            bool                                   hasNormals       = false;
            bool                                   excludedOccluder = false;
    };

    struct Triangle
    {
            osg::vec3     v0;
            osg::vec3     v1;
            osg::vec3     v2;
            osg::vec3     normal;
            osg::vec3     centroid;
            osg::box      bounds;
            float         longestEdge   = 0.0F;
            std::uint32_t materialIndex = 0U;
            std::uint32_t recordIndex   = invalidRecordIndex;
            std::uint32_t i0            = 0U;
            std::uint32_t i1            = 0U;
            std::uint32_t i2            = 0U;
    };

    struct MaterialAlbedo
    {
            osg::vec3   albedo{ 1.0F, 1.0F, 1.0F };
            std::string name;
            bool        textured = false;
    };

    struct RayHit
    {
            float         distance      = std::numeric_limits<float>::max();
            std::uint32_t triangleIndex = 0U;
            bool          hit           = false;
    };

    struct PointBake
    {
            osg::vec3 bentNormal{ 0.0F, 1.0F, 0.0F };
            osg::vec3 radiance{ 0.0F, 0.0F, 0.0F };
            float     visibility = 1.0F;
    };

    struct VertexSamplePoint
    {
            osg::vec3 position{ 0.0F, 0.0F, 0.0F };
            osg::vec3 normal{ 0.0F, 1.0F, 0.0F };
            float     originOffset = rayOriginOffset;
            bool      extra        = false;
    };

    struct BvhNode
    {
            osg::box      bounds;
            std::uint32_t left  = 0U;
            std::uint32_t right = 0U;
            std::uint32_t start = 0U;
            std::uint32_t count = 0U;
    };

    struct Bvh
    {
            std::vector<Triangle>      triangles;
            std::vector<std::uint32_t> triangleIndices;
            std::vector<BvhNode>       nodes;
    };

    struct BakeArrays
    {
            std::vector<std::vector<osg::vec4>> visibility;
            std::vector<std::vector<osg::vec3>> radiance;
    };

    struct ModelStamp
    {
            std::uint64_t size       = 0U;
            std::int64_t  mtimeTicks = 0;
    };

    float
    axisValue( const osg::vec3& value,
               unsigned int     axis )
    {
        switch( axis )
        {
            case 0U :
                return value.x;
            case 1U :
                return value.y;
            default :
                return value.z;
        }
    }

    osg::vec3
    safeNormalize( const osg::vec3& value,
                   const osg::vec3& fallback )
    {
        const float len2 = osg::length2( value );
        if( len2 <= minNormalLength2 )
        {
            return fallback;
        }
        return value * ( 1.0F / std::sqrt( len2 ) );
    }

    osg::vec3
    transformPoint( const osg::dmat4& matrix,
                    const osg::vec3&  point )
    {
        const osg::dvec3 world = matrix * osg::dvec3( static_cast<double>( point.x ),
                                                      static_cast<double>( point.y ),
                                                      static_cast<double>( point.z ) );
        return osg::vec3( static_cast<float>( world.x ),
                          static_cast<float>( world.y ),
                          static_cast<float>( world.z ) );
    }

    osg::vec3
    transformNormal( const osg::dmat4& normalMatrix,
                     const osg::vec3&  normal )
    {
        const osg::dvec3 n( static_cast<double>( normal.x ),
                            static_cast<double>( normal.y ),
                            static_cast<double>( normal.z ) );
        const osg::dvec3 world( normalMatrix[0][0] *
                                    n.x +
                                    normalMatrix[0][1] *
                                    n.y +
                                    normalMatrix[0][2] *
                                    n.z,
                                normalMatrix[1][0] *
                                    n.x +
                                    normalMatrix[1][1] *
                                    n.y +
                                    normalMatrix[1][2] *
                                    n.z,
                                normalMatrix[2][0] *
                                    n.x +
                                    normalMatrix[2][1] *
                                    n.y +
                                    normalMatrix[2][2] *
                                    n.z );
        return osg::vec3( static_cast<float>( world.x ),
                          static_cast<float>( world.y ),
                          static_cast<float>( world.z ) );
    }

    osg::dmat4
    makeNormalMatrix( const osg::dmat4& localToWorld )
    {
        return osg::inverse( localToWorld );
    }

    bool
    isExcludedOccluderName( const std::string& name )
    {
        return name == "glass" || name == "lamp_glass_01" || name == "dirt_decal";
    }

    bool
    isExcludedOccluder( const osg::Drawable& drawable )
    {
        const osg::StateSet* stateSet = drawable.getStateSet();
        return isExcludedOccluderName( drawable.getName() ) ||
               ( stateSet && isExcludedOccluderName( stateSet->getName() ) );
    }

    double
    luminance( const osg::dvec3& color )
    {
        return color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
    }

    float
    srgbToLinear( float value )
    {
        return std::pow( std::clamp( value, 0.0F, 1.0F ), 2.2F );
    }

    osg::vec3
    multiplyComponents( const osg::vec3& lhs,
                        const osg::vec3& rhs )
    {
        return osg::vec3( lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b );
    }

    bool
    canReadFloatRgb( const osg::Image& image )
    {
        return image.valid() &&
               image.getDataType() ==
               GL_FLOAT &&
               ( image.getPixelFormat() == GL_RGB || image.getPixelFormat() == GL_RGBA );
    }

    osg::dvec3
    readFloatRgb( const osg::Image& image,
                  int               x,
                  int               y )
    {
        const auto* pixel = reinterpret_cast<const float*>(
            image.data( static_cast<unsigned int>( x ), static_cast<unsigned int>( y ) )
        );
        return osg::dvec3( static_cast<double>( pixel[0] ),
                           static_cast<double>( pixel[1] ),
                           static_cast<double>( pixel[2] ) );
    }

    class MaterialTable
    {
        public:

            MaterialTable()
            {
                MaterialAlbedo fallback;
                fallback.name = "<default>";
                _materials.push_back( fallback );
            }

            std::uint32_t
            materialIndex( const osg::StateSet* stateSet )
            {
                if( stateSet == nullptr )
                {
                    _fallbackUsed = true;
                    return 0U;
                }

                const auto found = _indices.find( stateSet );
                if( found != _indices.end() )
                {
                    return found->second;
                }

                const std::uint32_t index =
                    static_cast<std::uint32_t>( _materials.size() );
                _indices[stateSet] = index;
                _materials.push_back( computeMaterialAlbedo( *stateSet ) );
                return index;
            }

            const osg::vec3&
            albedo( std::uint32_t index ) const
            {
                if( index >= _materials.size() )
                {
                    return _materials.front().albedo;
                }
                return _materials[static_cast<std::size_t>( index )].albedo;
            }

            float
            averageAlbedoReflectance() const
            {
                const std::size_t begin = _fallbackUsed ? 0U : 1U;
                if( begin >= _materials.size() )
                {
                    return 1.0F;
                }

                double sum = 0.0;
                for( std::size_t i = begin; i < _materials.size(); ++i )
                {
                    const osg::vec3& albedo = _materials[i].albedo;
                    sum += luminance( osg::dvec3( static_cast<double>( albedo.r ),
                                                  static_cast<double>( albedo.g ),
                                                  static_cast<double>( albedo.b ) ) );
                }

                const double mean =
                    sum / static_cast<double>( _materials.size() - begin );
                return std::clamp( static_cast<float>( mean ), 0.0F, 0.95F );
            }

            void
            logStats() const
            {
                const std::size_t begin = _fallbackUsed ? 0U : 1U;
                if( begin >= _materials.size() )
                {
                    return;
                }

                osg::vec3   minAlbedo( std::numeric_limits<float>::max(),
                                       std::numeric_limits<float>::max(),
                                       std::numeric_limits<float>::max() );
                osg::vec3   maxAlbedo( 0.0F, 0.0F, 0.0F );
                std::size_t textured = 0U;
                for( std::size_t i = begin; i < _materials.size(); ++i )
                {
                    const MaterialAlbedo& material = _materials[i];
                    minAlbedo.r = std::min( minAlbedo.r, material.albedo.r );
                    minAlbedo.g = std::min( minAlbedo.g, material.albedo.g );
                    minAlbedo.b = std::min( minAlbedo.b, material.albedo.b );
                    maxAlbedo.r = std::max( maxAlbedo.r, material.albedo.r );
                    maxAlbedo.g = std::max( maxAlbedo.g, material.albedo.g );
                    maxAlbedo.b = std::max( maxAlbedo.b, material.albedo.b );
                    if( material.textured )
                    {
                        ++textured;
                    }
                }

                OSG_NOTICE << "Sponza radiance bake material albedo table: "
                           << ( _materials.size() - begin ) << " materials (" << textured
                           << " textured), linear RGB range min=(" << minAlbedo.r << ", "
                           << minAlbedo.g << ", " << minAlbedo.b << ") max=("
                           << maxAlbedo.r << ", " << maxAlbedo.g << ", " << maxAlbedo.b
                           << "), rhoBar=" << averageAlbedoReflectance() << std::endl;
            }

        private:

            static osg::vec3
            readBaseColorFactor( const osg::StateSet& stateSet )
            {
                osg::vec4           factor( 1.0F, 1.0F, 1.0F, 1.0F );
                const osg::Uniform* uniform = stateSet.getUniform( "uBaseColorFactor" );
                if( uniform != nullptr )
                {
                    osg::vec4 value;
                    if( uniform->get( value ) )
                    {
                        factor = value;
                    }
                }
                return osg::vec3( factor.r, factor.g, factor.b );
            }

            static const osg::Image*
            baseColorImage( const osg::StateSet& stateSet )
            {
                const osg::StateAttribute* attribute =
                    stateSet.getTextureAttribute( 0U,
                                                  osg::StateAttribute::Type::TEXTURE );
                const auto* texture = dynamic_cast<const osg::Texture2D*>( attribute );
                return texture != nullptr ? texture->getImage( 0U ) : nullptr;
            }

            static osg::vec3
            averageTextureAlbedo( const osg::Image& image )
            {
                const int width  = image.s();
                const int height = image.t();
                if( width <= 0 || height <= 0 )
                {
                    return osg::vec3( 1.0F, 1.0F, 1.0F );
                }

                const int totalTexels = width * height;
                const int stride =
                    std::max( 1,
                              static_cast<int>(
                                  std::sqrt( static_cast<double>( totalTexels ) /
                                             static_cast<double>( albedoTargetSamples ) )
                              ) );

                osg::dvec3  sum( 0.0, 0.0, 0.0 );
                std::size_t sampleCount = 0U;
                for( int y = 0; y < height; y += stride )
                {
                    for( int x = 0; x < width; x += stride )
                    {
                        const osg::vec4 srgb =
                            image.getColor( static_cast<unsigned int>( x ),
                                            static_cast<unsigned int>( y ),
                                            0U );
                        sum +=
                            osg::dvec3( static_cast<double>( srgbToLinear( srgb.r ) ),
                                        static_cast<double>( srgbToLinear( srgb.g ) ),
                                        static_cast<double>( srgbToLinear( srgb.b ) ) );
                        ++sampleCount;
                    }
                }

                if( sampleCount == 0U )
                {
                    return osg::vec3( 1.0F, 1.0F, 1.0F );
                }

                const double invCount = 1.0 / static_cast<double>( sampleCount );
                return osg::vec3( static_cast<float>( sum.r * invCount ),
                                  static_cast<float>( sum.g * invCount ),
                                  static_cast<float>( sum.b * invCount ) );
            }

            static MaterialAlbedo
            computeMaterialAlbedo( const osg::StateSet& stateSet )
            {
                MaterialAlbedo material;
                material.name                   = stateSet.getName();
                const osg::vec3   factor        = readBaseColorFactor( stateSet );
                const osg::Image* image         = baseColorImage( stateSet );
                const osg::vec3   textureAlbedo = image != nullptr
                                                    ? averageTextureAlbedo( *image )
                                                    : osg::vec3( 1.0F, 1.0F, 1.0F );
                material.albedo   = multiplyComponents( textureAlbedo, factor );
                material.textured = image != nullptr;
                return material;
            }

            std::vector<MaterialAlbedo>                   _materials;
            std::map<const osg::StateSet*, std::uint32_t> _indices;
            bool                                          _fallbackUsed = false;
    };

    osg::box
    triangleBounds( const osg::vec3& v0,
                    const osg::vec3& v1,
                    const osg::vec3& v2 )
    {
        osg::box bounds;
        bounds.expandBy( v0 );
        bounds.expandBy( v1 );
        bounds.expandBy( v2 );
        return bounds;
    }

    float
    triangleLongestEdge( const osg::vec3& v0,
                         const osg::vec3& v1,
                         const osg::vec3& v2 )
    {
        return std::max( osg::length( v1 - v0 ),
                         std::max( osg::length( v2 - v1 ), osg::length( v0 - v2 ) ) );
    }

    float
    adaptiveOriginOffset( float longestEdge )
    {
        return std::max( rayOriginOffset, adaptiveOffsetScale * longestEdge );
    }

    void
    appendTriangle( std::vector<Triangle>& triangles,
                    const osg::Vec3Array&  vertices,
                    const osg::dmat4&      localToWorld,
                    std::uint32_t          materialIndex,
                    std::uint32_t          recordIndex,
                    unsigned int           i0,
                    unsigned int           i1,
                    unsigned int           i2 )
    {
        const unsigned int vertexCount = vertices.getNumElements();
        if( i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount )
        {
            return;
        }

        const osg::vec3 v0         = transformPoint( localToWorld, vertices[i0] );
        const osg::vec3 v1         = transformPoint( localToWorld, vertices[i1] );
        const osg::vec3 v2         = transformPoint( localToWorld, vertices[i2] );
        const osg::vec3 faceNormal = osg::cross( v1 - v0, v2 - v0 );
        if( osg::length2( faceNormal ) <= 1.0E-12F )
        {
            return;
        }

        Triangle triangle;
        triangle.v0       = v0;
        triangle.v1       = v1;
        triangle.v2       = v2;
        triangle.normal   = safeNormalize( faceNormal, osg::vec3( 0.0F, 1.0F, 0.0F ) );
        triangle.centroid = ( v0 + v1 + v2 ) * ( 1.0F / 3.0F );
        triangle.bounds   = triangleBounds( v0, v1, v2 );
        triangle.longestEdge   = triangleLongestEdge( v0, v1, v2 );
        triangle.materialIndex = materialIndex;
        triangle.recordIndex   = recordIndex;
        triangle.i0            = i0;
        triangle.i1            = i1;
        triangle.i2            = i2;
        triangles.push_back( triangle );
    }

    void
    appendAdjacentFace( GeometryRecord&       record,
                        const osg::Vec3Array& vertices,
                        unsigned int          i0,
                        unsigned int          i1,
                        unsigned int          i2 )
    {
        const unsigned int vertexCount = vertices.getNumElements();
        if( i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount )
        {
            return;
        }

        const osg::vec3 v0         = transformPoint( record.localToWorld, vertices[i0] );
        const osg::vec3 v1         = transformPoint( record.localToWorld, vertices[i1] );
        const osg::vec3 v2         = transformPoint( record.localToWorld, vertices[i2] );
        const osg::vec3 faceNormal = osg::cross( v1 - v0, v2 - v0 );
        if( osg::length2( faceNormal ) <= 1.0E-12F )
        {
            return;
        }

        AdjacentFace face;
        face.normal      = safeNormalize( faceNormal, osg::vec3( 0.0F, 1.0F, 0.0F ) );
        face.centroid    = ( v0 + v1 + v2 ) * ( 1.0F / 3.0F );
        face.longestEdge = triangleLongestEdge( v0, v1, v2 );

        record.adjacentFaces[static_cast<std::size_t>( i0 )].push_back( face );
        record.adjacentFaces[static_cast<std::size_t>( i1 )].push_back( face );
        record.adjacentFaces[static_cast<std::size_t>( i2 )].push_back( face );
    }

    void
    appendPrimitiveTriangles( std::vector<Triangle>&   triangles,
                              const osg::Vec3Array&    vertices,
                              const osg::dmat4&        localToWorld,
                              std::uint32_t            materialIndex,
                              const osg::PrimitiveSet& primitiveSet,
                              std::uint32_t            recordIndex )
    {
        const unsigned int indexCount = primitiveSet.getNumIndices();
        switch( primitiveSet.getMode() )
        {
            case GL_TRIANGLES :
                for( unsigned int i = 0U; i + 2U < indexCount; i += 3U )
                {
                    appendTriangle( triangles,
                                    vertices,
                                    localToWorld,
                                    materialIndex,
                                    recordIndex,
                                    primitiveSet.index( i ),
                                    primitiveSet.index( i + 1U ),
                                    primitiveSet.index( i + 2U ) );
                }
                break;

            case GL_TRIANGLE_STRIP :
                for( unsigned int i = 0U; i + 2U < indexCount; ++i )
                {
                    if( ( i & 1U ) == 0U )
                    {
                        appendTriangle( triangles,
                                        vertices,
                                        localToWorld,
                                        materialIndex,
                                        recordIndex,
                                        primitiveSet.index( i ),
                                        primitiveSet.index( i + 1U ),
                                        primitiveSet.index( i + 2U ) );
                    }
                    else
                    {
                        appendTriangle( triangles,
                                        vertices,
                                        localToWorld,
                                        materialIndex,
                                        recordIndex,
                                        primitiveSet.index( i + 1U ),
                                        primitiveSet.index( i ),
                                        primitiveSet.index( i + 2U ) );
                    }
                }
                break;

            case GL_TRIANGLE_FAN :
                for( unsigned int i = 1U; i + 1U < indexCount; ++i )
                {
                    appendTriangle( triangles,
                                    vertices,
                                    localToWorld,
                                    materialIndex,
                                    recordIndex,
                                    primitiveSet.index( 0U ),
                                    primitiveSet.index( i ),
                                    primitiveSet.index( i + 1U ) );
                }
                break;

            case GL_QUADS :
                for( unsigned int i = 0U; i + 3U < indexCount; i += 4U )
                {
                    const unsigned int i0 = primitiveSet.index( i );
                    const unsigned int i1 = primitiveSet.index( i + 1U );
                    const unsigned int i2 = primitiveSet.index( i + 2U );
                    const unsigned int i3 = primitiveSet.index( i + 3U );
                    appendTriangle( triangles,
                                    vertices,
                                    localToWorld,
                                    materialIndex,
                                    recordIndex,
                                    i0,
                                    i1,
                                    i2 );
                    appendTriangle( triangles,
                                    vertices,
                                    localToWorld,
                                    materialIndex,
                                    recordIndex,
                                    i0,
                                    i2,
                                    i3 );
                }
                break;

            default :
                break;
        }
    }

    void
    appendPrimitiveAdjacentFaces( GeometryRecord&          record,
                                  const osg::Vec3Array&    vertices,
                                  const osg::PrimitiveSet& primitiveSet )
    {
        const unsigned int indexCount = primitiveSet.getNumIndices();
        switch( primitiveSet.getMode() )
        {
            case GL_TRIANGLES :
                for( unsigned int i = 0U; i + 2U < indexCount; i += 3U )
                {
                    appendAdjacentFace( record,
                                        vertices,
                                        primitiveSet.index( i ),
                                        primitiveSet.index( i + 1U ),
                                        primitiveSet.index( i + 2U ) );
                }
                break;

            case GL_TRIANGLE_STRIP :
                for( unsigned int i = 0U; i + 2U < indexCount; ++i )
                {
                    if( ( i & 1U ) == 0U )
                    {
                        appendAdjacentFace( record,
                                            vertices,
                                            primitiveSet.index( i ),
                                            primitiveSet.index( i + 1U ),
                                            primitiveSet.index( i + 2U ) );
                    }
                    else
                    {
                        appendAdjacentFace( record,
                                            vertices,
                                            primitiveSet.index( i + 1U ),
                                            primitiveSet.index( i ),
                                            primitiveSet.index( i + 2U ) );
                    }
                }
                break;

            case GL_TRIANGLE_FAN :
                for( unsigned int i = 1U; i + 1U < indexCount; ++i )
                {
                    appendAdjacentFace( record,
                                        vertices,
                                        primitiveSet.index( 0U ),
                                        primitiveSet.index( i ),
                                        primitiveSet.index( i + 1U ) );
                }
                break;

            case GL_QUADS :
                for( unsigned int i = 0U; i + 3U < indexCount; i += 4U )
                {
                    const unsigned int i0 = primitiveSet.index( i );
                    const unsigned int i1 = primitiveSet.index( i + 1U );
                    const unsigned int i2 = primitiveSet.index( i + 2U );
                    const unsigned int i3 = primitiveSet.index( i + 3U );
                    appendAdjacentFace( record, vertices, i0, i1, i2 );
                    appendAdjacentFace( record, vertices, i0, i2, i3 );
                }
                break;

            default :
                break;
        }
    }

    struct DensifyStats
    {
            std::size_t visitedGeometryCount = 0U;
            std::size_t changedGeometryCount = 0U;
            std::size_t skippedGeometryCount = 0U;
            std::size_t inputTriangles       = 0U;
            std::size_t outputTriangles      = 0U;
    };

    struct DensifyPlan
    {
            bool        changed         = false;
            std::size_t inputTriangles  = 0U;
            std::size_t outputTriangles = 0U;
            std::size_t outputVertices  = 0U;
    };

    struct DensifySourceArrays
    {
            const osg::Vec3Array* vertices  = nullptr;
            const osg::Vec3Array* normals   = nullptr;
            const osg::Vec4Array* tangents  = nullptr;
            const osg::Vec2Array* texCoord0 = nullptr;
            const osg::Vec2Array* texCoord1 = nullptr;
            const osg::Vec3Array* colors3   = nullptr;
            const osg::Vec4Array* colors4   = nullptr;
    };

    struct DensifyOutputArrays
    {
            osg::ref_ptr<osg::Vec3Array>        vertices;
            osg::ref_ptr<osg::Vec3Array>        normals;
            osg::ref_ptr<osg::Vec4Array>        tangents;
            osg::ref_ptr<osg::Vec2Array>        texCoord0;
            osg::ref_ptr<osg::Vec2Array>        texCoord1;
            osg::ref_ptr<osg::Vec3Array>        colors3;
            osg::ref_ptr<osg::Vec4Array>        colors4;
            osg::ref_ptr<osg::DrawElementsUInt> indices;
    };

    struct BarycentricWeights
    {
            float w0 = 1.0F;
            float w1 = 0.0F;
            float w2 = 0.0F;
    };

    bool
    hasVertexElements( const osg::Array* array,
                       std::size_t       vertexCount )
    {
        return array !=
               nullptr &&
               static_cast<std::size_t>( array->getNumElements() ) >= vertexCount;
    }

    bool
    readDensifySourceArrays( const osg::Geometry& geometry,
                             DensifySourceArrays& source,
                             std::size_t&         vertexCount )
    {
        const osg::Array* vertexArray = geometry.getVertexArray();
        source.vertices = dynamic_cast<const osg::Vec3Array*>( vertexArray );
        if( source.vertices == nullptr )
        {
            return false;
        }

        vertexCount = static_cast<std::size_t>( source.vertices->getNumElements() );
        if( vertexCount == 0U )
        {
            return false;
        }

        source.normals =
            dynamic_cast<const osg::Vec3Array*>( geometry.getNormalArray() );
        source.tangents = dynamic_cast<const osg::Vec4Array*>(
            geometry.getVertexAttribArray( tangentAttribLocation )
        );
        source.texCoord0 =
            dynamic_cast<const osg::Vec2Array*>( geometry.getTexCoordArray( 0U ) );
        if( !hasVertexElements( source.normals, vertexCount ) ||
            !hasVertexElements( source.tangents, vertexCount ) ||
            !hasVertexElements( source.texCoord0, vertexCount ) )
        {
            return false;
        }

        const osg::Array* texCoord1Array = geometry.getTexCoordArray( 1U );
        if( texCoord1Array != nullptr )
        {
            source.texCoord1 = dynamic_cast<const osg::Vec2Array*>( texCoord1Array );
            if( !hasVertexElements( source.texCoord1, vertexCount ) )
            {
                return false;
            }
        }

        const osg::Array* colorArray = geometry.getColorArray();
        if( colorArray !=
            nullptr &&
            colorArray->getBinding() == osg::Array::BIND_PER_VERTEX )
        {
            source.colors4 = dynamic_cast<const osg::Vec4Array*>( colorArray );
            source.colors3 = source.colors4 == nullptr
                               ? dynamic_cast<const osg::Vec3Array*>( colorArray )
                               : nullptr;
            if( !hasVertexElements(
                    source.colors4 != nullptr
                        ? static_cast<const osg::Array*>( source.colors4 )
                        : static_cast<const osg::Array*>( source.colors3 ),
                    vertexCount
                ) )
            {
                return false;
            }
        }

        return true;
    }

    bool
    allPrimitiveSetsAreTriangles( const osg::Geometry& geometry )
    {
        if( geometry.getNumPrimitiveSets() == 0U )
        {
            return false;
        }

        for( unsigned int primitiveIndex = 0U;
             primitiveIndex < geometry.getNumPrimitiveSets();
             ++primitiveIndex )
        {
            const osg::PrimitiveSet* primitiveSet =
                geometry.getPrimitiveSet( primitiveIndex );
            if( primitiveSet ==
                nullptr ||
                primitiveSet->getMode() !=
                GL_TRIANGLES ||
                ( primitiveSet->getNumIndices() % 3U ) != 0U )
            {
                return false;
            }
        }

        return true;
    }

    bool
    validTriangleIndices( const osg::Vec3Array& vertices,
                          unsigned int          i0,
                          unsigned int          i1,
                          unsigned int          i2 )
    {
        const unsigned int vertexCount = vertices.getNumElements();
        return i0 < vertexCount && i1 < vertexCount && i2 < vertexCount;
    }

    unsigned int
    triangleSubdivisions( const osg::Vec3Array& vertices,
                          const osg::dmat4&     localToWorld,
                          unsigned int          i0,
                          unsigned int          i1,
                          unsigned int          i2,
                          float                 maxEdgeLength,
                          unsigned int          maxSubdivisions )
    {
        if( !validTriangleIndices( vertices, i0, i1, i2 ) )
        {
            return 1U;
        }

        const osg::vec3 v0          = transformPoint( localToWorld, vertices[i0] );
        const osg::vec3 v1          = transformPoint( localToWorld, vertices[i1] );
        const osg::vec3 v2          = transformPoint( localToWorld, vertices[i2] );
        const float     longestEdge = triangleLongestEdge( v0, v1, v2 );
        if( longestEdge <= maxEdgeLength )
        {
            return 1U;
        }

        const float rawSubdivisions = std::ceil( longestEdge / maxEdgeLength );
        const auto  unclamped =
            static_cast<unsigned int>( std::max( rawSubdivisions, 1.0F ) );
        return std::clamp( unclamped, 1U, maxSubdivisions );
    }

    DensifyPlan
    planDensifyGeometry( const osg::Geometry&  geometry,
                         const osg::Vec3Array& vertices,
                         const osg::dmat4&     localToWorld,
                         float                 maxEdgeLength,
                         unsigned int          maxSubdivisions )
    {
        DensifyPlan plan;
        for( unsigned int primitiveIndex = 0U;
             primitiveIndex < geometry.getNumPrimitiveSets();
             ++primitiveIndex )
        {
            const osg::PrimitiveSet* primitiveSet =
                geometry.getPrimitiveSet( primitiveIndex );
            if( primitiveSet == nullptr )
            {
                continue;
            }

            const unsigned int indexCount = primitiveSet->getNumIndices();
            for( unsigned int index = 0U; index + 2U < indexCount; index += 3U )
            {
                const unsigned int i0 = primitiveSet->index( index );
                const unsigned int i1 = primitiveSet->index( index + 1U );
                const unsigned int i2 = primitiveSet->index( index + 2U );
                if( !validTriangleIndices( vertices, i0, i1, i2 ) )
                {
                    continue;
                }

                const unsigned int subdivisions =
                    triangleSubdivisions( vertices,
                                          localToWorld,
                                          i0,
                                          i1,
                                          i2,
                                          maxEdgeLength,
                                          maxSubdivisions );
                ++plan.inputTriangles;
                plan.outputTriangles += static_cast<std::size_t>( subdivisions ) *
                                        static_cast<std::size_t>( subdivisions );
                if( subdivisions > 1U )
                {
                    const std::size_t gridSize =
                        static_cast<std::size_t>( subdivisions + 1U ) *
                        static_cast<std::size_t>( subdivisions + 2U ) /
                        2U;
                    plan.outputVertices += gridSize;
                    plan.changed         = true;
                }
            }
        }
        return plan;
    }

    template<typename T>
    T
    interpolateVertexValue( const T&                  v0,
                            const T&                  v1,
                            const T&                  v2,
                            const BarycentricWeights& weights )
    {
        return v0 * weights.w0 + v1 * weights.w1 + v2 * weights.w2;
    }

    BarycentricWeights
    gridWeights( unsigned int gridI,
                 unsigned int gridJ,
                 unsigned int subdivisions )
    {
        const float        invSubdivisions = 1.0F / static_cast<float>( subdivisions );
        BarycentricWeights weights;
        weights.w1 = static_cast<float>( gridI ) * invSubdivisions;
        weights.w2 = static_cast<float>( gridJ ) * invSubdivisions;
        weights.w0 = 1.0F - weights.w1 - weights.w2;
        return weights;
    }

    unsigned int
    appendDensifiedVertex( const DensifySourceArrays& source,
                           DensifyOutputArrays&       output,
                           unsigned int               i0,
                           unsigned int               i1,
                           unsigned int               i2,
                           const BarycentricWeights&  weights )
    {
        const auto newIndex = static_cast<unsigned int>( output.vertices->size() );
        output.vertices->push_back( interpolateVertexValue( ( *source.vertices )[i0],
                                                            ( *source.vertices )[i1],
                                                            ( *source.vertices )[i2],
                                                            weights ) );

        const osg::vec3 normal =
            safeNormalize( interpolateVertexValue( ( *source.normals )[i0],
                                                   ( *source.normals )[i1],
                                                   ( *source.normals )[i2],
                                                   weights ),
                           ( *source.normals )[i0] );
        output.normals->push_back( normal );

        const osg::vec4 tangent4 = interpolateVertexValue( ( *source.tangents )[i0],
                                                           ( *source.tangents )[i1],
                                                           ( *source.tangents )[i2],
                                                           weights );
        const osg::vec3 tangent =
            safeNormalize( osg::vec3( tangent4.x, tangent4.y, tangent4.z ),
                           osg::vec3( ( *source.tangents )[i0].x,
                                      ( *source.tangents )[i0].y,
                                      ( *source.tangents )[i0].z ) );
        output.tangents->push_back( osg::vec4( tangent.x,
                                               tangent.y,
                                               tangent.z,
                                               tangent4.w < 0.0F ? -1.0F : 1.0F ) );

        output.texCoord0->push_back( interpolateVertexValue( ( *source.texCoord0 )[i0],
                                                             ( *source.texCoord0 )[i1],
                                                             ( *source.texCoord0 )[i2],
                                                             weights ) );
        if( output.texCoord1.valid() && source.texCoord1 != nullptr )
        {
            output.texCoord1->push_back(
                interpolateVertexValue( ( *source.texCoord1 )[i0],
                                        ( *source.texCoord1 )[i1],
                                        ( *source.texCoord1 )[i2],
                                        weights )
            );
        }
        if( output.colors4.valid() && source.colors4 != nullptr )
        {
            output.colors4->push_back( interpolateVertexValue( ( *source.colors4 )[i0],
                                                               ( *source.colors4 )[i1],
                                                               ( *source.colors4 )[i2],
                                                               weights ) );
        }
        if( output.colors3.valid() && source.colors3 != nullptr )
        {
            output.colors3->push_back( interpolateVertexValue( ( *source.colors3 )[i0],
                                                               ( *source.colors3 )[i1],
                                                               ( *source.colors3 )[i2],
                                                               weights ) );
        }
        return newIndex;
    }

    void
    appendOutputTriangle( DensifyOutputArrays& output,
                          unsigned int         i0,
                          unsigned int         i1,
                          unsigned int         i2 )
    {
        output.indices->addElement( i0 );
        output.indices->addElement( i1 );
        output.indices->addElement( i2 );
    }

    std::size_t
    densifyGridIndex( unsigned int gridI,
                      unsigned int gridJ,
                      unsigned int subdivisions )
    {
        return static_cast<std::size_t>( gridI ) *
               static_cast<std::size_t>( subdivisions + 1U ) -
               ( static_cast<std::size_t>( gridI ) *
                 static_cast<std::size_t>( gridI - ( gridI > 0U ? 1U : 0U ) ) ) /
               2U +
               static_cast<std::size_t>( gridJ );
    }

    void
    appendDensifiedTriangle( const DensifySourceArrays& source,
                             DensifyOutputArrays&       output,
                             unsigned int               i0,
                             unsigned int               i1,
                             unsigned int               i2,
                             unsigned int               subdivisions )
    {
        if( subdivisions <= 1U )
        {
            appendOutputTriangle( output, i0, i1, i2 );
            return;
        }

        std::vector<unsigned int> gridIndices;
        gridIndices.reserve( static_cast<std::size_t>( subdivisions + 1U ) *
                             static_cast<std::size_t>( subdivisions + 2U ) /
                             2U );
        for( unsigned int gridI = 0U; gridI <= subdivisions; ++gridI )
        {
            for( unsigned int gridJ = 0U; gridI + gridJ <= subdivisions; ++gridJ )
            {
                gridIndices.push_back(
                    appendDensifiedVertex( source,
                                           output,
                                           i0,
                                           i1,
                                           i2,
                                           gridWeights( gridI, gridJ, subdivisions ) )
                );
            }
        }

        for( unsigned int gridI = 0U; gridI < subdivisions; ++gridI )
        {
            for( unsigned int gridJ = 0U; gridI + gridJ < subdivisions; ++gridJ )
            {
                appendOutputTriangle(
                    output,
                    gridIndices[densifyGridIndex( gridI, gridJ, subdivisions )],
                    gridIndices[densifyGridIndex( gridI + 1U, gridJ, subdivisions )],
                    gridIndices[densifyGridIndex( gridI, gridJ + 1U, subdivisions )]
                );
                if( gridI + gridJ + 1U < subdivisions )
                {
                    appendOutputTriangle(
                        output,
                        gridIndices[densifyGridIndex( gridI + 1U, gridJ, subdivisions )],
                        gridIndices[densifyGridIndex( gridI + 1U,
                                                      gridJ + 1U,
                                                      subdivisions )],
                        gridIndices[densifyGridIndex( gridI, gridJ + 1U, subdivisions )]
                    );
                }
            }
        }
    }

    DensifyOutputArrays
    makeDensifyOutputArrays( const DensifySourceArrays& source,
                             std::size_t                expectedVertexCount,
                             std::size_t                expectedIndexCount )
    {
        DensifyOutputArrays output;
        output.vertices  = new osg::Vec3Array( *source.vertices );
        output.normals   = new osg::Vec3Array( *source.normals );
        output.tangents  = new osg::Vec4Array( *source.tangents );
        output.texCoord0 = new osg::Vec2Array( *source.texCoord0 );
        output.indices   = new osg::DrawElementsUInt( GL_TRIANGLES );
        if( source.texCoord1 != nullptr )
        {
            output.texCoord1 = new osg::Vec2Array( *source.texCoord1 );
        }
        if( source.colors4 != nullptr )
        {
            output.colors4 = new osg::Vec4Array( *source.colors4 );
        }
        if( source.colors3 != nullptr )
        {
            output.colors3 = new osg::Vec3Array( *source.colors3 );
        }

        output.vertices->reserve( expectedVertexCount );
        output.normals->reserve( expectedVertexCount );
        output.tangents->reserve( expectedVertexCount );
        output.texCoord0->reserve( expectedVertexCount );
        if( output.texCoord1.valid() )
        {
            output.texCoord1->reserve( expectedVertexCount );
        }
        if( output.colors4.valid() )
        {
            output.colors4->reserve( expectedVertexCount );
        }
        if( output.colors3.valid() )
        {
            output.colors3->reserve( expectedVertexCount );
        }
        output.indices->reserveElements(
            static_cast<unsigned int>( expectedIndexCount )
        );

        output.normals->setNormalize( source.normals->getNormalize() );
        output.tangents->setNormalize( source.tangents->getNormalize() );
        output.texCoord0->setNormalize( source.texCoord0->getNormalize() );
        if( output.texCoord1.valid() && source.texCoord1 != nullptr )
        {
            output.texCoord1->setNormalize( source.texCoord1->getNormalize() );
        }
        if( output.colors4.valid() && source.colors4 != nullptr )
        {
            output.colors4->setNormalize( source.colors4->getNormalize() );
        }
        if( output.colors3.valid() && source.colors3 != nullptr )
        {
            output.colors3->setNormalize( source.colors3->getNormalize() );
        }
        return output;
    }

    void
    replaceGeometryWithDensifiedArrays( osg::Geometry&             geometry,
                                        const DensifyOutputArrays& output )
    {
        geometry.setVertexArray( output.vertices.get() );
        geometry.setNormalArray( output.normals.get(), osg::Array::BIND_PER_VERTEX );
        geometry.setColorArray( nullptr );
        if( output.colors4.valid() )
        {
            geometry.setColorArray( output.colors4.get(), osg::Array::BIND_PER_VERTEX );
        }
        else if( output.colors3.valid() )
        {
            geometry.setColorArray( output.colors3.get(), osg::Array::BIND_PER_VERTEX );
        }

        osg::Geometry::ArrayList emptyArrays;
        geometry.setTexCoordArrayList( emptyArrays );
        geometry.setTexCoordArray( 0U,
                                   output.texCoord0.get(),
                                   osg::Array::BIND_PER_VERTEX );
        if( output.texCoord1.valid() )
        {
            geometry.setTexCoordArray( 1U,
                                       output.texCoord1.get(),
                                       osg::Array::BIND_PER_VERTEX );
        }

        geometry.setVertexAttribArrayList( emptyArrays );
        geometry.setVertexAttribArray( tangentAttribLocation,
                                       output.tangents.get(),
                                       osg::Array::BIND_PER_VERTEX );

        osg::Geometry::PrimitiveSetList primitiveSets;
        primitiveSets.push_back( output.indices.get() );
        geometry.setPrimitiveSetList( primitiveSets );
    }

    bool
    densifyGeometryForBake( osg::Geometry&    geometry,
                            const osg::dmat4& localToWorld,
                            float             maxEdgeLength,
                            unsigned int      maxSubdivisions,
                            DensifyStats&     stats )
    {
        ++stats.visitedGeometryCount;

        DensifySourceArrays source;
        std::size_t         vertexCount = 0U;
        if( !readDensifySourceArrays( geometry, source, vertexCount ) ||
            !allPrimitiveSetsAreTriangles( geometry ) )
        {
            ++stats.skippedGeometryCount;
            return false;
        }

        const DensifyPlan plan  = planDensifyGeometry( geometry,
                                                       *source.vertices,
                                                       localToWorld,
                                                       maxEdgeLength,
                                                       maxSubdivisions );
        stats.inputTriangles   += plan.inputTriangles;
        stats.outputTriangles  += plan.outputTriangles;
        if( !plan.changed || plan.inputTriangles == 0U )
        {
            return false;
        }

        const std::size_t expectedVertexCount = vertexCount + plan.outputVertices;
        const std::size_t expectedIndexCount  = plan.outputTriangles * 3U;
        const std::size_t maxDrawCount =
            static_cast<std::size_t>( std::numeric_limits<GLsizei>::max() );
        const std::size_t maxIndex =
            static_cast<std::size_t>( std::numeric_limits<unsigned int>::max() );
        if( expectedVertexCount > maxIndex || expectedIndexCount > maxDrawCount )
        {
            ++stats.skippedGeometryCount;
            return false;
        }

        DensifyOutputArrays output =
            makeDensifyOutputArrays( source, expectedVertexCount, expectedIndexCount );
        for( unsigned int primitiveIndex = 0U;
             primitiveIndex < geometry.getNumPrimitiveSets();
             ++primitiveIndex )
        {
            const osg::PrimitiveSet* primitiveSet =
                geometry.getPrimitiveSet( primitiveIndex );
            if( primitiveSet == nullptr )
            {
                continue;
            }

            const unsigned int indexCount = primitiveSet->getNumIndices();
            for( unsigned int index = 0U; index + 2U < indexCount; index += 3U )
            {
                const unsigned int i0 = primitiveSet->index( index );
                const unsigned int i1 = primitiveSet->index( index + 1U );
                const unsigned int i2 = primitiveSet->index( index + 2U );
                if( !validTriangleIndices( *source.vertices, i0, i1, i2 ) )
                {
                    continue;
                }

                const unsigned int subdivisions =
                    triangleSubdivisions( *source.vertices,
                                          localToWorld,
                                          i0,
                                          i1,
                                          i2,
                                          maxEdgeLength,
                                          maxSubdivisions );
                appendDensifiedTriangle( source, output, i0, i1, i2, subdivisions );
            }
        }

        replaceGeometryWithDensifiedArrays( geometry, output );
        ++stats.changedGeometryCount;
        return true;
    }

    class BakeDensifyVisitor : public osg::NodeVisitor
    {
        public:

            BakeDensifyVisitor( float        maxEdgeLength,
                                unsigned int maxSubdivisions ) :
                osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
                _maxEdgeLength( maxEdgeLength ),
                _maxSubdivisions( maxSubdivisions )
            {
                _worldStack.push_back( osg::dmat4() );
            }

            void
            apply( osg::Transform& transform ) override
            {
                osg::dmat4 world = _worldStack.back();
                transform.computeLocalToWorldMatrix( world, this );
                _worldStack.push_back( world );
                traverse( transform );
                _worldStack.pop_back();
            }

            void
            apply( osg::Drawable& drawable ) override
            {
                osg::Geometry* geometry = drawable.asGeometry();
                if( geometry == nullptr )
                {
                    return;
                }

                densifyGeometryForBake( *geometry,
                                        _worldStack.back(),
                                        _maxEdgeLength,
                                        _maxSubdivisions,
                                        stats );
            }

            DensifyStats stats;

        private:

            std::vector<osg::dmat4> _worldStack;
            float                   _maxEdgeLength   = bakeMaxEdgeLength;
            unsigned int            _maxSubdivisions = bakeMaxSubdivisions;
    };

    DensifyStats
    densifyBakeGeometry( osg::Node&   model,
                         float        maxEdgeLength,
                         unsigned int maxSubdivisions )
    {
        BakeDensifyVisitor visitor( maxEdgeLength, maxSubdivisions );
        model.accept( visitor );
        return visitor.stats;
    }

    class GeometryRecordVisitor : public osg::NodeVisitor
    {
        public:

            explicit GeometryRecordVisitor( MaterialTable& materialTable ) :
                osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
                _materialTable( materialTable )
            {
                _worldStack.push_back( osg::dmat4() );
            }

            void
            apply( osg::Transform& transform ) override
            {
                osg::dmat4 world = _worldStack.back();
                transform.computeLocalToWorldMatrix( world, this );
                _worldStack.push_back( world );
                traverse( transform );
                _worldStack.pop_back();
            }

            void
            apply( osg::Drawable& drawable ) override
            {
                osg::Geometry* geometry = drawable.asGeometry();
                if( geometry == nullptr )
                {
                    return;
                }
                const std::uint32_t materialIndex =
                    _materialTable.materialIndex( drawable.getStateSet() );

                const osg::Array*     vertexArray = geometry->getVertexArray();
                const osg::Vec3Array* vertices =
                    dynamic_cast<const osg::Vec3Array*>( geometry->getVertexArray() );
                const osg::Vec3Array* normals =
                    dynamic_cast<const osg::Vec3Array*>( geometry->getNormalArray() );
                const std::size_t vertexCount =
                    vertexArray
                        ? static_cast<std::size_t>( vertexArray->getNumElements() )
                        : 0U;
                const bool hasNormals =
                    vertices &&
                    normals &&
                    static_cast<std::size_t>( normals->getNumElements() ) >= vertexCount;

                GeometryRecord record;
                record.geometry     = geometry;
                record.vertices     = vertices;
                record.normals      = normals;
                record.localToWorld = _worldStack.back();
                record.normalMatrix = makeNormalMatrix( record.localToWorld );
                record.name         = drawable.getName();
                if( record.name.empty() && drawable.getStateSet() != nullptr )
                {
                    record.name = drawable.getStateSet()->getName();
                }
                record.vertexCount      = vertexCount;
                record.hasNormals       = hasNormals;
                record.materialIndex    = materialIndex;
                record.excludedOccluder = isExcludedOccluder( drawable );
                if( vertices != nullptr )
                {
                    record.adjacentFaces.assign( vertexCount,
                                                 std::vector<AdjacentFace>() );
                    for( unsigned int primitiveIndex = 0U;
                         primitiveIndex < geometry->getNumPrimitiveSets();
                         ++primitiveIndex )
                    {
                        const osg::PrimitiveSet* primitiveSet =
                            geometry->getPrimitiveSet( primitiveIndex );
                        if( primitiveSet != nullptr )
                        {
                            appendPrimitiveAdjacentFaces( record,
                                                          *vertices,
                                                          *primitiveSet );
                        }
                    }
                }
                records.push_back( record );
            }

            std::vector<GeometryRecord> records;

        private:

            std::vector<osg::dmat4> _worldStack;
            MaterialTable&          _materialTable;
    };

    std::vector<GeometryRecord>
    collectGeometryRecords( osg::Node&     model,
                            MaterialTable& materialTable )
    {
        GeometryRecordVisitor visitor( materialTable );
        model.accept( visitor );
        return visitor.records;
    }

    std::vector<Triangle>
    collectOccluderTriangles( const std::vector<GeometryRecord>& records )
    {
        std::vector<Triangle> triangles;
        for( std::size_t recordIndex = 0U; recordIndex < records.size(); ++recordIndex )
        {
            const GeometryRecord& record = records[recordIndex];
            if( record.geometry ==
                nullptr ||
                record.vertices ==
                nullptr ||
                record.excludedOccluder )
            {
                continue;
            }

            for( unsigned int primitiveIndex = 0U;
                 primitiveIndex < record.geometry->getNumPrimitiveSets();
                 ++primitiveIndex )
            {
                const osg::PrimitiveSet* primitiveSet =
                    record.geometry->getPrimitiveSet( primitiveIndex );
                if( primitiveSet != nullptr )
                {
                    appendPrimitiveTriangles(
                        triangles,
                        *record.vertices,
                        record.localToWorld,
                        record.materialIndex,
                        *primitiveSet,
                        static_cast<std::uint32_t>( recordIndex )
                    );
                }
            }
        }
        return triangles;
    }

    std::uint32_t
    buildBvhNode( Bvh&        bvh,
                  std::size_t start,
                  std::size_t end )
    {
        const std::size_t nodeIndex = bvh.nodes.size();
        bvh.nodes.push_back( BvhNode{} );

        osg::box bounds;
        osg::box centroidBounds;
        for( std::size_t i = start; i < end; ++i )
        {
            const Triangle& triangle =
                bvh.triangles[static_cast<std::size_t>( bvh.triangleIndices[i] )];
            bounds.add( triangle.bounds );
            centroidBounds.expandBy( triangle.centroid );
        }

        bvh.nodes[nodeIndex].bounds = bounds;

        const std::size_t count     = end - start;
        if( count <= bvhLeafSize )
        {
            bvh.nodes[nodeIndex].start = static_cast<std::uint32_t>( start );
            bvh.nodes[nodeIndex].count = static_cast<std::uint32_t>( count );
            return static_cast<std::uint32_t>( nodeIndex );
        }

        const osg::vec3 extent = centroidBounds.max - centroidBounds.min;
        unsigned int    axis   = 0U;
        if( extent.y > extent.x && extent.y >= extent.z )
        {
            axis = 1U;
        }
        else if( extent.z > extent.x && extent.z > extent.y )
        {
            axis = 2U;
        }

        const std::size_t mid = start + count / 2U;
        std::nth_element(
            bvh.triangleIndices.begin() + static_cast<std::ptrdiff_t>( start ),
            bvh.triangleIndices.begin() + static_cast<std::ptrdiff_t>( mid ),
            bvh.triangleIndices.begin() + static_cast<std::ptrdiff_t>( end ),
            [&bvh, axis]( std::uint32_t lhs, std::uint32_t rhs )
            {
                return axisValue(
                           bvh.triangles[static_cast<std::size_t>( lhs )].centroid,
                           axis
                       ) <
                       axisValue(
                           bvh.triangles[static_cast<std::size_t>( rhs )].centroid,
                           axis
                       );
            }
        );

        const std::uint32_t left   = buildBvhNode( bvh, start, mid );
        const std::uint32_t right  = buildBvhNode( bvh, mid, end );
        bvh.nodes[nodeIndex].left  = left;
        bvh.nodes[nodeIndex].right = right;
        return static_cast<std::uint32_t>( nodeIndex );
    }

    Bvh
    buildBvh( std::vector<Triangle>&& triangles )
    {
        Bvh bvh;
        bvh.triangles = std::move( triangles );
        bvh.triangleIndices.resize( bvh.triangles.size() );
        std::iota( bvh.triangleIndices.begin(),
                   bvh.triangleIndices.end(),
                   std::uint32_t{ 0U } );
        if( !bvh.triangles.empty() )
        {
            buildBvhNode( bvh, 0U, bvh.triangles.size() );
        }
        return bvh;
    }

    bool
    rayIntersectsBox( const osg::box&  bounds,
                      const osg::vec3& origin,
                      const osg::vec3& direction,
                      float            maxDistance )
    {
        float tMin = 0.0F;
        float tMax = maxDistance;

        for( unsigned int axis = 0U; axis < 3U; ++axis )
        {
            const float originAxis    = axisValue( origin, axis );
            const float directionAxis = axisValue( direction, axis );
            const float minAxis       = axisValue( bounds.min, axis );
            const float maxAxis       = axisValue( bounds.max, axis );
            if( std::fabs( directionAxis ) < 1.0E-8F )
            {
                if( originAxis < minAxis || originAxis > maxAxis )
                {
                    return false;
                }
                continue;
            }

            const float inverseDirection = 1.0F / directionAxis;
            float       t0               = ( minAxis - originAxis ) * inverseDirection;
            float       t1               = ( maxAxis - originAxis ) * inverseDirection;
            if( t0 > t1 )
            {
                std::swap( t0, t1 );
            }

            tMin = std::max( tMin, t0 );
            tMax = std::min( tMax, t1 );
            if( tMin > tMax )
            {
                return false;
            }
        }

        return tMax >= rayHitEpsilon;
    }

    bool
    rayTriangleIntersection( const osg::vec3& origin,
                             const osg::vec3& direction,
                             float            maxDistance,
                             const Triangle&  triangle,
                             float&           distance )
    {
        const osg::vec3 edge1 = triangle.v1 - triangle.v0;
        const osg::vec3 edge2 = triangle.v2 - triangle.v0;
        const osg::vec3 pvec  = osg::cross( direction, edge2 );
        const float     det   = osg::dot( edge1, pvec );
        if( std::fabs( det ) < 1.0E-8F )
        {
            return false;
        }

        const float     invDet = 1.0F / det;
        const osg::vec3 tvec   = origin - triangle.v0;
        const float     u      = osg::dot( tvec, pvec ) * invDet;
        if( u < 0.0F || u > 1.0F )
        {
            return false;
        }

        const osg::vec3 qvec = osg::cross( tvec, edge1 );
        const float     v    = osg::dot( direction, qvec ) * invDet;
        if( v < 0.0F || u + v > 1.0F )
        {
            return false;
        }

        const float t = osg::dot( edge2, qvec ) * invDet;
        if( t <= rayHitEpsilon || t > maxDistance )
        {
            return false;
        }

        distance = t;
        return true;
    }

    bool
    rayIntersectsTriangle( const osg::vec3& origin,
                           const osg::vec3& direction,
                           float            maxDistance,
                           const Triangle&  triangle )
    {
        float distance = 0.0F;
        return rayTriangleIntersection( origin,
                                        direction,
                                        maxDistance,
                                        triangle,
                                        distance );
    }

    bool
    bvhAnyHit( const Bvh&       bvh,
               const osg::vec3& origin,
               const osg::vec3& direction,
               float            maxDistance )
    {
        if( bvh.nodes.empty() )
        {
            return false;
        }

        std::array<std::uint32_t, 96U> stack{};
        std::size_t                    stackSize = 1U;
        stack[0]                                 = 0U;

        while( stackSize > 0U )
        {
            --stackSize;
            const BvhNode& node =
                bvh.nodes[static_cast<std::size_t>( stack[stackSize] )];
            if( !rayIntersectsBox( node.bounds, origin, direction, maxDistance ) )
            {
                continue;
            }

            if( node.count > 0U )
            {
                const std::size_t end = static_cast<std::size_t>( node.start ) +
                                        static_cast<std::size_t>( node.count );
                for( std::size_t i = static_cast<std::size_t>( node.start ); i < end;
                     ++i )
                {
                    const Triangle& triangle = bvh.triangles[static_cast<std::size_t>(
                        bvh.triangleIndices[i]
                    )];
                    if( rayIntersectsTriangle( origin,
                                               direction,
                                               maxDistance,
                                               triangle ) )
                    {
                        return true;
                    }
                }
            }
            else
            {
                if( stackSize + 2U > stack.size() )
                {
                    return false;
                }
                stack[stackSize] = node.left;
                ++stackSize;
                stack[stackSize] = node.right;
                ++stackSize;
            }
        }

        return false;
    }

    RayHit
    bvhClosestHit( const Bvh&       bvh,
                   const osg::vec3& origin,
                   const osg::vec3& direction,
                   float            maxDistance )
    {
        RayHit result;
        if( bvh.nodes.empty() )
        {
            return result;
        }

        std::array<std::uint32_t, 96U> stack{};
        std::size_t                    stackSize = 1U;
        stack[0]                                 = 0U;

        while( stackSize > 0U )
        {
            --stackSize;
            const BvhNode& node =
                bvh.nodes[static_cast<std::size_t>( stack[stackSize] )];
            const float currentMax = result.hit ? result.distance : maxDistance;
            if( !rayIntersectsBox( node.bounds, origin, direction, currentMax ) )
            {
                continue;
            }

            if( node.count > 0U )
            {
                const std::size_t end = static_cast<std::size_t>( node.start ) +
                                        static_cast<std::size_t>( node.count );
                for( std::size_t i = static_cast<std::size_t>( node.start ); i < end;
                     ++i )
                {
                    const std::uint32_t triangleIndex = bvh.triangleIndices[i];
                    const Triangle&     triangle =
                        bvh.triangles[static_cast<std::size_t>( triangleIndex )];
                    float       distance    = 0.0F;
                    const float triangleMax = result.hit ? result.distance : currentMax;
                    if( rayTriangleIntersection( origin,
                                                 direction,
                                                 triangleMax,
                                                 triangle,
                                                 distance ) )
                    {
                        result.hit           = true;
                        result.distance      = distance;
                        result.triangleIndex = triangleIndex;
                    }
                }
            }
            else
            {
                if( stackSize + 2U > stack.size() )
                {
                    break;
                }
                stack[stackSize] = node.left;
                ++stackSize;
                stack[stackSize] = node.right;
                ++stackSize;
            }
        }

        return result;
    }

    float
    radicalInverseVdC( std::uint32_t bits )
    {
        bits = ( bits << 16U ) | ( bits >> 16U );
        bits = ( ( bits & 0X55'55'55'55U ) << 1U ) | ( ( bits & 0XAA'AA'AA'AAU ) >> 1U );
        bits = ( ( bits & 0X33'33'33'33U ) << 2U ) | ( ( bits & 0XCC'CC'CC'CCU ) >> 2U );
        bits = ( ( bits & 0X0F'0F'0F'0FU ) << 4U ) | ( ( bits & 0XF0'F0'F0'F0U ) >> 4U );
        bits = ( ( bits & 0X00'FF'00'FFU ) << 8U ) | ( ( bits & 0XFF'00'FF'00U ) >> 8U );
        return static_cast<float>( static_cast<double>( bits ) *
                                   2.3283064365386963E-10 );
    }

    std::vector<osg::vec3>
    makeHemisphereSamples( int rayCount )
    {
        std::vector<osg::vec3> samples;
        samples.reserve( static_cast<std::size_t>( rayCount ) );
        const float invRayCount = 1.0F / static_cast<float>( rayCount );
        for( int i = 0; i < rayCount; ++i )
        {
            const float u1  = ( static_cast<float>( i ) + 0.5F ) * invRayCount;
            const float u2  = radicalInverseVdC( static_cast<std::uint32_t>( i ) );
            const float r   = std::sqrt( u1 );
            const float phi = 2.0F * pi * u2;
            samples.push_back( osg::vec3( r * std::cos( phi ),
                                          r * std::sin( phi ),
                                          std::sqrt( std::max( 0.0F, 1.0F - u1 ) ) ) );
        }
        return samples;
    }

    void
    makeBasis( const osg::vec3& normal,
               osg::vec3&       tangent,
               osg::vec3&       bitangent )
    {
        const osg::vec3 up = std::fabs( normal.z ) < 0.999F
                               ? osg::vec3( 0.0F, 0.0F, 1.0F )
                               : osg::vec3( 0.0F, 1.0F, 0.0F );
        tangent =
            safeNormalize( osg::cross( up, normal ), osg::vec3( 1.0F, 0.0F, 0.0F ) );
        bitangent = safeNormalize( osg::cross( normal, tangent ),
                                   osg::vec3( 0.0F, 1.0F, 0.0F ) );
    }

    int
    wrapTexelIndex( int value,
                    int size )
    {
        if( size <= 0 )
        {
            return 0;
        }
        int wrapped = value % size;
        if( wrapped < 0 )
        {
            wrapped += size;
        }
        return wrapped;
    }

    osg::vec3
    readSkyTexel( const osg::Image& image,
                  int               x,
                  int               y )
    {
        const osg::dvec3 color = readFloatRgb( image, x, y );
        const double     lum   = luminance( color );
        if( lum > sunDiscLuminance )
        {
            const double scale = sunDiscLuminance / lum;
            return osg::vec3( static_cast<float>( color.r * scale ),
                              static_cast<float>( color.g * scale ),
                              static_cast<float>( color.b * scale ) );
        }
        return osg::vec3( static_cast<float>( color.r ),
                          static_cast<float>( color.g ),
                          static_cast<float>( color.b ) );
    }

    void
    logEnvironmentStats( const osg::Image& image )
    {
        const int width  = image.s();
        const int height = image.t();
        if( width <= 0 || height <= 0 )
        {
            return;
        }

        double meanLuminance = 0.0;
        double maxLuminance  = 0.0;
        for( int y = 0; y < height; ++y )
        {
            for( int x = 0; x < width; ++x )
            {
                const double lum  = luminance( readFloatRgb( image, x, y ) );
                meanLuminance    += lum;
                maxLuminance      = std::max( maxLuminance, lum );
            }
        }
        meanLuminance /= static_cast<double>( width ) * static_cast<double>( height );

        OSG_NOTICE << "Sponza radiance bake environment HDR " << width << "x" << height
                   << " GL_FLOAT "
                   << ( image.getPixelFormat() == GL_RGBA ? "RGBA" : "RGB" )
                   << ", mean luminance " << meanLuminance << ", max luminance "
                   << maxLuminance << std::endl;
    }

    class EnvironmentSampler
    {
        public:

            EnvironmentSampler( const osg::Image* image,
                                float             envRotation ) :
                _image( image ),
                _envRotation( envRotation )
            {
                _valid = image !=
                         nullptr &&
                         canReadFloatRgb( *image ) &&
                         image->s() >
                         0 &&
                         image->t() > 0;
                if( !_valid && image != nullptr )
                {
                    OSG_WARN << "Sponza radiance bake requires GL_RGB/GL_RGBA float HDR "
                             << "image data for escaped sky rays" << std::endl;
                }
                if( _valid )
                {
                    logEnvironmentStats( *image );
                }
            }

            osg::vec3
            sample( const osg::vec3& dirWorld ) const
            {
                if( !_valid || _image == nullptr )
                {
                    return osg::vec3( 0.0F, 0.0F, 0.0F );
                }

                const osg::vec3 direction =
                    safeNormalize( dirWorld, osg::vec3( 0.0F, 1.0F, 0.0F ) );
                const double lon =
                    ( std::fabs( static_cast<double>( direction.x ) ) +
                      std::fabs( static_cast<double>( direction.z ) ) < 1.0E-5 )
                        ? 0.0
                        : std::atan2( static_cast<double>( direction.z ),
                                      static_cast<double>( direction.x ) );
                double u =
                    lon / twoPi + 0.5 + static_cast<double>( _envRotation ) / twoPi;
                u = u - std::floor( u );
                const double v =
                    std::acos(
                        std::clamp( static_cast<double>( direction.y ), -1.0, 1.0 )
                    ) /
                    static_cast<double>( pi );

                const int    width  = _image->s();
                const int    height = _image->t();
                const double imageX = u * static_cast<double>( width ) - 0.5;
                const double imageY =
                    std::clamp( ( 1.0 - v ) * static_cast<double>( height ) - 0.5,
                                0.0,
                                static_cast<double>( height - 1 ) );

                const double    floorX = std::floor( imageX );
                const double    floorY = std::floor( imageY );
                const int       x0     = static_cast<int>( floorX );
                const int       y0     = static_cast<int>( floorY );
                const int       x1     = x0 + 1;
                const int       y1     = std::min( y0 + 1, height - 1 );
                const float     tx     = static_cast<float>( imageX - floorX );
                const float     ty     = static_cast<float>( imageY - floorY );

                const osg::vec3 c00 =
                    readSkyTexel( *_image, wrapTexelIndex( x0, width ), y0 );
                const osg::vec3 c10 =
                    readSkyTexel( *_image, wrapTexelIndex( x1, width ), y0 );
                const osg::vec3 c01 =
                    readSkyTexel( *_image, wrapTexelIndex( x0, width ), y1 );
                const osg::vec3 c11 =
                    readSkyTexel( *_image, wrapTexelIndex( x1, width ), y1 );
                const osg::vec3 cx0 = c00 * ( 1.0F - tx ) + c10 * tx;
                const osg::vec3 cx1 = c01 * ( 1.0F - tx ) + c11 * tx;
                return cx0 * ( 1.0F - ty ) + cx1 * ty;
            }

        private:

            const osg::Image* _image       = nullptr;
            float             _envRotation = 0.0F;
            bool              _valid       = false;
    };

    std::size_t
    extraSamplePointCount( const GeometryRecord& record,
                           std::size_t           vertexIndex )
    {
        if( vertexIndex >= record.adjacentFaces.size() )
        {
            return 0U;
        }

        return std::min( maxExtraSamplePoints,
                         record.adjacentFaces[vertexIndex].size() *
                             faceSampleFractions.size() );
    }

    osg::vec3
    insetTowardCentroid( const osg::vec3& position,
                         const osg::vec3& centroid,
                         float            fraction )
    {
        const osg::vec3 toCentroid = centroid - position;
        const float     distance   = osg::length( toCentroid );
        if( distance <= 1.0E-5F )
        {
            return position;
        }

        const float requested = distance * fraction;
        const float nudge     = std::min( facePlaneNudge, distance * 0.9F );
        const float inset = std::min( distance * 0.95F, std::max( requested, nudge ) );
        return position + toCentroid * ( inset / distance );
    }

    std::vector<VertexSamplePoint>
    makeVertexSamplePoints( const GeometryRecord& record,
                            std::size_t           vertexIndex,
                            const osg::vec3&      position,
                            const osg::vec3&      vertexNormal )
    {
        std::vector<VertexSamplePoint> points;
        points.reserve( 1U + maxExtraSamplePoints );

        float longestAdjacentEdge = 0.0F;
        if( vertexIndex < record.adjacentFaces.size() )
        {
            for( const AdjacentFace& face : record.adjacentFaces[vertexIndex] )
            {
                longestAdjacentEdge = std::max( longestAdjacentEdge, face.longestEdge );
            }
        }

        points.push_back( VertexSamplePoint{
            position,
            vertexNormal,
            adaptiveOriginOffset( longestAdjacentEdge ),
            false
        } );

        if( vertexIndex >= record.adjacentFaces.size() )
        {
            return points;
        }

        std::size_t extraPoints = 0U;
        for( const AdjacentFace& face : record.adjacentFaces[vertexIndex] )
        {
            osg::vec3 faceNormal = safeNormalize( face.normal, vertexNormal );
            if( osg::dot( faceNormal, vertexNormal ) < 0.0F )
            {
                faceNormal = -faceNormal;
            }

            for( float fraction : faceSampleFractions )
            {
                if( extraPoints >= maxExtraSamplePoints )
                {
                    return points;
                }

                points.push_back( VertexSamplePoint{
                    insetTowardCentroid( position, face.centroid, fraction ),
                    faceNormal,
                    adaptiveOriginOffset( face.longestEdge ),
                    true
                } );
                ++extraPoints;
            }
        }

        return points;
    }

    osg::vec3
    clampNonNegative( const osg::vec3& value )
    {
        return osg::vec3( std::max( value.r, 0.0F ),
                          std::max( value.g, 0.0F ),
                          std::max( value.b, 0.0F ) );
    }

    float
    vertexVisibility( const BakeArrays& bake,
                      std::uint32_t     recordIndex,
                      std::uint32_t     vertexIndex )
    {
        if( recordIndex ==
            invalidRecordIndex ||
            recordIndex >=
            bake.visibility.size() ||
            vertexIndex >= bake.visibility[recordIndex].size() )
        {
            return 1.0F;
        }
        return std::clamp( bake.visibility[recordIndex][vertexIndex].w, 0.0F, 1.0F );
    }

    float
    averageTriangleVisibility( const Triangle&   triangle,
                               const BakeArrays& bake )
    {
        return ( vertexVisibility( bake, triangle.recordIndex, triangle.i0 ) +
                 vertexVisibility( bake, triangle.recordIndex, triangle.i1 ) +
                 vertexVisibility( bake, triangle.recordIndex, triangle.i2 ) ) *
               ( 1.0F / 3.0F );
    }

    float
    interpolateTriangleVisibility( const Triangle&   triangle,
                                   const osg::vec3&  hitPoint,
                                   const BakeArrays& bake )
    {
        if( triangle.recordIndex == invalidRecordIndex )
        {
            return 1.0F;
        }

        const osg::vec3 edge0 = triangle.v1 - triangle.v0;
        const osg::vec3 edge1 = triangle.v2 - triangle.v0;
        const osg::vec3 toHit = hitPoint - triangle.v0;
        const float     d00   = osg::dot( edge0, edge0 );
        const float     d01   = osg::dot( edge0, edge1 );
        const float     d11   = osg::dot( edge1, edge1 );
        const float     d20   = osg::dot( toHit, edge0 );
        const float     d21   = osg::dot( toHit, edge1 );
        const float     denom = d00 * d11 - d01 * d01;
        if( std::fabs( denom ) <= 1.0E-10F )
        {
            return averageTriangleVisibility( triangle, bake );
        }

        float w1              = ( d11 * d20 - d01 * d21 ) / denom;
        float w2              = ( d00 * d21 - d01 * d20 ) / denom;
        float w0              = 1.0F - w1 - w2;
        w0                    = std::clamp( w0, 0.0F, 1.0F );
        w1                    = std::clamp( w1, 0.0F, 1.0F );
        w2                    = std::clamp( w2, 0.0F, 1.0F );

        const float weightSum = w0 + w1 + w2;
        if( weightSum <= 1.0E-6F )
        {
            return averageTriangleVisibility( triangle, bake );
        }
        const float invWeightSum  = 1.0F / weightSum;
        w0                       *= invWeightSum;
        w1                       *= invWeightSum;
        w2                       *= invWeightSum;

        return std::clamp(
            vertexVisibility( bake, triangle.recordIndex, triangle.i0 ) *
                w0 +
                vertexVisibility( bake, triangle.recordIndex, triangle.i1 ) *
                w1 +
                vertexVisibility( bake, triangle.recordIndex, triangle.i2 ) *
                w2,
            0.0F,
            1.0F
        );
    }

    PointBake
    computePointVisibility( const VertexSamplePoint&      point,
                            const Bvh&                    bvh,
                            const std::vector<osg::vec3>& samples,
                            float                         visibilityMaxDistance )
    {
        PointBake       result;
        const osg::vec3 normal =
            safeNormalize( point.normal, osg::vec3( 0.0F, 1.0F, 0.0F ) );
        result.bentNormal = normal;
        if( samples.empty() )
        {
            return result;
        }

        osg::vec3 tangent;
        osg::vec3 bitangent;
        makeBasis( normal, tangent, bitangent );

        const osg::vec3 origin     = point.position + normal * point.originOffset;
        std::size_t     unoccluded = 0U;
        osg::vec3       unoccludedDirectionSum( 0.0F, 0.0F, 0.0F );
        for( const osg::vec3& sample : samples )
        {
            const osg::vec3 direction = safeNormalize(
                tangent * sample.x + bitangent * sample.y + normal * sample.z,
                normal
            );
            if( !bvhAnyHit( bvh, origin, direction, visibilityMaxDistance ) )
            {
                ++unoccluded;
                unoccludedDirectionSum += direction;
            }
        }

        result.visibility =
            static_cast<float>( unoccluded ) / static_cast<float>( samples.size() );
        result.bentNormal = result.visibility <=
                                    1.0E-4F ||
                                    osg::length2( unoccludedDirectionSum ) <=
                                    minNormalLength2
                              ? normal
                              : safeNormalize( unoccludedDirectionSum, normal );
        return result;
    }

    PointBake
    computePointRadiance( const VertexSamplePoint&                point,
                          const Bvh&                              bvh,
                          const BakeArrays&                       bake,
                          const MaterialTable&                    materialTable,
                          const EnvironmentSampler&               environment,
                          const std::vector<osg::vec3>&           samples,
                          const osg::vec3&                        sunDirection,
                          const osg::vec3&                        sunRadiance,
                          const sponza::IrradianceShCoefficients& skyIrradianceSh,
                          bool                                    skyIrradianceValid,
                          float                                   visibilityMaxDistance,
                          float                                   radianceMaxDistance )
    {
        PointBake       result;
        const osg::vec3 normal =
            safeNormalize( point.normal, osg::vec3( 0.0F, 1.0F, 0.0F ) );
        result.bentNormal = normal;
        if( samples.empty() )
        {
            return result;
        }

        osg::vec3 tangent;
        osg::vec3 bitangent;
        makeBasis( normal, tangent, bitangent );

        const osg::vec3 origin     = point.position + normal * point.originOffset;
        std::size_t     unoccluded = 0U;
        osg::vec3       unoccludedDirectionSum( 0.0F, 0.0F, 0.0F );
        osg::vec3       radianceSum( 0.0F, 0.0F, 0.0F );
        for( const osg::vec3& sample : samples )
        {
            const osg::vec3 direction = safeNormalize(
                tangent * sample.x + bitangent * sample.y + normal * sample.z,
                normal
            );
            const RayHit hit =
                bvhClosestHit( bvh, origin, direction, radianceMaxDistance );
            if( !hit.hit || hit.distance > visibilityMaxDistance )
            {
                ++unoccluded;
                unoccludedDirectionSum += direction;
            }

            if( !hit.hit )
            {
                radianceSum += environment.sample( direction );
                continue;
            }

            const Triangle& triangle =
                bvh.triangles[static_cast<std::size_t>( hit.triangleIndex )];
            const osg::vec3 hitPoint = origin + direction * hit.distance;
            const osg::vec3 hitNormal =
                safeNormalize( triangle.normal, osg::vec3( 0.0F, 1.0F, 0.0F ) );
            osg::vec3   hitIrradiance( 0.0F, 0.0F, 0.0F );

            const float nDotSun = osg::dot( hitNormal, sunDirection );
            if( nDotSun > 0.0F )
            {
                const osg::vec3 shadowOrigin =
                    hitPoint + hitNormal * adaptiveOriginOffset( triangle.longestEdge );
                if( !bvhAnyHit( bvh, shadowOrigin, sunDirection, radianceMaxDistance ) )
                {
                    hitIrradiance += sunRadiance * nDotSun;
                }
            }

            if( skyIrradianceValid )
            {
                const float hitVisibility =
                    interpolateTriangleVisibility( triangle, hitPoint, bake );
                const osg::vec3 skyEOverPi = clampNonNegative(
                    sponza::evaluateIrradianceShEOverPi( skyIrradianceSh, hitNormal )
                );
                // SH coefficients store sky irradiance divided by PI. Convert back to
                // E_sky = PI * EoverPI * V(P), add sun irradiance E_sun, then emit
                // diffuse outgoing radiance L_hit = albedo / PI * (E_sun + E_sky).
                hitIrradiance += skyEOverPi * ( pi * hitVisibility );
            }

            radianceSum +=
                multiplyComponents( materialTable.albedo( triangle.materialIndex ),
                                    hitIrradiance ) *
                ( 1.0F / pi );
        }

        result.visibility =
            static_cast<float>( unoccluded ) / static_cast<float>( samples.size() );
        result.bentNormal = result.visibility <=
                                    1.0E-4F ||
                                    osg::length2( unoccludedDirectionSum ) <=
                                    minNormalLength2
                              ? normal
                              : safeNormalize( unoccludedDirectionSum, normal );
        // The shader multiplies this value by albedo directly, matching the existing
        // SH path where coefficients are irradiance divided by PI. With cosine-weighted
        // samples (pdf = cos(theta) / PI), E / PI is the mean incoming radiance.
        result.radiance = radianceSum * ( 1.0F / static_cast<float>( samples.size() ) );
        return result;
    }

    osg::vec3
    applyMultibounceClosure( const osg::vec3& radiance,
                             float            visibility,
                             float            rhoBar )
    {
        // Approximate higher diffuse bounces as a radiosity series:
        // E_total = E_1 / (1 - rhoBar * (1 - V)), where V is sky visibility.
        const float denominator =
            std::max( minMultibounceDenom,
                      1.0F - rhoBar * ( 1.0F - std::clamp( visibility, 0.0F, 1.0F ) ) );
        return radiance * ( 1.0F / denominator );
    }

    osg::vec4
    computeVertexVisibility( const GeometryRecord&         record,
                             std::size_t                   vertexIndex,
                             const Bvh&                    bvh,
                             const std::vector<osg::vec3>& primarySamples,
                             const std::vector<osg::vec3>& extraSamples,
                             float                         visibilityMaxDistance )
    {
        osg::vec4 result( 0.0F, 1.0F, 0.0F, 1.0F );
        if( record.vertices ==
            nullptr ||
            record.normals ==
            nullptr ||
            vertexIndex >=
            record.vertexCount ||
            vertexIndex >= static_cast<std::size_t>( record.normals->getNumElements() ) )
        {
            return result;
        }

        const osg::vec3 transformedNormal = transformNormal(
            record.normalMatrix,
            ( *record.normals )[static_cast<unsigned int>( vertexIndex )]
        );
        if( osg::length2( transformedNormal ) <= minNormalLength2 )
        {
            return result;
        }
        const osg::vec3 normal =
            safeNormalize( transformedNormal, osg::vec3( 0.0F, 1.0F, 0.0F ) );

        const osg::vec3 position = transformPoint(
            record.localToWorld,
            ( *record.vertices )[static_cast<unsigned int>( vertexIndex )]
        );
        const std::vector<VertexSamplePoint> samplePoints =
            makeVertexSamplePoints( record, vertexIndex, position, normal );

        float     visibilitySum = 0.0F;
        osg::vec3 bentNormalSum( 0.0F, 0.0F, 0.0F );
        for( const VertexSamplePoint& samplePoint : samplePoints )
        {
            PointBake pointBake =
                computePointVisibility( samplePoint,
                                        bvh,
                                        samplePoint.extra ? extraSamples
                                                          : primarySamples,
                                        visibilityMaxDistance );

            visibilitySum += pointBake.visibility;
            bentNormalSum +=
                pointBake.bentNormal * std::max( pointBake.visibility, 1.0E-4F );
        }

        const float invPointCount =
            1.0F / static_cast<float>( std::max<std::size_t>( samplePoints.size(),
                                                              std::size_t{ 1U } ) );
        const float     visibility = visibilitySum * invPointCount;
        const osg::vec3 bentNormal = osg::length2( bentNormalSum ) <= minNormalLength2
                                       ? normal
                                       : safeNormalize( bentNormalSum, normal );
        result                     = osg::vec4( bentNormal, visibility );
        return result;
    }

    osg::vec3
    computeVertexRadiance( const GeometryRecord&                   record,
                           std::size_t                             vertexIndex,
                           const Bvh&                              bvh,
                           const BakeArrays&                       bake,
                           const MaterialTable&                    materialTable,
                           const EnvironmentSampler&               environment,
                           const std::vector<osg::vec3>&           primarySamples,
                           const std::vector<osg::vec3>&           extraSamples,
                           const osg::vec3&                        sunDirection,
                           const osg::vec3&                        sunRadiance,
                           const sponza::IrradianceShCoefficients& skyIrradianceSh,
                           bool                                    skyIrradianceValid,
                           float                                   visibilityMaxDistance,
                           float                                   radianceMaxDistance,
                           float                                   rhoBar,
                           bool                                    multibounceEnabled )
    {
        if( record.vertices ==
            nullptr ||
            record.normals ==
            nullptr ||
            vertexIndex >=
            record.vertexCount ||
            vertexIndex >= static_cast<std::size_t>( record.normals->getNumElements() ) )
        {
            return osg::vec3( 0.0F, 0.0F, 0.0F );
        }

        const osg::vec3 transformedNormal = transformNormal(
            record.normalMatrix,
            ( *record.normals )[static_cast<unsigned int>( vertexIndex )]
        );
        if( osg::length2( transformedNormal ) <= minNormalLength2 )
        {
            return osg::vec3( 0.0F, 0.0F, 0.0F );
        }
        const osg::vec3 normal =
            safeNormalize( transformedNormal, osg::vec3( 0.0F, 1.0F, 0.0F ) );

        const osg::vec3 position = transformPoint(
            record.localToWorld,
            ( *record.vertices )[static_cast<unsigned int>( vertexIndex )]
        );
        const std::vector<VertexSamplePoint> samplePoints =
            makeVertexSamplePoints( record, vertexIndex, position, normal );

        osg::vec3 radianceSum( 0.0F, 0.0F, 0.0F );
        for( const VertexSamplePoint& samplePoint : samplePoints )
        {
            PointBake pointBake =
                computePointRadiance( samplePoint,
                                      bvh,
                                      bake,
                                      materialTable,
                                      environment,
                                      samplePoint.extra ? extraSamples : primarySamples,
                                      sunDirection,
                                      sunRadiance,
                                      skyIrradianceSh,
                                      skyIrradianceValid,
                                      visibilityMaxDistance,
                                      radianceMaxDistance );
            if( multibounceEnabled )
            {
                pointBake.radiance = applyMultibounceClosure( pointBake.radiance,
                                                              pointBake.visibility,
                                                              rhoBar );
            }
            radianceSum += pointBake.radiance;
        }

        const float invPointCount =
            1.0F / static_cast<float>( std::max<std::size_t>( samplePoints.size(),
                                                              std::size_t{ 1U } ) );
        const osg::vec3 result = radianceSum * invPointCount;
        return result;
    }

    struct SamplingExample
    {
            std::string name;
            std::size_t vertexCount      = 0U;
            double      averageExtra     = 0.0;
            std::size_t maxAdjacentFaces = 0U;
            float       longestEdge      = 0.0F;
    };

    void
    logVertexSamplingStats( const std::vector<GeometryRecord>& records,
                            int                                primaryRayCount,
                            int                                extraRayCount )
    {
        std::size_t                  totalVertices      = 0U;
        std::size_t                  verticesWithFaces  = 0U;
        std::size_t                  totalAdjacentFaces = 0U;
        std::size_t                  totalExtraPoints   = 0U;
        std::size_t                  maxAdjacentFaces   = 0U;
        std::size_t                  maxExtraPoints     = 0U;
        std::vector<SamplingExample> examples;

        for( std::size_t recordIndex = 0U; recordIndex < records.size(); ++recordIndex )
        {
            const GeometryRecord& record = records[recordIndex];
            if( record.vertexCount == 0U )
            {
                continue;
            }

            SamplingExample example;
            example.name =
                record.name.empty()
                    ? std::string( "<geometry " ) + std::to_string( recordIndex ) + ">"
                    : record.name;
            example.vertexCount           = record.vertexCount;

            std::size_t recordExtraPoints = 0U;
            for( std::size_t vertexIndex = 0U; vertexIndex < record.vertexCount;
                 ++vertexIndex )
            {
                const std::size_t adjacentCount =
                    vertexIndex < record.adjacentFaces.size()
                        ? record.adjacentFaces[vertexIndex].size()
                        : 0U;
                const std::size_t extraCount =
                    extraSamplePointCount( record, vertexIndex );
                totalVertices      += 1U;
                totalAdjacentFaces += adjacentCount;
                totalExtraPoints   += extraCount;
                recordExtraPoints  += extraCount;
                maxAdjacentFaces    = std::max( maxAdjacentFaces, adjacentCount );
                maxExtraPoints      = std::max( maxExtraPoints, extraCount );
                example.maxAdjacentFaces =
                    std::max( example.maxAdjacentFaces, adjacentCount );
                if( adjacentCount > 0U )
                {
                    ++verticesWithFaces;
                }
                if( vertexIndex < record.adjacentFaces.size() )
                {
                    for( const AdjacentFace& face : record.adjacentFaces[vertexIndex] )
                    {
                        example.longestEdge =
                            std::max( example.longestEdge, face.longestEdge );
                    }
                }
            }

            example.averageExtra = static_cast<double>( recordExtraPoints ) /
                                   static_cast<double>( record.vertexCount );
            if( example.longestEdge > 0.0F )
            {
                examples.push_back( example );
            }
        }

        const double averageExtra    = totalVertices == 0U
                                         ? 0.0
                                         : static_cast<double>( totalExtraPoints ) /
                                               static_cast<double>( totalVertices );
        const double averageAdjacent = totalVertices == 0U
                                         ? 0.0
                                         : static_cast<double>( totalAdjacentFaces ) /
                                               static_cast<double>( totalVertices );
        const double rayMultiplier   = primaryRayCount <= 0
                                         ? 0.0
                                         : ( static_cast<double>( primaryRayCount ) +
                                             averageExtra *
                                             static_cast<double>( extraRayCount ) ) /
                                               static_cast<double>( primaryRayCount );

        OSG_NOTICE << "Sponza radiance bake vertex sampling: " << totalVertices
                   << " vertices, " << verticesWithFaces
                   << " with adjacent faces, avg adjacent faces/vertex "
                   << averageAdjacent << ", avg extra points/vertex " << averageExtra
                   << ", max adjacent faces " << maxAdjacentFaces
                   << ", max extra points " << maxExtraPoints << ", primary rays/point "
                   << primaryRayCount << ", extra rays/point " << extraRayCount
                   << ", estimated ray multiplier " << rayMultiplier << std::endl;

        std::sort( examples.begin(),
                   examples.end(),
                   []( const SamplingExample& lhs, const SamplingExample& rhs )
                   {
                       return lhs.longestEdge > rhs.longestEdge;
                   } );
        const std::size_t exampleCount = std::min<std::size_t>( examples.size(), 3U );
        for( std::size_t i = 0U; i < exampleCount; ++i )
        {
            const SamplingExample& example = examples[i];
            OSG_NOTICE << "Sponza radiance bake large-face sample record: \""
                       << example.name << "\" vertices " << example.vertexCount
                       << ", avg extra points/vertex " << example.averageExtra
                       << ", max adjacent faces " << example.maxAdjacentFaces
                       << ", longest edge " << example.longestEdge << std::endl;
        }
    }

    BakeArrays
    bakeVisibilityAndRadiance(
        const std::vector<GeometryRecord>& records,
        const Bvh&                         bvh,
        const MaterialTable&               materialTable,
        const osg::Image*                  envImage,
        const sponza::IrradianceShResult*  precomputedSkyIrradiance,
        const sponza::SponzaOptions&       options,
        int                                rayCount,
        float                              visibilityMaxDistance,
        float                              radianceMaxDistance
    )
    {
        BakeArrays bake;
        bake.visibility.resize( records.size() );
        bake.radiance.resize( records.size() );
        std::vector<std::size_t> offsets;
        offsets.reserve( records.size() + 1U );
        offsets.push_back( 0U );

        std::size_t totalVertices = 0U;
        for( std::size_t i = 0U; i < records.size(); ++i )
        {
            bake.visibility[i].assign( records[i].vertexCount,
                                       osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
            bake.radiance[i].assign( records[i].vertexCount,
                                     osg::vec3( 0.0F, 0.0F, 0.0F ) );
            totalVertices += records[i].vertexCount;
            offsets.push_back( totalVertices );
        }

        if( totalVertices == 0U )
        {
            return bake;
        }

        // Face-inset points use half rays to bound the multipoint bake cost.
        const int                    extraRayCount  = std::max( 1, rayCount / 2 );
        const std::vector<osg::vec3> primarySamples = makeHemisphereSamples( rayCount );
        const std::vector<osg::vec3> extraSamples =
            makeHemisphereSamples( extraRayCount );
        const EnvironmentSampler environment( envImage, options.envRotation );
        const osg::dvec3 sunDirectionD = sponza::computeSunDirectionWorld( options );
        const osg::vec3  sunDirection =
            safeNormalize( osg::vec3( static_cast<float>( sunDirectionD.x ),
                                      static_cast<float>( sunDirectionD.y ),
                                      static_cast<float>( sunDirectionD.z ) ),
                           osg::vec3( 0.0F, 1.0F, 0.0F ) );
        const osg::vec3 sunRadiance =
            sponza::scaledColor( options.sunColor, options.sunIntensity );
        const float rhoBar = materialTable.averageAlbedoReflectance();
        OSG_NOTICE << "Sponza radiance bake multibounce "
                   << ( options.radianceMultibounce ? "on" : "off" ) << ", rhoBar "
                   << rhoBar << std::endl;
        std::atomic<std::size_t> nextVertex{ 0U };
        const std::size_t        chunkSize       = 64U;
        const unsigned int       hardwareThreads = std::thread::hardware_concurrency();
        std::size_t              threadCount =
            std::max<std::size_t>( 1U, static_cast<std::size_t>( hardwareThreads ) );
        threadCount           = std::min( threadCount, totalVertices );

        auto visibilityWorker = [&records,
                                 &bake,
                                 &offsets,
                                 &nextVertex,
                                 &primarySamples,
                                 &extraSamples,
                                 &bvh,
                                 visibilityMaxDistance,
                                 totalVertices]()
        {
            while( true )
            {
                const std::size_t begin = nextVertex.fetch_add( chunkSize );
                if( begin >= totalVertices )
                {
                    break;
                }
                const std::size_t end = std::min( begin + chunkSize, totalVertices );
                std::size_t       recordIndex =
                    static_cast<std::size_t>(
                        std::upper_bound( offsets.begin(), offsets.end(), begin ) -
                        offsets.begin()
                    ) -
                    1U;

                for( std::size_t globalIndex = begin; globalIndex < end; ++globalIndex )
                {
                    while( globalIndex >= offsets[recordIndex + 1U] )
                    {
                        ++recordIndex;
                    }
                    const std::size_t vertexIndex = globalIndex - offsets[recordIndex];
                    if( records[recordIndex].hasNormals )
                    {
                        bake.visibility[recordIndex][vertexIndex] =
                            computeVertexVisibility( records[recordIndex],
                                                     vertexIndex,
                                                     bvh,
                                                     primarySamples,
                                                     extraSamples,
                                                     visibilityMaxDistance );
                    }
                }
            }
        };

        OSG_NOTICE << "Sponza radiance bake pass 1/2: visibility and bent normals"
                   << std::endl;
        std::vector<std::thread> threads;
        threads.reserve( threadCount );
        for( std::size_t i = 0U; i < threadCount; ++i )
        {
            threads.emplace_back( visibilityWorker );
        }
        for( std::thread& thread : threads )
        {
            thread.join();
        }

        sponza::IrradianceShCoefficients skyIrradianceSh{};
        for( osg::vec3& coefficient : skyIrradianceSh )
        {
            coefficient.set( 0.0F, 0.0F, 0.0F );
        }
        bool skyIrradianceValid = false;
        if( precomputedSkyIrradiance != nullptr && precomputedSkyIrradiance->valid )
        {
            skyIrradianceSh    = precomputedSkyIrradiance->coefficients;
            skyIrradianceValid = true;
            OSG_NOTICE << "Sponza radiance bake shared sky SH E/pi up-normal "
                       << "reconstruction: (" << precomputedSkyIrradiance->upNormal.r
                       << ", " << precomputedSkyIrradiance->upNormal.g << ", "
                       << precomputedSkyIrradiance->upNormal.b << "), excluded "
                       << precomputedSkyIrradiance->excludedSunPixels
                       << " sun-disc pixels" << std::endl;
        }
        else if( envImage != nullptr )
        {
            const sponza::IrradianceShResult irradianceSh =
                sponza::computeSunExcludedIrradianceSh( *envImage );
            if( irradianceSh.valid )
            {
                skyIrradianceSh    = irradianceSh.coefficients;
                skyIrradianceValid = true;
                OSG_NOTICE << "Sponza radiance bake sky SH E/pi up-normal "
                           << "reconstruction: (" << irradianceSh.upNormal.r << ", "
                           << irradianceSh.upNormal.g << ", " << irradianceSh.upNormal.b
                           << "), excluded " << irradianceSh.excludedSunPixels
                           << " sun-disc pixels" << std::endl;
            }
        }
        if( !skyIrradianceValid )
        {
            OSG_WARN << "Sponza radiance bake sky-lit hit bounce disabled; no valid "
                     << "environment SH irradiance" << std::endl;
        }

        nextVertex.store( 0U );
        auto radianceWorker = [&records,
                               &bake,
                               &offsets,
                               &nextVertex,
                               &primarySamples,
                               &extraSamples,
                               &bvh,
                               &materialTable,
                               &environment,
                               &skyIrradianceSh,
                               sunDirection,
                               sunRadiance,
                               skyIrradianceValid,
                               visibilityMaxDistance,
                               radianceMaxDistance,
                               rhoBar,
                               multibounceEnabled = options.radianceMultibounce,
                               totalVertices]()
        {
            while( true )
            {
                const std::size_t begin = nextVertex.fetch_add( chunkSize );
                if( begin >= totalVertices )
                {
                    break;
                }
                const std::size_t end = std::min( begin + chunkSize, totalVertices );
                std::size_t       recordIndex =
                    static_cast<std::size_t>(
                        std::upper_bound( offsets.begin(), offsets.end(), begin ) -
                        offsets.begin()
                    ) -
                    1U;

                for( std::size_t globalIndex = begin; globalIndex < end; ++globalIndex )
                {
                    while( globalIndex >= offsets[recordIndex + 1U] )
                    {
                        ++recordIndex;
                    }
                    const std::size_t vertexIndex = globalIndex - offsets[recordIndex];
                    if( records[recordIndex].hasNormals )
                    {
                        bake.radiance[recordIndex][vertexIndex] =
                            computeVertexRadiance( records[recordIndex],
                                                   vertexIndex,
                                                   bvh,
                                                   bake,
                                                   materialTable,
                                                   environment,
                                                   primarySamples,
                                                   extraSamples,
                                                   sunDirection,
                                                   sunRadiance,
                                                   skyIrradianceSh,
                                                   skyIrradianceValid,
                                                   visibilityMaxDistance,
                                                   radianceMaxDistance,
                                                   rhoBar,
                                                   multibounceEnabled );
                    }
                }
            }
        };

        OSG_NOTICE << "Sponza radiance bake pass 2/2: sky-aware radiance gathering"
                   << std::endl;
        threads.clear();
        threads.reserve( threadCount );
        for( std::size_t i = 0U; i < threadCount; ++i )
        {
            threads.emplace_back( radianceWorker );
        }
        for( std::thread& thread : threads )
        {
            thread.join();
        }

        return bake;
    }

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
        cachePath                       += ".visbake";
        if( options.bakeDensifyEnabled )
        {
            cachePath += densifyCacheSuffix( options );
        }
        return cachePath;
    }

    enum class CacheLoadStatus
    {
        Missing,
        Stale,
        Loaded
    };

    bool
    cacheLightingKeyChanged( double                       cachedSunAzimuth,
                             double                       cachedSunElevation,
                             float                        cachedSunIntensity,
                             const osg::vec3&             cachedSunColor,
                             float                        cachedEnvRotation,
                             float                        cachedRadianceDistance,
                             std::uint8_t                 cachedRadianceMultibounce,
                             const sponza::SponzaOptions& options,
                             float                        radianceDistance )
    {
        return cachedSunAzimuth !=
               options.sunAzimuthDeg ||
               cachedSunElevation !=
               options.sunElevationDeg ||
               cachedSunIntensity !=
               options.sunIntensity ||
               cachedSunColor !=
               options.sunColor ||
               cachedEnvRotation !=
               options.envRotation ||
               cachedRadianceDistance !=
               radianceDistance ||
               cachedRadianceMultibounce !=
               static_cast<std::uint8_t>( options.radianceMultibounce ? 1U : 0U );
    }

    CacheLoadStatus
    loadVisibilityCache( const std::filesystem::path&       cachePath,
                         const ModelStamp&                  stamp,
                         int                                rayCount,
                         float                              rayDistance,
                         float                              radianceDistance,
                         const sponza::SponzaOptions&       options,
                         const std::vector<GeometryRecord>& records,
                         BakeArrays&                        bake )
    {
        osgDB::ifstream stream( cachePath.string().c_str(),
                                std::ios::in | std::ios::binary );
        if( !stream )
        {
            return CacheLoadStatus::Missing;
        }

        std::array<char, 8U> magic{};
        std::uint32_t        version = 0U;
        if( !readValue( stream, magic.data(), magic.size() ) ||
            !readPod( stream, version ) )
        {
            return CacheLoadStatus::Stale;
        }

        if( magic != cacheMagic )
        {
            OSG_NOTICE << "Sponza visibility/radiance bake cache magic changed; "
                       << "rebaking " << cachePath.string() << std::endl;
            return CacheLoadStatus::Stale;
        }
        if( version != cacheVersion )
        {
            OSG_NOTICE << "Sponza visibility/radiance bake cache version changed to v"
                       << cacheVersion << "; rebaking " << cachePath.string()
                       << std::endl;
            return CacheLoadStatus::Stale;
        }

        std::uint32_t cachedRayCount         = 0U;
        float         cachedDistance         = 0.0F;
        float         cachedRadianceDistance = 0.0F;
        double        cachedSunAzimuth       = 0.0;
        double        cachedSunElevation     = 0.0;
        float         cachedSunIntensity     = 0.0F;
        osg::vec3     cachedSunColor( 0.0F, 0.0F, 0.0F );
        float         cachedEnvRotation         = 0.0F;
        std::uint8_t  cachedRadianceMultibounce = 0U;
        std::uint64_t cachedSize                = 0U;
        std::int64_t  cachedMtime               = 0;
        std::uint64_t geometryCount             = 0U;
        if( !readPod( stream, cachedRayCount ) ||
            !readPod( stream, cachedDistance ) ||
            !readPod( stream, cachedRadianceDistance ) ||
            !readPod( stream, cachedSunAzimuth ) ||
            !readPod( stream, cachedSunElevation ) ||
            !readPod( stream, cachedSunIntensity ) ||
            !readPod( stream, cachedSunColor ) ||
            !readPod( stream, cachedEnvRotation ) ||
            !readPod( stream, cachedRadianceMultibounce ) ||
            !readPod( stream, cachedSize ) ||
            !readPod( stream, cachedMtime ) ||
            !readPod( stream, geometryCount ) )
        {
            return CacheLoadStatus::Stale;
        }

        if( cacheLightingKeyChanged( cachedSunAzimuth,
                                     cachedSunElevation,
                                     cachedSunIntensity,
                                     cachedSunColor,
                                     cachedEnvRotation,
                                     cachedRadianceDistance,
                                     cachedRadianceMultibounce,
                                     options,
                                     radianceDistance ) )
        {
            OSG_NOTICE << "Sponza radiance bake cache lighting key changed; rebaking "
                       << cachePath.string() << std::endl;
            return CacheLoadStatus::Stale;
        }

        if( cachedRayCount !=
            static_cast<std::uint32_t>( rayCount ) ||
            cachedDistance !=
            rayDistance ||
            cachedSize !=
            stamp.size ||
            cachedMtime !=
            stamp.mtimeTicks ||
            geometryCount != static_cast<std::uint64_t>( records.size() ) )
        {
            OSG_NOTICE << "Sponza visibility/radiance bake cache geometry key changed; "
                       << "rebaking " << cachePath.string() << std::endl;
            return CacheLoadStatus::Stale;
        }

        std::vector<std::uint64_t> vertexCounts( records.size(), 0U );
        for( std::size_t i = 0U; i < records.size(); ++i )
        {
            if( !readPod( stream, vertexCounts[i] ) ||
                vertexCounts[i] != static_cast<std::uint64_t>( records[i].vertexCount ) )
            {
                return CacheLoadStatus::Stale;
            }
        }

        bake.visibility.resize( records.size() );
        bake.radiance.resize( records.size() );
        for( std::size_t i = 0U; i < records.size(); ++i )
        {
            bake.visibility[i].resize( records[i].vertexCount,
                                       osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
            if( !bake.visibility[i].empty() &&
                !readValue( stream,
                            bake.visibility[i].data(),
                            bake.visibility[i].size() * sizeof( osg::vec4 ) ) )
            {
                return CacheLoadStatus::Stale;
            }
        }

        for( std::size_t i = 0U; i < records.size(); ++i )
        {
            bake.radiance[i].resize( records[i].vertexCount,
                                     osg::vec3( 0.0F, 0.0F, 0.0F ) );
            if( !bake.radiance[i].empty() &&
                !readValue( stream,
                            bake.radiance[i].data(),
                            bake.radiance[i].size() * sizeof( osg::vec3 ) ) )
            {
                return CacheLoadStatus::Stale;
            }
        }

        return CacheLoadStatus::Loaded;
    }

    bool
    saveVisibilityCache( const std::filesystem::path&       cachePath,
                         const ModelStamp&                  stamp,
                         int                                rayCount,
                         float                              rayDistance,
                         float                              radianceDistance,
                         const sponza::SponzaOptions&       options,
                         const std::vector<GeometryRecord>& records,
                         const BakeArrays&                  bake )
    {
        osgDB::ofstream stream( cachePath.string().c_str(),
                                std::ios::out | std::ios::binary | std::ios::trunc );
        if( !stream )
        {
            return false;
        }

        const std::uint32_t cachedRayCount = static_cast<std::uint32_t>( rayCount );
        const std::uint64_t geometryCount = static_cast<std::uint64_t>( records.size() );
        const std::uint8_t  radianceMultibounce =
            static_cast<std::uint8_t>( options.radianceMultibounce ? 1U : 0U );
        if( !writeValue( stream, cacheMagic.data(), cacheMagic.size() ) ||
            !writePod( stream, cacheVersion ) ||
            !writePod( stream, cachedRayCount ) ||
            !writePod( stream, rayDistance ) ||
            !writePod( stream, radianceDistance ) ||
            !writePod( stream, options.sunAzimuthDeg ) ||
            !writePod( stream, options.sunElevationDeg ) ||
            !writePod( stream, options.sunIntensity ) ||
            !writePod( stream, options.sunColor ) ||
            !writePod( stream, options.envRotation ) ||
            !writePod( stream, radianceMultibounce ) ||
            !writePod( stream, stamp.size ) ||
            !writePod( stream, stamp.mtimeTicks ) ||
            !writePod( stream, geometryCount ) )
        {
            return false;
        }

        for( const GeometryRecord& record : records )
        {
            const std::uint64_t vertexCount =
                static_cast<std::uint64_t>( record.vertexCount );
            if( !writePod( stream, vertexCount ) )
            {
                return false;
            }
        }

        for( const std::vector<osg::vec4>& values : bake.visibility )
        {
            if( !values.empty() && !writeValue( stream,
                                                values.data(),
                                                values.size() * sizeof( osg::vec4 ) ) )
            {
                return false;
            }
        }
        for( const std::vector<osg::vec3>& values : bake.radiance )
        {
            if( !values.empty() && !writeValue( stream,
                                                values.data(),
                                                values.size() * sizeof( osg::vec3 ) ) )
            {
                return false;
            }
        }

        return true;
    }

    void
    applyBakeAttributes( const std::vector<GeometryRecord>& records,
                         const BakeArrays&                  bake )
    {
        bool warnedExistingVisibilityAttribute = false;
        bool warnedExistingRadianceAttribute   = false;
        for( std::size_t recordIndex = 0U; recordIndex < records.size(); ++recordIndex )
        {
            osg::Geometry* geometry = records[recordIndex].geometry;
            if( geometry == nullptr || records[recordIndex].vertexCount == 0U )
            {
                continue;
            }

            if( geometry->getVertexAttribArray( visibilityAttribLocation ) !=
                nullptr &&
                !warnedExistingVisibilityAttribute )
            {
                OSG_WARN << "Sponza visibility bake replacing existing vertex "
                         << "attribute " << visibilityAttribLocation << std::endl;
                warnedExistingVisibilityAttribute = true;
            }
            if( geometry->getVertexAttribArray( radianceAttribLocation ) !=
                nullptr &&
                !warnedExistingRadianceAttribute )
            {
                OSG_WARN << "Sponza radiance bake replacing existing vertex "
                         << "attribute " << radianceAttribLocation << std::endl;
                warnedExistingRadianceAttribute = true;
            }

            osg::ref_ptr<osg::Vec4Array> visibilityArray = new osg::Vec4Array(
                static_cast<unsigned int>( records[recordIndex].vertexCount )
            );
            osg::ref_ptr<osg::Vec3Array> radianceArray = new osg::Vec3Array(
                static_cast<unsigned int>( records[recordIndex].vertexCount )
            );
            for( std::size_t i = 0U; i < records[recordIndex].vertexCount; ++i )
            {
                ( *visibilityArray )[static_cast<unsigned int>( i )] =
                    bake.visibility[recordIndex][i];
                ( *radianceArray )[static_cast<unsigned int>( i )] =
                    bake.radiance[recordIndex][i];
            }
            visibilityArray->setNormalize( false );
            radianceArray->setNormalize( false );
            geometry->setVertexAttribArray( visibilityAttribLocation,
                                            visibilityArray.get(),
                                            osg::Array::BIND_PER_VERTEX );
            geometry->setVertexAttribArray( radianceAttribLocation,
                                            radianceArray.get(),
                                            osg::Array::BIND_PER_VERTEX );
        }
    }

    void
    setVisibilityUniforms( osg::Node& model,
                           bool       hasVisibility,
                           bool       useRadianceBake,
                           bool       useRadianceDebug,
                           float      strength,
                           float      power,
                           float      bentStrength,
                           float      radianceScale )
    {
        osg::StateSet* stateSet = model.getOrCreateStateSet();
        stateSet->addUniform( new osg::Uniform( "uHasVisBake", hasVisibility ),
                              osg::StateAttribute::OVERRIDE );
        stateSet->addUniform( new osg::Uniform( "uVisStrength", strength ),
                              osg::StateAttribute::OVERRIDE );
        stateSet->addUniform( new osg::Uniform( "uVisPower", power ),
                              osg::StateAttribute::OVERRIDE );
        stateSet->addUniform( new osg::Uniform( "uVisBentStrength", bentStrength ),
                              osg::StateAttribute::OVERRIDE );
        stateSet->addUniform( new osg::Uniform( "uUseRadianceBake", useRadianceBake ),
                              osg::StateAttribute::OVERRIDE );
        stateSet->addUniform( new osg::Uniform( "uRadianceDebug", useRadianceDebug ),
                              osg::StateAttribute::OVERRIDE );
        stateSet->addUniform( new osg::Uniform( "uRadianceScale", radianceScale ),
                              osg::StateAttribute::OVERRIDE );
    }

    float
    computeSceneDiagonal( const std::vector<GeometryRecord>& records )
    {
        osg::box bounds;
        for( const GeometryRecord& record : records )
        {
            if( record.vertices == nullptr )
            {
                continue;
            }
            for( unsigned int i = 0U; i < record.vertices->getNumElements(); ++i )
            {
                bounds.expandBy( transformPoint( record.localToWorld,
                                                 ( *record.vertices )[i] ) );
            }
        }
        return bounds.valid() ? std::max( osg::length( bounds.max - bounds.min ), 1.0F )
                              : 1.0F;
    }

}

namespace sponza
{

    VisibilityBakeResult
    applyVisibilityBake( osg::Node*                model,
                         const SponzaOptions&      options,
                         const osg::Image*         preloadedEnvImage,
                         const IrradianceShResult* precomputedSkyIrradiance )
    {
        VisibilityBakeResult result;
        if( model == nullptr )
        {
            return result;
        }

        setVisibilityUniforms( *model,
                               false,
                               false,
                               false,
                               options.visBakeStrength,
                               options.visBakePower,
                               options.visBentStrength,
                               options.radianceScale );

        if( !options.visBakeEnabled )
        {
            OSG_NOTICE << "Sponza visibility bake disabled" << std::endl;
            return result;
        }

        const osg::Timer_t startTick = osg::Timer::instance()->tick();
        if( options.bakeDensifyEnabled )
        {
            const DensifyStats densifyStats = densifyBakeGeometry(
                *model,
                options.bakeDensifyMaxEdge,
                static_cast<unsigned int>( options.bakeDensifyMaxSubdiv )
            );
            if( densifyStats.changedGeometryCount > 0U )
            {
                OSG_NOTICE << "Sponza bake densified "
                           << densifyStats.changedGeometryCount << "/"
                           << densifyStats.visitedGeometryCount
                           << " geometries, triangles " << densifyStats.inputTriangles
                           << " -> " << densifyStats.outputTriangles << ", skipped "
                           << densifyStats.skippedGeometryCount << ", max edge "
                           << options.bakeDensifyMaxEdge << ", max subdivisions "
                           << options.bakeDensifyMaxSubdiv << std::endl;
            }
        }
        MaterialTable                     materialTable;
        const std::vector<GeometryRecord> records =
            collectGeometryRecords( *model, materialTable );
        materialTable.logStats();
        const std::size_t vertexCount =
            std::accumulate( records.begin(),
                             records.end(),
                             std::size_t{ 0U },
                             []( std::size_t total, const GeometryRecord& record )
                             {
                                 return total + record.vertexCount;
                             } );

        result.enabled       = true;
        result.geometryCount = records.size();
        result.vertexCount   = vertexCount;
        logVertexSamplingStats( records,
                                options.visBakeRays,
                                std::max( 1, options.visBakeRays / 2 ) );

        const std::filesystem::path modelPath = resolveModelPath( options.modelPath );
        const std::filesystem::path cachePath = cachePathForModel( modelPath, options );
        result.cachePath                      = cachePath.string();

        ModelStamp  stamp;
        const bool  hasStamp           = modelStamp( modelPath, stamp );
        const float sceneDiagonalEarly = computeSceneDiagonal( records );
        const float rayDistance =
            std::min( options.visBakeDistance, sceneDiagonalEarly );
        const float radianceDistance = sceneDiagonalEarly;
        BakeArrays  bake;
        if( hasStamp && !options.visBakeRefresh )
        {
            const CacheLoadStatus cacheStatus = loadVisibilityCache( cachePath,
                                                                     stamp,
                                                                     options.visBakeRays,
                                                                     rayDistance,
                                                                     radianceDistance,
                                                                     options,
                                                                     records,
                                                                     bake );
            if( cacheStatus == CacheLoadStatus::Loaded )
            {
                applyBakeAttributes( records, bake );
                setVisibilityUniforms( *model,
                                       true,
                                       options.radianceBakeEnabled,
                                       options.radianceDebug,
                                       options.visBakeStrength,
                                       options.visBakePower,
                                       options.visBentStrength,
                                       options.radianceScale );
                const osg::Timer_t endTick = osg::Timer::instance()->tick();
                result.loadedFromCache     = true;
                result.wallTimeSeconds =
                    osg::Timer::instance()->delta_s( startTick, endTick );
                OSG_NOTICE << "Sponza visibility/radiance bake loaded " << vertexCount
                           << " vertices from " << result.cachePath << " in "
                           << result.wallTimeSeconds << " s" << std::endl;
                return result;
            }
        }

        std::vector<Triangle> triangles = collectOccluderTriangles( records );
        result.occluderTriangles        = triangles.size();
        Bvh                      bvh    = buildBvh( std::move( triangles ) );

        osg::ref_ptr<osg::Image> loadedEnvImage;
        const osg::Image*        envImage = preloadedEnvImage;
        if( envImage == nullptr )
        {
            loadedEnvImage = sponza::loadEnvironmentImage();
            envImage       = loadedEnvImage.get();
        }
        if( envImage == nullptr )
        {
            OSG_WARN << "Sponza radiance bake could not load environment HDR; "
                     << "escaped rays contribute black" << std::endl;
        }
        bake = bakeVisibilityAndRadiance( records,
                                          bvh,
                                          materialTable,
                                          envImage,
                                          precomputedSkyIrradiance,
                                          options,
                                          options.visBakeRays,
                                          rayDistance,
                                          radianceDistance );
        applyBakeAttributes( records, bake );
        setVisibilityUniforms( *model,
                               true,
                               options.radianceBakeEnabled,
                               options.radianceDebug,
                               options.visBakeStrength,
                               options.visBakePower,
                               options.visBentStrength,
                               options.radianceScale );

        if( hasStamp )
        {
            std::error_code             error;
            const std::filesystem::path parent = cachePath.parent_path();
            if( !parent.empty() )
            {
                std::filesystem::create_directories( parent, error );
            }
            if( !saveVisibilityCache( cachePath,
                                      stamp,
                                      options.visBakeRays,
                                      rayDistance,
                                      radianceDistance,
                                      options,
                                      records,
                                      bake ) )
            {
                OSG_WARN << "Sponza visibility/radiance bake could not write cache "
                         << result.cachePath << std::endl;
            }
        }
        else
        {
            OSG_WARN << "Sponza visibility bake could not stat model file "
                     << modelPath.string() << "; cache disabled" << std::endl;
        }

        const osg::Timer_t endTick = osg::Timer::instance()->tick();
        result.wallTimeSeconds = osg::Timer::instance()->delta_s( startTick, endTick );
        OSG_NOTICE << "Sponza visibility/radiance bake processed " << vertexCount
                   << " vertices, " << result.occluderTriangles
                   << " occluder triangles, " << options.visBakeRays
                   << " rays/vertex, visibility length " << rayDistance
                   << ", radiance length " << radianceDistance << " in "
                   << result.wallTimeSeconds << " s" << std::endl;
        return result;
    }

}
