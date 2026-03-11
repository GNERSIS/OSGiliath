/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osghangglide example application
 */
#include "GliderManipulator.hpp"

#include <iostream>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/ClearNode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/StateSet.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgUtil/culling/CullVisitor.hpp>
#include <osgViewer/core/Viewer.hpp>

extern osg::Node*
makeTerrain( void );
extern osg::Node*
makeTrees( void );
extern osg::Node*
makeTank( void );
extern osg::Node*
makeWindsocks( void );
extern osg::Node*
makeGliders( void );
extern osg::Node*
makeGlider( void );
extern osg::Node*
makeSky( void );
extern osg::Node*
makeBase( void );
extern osg::Node*
makeClouds( void );

class MoveEarthySkyWithEyePointTransform : public osg::Transform
{
    public:

        /** Get the transformation matrix which moves from local coords to world
         * coords.*/
        virtual bool
        computeLocalToWorldMatrix( osg::dmat4&       matrix,
                                   osg::NodeVisitor* nv ) const
        {
            osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>( nv );
            if( cv )
            {
                osg::vec3 eyePointLocal = cv->getEyeLocal();
                osg::preMultTranslate(
                    matrix,
                    osg::dvec3( eyePointLocal.x, eyePointLocal.y, 0.0 )
                );
            }
            return true;
        }

        /** Get the transformation matrix which moves from world coords to local
         * coords.*/
        virtual bool
        computeWorldToLocalMatrix( osg::dmat4&       matrix,
                                   osg::NodeVisitor* nv ) const
        {
            std::cout << "computing transform" << std::endl;

            osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>( nv );
            if( cv )
            {
                osg::vec3 eyePointLocal = cv->getEyeLocal();
                osg::postMultTranslate(
                    matrix,
                    osg::dvec3( -eyePointLocal.x, -eyePointLocal.y, 0.0 )
                );
            }
            return true;
        }
};

osg::Group*
createModel()
{
    // no database loaded so automatically create Ed Levin Park
    osg::Group*     group = new osg::Group;

    // the base and sky subgraphs go to set the earth sky of the
    // model and clear the color and depth buffer for us, by using
    // osg::Depth, and setting their bin numbers to less than 0,
    // to force them to draw before the rest of the scene.

    osg::ClearNode* clearNode = new osg::ClearNode;
    clearNode->setRequiresClear( false );    // we've got base and sky to do it.

    // use a transform to make the sky and base move around with the eye point.
    osg::Transform* transform = new MoveEarthySkyWithEyePointTransform;

    // transform's value isn't knowm until in the cull traversal so its bounding
    // volume is can't be determined, therefore culling will be invalid,
    // so switch it off, this causes all our paresnts to switch culling
    // off as well. But don't worry, culling will be back on once underneath
    // this node or any other branch above this transform.
    transform->setCullingActive( false );

    // add the sky and base layer.
    transform->addChild( makeSky() );     // bin number -2 so drawn first.
    transform->addChild( makeBase() );    // bin number -1 so draw second.

    // add the transform to the earth sky.
    clearNode->addChild( transform );

    // add to earth sky to the scene.
    group->addChild( clearNode );

    // the rest of the scene is drawn after the base and sky above.
    group->addChild(
        makeTrees()
    );    // will drop into a transparent, depth sorted bin (1)
    group->addChild( makeTerrain() );    // will drop into default bin - state sorted 0
    group->addChild( makeTank() );       // will drop into default bin - state sorted 0
    // add the following in the future...
    // makeGliders
    // makeClouds

    return group;
}

int
main( int    argc,
      char** argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the example which demonstrates how to create a scene programmatically, in "
        "this case a hang gliding flying site."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;

    // if user requests help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    bool customWindows = false;
    while( arguments.read( "-2" ) )
    {
        customWindows = true;
    }

    if( customWindows )
    {
        osg::GraphicsContext::WindowingSystemInterface* wsi =
            osg::GraphicsContext::getWindowingSystemInterface();
        if( !wsi )
        {
            osg::notify( osg::NOTICE )
                << "View::setUpViewAcrossAllScreens() : Error, no WindowSystemInterface "
                   "available, cannot create windows."
                << std::endl;
            return 0;
        }

        osg::ref_ptr<osg::GraphicsContext::Traits> traits =
            new osg::GraphicsContext::Traits;
        traits->x                = 1'000;
        traits->y                = 100;
        traits->width            = 640;
        traits->height           = 480;
        traits->windowDecoration = true;
        traits->doubleBuffer     = true;
        traits->sharedContext    = 0;
        traits->readDISPLAY();
        traits->setUndefinedScreenDetailsToDefaultScreen();

        osg::ref_ptr<osg::GraphicsContext> gc =
            osg::GraphicsContext::createGraphicsContext( traits.get() );
        if( gc.valid() )
        {
            // need to ensure that the window is cleared make sure that the complete
            // window is set the correct colour rather than just the parts of the window
            // that are under the camera's viewports
            gc->setClearColor( osg::vec4( 0.2F, 0.2F, 0.6F, 1.0F ) );
            gc->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        }
        else
        {
            osg::notify( osg::NOTICE )
                << "  GraphicsWindow has not been created successfully." << std::endl;
        }

        unsigned int numCameras       = 2;
        double       aspectRatioScale = 1.0;
        for( unsigned int i = 0; i < numCameras; ++i )
        {
            osg::ref_ptr<osg::Camera> camera = new osg::Camera;
            camera->setGraphicsContext( gc.get() );
            camera->setViewport( new osg::Viewport( ( i * traits->width ) / numCameras,
                                                    ( i * traits->height ) / numCameras,
                                                    traits->width / numCameras,
                                                    traits->height / numCameras ) );
            GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
            camera->setDrawBuffer( buffer );
            camera->setReadBuffer( buffer );

            viewer.addSlave( camera.get(),
                             osg::dmat4(),
                             osg::scale( aspectRatioScale, 1.0, 1.0 ) );
        }
    }
    else
    {
        viewer.setUpViewInWindow( 1'000, 100, 640, 480 );
    }

    // set up the camera manipulation with our custom manipultor
    viewer.setCameraManipulator( new GliderManipulator() );

    // pass the scene graph to the viewer
    viewer.setSceneData( createModel() );
    return viewer.run();
}
