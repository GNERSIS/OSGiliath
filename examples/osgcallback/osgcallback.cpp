/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgcallback example application
 */
#include <iostream>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Billboard.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

class UpdateCallback : public osg::NodeCallback
{
        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            std::cout << "update callback - pre traverse" << node << std::endl;
            traverse( node, nv );
            std::cout << "update callback - post traverse" << node << std::endl;
        }
};

class CullCallback : public osg::NodeCallback
{
        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            std::cout << "cull callback - pre traverse" << node << std::endl;
            traverse( node, nv );
            std::cout << "cull callback - post traverse" << node << std::endl;
        }
};

class DrawableDrawCallback : public osg::Drawable::DrawCallback
{
        virtual void
        drawImplementation( osg::RenderInfo&     renderInfo,
                            const osg::Drawable* drawable ) const
        {
            std::cout << "draw call back - pre drawImplementation" << drawable
                      << std::endl;
            drawable->drawImplementation( renderInfo );
            std::cout << "draw call back - post drawImplementation" << drawable
                      << std::endl;
        }
};

struct DrawableUpdateCallback : public osg::DrawableUpdateCallback
{
        virtual void
        update( osg::NodeVisitor*,
                osg::Drawable* drawable )
        {
            std::cout << "Drawable update callback " << drawable << std::endl;
        }
};

struct DrawableCullCallback : public osg::DrawableCullCallback
{
        /** do customized cull code.*/
        virtual bool
        cull( osg::NodeVisitor*,
              osg::Drawable* drawable,
              osg::State* /*state*/ ) const
        {
            std::cout << "Drawable cull callback " << drawable << std::endl;
            return false;
        }
};

class InsertCallbacksVisitor : public osg::DualModeVisitor
{

    public:

        InsertCallbacksVisitor() :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
        {
        }

        virtual void
        apply( osg::Node& node )
        {
            node.setUpdateCallback( new UpdateCallback() );
            node.setCullCallback( new CullCallback() );
            traverse( node );
        }

        virtual void
        apply( osg::Geode& geode )
        {
            geode.setUpdateCallback( new UpdateCallback() );

            // note, it makes no sense to attach a cull callback to the node
            // at there are no nodes to traverse below the geode, only
            // drawables, and as such the Cull node callbacks is ignored.
            // If you wish to control the culling of drawables
            // then use a drawable cullback...

            for( unsigned int i = 0; i < geode.getNumDrawables(); ++i )
            {
                geode.getDrawable( i )->setUpdateCallback(
                    new DrawableUpdateCallback()
                );
                geode.getDrawable( i )->setCullCallback( new DrawableCullCallback() );
                geode.getDrawable( i )->setDrawCallback( new DrawableDrawCallback() );
            }
        }

        virtual void
        apply( osg::Transform& node )
        {
            apply( ( osg::Node& )node );
        }
};

class MyReadFileCallback : public osgDB::Registry::ReadFileCallback
{
    public:

        virtual osgDB::ReaderWriter::ReadResult
        readNode( const std::string&                  fileName,
                  const osgDB::ReaderWriter::Options* options )
        {
            std::cout << "before readNode" << std::endl;
            // note when calling the Registry to do the read you have to call
            // readNodeImplementation NOT readNode, as this will cause on infinite
            // recusive loop.
            osgDB::ReaderWriter::ReadResult result =
                osgDB::Registry::instance()->readNodeImplementation( fileName, options );
            std::cout << "after readNode" << std::endl;
            return result;
        }
};

class CameraUpdateCallback : public osg::NodeCallback
{
        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            std::cout << "Camera update callback - pre traverse" << node << std::endl;
            traverse( node, nv );
            std::cout << "Camera update callback - post traverse" << node << std::endl;
        }
};

class CameraEventCallback : public osg::NodeCallback
{
        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            std::cout << "Camera event callback - pre traverse" << node << std::endl;
            traverse( node, nv );
            std::cout << "Camera event callback - post traverse" << node << std::endl;
        }
};

