/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Lightweight C++ thread wrapper. Provides start, join, detach, and
 * priority control using std::thread. Replaces OpenThreads.
 */
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <osg/core/Export.hpp>
#include <set>
#include <thread>

namespace osg
{

    /** Thread wrapper providing a subclass-based threading model using std::thread.
     * Subclasses override run() which is called in the new thread when start() is
     * invoked. Replaces the former OpenThreads::Thread class.
     */
    class OSG_EXPORT Thread
    {
        public:

            Thread() = default;
            virtual ~Thread();

            Thread( const Thread& ) = delete;
            Thread&
            operator=( const Thread& ) = delete;

            /** Override this to define the thread's work. Called in the new thread. */
            virtual void
            run() = 0;

            /** Start the thread. Returns 0 on success, -1 if already running (matches
             * old OpenThreads API). */
            int
            start();

            /** Alias for start(), for compatibility with osg::Thread::startThread(). */
            int
            startThread()
            {
                return start();
            }

            /** Wait for the thread to finish. Returns 0 on success. */
            int
            join();

            /** Detach the thread (cannot be joined after this). Returns 0 on success. */
            int
            detach();

            /** Check if the thread is currently running. */
            bool
            isRunning() const
            {
                return _running.load( std::memory_order_acquire );
            }

            /** Get the std::thread::id of this thread. */
            std::thread::id
            getThreadId() const
            {
                return _id;
            }

            /** Cancel the thread. Sets done flag and waits for the thread to finish.
             * Subclasses can override to add custom cancellation logic.
             * Returns 0 on success. */
            virtual int
            cancel();

            /** Test cancellation. Returns 0 if not cancelled, non-zero if cancelled. */
            int
            testCancel()
            {
                return _done.load( std::memory_order_relaxed ) ? 1 : 0;
            }

            /** Set the thread's schedule priority (no-op in std::thread, retained for
             * API compatibility). */
            int
            setSchedulePriority( int /*priority*/ )
            {
                return 0;
            }

            /** Get the thread's schedule priority (returns 0, retained for API
             * compatibility). */
            int
            getSchedulePriority()
            {
                return 0;
            }

            /** Set the thread's scheduling policy (no-op, retained for API
             * compatibility). */
            int
            setSchedulePolicy( int /*policy*/ )
            {
                return 0;
            }

            /** Get the thread's scheduling policy (returns 0, retained for API
             * compatibility). */
            int
            getSchedulePolicy()
            {
                return 0;
            }

            /** Set stack size (no-op, retained for API compatibility). */
            int
            setStackSize( size_t /*size*/ )
            {
                return 0;
            }

            /** Get stack size (returns 0, retained for API compatibility). */
            size_t
            getStackSize()
            {
                return 0;
            }

            /** Thread priority enum retained for API compatibility. */
            enum ThreadPriority
            {
                THREAD_PRIORITY_MAX,
                THREAD_PRIORITY_HIGH,
                THREAD_PRIORITY_NOMINAL,
                THREAD_PRIORITY_LOW,
                THREAD_PRIORITY_MIN,
                THREAD_PRIORITY_DEFAULT,
            };

            /** Thread scheduling policy enum retained for API compatibility. */
            enum ThreadPolicy
            {
                THREAD_SCHEDULE_FIFO,
                THREAD_SCHEDULE_ROUND_ROBIN,
                THREAD_SCHEDULE_TIME_SHARE,
                THREAD_SCHEDULE_DEFAULT,
            };

            /** Processor affinity specification. */
            struct Affinity
            {
                    Affinity()
                    {
                    }

                    Affinity( unsigned int cpuNumber )
                    {
                        activeCPUs.insert( cpuNumber );
                    }

                    Affinity( unsigned int cpuNumber,
                              unsigned int cpuCount )
                    {
                        while( cpuCount > 0 )
                        {
                            activeCPUs.insert( cpuNumber++ );
                            --cpuCount;
                        }
                    }

                    Affinity( const Affinity& rhs ) :
                        activeCPUs( rhs.activeCPUs )
                    {
                    }

