/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Simple count-down latch for thread synchronization.
 * Threads wait until the count reaches zero.
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace osg
{

    /** Countdown latch with a lock-free hot path.
     *  count_down() decrements atomically; only acquires the mutex to notify
     *  when the count reaches zero. wait() blocks until the count is zero. */
    class Latch
    {
        public:

            explicit Latch( int count ) noexcept :
                _count( count )
            {
            }

            /** Decrement the count. Returns true if this call triggered the release. */
            bool
            count_down() noexcept
            {
                if( _count.fetch_sub( 1, std::memory_order_acq_rel ) <= 1 )
                {
                    std::lock_guard<std::mutex> lock( _mutex );
                    _cv.notify_all();
                    return true;
                }
                return false;
            }

            /** Block until the count reaches zero. */
            void
            wait()
            {
                std::unique_lock<std::mutex> lock( _mutex );
                _cv.wait( lock,
                          [this]
                          {
                              return _count.load( std::memory_order_acquire ) <= 0;
                          } );
            }

            /** Non-blocking check. */
            [[nodiscard]]
            bool
            try_wait() const noexcept
            {
                return _count.load( std::memory_order_acquire ) <= 0;
            }

            /** Reset for reuse. Not thread-safe — call only when no threads are waiting.
             */
            void
            reset( int count ) noexcept
            {
                _count.store( count, std::memory_order_release );
            }

        private:

            std::atomic<int>        _count;
            std::mutex              _mutex;
            std::condition_variable _cv;
    };

}    // namespace osg
