/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2018 Robert Osfield */

#pragma once

#include <limits>

namespace osg
{

    template<typename T>
    struct numbers
    {
            static constexpr T
            zero()
            {
                return static_cast<T>( 0.0 );
            }

            static constexpr T
            half()
            {
                return static_cast<T>( 0.5 );
            }

            static constexpr T
            one()
            {
                return static_cast<T>( 1.0 );
            }

            static constexpr T
            two()
            {
                return static_cast<T>( 2.0 );
            }

            static constexpr T
            three()
            {
                return static_cast<T>( 3.0 );
            }

            static constexpr T
            minus_one()
            {
                return static_cast<T>( -1.0 );
            }

            static constexpr T
            epsilon()
            {
                return std::numeric_limits<T>::epsilon();
            }

            static constexpr T
            PI()
            {
                return static_cast<T>( 3.14159265358979323846 );
            }

            static constexpr T
            degrees_to_radians()
            {
                return PI() / static_cast<T>( 180.0 );
            }

            static constexpr T
            radians_to_degrees()
            {
                return static_cast<T>( 180.0 ) / PI();
            }
    };

}    // namespace osg
