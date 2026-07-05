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
#include <osg/nodes/Node.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/fstream.hpp>
#include <thread>
#include <vector>

namespace
{

    constexpr unsigned int         visibilityAttribLocation = 7U;
    constexpr unsigned int         radianceAttribLocation   = 1U;
    constexpr std::array<char, 8U> cacheMagic{ 'O', 'S', 'G', 'V', 'I', 'S', 'B', 'K' };
    // v4 stores the v3 vec4 per vertex plus shader-ready diffuse irradiance/pi.
    constexpr std::uint32_t        cacheVersion        = 4U;
    constexpr std::size_t          bvhLeafSize         = 8U;
    constexpr float                rayOriginOffset     = 0.02F;
    constexpr float                rayHitEpsilon       = 1.0E-4F;
    constexpr float                minNormalLength2    = 1.0E-10F;
    constexpr float                pi                  = 3.14159265358979323846F;
    constexpr double               twoPi               = 6.28318530717958647692;
    constexpr double               sunDiscLuminance    = 500.0;
    constexpr int                  albedoTargetSamples = 4'096;

    struct GeometryRecord
    {
            osg::Geometry*        geometry = nullptr;
            const osg::Vec3Array* vertices = nullptr;
            const osg::Vec3Array* normals  = nullptr;
            osg::dmat4            localToWorld;
            osg::dmat4            normalMatrix;
            std::size_t           vertexCount = 0U;
            bool                  hasNormals  = false;
    };

    struct Triangle
    {
            osg::vec3     v0;
            osg::vec3     v1;
            osg::vec3     v2;
            osg::vec3     normal;
            osg::vec3     centroid;
            osg::box      bounds;
            std::uint32_t materialIndex = 0U;
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

    struct VertexBake
    {
            osg::vec4 visibility{ 0.0F, 1.0F, 0.0F, 1.0F };
            osg::vec3 radiance{ 0.0F, 0.0F, 0.0F };
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
                           << ")" << std::endl;
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

    void
    appendTriangle( std::vector<Triangle>& triangles,
                    const osg::Vec3Array&  vertices,
                    const osg::dmat4&      localToWorld,
                    std::uint32_t          materialIndex,
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
        triangle.materialIndex = materialIndex;
        triangles.push_back( triangle );
    }

    void
    appendPrimitiveTriangles( std::vector<Triangle>&   triangles,
                              const osg::Vec3Array&    vertices,
                              const osg::dmat4&        localToWorld,
                              std::uint32_t            materialIndex,
                              const osg::PrimitiveSet& primitiveSet )
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
                                    i0,
                                    i1,
                                    i2 );
                    appendTriangle( triangles,
                                    vertices,
                                    localToWorld,
                                    materialIndex,
                                    i0,
                                    i2,
                                    i3 );
                }
                break;

