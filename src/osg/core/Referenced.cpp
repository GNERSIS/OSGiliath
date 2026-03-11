/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Intrusive reference counting base class. All OSG objects derive
 * from this. Works with ref_ptr<T> for automatic memory management.
 */
#include <osg/core/Referenced.hpp>

#include <memory>
#include <mutex>
#include <osg/core/ApplicationUsage.hpp>
#include <osg/core/DeleteHandler.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/Observer.hpp>
#include <set>
#include <stdlib.h>
#include <typeinfo>

namespace osg
{

    // #define ENFORCE_THREADSAFE
    // #define DEBUG_OBJECT_ALLOCATION_DESTRUCTION

    // specialized smart pointer, used to get round auto_ptr<>'s lack of the destructor
    // resetting itself to 0.
    template<typename T>
    struct ResetPointer
    {
            ResetPointer() :
                _ptr( 0 )
            {
            }

            ResetPointer( T* ptr ) :
                _ptr( ptr )
            {
            }

            ~ResetPointer()
            {
                delete _ptr;
                _ptr = 0;
            }

            inline ResetPointer&
            operator=( T* ptr )
            {
                if( _ptr == ptr )
                {
                    return *this;
                }
                delete _ptr;
                _ptr = ptr;
                return *this;
            }

            void
            reset( T* ptr )
            {
                if( _ptr == ptr )
                {
                    return;
                }
                delete _ptr;
                _ptr = ptr;
            }

            inline T&
            operator*()
            {
                return *_ptr;
            }

            inline const T&
            operator*() const
            {
                return *_ptr;
            }

            inline T*
            operator->()
            {
                return _ptr;
            }

            inline const T*
            operator->() const
            {
                return _ptr;
            }

            T*
            get()
            {
                return _ptr;
            }

            const T*
            get() const
            {
                return _ptr;
            }

            T* _ptr;
    };

    typedef ResetPointer<DeleteHandler> DeleteHandlerPointer;
    typedef ResetPointer<std::mutex>    GlobalMutexPointer;

    std::mutex*
    Referenced::getGlobalReferencedMutex()
    {
        static GlobalMutexPointer s_ReferencedGlobalMutext = new std::mutex;
        return s_ReferencedGlobalMutext.get();
    }

    // helper class for forcing the global mutex to be constructed when the library is
    // loaded.
    struct InitGlobalMutexes
    {
            InitGlobalMutexes()
            {
                Referenced::getGlobalReferencedMutex();
            }
    };

    static InitGlobalMutexes    s_initGlobalMutexes;

    // static std::auto_ptr<DeleteHandler> s_deleteHandler(0);
    static DeleteHandlerPointer s_deleteHandler( 0 );

    void
    Referenced::setDeleteHandler( DeleteHandler* handler )
    {
        s_deleteHandler.reset( handler );
    }

    DeleteHandler*
    Referenced::getDeleteHandler()
    {
        return s_deleteHandler.get();
    }

#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
    std::mutex&
    getNumObjectMutex()
    {
        static std::mutex s_numObjectMutex;
        return s_numObjectMutex;
    }

    static int s_numObjects = 0;
#endif

    Referenced::Referenced()
    {
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
        {
            std::lock_guard<std::mutex> lock( getNumObjectMutex() );
            ++s_numObjects;
            printf( "Object created, total num=%d\n", s_numObjects );
        }
#endif
    }

    Referenced::Referenced( bool /*threadSafeRefUnref*/ )
    {
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
        {
            std::lock_guard<std::mutex> lock( getNumObjectMutex() );
            ++s_numObjects;
            printf( "Object created, total num=%d\n", s_numObjects );
        }
#endif
    }

    Referenced::Referenced( const Referenced& )
    {
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
        {
            std::lock_guard<std::mutex> lock( getNumObjectMutex() );
            ++s_numObjects;
            printf( "Object created, total num=%d\n", s_numObjects );
        }
#endif
    }

    Referenced::~Referenced()
    {
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
        {
            std::lock_guard<std::mutex> lock( getNumObjectMutex() );
            --s_numObjects;
            printf( "Object created, total num=%d\n", s_numObjects );
        }
#endif

        if( _refCount.load( std::memory_order_relaxed ) > 0 )
        {
            OSG_WARN << "Warning: deleting still referenced object " << this
                     << " of type '" << typeid( this ).name() << "'" << std::endl;
            OSG_WARN << "         the final reference count was " << _refCount.load()
                     << ", memory corruption possible." << std::endl;
        }

        // signal observers that we are being deleted.
        signalObserversAndDelete( true, false );

        // delete the ObserverSet
        void* obsSet = _observerSet.load( std::memory_order_acquire );
        if( obsSet )
        {
            static_cast<ObserverSet*>( obsSet )->unref();
        }
    }

    ObserverSet*
    Referenced::getOrCreateObserverSet() const
    {
        ObserverSet* observerSet =
            static_cast<ObserverSet*>( _observerSet.load( std::memory_order_acquire ) );
        while( 0 == observerSet )
        {
            ObserverSet* newObserverSet = new ObserverSet( this );
            newObserverSet->ref();

            void* expected = nullptr;
            if( !_observerSet.compare_exchange_strong(
                    expected,
                    static_cast<void*>( newObserverSet ),
                    std::memory_order_acq_rel
                ) )
            {
                newObserverSet->unref();
            }

            observerSet = static_cast<ObserverSet*>(
                _observerSet.load( std::memory_order_acquire )
            );
        }
        return observerSet;
    }

    void
    Referenced::addObserver( Observer* observer ) const
    {
        getOrCreateObserverSet()->addObserver( observer );
    }

    void
    Referenced::removeObserver( Observer* observer ) const
    {
        getOrCreateObserverSet()->removeObserver( observer );
    }

    void
    Referenced::signalObserversAndDelete( bool signalDelete,
                                          bool doDelete ) const
    {
        ObserverSet* observerSet =
            static_cast<ObserverSet*>( _observerSet.load( std::memory_order_acquire ) );

        if( observerSet && signalDelete )
        {
            observerSet->signalObjectDeleted( const_cast<Referenced*>( this ) );
        }

        if( doDelete )
        {
            if( _refCount.load( std::memory_order_relaxed ) != 0 )
            {
                OSG_NOTICE << "Warning Referenced::signalObserversAndDelete(,,) doing "
                              "delete with _refCount="
                           << _refCount.load() << std::endl;
            }

            if( getDeleteHandler() )
            {
                deleteUsingDeleteHandler();
            }
            else
            {
                delete this;
            }
        }
    }

    int
    Referenced::unref_nodelete() const
    {
        return _refCount.fetch_sub( 1, std::memory_order_acq_rel ) - 1;
    }

    void
    Referenced::deleteUsingDeleteHandler() const
    {
        getDeleteHandler()->requestDelete( this );
    }

}    // end of namespace osg
