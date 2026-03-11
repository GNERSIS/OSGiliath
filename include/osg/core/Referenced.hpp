/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Intrusive reference counting base class. All OSG objects derive
 * from this. Works with ref_ptr<T> for automatic memory management.
 */
#pragma once

#include <atomic>
#include <mutex>
#include <osg/core/Export.hpp>

namespace osg
{

    // forward declare, declared after Referenced below.
    class DeleteHandler;
    class Observer;
    class ObserverSet;
    class State;

    /** template class to help enforce static initialization order. */
    template<typename T, T M()>
    struct depends_on
    {
            depends_on()
            {
                M();
            }
    };

    /** Base class for providing reference counted objects.*/
    class OSG_EXPORT Referenced
    {

        public:

            Referenced();

            /** Deprecated, Referenced is now always uses thread safe ref/unref, use
             * default Referenced() constructor instead */
            explicit Referenced( bool threadSafeRefUnref );

            Referenced( const Referenced& );

            inline Referenced&
            operator=( const Referenced& )
            {
                return *this;
            }

            /** Deprecated, Referenced is always theadsafe so there method now has no
             * effect and does not need to be called.*/
            virtual void
            setThreadSafeRefUnref( bool /*threadSafe*/ )
            {
            }

            /** Get whether a mutex is used to ensure ref() and unref() are thread
             * safe.*/
            bool
            getThreadSafeRefUnref() const noexcept
            {
                return true;
            }

            /** Get the mutex used to ensure thread safety of ref()/unref(). */
            std::mutex*
            getRefMutex() const
            {
                return getGlobalReferencedMutex();
            }

            /** Get the optional global Referenced mutex, this can be shared between all
             * osg::Referenced.*/
            static std::mutex*
            getGlobalReferencedMutex();

            /** Increment the reference count by one, indicating that
                this object has another pointer which is referencing it.*/
            inline int
            ref() const noexcept;

            /** Decrement the reference count by one, indicating that
                a pointer to this object is no longer referencing it.  If the
                reference count goes to zero, it is assumed that this object
                is no longer referenced and is automatically deleted.*/
            inline int
            unref() const noexcept;

            /** Decrement the reference count by one, indicating that
                a pointer to this object is no longer referencing it.  However, do
                not delete it, even if ref count goes to 0.  Warning, unref_nodelete()
                should only be called if the user knows exactly who will
                be responsible for, one should prefer unref() over unref_nodelete()
                as the latter can lead to memory leaks.*/
            int
            unref_nodelete() const;

            /** Return the number of pointers currently referencing this object. */
            inline int
            referenceCount() const noexcept
            {
                return _refCount.load( std::memory_order_relaxed );
            }

            /** Get the ObserverSet if one is attached, otherwise return NULL.*/
            ObserverSet*
            getObserverSet() const noexcept
            {
                return static_cast<ObserverSet*>(
                    _observerSet.load( std::memory_order_acquire )
                );
            }

            /** Get the ObserverSet if one is attached, otherwise create an ObserverSet,
             * attach it, then return this newly created ObserverSet.*/
            ObserverSet*
            getOrCreateObserverSet() const;

            /** Add a Observer that is observing this object, notify the Observer when
             * this object gets deleted.*/
            void
            addObserver( Observer* observer ) const;

            /** Remove Observer that is observing this object.*/
            void
            removeObserver( Observer* observer ) const;

        public:

            friend class DeleteHandler;

            /** Set a DeleteHandler to which deletion of all referenced counted objects
             * will be delegated.*/
            static void
            setDeleteHandler( DeleteHandler* handler );

            /** Get a DeleteHandler.*/
            static DeleteHandler*
            getDeleteHandler();

        protected:

            virtual ~Referenced();

            void
            signalObserversAndDelete( bool signalDelete,
                                      bool doDelete ) const;

            void
                                       deleteUsingDeleteHandler() const;

            mutable std::atomic<void*> _observerSet{ nullptr };
            mutable std::atomic<int>   _refCount{ 0 };
    };

    inline int
    Referenced::ref() const noexcept
    {
        return _refCount.fetch_add( 1, std::memory_order_relaxed ) + 1;
    }

    inline int
    Referenced::unref() const noexcept
    {
        int  newRef     = _refCount.fetch_sub( 1, std::memory_order_acq_rel ) - 1;
        bool needDelete = ( newRef == 0 );

        if( needDelete )
        {
            signalObserversAndDelete( true, true );
        }
        return newRef;
    }

    // intrusive_ptr_add_ref and intrusive_ptr_release allow
    // use of osg Referenced classes with boost::intrusive_ptr
    inline void
    intrusive_ptr_add_ref( Referenced* p )
    {
        p->ref();
    }

    inline void
    intrusive_ptr_release( Referenced* p )
    {
        p->unref();
    }

}