                    Affinity&
                    operator=( const Affinity& rhs )
                    {
                        if( &rhs != this )
                        {
                            activeCPUs = rhs.activeCPUs;
                        }
                        return *this;
                    }

                    void
                    add( unsigned int cpuNumber )
                    {
                        activeCPUs.insert( cpuNumber );
                    }

                    void
                    remove( unsigned int cpuNumber )
                    {
                        activeCPUs.erase( cpuNumber );
                    }

                    operator bool() const
                    {
                        return !activeCPUs.empty();
                    }

                    typedef std::set<unsigned int> ActiveCPUs;
                    ActiveCPUs                     activeCPUs;
            };

            /** Set processor affinity (no-op, retained for API compatibility). */
            int
            setProcessorAffinity( const Affinity& /*affinity*/ )
            {
                return 0;
            }

            // Static utility methods

            /** Sleep for the specified number of microseconds. */
            static int
            microSleep( unsigned int microseconds )
            {
                std::this_thread::sleep_for( std::chrono::microseconds( microseconds ) );
                return 0;
            }

            /** Yield the current thread's time slice. */
            static int
            YieldCurrentThread()
            {
                std::this_thread::yield();
                return 0;
            }

            /** Yield (lowercase alias). */
            static void
            yieldCurrentThread()
            {
                std::this_thread::yield();
            }

            /** Get the current thread's ID. */
            static size_t
            CurrentThreadId()
            {
                return std::hash<std::thread::id>{}( std::this_thread::get_id() );
            }

            /** Get the current thread's std::thread::id. */
            static std::thread::id
            currentThreadId()
            {
                return std::this_thread::get_id();
            }

            /** Print scheduling info (no-op). */
            void
            printSchedulingInfo()
            {
            }

            /** Cancel cleanup hook — subclasses can override. */
            virtual void
            cancelCleanup()
            {
            }

            /** Disable cancel mode (no-op). */
            int
            setCancelModeDisable()
            {
                return 0;
            }

            /** Set cancel mode to asynchronous (no-op). */
            int
            setCancelModeAsynchronous()
            {
                return 0;
            }

            /** Set cancel mode to deferred (no-op). */
            int
            setCancelModeDeferred()
            {
                return 0;
            }

        private:

            std::thread       _thread;
            std::atomic<bool> _running{ false };
            std::atomic<bool> _done{ false };
            std::thread::id   _id;
    };

    /** Block is a synchronization primitive that can halt a thread until another thread
     * releases it. */
    class OSG_EXPORT Block
    {
        public:

            Block() :
                _released( false )
            {
            }

            ~Block()
            {
                release();
            }

            bool
            block()
            {
                std::unique_lock<std::mutex> lock( _mutex );
                if( !_released )
                {
                    _cond.wait( lock,
                                [this]
                                {
                                    return _released;
                                } );
                }
                return true;
            }

            bool
            block( unsigned long timeout )
            {
                std::unique_lock<std::mutex> lock( _mutex );
                if( !_released )
                {
                    return _cond.wait_for( lock,
                                           std::chrono::milliseconds( timeout ),
                                           [this]
                                           {
                                               return _released;
                                           } );
                }
                return true;
            }

            void
            release()
            {
                std::lock_guard<std::mutex> lock( _mutex );
                if( !_released )
                {
                    _released = true;
                    _cond.notify_all();
                }
            }

            void
            reset()
            {
                std::lock_guard<std::mutex> lock( _mutex );
                _released = false;
            }

            void
            set( bool doRelease )
            {
                if( doRelease != _released )
                {
                    if( doRelease )
                    {
                        release();
                    }
                    else
                    {
                        reset();
                    }
                }
            }

        protected:

            std::mutex              _mutex;
            std::condition_variable _cond;
            bool                    _released;

        private:

            Block( const Block& )
            {
            }
    };

    /** BlockCount halts a thread waiting for a specified number of operations to
     * complete. */
    class OSG_EXPORT BlockCount
    {
        public:

