/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

namespace
{

    constexpr char pbrVertexShader[]   = R"glsl(
#version 460 core

#pragma import_defines(OSG_GLTF_PBR_ENABLE_NORMAL_MAP)
#pragma import_defines(OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE)
#pragma import_defines(OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE)

#ifndef OSG_GLTF_PBR_ENABLE_NORMAL_MAP
#define OSG_GLTF_PBR_ENABLE_NORMAL_MAP 1
#endif
#ifndef OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
#define OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE 1
#endif
#ifndef OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
#define OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE 1
#endif

layout(location = 0) in vec4 osg_Vertex;
#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
layout(location = 1) in vec3 osg_RadianceBake;
#endif
layout(location = 2) in vec3 osg_Normal;
#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
layout(location = 6) in vec4 osg_Tangent;
#endif
#if OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
layout(location = 7) in vec4 osg_VisBake;
#endif
layout(location = 8) in vec4 osg_MultiTexCoord0;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform mat3 osg_NormalMatrix;

out vec3 v_position;
out vec3 v_normal;
#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
out vec3 v_tangent;
out vec3 v_bitangent;
#endif
out vec2 v_texcoord0;
#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
out vec3 vBakedIrradiance;
#endif
#if OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
out vec3 vBentNormalWorld;
out float vVisBake;
#endif

vec3 safeNormalize(vec3 value, vec3 fallback)
{
    float lengthSquared = dot(value, value);
    return (lengthSquared > 1e-10) ? value * inversesqrt(lengthSquared) : fallback;
}

#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
vec3 fallbackTangent(vec3 normal)
{
    vec3 up = (abs(normal.z) < 0.999) ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    return safeNormalize(cross(up, normal), vec3(1.0, 0.0, 0.0));
}
#endif

void main()
{
    v_position = (osg_ModelViewMatrix * osg_Vertex).xyz;

    vec3 normal = safeNormalize(osg_NormalMatrix * osg_Normal, vec3(0.0, 0.0, 1.0));
#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
    vec3 tangent = osg_NormalMatrix * osg_Tangent.xyz;
    tangent = tangent - normal * dot(normal, tangent);
    tangent = safeNormalize(tangent, fallbackTangent(normal));

    float handedness = (osg_Tangent.w < 0.0) ? -1.0 : 1.0;
    vec3 bitangent = safeNormalize(cross(normal, tangent), vec3(0.0, 1.0, 0.0)) * handedness;
#endif

    v_normal = normal;
#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
    v_tangent = tangent;
    v_bitangent = bitangent;
#endif
    v_texcoord0 = osg_MultiTexCoord0.xy;
#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
    vBakedIrradiance = osg_RadianceBake;
#endif
#if OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
    vBentNormalWorld = safeNormalize(osg_VisBake.xyz, vec3(0.0, 1.0, 0.0));
    vVisBake = osg_VisBake.w;
#endif

    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
}
)glsl";

    constexpr char pbrFragmentShader[] = R"glsl(
#version 460 core

#pragma import_defines(OSG_GLTF_PBR_ENABLE_NORMAL_MAP)
#pragma import_defines(OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE)
#pragma import_defines(OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE)
#pragma import_defines(OSG_GLTF_PBR_ENABLE_SHADOW_MAP)
#pragma import_defines(OSGSPONZA_SHADOW_TAPS)
#pragma import_defines(OSGSPONZA_SHADOW_FILTER)
#pragma import_defines(OSG_GLTF_PBR_ENABLE_RT_SHADOWS)
#pragma import_defines(OSG_GLTF_PBR_ENABLE_IBL_SPECULAR)
#pragma import_defines(OSG_GLTF_PBR_ENABLE_DIRECT_SPECULAR)
#pragma import_defines(OSGSPONZA_MERGE_INDIRECT)

