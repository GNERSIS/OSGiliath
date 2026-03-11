/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convenience include for math utilities. Pulls in numeric constants,
 * clamp functions, and degrees/radians conversion.
 */
#pragma once

#include <cfloat>
#include <cmath>
#include <osg/core/Export.hpp>
#include <osg/maths/common.hpp>
#include <osg/maths/vec3.hpp>
#include <type_traits>

namespace osg
{

    // PI and PIf are now defined in <osg/maths/common.hpp>
    // Additional constants not in common.h:
    constexpr double PI_2     = PI * 0.5;
    constexpr double PI_4     = PI * 0.25;
    constexpr double LN_2     = 0.69314718055994530942;
    constexpr double INVLN_2  = 1.0 / LN_2;

    constexpr float  PI_2f    = PIf * 0.5F;
    constexpr float  PI_4f    = PIf * 0.25F;
    constexpr float  LN_2f    = 0.69314718055994530942F;
    constexpr float  INVLN_2f = 1.0F / LN_2f;

    template<typename T>
    inline T
    default_value()
    {
        return T();
    }

    template<>
    inline float
    default_value<float>()
    {
        return 0.0F;
    }

    template<>
    inline double
    default_value<double>()
    {
        return 0.0;
    }

    template<>
    inline char
    default_value<char>()
    {
        return 0;
    }

    template<>
    inline unsigned char
    default_value<unsigned char>()
    {
        return 0;
    }

    template<>
    inline short
    default_value<short>()
    {
        return 0;
    }

    template<>
    inline unsigned short
    default_value<unsigned short>()
    {
        return 0;
    }

    template<>
    inline int
    default_value<int>()
    {
        return 0;
    }

    template<>
    inline unsigned int
    default_value<unsigned int>()
    {
        return 0;
    }

    template<typename T>
    inline T
    absolute( T v )
    {
        return v < ( T )0 ? -v : v;
    }

    /** return true if float lhs and rhs are equivalent,
     * meaning that the difference between them is less than an epsilon value
     * which defaults to 1e-6.
     */
    inline bool
    equivalent( float lhs,
                float rhs,
                float epsilon = 1E-6F )
    {
        float delta = rhs - lhs;
        return delta < 0.0F ? delta >= -epsilon : delta <= epsilon;
    }

    /** return true if double lhs and rhs are equivalent,
     * meaning that the difference between them is less than an epsilon value
     * which defaults to 1e-6.
     */
    inline bool
    equivalent( double lhs,
                double rhs,
                double epsilon = 1E-6 )
    {
        double delta = rhs - lhs;
        return delta < 0.0 ? delta >= -epsilon : delta <= epsilon;
    }

    /** return the minimum of two values, equivalent to std::min.
     * std::min not used because of STL implementation under IRIX not containing
     * std::min.
     */
    template<typename T>
    inline T
    minimum( T lhs,
             T rhs )
    {
        return lhs < rhs ? lhs : rhs;
    }

    /** return the maximum of two values, equivalent to std::max.
     * std::max not used because of STL implementation under IRIX not containing
     * std::max.
     */
    template<typename T>
    inline T
    maximum( T lhs,
             T rhs )
    {
        return lhs > rhs ? lhs : rhs;
    }

    template<typename T>
    inline T
    clampTo( T v,
             T minimum,
             T maximum )
    {
        return v < minimum ? minimum : v > maximum ? maximum : v;
    }

    template<typename T>
    inline T
    clampAbove( T v,
                T minimum )
    {
        return v < minimum ? minimum : v;
    }

    template<typename T>
    inline T
    clampBelow( T v,
                T maximum )
    {
        return v > maximum ? maximum : v;
    }

    template<typename T>
    inline T
    clampBetween( T v,
                  T minimum,
                  T maximum )
    {
        return clampBelow( clampAbove( v, minimum ), maximum );
    }

    template<typename T>
    inline T
    sign( T v )
    {
        return v < ( T )0 ? ( T )-1 : ( T )1;
    }

    template<typename T>
    inline T
    signOrZero( T v )
    {
        return v < ( T )0 ? ( T )-1 : ( v > ( T )0 ? ( T )1 : 0 );
    }

    // square() is now defined in <osg/maths/common.hpp> for float and double.
    // This generic template handles other types.
    template<typename T,
             std::enable_if_t<!std::is_floating_point_v<T>,
                              int> = 0>
    inline T
    square( T v )
    {
        return v * v;
    }

    template<typename T>
    inline T
    signedSquare( T v )
    {
        return v < ( T )0 ? -v * v : v * v;
        ;
    }

    inline float
    inDegrees( float angle )
    {
        return angle * ( float )PI / 180.0F;
    }

    inline double
    inDegrees( double angle )
    {
        return angle * PI / 180.0;
    }

    template<typename T>
    inline T
    inRadians( T angle )
    {
        return angle;
    }

    inline float
    DegreesToRadians( float angle )
    {
        return angle * ( float )PI / 180.0F;
    }

    inline double
    DegreesToRadians( double angle )
    {
        return angle * PI / 180.0;
    }

    inline float
    RadiansToDegrees( float angle )
    {
        return angle * 180.0F / ( float )PI;
    }

    inline double
    RadiansToDegrees( double angle )
    {
        return angle * 180.0 / PI;
    }

    inline float
    round( float v )
    {
        return v >= 0.0F ? floorf( v + 0.5F ) : ceilf( v - 0.5F );
    }

    inline double
    round( double v )
    {
        return v >= 0.0 ? floor( v + 0.5 ) : ceil( v - 0.5 );
    }

#if defined( _MSC_VER )
    inline bool
    isNaN( double v )
    {
        return _isnan( v ) != 0;
    }
#elif defined( __ANDROID__ )
    inline bool
    isNaN( float v )
    {
        return isnan( v );
    }

    inline bool
    isNaN( double v )
    {
        return isnan( v );
    }
#else
    inline bool
    isNaN( float v )
    {
        return std::isnan( v );
    }

    inline bool
    isNaN( double v )
    {
        return std::isnan( v );
    }
#endif

    /** compute the volume of a tetrahedron. */
    template<typename T>
    inline float
    computeVolume( const T& a,
                   const T& b,
                   const T& c,
                   const T& d )
    {
        return fabsf( dot( cross( b - c, a - b ), d - b ) );
    }

    /** compute the volume of a prism. */
    template<typename T>
    inline float
    computeVolume( const T& f1,
                   const T& f2,
                   const T& f3,
                   const T& b1,
                   const T& b2,
                   const T& b3 )
    {
        return computeVolume( f1, f2, f3, b1 ) +
               computeVolume( b1, b2, b3, f2 ) +
               computeVolume( b1, b3, f2, f3 );
    }

    /** Convert a ascii number to a double, ignoring locale settings.*/
    extern OSG_EXPORT double
    asciiToDouble( const char* str );

    /** Convert a ascii number to a float, ignoring locale settings.*/
    inline float
    asciiToFloat( const char* str )
    {
        return static_cast<float>( asciiToDouble( str ) );
    }

    /** Detect first ascii POSITIVE number in string and convert to double.*/
    extern OSG_EXPORT double
    findAsciiToDouble( const char* str );

    /** Detect first ascii POSITIVE number in string and convert to double.*/
    inline float
    findAsciiToFloat( const char* str )
    {
        return static_cast<float>( findAsciiToDouble( str ) );
    }

}
