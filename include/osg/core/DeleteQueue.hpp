/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Lock-free queue of objects scheduled for deletion.
 * Processes deferred deletions after frame completion.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <osg/core/Object.hpp>
#include <osg/core/ref_ptr.hpp>
#include <vector>

namespace osg
{

    /** Deferred deletion queue with frame-based retention.
     *  Objects are kept alive for a configurable number of frames after being
     *  queued, preventing GPU use-after-free in asynchronous pipelines.
     *  Thread-safe: add() and flush() can be called from different threads. */
    class DeleteQueue
    {
        public:

            explicit DeleteQueue( uint64_t retainFrames = 3 ) noexcept :
                _retainFrames( retainFrames )
            {
            }

            /** Queue an object for deferred deletion. */
            void
            add( ref_ptr<Object> obj )
            {
                if( !obj )
                {
                    return;
                }
                std::lock_guard<std::mutex> lock( _mutex );
                _pending.push_back(
                    { _currentFrame + _retainFrames, std::move( obj ) }
                );
            }

            /** Advance the frame counter. Call once per frame from the main thread. */
            void
            advance( uint64_t frame ) noexcept
            {
                _currentFrame = frame;
            }

            /** Delete all objects whose retention period has expired.
             *  Returns the number of objects deleted. */
            std::size_t
            flush()
            {
                std::vector<Entry> expired;
                {
                    std::lock_guard<std::mutex> lock( _mutex );
                    auto it = std::partition( _pending.begin(),
                                              _pending.end(),
                                              [this]( const Entry& e )
                                              {
                                                  return e.expireFrame > _currentFrame;
                                              } );
                    expired.assign( std::make_move_iterator( it ),
                                    std::make_move_iterator( _pending.end() ) );
                    _pending.erase( it, _pending.end() );
                }
                // expired vector destructors release the ref_ptrs outside the lock
                return expired.size();
            }

            /** Clear all pending objects immediately regardless of frame count. */
            void
            clear()
            {
                std::vector<Entry> all;
                {
                    std::lock_guard<std::mutex> lock( _mutex );
                    all.swap( _pending );
                }
            }

            [[nodiscard]]
            std::size_t
            size() const
            {
                std::lock_guard<std::mutex> lock( _mutex );
                return _pending.size();
            }

            void
            setRetainFrames( uint64_t frames ) noexcept
            {
                _retainFrames = frames;
            }

            [[nodiscard]]
            uint64_t
            getRetainFrames() const noexcept
            {
                return _retainFrames;
            }

        private:

            struct Entry
            {
                    uint64_t        expireFrame;
                    ref_ptr<Object> object;
            };

            mutable std::mutex _mutex;
            std::vector<Entry> _pending;
            uint64_t           _currentFrame = 0;
            uint64_t           _retainFrames;
    };

}    // namespace osg
