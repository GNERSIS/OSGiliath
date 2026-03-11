/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2022 Robert Osfield */

#pragma once

#include <cmath>
#include <osg/maths/color.hpp>

namespace osg
{

    /// clamp value between 0 and 1, implementing clamp-to-edge behavior
    template<typename T>
    constexpr T
    clamp_to_edge( T value )
    {
        return value <= T( 0.0 ) ? T( 0.0 ) : value >= T( 1.0 ) ? T( 1.0 ) : value;
    }

    /// clamp value between 0 and 1, implementing repeat behavior
    template<typename T>
    constexpr T
    repeat( T value )
    {
        T result = value - std::floor( value );
        if( result != T( 0.0 ) )
        {
            return result;
        }
        return ( value > T( 0.0 ) ) ? T( 1.0 ) : T( 0.0 );
    }

    /// clamp value between 0 and 1, implementing mirror repeat behavior
    template<typename T>
    constexpr T
    mirror_repeat( T value )
    {
        T half_value = ( std::abs( value ) * T( 0.5 ) );
        T v_fract    = half_value - std::floor( half_value );
        return T( 1.0 ) - std::abs( T( 1.0 ) - v_fract * T( 2.0 ) );
    }

}    // namespace osg
