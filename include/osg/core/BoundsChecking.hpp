/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for clamping values within bounds.
 * Provides clampTo, clampBelow, clampAbove, and clampArrayElements.
 */
#pragma once

#include <osg/core/Notify.hpp>

namespace osg
{

    /** If value is greater than or equal to minValue do nothing - legal value,
     * Otherwise set value to minValue, and warn that valueName was clamped.
     * Note this is effectively A=max(A,B). */
    template<typename T>
    inline void
    clampGEQUAL( T&          value,
                 const T     minValue,
                 const char* valueName )
    {
        if( value < minValue )
        {
            notify( WARN ) << "Warning: " << valueName << " of " << value
                           << " is below permitted minimum, clamping to " << minValue
                           << "." << std::endl;
            value = minValue;
        }
    }

    /** If value is less than or equal to maxValue do nothing - legal value,
     * Otherwise set value to maxValue, and warn that valueName was clamped.
     * Note this is effectively A=min(A,B). */
    template<typename T>
    inline void
    clampLEQUAL( T&          value,
                 const T     maxValue,
                 const char* valueName )
    {
        if( value > maxValue )
        {
            notify( WARN ) << "Warning: " << valueName << " of " << value
                           << " is above permitted maximum, clamping to " << maxValue
                           << "." << std::endl;
            value = maxValue;
        }
    }

    /** If value is between or equal to minValue and maxValue do nothing - legal
     * value, Otherwise clamp value to specified range and warn that valueName
     * was clamped. Equivalent to calling
     *   clampGEQUAL( value, minValue, valueName );
     *   clampLEQUAL( value, maxValue, valueName ); */
    template<typename T>
    inline void
    clampBetweenRange( T&          value,
                       const T     minValue,
                       const T     maxValue,
                       const char* valueName )
    {
        if( value < minValue )
        {
            notify( WARN ) << "Warning: " << valueName << " of " << value
                           << " is below permitted minimum, clamping to " << minValue
                           << "." << std::endl;
            value = minValue;
        }
        else if( value > maxValue )
        {
            notify( WARN ) << "Warning: " << valueName << " of " << value
                           << " is above permitted maximum, clamping to " << maxValue
                           << "." << std::endl;
            value = maxValue;
        }
    }

    /** If value[i] is greater than or equal to minValue do nothing - legal value,
     * Otherwise set value[i] to minValue, and warn that valueName[i] was clamped.
     * Note this is effectively A[i]=max(A[i],B). */
    template<typename A,
             typename T>
    inline void
    clampArrayElementGEQUAL( A&           value,
                             unsigned int i,
                             const T      minValue,
                             const char*  valueName )
    {
        if( value[i] < minValue )
        {
            notify( WARN ) << "Warning: " << valueName << "[" << i << "] of " << value[i]
                           << " is below permitted minimum, clamping to " << minValue
                           << "." << std::endl;
            value[i] = minValue;
        }
    }

    /** If value[i] is less than or equal to maxValue do nothing - legal value,
     * Otherwise set value[i] to maxValue, and warn that valueName[i] was clamped.
     * Note this is effectively A[i]=min(A[i],B). */
    template<typename A,
             typename T>
    inline void
    clampArrayElementLEQUAL( A&           value,
                             unsigned int i,
                             const T      maxValue,
                             const char*  valueName )
    {
        if( value[i] > maxValue )
        {
            notify( WARN ) << "Warning: " << valueName << "[" << i << "] of " << value[i]
                           << " is above permitted maximum, clamping to " << maxValue
                           << "." << std::endl;
            value = maxValue;
        }
    }

    /** If value[i] is between or equal to minValue and maxValue do nothing - legal
     * value, Otherwise clamp value[i] to specified range and warn that
     * valueName[i] was clamped. Equivalent to calling
     *   clampArrayElementGEQUAL( value, i, minValue, valueName );
     *   clampArrayElementLEQUAL( value, i, maxValue, valueName ); */
    template<typename A,
             typename T>
    inline void
    clampArrayElementBetweenRange( A&           value,
                                   unsigned int i,
                                   const T      minValue,
                                   const T      maxValue,
                                   const char*  valueName )
    {
        if( value[i] < minValue )
        {
            notify( WARN ) << "Warning: " << valueName << "[" << i << "] of " << value[i]
                           << " is below permitted minimum, clamping to " << minValue
                           << "." << std::endl;
            value[i] = minValue;
        }
        else if( value[i] > maxValue )
        {
            notify( WARN ) << "Warning: " << valueName << "[" << i << "] of " << value[i]
                           << " is above permitted maximum, clamping to " << maxValue
                           << "." << std::endl;
            value[i] = maxValue;
        }
    }