            default :
                break;
        }
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
                record.vertexCount  = vertexCount;
                record.hasNormals   = hasNormals;
                records.push_back( record );
            }

            std::vector<GeometryRecord> records;

        private:

            std::vector<osg::dmat4> _worldStack;
            MaterialTable&          _materialTable;
    };

    class OccluderTriangleVisitor : public osg::NodeVisitor
    {
        public:

            explicit OccluderTriangleVisitor( MaterialTable& materialTable ) :
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
                if( geometry == nullptr || isExcludedOccluder( drawable ) )
                {
                    return;
                }

                const osg::Vec3Array* vertices =
                    dynamic_cast<const osg::Vec3Array*>( geometry->getVertexArray() );
                if( vertices == nullptr )
                {
                    return;
                }

                const std::uint32_t materialIndex =
                    _materialTable.materialIndex( drawable.getStateSet() );
                for( unsigned int primitiveIndex = 0U;
                     primitiveIndex < geometry->getNumPrimitiveSets();
                     ++primitiveIndex )
                {
                    const osg::PrimitiveSet* primitiveSet =
                        geometry->getPrimitiveSet( primitiveIndex );
                    if( primitiveSet )
                    {
                        appendPrimitiveTriangles( triangles,
                                                  *vertices,
                                                  _worldStack.back(),
                                                  materialIndex,
                                                  *primitiveSet );
                    }
                }
            }

            std::vector<Triangle> triangles;

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
    collectOccluderTriangles( osg::Node&     model,
                              MaterialTable& materialTable )
    {
        OccluderTriangleVisitor visitor( materialTable );
        model.accept( visitor );
        return visitor.triangles;
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

    VertexBake
    computeVertexBake( const GeometryRecord&         record,
                       std::size_t                   vertexIndex,
                       const Bvh&                    bvh,
                       const MaterialTable&          materialTable,
                       const EnvironmentSampler&     environment,
                       const std::vector<osg::vec3>& samples,
                       const osg::vec3&              sunDirection,
                       const osg::vec3&              sunRadiance,
                       float                         visibilityMaxDistance,
                       float                         radianceMaxDistance )
    {
        VertexBake result;
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

        osg::vec3 tangent;
        osg::vec3 bitangent;
        makeBasis( normal, tangent, bitangent );

        const osg::vec3 position = transformPoint(
            record.localToWorld,
            ( *record.vertices )[static_cast<unsigned int>( vertexIndex )]
        );
        const osg::vec3 origin     = position + normal * rayOriginOffset;

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
            const float nDotSun = osg::dot( hitNormal, sunDirection );
            if( nDotSun <= 0.0F )
            {
                continue;
            }

            const osg::vec3 shadowOrigin = hitPoint + hitNormal * rayOriginOffset;
            if( bvhAnyHit( bvh, shadowOrigin, sunDirection, radianceMaxDistance ) )
            {
                continue;
            }

            const osg::vec3 albedo = materialTable.albedo( triangle.materialIndex );
            radianceSum += multiplyComponents( sunRadiance, albedo ) * ( nDotSun / pi );
        }

        const float visibility =
            static_cast<float>( unoccluded ) / static_cast<float>( samples.size() );
        const osg::vec3 bentNormal = visibility <=
                                             1.0E-4F ||
                                             osg::length2( unoccludedDirectionSum ) <=
                                             minNormalLength2
                                       ? normal
                                       : safeNormalize( unoccludedDirectionSum, normal );
        result.visibility          = osg::vec4( bentNormal, visibility );
        // The shader multiplies this value by albedo directly, matching the existing
        // SH path where coefficients are irradiance divided by PI. With cosine-weighted
        // samples (pdf = cos(theta) / PI), E / PI is the mean incoming radiance.
        result.radiance = radianceSum * ( 1.0F / static_cast<float>( samples.size() ) );
        return result;
    }

    struct BakeArrays
    {
            std::vector<std::vector<osg::vec4>> visibility;
            std::vector<std::vector<osg::vec3>> radiance;
    };

    BakeArrays
    bakeVisibilityAndRadiance( const std::vector<GeometryRecord>& records,
                               const Bvh&                         bvh,
                               const MaterialTable&               materialTable,
                               const osg::Image*                  envImage,
                               const sponza::SponzaOptions&       options,
                               int                                rayCount,
                               float                              visibilityMaxDistance,
                               float                              radianceMaxDistance )
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

        const std::vector<osg::vec3> samples = makeHemisphereSamples( rayCount );
        const EnvironmentSampler     environment( envImage, options.envRotation );
        const osg::dvec3 sunDirectionD = sponza::computeSunDirectionWorld( options );
        const osg::vec3  sunDirection =
            safeNormalize( osg::vec3( static_cast<float>( sunDirectionD.x ),
                                      static_cast<float>( sunDirectionD.y ),
                                      static_cast<float>( sunDirectionD.z ) ),
                           osg::vec3( 0.0F, 1.0F, 0.0F ) );
        const osg::vec3 sunRadiance =
            sponza::scaledColor( options.sunColor, options.sunIntensity );
        std::atomic<std::size_t> nextVertex{ 0U };
        const std::size_t        chunkSize       = 64U;
        const unsigned int       hardwareThreads = std::thread::hardware_concurrency();
        std::size_t              threadCount =
            std::max<std::size_t>( 1U, static_cast<std::size_t>( hardwareThreads ) );
        threadCount = std::min( threadCount, totalVertices );

        auto worker = [&records,
                       &bake,
                       &offsets,
                       &nextVertex,
                       &samples,
                       &bvh,
                       &materialTable,
                       &environment,
                       sunDirection,
                       sunRadiance,
                       visibilityMaxDistance,
                       radianceMaxDistance,
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
                        const VertexBake vertexBake =
                            computeVertexBake( records[recordIndex],
                                               vertexIndex,
                                               bvh,
                                               materialTable,
                                               environment,
                                               samples,
                                               sunDirection,
                                               sunRadiance,
                                               visibilityMaxDistance,
                                               radianceMaxDistance );
                        bake.visibility[recordIndex][vertexIndex] =
                            vertexBake.visibility;
                        bake.radiance[recordIndex][vertexIndex] = vertexBake.radiance;
                    }
                }
            }
        };

        std::vector<std::thread> threads;
        threads.reserve( threadCount );
        for( std::size_t i = 0U; i < threadCount; ++i )
        {
            threads.emplace_back( worker );
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
    cachePathForModel( const std::filesystem::path& modelPath )
    {
        std::filesystem::path cachePath  = modelPath;
        cachePath                       += ".visbake";
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
               cachedRadianceDistance != radianceDistance;
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
        float         cachedEnvRotation = 0.0F;
        std::uint64_t cachedSize        = 0U;
        std::int64_t  cachedMtime       = 0;
        std::uint64_t geometryCount     = 0U;
        if( !readPod( stream, cachedRayCount ) ||
            !readPod( stream, cachedDistance ) ||
            !readPod( stream, cachedRadianceDistance ) ||
            !readPod( stream, cachedSunAzimuth ) ||
            !readPod( stream, cachedSunElevation ) ||
            !readPod( stream, cachedSunIntensity ) ||
            !readPod( stream, cachedSunColor ) ||
            !readPod( stream, cachedEnvRotation ) ||
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
    applyVisibilityBake( osg::Node*           model,
                         const SponzaOptions& options )
    {
        VisibilityBakeResult result;
        if( model == nullptr )
        {
            return result;
        }

        setVisibilityUniforms( *model,
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

        const osg::Timer_t                startTick = osg::Timer::instance()->tick();
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

        result.enabled                        = true;
        result.geometryCount                  = records.size();
        result.vertexCount                    = vertexCount;

        const std::filesystem::path modelPath = resolveModelPath( options.modelPath );
        const std::filesystem::path cachePath = cachePathForModel( modelPath );
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

        std::vector<Triangle> triangles =
            collectOccluderTriangles( *model, materialTable );
        result.occluderTriangles          = triangles.size();
        Bvh                      bvh      = buildBvh( std::move( triangles ) );

        osg::ref_ptr<osg::Image> envImage = sponza::loadEnvironmentImage();
        if( !envImage )
        {
            OSG_WARN << "Sponza radiance bake could not load environment HDR; "
                     << "escaped rays contribute black" << std::endl;
        }
        bake = bakeVisibilityAndRadiance( records,
                                          bvh,
                                          materialTable,
                                          envImage.get(),
                                          options,
                                          options.visBakeRays,
                                          rayDistance,
                                          radianceDistance );
        applyBakeAttributes( records, bake );
        setVisibilityUniforms( *model,
                               true,
                               options.radianceBakeEnabled,
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
