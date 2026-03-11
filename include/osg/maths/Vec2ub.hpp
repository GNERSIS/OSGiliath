/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 2D unsigned byte vector for compact per-vertex data.
 * 8-bit unsigned components, used for texture coordinates.
 */
#pragma once

namespace osg
{

    /** General purpose unsigned byte pair.
     */
    class ubvec2
    {
        public:

            /** Data type of vector components.*/
            typedef unsigned char value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 2,
            };

            /** Vec member variable. */
            value_type _v[2];

            /** Constructor that sets all components of the vector to zero */
            ubvec2()
            {
                _v[0] = 0;
                _v[1] = 0;
            }

            ubvec2( value_type r,
                    value_type g )
            {
                _v[0] = r;
                _v[1] = g;
            }

            inline bool
            operator==( const ubvec2& v ) const
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1];
            }

            inline bool
            operator!=( const ubvec2& v ) const
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1];
            }

            inline bool
            operator<( const ubvec2& v ) const
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
            set( const ubvec2& rhs )
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

            /** Multiply by scalar. */
            inline ubvec2
            operator*( float rhs ) const
            {
                ubvec2 col( *this );
                col *= rhs;
                return col;
            }

            /** Unary multiply by scalar. */
            inline ubvec2&
            operator*=( float rhs )
            {
                _v[0] = ( value_type )( ( float )_v[0] * rhs );
                _v[1] = ( value_type )( ( float )_v[1] * rhs );
                return *this;
            }

            /** Divide by scalar. */
            inline ubvec2
            operator/( float rhs ) const
            {
                ubvec2 col( *this );
                col /= rhs;
                return col;
            }

            /** Unary divide by scalar. */
            inline ubvec2&
            operator/=( float rhs )
            {
                float div  = 1.0F / rhs;
                *this     *= div;
                return *this;
            }

            /** Binary vector add. */
            inline ubvec2
            operator+( const ubvec2& rhs ) const
            {
                return ubvec2( _v[0] + rhs._v[0], _v[1] + rhs._v[1] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            inline ubvec2&
            operator+=( const ubvec2& rhs )
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                return *this;
            }

            /** Binary vector subtract. */
            inline ubvec2
            operator-( const ubvec2& rhs ) const
            {
                return ubvec2( _v[0] - rhs._v[0], _v[1] - rhs._v[1] );
            }

            /** Unary vector subtract. */
            inline ubvec2&
            operator-=( const ubvec2& rhs )
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                return *this;
            }

    };    // end of class ubvec2

    /** multiply by vector components. */
    inline ubvec2
    componentMultiply( const ubvec2& lhs,
                       const ubvec2& rhs )
    {
        return ubvec2( lhs[0] * rhs[0], lhs[1] * rhs[1] );
    }

    /** divide rhs components by rhs vector components. */
    inline ubvec2
    componentDivide( const ubvec2& lhs,
                     const ubvec2& rhs )
    {
        return ubvec2( lhs[0] / rhs[0], lhs[1] / rhs[1] );
    }

}    // end of namespace osg
