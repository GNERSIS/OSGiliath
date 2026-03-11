/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Shadow volume occluder for software occlusion culling.
 * Projects an occluder polyhedron to test scene node visibility.
 */
#include <osg/traversal/ShadowVolumeOccluder.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/traversal/CullStack.hpp>

using namespace osg;

typedef std::pair<unsigned int, vec3> Point;    // bool=true signifies a newly created
                                                // point, false indicates original point.
typedef std::vector<Point>            PointList;
typedef std::vector<vec3>             VertexList;

// copyVertexListToPointList a vector for vec3 into a vector of Point's.
void
copyVertexListToPointList( const VertexList& in,
                           PointList&        out )
{
    out.reserve( in.size() );
    for( VertexList::const_iterator itr = in.begin(); itr != in.end(); ++itr )
    {
        out.push_back( Point( 0, *itr ) );
    }
}

void
copyPointListToVertexList( const PointList& in,
                           VertexList&      out )
{
    out.reserve( in.size() );
    for( PointList::const_iterator itr = in.begin(); itr != in.end(); ++itr )
    {
        out.push_back( itr->second );
    }
}

// clip the convex hull 'in' to plane to generate a clipped convex hull 'out'
// return true if points remain after clipping.
unsigned int
clip( const Plane&     plane,
      const PointList& in,
      PointList&       out,
      unsigned int     planeMask )
{
    std::vector<float> distance;
    distance.reserve( in.size() );
    for( PointList::const_iterator itr = in.begin(); itr != in.end(); ++itr )
    {
        distance.push_back( plane.distance( itr->second ) );
    }

    out.clear();

    for( unsigned int i = 0; i < in.size(); ++i )
    {
        unsigned int i_1 =
            ( i + 1 ) %
            in.size();    // do the mod to wrap the index round back to the start.

        if( distance[i] >= 0.0F )
        {
            out.push_back( in[i] );

            if( distance[i_1] < 0.0F )
            {
                unsigned int mask = ( in[i].first & in[i_1].first ) | planeMask;
                float        r    = distance[i_1] / ( distance[i_1] - distance[i] );
                out.push_back(
                    Point( mask, in[i].second * r + in[i_1].second * ( 1.0F - r ) )
                );
            }
        }
        else if( distance[i_1] > 0.0F )
        {
            unsigned int mask = ( in[i].first & in[i_1].first ) | planeMask;
            float        r    = distance[i_1] / ( distance[i_1] - distance[i] );
            out.push_back( Point( mask,
                                  in[i].second * r + in[i_1].second * ( 1.0F - r ) ) );
        }
    }

    return static_cast<unsigned int>( out.size() );
}

// clip the convex hull 'in' to planeList to generate a clipped convex hull 'out'
// return true if points remain after clipping.
unsigned int
clip( const Polytope::PlaneList& planeList,
      const VertexList&          vin,
      PointList&                 out )
{
    PointList in;
    copyVertexListToPointList( vin, in );

    unsigned int planeMask = 0X1;
    for( Polytope::PlaneList::const_iterator itr = planeList.begin();
         itr != planeList.end();
         ++itr )
    {
        if( !clip( *itr, in, out, planeMask ) )
        {
            return false;
        }
        in.swap( out );
        planeMask <<= 1;
    }

    in.swap( out );

    return static_cast<unsigned int>( out.size() );
}

void
transformPoints( PointList&        points,
                 const osg::dmat4& matrix )
{
    for( PointList::iterator itr = points.begin(); itr != points.end(); ++itr )
    {
        itr->second = vec3( matrix * dvec3( itr->second ) );
    }
}

void
transformPoints( const PointList&  in,
                 PointList&        out,
                 const osg::dmat4& matrix )
{
    for( PointList::const_iterator itr = in.begin(); itr != in.end(); ++itr )
    {
        out.push_back( Point( itr->first, vec3( matrix * dvec3( itr->second ) ) ) );
    }
}

