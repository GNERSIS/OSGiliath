#include <iostream>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/traversal/AnimationPath.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <vector>

struct CollectAnimCallbacks : public osg::NodeVisitor
{
        std::vector<osg::ref_ptr<osg::AnimationPathCallback>> callbacks;

        CollectAnimCallbacks() :
            osg::NodeVisitor( TRAVERSE_ALL_CHILDREN )
        {
        }

        void
        apply( osg::Node& node ) override
        {
            if( auto* cb = dynamic_cast<osg::AnimationPathCallback*>(
                    node.getUpdateCallback()
                ) )
            {
                callbacks.push_back( cb );
            }
            traverse( node );
        }
};

struct PauseHandler : public osgGA::GUIEventHandler
{
        std::vector<osg::ref_ptr<osg::AnimationPathCallback>> _callbacks;
        bool                                                  _paused = false;

        PauseHandler( osg::Node* scene )
        {
            CollectAnimCallbacks collector;
            scene->accept( collector );
            _callbacks = std::move( collector.callbacks );
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& ) override
        {
            if( ea.getEventType() ==
                osgGA::GUIEventAdapter::KEYDOWN &&
                ea.getKey() == ' ' )
            {
                _paused = !_paused;
                for( auto& cb : _callbacks )
                {
                    cb->setPause( _paused );
                }
                return true;
            }
            return false;
        }
};

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    auto                model = osgDB::readRefNodeFile( "avocado.glb" );
    if( !model )
    {
        std::cerr << "Failed to load avocado.glb" << std::endl;
        return 1;
    }

    // Wrap model in a turntable MatrixTransform that rotates around Z
    auto* turntable = new osg::MatrixTransform;
    turntable->addChild( model );
    turntable->setUpdateCallback(
        new osg::AnimationPathCallback( osg::dvec3( 0, 0, 0 ),
                                        osg::dvec3( 0, 0, 1 ),
                                        osg::radians( 30.0F ) )
    );

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "avocado.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( turntable );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.getCameraManipulator()->setHomePosition( osg::dvec3( 0.1, 0.05, 0.1 ),
                                                    osg::dvec3( 0.0, 0.03, 0.0 ),
                                                    osg::dvec3( 0.0, 1.0, 0.0 ) );
    viewer.addEventHandler( new PauseHandler( turntable ) );
    return viewer.run();
}
