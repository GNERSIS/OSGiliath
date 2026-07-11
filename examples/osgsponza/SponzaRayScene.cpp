/* OSGiliath -- OpenSceneGraph fork. See LICENSE.txt */
#include "SponzaRayScene.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <osg/geometry/Drawable.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/GL>
#include <osg/maths/transform.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <utility>

namespace
{

    constexpr std::size_t bvhLeafSize      = 8U;
    constexpr float       minNormalLength2 = 1.0E-10F;

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

    osg::dmat4
    makeNormalMatrix( const osg::dmat4& localToWorld )
    {
        return osg::inverse( localToWorld );
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

    float
    triangleLongestEdge( const osg::vec3& v0,
                         const osg::vec3& v1,
                         const osg::vec3& v2 )
    {
        return std::max( osg::length( v1 - v0 ),
                         std::max( osg::length( v2 - v1 ), osg::length( v0 - v2 ) ) );
    }

    std::size_t
    primitiveTriangleReserveCount( const osg::PrimitiveSet& primitiveSet )
    {
        const std::size_t indexCount =
            static_cast<std::size_t>( primitiveSet.getNumIndices() );
        switch( primitiveSet.getMode() )
        {
            case GL_TRIANGLES :
                return indexCount / 3U;

            case GL_TRIANGLE_STRIP :
            case GL_TRIANGLE_FAN :
                return indexCount >= 3U ? indexCount - 2U : 0U;

            case GL_QUADS :
                return ( indexCount / 4U ) * 2U;

            default :
                return 0U;
        }
    }

    void
    appendTriangle( std::vector<sponza::Triangle>& triangles,
                    const osg::Vec3Array&          vertices,
                    const osg::dmat4&              localToWorld,
                    std::uint32_t                  materialIndex,
                    std::uint32_t                  recordIndex,
                    unsigned int                   i0,
                    unsigned int                   i1,
                    unsigned int                   i2 )
    {
        const unsigned int vertexCount = vertices.getNumElements();
        if( i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount )
        {
            return;
        }

        const osg::vec3 v0 = sponza::transformPoint( localToWorld, vertices[i0] );
        const osg::vec3 v1 = sponza::transformPoint( localToWorld, vertices[i1] );
        const osg::vec3 v2 = sponza::transformPoint( localToWorld, vertices[i2] );
        const osg::vec3 faceNormal = osg::cross( v1 - v0, v2 - v0 );
        if( osg::length2( faceNormal ) <= 1.0E-12F )
        {
            return;
        }

        sponza::Triangle triangle;
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

                sponza::GeometryRecord record;
                record.geometry     = geometry;
                record.vertices     = vertices;
                record.normals      = normals;
                record.stateSet     = drawable.getStateSet();
                record.localToWorld = _worldStack.back();
                record.normalMatrix = makeNormalMatrix( record.localToWorld );
                record.name         = drawable.getName();
                if( record.name.empty() && record.stateSet != nullptr )
                {
                    record.name = record.stateSet->getName();
                }
                record.vertexCount      = vertexCount;
                record.hasNormals       = hasNormals;
                record.materialIndex    = materialIndex( record.stateSet );
                record.excludedOccluder = sponza::isExcludedOccluder( drawable );

                if( vertices != nullptr )
                {
                    record.worldPositions.reserve( vertexCount );
                    for( std::size_t i = 0U; i < vertexCount; ++i )
                    {
                        const unsigned int vertexIndex = static_cast<unsigned int>( i );
                        record.worldPositions.push_back(
                            sponza::transformPoint( record.localToWorld,
                                                    ( *vertices )[vertexIndex] )
                        );
                    }
                }

                if( hasNormals )
                {
                    record.worldNormals.reserve( vertexCount );
                    for( std::size_t i = 0U; i < vertexCount; ++i )
                    {
                        const unsigned int normalIndex = static_cast<unsigned int>( i );
                        record.worldNormals.push_back( safeNormalize(
                            sponza::transformNormal( record.normalMatrix,
                                                     ( *normals )[normalIndex] ),
                            osg::vec3( 0.0F, 1.0F, 0.0F )
                        ) );
                    }
                }

                records.push_back( std::move( record ) );
            }

            std::vector<sponza::GeometryRecord> records;

        private:

            std::uint32_t
            materialIndex( const osg::StateSet* stateSet )
            {
                if( stateSet == nullptr )
                {
                    return 0U;
                }

                const auto found = _materialIndices.find( stateSet );
                if( found != _materialIndices.end() )
                {
                    return found->second;
                }

                const std::uint32_t next =
                    static_cast<std::uint32_t>( _materialIndices.size() + 1U );
                _materialIndices.emplace( stateSet, next );
                return next;
            }

            std::vector<osg::dmat4>                       _worldStack;
            std::map<const osg::StateSet*, std::uint32_t> _materialIndices;
    };

