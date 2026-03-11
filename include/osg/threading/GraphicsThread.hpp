/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Dedicated thread for a graphics context. Runs GL operations
 * (draw, compile, swap) in sequence for its associated context.
 */
#pragma once

#include <osg/state/State.hpp>
#include <osg/threading/OperationThread.hpp>

namespace osg
{

    class GraphicsContext;

    /** GraphicsThread is a helper class for running OpenGL GraphicsOperation within a
     * single thread assigned to a specific GraphicsContext.*/
    class OSG_EXPORT GraphicsThread : public osg::OperationThread
    {
        public:

            GraphicsThread();

            /** Run does the graphics thread run loop.*/
            virtual void
            run();
    };

    struct OSG_EXPORT GraphicsOperation : public Operation
    {
            GraphicsOperation( const std::string& name,
                               bool               keep ) :
                Operation( name,
                           keep )
            {
            }

            /** Override the standard Operation operator and dynamic cast object to a
             * GraphicsContext, on success call operation()(GraphicsContext*).*/
            virtual void
            operator()( Object* object );

            virtual void
            operator()( GraphicsContext* context ) = 0;

            /** Resize any per context GLObject buffers to specified size. */
            virtual void
            resizeGLObjectBuffers( unsigned int /*maxSize*/ )
            {
            }

            /** If State is non-zero, this function releases any associated OpenGL
             * objects for the specified graphics context. Otherwise, releases OpenGL
             * objects for all graphics contexts. */
            virtual void
            releaseGLObjects( osg::State* = 0 ) const
            {
            }
    };

    /** SwapBufferOperation calls swap buffers on the GraphicsContext.*/
    struct OSG_EXPORT SwapBuffersOperation : public GraphicsOperation
    {
            SwapBuffersOperation() :
                osg::Referenced( true ),
                GraphicsOperation( "SwapBuffers",
                                   true )
            {
            }

            virtual void
            operator()( GraphicsContext* context );
    };

    /** BarrierOperation allows one to synchronize multiple GraphicsThreads with each
     * other.*/
    struct OSG_EXPORT BarrierOperation : public Operation,
                                         public osg::Barrier
    {
            enum PreBlockOp
            {
                NO_OPERATION,
                GL_FLUSH,
                GL_FINISH,
            };

            BarrierOperation( int        numThreads,
                              PreBlockOp op   = NO_OPERATION,
                              bool       keep = true ) :
                osg::Referenced( true ),
                Operation( "Barrier",
                           keep ),
                osg::Barrier( numThreads ),
                _preBlockOp( op )
            {
            }

            virtual void
            release();

            virtual void
                       operator()( Object* object );

            PreBlockOp _preBlockOp;
    };

    /** ReleaseContext_Block_MakeCurrentOperation releases the context for another thread
     * to acquire, then blocks waiting for context to be released, once the block is
     * release the context is re-acquired.*/
    struct OSG_EXPORT ReleaseContext_Block_MakeCurrentOperation
        : public GraphicsOperation,
          public RefBlock
    {
            ReleaseContext_Block_MakeCurrentOperation() :
                osg::Referenced( true ),
                GraphicsOperation( "ReleaseContext_Block_MakeCurrent",
                                   false )
            {
            }

            virtual void
            release();

            virtual void
            operator()( GraphicsContext* context );
    };

    struct OSG_EXPORT BlockAndFlushOperation : public GraphicsOperation,
                                               public osg::Block
    {
            BlockAndFlushOperation();

            virtual void
            release();

            virtual void
            operator()( GraphicsContext* );
    };

    struct OSG_EXPORT FlushDeletedGLObjectsOperation : public GraphicsOperation
    {
            FlushDeletedGLObjectsOperation( double availableTime,
                                            bool   keep = false );

            virtual void
                   operator()( GraphicsContext* );

            double _availableTime;
    };

    class OSG_EXPORT RunOperations : public osg::GraphicsOperation
    {
        public:

            RunOperations() :
                osg::GraphicsOperation( "RunOperation",
                                        true )
            {
            }

            virtual void
            operator()( osg::GraphicsContext* context );
    };

    class OSG_EXPORT EndOfDynamicDrawBlock
        : public osg::BlockCount,
          public osg::State::DynamicObjectRenderingCompletedCallback
    {
        public:

            EndOfDynamicDrawBlock( unsigned int );

            void
            completed( osg::State* state );

        protected:

            ~EndOfDynamicDrawBlock()
            {
            }
    };

}