#ifndef OSG_GLTF_PBR_ENABLE_NORMAL_MAP
#define OSG_GLTF_PBR_ENABLE_NORMAL_MAP 1
#endif
#ifndef OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
#define OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE 1
#endif
#ifndef OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
#define OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE 1
#endif
#ifndef OSG_GLTF_PBR_ENABLE_SHADOW_MAP
#define OSG_GLTF_PBR_ENABLE_SHADOW_MAP 1
#endif
#ifndef OSGSPONZA_SHADOW_TAPS
#define OSGSPONZA_SHADOW_TAPS 16
#endif
#ifndef OSGSPONZA_SHADOW_FILTER
#define OSGSPONZA_SHADOW_FILTER 1
#endif
#ifndef OSG_GLTF_PBR_ENABLE_RT_SHADOWS
#define OSG_GLTF_PBR_ENABLE_RT_SHADOWS 1
#endif
#ifndef OSG_GLTF_PBR_ENABLE_IBL_SPECULAR
#define OSG_GLTF_PBR_ENABLE_IBL_SPECULAR 1
#endif
#ifndef OSG_GLTF_PBR_ENABLE_DIRECT_SPECULAR
#define OSG_GLTF_PBR_ENABLE_DIRECT_SPECULAR 1
#endif
#ifndef OSGSPONZA_MERGE_INDIRECT
#define OSGSPONZA_MERGE_INDIRECT 0
#endif

const float PI = 3.14159265358979323846;

struct osg_LightSourceParameters {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
    vec3 spotDirection;
    float spotExponent;
    float spotCutoff;
    float spotCosCutoff;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};
uniform osg_LightSourceParameters osg_LightSource;

uniform vec4 osg_LightModel_ambient;
uniform bool osg_LightingEnabled;

uniform sampler2D uBaseColorMap;
uniform sampler2D uMetallicRoughnessMap;
#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
uniform sampler2D uNormalMap;
#endif
uniform sampler2D uOcclusionMap;
uniform sampler2D uEmissiveMap;
uniform sampler2D uEnvMap;
#if OSG_GLTF_PBR_ENABLE_SHADOW_MAP
layout(binding = 6) uniform sampler2DShadow uShadowMap;
#endif

uniform vec4 uBaseColorFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform float uNormalScale;
uniform float uOcclusionStrength;
uniform vec3 uEmissiveFactor;
uniform float uAlphaCutoff;
uniform float uEnvMaxLod;
uniform float uEnvClamp;
uniform float uIblIntensity;
uniform float uIblDiffuse;
uniform float uIblSpecular;
uniform float uGlassReflectance;
uniform float uDarkMetalLift;
uniform float uEnvRotation;
#if OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
uniform float uVisStrength;
uniform float uVisPower;
uniform float uVisBentStrength;
#endif
uniform vec3 uIrradianceSH[9];
uniform vec3 uBounceRadiance;
#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
uniform float uRadianceScale;
#endif
#if OSG_GLTF_PBR_ENABLE_SHADOW_MAP
uniform mat4 uShadowMatrix;
uniform float uShadowSoftness;
uniform float uShadowBias;
uniform float uShadowNormalOffset;
#endif
uniform mat3 uViewToWorldRot;
#if OSG_GLTF_PBR_ENABLE_RT_SHADOWS
uniform bool uUseRtSunShadow;
uniform float uRtShadowNormalOffset;
uniform float uRtShadowMaxDistance;
uniform int uRtShadowSamples;
uniform float uRtSunAngularRadius;
uniform int uRtBvhNodeCount;
uniform int uRtTriangleCount;
uniform int uRtTriangleIndexCount;
uniform vec3 uRtViewOriginWorld;
uniform vec3 uRtSunDirectionWorld;
uniform bool uRtShadowDebug;
uniform float uRtRasterGateMin;
uniform float uRtRasterGateMax;
#else
uniform vec3 uRtSunDirectionWorld;
#endif

uniform bool uHasBaseColorMap;
uniform bool uHasMetallicRoughnessMap;
#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
uniform bool uHasNormalMap;
#endif
uniform bool uHasOcclusionMap;
uniform bool uHasEmissiveMap;
uniform bool uAlphaMask;
uniform bool uHasEnv;
uniform bool uUseShIrradiance;
#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
uniform bool uUseRadianceBake;
uniform bool uRadianceDebug;
#endif
#if OSG_GLTF_PBR_ENABLE_SHADOW_MAP
uniform bool uHasShadow;
#endif
#if OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
uniform bool uHasVisBake;
#endif
uniform bool uDoubleSidedMaterial;

#if OSG_GLTF_PBR_ENABLE_RT_SHADOWS
layout(std430, binding = 0) readonly buffer RtNodeMinBuffer {
    vec4 nodeMin[];
};

layout(std430, binding = 1) readonly buffer RtNodeMaxBuffer {
    vec4 nodeMax[];
};

