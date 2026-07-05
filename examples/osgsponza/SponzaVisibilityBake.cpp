/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
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
#include <numeric>
#include <osg/core/Notify.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Timer.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/GL>
#include <osg/maths/box.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/transform.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/fstream.hpp>
#include <thread>
#include <vector>

namespace
{

    constexpr unsigned int         visibilityAttribLocation = 7U;
    constexpr std::array<char, 8U> cacheMagic{ 'O', 'S', 'G', 'V', 'I', 'S', 'B', 'K' };
    constexpr std::uint32_t        cacheVersion     = 2U;
    constexpr std::size_t          bvhLeafSize      = 8U;
    constexpr float                rayOriginOffset  = 0.02F;
    constexpr float                rayHitEpsilon    = 1.0E-4F;
    constexpr float                minNormalLength2 = 1.0E-10F;
    constexpr float                pi               = 3.14159265358979323846F;

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
            osg::vec3 v0;
            osg::vec3 v1;
            osg::vec3 v2;
            osg::vec3 centroid;
            osg::box  bounds;
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
                    unsigned int           i0,
                    unsigned int           i1,
                    unsigned int           i2 )
    {
        const unsigned int vertexCount = vertices.getNumElements();
        if( i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount )
        {
            return;
        }

        const osg::vec3 v0 = transformPoint( localToWorld, vertices[i0] );
        const osg::vec3 v1 = transformPoint( localToWorld, vertices[i1] );
        const osg::vec3 v2 = transformPoint( localToWorld, vertices[i2] );
        if( osg::length2( osg::cross( v1 - v0, v2 - v0 ) ) <= 1.0E-12F )
        {
            return;
        }

        Triangle triangle;
        triangle.v0       = v0;
        triangle.v1       = v1;
        triangle.v2       = v2;
        triangle.centroid = ( v0 + v1 + v2 ) * ( 1.0F / 3.0F );
        triangle.bounds   = triangleBounds( v0, v1, v2 );
        triangles.push_back( triangle );
    }

    void
    appendPrimitiveTriangles( std::vector<Triangle>&   triangles,
                              const osg::Vec3Array&    vertices,
                              const osg::dmat4&        localToWorld,
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
                                        primitiveSet.index( i ),
                                        primitiveSet.index( i + 1U ),
                                        primitiveSet.index( i + 2U ) );
                    }
                    else
                    {
                        appendTriangle( triangles,
                                        vertices,
                                        localToWorld,
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
                    appendTriangle( triangles, vertices, localToWorld, i0, i1, i2 );
                    appendTriangle( triangles, vertices, localToWorld, i0, i2, i3 );
                }
                break;

            default :
                break;
        }
    }

    class GeometryRecordVisitor : public osg::NodeVisitor
    {
        public:

            GeometryRecordVisitor() :
                osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
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
    };

    class OccluderTriangleVisitor : public osg::NodeVisitor
    {
        public:

            OccluderTriangleVisitor() :
                osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
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
                                                  *primitiveSet );
                    }
                }
            }

            std::vector<Triangle> triangles;

        private:

            std::vector<osg::dmat4> _worldStack;
    };

    std::vector<GeometryRecord>
    collectGeometryRecords( osg::Node& model )
    {
        GeometryRecordVisitor visitor;
        model.accept( visitor );
        return visitor.records;
    }

    std::vector<Triangle>
    collectOccluderTriangles( osg::Node& model )
    {
        OccluderTriangleVisitor visitor;
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
    rayIntersectsTriangle( const osg::vec3& origin,
                           const osg::vec3& direction,
                           float            maxDistance,
                           const Triangle&  triangle )
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
        return t > rayHitEpsilon && t <= maxDistance;
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

    float
    computeVertexVisibility( const GeometryRecord&         record,
                             std::size_t                   vertexIndex,
                             const Bvh&                    bvh,
                             const std::vector<osg::vec3>& samples,
                             float                         maxDistance )
    {
        if( record.vertices ==
            nullptr ||
            record.normals ==
            nullptr ||
            vertexIndex >=
            record.vertexCount ||
            vertexIndex >= static_cast<std::size_t>( record.normals->getNumElements() ) )
        {
            return 1.0F;
        }

        const osg::vec3 transformedNormal = transformNormal(
            record.normalMatrix,
            ( *record.normals )[static_cast<unsigned int>( vertexIndex )]
        );
        if( osg::length2( transformedNormal ) <= minNormalLength2 )
        {
            return 1.0F;
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
        for( const osg::vec3& sample : samples )
        {
            const osg::vec3 direction = safeNormalize(
                tangent * sample.x + bitangent * sample.y + normal * sample.z,
                normal
            );
            if( !bvhAnyHit( bvh, origin, direction, maxDistance ) )
            {
                ++unoccluded;
            }
        }

        return static_cast<float>( unoccluded ) / static_cast<float>( samples.size() );
    }

    std::vector<std::vector<float>>
    bakeVisibility( const std::vector<GeometryRecord>& records,
                    const Bvh&                         bvh,
                    int                                rayCount,
                    float                              maxDistance )
    {
        std::vector<std::vector<float>> visibility( records.size() );
        std::vector<std::size_t>        offsets;
        offsets.reserve( records.size() + 1U );
        offsets.push_back( 0U );

        std::size_t totalVertices = 0U;
        for( std::size_t i = 0U; i < records.size(); ++i )
        {
            visibility[i].assign( records[i].vertexCount, 1.0F );
            totalVertices += records[i].vertexCount;
            offsets.push_back( totalVertices );
        }

        if( totalVertices == 0U || bvh.triangles.empty() )
        {
            return visibility;
        }

        const std::vector<osg::vec3> samples = makeHemisphereSamples( rayCount );
        std::atomic<std::size_t>     nextVertex{ 0U };
        const std::size_t            chunkSize = 64U;
        const unsigned int hardwareThreads     = std::thread::hardware_concurrency();
        std::size_t        threadCount =
            std::max<std::size_t>( 1U, static_cast<std::size_t>( hardwareThreads ) );
        threadCount = std::min( threadCount, totalVertices );

        auto worker = [&records,
                       &visibility,
                       &offsets,
                       &nextVertex,
                       &samples,
                       &bvh,
                       maxDistance,
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
                        visibility[recordIndex][vertexIndex] =
                            computeVertexVisibility( records[recordIndex],
                                                     vertexIndex,
                                                     bvh,
                                                     samples,
                                                     maxDistance );
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

        return visibility;
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

    bool
    loadVisibilityCache( const std::filesystem::path&       cachePath,
                         const ModelStamp&                  stamp,
                         int                                rayCount,
                         float                              rayDistance,
                         const std::vector<GeometryRecord>& records,
                         std::vector<std::vector<float>>&   visibility )
    {
        osgDB::ifstream stream( cachePath.string().c_str(),
                                std::ios::in | std::ios::binary );
        if( !stream )
        {
            return false;
        }

        std::array<char, 8U> magic{};
        std::uint32_t        version        = 0U;
        std::uint32_t        cachedRayCount = 0U;
        float                cachedDistance = 0.0F;
        std::uint64_t        cachedSize     = 0U;
        std::int64_t         cachedMtime    = 0;
        std::uint64_t        geometryCount  = 0U;
        if( !readValue( stream, magic.data(), magic.size() ) ||
            !readPod( stream, version ) ||
            !readPod( stream, cachedRayCount ) ||
            !readPod( stream, cachedDistance ) ||
            !readPod( stream, cachedSize ) ||
            !readPod( stream, cachedMtime ) ||
            !readPod( stream, geometryCount ) )
        {
            return false;
        }

        if( magic !=
            cacheMagic ||
            version !=
            cacheVersion ||
            cachedRayCount !=
            static_cast<std::uint32_t>( rayCount ) ||
            cachedDistance !=
            rayDistance ||
            cachedSize !=
            stamp.size ||
            cachedMtime !=
            stamp.mtimeTicks ||
            geometryCount != static_cast<std::uint64_t>( records.size() ) )
        {
            return false;
        }

        std::vector<std::uint64_t> vertexCounts( records.size(), 0U );
        for( std::size_t i = 0U; i < records.size(); ++i )
        {
            if( !readPod( stream, vertexCounts[i] ) ||
                vertexCounts[i] != static_cast<std::uint64_t>( records[i].vertexCount ) )
            {
                return false;
            }
        }

        visibility.resize( records.size() );
        for( std::size_t i = 0U; i < records.size(); ++i )
        {
            visibility[i].resize( records[i].vertexCount, 1.0F );
            if( !visibility[i].empty() &&
                !readValue( stream,
                            visibility[i].data(),
                            visibility[i].size() * sizeof( float ) ) )
            {
                return false;
            }
        }

        return true;
    }

    bool
    saveVisibilityCache( const std::filesystem::path&           cachePath,
                         const ModelStamp&                      stamp,
                         int                                    rayCount,
                         float                                  rayDistance,
                         const std::vector<GeometryRecord>&     records,
                         const std::vector<std::vector<float>>& visibility )
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

        for( const std::vector<float>& values : visibility )
        {
            if( !values.empty() &&
                !writeValue( stream, values.data(), values.size() * sizeof( float ) ) )
            {
                return false;
            }
        }

        return true;
    }

    void
    applyVisibilityAttributes( const std::vector<GeometryRecord>&     records,
                               const std::vector<std::vector<float>>& visibility )
    {
        bool warnedExistingAttribute = false;
        for( std::size_t recordIndex = 0U; recordIndex < records.size(); ++recordIndex )
        {
            osg::Geometry* geometry = records[recordIndex].geometry;
            if( geometry == nullptr || records[recordIndex].vertexCount == 0U )
            {
                continue;
            }

            if( geometry->getVertexAttribArray( visibilityAttribLocation ) !=
                nullptr &&
                !warnedExistingAttribute )
            {
                OSG_WARN << "Sponza visibility bake replacing existing vertex "
                         << "attribute " << visibilityAttribLocation << std::endl;
                warnedExistingAttribute = true;
            }

            osg::ref_ptr<osg::FloatArray> array = new osg::FloatArray(
                static_cast<unsigned int>( records[recordIndex].vertexCount )
            );
            for( std::size_t i = 0U; i < records[recordIndex].vertexCount; ++i )
            {
                ( *array )[static_cast<unsigned int>( i )] = visibility[recordIndex][i];
            }
            array->setNormalize( false );
            geometry->setVertexAttribArray( visibilityAttribLocation,
                                            array.get(),
                                            osg::Array::BIND_PER_VERTEX );
        }
    }

    void
    setVisibilityUniforms( osg::Node& model,
                           bool       hasVisibility,
                           float      strength,
                           float      power )
    {
        osg::StateSet* stateSet = model.getOrCreateStateSet();
        stateSet->addUniform( new osg::Uniform( "uHasVisBake", hasVisibility ),
                              osg::StateAttribute::OVERRIDE );
        stateSet->addUniform( new osg::Uniform( "uVisStrength", strength ),
                              osg::StateAttribute::OVERRIDE );
        stateSet->addUniform( new osg::Uniform( "uVisPower", power ),
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
                               options.visBakeStrength,
                               options.visBakePower );

        if( !options.visBakeEnabled )
        {
            OSG_NOTICE << "Sponza visibility bake disabled" << std::endl;
            return result;
        }

        const osg::Timer_t                startTick = osg::Timer::instance()->tick();
        const std::vector<GeometryRecord> records   = collectGeometryRecords( *model );
        const std::size_t                 vertexCount =
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
        std::vector<std::vector<float>> visibility;
        if( hasStamp &&
            !options.visBakeRefresh &&
            loadVisibilityCache( cachePath,
                                 stamp,
                                 options.visBakeRays,
                                 rayDistance,
                                 records,
                                 visibility ) )
        {
            applyVisibilityAttributes( records, visibility );
            setVisibilityUniforms( *model,
                                   true,
                                   options.visBakeStrength,
                                   options.visBakePower );
            const osg::Timer_t endTick = osg::Timer::instance()->tick();
            result.loadedFromCache     = true;
            result.wallTimeSeconds =
                osg::Timer::instance()->delta_s( startTick, endTick );
            OSG_NOTICE << "Sponza visibility bake loaded " << vertexCount
                       << " vertices from " << result.cachePath << " in "
                       << result.wallTimeSeconds << " s" << std::endl;
            return result;
        }

        std::vector<Triangle> triangles = collectOccluderTriangles( *model );
        result.occluderTriangles        = triangles.size();
        Bvh bvh                         = buildBvh( std::move( triangles ) );

        visibility = bakeVisibility( records, bvh, options.visBakeRays, rayDistance );
        applyVisibilityAttributes( records, visibility );
        setVisibilityUniforms( *model,
                               true,
                               options.visBakeStrength,
                               options.visBakePower );

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
                                      records,
                                      visibility ) )
            {
                OSG_WARN << "Sponza visibility bake could not write cache "
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
        OSG_NOTICE << "Sponza visibility bake processed " << vertexCount << " vertices, "
                   << result.occluderTriangles << " occluder triangles, "
                   << options.visBakeRays << " rays/vertex in " << result.wallTimeSeconds
                   << " s" << std::endl;
        return result;
    }

}
