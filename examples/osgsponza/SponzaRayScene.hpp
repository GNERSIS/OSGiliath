/* OSGiliath -- OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <osg/geometry/Array.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/vec3.hpp>
#include <string>
#include <vector>

namespace osg
{

    class Drawable;
    class Geometry;
    class Node;
    class PrimitiveSet;
    class StateSet;

}

namespace sponza
{

    inline constexpr std::uint32_t invalidRaySceneRecordIndex =
        std::numeric_limits<std::uint32_t>::max();

    struct GeometryRecord
    {
            osg::Geometry*         geometry = nullptr;
            const osg::Vec3Array*  vertices = nullptr;
            const osg::Vec3Array*  normals  = nullptr;
            const osg::StateSet*   stateSet = nullptr;
            osg::dmat4             localToWorld;
            osg::dmat4             normalMatrix;
            std::string            name;
            std::vector<osg::vec3> worldPositions;
            std::vector<osg::vec3> worldNormals;
            std::uint32_t          materialIndex    = 0U;
            std::size_t            vertexCount      = 0U;
            bool                   hasNormals       = false;
            bool                   excludedOccluder = false;
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
            std::uint32_t recordIndex   = invalidRaySceneRecordIndex;
            std::uint32_t i0            = 0U;
            std::uint32_t i1            = 0U;
            std::uint32_t i2            = 0U;
    };

    struct BvhNode
    {
            osg::box      bounds;
            std::uint32_t left  = 0U;
            std::uint32_t right = 0U;
            std::uint32_t start = 0U;
            std::uint32_t count = 0U;
    };

    struct RayScene
    {
            std::vector<GeometryRecord> geometryRecords;
            std::vector<Triangle>       triangles;
            std::vector<std::uint32_t>  triangleIndices;
            std::vector<BvhNode>        nodes;
            std::size_t                 materialCount = 0U;
    };

    osg::vec3
    transformPoint( const osg::dmat4& matrix,
                    const osg::vec3&  point );

    osg::vec3
    transformNormal( const osg::dmat4& normalMatrix,
                     const osg::vec3&  normal );

    bool
    isExcludedOccluderName( const std::string& name );

    bool
    isExcludedOccluder( const osg::Drawable& drawable );

    std::vector<GeometryRecord>
    collectGeometryRecords( osg::Node& model );

    void
    appendPrimitiveTriangles( std::vector<Triangle>&   triangles,
                              const osg::Vec3Array&    vertices,
                              const osg::dmat4&        localToWorld,
                              std::uint32_t            materialIndex,
                              const osg::PrimitiveSet& primitiveSet,
                              std::uint32_t            recordIndex );

    std::vector<Triangle>
    collectOccluderTriangles( const std::vector<GeometryRecord>& records );

    void
    buildBvh( RayScene& scene );

    RayScene
    buildRayScene( osg::Node& model );

}
