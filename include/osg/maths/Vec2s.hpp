/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 2D short vector for compact per-vertex data.
 * 16-bit signed components for reduced-precision attributes.
 */
#pragma once

namespace osg
{

    class svec2
    {
        public:

            /** Data type of vector components.*/
            typedef short value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 2,
            };

            value_type _v[2];

            /** Constructor that sets all components of the vector to zero */
            svec2()
            {
                _v[0] = 0;
                _v[1] = 0;
            }

            svec2( value_type x,
                   value_type y )
            {
                _v[0] = x;
                _v[1] = y;
            }

            inline bool
            operator==( const svec2& v ) const
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1];
            }

            inline bool
            operator!=( const svec2& v ) const
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1];
            }

            inline bool
            operator<( const svec2& v ) const
            {
                if( _v[0] < v._v[0] )
                {
                    return true;
                }
                else if( _v[0] > v._v[0] )
                {
                    return false;
                }
                else
                {
                    return ( _v[1] < v._v[1] );
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
                 value_type y )
            {
                _v[0] = x;
                _v[1] = y;
            }

            inline void
            set( const svec2& rhs )
            {
                _v[0] = rhs._v[0];
                _v[1] = rhs._v[1];
            }

            inline value_type&
            operator[]( int i )
            {
                return _v[i];
            }

            inline value_type
            operator[]( int i ) const
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

            inline svec2
            operator*( value_type rhs ) const
            {
                return svec2( _v[0] * rhs, _v[1] * rhs );
            }

            inline svec2&
            operator*=( value_type rhs )
            {
                _v[0] *= rhs;
                _v[1] *= rhs;
                return *this;
            }

            inline svec2
            operator/( value_type rhs ) const
            {
                return svec2( _v[0] / rhs, _v[1] / rhs );
            }

            inline svec2&
            operator/=( value_type rhs )
            {
                _v[0] /= rhs;
                _v[1] /= rhs;
                return *this;
            }

            inline svec2
            operator*( const svec2& rhs ) const
            {
                return svec2( _v[0] * rhs._v[0], _v[1] * rhs._v[1] );
            }

            inline svec2
            operator+( const svec2& rhs ) const
            {
                return svec2( _v[0] + rhs._v[0], _v[1] + rhs._v[1] );
            }

            inline svec2&
            operator+=( const svec2& rhs )
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                return *this;
            }

            inline svec2
            operator-( const svec2& rhs ) const
            {
                return svec2( _v[0] - rhs._v[0], _v[1] - rhs._v[1] );
            }

            inline svec2&
            operator-=( const svec2& rhs )
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                return *this;
            }

            inline svec2
            operator-() const
            {
                return svec2( -_v[0], -_v[1] );
            }

    };    // end of class svec2

    /** multiply by vector components. */
    inline svec2
    componentMultiply( const svec2& lhs,
                       const svec2& rhs )
    {
        return svec2( lhs[0] * rhs[0], lhs[1] * rhs[1] );
    }

    /** divide rhs components by rhs vector components. */
    inline svec2
    componentDivide( const svec2& lhs,
                     const svec2& rhs )
    {
        return svec2( lhs[0] / rhs[0], lhs[1] / rhs[1] );
    }

}    // end of namespace osg
