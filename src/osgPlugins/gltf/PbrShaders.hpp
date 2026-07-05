/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

namespace
{

    constexpr char pbrVertexShader[]   = R"glsl(
#version 460 core

layout(location = 0) in vec4 osg_Vertex;
layout(location = 2) in vec3 osg_Normal;
layout(location = 6) in vec4 osg_Tangent;
layout(location = 8) in vec4 osg_MultiTexCoord0;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform mat3 osg_NormalMatrix;

out vec3 v_position;
out vec3 v_normal;
out vec3 v_tangent;
out vec3 v_bitangent;
out vec2 v_texcoord0;

vec3 safeNormalize(vec3 value, vec3 fallback)
{
    float lengthSquared = dot(value, value);
    return (lengthSquared > 1e-10) ? value * inversesqrt(lengthSquared) : fallback;
}

vec3 fallbackTangent(vec3 normal)
{
    vec3 up = (abs(normal.z) < 0.999) ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    return safeNormalize(cross(up, normal), vec3(1.0, 0.0, 0.0));
}

void main()
{
    v_position = (osg_ModelViewMatrix * osg_Vertex).xyz;

    vec3 normal = safeNormalize(osg_NormalMatrix * osg_Normal, vec3(0.0, 0.0, 1.0));
    vec3 tangent = osg_NormalMatrix * osg_Tangent.xyz;
    tangent = tangent - normal * dot(normal, tangent);
    tangent = safeNormalize(tangent, fallbackTangent(normal));

    float handedness = (osg_Tangent.w < 0.0) ? -1.0 : 1.0;
    vec3 bitangent = safeNormalize(cross(normal, tangent), vec3(0.0, 1.0, 0.0)) * handedness;

    v_normal = normal;
    v_tangent = tangent;
    v_bitangent = bitangent;
    v_texcoord0 = osg_MultiTexCoord0.xy;

    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
}
)glsl";

    constexpr char pbrFragmentShader[] = R"glsl(
#version 460 core

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
uniform sampler2D uNormalMap;
uniform sampler2D uOcclusionMap;
uniform sampler2D uEmissiveMap;
uniform sampler2D uEnvMap;
layout(binding = 6) uniform sampler2DShadow uShadowMap;

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
uniform vec3 uIrradianceSH[9];
uniform vec3 uBounceRadiance;
uniform mat4 uShadowMatrix;
uniform float uShadowSoftness;
uniform float uShadowBias;
uniform float uShadowNormalOffset;
uniform mat3 uViewToWorldRot;

uniform bool uHasBaseColorMap;
uniform bool uHasMetallicRoughnessMap;
uniform bool uHasNormalMap;
uniform bool uHasOcclusionMap;
uniform bool uHasEmissiveMap;
uniform bool uAlphaMask;
uniform bool uHasEnv;
uniform bool uUseShIrradiance;
uniform bool uHasShadow;
uniform bool uDoubleSidedMaterial;

in vec3 v_position;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec2 v_texcoord0;

layout(location = 0) out vec4 osg_FragColor;
layout(location = 1) out vec4 osg_IndirectColor;

vec3 safeNormalize(vec3 value, vec3 fallback)
{
    float lengthSquared = dot(value, value);
    return (lengthSquared > 1e-10) ? value * inversesqrt(lengthSquared) : fallback;
}

vec3 fallbackTangent(vec3 normal)
{
    vec3 up = (abs(normal.z) < 0.999) ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    return safeNormalize(cross(up, normal), vec3(1.0, 0.0, 0.0));
}

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

vec2 envBRDFApprox(float rough, float ndv)
{
    const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = rough * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * ndv)) * r.x + r.y;
    return vec2(-1.04, 1.04) * a004 + r.zw;
}

const int shadowTapCount = 16;
const vec2 shadowPoisson[shadowTapCount] = vec2[shadowTapCount](
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
    float visibility = 0.0;
    for (int i = 0; i < shadowTapCount; ++i) {
        vec2 uv = sc.xy + rotation * shadowPoisson[i] * uShadowSoftness;
        visibility += texture(uShadowMap, vec3(uv, referenceDepth));
    }
    return visibility / float(shadowTapCount);
}

