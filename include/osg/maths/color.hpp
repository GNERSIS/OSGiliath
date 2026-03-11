/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2022 Robert Osfield */

#pragma once

#include <osg/maths/vec4.hpp>

namespace osg
{

    template<typename T>
    constexpr T
    color_cast( float r,
                float g,
                float b,
                float a )
    {
        return { r, g, b, a };
    }

    template<>
    constexpr float
    color_cast<float>( float r,
                       float,
                       float,
                       float )
    {
        return r;
    }

    template<>
    constexpr vec2
    color_cast<vec2>( float r,
                      float g,
                      float,
                      float )
    {
        return { r, g };
    }

    template<>
    constexpr vec3
    color_cast<vec3>( float r,
                      float g,
                      float b,
                      float )
    {
        return { r, g, b };
    }

    template<>
    constexpr vec4
    color_cast<vec4>( float r,
                      float g,
                      float b,
                      float a )
    {
        return { r, g, b, a };
    }

    template<>
    constexpr ubvec4
    color_cast<ubvec4>( float r,
                        float g,
                        float b,
                        float a )
    {
        return {
            static_cast<uint8_t>( r * 255.0F ),
            static_cast<uint8_t>( g * 255.0F ),
            static_cast<uint8_t>( b * 255.0F ),
            static_cast<uint8_t>( a * 255.0F )
        };
    }

    template<typename T>
    constexpr T
    transparent_black()
    {
        return color_cast<T>( 0.0F, 0.0F, 0.0F, 0.0F );
    }

    template<typename T>
    constexpr T
    opaque_black()
    {
        return color_cast<T>( 0.0F, 0.0F, 0.0F, 1.0F );
    }

    template<typename T>
    constexpr T
    transparent_white()
    {
        return color_cast<T>( 1.0F, 1.0F, 1.0F, 0.0F );
    }

    template<typename T>
    constexpr T
    opaque_white()
    {
        return color_cast<T>( 1.0F, 1.0F, 1.0F, 1.0F );
    }

}    // namespace osg
