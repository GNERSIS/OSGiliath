
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGPARTICLE_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgParticle)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/AccelOperator.hpp
    ${HEADER_PATH}/AngularAccelOperator.hpp
    ${HEADER_PATH}/BoxPlacer.hpp
    ${HEADER_PATH}/CenteredPlacer.hpp
    ${HEADER_PATH}/ConnectedParticleSystem.hpp
    ${HEADER_PATH}/ConstantRateCounter.hpp
    ${HEADER_PATH}/Counter.hpp
    ${HEADER_PATH}/Emitter.hpp
    ${HEADER_PATH}/ExplosionDebrisEffect.hpp
    ${HEADER_PATH}/ExplosionEffect.hpp
    ${HEADER_PATH}/Export.hpp
    ${HEADER_PATH}/FireEffect.hpp
    ${HEADER_PATH}/FluidFrictionOperator.hpp
    ${HEADER_PATH}/FluidProgram.hpp
    ${HEADER_PATH}/ForceOperator.hpp
    ${HEADER_PATH}/Interpolator.hpp
    ${HEADER_PATH}/LinearInterpolator.hpp
    ${HEADER_PATH}/ModularEmitter.hpp
    ${HEADER_PATH}/ModularProgram.hpp
    ${HEADER_PATH}/MultiSegmentPlacer.hpp
    ${HEADER_PATH}/Operator.hpp
    ${HEADER_PATH}/Particle.hpp
    ${HEADER_PATH}/ParticleEffect.hpp
    ${HEADER_PATH}/ParticleProcessor.hpp
    ${HEADER_PATH}/ParticleSystem.hpp
    ${HEADER_PATH}/ParticleSystemUpdater.hpp
    ${HEADER_PATH}/Placer.hpp
    ${HEADER_PATH}/PointPlacer.hpp
    ${HEADER_PATH}/PrecipitationEffect.hpp
    ${HEADER_PATH}/Program.hpp
    ${HEADER_PATH}/RadialShooter.hpp
    ${HEADER_PATH}/RandomRateCounter.hpp
    ${HEADER_PATH}/range.hpp
    ${HEADER_PATH}/SectorPlacer.hpp
    ${HEADER_PATH}/SegmentPlacer.hpp
    ${HEADER_PATH}/Shooter.hpp
    ${HEADER_PATH}/SmokeEffect.hpp
    ${HEADER_PATH}/SmokeTrailEffect.hpp
    ${HEADER_PATH}/VariableRateCounter.hpp
    ${HEADER_PATH}/Version.hpp
    
    ${HEADER_PATH}/CompositePlacer.hpp
    ${HEADER_PATH}/AngularDampingOperator.hpp
    ${HEADER_PATH}/DampingOperator.hpp
    ${HEADER_PATH}/ExplosionOperator.hpp
    ${HEADER_PATH}/OrbitOperator.hpp
    ${HEADER_PATH}/DomainOperator.hpp
    ${HEADER_PATH}/BounceOperator.hpp
    ${HEADER_PATH}/SinkOperator.hpp
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    ConnectedParticleSystem.cpp
    Emitter.cpp
    ExplosionDebrisEffect.cpp
    ExplosionEffect.cpp
    FireEffect.cpp
    FluidFrictionOperator.cpp
    FluidProgram.cpp
    ModularEmitter.cpp
    ModularProgram.cpp
    MultiSegmentPlacer.cpp
    Particle.cpp
    ParticleEffect.cpp
    ParticleProcessor.cpp
    ParticleSystem.cpp
    ParticleSystemUpdater.cpp
    PrecipitationEffect.cpp
    Program.cpp
    SmokeEffect.cpp
    SmokeTrailEffect.cpp
    Version.cpp
    
    DomainOperator.cpp
    BounceOperator.cpp
    SinkOperator.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgDB
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Particle system library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})
