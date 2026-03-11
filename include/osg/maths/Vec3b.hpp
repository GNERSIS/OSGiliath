/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 3D byte vector for compact per-vertex normals.
 * 8-bit signed components with normalize() support.
 */
#pragma once

namespace osg
{

    /** General purpose float triple.
     * Uses include representation of color coordinates.
     * No support yet added for float * bvec3 - is it necessary?
     * Need to define a non-member non-friend operator*  etc.
     * bvec3 * float is okay
     */
    class bvec3
    {
        public:

            /** Data type of vector components.*/
            typedef signed char value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 3,
            };

            value_type _v[3];

            /** Constructor that sets all components of the vector to zero */
            bvec3()
            {
                _v[0] = 0;
                _v[1] = 0;
                _v[2] = 0;
            }

            bvec3( value_type r,
                   value_type g,
                   value_type b )
            {
                _v[0] = r;
                _v[1] = g;
                _v[2] = b;
            }

            inline bool
            operator==( const bvec3& v ) const
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2];
            }

            inline bool
            operator!=( const bvec3& v ) const
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2];
            }

            inline bool
            operator<( const bvec3& v ) const
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
            set( const bvec3& rhs )
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
            inline bvec3
            operator*( float rhs ) const
            {
                bvec3 col( *this );
                col *= rhs;
                return col;
            }

            /** Unary multiply by scalar. */
            inline bvec3&
            operator*=( float rhs )
            {
                _v[0] = ( value_type )( ( float )_v[0] * rhs );
                _v[1] = ( value_type )( ( float )_v[1] * rhs );
                _v[2] = ( value_type )( ( float )_v[2] * rhs );
                return *this;
            }

            /** Divide by scalar. */
            inline bvec3
            operator/( float rhs ) const
            {
                bvec3 col( *this );
                col /= rhs;
                return col;
            }

            /** Unary divide by scalar. */
            inline bvec3&
            operator/=( float rhs )
            {
                float div  = 1.0F / rhs;
                *this     *= div;
                return *this;
            }

            /** Binary vector add. */
            inline bvec3
            operator+( const bvec3& rhs ) const
            {
                return bvec3( _v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            inline bvec3&
            operator+=( const bvec3& rhs )
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                _v[2] += rhs._v[2];
                return *this;
            }

            /** Binary vector subtract. */
            inline bvec3
            operator-( const bvec3& rhs ) const
            {
                return bvec3( _v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2] );
            }

            /** Unary vector subtract. */
            inline bvec3&
            operator-=( const bvec3& rhs )
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                _v[2] -= rhs._v[2];
                return *this;
            }

    };    // end of class bvec3

}    // end of namespace osg
