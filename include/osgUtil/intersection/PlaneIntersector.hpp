/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Plane intersection tester. Finds polylines where geometry
 * crosses a plane for cross-section visualization.
 */
#pragma once

#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osgUtil/intersection/IntersectionVisitor.hpp>

namespace osgUtil
{

    /** Concrete class for implementing polytope intersections with the scene graph.
     * To be used in conjunction with IntersectionVisitor. */
    class OSGUTIL_EXPORT PlaneIntersector : public Intersector
    {
        public:

            /** Construct a PolytopeIntersector using speified polytope in MODEL
             * coordinates.*/
            PlaneIntersector( const osg::Plane&    plane,
                              const osg::Polytope& boundingPolytope = osg::Polytope() );

            /** Construct a PolytopeIntersector using speified polytope in specified
             * coordinate frame.*/
            PlaneIntersector( CoordinateFrame      cf,
                              const osg::Plane&    plane,
                              const osg::Polytope& boundingPolytope = osg::Polytope() );

            struct Intersection
            {
                    Intersection()
                    {
                    }

                    bool
                    operator<( const Intersection& rhs ) const
                    {
                        if( polyline < rhs.polyline )
                        {
                            return true;
                        }
                        if( rhs.polyline < polyline )
                        {
                            return false;
                        }

                        if( nodePath < rhs.nodePath )
                        {
                            return true;
                        }
                        if( rhs.nodePath < nodePath )
                        {
                            return false;
                        }

                        return ( drawable < rhs.drawable );
                    }

                    typedef std::vector<osg::dvec3> Polyline;
                    typedef std::vector<double>     Attributes;

                    osg::NodePath                   nodePath;
                    osg::ref_ptr<osg::RefMatrix>    matrix;
                    osg::ref_ptr<osg::Drawable>     drawable;
                    Polyline                        polyline;
                    Attributes                      attributes;
            };

            typedef std::vector<Intersection> Intersections;

            inline void
            insertIntersection( const Intersection& intersection )
            {
                getIntersections().push_back( intersection );
            }

            inline Intersections&
            getIntersections()
            {
                return _parent ? _parent->_intersections : _intersections;
            }

            void
            setRecordHeightsAsAttributes( bool flag )
            {
                _recordHeightsAsAttributes = flag;
            }

            bool
            getRecordHeightsAsAttributes() const
            {
                return _recordHeightsAsAttributes;
            }

            void
            setEllipsoidModel( osg::EllipsoidModel* em )
            {
                _em = em;
            }

            const osg::EllipsoidModel*
            getEllipsoidModel() const
            {
                return _em.get();
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

            PlaneIntersector*                 _parent;

            bool                              _recordHeightsAsAttributes;
            osg::ref_ptr<osg::EllipsoidModel> _em;

            osg::Plane                        _plane;
            osg::Polytope                     _polytope;

            Intersections                     _intersections;
    };

}
