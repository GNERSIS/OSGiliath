/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 4D byte vector for compact RGBA data.
 * 8-bit signed components for compressed attributes.
 */
#pragma once

namespace osg
{

    /** General purpose float triple.
     * Uses include representation of color coordinates.
     * No support yet added for float * bvec4 - is it necessary?
     * Need to define a non-member non-friend operator*  etc.
     * bvec4 * float is okay
     */
    class bvec4
    {
        public:

            /** Data type of vector components.*/
            typedef signed char value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 4,
            };

            value_type _v[4];

            /** Constructor that sets all components of the vector to zero */
            bvec4()
            {
                _v[0] = 0;
                _v[1] = 0;
                _v[2] = 0;
                _v[3] = 0;
            }

            bvec4( value_type x,
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
            operator==( const bvec4& v ) const
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
            operator!=( const bvec4& v ) const
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
            operator<( const bvec4& v ) const
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
            inline bvec4
            operator*( float rhs ) const
            {
                bvec4 col( *this );
                col *= rhs;
                return col;
            }

            /** Unary multiply by scalar. */
            inline bvec4&
            operator*=( float rhs )
            {
                _v[0] = ( value_type )( ( float )_v[0] * rhs );
                _v[1] = ( value_type )( ( float )_v[1] * rhs );
                _v[2] = ( value_type )( ( float )_v[2] * rhs );
                _v[3] = ( value_type )( ( float )_v[3] * rhs );
                return *this;
            }

            /** Divide by scalar. */
            inline bvec4
            operator/( float rhs ) const
            {
                bvec4 col( *this );
                col /= rhs;
                return col;
            }

            /** Unary divide by scalar. */
            inline bvec4&
            operator/=( float rhs )
            {
                float div  = 1.0F / rhs;
                *this     *= div;
                return *this;
            }

            /** Binary vector add. */
            inline bvec4
            operator+( const bvec4& rhs ) const
            {
                return bvec4( _v[0] + rhs._v[0],
                              _v[1] + rhs._v[1],
                              _v[2] + rhs._v[2],
                              _v[3] + rhs._v[3] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            inline bvec4&
            operator+=( const bvec4& rhs )
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                _v[2] += rhs._v[2];
                _v[3] += rhs._v[3];
                return *this;
            }

            /** Binary vector subtract. */
            inline bvec4
            operator-( const bvec4& rhs ) const
            {
                return bvec4( _v[0] - rhs._v[0],
                              _v[1] - rhs._v[1],
                              _v[2] - rhs._v[2],
                              _v[3] - rhs._v[3] );
            }

            /** Unary vector subtract. */
            inline bvec4&
            operator-=( const bvec4& rhs )
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                _v[2] -= rhs._v[2];
                _v[3] -= rhs._v[3];
                return *this;
            }

    };    // end of class bvec4

}    // end of namespace osg
