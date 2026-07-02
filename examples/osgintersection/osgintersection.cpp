/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgintersection example application
 */
#include <iostream>
#include <osg/core/ApplicationUsage.hpp>
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/Timer.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgSim/ElevationSlice.hpp>
#include <osgSim/HeightAboveTerrain.hpp>
#include <osgSim/LineOfSight.hpp>
#include <osgUtil/intersection/IntersectionVisitor.hpp>
#include <osgUtil/intersection/LineSegmentIntersector.hpp>

struct MyReadCallback : public osgUtil::IntersectionVisitor::ReadCallback
{
#if 0
    virtual osg::Node* readNodeFile(const std::string& filename)
    {
        return osgDB::readRefNodeFile(filename).release();
    }
#endif
        virtual osg::ref_ptr<osg::Node>
        readNodeFile( const std::string& filename )
        {
            return osgDB::readRefNodeFile( filename );
        }
};

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser     arguments( &argc, argv );

    osg::ref_ptr<osg::Node> scene = osgDB::readRefNodeFiles( arguments );

    if( !scene )
    {
        std::cout << "No model loaded, please specify a valid model on the command line."
                  << std::endl;
        return 0;
    }

    std::cout << "Intersection " << std::endl;

    osg::sphere bs                  = scene->getBound();

    bool        useIntersectorGroup = true;
    bool        useLineOfSight      = true;

    // osg::CoordinateSystemNode* csn =
    // dynamic_cast<osg::CoordinateSystemNode*>(scene.get()); osg::EllipsoidModel* em =
    // csn ? csn->getEllipsoidModel() : 0;

    if( useLineOfSight )
    {

        osg::dvec3          start = bs.center + osg::dvec3( 0.0, bs.radius, 0.0 );
        osg::dvec3          end   = bs.center - osg::dvec3( 0.0, bs.radius, 0.0 );
        osg::dvec3          deltaRow( 0.0, 0.0, bs.radius * 0.01 );
        osg::dvec3          deltaColumn( bs.radius * 0.01, 0.0, 0.0 );

        osgSim::LineOfSight los;

#if 1
        unsigned int               numRows    = 20;
        unsigned int               numColumns = 20;
        osgSim::HeightAboveTerrain hat;
        hat.setDatabaseCacheReadCallback( los.getDatabaseCacheReadCallback() );

        for( unsigned int r = 0; r < numRows; ++r )
        {
            for( unsigned int c = 0; c < numColumns; ++c )
            {
                osg::dvec3 s =
                    start + deltaColumn * double( c ) + deltaRow * double( r );
                osg::dvec3 e = end + deltaColumn * double( c ) + deltaRow * double( r );
                los.addLOS( s, e );
                hat.addPoint( s );
            }
        }

        {
            std::cout << "Computing LineOfSight" << std::endl;

            osg::Timer_t startTick = osg::Timer::instance()->tick();

            los.computeIntersections( scene.get() );

            osg::Timer_t endTick = osg::Timer::instance()->tick();

            std::cout << "Completed in "
                      << osg::Timer::instance()->delta_s( startTick, endTick )
                      << std::endl;

            for( unsigned int i = 0; i < los.getNumLOS(); i++ )
            {
                const osgSim::LineOfSight::Intersections& intersections =
                    los.getIntersections( i );
                for( osgSim::LineOfSight::Intersections::const_iterator itr =
                         intersections.begin();
                     itr != intersections.end();
                     ++itr )
                {
                    std::cout << "  point " << *itr << std::endl;
                }
            }
        }

        {
            // now do a second traversal to test performance of cache.
            osg::Timer_t startTick = osg::Timer::instance()->tick();

            std::cout << "Computing HeightAboveTerrain" << std::endl;

            hat.computeIntersections( scene.get() );

            osg::Timer_t endTick = osg::Timer::instance()->tick();

            for( unsigned int i = 0; i < hat.getNumPoints(); i++ )
            {
                std::cout << "  point = " << hat.getPoint( i )
                          << " hat = " << hat.getHeightAboveTerrain( i ) << std::endl;
            }

            std::cout << "Completed in "
                      << osg::Timer::instance()->delta_s( startTick, endTick )
                      << std::endl;
        }
#endif

        {
            // now do a second traversal to test performance of cache.
            osg::Timer_t startTick = osg::Timer::instance()->tick();

            std::cout << "Computing ElevationSlice" << std::endl;
            osgSim::ElevationSlice es;
            es.setDatabaseCacheReadCallback( los.getDatabaseCacheReadCallback() );

            es.setStartPoint( bs.center + osg::dvec3( bs.radius, 0.0, 0.0 ) );
            es.setEndPoint( bs.center + osg::dvec3( 0.0, 0.0, bs.radius ) );

            es.computeIntersections( scene.get() );

            osg::Timer_t endTick = osg::Timer::instance()->tick();

            std::cout << "Completed in "
                      << osg::Timer::instance()->delta_s( startTick, endTick )
                      << std::endl;

            typedef osgSim::ElevationSlice::DistanceHeightList DistanceHeightList;
            const DistanceHeightList& dhl = es.getDistanceHeightIntersections();
            std::cout << "Number of intersections =" << dhl.size() << std::endl;
            for( DistanceHeightList::const_iterator dhitr = dhl.begin();
                 dhitr != dhl.end();
                 ++dhitr )
            {
                std::cout.precision( 10 );
                std::cout << "  " << dhitr->first << " " << dhitr->second << std::endl;
            }
        }
    }
    else if( useIntersectorGroup )
    {
        osg::Timer_t startTick = osg::Timer::instance()->tick();

        osg::dvec3   start     = bs.center + osg::dvec3( 0.0, bs.radius, 0.0 );
        osg::dvec3 end = osg::dvec3( bs.center );    // - osg::dvec3(0.0, bs.radius,0.0);
        osg::dvec3 deltaRow( 0.0, 0.0, bs.radius * 0.01 );
        osg::dvec3 deltaColumn( bs.radius * 0.01, 0.0, 0.0 );
        unsigned int                            numRows    = 20;
        unsigned int                            numColumns = 20;

        osg::ref_ptr<osgUtil::IntersectorGroup> intersectorGroup =
            new osgUtil::IntersectorGroup();

        for( unsigned int r = 0; r < numRows; ++r )
        {
            for( unsigned int c = 0; c < numColumns; ++c )
            {
                osg::dvec3 s =
                    start + deltaColumn * double( c ) + deltaRow * double( r );
                osg::dvec3 e = end + deltaColumn * double( c ) + deltaRow * double( r );
                osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
                    new osgUtil::LineSegmentIntersector( s, e );
                intersectorGroup->addIntersector( intersector.get() );
            }
        }

        osgUtil::IntersectionVisitor intersectVisitor( intersectorGroup.get(),
                                                       new MyReadCallback );
        scene->accept( intersectVisitor );

        osg::Timer_t endTick = osg::Timer::instance()->tick();

        std::cout << "Completed in "
                  << osg::Timer::instance()->delta_s( startTick, endTick ) << std::endl;

        if( intersectorGroup->containsIntersections() )
        {
            std::cout << "Found intersections " << std::endl;

            osgUtil::IntersectorGroup::Intersectors& intersectors =
                intersectorGroup->getIntersectors();
            for( osgUtil::IntersectorGroup::Intersectors::iterator intersector_itr =
                     intersectors.begin();
                 intersector_itr != intersectors.end();
                 ++intersector_itr )
            {
                osgUtil::LineSegmentIntersector* lsi =
                    dynamic_cast<osgUtil::LineSegmentIntersector*>(
                        intersector_itr->get()
                    );
                if( lsi )
                {
                    osgUtil::LineSegmentIntersector::Intersections& intersections =
                        lsi->getIntersections();
                    for( osgUtil::LineSegmentIntersector::Intersections::iterator itr =
                             intersections.begin();
                         itr != intersections.end();
                         ++itr )
                    {
                        const osgUtil::LineSegmentIntersector::Intersection&
                            intersection = *itr;
                        std::cout << "  ratio " << intersection.ratio << std::endl;
                        std::cout << "  point " << intersection.localIntersectionPoint
                                  << std::endl;
                        std::cout << "  normal " << intersection.localIntersectionNormal
                                  << std::endl;
                        std::cout << "  indices " << intersection.indexList.size()
                                  << std::endl;
                        std::cout << "  primitiveIndex " << intersection.primitiveIndex
                                  << std::endl;
                        std::cout << std::endl;
                    }
                }
            }
        }
    }
    else
    {
        osg::Timer_t startTick = osg::Timer::instance()->tick();

#if 1
        osg::dvec3 start = bs.center + osg::dvec3( 0.0, bs.radius, 0.0 );
        osg::dvec3 end   = bs.center - osg::dvec3( 0.0, bs.radius, 0.0 );
#else
        osg::dvec3 start = bs.center + osg::dvec3( 0.0, 0.0, bs.radius );
        osg::dvec3 end   = bs.center - osg::dvec3( 0.0, 0.0, bs.radius );
#endif

        osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
            new osgUtil::LineSegmentIntersector( start, end );

        osgUtil::IntersectionVisitor intersectVisitor( intersector.get(),
                                                       new MyReadCallback );

        scene->accept( intersectVisitor );

        osg::Timer_t endTick = osg::Timer::instance()->tick();

        std::cout << "Completed in "
                  << osg::Timer::instance()->delta_s( startTick, endTick ) << std::endl;

        if( intersector->containsIntersections() )
        {
            osgUtil::LineSegmentIntersector::Intersections& intersections =
                intersector->getIntersections();
            for( osgUtil::LineSegmentIntersector::Intersections::iterator itr =
                     intersections.begin();
                 itr != intersections.end();
                 ++itr )
            {
                const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
                std::cout << "  ratio " << intersection.ratio << std::endl;
                std::cout << "  point " << intersection.localIntersectionPoint
                          << std::endl;
                std::cout << "  normal " << intersection.localIntersectionNormal
                          << std::endl;
                std::cout << "  indices " << intersection.indexList.size() << std::endl;
                std::cout << "  primitiveIndex " << intersection.primitiveIndex
                          << std::endl;
                std::cout << std::endl;
            }
        }
    }

    return 0;
}
