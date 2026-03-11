/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Ray intersection tester extending LineSegmentIntersector.
 * Provides infinite-ray semantics for shadow and LOS queries.
 */
#pragma once

#include <osg/maths/compat.hpp>
#include <osgUtil/intersection/IntersectionVisitor.hpp>

namespace osgUtil
{

    /** RayIntersector implements possibly-infinite line intersections with the scene
     * graph.
     *
     * Compared with LineSegmentIntersector, RayIntersector supports infinite
     * intersection lines, start and end point can be given in homogeneous coordinates
     * and projection matrix is allowed to have z-far plane at infinity (often used in
     * shadow volume technique).
     *
     * Currently, picking of objects at infinity is not supported. Please, contribute.
     *
     * The class is be used in conjunction with IntersectionVisitor. */
    class OSGUTIL_EXPORT RayIntersector : public Intersector
    {
        public:

            /** Construct a RayIntersector. You will need to provide start and end point,
             *  or start point and direction. See setStart() and setDirecton(). */
            RayIntersector( CoordinateFrame                         cf     = MODEL,
                            RayIntersector*                         parent = NULL,
                            osgUtil::Intersector::IntersectionLimit intersectionLimit =
                                osgUtil::Intersector::NO_LIMIT );

            /** Construct a RayIntersector that runs from start point in specified
             * direction to the infinity. Start and direction are provided in MODEL
             * coordinates. */
            RayIntersector( const osg::dvec3& start,
                            const osg::dvec3& direction );

            /** Construct a RayIntersector the runs from start point in specified
             * direction to the infinity in the specified coordinate frame. */
            RayIntersector( CoordinateFrame                         cf,
                            const osg::dvec3&                       start,
                            const osg::dvec3&                       direction,
                            RayIntersector*                         parent = NULL,
                            osgUtil::Intersector::IntersectionLimit intersectionLimit =
                                osgUtil::Intersector::NO_LIMIT );

            /** Convenience constructor for supporting picking in WINDOW and PROJECTION
             * coordinates. In WINDOW coordinates, it creates a start value of (x,y,0)
             * and end value of (x,y,1). In PROJECTION coordinates (clip space cube), it
             * creates a start value of (x,y,-1) and end value of (x,y,1). In VIEW and
             * MODEL coordinates, it creates a start value of (x,y,0) and end value of
             * (x,y,1).*/
            RayIntersector( CoordinateFrame cf,
                            double          x,
                            double          y );

            struct OSGUTIL_EXPORT Intersection
            {
                    Intersection() :
                        distance( -1.0 ),
                        primitiveIndex( 0 )
                    {
                    }

                    bool
                    operator<( const Intersection& rhs ) const
                    {
                        return distance < rhs.distance;
                    }

                    typedef std::vector<unsigned int> IndexList;
                    typedef std::vector<double>       RatioList;

                    double                            distance;
                    osg::NodePath                     nodePath;
                    osg::ref_ptr<osg::Drawable>       drawable;
                    osg::ref_ptr<osg::RefMatrix>      matrix;
                    osg::dvec3                        localIntersectionPoint;
                    osg::vec3                         localIntersectionNormal;
                    IndexList                         indexList;
                    RatioList                         ratioList;
                    unsigned int                      primitiveIndex;

                    const osg::dvec3&
                    getLocalIntersectPoint() const
                    {
                        return localIntersectionPoint;
                    }

                    osg::dvec3
                    getWorldIntersectPoint() const
                    {
                        return matrix.valid() ? localIntersectionPoint * ( *matrix )
                                              : localIntersectionPoint;
                    }

                    const osg::vec3&
                    getLocalIntersectNormal() const
                    {
                        return localIntersectionNormal;
                    }

                    osg::vec3
                    getWorldIntersectNormal() const
                    {
                        return matrix.valid()
                                 ? osg::transform3x3( osg::inverse( *matrix ),
                                                      localIntersectionNormal )
                                 : localIntersectionNormal;
                    }

                    /** Convenience function for mapping the intersection point to any
                     * textures assigned to the objects intersected. Returns the Texture
                     * pointer and texture coords of object hit when a texture is
                     * available on the object, returns NULL otherwise.*/
                    osg::Texture*
                    getTextureLookUp( osg::vec3& tc ) const;
            };

            typedef std::multiset<Intersection> Intersections;

            inline void
            insertIntersection( const Intersection& intersection )
            {
                getIntersections().insert( intersection );
            }

            inline Intersections&
            getIntersections()
            {
                return _parent ? _parent->_intersections : _intersections;
            }

            inline Intersection
            getFirstIntersection()
            {
                Intersections& intersections = getIntersections();
                return intersections.empty() ? Intersection()
                                             : *( intersections.begin() );
            }

            virtual void
            setStart( const osg::dvec3& start )
            {
                _start = start;
            }

            inline const osg::dvec3&
            getStart() const
            {
                return _start;
            }

            virtual void
            setDirection( const osg::dvec3& dir )
            {
                _direction = dir;
            }

            inline const osg::dvec3&
            getDirection() const
            {
                return _direction;
            }

        public:

            virtual Intersector*
            clone( osgUtil::IntersectionVisitor& iv );

            virtual bool
            enter( const osg::Node& node );

            virtual void
            leave();

            virtual void
            intersect( osgUtil::IntersectionVisitor& iv,
                       osg::Drawable*                drawable );

            virtual void
            reset();

            virtual bool
            containsIntersections()
            {
                return !getIntersections().empty();
            }

        protected:

            virtual bool
            intersects( const osg::sphere& bs );
            bool
                            intersectAndClip( osg::dvec3&       s,
                                              const osg::dvec3& d,
                                              osg::dvec3&       e,
                                              const osg::box&   bb );

            RayIntersector* _parent;

            osg::dvec3      _start;
            osg::dvec3      _direction;

            Intersections   _intersections;
    };

}
