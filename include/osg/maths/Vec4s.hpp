/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 4D short vector for compact per-vertex data.
 * 16-bit signed components for reduced-precision attributes.
 */
#pragma once

namespace osg
{

    /** General purpose float quad. Uses include representation
     * of color coordinates.
     * No support yet added for float * vec4 - is it necessary?
     * Need to define a non-member non-friend operator*  etc.
     *    vec4 * float is okay
     */
    class svec4
    {
        public:

            /** Data type of vector components.*/
            typedef short value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 4,
            };

            /** Vec member variable. */
            value_type _v[4];

            /** Constructor that sets all components of the vector to zero */
            svec4()
            {
                _v[0] = 0;
                _v[1] = 0;
                _v[2] = 0;
                _v[3] = 0;
            }

            svec4( value_type x,
                   value_type y,
                   value_type z,
                   value_type w )
            {
                _v[0] = x;
                _v[1] = y;
                _v[2] = z;
                _v[3] = w;
            }

            inline bool
            operator==( const svec4& v ) const
            {
                return _v[0] ==
                       v._v[0] &&
                       _v[1] ==
                       v._v[1] &&
                       _v[2] ==
                       v._v[2] &&
                       _v[3] == v._v[3];
            }

            inline bool
            operator!=( const svec4& v ) const
            {
                return _v[0] !=
                       v._v[0] ||
                       _v[1] !=
                       v._v[1] ||
                       _v[2] !=
                       v._v[2] ||
                       _v[3] != v._v[3];
            }

            inline bool
            operator<( const svec4& v ) const
            {
                if( _v[0] < v._v[0] )
                {
                    return true;
                }
                else if( _v[0] > v._v[0] )
                {
                    return false;
                }
                else if( _v[1] < v._v[1] )
                {
                    return true;
                }
                else if( _v[1] > v._v[1] )
                {
                    return false;
                }
                else if( _v[2] < v._v[2] )
                {
                    return true;
                }
                else if( _v[2] > v._v[2] )
                {
                    return false;
                }
                else
                {
                    return ( _v[3] < v._v[3] );
                }
            }

            inline value_type*
            ptr()
            {
                return _v;
            }

            inline const value_type*
            ptr() const
            {
                return _v;
            }

            inline void
            set( value_type x,
                 value_type y,
                 value_type z,
                 value_type w )
            {
                _v[0] = x;
                _v[1] = y;
                _v[2] = z;
                _v[3] = w;
            }

            inline value_type&
            operator[]( unsigned int i )
            {
                return _v[i];
            }

            inline value_type
            operator[]( unsigned int i ) const
            {
                return _v[i];
            }

            inline value_type&
            x()
            {
                return _v[0];
            }

            inline value_type&
            y()
            {
                return _v[1];
            }

            inline value_type&
            z()
            {
                return _v[2];
            }

            inline value_type&
            w()
            {
                return _v[3];
            }

            inline value_type
            x() const
            {
                return _v[0];
            }

            inline value_type
            y() const
            {
                return _v[1];
            }

            inline value_type
            z() const
            {
                return _v[2];
            }

            inline value_type
            w() const
            {
                return _v[3];
            }

            inline value_type&
            r()
            {
                return _v[0];
            }

            inline value_type&
            g()
            {
                return _v[1];
            }

            inline value_type&
            b()
            {
                return _v[2];
            }

            inline value_type&
            a()
            {
                return _v[3];
            }

            inline value_type
            r() const
            {
                return _v[0];
            }

            inline value_type
            g() const
            {
                return _v[1];
            }

            inline value_type
            b() const
            {
                return _v[2];
            }

            inline value_type
            a() const
            {
                return _v[3];
            }

            /** Multiply by scalar. */
            inline svec4
            operator*( value_type rhs ) const
            {
                return svec4( _v[0] * rhs, _v[1] * rhs, _v[2] * rhs, _v[3] * rhs );
            }

            /** Unary multiply by scalar. */
            inline svec4&
            operator*=( value_type rhs )
            {
                _v[0] *= rhs;
                _v[1] *= rhs;
                _v[2] *= rhs;
                _v[3] *= rhs;
                return *this;
            }

            /** Divide by scalar. */
            inline svec4
            operator/( value_type rhs ) const
            {
                return svec4( _v[0] / rhs, _v[1] / rhs, _v[2] / rhs, _v[3] / rhs );
            }

            /** Unary divide by scalar. */
            inline svec4&
            operator/=( value_type rhs )
            {
                _v[0] /= rhs;
                _v[1] /= rhs;
                _v[2] /= rhs;
                _v[3] /= rhs;
                return *this;
            }

            /** Binary vector multiply. */
            inline svec4
            operator*( const svec4& rhs ) const
            {
                return svec4( _v[0] * rhs._v[0],
                              _v[1] * rhs._v[1],
                              _v[2] * rhs._v[2],
                              _v[3] * rhs._v[3] );
            }

            /** Binary vector add. */
            inline svec4
            operator+( const svec4& rhs ) const
            {
                return svec4( _v[0] + rhs._v[0],
                              _v[1] + rhs._v[1],
                              _v[2] + rhs._v[2],
                              _v[3] + rhs._v[3] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            inline svec4&
            operator+=( const svec4& rhs )
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                _v[2] += rhs._v[2];
                _v[3] += rhs._v[3];
                return *this;
            }

            /** Binary vector subtract. */
            inline svec4
            operator-( const svec4& rhs ) const
            {
                return svec4( _v[0] - rhs._v[0],
                              _v[1] - rhs._v[1],
                              _v[2] - rhs._v[2],
                              _v[3] - rhs._v[3] );
            }

            /** Unary vector subtract. */
            inline svec4&
            operator-=( const svec4& rhs )
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                _v[2] -= rhs._v[2];
                _v[3] -= rhs._v[3];
                return *this;
            }

            /** Negation operator. Returns the negative of the svec4. */
            inline svec4
            operator-() const
            {
                return svec4( -_v[0], -_v[1], -_v[2], -_v[3] );
            }

    };    // end of class svec4

    /** multiply by vector components. */
    inline svec4
    componentMultiply( const svec4& lhs,
                       const svec4& rhs )
    {
        return svec4( lhs[0] * rhs[0],
                      lhs[1] * rhs[1],
                      lhs[2] * rhs[2],
                      lhs[3] * rhs[3] );
    }

    /** divide rhs components by rhs vector components. */
    inline svec4
    componentDivide( const svec4& lhs,
                     const svec4& rhs )
    {
        return svec4( lhs[0] / rhs[0],
                      lhs[1] / rhs[1],
                      lhs[2] / rhs[2],
                      lhs[3] / rhs[3] );
    }

}    // end of namespace osg
