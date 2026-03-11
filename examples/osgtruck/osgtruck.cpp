#include <iostream>
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

    // The milk truck has glTF animation (wheels spin) loaded automatically
    auto                model = osgDB::readRefNodeFile( "milk_truck.glb" );
    if( !model )
    {
        std::cerr << "Failed to load milk_truck.glb" << std::endl;
        return 1;
    }

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "milk_truck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( model );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.getCameraManipulator()->setHomePosition( osg::dvec3( 6.0, 4.0, 6.0 ),
                                                    osg::dvec3( 0.0, 1.0, 0.0 ),
                                                    osg::dvec3( 0.0, 1.0, 0.0 ) );
    viewer.addEventHandler( new PauseHandler( model.get() ) );
    return viewer.run();
}
