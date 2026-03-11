/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Ray/line-segment intersection tester. Finds geometry hits along
 * a ray for picking, collision, and line-of-sight queries.
 */
#pragma once

#include <osg/maths/compat.hpp>
#include <osgUtil/intersection/IntersectionVisitor.hpp>

namespace osgUtil
{

    /** Concrete class for implementing line intersections with the scene graph.
     * To be used in conjunction with IntersectionVisitor. */
    class OSGUTIL_EXPORT LineSegmentIntersector : public Intersector
    {
        public:

            /** Construct a LineSegmentIntersector that runs between the specified start
             * and end points in MODEL coordinates. */
            LineSegmentIntersector( const osg::dvec3& start,
                                    const osg::dvec3& end );

            /** Construct a LineSegmentIntersector that runs between the specified start
             * and end points in the specified coordinate frame. */
            LineSegmentIntersector(
                CoordinateFrame                         cf,
                const osg::dvec3&                       start,
                const osg::dvec3&                       end,
                LineSegmentIntersector*                 parent = NULL,
                osgUtil::Intersector::IntersectionLimit intersectionLimit =
                    osgUtil::Intersector::NO_LIMIT
            );

            /** Convenience constructor for supporting picking in WINDOW, or PROJECTION
             * coordinates In WINDOW coordinates creates a start value of (x,y,0) and end
             * value of (x,y,1). In PROJECTION coordinates (clip space cube) creates a
             * start value of (x,y,-1) and end value of (x,y,1). In VIEW and MODEL
             * coordinates creates a start value of (x,y,0) and end value of (x,y,1).*/
            LineSegmentIntersector( CoordinateFrame cf,
                                    double          x,
                                    double          y );

            struct OSGUTIL_EXPORT Intersection
            {
                    Intersection() :
                        ratio( -1.0 ),
                        primitiveIndex( 0 )
                    {
                    }

                    bool
                    operator<( const Intersection& rhs ) const
                    {
                        return ratio < rhs.ratio;
                    }

                    typedef std::vector<unsigned int> IndexList;
                    typedef std::vector<double>       RatioList;

                    double                            ratio;
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

            inline void
            setStart( const osg::dvec3& start )
            {
                _start = start;
            }

            inline const osg::dvec3&
            getStart() const
            {
                return _start;
            }

            inline void
            setEnd( const osg::dvec3& end )
            {
                _end = end;
            }

            inline const osg::dvec3&
            getEnd() const
            {
                return _end;
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
            intersect( osgUtil::IntersectionVisitor& iv,
                       osg::Drawable*                drawable,
                       const osg::dvec3&             s,
                       const osg::dvec3&             e );

            virtual void
            reset();

            virtual bool
            containsIntersections()
            {
                return !getIntersections().empty();
            }

            /** Compute the matrix that transforms the local coordinate system of parent
               Intersector (usually the current intersector) into the child coordinate
               system of the child Intersector. cf parameter indicates the coordinate
               frame of parent Intersector. */
            static osg::dmat4
            getTransformation( osgUtil::IntersectionVisitor& iv,
                               CoordinateFrame               cf );

        protected:

            bool
            intersects( const osg::sphere& bs );
            bool
                                    intersectAndClip( osg::dvec3&     s,
                                                      osg::dvec3&     e,
                                                      const osg::box& bb );

            LineSegmentIntersector* _parent;

            osg::dvec3              _start;
            osg::dvec3              _end;

            Intersections           _intersections;
    };

}