struct TestDrawableUpdateCallback : public osg::DrawableUpdateCallback
{
        TestDrawableUpdateCallback( const std::string& message ) :
            _message( message )
        {
        }

        virtual void
        update( osg::NodeVisitor*,
                osg::Drawable* /*drw*/ )
        {
            printf( "%s\n", _message.c_str() );
        }

        std::string _message;
};

struct TestNodeUpdateCallback : public osg::NodeCallback
{
        TestNodeUpdateCallback( const std::string& message ) :
            _message( message )
        {
        }

        virtual void
        operator()( osg::Node* /*node*/,
                    osg::NodeVisitor* /*nv*/ )
        {
            printf( "%s\n", _message.c_str() );
        }

        std::string _message;
};

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set the osgDB::Registry read file callback to catch all requests for reading
    // files.
    osgDB::Registry::instance()->setReadFileCallback( new MyReadFileCallback() );

    // initialize the viewer.
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

    osgViewer::Viewer       viewer;

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> rootnode;

    if( arguments.read( "--test" ) )
    {
        osg::ref_ptr<osg::Group> root = new osg::Group();
        rootnode                      = root;

        osg::Node* test1              = new osg::Node();
        test1->setUpdateCallback( new TestNodeUpdateCallback( "test1" ) );
        root->addChild( test1 );

        osg::Drawable* test2 = new osg::Drawable();
        test2->osg::Node::setUpdateCallback( new TestNodeUpdateCallback( "test2" ) );
        root->addChild( test2 );

        osg::Drawable* test3 = new osg::Drawable();
        test3->setUpdateCallback( new TestDrawableUpdateCallback( "test3" ) );
        root->addChild( test3 );

        osg::Geode*    test4     = new osg::Geode();
        osg::Drawable* drawable1 = new osg::Drawable();
        drawable1->osg::Node::setUpdateCallback( new TestNodeUpdateCallback( "test4" ) );
        test4->addDrawable( drawable1 );
        root->addChild( test4 );

        osg::Geode*    test5     = new osg::Geode();
        osg::Drawable* drawable2 = new osg::Drawable();
        drawable2->setUpdateCallback( new TestDrawableUpdateCallback( "test5" ) );
        test5->addDrawable( drawable2 );
        root->addChild( test5 );

        osg::Geode*    test6     = new osg::Geode();
        osg::Drawable* drawable3 = new osg::Drawable();
        drawable3->setUpdateCallback( new TestDrawableUpdateCallback( "test6" ) );
        test6->addChild( drawable3 );
        root->addChild( test6 );

        osg::Geode*    test7     = new osg::Geode();
        osg::Drawable* drawable4 = new osg::Drawable();
        drawable4->osg::Node::setUpdateCallback( new TestNodeUpdateCallback( "test7" ) );
        test7->addChild( drawable4 );
        root->addChild( test7 );

        printf( "Numchildren with updates %u\n",
                rootnode->getNumChildrenRequiringUpdateTraversal() );
    }
    else
    {
        rootnode = osgDB::readRefNodeFiles( arguments );

        // if not loaded assume no arguments passed in, try use default mode instead.
        if( !rootnode )
        {
            rootnode = osgDB::readRefNodeFile( "duck.glb" );
        }

        if( !rootnode )
        {
            osg::notify( osg::NOTICE )
                << "Please specify a file on the command line" << std::endl;

            return 1;
        }

        // run optimization over the scene graph
        osgUtil::Optimizer optimzer;
        optimzer.optimize( rootnode.get() );

        // insert all the callbacks
        InsertCallbacksVisitor icv;
        rootnode->accept( icv );
    }

    viewer.getCamera()->setUpdateCallback( new CameraUpdateCallback() );
    viewer.getCamera()->setEventCallback( new CameraEventCallback() );

    // set the scene to render
    viewer.setSceneData( rootnode.get() );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    viewer.realize();

    while( !viewer.done() )
    {
        OSG_NOTICE << std::endl << "New Frame" << std::endl;
        viewer.frame();
    }

    return 0;
}
