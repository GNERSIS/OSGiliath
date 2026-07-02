/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Terrain elevation sampling along a line segment.
 * Returns height values for profile and cross-section analysis.
 */
#pragma once

#include <osgUtil/intersection/IntersectionVisitor.hpp>

// include so we can get access to the DatabaseCacheReadCallback
#include <osgSim/LineOfSight.hpp>

namespace osgSim
{

    /** Helper class for setting up and acquiring height above terrain intersections with
     * terrain. By default assigns a osgSim::DatabaseCacheReadCallback that enables
     * automatic loading of external PagedLOD tiles to ensure that the highest level of
     * detail is used in intersections. This automatic loading of tiles is done by the
     * intersection traversal that is done within the computeIntersections(..) method, so
     * can result in long intersection times when external tiles have to be loaded. The
     * external loading of tiles can be disabled by removing the read callback, this is
     * done by calling the setDatabaseCacheReadCallback(DatabaseCacheReadCallback*)
     * method with a value of 0.*/
    class OSGSIM_EXPORT ElevationSlice
    {
        public:

            ElevationSlice();

            /** Set the start point of the slice.*/
            void
            setStartPoint( const osg::dvec3& startPoint )
            {
                _startPoint = startPoint;
            }

            /** Get the start point of the slice.*/
            const osg::dvec3&
            getStartPoint() const
            {
                return _startPoint;
            }

            /** Set the end point of the slice.*/
            void
            setEndPoint( const osg::dvec3& endPoint )
            {
                _endPoint = endPoint;
            }

            /** Get the end point of the slice.*/
            const osg::dvec3&
            getEndPoint() const
            {
                return _endPoint;
            }

            typedef std::vector<osg::dvec3> Vec3dList;

            /** Get the intersections in the form of a vector of dvec3. */
            const Vec3dList&
            getIntersections() const
            {
                return _intersections;
            }

            typedef std::pair<double, double>   DistanceHeight;
            typedef std::vector<DistanceHeight> DistanceHeightList;

            /** Get the intersections in the form a vector of pair<double,double>
             * representing distance along the slice and height. */
            const DistanceHeightList&
            getDistanceHeightIntersections() const
            {
                return _distanceHeightIntersections;
            }

            /** Compute the intersections with the specified scene graph, the results are
             * stored in vectors of dvec3. Note, if the topmost node is a
             * CoordinateSystemNode then the input points are assumed to be geocentric,
             * with the up vector defined by the EllipsoidModel attached to the
             * CoordinateSystemNode. If the topmost node is not a CoordinateSystemNode
             * then a local coordinates frame is assumed, with a local up vector. */
            void
            computeIntersections( osg::Node*          scene,
                                  osg::Node::NodeMask traversalMask = 0XFF'FF'FF'FF );

            /** Compute the vertical distance between the specified scene graph and a
             * single HAT point.*/
            static Vec3dList
            computeElevationSlice( osg::Node*          scene,
                                   const osg::dvec3&   startPoint,
                                   const osg::dvec3&   endPoint,
                                   osg::Node::NodeMask traversalMask = 0XFF'FF'FF'FF );

            /** Clear the database cache.*/
            void
            clearDatabaseCache()
            {
                if( _dcrc.valid() )
                {
                    _dcrc->clearDatabaseCache();
                }
            }

            /** Set the ReadCallback that does the reading of external PagedLOD models,
             * and caching of loaded subgraphs. Note, if you have multiple LineOfSight or
             * ElevationSlice objects in use at one time then you should share a single
             * DatabaseCacheReadCallback between all of them. */
            void
            setDatabaseCacheReadCallback( DatabaseCacheReadCallback* dcrc );

            /** Get the ReadCallback that does the reading of external PagedLOD models,
             * and caching of loaded subgraphs.*/
            DatabaseCacheReadCallback*
            getDatabaseCacheReadCallback()
            {
                return _dcrc.get();
            }

        protected:

            osg::dvec3                              _startPoint;
            osg::dvec3                              _endPoint;
            Vec3dList                               _intersections;
            DistanceHeightList                      _distanceHeightIntersections;

            osg::ref_ptr<DatabaseCacheReadCallback> _dcrc;
            osgUtil::IntersectionVisitor            _intersectionVisitor;
    };

}
