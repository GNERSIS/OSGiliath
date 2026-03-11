/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 4D unsigned short vector for compact per-vertex data.
 * 16-bit unsigned components for reduced-precision attributes.
 */
#pragma once

namespace osg
{

    /** General purpose float quad. Uses include representation
     * of color coordinates.
     * No support yet added for float * usvec4 - is it necessary?
     * Need to define a non-member non-friend operator*  etc.
     *    usvec4 * float is okay
     */
    class usvec4
    {
        public:

            /** Data type of vector components.*/
            typedef unsigned short value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 4,
            };

            /** Vec member variable. */
            value_type _v[4];

            /** Constructor that sets all components of the vector to zero */
            usvec4()
            {
                _v[0] = 0;
                _v[1] = 0;
                _v[2] = 0;
                _v[3] = 0;
            }

            usvec4( value_type x,
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
            operator==( const usvec4& v ) const
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
            operator!=( const usvec4& v ) const
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
            operator<( const usvec4& v ) const
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
            inline usvec4
            operator*( value_type rhs ) const
            {
                return usvec4( _v[0] * rhs, _v[1] * rhs, _v[2] * rhs, _v[3] * rhs );
            }

            /** Unary multiply by scalar. */
            inline usvec4&
            operator*=( value_type rhs )
            {
                _v[0] *= rhs;
                _v[1] *= rhs;
                _v[2] *= rhs;
                _v[3] *= rhs;
                return *this;
            }

            /** Divide by scalar. */
            inline usvec4
            operator/( value_type rhs ) const
            {
                return usvec4( _v[0] / rhs, _v[1] / rhs, _v[2] / rhs, _v[3] / rhs );
            }

            /** Unary divide by scalar. */
            inline usvec4&
            operator/=( value_type rhs )
            {
                _v[0] /= rhs;
                _v[1] /= rhs;
                _v[2] /= rhs;
                _v[3] /= rhs;
                return *this;
            }

            /** Binary vector multiply. */
            inline usvec4
            operator*( const usvec4& rhs ) const
            {
                return usvec4( _v[0] * rhs._v[0],
                               _v[1] * rhs._v[1],
                               _v[2] * rhs._v[2],
                               _v[3] * rhs._v[3] );
            }

            /** Binary vector add. */
            inline usvec4
            operator+( const usvec4& rhs ) const
            {
                return usvec4( _v[0] + rhs._v[0],
                               _v[1] + rhs._v[1],
                               _v[2] + rhs._v[2],
                               _v[3] + rhs._v[3] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            inline usvec4&
            operator+=( const usvec4& rhs )
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                _v[2] += rhs._v[2];
                _v[3] += rhs._v[3];
                return *this;
            }

            /** Binary vector subtract. */
            inline usvec4
            operator-( const usvec4& rhs ) const
            {
                return usvec4( _v[0] - rhs._v[0],
                               _v[1] - rhs._v[1],
                               _v[2] - rhs._v[2],
                               _v[3] - rhs._v[3] );
            }

            /** Unary vector subtract. */
            inline usvec4&
            operator-=( const usvec4& rhs )
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                _v[2] -= rhs._v[2];
                _v[3] -= rhs._v[3];
                return *this;
            }

    };    // end of class usvec4

    /** multiply by vector components. */
    inline usvec4
    componentMultiply( const usvec4& lhs,
                       const usvec4& rhs )
    {
        return usvec4( lhs[0] * rhs[0],
                       lhs[1] * rhs[1],
                       lhs[2] * rhs[2],
                       lhs[3] * rhs[3] );
    }

    /** divide rhs components by rhs vector components. */
    inline usvec4
    componentDivide( const usvec4& lhs,
                     const usvec4& rhs )
    {
        return usvec4( lhs[0] / rhs[0],
                       lhs[1] / rhs[1],
                       lhs[2] / rhs[2],
                       lhs[3] / rhs[3] );
    }

}    // end of namespace osg
