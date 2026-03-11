/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 2D unsigned integer vector for index pairs and grid coordinates.
 * 32-bit unsigned integer components.
 */
#pragma once

namespace osg
{

    /** General purpose integer pair.
     */
    class uivec2
    {
        public:

            /** Type of Vec class.*/
            typedef unsigned int value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 2,
            };

            /** Vec member variable. */
            value_type _v[2];

            uivec2()
            {
                _v[0] = 0;
                _v[1] = 0;
            }

            uivec2( value_type x,
                    value_type y )
            {
                _v[0] = x;
                _v[1] = y;
            }

            inline bool
            operator==( const uivec2& v ) const
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1];
            }

            inline bool
            operator!=( const uivec2& v ) const
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1];
            }

            inline bool
            operator<( const uivec2& v ) const
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
            set( const uivec2& rhs )
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

            inline uivec2
            operator*( value_type rhs ) const
            {
                return uivec2( _v[0] * rhs, _v[1] * rhs );
            }

            inline uivec2
            operator/( value_type rhs ) const
            {
                return uivec2( _v[0] / rhs, _v[1] / rhs );
            }

            inline uivec2
            operator+( value_type rhs ) const
            {
                return uivec2( _v[0] + rhs, _v[1] + rhs );
            }

            inline uivec2
            operator-( value_type rhs ) const
            {
                return uivec2( _v[0] - rhs, _v[1] - rhs );
            }

            inline uivec2
            operator+( const uivec2& rhs ) const
            {
                return uivec2( _v[0] + rhs._v[0], _v[1] + rhs._v[1] );
            }

            inline uivec2
            operator-( const uivec2& rhs ) const
            {
                return uivec2( _v[0] - rhs._v[0], _v[1] - rhs._v[1] );
            }

            inline uivec2
            operator*( const uivec2& rhs ) const
            {
                return uivec2( _v[0] * rhs._v[0], _v[1] * rhs._v[1] );
            }
    };

}
