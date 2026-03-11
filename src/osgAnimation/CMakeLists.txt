IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGANIMATION_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgAnimation)

SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    # core
    ${HEADER_PATH}/core/Action.hpp
    ${HEADER_PATH}/core/ActionAnimation.hpp
    ${HEADER_PATH}/core/ActionBlendIn.hpp
    ${HEADER_PATH}/core/ActionBlendOut.hpp
    ${HEADER_PATH}/core/ActionCallback.hpp
    ${HEADER_PATH}/core/ActionStripAnimation.hpp
    ${HEADER_PATH}/core/ActionVisitor.hpp
    ${HEADER_PATH}/core/Animation.hpp
    ${HEADER_PATH}/core/AnimationManagerBase.hpp
    ${HEADER_PATH}/core/AnimationUpdateCallback.hpp
    ${HEADER_PATH}/core/BasicAnimationManager.hpp
    ${HEADER_PATH}/core/Channel.hpp
    ${HEADER_PATH}/core/CubicBezier.hpp
    ${HEADER_PATH}/core/EaseMotion.hpp
    ${HEADER_PATH}/core/Export.hpp
    ${HEADER_PATH}/core/FrameAction.hpp
    ${HEADER_PATH}/core/Interpolator.hpp
    ${HEADER_PATH}/core/Keyframe.hpp
    ${HEADER_PATH}/core/Sampler.hpp
    ${HEADER_PATH}/core/Target.hpp
    ${HEADER_PATH}/core/Timeline.hpp
    ${HEADER_PATH}/core/TimelineAnimationManager.hpp
    ${HEADER_PATH}/core/UpdateUniform.hpp
    ${HEADER_PATH}/core/Vec3Packed.hpp

    # skeletal
    ${HEADER_PATH}/skeletal/Bone.hpp
    ${HEADER_PATH}/skeletal/BoneMapVisitor.hpp
    ${HEADER_PATH}/skeletal/LinkVisitor.hpp
    ${HEADER_PATH}/skeletal/RigGeometry.hpp
    ${HEADER_PATH}/skeletal/RigTransform.hpp
    ${HEADER_PATH}/skeletal/RigTransformHardware.hpp
    ${HEADER_PATH}/skeletal/RigTransformSoftware.hpp
    ${HEADER_PATH}/skeletal/Skeleton.hpp
    ${HEADER_PATH}/skeletal/UpdateBone.hpp
    ${HEADER_PATH}/skeletal/VertexInfluence.hpp

    # morph
    ${HEADER_PATH}/morph/MorphGeometry.hpp
    ${HEADER_PATH}/morph/MorphTransformHardware.hpp
    ${HEADER_PATH}/morph/MorphTransformSoftware.hpp
    ${HEADER_PATH}/morph/UpdateMaterial.hpp

    # transform
    ${HEADER_PATH}/transform/StackedMatrixElement.hpp
    ${HEADER_PATH}/transform/StackedQuaternionElement.hpp
    ${HEADER_PATH}/transform/StackedRotateAxisElement.hpp
    ${HEADER_PATH}/transform/StackedScaleElement.hpp
    ${HEADER_PATH}/transform/StackedTransform.hpp
    ${HEADER_PATH}/transform/StackedTransformElement.hpp
    ${HEADER_PATH}/transform/StackedTranslateElement.hpp
    ${HEADER_PATH}/transform/StatsHandler.hpp
    ${HEADER_PATH}/transform/StatsVisitor.hpp
    ${HEADER_PATH}/transform/UpdateMatrixTransform.hpp
)

SET(TARGET_SRC
    # core
    core/Action.cpp
    core/ActionAnimation.cpp
    core/ActionBlendIn.cpp
    core/ActionBlendOut.cpp
    core/ActionCallback.cpp
    core/ActionStripAnimation.cpp
    core/ActionVisitor.cpp
    core/Animation.cpp
    core/AnimationManagerBase.cpp
    core/BasicAnimationManager.cpp
    core/Channel.cpp
    core/Target.cpp
    core/Timeline.cpp
    core/TimelineAnimationManager.cpp

    # skeletal
    skeletal/Bone.cpp
    skeletal/BoneMapVisitor.cpp
    skeletal/LinkVisitor.cpp
    skeletal/RigGeometry.cpp
    skeletal/RigTransformHardware.cpp
    skeletal/RigTransformSoftware.cpp
    skeletal/Skeleton.cpp
    skeletal/UpdateBone.cpp
    skeletal/VertexInfluence.cpp

    # morph
    morph/MorphGeometry.cpp
    morph/MorphTransformHardware.cpp
    morph/MorphTransformSoftware.cpp
    morph/UpdateMaterial.cpp

    # transform
    transform/StackedMatrixElement.cpp
    transform/StackedQuaternionElement.cpp
    transform/StackedRotateAxisElement.cpp
    transform/StackedScaleElement.cpp
    transform/StackedTransform.cpp
    transform/StackedTranslateElement.cpp
    transform/StatsHandler.cpp
    transform/StatsVisitor.cpp
    transform/UpdateMatrixTransform.cpp

    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osg
    osgText
    osgGA
    osgViewer
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Skinning and morphing library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})
