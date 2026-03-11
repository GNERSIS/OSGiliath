/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 3D unsigned integer vector for voxel coordinates.
 * 32-bit unsigned integer components.
 */
#pragma once

namespace osg
{

    /** General purpose integer triple
     **/
    class uivec3
    {
        public:

            /** Type of Vec class.*/
            typedef unsigned int value_type;

            /** Number of vector components. */
            enum
            {
                num_components = 3,
            };

            /** Vec member variable. */
            value_type _v[3];

            uivec3()
            {
                _v[0] = 0;
                _v[1] = 0;
                _v[2] = 0;
            }

            uivec3( value_type r,
                    value_type g,
                    value_type b )
            {
                _v[0] = r;
                _v[1] = g;
                _v[2] = b;
            }

            inline bool
            operator==( const uivec3& v ) const
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2];
            }

            inline bool
            operator!=( const uivec3& v ) const
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2];
            }

            inline bool
            operator<( const uivec3& v ) const
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
            set( const uivec3& rhs )
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
            inline uivec3
            operator*( value_type rhs ) const
            {
                return uivec3( _v[0] * rhs, _v[1] * rhs, _v[2] * rhs );
            }

            inline uivec3
            operator/( value_type rhs ) const
            {
                return uivec3( _v[0] / rhs, _v[1] / rhs, _v[2] / rhs );
            }

            inline uivec3
            operator+( value_type rhs ) const
            {
                return uivec3( _v[0] + rhs, _v[1] + rhs, _v[2] + rhs );
            }

            inline uivec3
            operator-( value_type rhs ) const
            {
                return uivec3( _v[0] - rhs, _v[1] - rhs, _v[2] - rhs );
            }

            inline uivec3
            operator+( const uivec3& rhs ) const
            {
                return uivec3( _v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2] );
            }

            inline uivec3
            operator-( const uivec3& rhs ) const
            {
                return uivec3( _v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2] );
            }

            inline uivec3
            operator*( const uivec3& rhs ) const
            {
                return uivec3( _v[0] * rhs._v[0], _v[1] * rhs._v[1], _v[2] * rhs._v[2] );
            }
    };

}
