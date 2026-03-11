/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Dedicated thread for a graphics context. Runs GL operations
 * (draw, compile, swap) in sequence for its associated context.
 */
#include <osg/threading/GraphicsThread.hpp>

#include <osg/core/Notify.hpp>
#include <osg/rendering/GLObjects.hpp>
#include <osg/rendering/GraphicsContext.hpp>

using namespace osg;

GraphicsThread::GraphicsThread()
{
}

void
GraphicsThread::run()
{
    // make the graphics context current.
    GraphicsContext* graphicsContext = dynamic_cast<GraphicsContext*>( _parent.get() );
    if( graphicsContext )
    {
        graphicsContext->makeCurrent();

        graphicsContext->getState()->initializeExtensionProcs();
    }

    OperationThread::run();

    // release operations before the thread stops working.
    _operationQueue->releaseAllOperations();

    if( graphicsContext )
    {
        graphicsContext->releaseContext();
    }
}

void
GraphicsOperation::operator()( Object* object )
{
    osg::GraphicsContext* context = dynamic_cast<osg::GraphicsContext*>( object );
    if( context )
    {
        operator()( context );
    }
}

void
SwapBuffersOperation::operator()( GraphicsContext* context )
{
    context->swapBuffersCallbackOrImplementation();
    context->clear();
}

void
BarrierOperation::release()
{
    Barrier::release();
}

void
BarrierOperation::operator()( Object* /*object*/ )
{
    if( _preBlockOp != NO_OPERATION )
    {
        if( _preBlockOp == GL_FLUSH )
        {
            glFlush();
        }
        else if( _preBlockOp == GL_FINISH )
        {
            glFinish();
        }
    }

    block();
}

void
ReleaseContext_Block_MakeCurrentOperation::release()
{
    Block::release();
}

void
ReleaseContext_Block_MakeCurrentOperation::operator()( GraphicsContext* context )
{
    // release the graphics context.
    context->releaseContext();

    // reset the block so that it the next call to block()
    reset();

    // block this thread, until the block is released externally.
    block();

    // re acquire the graphics context.
    context->makeCurrent();
}

BlockAndFlushOperation::BlockAndFlushOperation() :
    osg::Referenced( true ),
    GraphicsOperation( "Block",
                       false )
{
    reset();
}

void
BlockAndFlushOperation::release()
{
    Block::release();
}

void
BlockAndFlushOperation::operator()( GraphicsContext* )
{
    glFlush();
    Block::release();
}

FlushDeletedGLObjectsOperation::FlushDeletedGLObjectsOperation( double availableTime,
                                                                bool   keep ) :
    osg::Referenced( true ),
    GraphicsOperation( "FlushDeletedGLObjectsOperation",
                       keep ),
    _availableTime( availableTime )
{
}

void
FlushDeletedGLObjectsOperation::operator()( GraphicsContext* context )
{
    State*            state         = context->getState();
    unsigned int      contextID     = state ? state->getContextID() : 0;
    const FrameStamp* frameStamp    = state ? state->getFrameStamp() : 0;
    double            currentTime   = frameStamp ? frameStamp->getReferenceTime() : 0.0;
    double            availableTime = _availableTime;

    flushDeletedGLObjects( contextID, currentTime, availableTime );
}

void
RunOperations::operator()( osg::GraphicsContext* context )
{
    context->runOperations();
}

/////////////////////////////////////////////////////////////////////////////
//
// EndOfDynamicDrawBlock
//
EndOfDynamicDrawBlock::EndOfDynamicDrawBlock( unsigned int numberOfBlocks ) :
    BlockCount( numberOfBlocks )
{
}

void
EndOfDynamicDrawBlock::completed( osg::State* /*state*/ )
{
    BlockCount::completed();
}
