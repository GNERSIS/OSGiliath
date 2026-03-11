/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Per-context value array indexed by context ID. Provides
 * thread-safe storage for GL object handles across contexts.
 */
#pragma once

#include <osg/rendering/DisplaySettings.hpp>
#include <vector>

namespace osg
{

    /** Implements a simple buffered value for values that need to be buffered on
     * a per graphics context basis.
     */
    template<class T>
    class buffered_value
    {
        public:

            inline buffered_value() :
                _array( DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),
                        0 )
            {
            }

            inline buffered_value( unsigned int size ) :
                _array( size,
                        0 )
            {
            }

            buffered_value( const buffered_value& ) = default;

            buffered_value&
            operator=( const buffered_value& rhs )
            {
                _array = rhs._array;
                return *this;
            }

            inline void
            setAllElementsTo( const T& t )
            {
                std::fill( _array.begin(), _array.end(), t );
            }

            inline void
            clear()
            {
                _array.clear();
            }

            inline bool
            empty() const
            {
                return _array.empty();
            }

            inline unsigned int
            size() const
            {
                return static_cast<unsigned int>( _array.size() );
            }

            inline void
            resize( unsigned int newSize )
            {
                _array.resize( newSize, 0 );
            }

            inline T&
            operator[]( unsigned int pos )
            {
                // automatically resize array.
                if( _array.size() <= pos )
                {
                    _array.resize( pos + 1, 0 );
                }

                return _array[pos];
            }

            inline T
            operator[]( unsigned int pos ) const
            {
                // automatically resize array.
                if( _array.size() <= pos )
                {
                    _array.resize( pos + 1, 0 );
                }

                return _array[pos];
            }

        protected:

            mutable std::vector<T> _array;
    };

    template<class T>
    class buffered_object
    {
        public:

            inline buffered_object() :
                _array( DisplaySettings::instance()->getMaxNumberOfGraphicsContexts() )
            {
            }

            inline buffered_object( unsigned int size ) :
                _array( size )
            {
            }

            buffered_object&
            operator=( const buffered_object& rhs )
            {
                _array = rhs._array;
                return *this;
            }

            inline void
            setAllElementsTo( const T& t )
            {
                std::fill( _array.begin(), _array.end(), t );
            }

            inline void
            clear()
            {
                _array.clear();
            }

            inline bool
            empty() const
            {
                return _array.empty();
            }

            inline unsigned int
            size() const
            {
                return static_cast<unsigned int>( _array.size() );
            }

            inline void
            resize( unsigned int newSize )
            {
                _array.resize( newSize );
            }

            inline T&
            operator[]( unsigned int pos )
            {
                // automatically resize array.
                if( _array.size() <= pos )
                {
                    _array.resize( pos + 1 );
                }

                return _array[pos];
            }

            inline const T&
            operator[]( unsigned int pos ) const
            {
                // automatically resize array.
                if( _array.size() <= pos )
                {
                    _array.resize( pos + 1 );
                }

                return _array[pos];
            }

        protected:

            mutable std::vector<T> _array;
    };

}
