
# FIXME: For OS X, need flag for Framework or dylib
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGUTIL_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgUtil)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    # top-level
    ${HEADER_PATH}/Export.hpp

    # culling
    ${HEADER_PATH}/culling/CullVisitor.hpp
    ${HEADER_PATH}/culling/PositionalStateContainer.hpp
    ${HEADER_PATH}/culling/RenderBin.hpp
    ${HEADER_PATH}/culling/RenderLeaf.hpp
    ${HEADER_PATH}/culling/RenderStage.hpp
    ${HEADER_PATH}/culling/SceneView.hpp
    ${HEADER_PATH}/culling/StateGraph.hpp
    ${HEADER_PATH}/culling/UpdateVisitor.hpp

    # intersection
    ${HEADER_PATH}/intersection/IntersectionVisitor.hpp
    ${HEADER_PATH}/intersection/LineSegmentIntersector.hpp
    ${HEADER_PATH}/intersection/PlaneIntersector.hpp
    ${HEADER_PATH}/intersection/PolytopeIntersector.hpp
    ${HEADER_PATH}/intersection/RayIntersector.hpp

    # optimization
    ${HEADER_PATH}/optimization/DrawElementTypeSimplifier.hpp
    ${HEADER_PATH}/optimization/EdgeCollector.hpp
    ${HEADER_PATH}/optimization/MeshOptimizers.hpp
    ${HEADER_PATH}/optimization/Optimizer.hpp
    ${HEADER_PATH}/optimization/Simplifier.hpp
    ${HEADER_PATH}/optimization/SmoothingVisitor.hpp
    ${HEADER_PATH}/optimization/TangentSpaceGenerator.hpp

    # mesh
    ${HEADER_PATH}/mesh/DelaunayTriangulator.hpp
    ${HEADER_PATH}/mesh/ReversePrimitiveFunctor.hpp
    ${HEADER_PATH}/mesh/SceneGraphBuilder.hpp
    ${HEADER_PATH}/mesh/Tessellator.hpp
    ${HEADER_PATH}/mesh/TransformAttributeFunctor.hpp

    # utils
    ${HEADER_PATH}/utils/ConvertVec.hpp
    ${HEADER_PATH}/utils/CubeMapGenerator.hpp
    ${HEADER_PATH}/utils/DisplayRequirementsVisitor.hpp
    ${HEADER_PATH}/utils/GLObjectsVisitor.hpp
    ${HEADER_PATH}/utils/HalfWayMapGenerator.hpp
    ${HEADER_PATH}/utils/HighlightMapGenerator.hpp
    ${HEADER_PATH}/utils/IncrementalCompileOperation.hpp
    ${HEADER_PATH}/utils/OperationArrayFunctor.hpp
    ${HEADER_PATH}/utils/PerlinNoise.hpp
    ${HEADER_PATH}/utils/PrintVisitor.hpp
    ${HEADER_PATH}/utils/ReflectionMapGenerator.hpp
    ${HEADER_PATH}/utils/ShaderGen.hpp
    ${HEADER_PATH}/utils/Statistics.hpp
    ${HEADER_PATH}/utils/TransformCallback.hpp
    ${HEADER_PATH}/utils/Version.hpp
)

SET(TARGET_SRC
    # culling
    culling/CullVisitor.cpp
    culling/PositionalStateContainer.cpp
    culling/RenderBin.cpp
    culling/RenderLeaf.cpp
    culling/RenderStage.cpp
    culling/SceneView.cpp
    culling/StateGraph.cpp
    culling/UpdateVisitor.cpp

    # intersection
    intersection/IntersectionVisitor.cpp
    intersection/LineSegmentIntersector.cpp
    intersection/PlaneIntersector.cpp
    intersection/PolytopeIntersector.cpp
    intersection/RayIntersector.cpp

    # optimization
    optimization/DrawElementTypeSimplifier.cpp
    optimization/EdgeCollector.cpp
    optimization/MeshOptimizers.cpp
    optimization/Optimizer.cpp
    optimization/Simplifier.cpp
    optimization/SmoothingVisitor.cpp
    optimization/TangentSpaceGenerator.cpp

    # mesh
    mesh/DelaunayTriangulator.cpp
    mesh/ReversePrimitiveFunctor.cpp
    mesh/SceneGraphBuilder.cpp
    mesh/Tessellator.cpp
    mesh/TransformAttributeFunctor.cpp

    # utils
    utils/CubeMapGenerator.cpp
    utils/DisplayRequirementsVisitor.cpp
    utils/GLObjectsVisitor.cpp
    utils/HalfWayMapGenerator.cpp
    utils/HighlightMapGenerator.cpp
    utils/IncrementalCompileOperation.cpp
    utils/PerlinNoise.cpp
    utils/PrintVisitor.cpp
    utils/ShaderGen.cpp
    utils/Statistics.cpp
    utils/TransformCallback.cpp
    utils/Version.cpp

    ${OSGILIATH_VERSIONINFO_RC}
)

INCLUDE_DIRECTORIES("tristripper/include")

SET(TARGET_LIBRARIES
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Utility library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})
