#include <osg/threading/Thread.hpp>

using namespace osg;

Thread::~Thread()
{
    if( _thread.joinable() )
    {
        _thread.join();
    }
}

int
Thread::start()
{
    if( _running.load( std::memory_order_relaxed ) )
    {
        return -1;
    }

    _done.store( false, std::memory_order_relaxed );
    _running.store( true, std::memory_order_release );
    _thread = std::thread(
        [this]
        {
            _id = std::this_thread::get_id();
            run();
            _running.store( false, std::memory_order_release );
        }
    );
    return 0;
}

int
Thread::join()
{
    if( _thread.joinable() )
    {
        _thread.join();
        return 0;
    }
    return -1;
}

int
Thread::detach()
{
    if( _thread.joinable() )
    {
        _thread.detach();
        return 0;
    }
    return -1;
}

int
Thread::cancel()
{
    _done.store( true, std::memory_order_relaxed );
    if( _thread.joinable() )
    {
        _thread.join();
    }
    return 0;
}