vec3 faceOrientedNormal(vec3 normal)
{
    return (uDoubleSidedMaterial && !gl_FrontFacing) ? -normal : normal;
}

vec3 getNormal(vec3 geometricNormal)
{
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

    vec3 flatAmbient = albedo * ao * osg_LightModel_ambient.rgb;
    vec3 ambient = flatAmbient;
    if (uHasEnv) {
        vec3 irradiance = uUseShIrradiance
            ? evaluateIrradianceSH(normalWorld)
            : sampleEnvClamped(normalWorld, uEnvMaxLod - 2.0);
        vec3 iblDiffuse = irradiance * albedo * (1.0 - metallic);

        float nDotV = max(dot(normal, viewDir), 1e-3);
        float glassReflection = max(uGlassReflectance, 0.0);
        float glassBlend = clamp(glassReflection, 0.0, 1.0);
        float regularSpecularLod = max(roughness * uEnvMaxLod, uEnvMaxLod * 0.35);
        float specularLod = mix(regularSpecularLod, roughness * uEnvMaxLod, glassBlend);
        vec3 prefiltered = sampleEnvClamped(reflectionWorld, specularLod);
        vec3 f0 = mix(vec3(0.04), albedo, metallic);
        vec2 ab = envBRDFApprox(roughness, nDotV);
        vec3 iblSpecular = prefiltered * (f0 * ab.x + ab.y);
        float roughSpecularFade = 1.0 - smoothstep(0.6, 0.8, roughness);
        iblSpecular *= mix(roughSpecularFade, 1.0, glassBlend);

        float roughMetal = metallic * smoothstep(0.35, 0.75, roughness);
        vec3 darkMetalFill = max(irradiance, osg_LightModel_ambient.rgb)
            * albedo
            * roughMetal
            * (0.35 * max(uDarkMetalLift, 0.0));

        ambient = (iblDiffuse * uIblDiffuse
                   + iblSpecular * mix(uIblSpecular, glassReflection, glassBlend)
                   + darkMetalFill) * ao;
        ambient *= uIblIntensity;
        if (any(isnan(ambient)) || any(isinf(ambient))) {
            ambient = vec3(0.0);
        }
        ambient = max(ambient, vec3(0.0));
    }

    vec3 bounce = uBounceRadiance * clamp(0.5 - 0.5 * normalWorld.y, 0.0, 1.0);
    ambient += bounce * albedo * (1.0 - metallic) * ao;
    ambient = max(ambient, vec3(0.0));

    vec3 indirectColor = ambient;
    vec3 directColor = emissive;

    if (osg_LightingEnabled) {
        vec3 lightDir;
        float attenuation = 1.0;
        if (osg_LightSource.position.w == 0.0) {
            lightDir = safeNormalize(osg_LightSource.position.xyz, vec3(0.0, 0.0, 1.0));
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
            vec3 halfway = safeNormalize(viewDir + lightDir, normal);
            float nDotV = max(dot(normal, viewDir), 0.0);
            float hDotV = max(dot(halfway, viewDir), 0.0);

            vec3 f0 = mix(vec3(0.04), albedo, metallic);
            vec3 fresnel = fresnelSchlick(hDotV, f0);
            float distribution = distributionGGX(normal, halfway, roughness);
            float geometry = geometrySmith(normal, viewDir, lightDir, roughness);

            vec3 numerator = distribution * geometry * fresnel;
            float denominator = max(4.0 * nDotV * nDotL, 1e-3);
            vec3 specular = numerator / denominator;

            vec3 diffuse = (vec3(1.0) - fresnel) * (1.0 - metallic) * albedo / PI;
            vec3 radiance = osg_LightSource.diffuse.rgb * attenuation;
            float shadow = (uHasShadow && osg_LightSource.position.w == 0.0)
                ? sunShadow(v_position, geometricNormal)
                : 1.0;
            directColor += (diffuse + specular) * radiance * nDotL * shadow;
        }
    }

    osg_FragColor = vec4(directColor, baseColor.a);
    osg_IndirectColor = vec4(indirectColor, baseColor.a);
}
)glsl";

}
