/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 3D unsigned byte vector for compact vertex colors.
 * 8-bit unsigned components (0-255 range).
 */
#pragma once

namespace osg
{

    /** General purpose float triple.
     * Uses include representation of color coordinates.
     * No support yet added for float * ubvec3 - is it necessary?
     * Need to define a non-member non-friend operator*  etc.
     * ubvec3 * float is okay
     */
    class ubvec3
    {
        public:

            /** Data type of vector components.*/
            typedef unsigned char value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 3,
            };

            value_type _v[3];

            /** Constructor that sets all components of the vector to zero */
            ubvec3()
            {
                _v[0] = 0;
                _v[1] = 0;
                _v[2] = 0;
            }

            ubvec3( value_type r,
                    value_type g,
                    value_type b )
            {
                _v[0] = r;
                _v[1] = g;
                _v[2] = b;
            }

            inline bool
            operator==( const ubvec3& v ) const
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2];
            }

            inline bool
            operator!=( const ubvec3& v ) const
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2];
            }

            inline bool
            operator<( const ubvec3& v ) const
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
                else
                {
                    return ( _v[2] < v._v[2] );
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
            set( value_type r,
                 value_type g,
                 value_type b )
            {
                _v[0] = r;
                _v[1] = g;
                _v[2] = b;
            }

            inline void
            set( const ubvec3& rhs )
            {
                _v[0] = rhs._v[0];
                _v[1] = rhs._v[1];
                _v[2] = rhs._v[2];
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

            /** Multiply by scalar. */
            inline ubvec3
            operator*( float rhs ) const
            {
                ubvec3 col( *this );
                col *= rhs;
                return col;
            }

            /** Unary multiply by scalar. */
            inline ubvec3&
            operator*=( float rhs )
            {
                _v[0] = ( value_type )( ( float )_v[0] * rhs );
                _v[1] = ( value_type )( ( float )_v[1] * rhs );
                _v[2] = ( value_type )( ( float )_v[2] * rhs );
                return *this;
            }

            /** Divide by scalar. */
            inline ubvec3
            operator/( float rhs ) const
            {
                ubvec3 col( *this );
                col /= rhs;
                return col;
            }

            /** Unary divide by scalar. */
            inline ubvec3&
            operator/=( float rhs )
            {
                float div  = 1.0F / rhs;
                *this     *= div;
                return *this;
            }

            /** Binary vector add. */
            inline ubvec3
            operator+( const ubvec3& rhs ) const
            {
                return ubvec3( _v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            inline ubvec3&
            operator+=( const ubvec3& rhs )
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                _v[2] += rhs._v[2];
                return *this;
            }

            /** Binary vector subtract. */
            inline ubvec3
            operator-( const ubvec3& rhs ) const
            {
                return ubvec3( _v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2] );
            }

            /** Unary vector subtract. */
            inline ubvec3&
            operator-=( const ubvec3& rhs )
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                _v[2] -= rhs._v[2];
                return *this;
            }

    };    // end of class ubvec3

    /** multiply by vector components. */
    inline ubvec3
    componentMultiply( const ubvec3& lhs,
                       const ubvec3& rhs )
    {
        return ubvec3( lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2] );
    }

    /** divide rhs components by rhs vector components. */
    inline ubvec3
    componentDivide( const ubvec3& lhs,
                     const ubvec3& rhs )
    {
        return ubvec3( lhs[0] / rhs[0], lhs[1] / rhs[1], lhs[2] / rhs[2] );
    }

}    // end of namespace osg