void
pushToFarPlane( PointList& points )
{
    for( PointList::iterator itr = points.begin(); itr != points.end(); ++itr )
    {
        itr->second.z = 1.0F;
    }
}

void
computePlanes( const PointList&     front,
               const PointList&     back,
               Polytope::PlaneList& planeList )
{
    for( unsigned int i = 0; i < front.size(); ++i )
    {
        unsigned int i_1 =
            ( i + 1 ) %
            front.size();    // do the mod to wrap the index round back to the start.
        if( !( front[i].first & front[i_1].first ) )
        {
            planeList.push_back( Plane( dvec3( front[i].second ),
                                        dvec3( front[i_1].second ),
                                        dvec3( back[i].second ) ) );
        }
    }
}

Plane
computeFrontPlane( const PointList& front )
{
    return Plane( dvec3( front[2].second ),
                  dvec3( front[1].second ),
                  dvec3( front[0].second ) );
}

// compute the volume between the front and back polygons of the occluder/hole.
float
computePolytopeVolume( const PointList& front,
                       const PointList& back )
{
    float volume     = 0.0F;
    vec3  frontStart = front[0].second;
    vec3  backStart  = back[0].second;
    for( unsigned int i = 1; i < front.size() - 1; ++i )
    {
        volume += computeVolume( frontStart,
                                 front[i].second,
                                 front[i + 1].second,
                                 backStart,
                                 back[i].second,
                                 back[i + 1].second );
    }
    return volume;
}

bool
ShadowVolumeOccluder::computeOccluder( const NodePath&             nodePath,
                                       const ConvexPlanarOccluder& occluder,
                                       CullStack&                  cullStack,
                                       bool /*createDrawables*/ )
{

    // std::cout<<"    Computing Occluder"<<std::endl;

    CullingSet&      cullingset = cullStack.getCurrentCullingSet();

    const RefMatrix& MV         = *cullStack.getModelViewMatrix();
    const RefMatrix& P          = *cullStack.getProjectionMatrix();

    // take a reference to the NodePath to this occluder.
    _nodePath = nodePath;

    // take a reference to the projection matrix.
    _projectionMatrix = &P;

    // initialize the volume
    _volume = 0.0F;

    // compute the inverse of the projection matrix.
    dmat4             invP       = osg::inverse( dmat4( P ) );

    float             volumeview = cullStack.getFrustumVolume();

    // compute the transformation matrix which takes form local coords into clip space.
    dmat4             MVP( P * MV );

    // for the occluder polygon and each of the holes do
    //     first transform occluder polygon into clipspace by multiple it by c[i] =
    //     v[i]*(MV*P) then push to coords to far plane by setting its coord to c[i].z =
    //     -1. then transform far plane polygon back into projection space, by
    //     p[i]*inv(P) compute orientation of front plane, if normal.z<0 then facing away
    //     from eye point, so reverse the polygons, or simply invert planes. compute
    //     volume (quality) between front polygon in projection space and back polygon in
    //     projection space.

    const VertexList& vertices_in = occluder.getOccluder().getVertexList();

    PointList         clipped_points;

    if( clip( cullingset.getFrustum().getPlaneList(), vertices_in, clipped_points ) >=
        3 )
    {
        // compute the points on the far plane.
        PointList clipped_farPoints;
        clipped_farPoints.reserve( clipped_points.size() );
        transformPoints( clipped_points, clipped_farPoints, MVP );
        pushToFarPlane( clipped_farPoints );
        transformPoints( clipped_farPoints, invP );

        // move the occlude points into projection space.
        transformPoints( clipped_points, dmat4( MV ) );

        // use the points on the front plane as reference vertices on the _occluderVolume
        // so that the vertices can later by used to test for occlusion of the occluder
        // itself.
        copyPointListToVertexList( clipped_points,
                                   _occluderVolume.getReferenceVertexList() );

        // create the front face of the occluder
        Plane clipped_occludePlane = computeFrontPlane( clipped_points );
        _occluderVolume.add( clipped_occludePlane );

        // create the sides of the occluder
        computePlanes( clipped_points,
                       clipped_farPoints,
                       _occluderVolume.getPlaneList() );

        _occluderVolume.setupMask();

        // if the front face is pointing away from the eye point flip the whole polytope.
        if( clipped_occludePlane[3] > 0.0F )
        {
            _occluderVolume.flip();
        }

        _volume =
            computePolytopeVolume( clipped_points, clipped_farPoints ) / volumeview;

        for( ConvexPlanarOccluder::HoleList::const_iterator hitr =
                 occluder.getHoleList().begin();
             hitr != occluder.getHoleList().end();
             ++hitr )
        {
            PointList points;
            if( clip( cullingset.getFrustum().getPlaneList(),
                      hitr->getVertexList(),
                      points ) >= 3 )
            {
                _holeList.push_back( Polytope() );
                Polytope& polytope = _holeList.back();

                // compute the points on the far plane.
                PointList farPoints;
                farPoints.reserve( points.size() );
                transformPoints( points, farPoints, MVP );
                pushToFarPlane( farPoints );
                transformPoints( farPoints, invP );

                // move the occlude points into projection space.
                transformPoints( points, dmat4( MV ) );

                // use the points on the front plane as reference vertices on the
                // _occluderVolume so that the vertices can later by used to test for
                // occlusion of the occluder itself.
                copyPointListToVertexList( points, polytope.getReferenceVertexList() );

                // create the front face of the occluder
                Plane occludePlane = computeFrontPlane( points );

                // create the sides of the occluder
                computePlanes( points, farPoints, polytope.getPlaneList() );

                polytope.setupMask();

                // if the front face is pointing away from the eye point flip the whole
                // polytope.
                if( occludePlane[3] > 0.0F )
                {
                    polytope.flip();
                }

                // remove the hole's volume from the occluder volume.
                _volume -= computePolytopeVolume( points, farPoints ) / volumeview;
            }
        }

        // std::cout << "final volume = "<<_volume<<std::endl;

        return true;
    }
    return false;
}

