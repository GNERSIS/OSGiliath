#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>

namespace osgFFmpeg
{

    template<class T>
    class MessageQueue
    {
        public:

            typedef T      value_type;
            typedef size_t size_type;

            MessageQueue();
            ~MessageQueue();

            void
            clear();

            void
            push( const T& value );

            value_type
            pop();
            value_type
            tryPop( bool& is_empty );
            value_type
            timedPop( bool&         is_empty,
                      unsigned long ms );

        private:

            MessageQueue( const MessageQueue& );
            MessageQueue&
                                            operator=( const MessageQueue& );

            typedef std::deque<T>           Queue;
            typedef std::condition_variable Condition;
            typedef std::mutex              Mutex;

            Mutex                           m_mutex;
            Condition                       m_not_empty;
            Queue                           m_queue;
    };

    template<class T>
    MessageQueue<T>::MessageQueue()
    {
    }

    template<class T>
    MessageQueue<T>::~MessageQueue()
    {
    }

    template<class T>
    void
    MessageQueue<T>::clear()
    {
        std::lock_guard<std::mutex> lock( m_mutex );

        m_queue.clear();
    }

    template<class T>
    void
    MessageQueue<T>::push( const T& value )
    {
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            m_queue.push_back( value );
        }

        m_not_empty.notify_one();
    }

    template<class T>
    typename MessageQueue<T>::value_type
    MessageQueue<T>::pop()
    {
        std::unique_lock<std::mutex> lock( m_mutex );

        while( m_queue.empty() )
        {
            m_not_empty.wait( lock );
        }

        const value_type value = m_queue.front();
        m_queue.pop_front();

        return value;
    }

    template<class T>
    typename MessageQueue<T>::value_type
    MessageQueue<T>::tryPop( bool& is_empty )
    {
        std::lock_guard<std::mutex> lock( m_mutex );

        is_empty = m_queue.empty();

        if( is_empty )
        {
            return value_type();
        }

        const value_type value = m_queue.front();
        m_queue.pop_front();

        return value;
    }

    template<class T>
    typename MessageQueue<T>::value_type
    MessageQueue<T>::timedPop( bool&               is_empty,
                               const unsigned long ms )
    {
        std::unique_lock<std::mutex> lock( m_mutex );

        // We don't wait in a loop to avoid an infinite loop (as the ms timeout would not
        // be decremented). This means that timedPop() could return with (is_empty =
        // true) before the timeout has been hit.

        if( m_queue.empty() )
        {
            m_not_empty.wait_for( lock, std::chrono::milliseconds( ms ) );
        }

        is_empty = m_queue.empty();

        if( is_empty )
        {
            return value_type();
        }

        const value_type value = m_queue.front();
        m_queue.pop_front();

        return value;
    }

}    // namespace osgFFmpeg