layout(std430, binding = 2) readonly buffer RtNodeMetaBuffer {
    uvec4 nodeMeta[];
};

layout(std430, binding = 3) readonly buffer RtTriangleIndexBuffer {
    uint triangleIndices[];
};

layout(std430, binding = 4) readonly buffer RtTriangleV0EdgeBuffer {
    vec4 triV0Edge[];
};

layout(std430, binding = 5) readonly buffer RtTriangleV1Buffer {
    vec4 triV1[];
};

layout(std430, binding = 6) readonly buffer RtTriangleV2Buffer {
    vec4 triV2[];
};

layout(std430, binding = 7) readonly buffer RtTriangleNormalBuffer {
    vec4 triNormal[];
};
#endif

in vec3 v_position;
in vec3 v_normal;
#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
in vec3 v_tangent;
in vec3 v_bitangent;
#endif
in vec2 v_texcoord0;
#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
in vec3 vBakedIrradiance;
#endif
#if OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
in vec3 vBentNormalWorld;
in float vVisBake;
#endif

layout(location = 0) out vec4 osg_FragColor;
layout(location = 1) out vec4 osg_IndirectColor;

vec3 safeNormalize(vec3 value, vec3 fallback)
{
    float lengthSquared = dot(value, value);
    return (lengthSquared > 1e-10) ? value * inversesqrt(lengthSquared) : fallback;
}

#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
vec3 fallbackTangent(vec3 normal)
{
    vec3 up = (abs(normal.z) < 0.999) ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    return safeNormalize(cross(up, normal), vec3(1.0, 0.0, 0.0));
}
#endif

#if OSG_GLTF_PBR_ENABLE_DIRECT_SPECULAR
float distributionGGX(vec3 normal, vec3 halfway, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float nDotH = max(dot(normal, halfway), 0.0);
    float nDotH2 = nDotH * nDotH;
    float denom = (nDotH2 * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 1e-7);
}

float geometrySchlickGGX(float nDotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return nDotV / max(nDotV * (1.0 - k) + k, 1e-7);
}

float geometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness)
{
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);
    return geometrySchlickGGX(nDotV, roughness) * geometrySchlickGGX(nDotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
#endif

vec3 sampleEnv(vec3 dirWorld, float lod)
{
    vec3 d = dirWorld;
    float len = length(d);
    d = (len > 1e-6) ? d / len : vec3(0.0, 1.0, 0.0);
    float lon = (abs(d.x) + abs(d.z) < 1e-5) ? 0.0 : atan(d.z, d.x);
    float u = lon / (2.0 * PI) + 0.5 + uEnvRotation / (2.0 * PI);
    float v = acos(clamp(d.y, -1.0, 1.0)) / PI;
    return textureLod(uEnvMap, vec2(u, v), lod).rgb;
}

vec3 sampleEnvClamped(vec3 dirWorld, float lod)
{
    return min(sampleEnv(dirWorld, lod), vec3(uEnvClamp));
}

vec3 evaluateIrradianceSH(vec3 dirWorld)
{
    vec3 d = safeNormalize(dirWorld, vec3(0.0, 1.0, 0.0));
    return
        uIrradianceSH[0] * 0.28209479177387814 +
        uIrradianceSH[1] * (0.4886025119029199 * d.y) +
        uIrradianceSH[2] * (0.4886025119029199 * d.z) +
        uIrradianceSH[3] * (0.4886025119029199 * d.x) +
        uIrradianceSH[4] * (1.0925484305920792 * d.x * d.y) +
        uIrradianceSH[5] * (1.0925484305920792 * d.y * d.z) +
        uIrradianceSH[6] * (0.31539156525252005 * (3.0 * d.y * d.y - 1.0)) +
        uIrradianceSH[7] * (1.0925484305920792 * d.x * d.z) +
        uIrradianceSH[8] * (0.5462742152960396 * (d.x * d.x - d.z * d.z));
}

#if OSG_GLTF_PBR_ENABLE_IBL_SPECULAR
vec2 envBRDFApprox(float rough, float ndv)
{
    const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = rough * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * ndv)) * r.x + r.y;
    return vec2(-1.04, 1.04) * a004 + r.zw;
}
#endif

