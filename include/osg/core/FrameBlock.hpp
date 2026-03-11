/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Frame synchronization barrier using mutex and condition variable.
 * Blocks worker threads until the next frame signal.
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <osg/core/FrameStamp.hpp>
#include <osg/core/ref_ptr.hpp>

namespace osg
{

    /** Event-driven frame synchronization primitive.
     *  The main thread calls set() with a new FrameStamp each frame.
     *  Worker threads call wait_for_change() to block until the next frame.
     *  Call release() to wake all waiters and signal shutdown. */
    class FrameBlock
    {
        public:

            FrameBlock() = default;

            /** Set a new frame stamp and wake all waiting threads. */
            void
            set( ref_ptr<FrameStamp> frameStamp )
            {
                std::lock_guard<std::mutex> lock( _mutex );
                _value = frameStamp;
                _cv.notify_all();
            }

            /** Block until the frame stamp changes from `value`, then update `value`.
             *  Returns false if the block has been released (shutdown). */
            [[nodiscard]]
            bool
            wait_for_change( ref_ptr<FrameStamp>& value )
            {
                std::unique_lock<std::mutex> lock( _mutex );
                _cv.wait( lock,
                          [this, &value]
                          {
                              return _value !=
                                     value ||
                                     !_active.load( std::memory_order_acquire );
                          } );
                if( !_active.load( std::memory_order_acquire ) )
                {
                    return false;
                }
                value = _value;
                return true;
            }

            /** Signal all waiters to exit (shutdown). */
            void
            release()
            {
                _active.store( false, std::memory_order_release );
                std::lock_guard<std::mutex> lock( _mutex );
                _cv.notify_all();
            }

            [[nodiscard]]
            bool
            active() const noexcept
            {
                return _active.load( std::memory_order_acquire );
            }

        private:

            ref_ptr<FrameStamp>     _value;
            std::mutex              _mutex;
            std::condition_variable _cv;
            std::atomic_bool        _active{ true };
    };

}    // namespace osg
