/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "MainPass.hpp"
#include "ShadowPass.hpp"
#include "SponzaCameras.hpp"
#include "SponzaFrameContext.hpp"
#include "SponzaLighting.hpp"
#include "SponzaOptions.hpp"
#include "SponzaTargets.hpp"
#include "SsaoPass.hpp"
#include "TonemapPass.hpp"

#include <iostream>
#include <osg/core/ArgumentParser.hpp>
#include <osg/maths/transform.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/FirstPersonManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser   arguments( &argc, argv );

    sponza::SponzaOptions options;
    if( !sponza::parseSponzaOptions( arguments, options ) )
    {
        return 1;
    }

    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile( options.modelPath );
    if( !model )
    {
        std::cerr << "Failed to load " << options.modelPath << std::endl;
        return 1;
    }

    const osg::dmat4 rttView          = sponza::makeViewMatrix( options.camera );
    const osg::dmat4 projectionMatrix = sponza::makeProjectionMatrix( options.camera );
    osg::ref_ptr<osg::Texture2D> envTexture =
        sponza::applySunAndIbl( model.get(), options, rttView );

    sponza::SponzaTargets            targets = sponza::createSponzaTargets();
    const sponza::SponzaFrameContext frame{
        rttView,
        projectionMatrix,
        osg::inverse( projectionMatrix ),
        sponza::makeViewToWorldRotation( rttView ),
        envTexture,
        options.envRotation
    };

    sponza::ShadowPassResult shadowPass =
        sponza::createShadowPass( model.get(), options, frame );
    osg::ref_ptr<osg::Camera> rtt =
        sponza::createRttCamera( model.get(), options, targets, frame );
    sponza::applyShadowReceiverState( rtt->getOrCreateStateSet(),
                                      options,
                                      shadowPass.shadowTexture.get(),
                                      shadowPass.shadowMatrix,
                                      shadowPass.lightSpaceExtent,
                                      shadowPass.hasShadow );
    osg::ref_ptr<osg::Camera> ssao = sponza::createSsaoCamera( options, targets, frame );
    osg::ref_ptr<osg::Camera> tonemapCamera =
        sponza::createTonemapCamera( options, targets, frame );

    osg::ref_ptr<osg::Group> root = new osg::Group;
    if( shadowPass.camera )
    {
        root->addChild( shadowPass.camera.get() );
    }
    root->addChild( rtt.get() );
    root->addChild( ssao.get() );
    root->addChild( tonemapCamera.get() );

    if( options.headless )
    {
        return osg::headlessCapture( root.get(),
                                     options.headlessOutput,
                                     sponza::renderWidth,
                                     sponza::renderHeight,
                                     options.camera.eye,
                                     options.camera.center,
                                     options.camera.up )
                 ? 0
                 : 1;
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.setCameraManipulator( new osgGA::FirstPersonManipulator );
    viewer.getCameraManipulator()->setHomePosition( options.camera.eye,
                                                    options.camera.center,
                                                    options.camera.up );

    return viewer.run();
}
