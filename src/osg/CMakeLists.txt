if(DYNAMIC_OSGILIATH)
    add_definitions(-DOSG_LIBRARY)
else()
    add_definitions(-DOSG_LIBRARY_STATIC)
endif()

set(LIB_NAME osg)

option(OSG_FORCE_QUERY_RESULT_AVAILABLE_BEFORE_RETRIEVAL
    "Workaround for multi-threaded occlusion query crash" OFF)
if(OSG_FORCE_QUERY_RESULT_AVAILABLE_BEFORE_RETRIEVAL)
    add_definitions(-DFORCE_QUERY_RESULT_AVAILABLE_BEFORE_RETRIEVAL)
endif()

set(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})

# ── Headers ──────────────────────────────────────────────────

set(TARGET_H
    # core/
    ${HEADER_PATH}/core/ApplicationUsage.hpp
    ${HEADER_PATH}/core/ArgumentParser.hpp
    ${HEADER_PATH}/core/BoundsChecking.hpp
    ${HEADER_PATH}/core/buffered_value.hpp
    ${HEADER_PATH}/core/Callback.hpp
    ${HEADER_PATH}/core/compare.hpp
    ${HEADER_PATH}/core/CopyOp.hpp
    ${HEADER_PATH}/core/DeleteHandler.hpp
    ${HEADER_PATH}/core/DeleteQueue.hpp
    ${HEADER_PATH}/core/Endian.hpp
    ${HEADER_PATH}/core/Export.hpp
    ${HEADER_PATH}/core/fast_back_stack.hpp
    ${HEADER_PATH}/core/FrameBlock.hpp
    ${HEADER_PATH}/core/FrameStamp.hpp
    ${HEADER_PATH}/core/Identifier.hpp
    ${HEADER_PATH}/core/Inherit.hpp
    ${HEADER_PATH}/core/io_utils.hpp
    ${HEADER_PATH}/core/Latch.hpp
    ${HEADER_PATH}/core/MixinVector.hpp
    ${HEADER_PATH}/core/NodeCallback.hpp
    ${HEADER_PATH}/core/Notify.hpp
    ${HEADER_PATH}/core/Object.hpp
    ${HEADER_PATH}/core/Observer.hpp
    ${HEADER_PATH}/core/ObserverNodePath.hpp
    ${HEADER_PATH}/core/observer_ptr.hpp
    ${HEADER_PATH}/core/os_utils.hpp
    ${HEADER_PATH}/core/Referenced.hpp
    ${HEADER_PATH}/core/ref_ptr.hpp
    ${HEADER_PATH}/core/StateAttributeCallback.hpp
    ${HEADER_PATH}/core/Stats.hpp
    ${HEADER_PATH}/core/Timer.hpp
    ${HEADER_PATH}/core/type_name.hpp
    ${HEADER_PATH}/core/Types.hpp
    ${HEADER_PATH}/core/UserDataContainer.hpp
    ${HEADER_PATH}/core/ValueMap.hpp
    ${HEADER_PATH}/core/ValueObject.hpp
    ${HEADER_PATH}/core/ValueStack.hpp

    # maths/
    ${HEADER_PATH}/maths/BoundingBox.hpp
    ${HEADER_PATH}/maths/BoundingSphere.hpp
    ${HEADER_PATH}/maths/box.hpp
    ${HEADER_PATH}/maths/clamp.hpp
    ${HEADER_PATH}/maths/color.hpp
    ${HEADER_PATH}/maths/common.hpp
    ${HEADER_PATH}/maths/compat.hpp
    ${HEADER_PATH}/maths/mat2.hpp
    ${HEADER_PATH}/maths/mat3.hpp
    ${HEADER_PATH}/maths/mat4.hpp
    ${HEADER_PATH}/maths/Math.hpp
    ${HEADER_PATH}/maths/Matrix.hpp
    ${HEADER_PATH}/maths/Matrixd.hpp
    ${HEADER_PATH}/maths/Matrixf.hpp
    ${HEADER_PATH}/maths/MatrixTemplate.hpp
    ${HEADER_PATH}/maths/numbers.hpp
    ${HEADER_PATH}/maths/Plane.hpp
    ${HEADER_PATH}/maths/Quat.hpp
    ${HEADER_PATH}/maths/quat.hpp
    ${HEADER_PATH}/maths/RefMatrix.hpp
    ${HEADER_PATH}/maths/sample.hpp
    ${HEADER_PATH}/maths/sphere.hpp
    ${HEADER_PATH}/maths/transform.hpp
    ${HEADER_PATH}/maths/vec2.hpp
    ${HEADER_PATH}/maths/Vec2.hpp
    ${HEADER_PATH}/maths/Vec2b.hpp
    ${HEADER_PATH}/maths/Vec2d.hpp
    ${HEADER_PATH}/maths/Vec2f.hpp
    ${HEADER_PATH}/maths/Vec2i.hpp
    ${HEADER_PATH}/maths/Vec2s.hpp
    ${HEADER_PATH}/maths/Vec2ub.hpp
    ${HEADER_PATH}/maths/Vec2ui.hpp
    ${HEADER_PATH}/maths/Vec2us.hpp
    ${HEADER_PATH}/maths/vec3.hpp
    ${HEADER_PATH}/maths/Vec3.hpp
    ${HEADER_PATH}/maths/Vec3b.hpp
    ${HEADER_PATH}/maths/Vec3d.hpp
    ${HEADER_PATH}/maths/Vec3f.hpp
    ${HEADER_PATH}/maths/Vec3i.hpp
    ${HEADER_PATH}/maths/Vec3s.hpp
    ${HEADER_PATH}/maths/Vec3ub.hpp
    ${HEADER_PATH}/maths/Vec3ui.hpp
    ${HEADER_PATH}/maths/Vec3us.hpp
    ${HEADER_PATH}/maths/vec4.hpp
    ${HEADER_PATH}/maths/Vec4.hpp
    ${HEADER_PATH}/maths/Vec4b.hpp
    ${HEADER_PATH}/maths/Vec4d.hpp
    ${HEADER_PATH}/maths/Vec4f.hpp
    ${HEADER_PATH}/maths/Vec4i.hpp
    ${HEADER_PATH}/maths/Vec4s.hpp
    ${HEADER_PATH}/maths/Vec4ub.hpp
    ${HEADER_PATH}/maths/Vec4ui.hpp
    ${HEADER_PATH}/maths/Vec4us.hpp

    # nodes/
    ${HEADER_PATH}/nodes/AutoTransform.hpp
    ${HEADER_PATH}/nodes/Billboard.hpp
    ${HEADER_PATH}/nodes/Camera.hpp
    ${HEADER_PATH}/nodes/CameraView.hpp
    ${HEADER_PATH}/nodes/ClearNode.hpp
    ${HEADER_PATH}/nodes/CoordinateSystemNode.hpp
    ${HEADER_PATH}/nodes/Geode.hpp
    ${HEADER_PATH}/nodes/Group.hpp
    ${HEADER_PATH}/nodes/LOD.hpp
    ${HEADER_PATH}/nodes/MatrixTransform.hpp
    ${HEADER_PATH}/nodes/Node.hpp
    ${HEADER_PATH}/nodes/OccluderNode.hpp
    ${HEADER_PATH}/nodes/PagedLOD.hpp
    ${HEADER_PATH}/nodes/PositionAttitudeTransform.hpp
    ${HEADER_PATH}/nodes/Projection.hpp
    ${HEADER_PATH}/nodes/ProxyNode.hpp
    ${HEADER_PATH}/nodes/Sequence.hpp
    ${HEADER_PATH}/nodes/Switch.hpp
    ${HEADER_PATH}/nodes/Transform.hpp
    ${HEADER_PATH}/nodes/View.hpp

    # state/
    ${HEADER_PATH}/state/BindImageTexture.hpp
    ${HEADER_PATH}/state/BlendColor.hpp
    ${HEADER_PATH}/state/BlendEquation.hpp
    ${HEADER_PATH}/state/BlendEquationi.hpp
    ${HEADER_PATH}/state/BlendFunc.hpp
    ${HEADER_PATH}/state/BlendFunci.hpp
    ${HEADER_PATH}/state/BufferIndexBinding.hpp
    ${HEADER_PATH}/state/Capability.hpp
    ${HEADER_PATH}/state/ClampColor.hpp
    ${HEADER_PATH}/state/ClipControl.hpp
    ${HEADER_PATH}/state/ColorMask.hpp
    ${HEADER_PATH}/state/ColorMaski.hpp
    ${HEADER_PATH}/state/CullFace.hpp
    ${HEADER_PATH}/state/Depth.hpp
    ${HEADER_PATH}/state/DepthRangeIndexed.hpp
    ${HEADER_PATH}/state/DispatchCompute.hpp
    ${HEADER_PATH}/state/FrontFace.hpp
    ${HEADER_PATH}/state/GLDefines.hpp
    ${HEADER_PATH}/state/Hint.hpp
    ${HEADER_PATH}/state/LineWidth.hpp
    ${HEADER_PATH}/state/LogicOp.hpp
    ${HEADER_PATH}/state/Material.hpp
    ${HEADER_PATH}/state/Multisample.hpp
    ${HEADER_PATH}/state/PatchParameter.hpp
    ${HEADER_PATH}/state/Point.hpp
    ${HEADER_PATH}/state/PolygonMode.hpp
    ${HEADER_PATH}/state/PolygonOffset.hpp
    ${HEADER_PATH}/state/Program.hpp
    ${HEADER_PATH}/state/SampleMaski.hpp
    ${HEADER_PATH}/state/Scissor.hpp
    ${HEADER_PATH}/state/ScissorIndexed.hpp
    ${HEADER_PATH}/state/Shader.hpp
    ${HEADER_PATH}/state/ShaderAttribute.hpp
    ${HEADER_PATH}/state/State.hpp
    ${HEADER_PATH}/state/StateAttribute.hpp
    ${HEADER_PATH}/state/StateSet.hpp
    ${HEADER_PATH}/state/Stencil.hpp
    ${HEADER_PATH}/state/StencilTwoSided.hpp
    ${HEADER_PATH}/state/TextureAttribute.hpp
    ${HEADER_PATH}/state/Uniform.hpp
    ${HEADER_PATH}/state/UniformBase.hpp
    ${HEADER_PATH}/state/VertexAttribDivisor.hpp
    ${HEADER_PATH}/state/Viewport.hpp
    ${HEADER_PATH}/state/ViewportIndexed.hpp

    # geometry/
    ${HEADER_PATH}/geometry/Array.hpp
    ${HEADER_PATH}/geometry/AttributeDispatchers.hpp
    ${HEADER_PATH}/geometry/BufferObject.hpp
    ${HEADER_PATH}/geometry/BufferTemplate.hpp
    ${HEADER_PATH}/geometry/ConvexPlanarOccluder.hpp
    ${HEADER_PATH}/geometry/ConvexPlanarPolygon.hpp
    ${HEADER_PATH}/geometry/Drawable.hpp
    ${HEADER_PATH}/geometry/DrawPixels.hpp
    ${HEADER_PATH}/geometry/Geometry.hpp
    ${HEADER_PATH}/geometry/PrimitiveRestartIndex.hpp
    ${HEADER_PATH}/geometry/PrimitiveSet.hpp
    ${HEADER_PATH}/geometry/PrimitiveSetIndirect.hpp
    ${HEADER_PATH}/geometry/Shape.hpp
    ${HEADER_PATH}/geometry/ShapeDrawable.hpp
    ${HEADER_PATH}/geometry/TemplatePrimitiveFunctor.hpp
    ${HEADER_PATH}/geometry/TemplatePrimitiveIndexFunctor.hpp
    ${HEADER_PATH}/geometry/TriangleFunctor.hpp
    ${HEADER_PATH}/geometry/TriangleIndexFunctor.hpp
    ${HEADER_PATH}/geometry/TriangleLinePointIndexFunctor.hpp
    ${HEADER_PATH}/geometry/VertexArrayState.hpp

    # textures/
    ${HEADER_PATH}/textures/AudioStream.hpp
    ${HEADER_PATH}/textures/ImageSequence.hpp
    ${HEADER_PATH}/textures/ImageStream.hpp
    ${HEADER_PATH}/textures/Sampler.hpp
    ${HEADER_PATH}/textures/TexGen.hpp
    ${HEADER_PATH}/textures/Texture.hpp
    ${HEADER_PATH}/textures/Texture1D.hpp
    ${HEADER_PATH}/textures/Texture2D.hpp
    ${HEADER_PATH}/textures/Texture2DArray.hpp
    ${HEADER_PATH}/textures/Texture2DMultisample.hpp
    ${HEADER_PATH}/textures/Texture3D.hpp
    ${HEADER_PATH}/textures/TextureBuffer.hpp
    ${HEADER_PATH}/textures/TextureCubeMap.hpp
    ${HEADER_PATH}/textures/TextureRectangle.hpp
    ${HEADER_PATH}/textures/TransferFunction.hpp

    # images/
    ${HEADER_PATH}/images/Image.hpp
    ${HEADER_PATH}/images/ImageUtils.hpp

    # rendering/
    ${HEADER_PATH}/rendering/ContextData.hpp
    ${HEADER_PATH}/rendering/DisplaySettings.hpp
    ${HEADER_PATH}/rendering/FrameBufferObject.hpp
    ${HEADER_PATH}/rendering/GLExtensions.hpp
    ${HEADER_PATH}/rendering/GLObjects.hpp
    ${HEADER_PATH}/rendering/GLU.hpp
    ${HEADER_PATH}/rendering/GraphicsContext.hpp
    ${HEADER_PATH}/rendering/GraphicsCostEstimator.hpp
    ${HEADER_PATH}/rendering/HeadlessCapture.hpp
    ${HEADER_PATH}/rendering/OcclusionQueryNode.hpp
    ${HEADER_PATH}/rendering/RenderInfo.hpp

    # traversal/
    ${HEADER_PATH}/traversal/AnimationPath.hpp
    ${HEADER_PATH}/traversal/ClusterCullingCallback.hpp
    ${HEADER_PATH}/traversal/CollectOccludersVisitor.hpp
    ${HEADER_PATH}/traversal/ComputeBoundsVisitor.hpp
    ${HEADER_PATH}/traversal/CullingSet.hpp
    ${HEADER_PATH}/traversal/CullSettings.hpp
    ${HEADER_PATH}/traversal/CullStack.hpp
    ${HEADER_PATH}/geometry/KdTree.hpp
    ${HEADER_PATH}/traversal/LineSegment.hpp
    ${HEADER_PATH}/traversal/NodeTrackerCallback.hpp
    ${HEADER_PATH}/traversal/NodeVisitor.hpp
    ${HEADER_PATH}/traversal/Polytope.hpp
    ${HEADER_PATH}/traversal/ScriptEngine.hpp
    ${HEADER_PATH}/traversal/ShadowVolumeOccluder.hpp

    # lighting/
    ${HEADER_PATH}/lighting/Light.hpp
    ${HEADER_PATH}/lighting/LightSource.hpp

    # threading/
    ${HEADER_PATH}/threading/GraphicsThread.hpp
    ${HEADER_PATH}/threading/OperationThread.hpp
    ${HEADER_PATH}/threading/Thread.hpp

    # generated
    ${OSGILIATH_VERSION_HEADER}
    ${OSGILIATH_CONFIG_HEADER}
    ${OSGILIATH_OPENGL_HEADER}
)

