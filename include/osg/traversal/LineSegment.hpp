/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Line segment defined by start and end points. Used for ray-casting,
 * intersection testing, and line-of-sight queries.
 */
#pragma once

#include <osg/core/Export.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/sphere.hpp>

namespace osg
{

    /** LineSegment class for representing a line segment. */
    class OSG_EXPORT LineSegment : public Referenced
    {
        public:

            typedef dvec3                vec_type;
            typedef vec_type::value_type value_type;

            LineSegment() {};

            LineSegment( const LineSegment& seg ) :
                Referenced(),
                _s( seg._s ),
                _e( seg._e )
            {
            }

            LineSegment( const vec_type& s,
                         const vec_type& e ) :
                _s( s ),
                _e( e )
            {
            }

            LineSegment&
            operator=( const LineSegment& seg )
            {
                _s = seg._s;
                _e = seg._e;
                return *this;
            }

            inline void
            set( const vec_type& s,
                 const vec_type& e )
            {
                _s = s;
                _e = e;
            }

            inline vec_type&
            start()
            {
                return _s;
            }

            inline const vec_type&
            start() const
            {
                return _s;
            }

            inline vec_type&
            end()
            {
                return _e;
            }

            inline const vec_type&
            end() const
            {
                return _e;
            }

            inline bool
            valid() const
            {
                return _s != _e;
            }

            /** return true if segment intersects box. */
            bool
            intersect( const box& bb ) const;

            /** return true if segment intersects box and
             * set float ratios for the first and second intersections, where the ratio
             * is 0.0 at the segment start point, and 1.0 at the segment end point.
             */
            bool
            intersectAndComputeRatios( const box& bb,
                                       float&     ratioFromStartToEnd1,
                                       float&     ratioFromStartToEnd2 ) const;

            /** return true if segment intersects box and
             * set double ratios for the first and second intersections, where the ratio
             * is 0.0 at the segment start point, and 1.0 at the segment end point.
             */
            bool
            intersectAndComputeRatios( const box& bb,
                                       double&    ratioFromStartToEnd1,
                                       double&    ratioFromStartToEnd2 ) const;

            /** return true if segment intersects sphere. */
            bool
            intersect( const sphere& bs ) const;

            /** return true if segment intersects sphere and
             * set float ratios for the first and second intersections, where the ratio
             * is 0.0 at the segment start point, and 1.0 at the segment end point.
             */
            bool
            intersectAndComputeRatios( const sphere& bs,
                                       float&        ratioFromStartToEnd1,
                                       float&        ratioFromStartToEnd2 ) const;

            /** return true if segment intersects sphere and
             * set double ratios for the first and second intersections, where the ratio
             * is 0.0 at the segment start point, and 1.0 at the segment end point.
             */
            bool
            intersectAndComputeRatios( const sphere& bs,
                                       double&       ratioFromStartToEnd1,
                                       double&       ratioFromStartToEnd2 ) const;

            /** return true if segment intersects triangle and
             * set float ratios where the ratio is 0.0 at the segment start point,
             * and 1.0 at the segment end point.
             */
            bool
            intersect( const vec3& v1,
                       const vec3& v2,
                       const vec3& v3,
                       float&      ratioFromStartToEnd );

            /** return true if segment intersects triangle and
             * set double ratios where the ratio is 0.0 at the segment start point,
             * and 1.0 at the segment end point.
             */
            bool
            intersect( const dvec3& v1,
                       const dvec3& v2,
                       const dvec3& v3,
                       double&      ratioFromStartToEnd );

            /** post multiply a segment by matrix.*/
            inline void
            mult( const LineSegment& seg,
                  const dmat4&       m )
            {
                _s = seg._s * m;
                _e = seg._e * m;
            }

            /** pre multiply a segment by matrix.*/
            inline void
            mult( const dmat4&       m,
                  const LineSegment& seg )
            {
                _s = m * seg._s;
                _e = m * seg._e;
            }

        protected:

            virtual ~LineSegment();

            static bool
                     intersectAndClip( vec_type&  s,
                                       vec_type&  e,
                                       const box& bb );

            vec_type _s;
            vec_type _e;
    };

}
