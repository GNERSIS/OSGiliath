/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Pre-compilation callback for presentation slides.
 * Compiles GL objects before a slide is displayed to avoid stalls.
 */
#include <osgPresentation/CompileSlideCallback.hpp>

#include <osgUtil/utils/GLObjectsVisitor.hpp>

using namespace osgPresentation;

void
CompileSlideCallback::operator()( const osg::Camera& camera ) const
{
    osg::GraphicsContext* context =
        const_cast<osg::GraphicsContext*>( camera.getGraphicsContext() );
    if( !context )
    {
        return;
    }

    osg::State* state = context->getState();
    if( !state )
    {
        return;
    }

    const osg::FrameStamp* fs = state->getFrameStamp();
    if( !fs )
    {
        return;
    }

    if( _needCompile )
    {
        _frameNumber = fs->getFrameNumber();
        _needCompile = false;
    }

    if( _frameNumber != fs->getFrameNumber() )
    {
        return;
    }

    osgUtil::GLObjectsVisitor globjVisitor(
        osgUtil::GLObjectsVisitor::COMPILE_VERTEX_BUFFER_OBJECTS |
        osgUtil::GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES
    );

    globjVisitor.setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );

    globjVisitor.setNodeMaskOverride( 0XFF'FF'FF'FF );

    globjVisitor.setState( state );

    _sceneToCompile->accept( globjVisitor );
}
