/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convex polyhedron utility for shadow volume computation.
 * Clips and intersects convex volumes for shadow analysis.
 */
#pragma once

#include <osg/geometry/Geometry.hpp>
#include <osg/traversal/Polytope.hpp>
#include <osgShadow/Export.hpp>

////////////////////////////////////////////////////////////////////////////////
// Class based on CustomPolytope defined and used in osgSim::OverlayNode.cpp.
// Honors should go to Robert Osfield for writing such useful piece of code.
// First incarnations of my ConvexPolyhedron were derived from CustomPolytope.
// Later I made a number of modifications aimed at improving convex hull
// precision of intersection & extrusion operations and ended up with code
// so mixed that I decided to rewrite it as separate class.
////////////////////////////////////////////////////////////////////////////////

namespace osgShadow
{

    class OSGSHADOW_EXPORT ConvexPolyhedron
    {
        public:

            typedef std::vector<osg::dvec3> Vertices;

            static const osg::dmat4&        defaultMatrix;

            struct Face
            {
                    std::string name;
                    osg::Plane  plane;
                    Vertices    vertices;
            };

            typedef std::list<Face> Faces;
            Faces                   _faces;

            ConvexPolyhedron( void )
            {
            }

            ConvexPolyhedron( const osg::dmat4& matrix,
                              const osg::dmat4& inverse,
                              const osg::box&   bb = osg::box( -1,
                                                               -1,
                                                               -1,
                                                               1,
                                                               1,
                                                               1 ) );

            Face&
            createFace()
            {
                _faces.push_back( Face() );
                return _faces.back();
            }

            void
            clear()
            {
                _faces.clear();
            }

            void
            setToUnitFrustum( bool withNear = true,
                              bool withFar  = true );
            void
            setToBoundingBox( const osg::box& bb );
            void
            transform( const osg::dmat4& matrix,
                       const osg::dmat4& inverse );
            void
            transformClip( const osg::dmat4& matrix,
                           const osg::dmat4& inverse );

            bool
            mergeFaces( const Face& face0,
                        const Face& face1,
                        Face&       face );

            void
            mergeCoplanarFaces( const double& plane_normal_dot_tolerance = 0.0,
                                const double& plane_distance_tolerance   = 0.0 );

            void
            removeDuplicateVertices( void );

            static int
            pointsColinear( const osg::dvec3& va,
                            const osg::dvec3& vb,
                            const osg::dvec3& vc,
                            const double&     edge_normal_dot_tolerance  = 0.0,
                            const double&     null_edge_length_tolerance = 0.0 );

            static int
            isFacePolygonConvex( Face& face,
                                 bool  ignoreCollinearVertices = true );

            bool
            checkCoherency( bool        checkForNonConvexPolys = false,
                            const char* errorPrefix            = NULL );

            void
            cut( const osg::Polytope& polytope );

            void
            cut( const ConvexPolyhedron& polytope );

            void
            cut( const osg::Plane&  plane,
                 const std::string& name = std::string() );

            void
            extrude( const osg::dvec3& offset );

            void
            translate( const osg::dvec3& offset );

            void
            getPolytope( osg::Polytope& polytope ) const;
            void
            getPoints( Vertices& vertices ) const;
            osg::box
            computeBoundingBox( const osg::dmat4& m =
                                    osgShadow::ConvexPolyhedron::defaultMatrix ) const;

            osg::Geometry*
            buildGeometry( const osg::dvec4& colorOutline,
                           const osg::dvec4& colorInside,
                           osg::Geometry*    useGeometry = NULL ) const;

            bool
            dumpGeometry( const Face*       face              = NULL,
                          const osg::Plane* plane             = NULL,
                          ConvexPolyhedron* basehull          = NULL,
                          const char*       filename          = "convexpolyhedron.osg",
                          const osg::dvec4& colorOutline      = osg::dvec4( 0,
                                                                            1,
                                                                            0,
                                                                            0.5 ),
                          const osg::dvec4& colorInside       = osg::dvec4( 0,
                                                                            1,
                                                                            0,
                                                                            0.25 ),
                          const osg::dvec4& faceColorOutline  = osg::dvec4( 0,
                                                                            0,
                                                                            1,
                                                                            0.5 ),
                          const osg::dvec4& faceColorInside   = osg::dvec4( 0,
                                                                            0,
                                                                            1,
                                                                            0.25 ),
                          const osg::dvec4& planeColorOutline = osg::dvec4( 1,
                                                                            0,
                                                                            0,
                                                                            0.5 ),
                          const osg::dvec4& planeColorInside  = osg::dvec4( 1,
                                                                            0,
                                                                            0,
                                                                            0.25 ),
                          const osg::dvec4& baseColorOutline  = osg::dvec4( 0,
                                                                            0,
                                                                            0,
                                                                            0.5 ),
                          const osg::dvec4& baseColorInside = osg::dvec4( 0,
                                                                          0,
                                                                          0,
                                                                          0.25 ) ) const;
    };

}    // namespace osgShadow
