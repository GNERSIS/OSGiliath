/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Thread that processes a queue of Operation objects. Used for
 * background tasks like database paging and GL compilation.
 */
#pragma once

#include <list>
#include <osg/core/Object.hpp>
#include <osg/core/observer_ptr.hpp>
#include <osg/threading/Thread.hpp>
#include <set>

namespace osg
{

    class RefBlock : virtual public osg::Referenced,
                     public osg::Block
    {
        public:

            RefBlock() :
                osg::Referenced( true )
            {
            }
    };

    class RefBlockCount : virtual public osg::Referenced,
                          public osg::BlockCount
    {
        public:

            RefBlockCount( unsigned blockCount ) :
                osg::Referenced( true ),
                osg::BlockCount( blockCount )
            {
            }
    };

    /** Base class for implementing graphics operations.*/
    class Operation : virtual public Referenced
    {
        public:

            Operation( const std::string& name,
                       bool               keep ) :
                _name( name ),
                _keep( keep )
            {
            }

            /** Set the human readable name of the operation.*/
            void
            setName( const std::string& name )
            {
                _name = name;
            }

            /** Get the human readable name of the operation.*/
            const std::string&
            getName() const
            {
                return _name;
            }

            /** Set whether the operation should be kept once its been applied.*/
            void
            setKeep( bool keep )
            {
                _keep = keep;
            }

            /** Get whether the operation should be kept once its been applied.*/
            bool
            getKeep() const
            {
                return _keep;
            }

            /** if this operation is a barrier then release it.*/
            virtual void
            release()
            {
            }

            /** Do the actual task of this operation.*/
            virtual void
            operator()( Object* ) = 0;

        protected:

            Operation() :
                _keep( false )
            {
            }

            virtual ~Operation()
            {
            }

            std::string _name;
            bool        _keep;
    };

    class OperationThread;

    class OSG_EXPORT OperationQueue : public Referenced
    {
        public:

            OperationQueue();

            /** Get the next operation from the operation queue.
             * Return null ref_ptr<> if no operations are left in queue. */
            osg::ref_ptr<Operation>
            getNextOperation( bool blockIfEmpty = false );

            /** Return true if the operation queue is empty. */
            bool
            empty();

            /** Return the num of pending operations that are sitting in the
             * OperationQueue.*/
            unsigned int
            getNumOperationsInQueue();

            /** Add operation to end of OperationQueue, this will be
             * executed by the operation thread once this operation gets to the head of
             * the queue.*/
            void
            add( Operation* operation );

            /** Remove operation from OperationQueue.*/
            void
            remove( Operation* operation );

            /** Remove named operation from OperationQueue.*/
            void
            remove( const std::string& name );

            /** Remove all operations from OperationQueue.*/
            void
            removeAllOperations();

            /** Run the operations. */
            void
            runOperations( Object* callingObject = 0 );

            /** Call release on all operations. */
            void
            releaseAllOperations();

            /** Release operations block that is used to block threads that are waiting
             * on an empty operations queue.*/
            void
                                               releaseOperationsBlock();

            typedef std::set<OperationThread*> OperationThreads;

            /** Get the set of OperationThreads that are sharing this OperationQueue. */
            const OperationThreads&
            getOperationThreads() const
            {
                return _operationThreads;
            }

        protected:

            virtual ~OperationQueue();

            friend class OperationThread;

            void
            addOperationThread( OperationThread* thread );
            void
            removeOperationThread( OperationThread* thread );

            typedef std::list<osg::ref_ptr<Operation>> Operations;

            std::mutex                                 _operationsMutex;
            osg::ref_ptr<osg::RefBlock>                _operationsBlock;
            Operations                                 _operations;
            Operations::iterator                       _currentOperationIterator;

            OperationThreads                           _operationThreads;
    };

    /** OperationThread is a helper class for running Operation within a single thread.*/
    class OSG_EXPORT OperationThread : public Referenced,
                                       public osg::Thread
    {
        public:

            OperationThread();

            void
            setParent( Object* parent )
            {
                _parent = parent;
            }

            Object*
            getParent()
            {
                return _parent.get();
            }

            const Object*
            getParent() const
            {
                return _parent.get();
            }

            /** Set the OperationQueue. */
            void
            setOperationQueue( OperationQueue* opq );

            /** Get the OperationQueue. */
            OperationQueue*
            getOperationQueue()
            {
                return _operationQueue.get();
            }

            /** Get the const OperationQueue. */
            const OperationQueue*
            getOperationQueue() const
            {
                return _operationQueue.get();
            }

            /** Add operation to end of OperationQueue, this will be
             * executed by the graphics thread once this operation gets to the head of
             * the queue.*/
            void
            add( Operation* operation );

            /** Remove operation from OperationQueue.*/
            void
            remove( Operation* operation );

            /** Remove named operation from OperationQueue.*/
            void
            remove( const std::string& name );

            /** Remove all operations from OperationQueue.*/
            void
            removeAllOperations();

            /** Get the operation currently being run.*/
            osg::ref_ptr<Operation>
            getCurrentOperation()
            {
                return _currentOperation;
            }

            /** Run does the opertion thread run loop.*/
            virtual void
            run();

            void
            setDone( bool done );

            bool
            getDone() const
            {
                return _done != 0;
            }

            /** Cancel this graphics thread.*/
            virtual int
            cancel();

        protected:

            virtual ~OperationThread();

            observer_ptr<Object>         _parent;

            std::atomic<unsigned>        _done;

            std::mutex                   _threadMutex;
            osg::ref_ptr<OperationQueue> _operationQueue;
            osg::ref_ptr<Operation>      _currentOperation;
    };

    typedef OperationThread OperationsThread;

}
