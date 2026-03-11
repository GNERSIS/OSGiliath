/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 3D unsigned short vector for compact per-vertex data.
 * 16-bit unsigned components for reduced-precision attributes.
 */
#pragma once

namespace osg
{

    class usvec3
    {
        public:

            /** Data type of vector components.*/
            typedef unsigned short value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 3,
            };

            value_type _v[3];

            /** Constructor that sets all components of the vector to zero */
            usvec3()
            {
                _v[0] = 0;
                _v[1] = 0;
                _v[2] = 0;
            }

            usvec3( value_type r,
                    value_type g,
                    value_type b )
            {
                _v[0] = r;
                _v[1] = g;
                _v[2] = b;
            }

            inline bool
            operator==( const usvec3& v ) const
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2];
            }

            inline bool
            operator!=( const usvec3& v ) const
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2];
            }

            inline bool
            operator<( const usvec3& v ) const
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
            set( const usvec3& rhs )
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
            inline usvec3
            operator*( value_type rhs ) const
            {
                return usvec3( _v[0] * rhs, _v[1] * rhs, _v[2] * rhs );
            }

            /** Unary multiply by scalar. */
            inline usvec3&
            operator*=( value_type rhs )
            {
                _v[0] *= rhs;
                _v[1] *= rhs;
                _v[2] *= rhs;
                return *this;
            }

            /** Divide by scalar. */
            inline usvec3
            operator/( value_type rhs ) const
            {
                return usvec3( _v[0] / rhs, _v[1] / rhs, _v[2] / rhs );
            }

            /** Unary divide by scalar. */
            inline usvec3&
            operator/=( value_type rhs )
            {
                _v[0] /= rhs;
                _v[1] /= rhs;
                _v[2] /= rhs;
                return *this;
            }

            /** Binary vector multiply. */
            inline usvec3
            operator*( const usvec3& rhs ) const
            {
                return usvec3( _v[0] * rhs._v[0], _v[1] * rhs._v[1], _v[2] * rhs._v[2] );
            }

            /** Binary vector add. */
            inline usvec3
            operator+( const usvec3& rhs ) const
            {
                return usvec3( _v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            inline usvec3&
            operator+=( const usvec3& rhs )
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                _v[2] += rhs._v[2];
                return *this;
            }

            /** Binary vector subtract. */
            inline usvec3
            operator-( const usvec3& rhs ) const
            {
                return usvec3( _v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2] );
            }

            /** Unary vector subtract. */
            inline usvec3&
            operator-=( const usvec3& rhs )
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                _v[2] -= rhs._v[2];
                return *this;
            }

    };    // end of class usvec3

    /** multiply by vector components. */
    inline usvec3
    componentMultiply( const usvec3& lhs,
                       const usvec3& rhs )
    {
        return usvec3( lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2] );
    }

    /** divide rhs components by rhs vector components. */
    inline usvec3
    componentDivide( const usvec3& lhs,
                     const usvec3& rhs )
    {
        return usvec3( lhs[0] / rhs[0], lhs[1] / rhs[1], lhs[2] / rhs[2] );
    }

}    // end of namespace osg