    std::uint32_t
    buildBvhNode( sponza::RayScene& scene,
                  std::size_t       start,
                  std::size_t       end )
    {
        const std::size_t nodeIndex = scene.nodes.size();
        scene.nodes.push_back( sponza::BvhNode{} );

        osg::box bounds;
        osg::box centroidBounds;
        for( std::size_t i = start; i < end; ++i )
        {
            const sponza::Triangle& triangle =
                scene.triangles[static_cast<std::size_t>( scene.triangleIndices[i] )];
            bounds.add( triangle.bounds );
            centroidBounds.expandBy( triangle.centroid );
        }

        scene.nodes[nodeIndex].bounds = bounds;

        const std::size_t count       = end - start;
        if( count <= bvhLeafSize )
        {
            scene.nodes[nodeIndex].start = static_cast<std::uint32_t>( start );
            scene.nodes[nodeIndex].count = static_cast<std::uint32_t>( count );
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
            scene.triangleIndices.begin() + static_cast<std::ptrdiff_t>( start ),
            scene.triangleIndices.begin() + static_cast<std::ptrdiff_t>( mid ),
            scene.triangleIndices.begin() + static_cast<std::ptrdiff_t>( end ),
            [&scene, axis]( std::uint32_t lhs, std::uint32_t rhs )
            {
                return axisValue(
                           scene.triangles[static_cast<std::size_t>( lhs )].centroid,
                           axis
                       ) <
                       axisValue(
                           scene.triangles[static_cast<std::size_t>( rhs )].centroid,
                           axis
                       );
            }
        );

        const std::uint32_t left     = buildBvhNode( scene, start, mid );
        const std::uint32_t right    = buildBvhNode( scene, mid, end );
        scene.nodes[nodeIndex].left  = left;
        scene.nodes[nodeIndex].right = right;
        return static_cast<std::uint32_t>( nodeIndex );
    }

}

namespace sponza
{

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

    std::vector<GeometryRecord>
    collectGeometryRecords( osg::Node& model )
    {
        GeometryRecordVisitor visitor;
        model.accept( visitor );
        return visitor.records;
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

    std::vector<Triangle>
    collectOccluderTriangles( const std::vector<GeometryRecord>& records )
    {
        std::vector<Triangle> triangles;
        std::size_t           triangleCapacity = 0U;
        for( const GeometryRecord& record : records )
        {
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
                    triangleCapacity += primitiveTriangleReserveCount( *primitiveSet );
                }
            }
        }
        triangles.reserve( triangleCapacity );

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

    void
    buildBvh( RayScene& scene )
    {
        scene.triangleIndices.reserve( scene.triangles.size() );
        scene.triangleIndices.resize( scene.triangles.size() );
        std::iota( scene.triangleIndices.begin(),
                   scene.triangleIndices.end(),
                   std::uint32_t{ 0U } );
        scene.nodes.clear();
        scene.nodes.reserve( scene.triangles.size() );
        if( !scene.triangles.empty() )
        {
            buildBvhNode( scene, 0U, scene.triangles.size() );
        }
    }

    RayScene
    buildRayScene( osg::Node& model )
    {
        RayScene scene;
        scene.geometryRecords = collectGeometryRecords( model );
        for( const GeometryRecord& record : scene.geometryRecords )
        {
            scene.materialCount =
                std::max( scene.materialCount,
                          static_cast<std::size_t>( record.materialIndex ) + 1U );
        }
        scene.triangles = collectOccluderTriangles( scene.geometryRecords );
        buildBvh( scene );
        return scene;
    }

}