#if OSG_GLTF_PBR_ENABLE_SHADOW_MAP
const int shadowMaxTapCount = 16;
const int shadowTapCount = OSGSPONZA_SHADOW_TAPS;
const vec2 shadowPoisson[shadowMaxTapCount] = vec2[shadowMaxTapCount](
    vec2(-0.94201624, -0.39906216),
    vec2( 0.94558609, -0.76890725),
    vec2(-0.09418410, -0.92938870),
    vec2( 0.34495938,  0.29387760),
    vec2(-0.91588581,  0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543,  0.27676845),
    vec2( 0.97484398,  0.75648379),
    vec2( 0.44323325, -0.97511554),
    vec2( 0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2( 0.79197514,  0.19090188),
    vec2(-0.24188840,  0.99706507),
    vec2(-0.81409955,  0.91437590),
    vec2( 0.19984126,  0.78641367),
    vec2( 0.14383161, -0.14100790)
);

float sunShadow(vec3 viewPos, vec3 geometricNormal)
{
    vec3 normal = safeNormalize(geometricNormal, vec3(0.0, 0.0, 1.0));
    vec4 shadowCoord = uShadowMatrix * vec4(viewPos + normal * uShadowNormalOffset, 1.0);
    if (abs(shadowCoord.w) < 1e-7) {
        return 1.0;
    }

    vec3 sc = shadowCoord.xyz / shadowCoord.w;
    if (any(lessThan(sc, vec3(0.0))) || any(greaterThan(sc, vec3(1.0)))) {
        return 1.0;
    }

    float angle = fract(
        sin(dot(sc.xy * 4096.0, vec2(12.9898, 78.233))) * 43758.5453
    ) * 2.0 * PI;
    mat2 rotation = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
    float referenceDepth = sc.z - uShadowBias;
#if OSGSPONZA_SHADOW_FILTER == 0
    return texture(uShadowMap, vec3(sc.xy, referenceDepth));
#else
    if (shadowTapCount <= 1 || uShadowSoftness <= 0.0) {
        return texture(uShadowMap, vec3(sc.xy, referenceDepth));
    }

    float visibility = 0.0;
    for (int i = 0; i < shadowMaxTapCount; ++i) {
        if (i >= shadowTapCount) {
            break;
        }
        vec2 uv = sc.xy + rotation * shadowPoisson[i] * uShadowSoftness;
        visibility += texture(uShadowMap, vec3(uv, referenceDepth));
    }
    return visibility / float(shadowTapCount);
#endif
}
#endif

#if OSG_GLTF_PBR_ENABLE_RT_SHADOWS
const float rtRayHitEpsilon = 1e-4;
const int rtBvhStackSize = 96;
const int rtSunShadowSampleCount = 8;
const vec2 rtSunShadowSamples[rtSunShadowSampleCount] = vec2[rtSunShadowSampleCount](
    vec2( 0.0,  0.0),
    vec2( 0.5, -0.2),
    vec2(-0.4,  0.3),
    vec2( 0.1,  0.6),
    vec2(-0.7, -0.1),
    vec2( 0.7,  0.4),
    vec2(-0.1, -0.8),
    vec2( 0.3, -0.6)
);

bool rayIntersectsBox(vec3 boundsMin, vec3 boundsMax, vec3 origin, vec3 direction, float maxDistance)
{
    float tMin = 0.0;
    float tMax = maxDistance;

    for (int axis = 0; axis < 3; ++axis) {
        float originAxis = origin[axis];
        float directionAxis = direction[axis];
        float minAxis = boundsMin[axis];
        float maxAxis = boundsMax[axis];

        if (abs(directionAxis) < 1e-8) {
            if (originAxis < minAxis || originAxis > maxAxis) {
                return false;
            }
            continue;
        }

        float inverseDirection = 1.0 / directionAxis;
        float t0 = (minAxis - originAxis) * inverseDirection;
        float t1 = (maxAxis - originAxis) * inverseDirection;
        if (t0 > t1) {
            float tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        tMin = max(tMin, t0);
        tMax = min(tMax, t1);
        if (tMin > tMax) {
            return false;
        }
    }

    return tMax >= rtRayHitEpsilon;
}

bool rayTriangleIntersection(
    uint triangleIndex,
    vec3 origin,
    vec3 direction,
    float maxDistance,
    out float distance)
{
    if (triangleIndex >= uint(max(uRtTriangleCount, 0))) {
        return false;
    }

    vec3 v0 = triV0Edge[triangleIndex].xyz;
    vec3 v1 = triV1[triangleIndex].xyz;
    vec3 v2 = triV2[triangleIndex].xyz;
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 pvec = cross(direction, edge2);
    float det = dot(edge1, pvec);
    if (abs(det) < 1e-8) {
        return false;
    }

    float invDet = 1.0 / det;
    vec3 tvec = origin - v0;
    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0) {
        return false;
    }

    vec3 qvec = cross(tvec, edge1);
    float v = dot(direction, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }

    float t = dot(edge2, qvec) * invDet;
    if (t <= rtRayHitEpsilon || t > maxDistance) {
        return false;
    }

    distance = t;
    return true;
}

bool bvhAnyHit(vec3 origin, vec3 direction, float maxDistance)
{
    if (uRtBvhNodeCount <= 0 || uRtTriangleCount <= 0) {
        return false;
    }

    uint stack[rtBvhStackSize];
    int stackSize = 1;
    stack[0] = 0U;

    while (stackSize > 0) {
        --stackSize;
        uint nodeIndex = stack[stackSize];
        if (nodeIndex >= uint(uRtBvhNodeCount)) {
            continue;
        }

        if (!rayIntersectsBox(
                nodeMin[nodeIndex].xyz,
                nodeMax[nodeIndex].xyz,
                origin,
                direction,
                maxDistance)) {
            continue;
        }

        uvec4 meta = nodeMeta[nodeIndex];
        uint count = meta.w;
        if (count > 0U) {
            uint end = meta.z + count;
            for (uint indexOffset = meta.z; indexOffset < end; ++indexOffset) {
                if (indexOffset >= uint(max(uRtTriangleIndexCount, 0))) {
                    break;
                }

                float distance = 0.0;
                if (rayTriangleIntersection(
                        triangleIndices[indexOffset],
                        origin,
                        direction,
                        maxDistance,
                        distance)) {
                    return true;
                }
            }
        } else {
            if (stackSize + 2 > rtBvhStackSize) {
                return false;
            }
            stack[stackSize] = meta.x;
            ++stackSize;
            stack[stackSize] = meta.y;
            ++stackSize;
        }
    }

    return false;
}

float rtSunShadow(vec3 worldPos, vec3 geometricNormalWorld)
{
    vec3 normal = safeNormalize(geometricNormalWorld, vec3(0.0, 1.0, 0.0));
    vec3 direction = safeNormalize(uRtSunDirectionWorld, vec3(0.0, 1.0, 0.0));
    float maxDistance = (uRtShadowMaxDistance > 0.0) ? uRtShadowMaxDistance : 1.0e20;
    vec3 origin = worldPos + normal * uRtShadowNormalOffset;

    int sampleCount = clamp(uRtShadowSamples, 1, rtSunShadowSampleCount);
    if (sampleCount == 1 || uRtSunAngularRadius <= 0.0) {
        return bvhAnyHit(origin, direction, maxDistance) ? 0.0 : 1.0;
    }

    vec3 up = (abs(direction.y) < 0.95) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = safeNormalize(cross(up, direction), vec3(1.0, 0.0, 0.0));
    vec3 bitangent = cross(direction, tangent);

    float visibility = 0.0;
    for (int i = 0; i < rtSunShadowSampleCount; ++i) {
        if (i >= sampleCount) {
            break;
        }

        vec2 disk = rtSunShadowSamples[i] * uRtSunAngularRadius;
        vec3 sampleDirection = safeNormalize(
            direction + tangent * disk.x + bitangent * disk.y,
            direction);
        visibility += bvhAnyHit(origin, sampleDirection, maxDistance) ? 0.0 : 1.0;
    }

    return visibility / float(sampleCount);
}
#endif

vec3 faceOrientedNormal(vec3 normal)
{
    return (uDoubleSidedMaterial && !gl_FrontFacing) ? -normal : normal;
}

float indirectVisibilityScale()
{
#if OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
    if (!uHasVisBake) {
        return 1.0;
    }
    return mix(1.0, pow(clamp(vVisBake, 0.0, 1.0), uVisPower), uVisStrength);
#else
    return 1.0;
#endif
}

vec3 indirectIrradianceNormal(vec3 normalWorld, vec3 geometricNormalWorld)
{
#if OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
    if (!uHasVisBake || uVisBentStrength <= 0.0) {
        return normalWorld;
    }

    vec3 bentNormalWorld = safeNormalize(vBentNormalWorld, geometricNormalWorld);
    if (uDoubleSidedMaterial && !gl_FrontFacing) {
        bentNormalWorld = -bentNormalWorld;
    }

    float strength = clamp(uVisBentStrength, 0.0, 1.0);
    return safeNormalize(mix(normalWorld, bentNormalWorld, strength), normalWorld);
#else
    return normalWorld;
#endif
}

vec3 getNormal(vec3 geometricNormal)
{
#if OSG_GLTF_PBR_ENABLE_NORMAL_MAP
    if (!uHasNormalMap) {
        return geometricNormal;
    }

    vec3 tangent = safeNormalize(v_tangent, fallbackTangent(geometricNormal));
    vec3 bitangent = safeNormalize(v_bitangent, cross(geometricNormal, tangent));
    mat3 tbn = mat3(tangent, bitangent, geometricNormal);

    vec3 tangentNormal = texture(uNormalMap, v_texcoord0).xyz * 2.0 - 1.0;
    tangentNormal.xy *= uNormalScale;
    tangentNormal = safeNormalize(tangentNormal, vec3(0.0, 0.0, 1.0));
    return safeNormalize(tbn * tangentNormal, geometricNormal);
#else
    return geometricNormal;
#endif
}

void main()
{
    vec4 baseColor = uBaseColorFactor;
    if (uHasBaseColorMap) {
        baseColor *= texture(uBaseColorMap, v_texcoord0);
    }

    if (uAlphaMask && baseColor.a < uAlphaCutoff) {
        discard;
    }

#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
    if (uRadianceDebug) {
        osg_FragColor = vec4(max(vBakedIrradiance * uRadianceScale, vec3(0.0)), baseColor.a);
        osg_IndirectColor = vec4(0.0, 0.0, 0.0, baseColor.a);
        return;
    }
#endif

    float metallic = clamp(uMetallicFactor, 0.0, 1.0);
    float roughness = clamp(uRoughnessFactor, 0.04, 1.0);
    if (uHasMetallicRoughnessMap) {
        vec4 metallicRoughness = texture(uMetallicRoughnessMap, v_texcoord0);
        roughness = clamp(metallicRoughness.g * uRoughnessFactor, 0.04, 1.0);
        metallic = clamp(metallicRoughness.b * uMetallicFactor, 0.0, 1.0);
    }

    float ao = 1.0;
    if (uHasOcclusionMap) {
        float occlusion = texture(uOcclusionMap, v_texcoord0).r;
        ao = mix(1.0, occlusion, clamp(uOcclusionStrength, 0.0, 1.0));
    }

    vec3 emissive = uEmissiveFactor;
    if (uHasEmissiveMap) {
        emissive *= texture(uEmissiveMap, v_texcoord0).rgb;
    }

    vec3 albedo = baseColor.rgb;
    vec3 geometricNormal = faceOrientedNormal(safeNormalize(v_normal, vec3(0.0, 0.0, 1.0)));
    vec3 normal = getNormal(geometricNormal);
    vec3 viewDir = safeNormalize(-v_position, vec3(0.0, 0.0, 1.0));
#if OSG_GLTF_PBR_ENABLE_RT_SHADOWS
    vec3 worldPos = uRtViewOriginWorld + uViewToWorldRot * v_position;
#endif

    vec3 geometricNormalWorld = uViewToWorldRot * geometricNormal;
    geometricNormalWorld = (length(geometricNormalWorld) > 1e-6)
        ? normalize(geometricNormalWorld)
        : vec3(0.0, 1.0, 0.0);

    vec3 normalWorld = uViewToWorldRot * normal;
    normalWorld = (length(normalWorld) > 1e-6)
        ? normalize(normalWorld)
        : geometricNormalWorld;

    vec3 viewWorld = uViewToWorldRot * viewDir;
    viewWorld = (length(viewWorld) > 1e-6) ? normalize(viewWorld) : vec3(0.0, 0.0, 1.0);

    vec3 reflectionWorld = reflect(-viewWorld, normalWorld);
    reflectionWorld = (length(reflectionWorld) > 1e-6)
        ? normalize(reflectionWorld)
        : normalWorld;

    float indirectVisibility = indirectVisibilityScale();
    vec3 irradianceNormalWorld = indirectIrradianceNormal(normalWorld, geometricNormalWorld);
    vec3 flatAmbient = albedo * ao * osg_LightModel_ambient.rgb;
    vec3 ambient = flatAmbient;
    if (uHasEnv) {
        vec3 irradiance = uUseShIrradiance
            ? evaluateIrradianceSH(irradianceNormalWorld)
            : sampleEnvClamped(irradianceNormalWorld, uEnvMaxLod - 2.0);
        vec3 iblDiffuse = irradiance * albedo * (1.0 - metallic);
#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
        if (uUseRadianceBake) {
            iblDiffuse = vBakedIrradiance * uRadianceScale * albedo * (1.0 - metallic);
        }
#endif

        float glassReflection = max(uGlassReflectance, 0.0);
        float glassBlend = clamp(glassReflection, 0.0, 1.0);
        vec3 iblSpecular = vec3(0.0);
#if OSG_GLTF_PBR_ENABLE_IBL_SPECULAR
        float nDotV = max(dot(normal, viewDir), 1e-3);
        float regularSpecularLod = max(roughness * uEnvMaxLod, uEnvMaxLod * 0.35);
        float specularLod = mix(regularSpecularLod, roughness * uEnvMaxLod, glassBlend);
        vec3 prefiltered = sampleEnvClamped(reflectionWorld, specularLod);
        vec3 f0 = mix(vec3(0.04), albedo, metallic);
        vec2 ab = envBRDFApprox(roughness, nDotV);
        iblSpecular = prefiltered * (f0 * ab.x + ab.y);
        float roughSpecularFade = 1.0 - smoothstep(0.6, 0.8, roughness);
        iblSpecular *= mix(roughSpecularFade, 1.0, glassBlend);
#endif

        float roughMetal = metallic * smoothstep(0.35, 0.75, roughness);
        vec3 darkMetalFill = max(irradiance, osg_LightModel_ambient.rgb)
            * albedo
            * roughMetal
            * (0.35 * max(uDarkMetalLift, 0.0));

#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
        if (uUseRadianceBake) {
            ambient = (iblDiffuse * uIblDiffuse
                       + iblSpecular * mix(uIblSpecular, glassReflection, glassBlend) * indirectVisibility
                       + darkMetalFill) * ao;
            ambient *= uIblIntensity;
        } else {
            ambient = (iblDiffuse * uIblDiffuse
                       + iblSpecular * mix(uIblSpecular, glassReflection, glassBlend)
                       + darkMetalFill) * ao;
            ambient *= uIblIntensity * indirectVisibility;
        }
#else
        ambient = (iblDiffuse * uIblDiffuse
                   + iblSpecular * mix(uIblSpecular, glassReflection, glassBlend)
                   + darkMetalFill) * ao;
        ambient *= uIblIntensity * indirectVisibility;
#endif
        if (any(isnan(ambient)) || any(isinf(ambient))) {
            ambient = vec3(0.0);
        }
        ambient = max(ambient, vec3(0.0));
    }

#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
    if (!uUseRadianceBake) {
#endif
#if OSG_GLTF_PBR_ENABLE_VISIBILITY_BAKE
        vec3 bounceNormalWorld =
            (uHasVisBake && uVisBentStrength > 0.0) ? geometricNormalWorld : normalWorld;
#else
        vec3 bounceNormalWorld = normalWorld;
#endif
        vec3 bounce = uBounceRadiance * clamp(0.5 - 0.5 * bounceNormalWorld.y, 0.0, 1.0);
        ambient += bounce * albedo * (1.0 - metallic) * ao * indirectVisibility;
#if OSG_GLTF_PBR_ENABLE_RADIANCE_BAKE
    }
#endif
    ambient = max(ambient, vec3(0.0));

    vec3 indirectColor = ambient;
    vec3 directColor = emissive;

    if (osg_LightingEnabled) {
        vec3 lightDir;
        float attenuation = 1.0;
        if (osg_LightSource.position.w == 0.0) {
            lightDir = safeNormalize(transpose(uViewToWorldRot) * uRtSunDirectionWorld,
                                     safeNormalize(osg_LightSource.position.xyz, vec3(0.0, 0.0, 1.0)));
        } else {
            vec3 lightVector = osg_LightSource.position.xyz - v_position;
            float lightDistance = length(lightVector);
            lightDir = (lightDistance > 1e-5) ? lightVector / lightDistance : vec3(0.0, 0.0, 1.0);
            attenuation = 1.0 / max(osg_LightSource.constantAttenuation +
                                    osg_LightSource.linearAttenuation * lightDistance +
                                    osg_LightSource.quadraticAttenuation * lightDistance * lightDistance,
                                    1e-5);
            if (osg_LightSource.spotCutoff < 180.0) {
                float spotDot = dot(-lightDir, safeNormalize(osg_LightSource.spotDirection, vec3(0.0, 0.0, -1.0)));
                attenuation *= (spotDot >= osg_LightSource.spotCosCutoff) ? pow(spotDot, osg_LightSource.spotExponent) : 0.0;
            }
        }

        float nDotL = max(dot(normal, lightDir), 0.0);
        if (nDotL > 0.0) {
            vec3 diffuse = (1.0 - metallic) * albedo / PI;
            vec3 specular = vec3(0.0);
#if OSG_GLTF_PBR_ENABLE_DIRECT_SPECULAR
            {
                vec3 halfway = safeNormalize(viewDir + lightDir, normal);
                float nDotV = max(dot(normal, viewDir), 0.0);
                float hDotV = max(dot(halfway, viewDir), 0.0);

                vec3 f0 = mix(vec3(0.04), albedo, metallic);
                vec3 fresnel = fresnelSchlick(hDotV, f0);
                float distribution = distributionGGX(normal, halfway, roughness);
                float geometry = geometrySmith(normal, viewDir, lightDir, roughness);

                vec3 numerator = distribution * geometry * fresnel;
                float denominator = max(4.0 * nDotV * nDotL, 1e-3);
                specular = numerator / denominator;
                diffuse = (vec3(1.0) - fresnel) * (1.0 - metallic) * albedo / PI;
            }
#endif
            vec3 radiance = osg_LightSource.diffuse.rgb * attenuation;
            float shadow = 1.0;
#if OSG_GLTF_PBR_ENABLE_SHADOW_MAP || OSG_GLTF_PBR_ENABLE_RT_SHADOWS
            if (osg_LightSource.position.w == 0.0) {
#if OSG_GLTF_PBR_ENABLE_SHADOW_MAP && OSG_GLTF_PBR_ENABLE_RT_SHADOWS
                float rasterShadow = uHasShadow ? sunShadow(v_position, geometricNormal) : 1.0;
                if (!uUseRtSunShadow) {
                    shadow = rasterShadow;
                } else if (!uHasShadow) {
                    shadow = rtSunShadow(worldPos, geometricNormalWorld);
                } else if (rasterShadow <= uRtRasterGateMin || rasterShadow >= uRtRasterGateMax) {
                    shadow = rasterShadow;
                } else {
                    shadow = min(rasterShadow, rtSunShadow(worldPos, geometricNormalWorld));
                }
#elif OSG_GLTF_PBR_ENABLE_SHADOW_MAP
                shadow = uHasShadow ? sunShadow(v_position, geometricNormal) : 1.0;
#elif OSG_GLTF_PBR_ENABLE_RT_SHADOWS
                if (uUseRtSunShadow) {
                    shadow = rtSunShadow(worldPos, geometricNormalWorld);
                }
#endif
            }
#endif

#if OSG_GLTF_PBR_ENABLE_RT_SHADOWS
            if (uUseRtSunShadow && uRtShadowDebug) {
                directColor = mix(vec3(1.0, 0.08, 0.02), vec3(0.04, 0.75, 1.0), shadow)
                    * nDotL;
            } else {
                directColor += (diffuse + specular) * radiance * nDotL * shadow;
            }
#else
            directColor += (diffuse + specular) * radiance * nDotL * shadow;
#endif
        }
    }

    osg_FragColor = vec4(directColor
#if OSGSPONZA_MERGE_INDIRECT
                          + indirectColor
#endif
                          , baseColor.a);
#if OSGSPONZA_MERGE_INDIRECT
    osg_IndirectColor = vec4(0.0, 0.0, 0.0, baseColor.a);
#else
    osg_IndirectColor = vec4(indirectColor, baseColor.a);
#endif
}
)glsl";

}