bool
ShadowVolumeOccluder::contains( const std::vector<vec3>& vertices )
{
    if( _occluderVolume.containsAllOf( vertices ) )
    {
        for( HoleList::iterator itr = _holeList.begin(); itr != _holeList.end(); ++itr )
        {
            PointList points;
            if( clip( itr->getPlaneList(), vertices, points ) >= 3 )
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool
ShadowVolumeOccluder::contains( const sphere& bound )
{
    // std::cout << "Sphere testing occluder "<<this<<"
    // mask="<<_occluderVolume.getCurrentMask();
    if( _occluderVolume.containsAllOf( bound ) )
    {
        for( HoleList::iterator itr = _holeList.begin(); itr != _holeList.end(); ++itr )
        {
            if( itr->contains( bound ) )
            {
                // std::cout << " - not in occluder"<<std::endl;
                return false;
            }
        }
        // std::cout << " - in occluder ******"<<std::endl;
        return true;
    }
    // std::cout << " - not in occluder"<<std::endl;
    return false;
}

bool
ShadowVolumeOccluder::contains( const box& bound )
{
    // std::cout << "Box testing occluder "<<this<<"
    // mask="<<_occluderVolume.getCurrentMask();
    if( _occluderVolume.containsAllOf( bound ) )
    {
        for( HoleList::iterator itr = _holeList.begin(); itr != _holeList.end(); ++itr )
        {
            if( itr->contains( bound ) )
            {
                // std::cout << " + not in occluder"<<std::endl;
                return false;
            }
        }
        // std::cout << "+ in occluder ********"<<std::endl;
        return true;
    }
    // std::cout << "+ not in occluder"<<std::endl;
    return false;
}
