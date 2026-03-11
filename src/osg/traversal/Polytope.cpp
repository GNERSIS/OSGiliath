/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convex polytope defined by a set of clipping planes.
 * Used for frustum culling, polytope intersection, and view volumes.
 */
#include <osg/traversal/Polytope.hpp>

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>

using namespace osg;

bool
Polytope::contains( const osg::vec3& v0,
                    const osg::vec3& v1,
                    const osg::vec3& v2 ) const
{
    if( !_maskStack.back() )
    {
        return true;
    }

    // initialize the set of vertices to test.
    typedef std::vector<osg::dvec3> Vertices;

    Vertices                        src, dest;
    src.reserve( 4 + _planeList.size() );
    dest.reserve( 4 + _planeList.size() );

    src.push_back( dvec3( v0 ) );
    src.push_back( dvec3( v1 ) );
    src.push_back( dvec3( v2 ) );
    src.push_back( dvec3( v0 ) );

    ClippingMask resultMask    = _maskStack.back();
    ClippingMask selector_mask = 0X1;

    for( PlaneList::const_iterator pitr = _planeList.begin(); pitr != _planeList.end();
         ++pitr )
    {
        if( resultMask & selector_mask )
        {
            // OSG_NOTICE<<"Polytope::contains() Plane testing"<<std::endl;

            dest.clear();

            const osg::Plane&  plane      = *pitr;
            Vertices::iterator vitr       = src.begin();

            osg::dvec3*        v_previous = &( *( vitr++ ) );
            double             d_previous = plane.distance( *v_previous );

            for( ; vitr != src.end(); ++vitr )
            {
                osg::dvec3* v_current = &( *vitr );
                double      d_current = plane.distance( *v_current );

                if( d_previous >= 0.0 )
                {
                    dest.push_back( *v_previous );
                }

                if( d_previous * d_current < 0.0 )
                {
                    // edge crosses plane so insert the vertex between them.
                    double     distance  = d_previous - d_current;
                    double     r_current = d_previous / distance;
                    osg::dvec3 v_new     = ( *v_previous ) *
                                           ( 1.0 - r_current ) +
                                           ( *v_current ) *
                                           r_current;
                    dest.push_back( v_new );
                }

                d_previous = d_current;
                v_previous = v_current;
            }

            if( d_previous >= 0.0 )
            {
                dest.push_back( *v_previous );
            }

            if( dest.size() <= 1 )
            {
                // OSG_NOTICE<<"Polytope::contains() All points on triangle culled,
                // dest.size()="<<dest.size()<<std::endl;
                return false;
            }

            dest.swap( src );
        }
        else
        {
            // OSG_NOTICE<<"Polytope::contains() Plane disabled"<<std::endl;
        }

        selector_mask <<= 1;
    }

    // OSG_NOTICE<<"Polytope::contains() triangle within Polytope,
    // src.size()="<<src.size()<<std::endl;
    return true;
}
