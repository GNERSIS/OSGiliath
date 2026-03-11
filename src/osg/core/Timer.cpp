/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * High-resolution timer for performance measurement. Provides
 * tick counts and seconds conversion using platform-specific clocks.
 */
// #include <stdlib.h>
#include <osg/core/Timer.hpp>

#include <osg/core/Notify.hpp>
#include <stdio.h>
#include <string.h>

using namespace osg;

// follows are the constructors of the Timer class, once version
// for each OS combination.  The order is _WIN32, FreeBSD, Linux, IRIX,
// and the rest of the world.
//
// all the rest of the timer methods are implemented within the header.

Timer*
Timer::instance()
{
    static Timer s_timer;
    return &s_timer;
}

#ifdef _WIN32

    #include <fcntl.h>
    #include <sys/types.h>
    #include <winbase.h>
    #include <windows.h>

Timer::Timer()
{
    LARGE_INTEGER frequency;
    if( QueryPerformanceFrequency( &frequency ) )
    {
        _secsPerTick = 1.0 / ( double )frequency.QuadPart;
    }
    else
    {
        _secsPerTick = 1.0;
        OSG_NOTICE << "Error: Timer::Timer() unable to use QueryPerformanceFrequency, "
                   << std::endl;
        OSG_NOTICE << "timing code will be wrong, Windows error code: " << GetLastError()
                   << std::endl;
    }

    setStartTick();
}

Timer_t
Timer::tick() const
{
    LARGE_INTEGER qpc;
    if( QueryPerformanceCounter( &qpc ) )
    {
        return qpc.QuadPart;
    }
    else
    {
        OSG_NOTICE << "Error: Timer::Timer() unable to use QueryPerformanceCounter, "
                   << std::endl;
        OSG_NOTICE << "timing code will be wrong, Windows error code: " << GetLastError()
                   << std::endl;
        return 0;
    }
}

#else
    #include <unistd.h>

Timer::Timer( void )
{
    _secsPerTick = ( 1.0 / ( double )1'000'000 );

    setStartTick();
}

    #if defined( _POSIX_TIMERS ) &&       \
        ( _POSIX_TIMERS > 0 ) &&          \
        defined( _POSIX_MONOTONIC_CLOCK )
        #include <time.h>

Timer_t
Timer::tick() const
{
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    return ( ( osg::Timer_t )ts.tv_sec ) *
           1'000'000 +
           ( osg::Timer_t )ts.tv_nsec /
           1'000;
}
    #else
        #include <sys/time.h>

Timer_t
Timer::tick() const
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return ( ( osg::Timer_t )tv.tv_sec ) * 1'000'000 + ( osg::Timer_t )tv.tv_usec;
}
    #endif

#endif
