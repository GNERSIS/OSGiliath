/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Observer interface notified when a Referenced object is deleted.
 * Used by observer_ptr for weak-reference tracking.
 */
#include <osg/core/Notify.hpp>
#include <osg/core/ObserverNodePath.hpp>

using namespace osg;

Observer::Observer()
{
}

Observer::~Observer()
{
}

ObserverSet::ObserverSet( const Referenced* observedObject ) :
    _observedObject( const_cast<Referenced*>( observedObject ) )
{
    // OSG_NOTICE<<"ObserverSet::ObserverSet() "<<this<<std::endl;
}

ObserverSet::~ObserverSet()
{
    // OSG_NOTICE<<"ObserverSet::~ObserverSet() "<<this<<",
    // _observers.size()="<<_observers.size()<<std::endl;
}

void
ObserverSet::addObserver( Observer* observer )
{
    // OSG_NOTICE<<"ObserverSet::addObserver("<<observer<<") "<<this<<std::endl;
    std::lock_guard<std::mutex> lock( _mutex );
    _observers.insert( observer );
}

void
ObserverSet::removeObserver( Observer* observer )
{
    // OSG_NOTICE<<"ObserverSet::removeObserver("<<observer<<") "<<this<<std::endl;
    std::lock_guard<std::mutex> lock( _mutex );
    _observers.erase( observer );
}

Referenced*
ObserverSet::addRefLock()
{
    std::lock_guard<std::mutex> lock( _mutex );

    if( !_observedObject )
    {
        return 0;
    }

    int refCount = _observedObject->ref();
    if( refCount == 1 )
    {
        // The object is in the process of being deleted, but our
        // objectDeleted() method hasn't been run yet (and we're
        // blocking it -- and the final destruction -- with our lock).
        _observedObject->unref_nodelete();
        return 0;
    }

    return _observedObject;
}

void
ObserverSet::signalObjectDeleted( void* ptr )
{
    std::lock_guard<std::mutex> lock( _mutex );

    for( Observers::iterator itr = _observers.begin(); itr != _observers.end(); ++itr )
    {
        ( *itr )->objectDeleted( ptr );
    }
    _observers.clear();

    // reset the observed object so that we know that it's now detached.
    _observedObject = 0;
}