            BlockCount( unsigned int blockCount ) :
                _blockCount( blockCount ),
                _currentCount( 0 )
            {
            }

            ~BlockCount()
            {
                _blockCount = 0;
                release();
            }

            void
            completed()
            {
                std::lock_guard<std::mutex> lock( _mutex );
                if( _currentCount > 0 )
                {
                    --_currentCount;
                    if( _currentCount == 0 )
                    {
                        _cond.notify_all();
                    }
                }
            }

            void
            block()
            {
                std::unique_lock<std::mutex> lock( _mutex );
                if( _currentCount )
                {
                    _cond.wait( lock,
                                [this]
                                {
                                    return _currentCount == 0;
                                } );
                }
            }

            void
            reset()
            {
                std::lock_guard<std::mutex> lock( _mutex );
                if( _currentCount != _blockCount )
                {
                    if( _blockCount == 0 )
                    {
                        _cond.notify_all();
                    }
                    _currentCount = _blockCount;
                }
            }

            void
            release()
            {
                std::lock_guard<std::mutex> lock( _mutex );
                if( _currentCount )
                {
                    _currentCount = 0;
                    _cond.notify_all();
                }
            }

            void
            setBlockCount( unsigned int blockCount )
            {
                _blockCount = blockCount;
            }

            unsigned int
            getBlockCount() const
            {
                return _blockCount;
            }

            unsigned int
            getCurrentCount() const
            {
                return _currentCount;
            }

        protected:

            std::mutex              _mutex;
            std::condition_variable _cond;
            unsigned int            _blockCount;
            unsigned int            _currentCount;

        private:

            BlockCount( const BlockCount& )
            {
            }
    };

    /** Barrier synchronization primitive — blocks until a specified number of threads
     * have arrived. */
    class OSG_EXPORT Barrier
    {
        public:

            Barrier( int numThreads = 0 ) :
                _numThreads( static_cast<unsigned int>( numThreads ) ),
                _count( 0 ),
                _generation( 0 ),
                _valid( true )
            {
            }

            virtual ~Barrier()
            {
            }

            virtual void
            reset()
            {
                std::lock_guard<std::mutex> lock( _mutex );
                _count = 0;
            }

            virtual void
            block( unsigned int numThreads = 0 )
            {
                std::unique_lock<std::mutex> lock( _mutex );
                if( !_valid )
                {
                    return;
                }

                unsigned int target = ( numThreads > 0 ) ? numThreads : _numThreads;
                if( target == 0 )
                {
                    return;
                }

                unsigned int gen = _generation;
                ++_count;
                if( _count >= target )
                {
                    _count = 0;
                    ++_generation;
                    _cond.notify_all();
                }
                else
                {
                    _cond.wait( lock,
                                [this, gen]
                                {
                                    return gen != _generation || !_valid;
                                } );
                }
            }

            virtual void
            release()
            {
                std::lock_guard<std::mutex> lock( _mutex );
                _count = 0;
                ++_generation;
                _cond.notify_all();
            }

            virtual int
            numThreadsCurrentlyBlocked()
            {
                std::lock_guard<std::mutex> lock( _mutex );
                return static_cast<int>( _count );
            }

            void
            invalidate()
            {
                std::lock_guard<std::mutex> lock( _mutex );
                _valid = false;
                _cond.notify_all();
            }

        private:

            Barrier( const Barrier& )
            {
            }

            Barrier&
            operator=( const Barrier& )
            {
                return *this;
            }

            std::mutex              _mutex;
            std::condition_variable _cond;
            unsigned int            _numThreads;
            unsigned int            _count;
            unsigned int            _generation;
            bool                    _valid;
    };

    /** Get the number of processors available. */
    inline int
    GetNumberOfProcessors()
    {
        int count = static_cast<int>( std::thread::hardware_concurrency() );
        return count > 0 ? count : 1;
    }

    /** Set the processor affinity of the current thread (no-op on most platforms). */
    inline int
    SetProcessorAffinityOfCurrentThread( const Thread::Affinity& /*affinity*/ )
    {
        return 0;
    }

}    // namespace osg
