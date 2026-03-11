/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Lightweight stack using a deque with O(1) back() access.
 * Used internally for state and matrix stacks during traversal.
 */
#pragma once

#include <vector>

namespace osg
{

    /** Simple stack implementation that keeps the back() cached locally for fast access
     * rather than at the back of the vector which is the traditional stack
     * implementation. A conventional std::vector<> stores the rest of the stack.
     * Although fast_back_stack contains a stl container it only implements the back
     * push_back(),pop_back() and back() methods so is not as general purpose as stl
     * stack implementation. The focus of the fast_back_stack is purely to maximize the
     * speed at which the back can be accessed.*/

    template<class T>
    class fast_back_stack
    {
        public:

            inline fast_back_stack() :
                _value(),
                _stack(),
                _size( 0 )
            {
            }

            inline fast_back_stack( const fast_back_stack& fbs ) :
                _value( fbs._value ),
                _stack( fbs._stack ),
                _size( fbs._size )
            {
            }

            inline fast_back_stack( const T& value ) :
                _value( value ),
                _stack(),
                _size( 1 )
            {
            }

            fast_back_stack&
            operator=( const fast_back_stack& fbs )
            {
                _value = fbs._value;
                _stack = fbs._stack;
                _size  = fbs._size;
                return *this;
            }

            inline void
            clear()
            {
                _stack.clear();
                _size = 0;
            }

            inline bool
            empty() const
            {
                return _size == 0;
            }

            inline unsigned int
            size() const
            {
                return _size;
            }

            inline T&
            back()
            {
                return _value;
            }

            inline const T&
            back() const
            {
                return _value;
            }

            inline void
            push_back()
            {
                if( _size > 0 )
                {
                    _stack.push_back( _value );
                }
                ++_size;
            }

            inline void
            push_back( const T& value )
            {
                if( _size > 0 )
                {
                    _stack.push_back( _value );
                }
                _value = value;
                ++_size;
            }

            inline void
            pop_back()
            {
                if( _size > 0 )
                {
                    if( !_stack.empty() )
                    {
                        _value = _stack.back();
                        _stack.pop_back();
                    }
                    --_size;
                }    // else error condition.
            }

            T              _value;
            std::vector<T> _stack;
            unsigned int   _size;
    };

}