    /** For each element of value[] in the range (first,last), if the element is
     * greater than or equal to minValue do nothing - legal value, Otherwise
     * clamp the element to minValue, and warn that valueName[i] was clamped. */
    template<typename A,
             typename T>
    inline void
    clampArrayElementsGEQUAL( A&           value,
                              unsigned int first,
                              unsigned int last,
                              const T      minValue,
                              const char*  valueName )
    {
        for( unsigned int i = first; i <= last; ++i )
        {
            clampArrayElementGEQUAL( value, i, minValue, valueName );
        }
    }

    /** For each element of value[] in the range (first,last), if the element is
     * less than or equal to maxValue do nothing - legal value, Otherwise clamp
     * the element to maxValue, and warn that valueName[i] was clamped. */
    template<typename A,
             typename T>
    inline void
    clampArrayElementsLEQUAL( A&           value,
                              unsigned int first,
                              unsigned int last,
                              const T      maxValue,
                              const char*  valueName )
    {
        for( unsigned int i = first; i <= last; ++i )
        {
            clampArrayElementLEQUAL( value, i, maxValue, valueName );
        }
    }

    /** For each element of value[] in the range (first,last), if the element is
     * between or equal to minValue and maxValue do nothing - legal value,
     * Otherwise clamp the element to the range and warn that valueName[i] was
     * clamped. Equivalent to calling
     *   clampArrayElementsGEQUAL( value, first, last, minValue, valueName);
     *   clampArrayElementsLEQUAL( value, first, last, maxValue, valueName); */
    template<typename A,
             typename T>
    inline void
    clampArrayElementsBetweenRange( A&           value,
                                    unsigned int first,
                                    unsigned int last,
                                    const T      minValue,
                                    const T      maxValue,
                                    const char*  valueName )
    {
        for( unsigned int i = first; i <= last; ++i )
        {
            clampArrayElementBetweenRange( value, i, minValue, maxValue, valueName );
        }
    }

    /** For each element of the three-element array value[], if the element is
     * greater than or equal to minValue do nothing - legal value, Otherwise
     * clamp the element to minValue, and warn that valueName[i] was clamped. */
    template<typename A,
             typename T>
    inline void
    clampArray3GEQUAL( A&          value,
                       const T     minValue,
                       const char* valueName )
    {
        clampArrayElementsGEQUAL( value, 0U, 2U, minValue, valueName );
    }

    /** For each element of the three-element array value[], if the element is
     * less than or equal to maxValue do nothing - legal value, Otherwise clamp
     * the element to maxValue, and warn that valueName[i] was clamped. */
    template<typename A,
             typename T>
    inline void
    clampArray3LEQUAL( A&          value,
                       const T     maxValue,
                       const char* valueName )
    {
        clampArrayElementsLEQUAL( value, 0U, 2U, maxValue, valueName );
    }

    /** For each element of the three-element array value[], if the element is
     * between or equal to minValue and maxValue do nothing - legal value,
     * Otherwise clamp the element to the range and warn that valueName[i] was
     * clamped. Equivalent to calling
     *   clampArray3GEQUAL( value, minValue, valueName);
     *   clampArray3LEQUAL( value, maxValue, valueName); */
    template<typename A,
             typename T>
    inline void
    clampArray3BetweenRange( A&          value,
                             const T     minValue,
                             const T     maxValue,
                             const char* valueName )
    {
        clampArrayElementsBetweenRange( value, 0U, 2U, minValue, maxValue, valueName );
    }

    /** For each element of the four-element array value[], if the element is
     * greater than or equal to minValue do nothing - legal value, Otherwise
     * clamp the element to minValue, and warn that valueName[i] was clamped. */
    template<typename A,
             typename T>
    inline void
    clampArray4GEQUAL( A&          value,
                       const T     minValue,
                       const char* valueName )
    {
        clampArrayElementsGEQUAL( value, 0U, 3U, minValue, valueName );
    }

    /** For each element of the four-element array value[], if the element is
     * less than or equal to maxValue do nothing - legal value, Otherwise clamp
     * the element to maxValue, and warn that valueName[i] was clamped. */
    template<typename A,
             typename T>
    inline void
    clampArray4LEQUAL( A&          value,
                       const T     maxValue,
                       const char* valueName )
    {
        clampArrayElementsLEQUAL( value, 0U, 3U, maxValue, valueName );
    }

    /** For each element of the four-element array value[], if the element is
     * between or equal to minValue and maxValue do nothing - legal value,
     * Otherwise clamp the element to the range and warn that valueName[i] was
     * clamped. Equivalent to calling
     *   clampArray4GEQUAL( value, minValue, valueName);
     *   clampArray4LEQUAL( value, maxValue, valueName); */
    template<typename A,
             typename T>
    inline void
    clampArray4BetweenRange( A&          value,
                             const T     minValue,
                             const T     maxValue,
                             const char* valueName )
    {
        clampArrayElementsBetweenRange( value, 0U, 3U, minValue, maxValue, valueName );
    }

}
