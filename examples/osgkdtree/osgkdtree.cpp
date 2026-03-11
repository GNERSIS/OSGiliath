/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgkdtree example application
 */
#include <iostream>
#include <osg/core/ApplicationUsage.hpp>
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/Timer.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/KdTree.hpp>
#include <osg/geometry/TriangleIndexFunctor.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgSim/ElevationSlice>
#include <osgSim/HeightAboveTerrain>
#include <osgSim/LineOfSight>
#include <osgUtil/culling/UpdateVisitor.hpp>
#include <osgUtil/intersection/IntersectionVisitor.hpp>
#include <osgUtil/intersection/LineSegmentIntersector.hpp>
#include <osgViewer/core/Viewer.hpp>

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    int                 maxNumLevels            = 16;
    int                 targetNumIndicesPerLeaf = 16;

    while( arguments.read( "--max", maxNumLevels ) )
    {
    }
    while( arguments.read( "--leaf", targetNumIndicesPerLeaf ) )
    {
    }

    osgDB::Registry::instance()->setBuildKdTreesHint(
        osgDB::ReaderWriter::Options::BUILD_KDTREES
    );

    osg::ref_ptr<osg::Node> scene = osgDB::readRefNodeFiles( arguments );

    if( !scene )
    {
        std::cout << "No model loaded, please specify a valid model on the command line."
                  << std::endl;
        return 0;
    }

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
    viewer.setSceneData( scene );
    return viewer.run();
}
