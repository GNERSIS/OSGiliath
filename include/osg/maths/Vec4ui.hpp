/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 4D unsigned integer vector for integer attributes.
 * 32-bit unsigned integer components.
 */
#pragma once

namespace osg
{

    /** General purpose integer quad
     **/
    class uivec4
    {
        public:

            /** Type of Vec class.*/
            typedef unsigned int value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 4,
            };

            /** Vec member variable. */
            value_type _v[4];

            uivec4()
            {
                _v[0] = 0;
                _v[1] = 0;
                _v[2] = 0;
                _v[3] = 0;
            }

            uivec4( value_type x,
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
            operator==( const uivec4& v ) const
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
            operator!=( const uivec4& v ) const
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
            operator<( const uivec4& v ) const
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

            inline uivec4
            operator*( value_type rhs ) const
            {
                return uivec4( _v[0] * rhs, _v[1] * rhs, _v[2] * rhs, _v[3] * rhs );
            }

            inline uivec4
            operator/( value_type rhs ) const
            {
                return uivec4( _v[0] / rhs, _v[1] / rhs, _v[2] / rhs, _v[3] / rhs );
            }

            inline uivec4
            operator+( value_type rhs ) const
            {
                return uivec4( _v[0] + rhs, _v[1] + rhs, _v[2] + rhs, _v[3] + rhs );
            }

            inline uivec4
            operator-( value_type rhs ) const
            {
                return uivec4( _v[0] - rhs, _v[1] - rhs, _v[2] - rhs, _v[3] - rhs );
            }

            inline uivec4
            operator+( const uivec4& rhs ) const
            {
                return uivec4( _v[0] + rhs._v[0],
                               _v[1] + rhs._v[1],
                               _v[2] + rhs._v[2],
                               _v[3] + rhs._v[3] );
            }

            inline uivec4
            operator-( const uivec4& rhs ) const
            {
                return uivec4( _v[0] - rhs._v[0],
                               _v[1] - rhs._v[1],
                               _v[2] - rhs._v[2],
                               _v[3] - rhs._v[3] );
            }

            inline uivec4
            operator*( const uivec4& rhs ) const
            {
                return uivec4( _v[0] * rhs._v[0],
                               _v[1] * rhs._v[1],
                               _v[2] * rhs._v[2],
                               _v[3] * rhs._v[3] );
            }
    };

}
