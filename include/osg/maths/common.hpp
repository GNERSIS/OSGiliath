/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2018 Robert Osfield */

#pragma once

#include <cmath>
#include <osg/maths/mat3.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec3.hpp>

namespace osg
{

    constexpr float  PIf = numbers<float>::PI();
    constexpr double PI  = numbers<double>::PI();

    /// convert degrees to radians
    constexpr float
    radians( float degrees ) noexcept
    {
        return degrees * numbers<float>::degrees_to_radians();
    }

    constexpr double
    radians( double degrees ) noexcept
    {
        return degrees * numbers<double>::degrees_to_radians();
    }

    /// convert radians to degrees
    constexpr float
    degrees( float radians ) noexcept
    {
        return radians * numbers<float>::radians_to_degrees();
    }

    constexpr double
    degrees( double radians ) noexcept
    {
        return radians * numbers<double>::radians_to_degrees();
    }

    /// compute value^2
    constexpr float
    square( float v ) noexcept
    {
        return v * v;
    };

    constexpr double
    square( double v ) noexcept
    {
        return v * v;
    };

    /// Hermite interpolation between edge0 and edge1
    template<typename T>
    T
    smoothstep( T edge0,
                T edge1,
                T x )
    {
        if( x <= edge0 )
        {
            return edge0;
        }
        else if( x >= edge1 )
        {
            return edge1;
        }
        T r = ( x - edge0 ) / ( edge1 - edge0 );
        return edge0 +
               ( r * r * ( numbers<T>::three() - numbers<T>::two() * r ) ) *
               ( edge1 - edge0 );
    }

    /// Hermite interpolation between 0.0 and 1.0
    template<typename T>
    T
    smoothstep( T r )
    {
        if( r <= numbers<T>::zero() )
        {
            return numbers<T>::zero();
        }
        else if( r >= numbers<T>::one() )
        {
            return numbers<T>::one();
        }
        return r * r * ( numbers<T>::three() - numbers<T>::two() * r );
    }

    /// interpolate between two values
    template<typename T>
    T
    mix( T start,
         T end,
         T r )
    {
        T one_minus_r = numbers<T>::one() - r;
        return start * one_minus_r + end * r;
    }

}    // namespace osg