# ── Sources ──────────────────────────────────────────────────

set(TARGET_SRC
    # core/
    core/ApplicationUsage.cpp
    core/ArgumentParser.cpp
    core/Callback.cpp
    core/CopyOp.cpp
    core/DeleteHandler.cpp
    core/FrameStamp.cpp
    core/Identifier.cpp
    core/Notify.cpp
    core/Object.cpp
    core/Observer.cpp
    core/ObserverNodePath.cpp
    core/os_utils.cpp
    core/Referenced.cpp
    core/Stats.cpp
    core/Thread.cpp
    core/Timer.cpp
    core/UserDataContainer.cpp
    core/ValueMap.cpp
    core/ValueStack.cpp
    core/Version.cpp

    # maths/
    maths/Math.cpp
    maths/maths_impl.cpp

    # nodes/
    nodes/AutoTransform.cpp
    nodes/Billboard.cpp
    nodes/Camera.cpp
    nodes/CameraView.cpp
    nodes/ClearNode.cpp
    nodes/CoordinateSystemNode.cpp
    nodes/Geode.cpp
    nodes/Group.cpp
    nodes/LOD.cpp
    nodes/MatrixTransform.cpp
    nodes/Node.cpp
    nodes/OccluderNode.cpp
    nodes/PagedLOD.cpp
    nodes/PositionAttitudeTransform.cpp
    nodes/Projection.cpp
    nodes/ProxyNode.cpp
    nodes/Sequence.cpp
    nodes/Switch.cpp
    nodes/Transform.cpp
    nodes/View.cpp

    # state/
    state/BindImageTexture.cpp
    state/BlendColor.cpp
    state/BlendEquation.cpp
    state/BlendEquationi.cpp
    state/BlendFunc.cpp
    state/BlendFunci.cpp
    state/BufferIndexBinding.cpp
    state/Capability.cpp
    state/ClampColor.cpp
    state/ClipControl.cpp
    state/ColorMask.cpp
    state/ColorMaski.cpp
    state/CullFace.cpp
    state/Depth.cpp
    state/DepthRangeIndexed.cpp
    state/DispatchCompute.cpp
    state/FrontFace.cpp
    state/Hint.cpp
    state/LineWidth.cpp
    state/LogicOp.cpp
    state/Material.cpp
    state/Multisample.cpp
    state/PatchParameter.cpp
    state/Point.cpp
    state/PolygonMode.cpp
    state/PolygonOffset.cpp
    state/Program.cpp
    state/SampleMaski.cpp
    state/Scissor.cpp
    state/ScissorIndexed.cpp
    state/Shader.cpp
    state/ShaderAttribute.cpp
    state/State.cpp
    state/StateAttribute.cpp
    state/StateSet.cpp
    state/Stencil.cpp
    state/StencilTwoSided.cpp
    state/Uniform.cpp
    state/VertexAttribDivisor.cpp
    state/Viewport.cpp
    state/ViewportIndexed.cpp

    # geometry/
    geometry/Array.cpp
    geometry/AttributeDispatchers.cpp
    geometry/BufferObject.cpp
    geometry/ConvexPlanarOccluder.cpp
    geometry/ConvexPlanarPolygon.cpp
    geometry/Drawable.cpp
    geometry/DrawPixels.cpp
    geometry/Geometry.cpp
    geometry/KdTree.cpp
    geometry/PrimitiveRestartIndex.cpp
    geometry/PrimitiveSet.cpp
    geometry/PrimitiveSetIndirect.cpp
    geometry/Shape.cpp
    geometry/ShapeDrawable.cpp
    geometry/VertexArrayState.cpp

    # textures/
    textures/AudioStream.cpp
    textures/ImageSequence.cpp
    textures/ImageStream.cpp
    textures/Sampler.cpp
    textures/TexGen.cpp
    textures/Texture.cpp
    textures/Texture1D.cpp
    textures/Texture2D.cpp
    textures/Texture2DArray.cpp
    textures/Texture2DMultisample.cpp
    textures/Texture3D.cpp
    textures/TextureBuffer.cpp
    textures/TextureCubeMap.cpp
    textures/TextureRectangle.cpp
    textures/TransferFunction.cpp

    # images/
    images/dxtctool.cpp
    images/Image.cpp
    images/ImageUtils.cpp

    # rendering/
    rendering/ContextData.cpp
    rendering/DisplaySettings.cpp
    rendering/FrameBufferObject.cpp
    rendering/GLExtensions.cpp
    rendering/GLObjects.cpp
    rendering/GLStaticLibrary.cpp
    rendering/GraphicsContext.cpp
    rendering/GraphicsCostEstimator.cpp
    rendering/OcclusionQueryNode.cpp

    # traversal/
    traversal/AnimationPath.cpp
    traversal/ClusterCullingCallback.cpp
    traversal/CollectOccludersVisitor.cpp
    traversal/ComputeBoundsVisitor.cpp
    traversal/CullingSet.cpp
    traversal/CullSettings.cpp
    traversal/CullStack.cpp
    traversal/LineSegment.cpp
    traversal/NodeTrackerCallback.cpp
    traversal/NodeVisitor.cpp
    traversal/Polytope.cpp
    traversal/ScriptEngine.cpp
    traversal/ShadowVolumeOccluder.cpp

    # lighting/
    lighting/Light.cpp
    lighting/LightSource.cpp

    # threading/
    threading/GraphicsThread.cpp
    threading/OperationThread.cpp

    # glu/
    glu/libutil/error.cpp
    glu/libutil/mipmap.cpp
    glu/libtess/dict.cpp
    glu/libtess/geom.cpp
    glu/libtess/memalloc.cpp
    glu/libtess/mesh.cpp
    glu/libtess/normal.cpp
    glu/libtess/priorityq.cpp
    glu/libtess/render.cpp
    glu/libtess/sweep.cpp
    glu/libtess/tess.cpp
    glu/libtess/tessmono.cpp
)

set(TARGET_LIBRARIES)

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(EGL QUIET egl)
endif()

set(TARGET_EXTERNAL_LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
    ${MATH_LIBRARY}
    ${RT_LIBRARY}
    ${DL_LIBRARY}
    GLEW::GLEW
    ${OPENGL_gl_LIBRARY}
    EGL
)

set(COMPONENT_PKGCONFIG_DESCRIPTION "3D scenegraph")

SETUP_LIBRARY(${LIB_NAME})
