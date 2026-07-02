/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Min/max range template for particle property randomization.
 * Stores minimum and maximum values with random sampling.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

// include Export simply to disable Visual Studio silly warnings.
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osgParticle/Export.hpp>
#include <stdlib.h>

namespace osgParticle
{

    /**
        A simple struct template useful to store ranges of values as min/max pairs.
        This struct template helps storing min/max ranges for values of any kind; class
       <CODE>ValueType</CODE> is the type of values to be stored, and it must support
       operations <CODE>ValueType + ValueType</CODE>, <CODE>ValueType - ValueType</CODE>,
        and <CODE>ValueType * float</CODE>, otherwise the
       <CODE>geValueTyperandom()</CODE> method will not compile. This struct could be
       extended to customize the random number generator (now it uses only
        <CODE>std::rand()</CODE>).
    */
    template<class ValueType>
    struct range
    {

            /// Lower bound.
            ValueType minimum;

            /// Higher bound.
            ValueType maximum;

            /// Construct the object by calling default constructors for min and max.
            range() :
                minimum( ValueType() ),
                maximum( ValueType() )
            {
            }

            /// Construct and initialize min and max directly.
            range( const ValueType& mn,
                   const ValueType& mx ) :
                minimum( mn ),
                maximum( mx )
            {
            }

            /// Set min and max.
            void
            set( const ValueType& mn,
                 const ValueType& mx )
            {
                minimum = mn;
                maximum = mx;
            }

            /// Get a random value between min and max.
            ValueType
            get_random() const
            {
                return minimum +
                       ( maximum - minimum ) *
                       ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) );
            }

            /// Get a random square root value between min and max.
            ValueType
            get_random_sqrtf() const
            {
                return minimum +
                       ( maximum - minimum ) *
                       sqrtf( static_cast<float>( rand() ) /
                              static_cast<float>( RAND_MAX ) );
            }

            ValueType
            mid() const
            {
                return ( minimum + maximum ) * 0.5F;
            }
    };

    /// Range of floats.
    typedef range<float>     rangef;

    /// Range of osg::svec2.
    typedef range<osg::vec2> rangev2;

    /// Range of osg::svec3.
    typedef range<osg::vec3> rangev3;

    /// Range of osg::svec4.
    typedef range<osg::vec4> rangev4;

}
