/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "MainPass.hpp"
#include "SponzaFrameContext.hpp"
#include "SponzaOptions.hpp"
#include "SponzaPassOrder.hpp"
#include "SponzaTargets.hpp"

#include <osg/GL>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Node.hpp>

namespace sponza
{

    osg::ref_ptr<osg::Camera>
    createRttCamera( osg::Node*                model,
                     const SponzaOptions&      options,
                     SponzaTargets&            targets,
                     const SponzaFrameContext& frame )
    {
        osg::ref_ptr<osg::Camera> rtt = new osg::Camera;
        rtt->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
        rtt->attach( osg::Camera::COLOR_BUFFER0, targets.hdrColor.get() );
        if( targets.indirectColor.valid() )
        {
            rtt->attach( osg::Camera::COLOR_BUFFER1, targets.indirectColor.get() );
        }
        rtt->attach( osg::Camera::DEPTH_BUFFER, targets.sceneDepth.get() );
        rtt->setViewport( 0,
                          0,
                          renderTargetWidth( options ),
                          renderTargetHeight( options ) );
        rtt->setRenderOrder( osg::Camera::PRE_RENDER, mainPassOrder );
        rtt->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        rtt->setClearColor( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
        rtt->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        rtt->setProjectionMatrix( frame.proj );
        rtt->setViewMatrix( frame.view );
        rtt->addChild( model );
        return rtt;
    }

}
