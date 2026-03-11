/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Boundary, derived from Referenced.
 * Provides: Segment, first, Segment, first, second, thickness.
 */
#include "GlyphGeometry.hpp"

#include <limits.h>
#include <osg/core/io_utils.hpp>
#include <osg/geometry/TriangleIndexFunctor.hpp>
#include <osg/maths/Math.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/LineWidth.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgUtil/mesh/Tessellator.hpp>

#define REPORT_TIME 0

#if REPORT_TIME
    #include <osg/core/Timer.hpp>
    #include <osg/maths/compat.hpp>
    #include <osg/maths/Math.hpp>
#endif

namespace osgText
{

    /////////////////////////////////////////////////////////////////////////////////////////
    //
    // Boundary
    //
    class Boundary : public osg::Referenced
    {
        public:

            struct Segment
            {
                    Segment( unsigned int f,
                             unsigned int s,
                             float        t ) :
                        first( f ),
                        second( s ),
                        thickness( t ),
                        suggestedThickness( t )
                    {
                    }

                    Segment( const Segment& seg ) :
                        first( seg.first ),
                        second( seg.second ),
                        thickness( seg.thickness ),
                        suggestedThickness( seg.suggestedThickness )
                    {
                    }

                    Segment&
                    operator=( const Segment& seg )
                    {
                        first              = seg.first;
                        second             = seg.second;
                        thickness          = seg.thickness;
                        suggestedThickness = seg.suggestedThickness;
                        return *this;
                    }

                    unsigned int first;
                    unsigned int second;
                    float        thickness;
                    float        suggestedThickness;
            };

            // typedef std::pair<unsigned int, unsigned int> Segment;
            typedef std::vector<Segment>                Segments;
            osg::ref_ptr<const osg::Vec3Array>          _vertices;
            osg::ref_ptr<const osg::DrawElementsUShort> _elements;
            Segments                                    _segments;
            bool                                        verbose;

            Boundary( const osg::Vec3Array*    vertices,
                      const osg::PrimitiveSet* primitiveSet,
                      float                    thickness ) :
                verbose( false )
            {
                const osg::DrawArrays* drawArrays =
                    dynamic_cast<const osg::DrawArrays*>( primitiveSet );
                if( drawArrays )
                {
                    set( vertices,
                         static_cast<unsigned int>( drawArrays->getFirst() ),
                         static_cast<unsigned int>( drawArrays->getCount() ),
                         thickness );
                }
                else
                {
                    const osg::DrawElementsUShort* elements =
                        dynamic_cast<const osg::DrawElementsUShort*>( primitiveSet );
                    if( elements )
                    {
                        set( vertices, elements, thickness );
                    }
                }
            }

            void
            set( const osg::Vec3Array* vertices,
                 unsigned int          start,
                 unsigned int          count,
                 float                 thickness )
            {
                osg::DrawElementsUShort* elements =
                    new osg::DrawElementsUShort( osg::PrimitiveSet::POLYGON );
                for( unsigned int i = start; i < start + count; ++i )
                {
                    elements->push_back( static_cast<unsigned short>( i ) );
                }

                set( vertices, elements, thickness );
            }

            void
            set( const osg::Vec3Array*          vertices,
                 const osg::DrawElementsUShort* elements,
                 float                          thickness )
            {
                _vertices = vertices;
                _elements = elements;

                _segments.clear();

                if( elements->empty() )
                {
                    return;
                }

                _segments.reserve( elements->size() - 1 );
                for( unsigned int i = 0; i < elements->size() - 1; ++i )
                {
                    _segments.push_back(
                        Segment( ( *elements )[i], ( *elements )[i + 1], thickness )
                    );
                }
            }

            bool
            shorter( float original_thickness,
                     float new_thickness ) const
            {
                return ( original_thickness < 0.0F )
                         ? ( new_thickness > original_thickness )
                         : ( new_thickness < original_thickness );
            }

            bool
            shorten( float& original_thickness,
                     float  new_thickness )
            {
                if( shorter( original_thickness, new_thickness ) )
                {
                    original_thickness = new_thickness;
                    return true;
                }
                else
                {
                    return false;
                }
            }

            bool
            shortenBisector( unsigned int i,
                             float        new_thickness )
            {
                bool r1 = shorten(
                    _segments[( i + _segments.size() - 1 ) % ( _segments.size() )]
                        .suggestedThickness,
                    new_thickness
                );
                bool r2 = shorten( _segments[i].suggestedThickness, new_thickness );
                return r1 || r2;
            }

            void
            applySuggestedThickness()
            {
                for( Segments::iterator itr = _segments.begin(); itr != _segments.end();
                     ++itr )
                {
                    ( *itr ).thickness = ( *itr ).suggestedThickness;
                }
            }

            void
            applyThickness( float thickness )
            {
                for( Segments::iterator itr = _segments.begin(); itr != _segments.end();
                     ++itr )
                {
                    ( *itr ).thickness          = thickness;
                    ( *itr ).suggestedThickness = thickness;
                }
            }

            void
            getSuggestedThicknessRange( float& smallest,
                                        float& largest )
            {
                for( Segments::iterator itr = _segments.begin(); itr != _segments.end();
                     ++itr )
                {
                    float t = ( *itr ).suggestedThickness;
                    if( t < smallest )
                    {
                        smallest = t;
                    }
                    if( t > largest )
                    {
                        largest = t;
                    }
                }

                if( largest < 0.0F )
                {
                    std::swap( smallest, largest );
                }
            }

            osg::vec3
            computeRayIntersectionPoint( const osg::vec3& a,
                                         const osg::vec3& an,
                                         const osg::vec3& c,
                                         const osg::vec3& cn )
            {
                float denominator = ( cn.x * an.y - cn.y * an.x );
                if( denominator == 0.0F )
                {
                    // OSG_NOTICE<<"computeRayIntersectionPoint()<<denominator==0.0"<<std::endl;
                    //  line segments must be parallel.
                    return ( a + c ) * 0.5F;
                }

                float t = ( ( a.x - c.x ) * an.y - ( a.y - c.y ) * an.x ) / denominator;
                return c + cn * t;
            }

            osg::vec3
            computeIntersectionPoint( const osg::vec3& a,
                                      const osg::vec3& b,
                                      const osg::vec3& c,
                                      const osg::vec3& d )
            {
                return computeRayIntersectionPoint( a, b - a, c, d - c );
            }

            osg::vec3
            computeBisectorNormal( const osg::vec3& a,
                                   const osg::vec3& b,
                                   const osg::vec3& c,
                                   const osg::vec3& d )
            {
                osg::vec2 ab( a.x - b.x, a.y - b.y );
                osg::vec2 dc( d.x - c.x, d.y - c.y );
                /*float length_ab =*/ab = osg::normalize( ab );
                /*float length_dc =*/dc = osg::normalize( dc );

                float e                 = dc.y - ab.y;
                float f                 = ab.x - dc.x;
                float denominator       = sqrtf( e * e + f * f );
                float nx                = e / denominator;
                float ny                = f / denominator;
                if( ( ab.x * ny - ab.y * nx ) > 0.0F )
                {
                    // OSG_NOTICE<<"   computeBisectorNormal(a=["<<a<<"], b=["<<b<<"],
                    // c=["<<c<<"], d=["<<d<<"]), nx="<<nx<<", ny="<<ny<<",
                    // denominator="<<denominator<<" no need to swap"<<std::endl;
                    return osg::vec3( nx, ny, 0.0F );
                }
                else
                {
                    OSG_INFO << "   computeBisectorNormal(a=[" << a << "], b=[" << b
                             << "], c=[" << c << "], d=[" << d << "]), nx=" << nx
                             << ", ny=" << ny << ", denominator=" << denominator
                             << " need to swap!!!" << std::endl;
                    return osg::vec3( -nx, -ny, 0.0F );
                }
            }

            float
            computeBisectorIntersectorThickness( const osg::vec3& a,
                                                 const osg::vec3& b,
                                                 const osg::vec3& c,
                                                 const osg::vec3& d,
                                                 const osg::vec3& e,
                                                 const osg::vec3& f )
            {
                osg::vec3 intersection_abcd = computeIntersectionPoint( a, b, c, d );
                osg::vec3 bisector_abcd     = computeBisectorNormal( a, b, c, d );
                osg::vec3 intersection_cdef = computeIntersectionPoint( c, d, e, f );
                osg::vec3 bisector_cdef     = computeBisectorNormal( c, d, e, f );
                if( bisector_abcd == bisector_cdef )
                {
                    // OSG_NOTICE<<"computeBisectorIntersector(["<<a<<"], ["<<b<<"],
                    // ["<<c<<"], ["<<d<<"], ["<<e<<"], ["<<f<<"[)"<<std::endl;
                    // OSG_NOTICE<<"   bisectors parallel, thickness =
                    // "<<FLT_MAX<<std::endl;
                    return FLT_MAX;
                }

                osg::vec3 bisector_intersection =
                    computeRayIntersectionPoint( intersection_abcd,
                                                 bisector_abcd,
                                                 intersection_cdef,
                                                 bisector_cdef );
                osg::vec3 normal( d.y - c.y, c.x - d.x, 0.0 );
                float     cd_length = osg::length( normal );
                normal              = osg::normalize( normal );
                if( cd_length == 0 )
                {
                    // OSG_NOTICE<<"computeBisectorIntersector(["<<a<<"], ["<<b<<"],
                    // ["<<c<<"], ["<<d<<"], ["<<e<<"], ["<<f<<"[)"<<std::endl;
                    // OSG_NOTICE<<"   segment length==0, thickness =
                    // "<<FLT_MAX<<std::endl;
                    return FLT_MAX;
                }

                float thickness = osg::dot( bisector_intersection - c, normal );
#if 0
        OSG_NOTICE<<"computeBisectorIntersector(["<<a<<"], ["<<b<<"], ["<<c<<"], ["<<d<<"], ["<<e<<"], ["<<f<<"[)"<<std::endl;
        OSG_NOTICE<<"   bisector_abcd = "<<bisector_abcd<<", bisector_cdef="<<bisector_cdef<<std::endl;
        OSG_NOTICE<<"   bisector_intersection = "<<bisector_intersection<<", thickness = "<<thickness<<std::endl;
#endif
                return thickness;
            }

            float
            computeThickness( unsigned int i )
            {
                Segment& seg_before =
                    _segments[( i + _segments.size() - 1 ) % _segments.size()];
                Segment& seg_target = _segments[( i ) % _segments.size()];
                Segment& seg_after  = _segments[( i + 1 ) % _segments.size()];
                return computeBisectorIntersectorThickness(
                    ( *_vertices )[seg_before.first],
                    ( *_vertices )[seg_before.second],
                    ( *_vertices )[seg_target.first],
                    ( *_vertices )[seg_target.second],
                    ( *_vertices )[seg_after.first],
                    ( *_vertices )[seg_after.second]
                );
            }

            bool
            findMinThickness( unsigned int& minThickness_i,
                              float&        minThickness )
            {
                minThickness_i = static_cast<unsigned int>( _segments.size() );
                for( unsigned int i = 0; i < _segments.size(); ++i )
                {
                    float thickness = computeThickness( i );
                    if( thickness > 0.0 && thickness < minThickness )
                    {
                        minThickness   = thickness;
                        minThickness_i = i;
                    }
                }

                return minThickness_i != static_cast<unsigned int>( _segments.size() );
            }

            void
            removeAllSegmentsBelowThickness( float targetThickness )
            {
                // OSG_NOTICE<<"removeAllSegmentsBelowThickness("<<targetThickness<<")"<<std::endl;
                for( ;; )
                {
                    unsigned int minThickness_i =
                        static_cast<unsigned int>( _segments.size() );
                    float minThickness = targetThickness;
                    if( !findMinThickness( minThickness_i, minThickness ) )
                    {
                        break;
                    }

                    // OSG_NOTICE<<"  removing segment _segments["<<minThickness_i<<"]
                    // ("<<_segments[minThickness_i].first<<",
                    // "<<_segments[minThickness_i].second<<" with
                    // thickness="<<minThickness<<" "<<std::endl;
                    _segments.erase( _segments.begin() + minThickness_i );
                }
            }

            bool
            findMaxThickness( unsigned int& maxThickness_i,
                              float&        maxThickness )
            {
                maxThickness_i = static_cast<unsigned int>( _segments.size() );
                for( unsigned int i = 0; i < _segments.size(); ++i )
                {
                    float thickness = computeThickness( i );
                    if( thickness < 0.0 && thickness > maxThickness )
                    {
                        maxThickness   = thickness;
                        maxThickness_i = i;
                    }
                }

                return maxThickness_i != static_cast<unsigned int>( _segments.size() );
            }

            void
            removeAllSegmentsAboveThickness( float targetThickness )
            {
                // OSG_NOTICE<<"removeAllSegmentsBelowThickness("<<targetThickness<<")"<<std::endl;
                for( ;; )
                {
                    unsigned int maxThickness_i =
                        static_cast<unsigned int>( _segments.size() );
                    float maxThickness = targetThickness;
                    if( !findMaxThickness( maxThickness_i, maxThickness ) )
                    {
                        break;
                    }

                    // OSG_NOTICE<<"  removing segment _segments["<<minThickness_i<<"]
                    // ("<<_segments[minThickness_i].first<<",
                    // "<<_segments[minThickness_i].second<<" with
                    // thickness="<<minThickness<<" "<<std::endl;
                    _segments.erase( _segments.begin() + maxThickness_i );
                }
            }

            float
            computeBisectorPoint( unsigned int i,
                                  float        targetThickness,
                                  osg::vec3&   va,
                                  osg::vec3&   vb )
            {
                Segment& seg_before =
                    _segments[( i + _segments.size() - 1 ) % _segments.size()];
                Segment&         seg_target = _segments[( i ) % _segments.size()];
                const osg::vec3& a          = ( *_vertices )[seg_before.first];
                const osg::vec3& b          = ( *_vertices )[seg_before.second];
                const osg::vec3& c          = ( *_vertices )[seg_target.first];
                const osg::vec3& d          = ( *_vertices )[seg_target.second];
                osg::vec3 intersection_abcd = computeIntersectionPoint( a, b, c, d );
                osg::vec3 bisector_abcd     = computeBisectorNormal( a, b, c, d );
                osg::vec3 ab_sidevector( b.y - a.y, a.x - b.x, 0.0 );
                ab_sidevector          = osg::normalize( ab_sidevector );
                float     scale_factor = 1.0F / osg::dot( bisector_abcd, ab_sidevector );
                float     new_thickness = scale_factor * targetThickness;
                osg::vec3 new_vertex = intersection_abcd + bisector_abcd * new_thickness;

                // OSG_NOTICE<<"computeBisectorPoint("<<i<<",
                // targetThickness="<<targetThickness<<", bisector_abcd =
                // "<<bisector_abcd<<", ab_sidevector="<<ab_sidevector<<", b-a="<<b-a<<",
                // scale_factor="<<scale_factor<<std::endl;

                va = intersection_abcd;
                vb = new_vertex;

                return new_thickness;
            }

            float
            computeBisectorPoint( unsigned int i,
                                  osg::vec3&   va,
                                  osg::vec3&   vb )
            {
                float tbefore =
                    _segments[( i + _segments.size() - 1 ) % _segments.size()].thickness;
                float tafter =
                    _segments[( i + _segments.size() ) % _segments.size()].thickness;
                float t = tafter < 0.0 ? std::max( tbefore, tafter )
                                       : std::min( tbefore, tafter );
                return computeBisectorPoint( i, t, va, vb );
            }

            float
            computeThicknessThatBisectorAndSegmentMeet( const osg::vec3& va,
                                                        const osg::vec3& vb,
                                                        unsigned int     bi,
                                                        float original_thickness )
            {
                osg::vec3 bisector  = ( vb - va );

                bisector           /= original_thickness;

                Segment& seg_opposite =
                    _segments[( bi + _segments.size() ) % _segments.size()];
                const osg::vec3& vc = ( *_vertices )[seg_opposite.first];
                const osg::vec3& vd = ( *_vertices )[seg_opposite.second];
                osg::vec3        cdn( vd.y - vc.y, vc.x - vd.x, 0.0 );

                float            cdn_len = osg::length( cdn );
                if( cdn_len == 0.0F )
                {
                    return false;
                }
                cdn         = osg::normalize( cdn );

                float denom = ( 1.0F - osg::dot( bisector, cdn ) );
                if( denom == 0.0F )
                {
                    return FLT_MAX;
                }

                float h = osg::dot( va - vc, cdn ) / denom;
                if( h < 0.0 )
                {
                    return FLT_MAX;
                }

                return h;
            }

            int
            clampSegmentToEdge( osg::vec3&       va,
                                osg::vec3&       vb,
                                const osg::vec3& vc,
                                const osg::vec3& vd )
            {
                osg::vec2 ncd( vc.y - vd.y, vd.x - vc.x );
                float     na = ( va.x - vc.x ) * ncd.x + ( va.y - vc.y ) * ncd.y;
                float     nb = ( vb.x - vc.x ) * ncd.x + ( vb.y - vc.y ) * ncd.y;

                if( na >= 0.0F )    // check if na is inside
                {
                    // check if wholly inside
                    if( nb >= 0.0F )
                    {
                        return 1;
                    }

                    // na is inside, nb outside, need to shift nb to (vc, vd) line.
                    float d = na - nb;

                    if( d == 0.0F )
                    {
                        return 1;
                    }

                    float r = na / d;
                    vb      = va + ( vb - va ) * r;
                }
                else    // na<0.0 and therefore outside
                {
                    // check if wholly outside
                    if( nb <= 0.0F )
                    {
                        return -1;
                    }

                    // na is outside, nb inside, need to shift na to (vc, vd) line.
                    float d = nb - na;
                    if( d == 0.0F )
                    {
                        return -1;
                    }

                    float r = -na / d;

                    va      = va + ( vb - va ) * r;
                }

                return 0;
            }

            bool
            doesSegmentIntersectQuad( osg::vec3        va,
                                      osg::vec3        vb,
                                      const osg::vec3& v1,
                                      const osg::vec3& v2,
                                      const osg::vec3& v3,
                                      const osg::vec3& v4 )
            {
                osg::vec2 ncd( v1.y - v2.y, v2.x - v1.x );

                // catch case of bisector boundary point being behind the segment of
                // interest
                float     na = ( va.x - v1.x ) * ncd.x + ( va.y - v1.y ) * ncd.y;
                if( na >= 0.0 )
                {
                    return false;
                }

                // catch case of bisector pointing away from the segment of interest
                float nb = ( vb.x - v1.x ) * ncd.x + ( vb.y - v1.y ) * ncd.y;
                if( na >= nb )
                {
                    return false;
                }

                osg::vec3 cp123            = osg::cross( v2 - v1, v3 - v2 );
                osg::vec3 cp234            = osg::cross( v3 - v2, v4 - v3 );
                float     dot_1234         = osg::dot( cp123, cp234 );
                bool      not_crossed_over = ( dot_1234 >= 0.0 );

                if( clampSegmentToEdge( va, vb, v1, v4 ) < 0 )
                {
                    return false;
                }
                if( clampSegmentToEdge( va, vb, v3, v2 ) < 0 )
                {
                    return false;
                }
                if( clampSegmentToEdge( va, vb, v2, v1 ) < 0 )
                {
                    return false;
                }
                if( not_crossed_over && clampSegmentToEdge( va, vb, v4, v3 ) < 0 )
                {
                    return false;
                }

                return true;
            }

            float
            checkBisectorAgainstBoundary( osg::vec3 va,
                                          osg::vec3 vb,
                                          float     original_thickness )
            {
                float     thickness = original_thickness;

                osg::vec3 before_outer, before_inner;
                osg::vec3 after_outer, after_inner;

                for( unsigned int i = 0; i < _segments.size(); ++i )
                {
                    Segment& seg_before =
                        _segments[( i + _segments.size() - 1 ) % _segments.size()];
                    Segment& seg_target = _segments[( i ) % _segments.size()];
                    Segment& seg_after  = _segments[( i + 1 ) % _segments.size()];

                    computeBisectorPoint( i, before_outer, before_inner );
                    computeBisectorPoint( i + 1, after_outer, after_inner );

                    if( doesSegmentIntersectQuad( va,
                                                  vb,
                                                  before_outer,
                                                  after_outer,
                                                  after_inner,
                                                  before_inner ) )
                    {
                        float new_thickness = computeThicknessThatBisectorAndSegmentMeet(
                            va,
                            vb,
                            i,
                            original_thickness
                        );

                        shorten( thickness, new_thickness );
                        shorten( seg_before.suggestedThickness, new_thickness );
                        shorten( seg_target.suggestedThickness, new_thickness );
                        shorten( seg_after.suggestedThickness, new_thickness );
                    }
                }
                return thickness;
            }

            void
            checkBoundaries( Boundary& boundary )
            {
                osg::vec3 va, vb;
                float     min_thickiness = FLT_MAX;
                for( unsigned int i = 0; i < boundary._segments.size(); ++i )
                {
                    boundary.computeBisectorPoint( i, va, vb );
                    float new_thickness =
                        checkBisectorAgainstBoundary( va,
                                                      vb,
                                                      boundary._segments[i].thickness );
                    if( boundary.shortenBisector( i, new_thickness ) )
                    {
                        shorten( min_thickiness, new_thickness );
                    }
                }
            }

            void
            addBoundaryToGeometry( osg::Geometry* geometry,
                                   float /*targetThickness*/,
                                   const std::string& faceName,
                                   const std::string& bevelName )
            {
                if( _segments.empty() )
                {
                    return;
                }

                unsigned int    start = ( *_elements )[0];
                unsigned int    count = static_cast<unsigned int>( _elements->size() );

                osg::Vec3Array* new_vertices =
                    dynamic_cast<osg::Vec3Array*>( geometry->getVertexArray() );
                if( !new_vertices )
                {
                    new_vertices = new osg::Vec3Array( *_vertices );
                    geometry->setVertexArray( new_vertices );
                }

                // allocate the primitive set to store the face geometry
                osg::ref_ptr<osg::DrawElementsUShort> face =
                    new osg::DrawElementsUShort( GL_POLYGON );
                face->setName( faceName );

                // reserve enough space in the vertex array to accommodate the vertices
                // associated with the segments
                new_vertices->reserve(
                    new_vertices->size() + _segments.size() + 1 + count
                );

                // create vertices
                unsigned int previous_second = _segments[0].second;

                osg::vec3    boundaryPoint, newPoint;
                computeBisectorPoint( 0, boundaryPoint, newPoint );

                unsigned int first = static_cast<unsigned int>( new_vertices->size() );
                new_vertices->push_back( newPoint );

                if( _segments[0].first != start )
                {
                    // OSG_NOTICE<<"We have pruned from the start"<<std::endl;
                    for( unsigned int j = start; j <= _segments[0].first; ++j )
                    {
                        face->push_back( static_cast<unsigned short>( first ) );
                    }
                }
                else
                {
                    face->push_back( static_cast<unsigned short>( first ) );
                }

                for( unsigned int i = 1; i < _segments.size(); ++i )
                {

                    computeBisectorPoint( i, boundaryPoint, newPoint );

                    unsigned int vi = static_cast<unsigned int>( new_vertices->size() );
                    new_vertices->push_back( newPoint );

                    if( previous_second != _segments[i].first )
                    {
                        // OSG_NOTICE<<"Gap in boundary"<<previous_second<<" to
                        // "<<_segments[i].first<<std::endl;
                        for( unsigned int j = previous_second; j <= _segments[i].first;
                             ++j )
                        {
                            face->push_back( static_cast<unsigned short>( vi ) );
                        }
                    }
                    else
                    {
                        face->push_back( static_cast<unsigned short>( vi ) );
                    }

                    previous_second = _segments[i].second;
                }

                // fill the end of the polygon with repititions of the first index in the
                // polygon to ensure that the original and new boundary polygons have the
                // same number and pairing of indices. This ensures that the bevel can be
                // created coherently.
                while( face->size() < count )
                {
                    face->push_back( static_cast<unsigned short>( first ) );
                }

                if( !faceName.empty() )
                {
                    // add face primitive set for polygon
                    geometry->addPrimitiveSet( face.get() );
                }

                osg::DrawElementsUShort* bevel =
                    new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
                bevel->setName( bevelName );
                bevel->reserve( count * 2 );
                for( unsigned int i = 0; i < count; ++i )
                {
                    bevel->push_back( ( *_elements )[i] );
                    bevel->push_back( ( *face )[i] );
                }
                geometry->addPrimitiveSet( bevel );
            }

        protected:

            virtual ~Boundary()
            {
            }
    };

    /////////////////////////////////////////////////////////////////////////////////////////
    //
    // computeGlyphGeometry
    //
    struct CollectTriangleIndicesFunctor
    {
            CollectTriangleIndicesFunctor()
            {
            }

            typedef std::vector<unsigned int> Indices;
            Indices                           _indices;

            void
            operator()( unsigned int p1,
                        unsigned int p2,
                        unsigned int p3 )
            {
                if( p1 == p2 || p2 == p3 || p1 == p3 )
                {
                    return;
                }

                _indices.push_back( p1 );
                _indices.push_back( p3 );
                _indices.push_back( p2 );
            }
    };

    OSGTEXT_EXPORT osg::Geometry*
                   computeGlyphGeometry( const osgText::Glyph3D* glyph,
                                         const Bevel&            bevel,
                                         float                   shellThickness )
    {
#if REPORT_TIME
        osg::ElapsedTime timer;
#endif

        float                 bevelThickness  = bevel.getBevelThickness();
        bool                  roundedWebs     = bevel.getSmoothConcaveJunctions();

        const osg::Vec3Array* source_vertices = glyph->getRawVertexArray();
        const osg::Geometry::PrimitiveSetList& source_primitives =
            glyph->getRawFacePrimitiveSetList();

        if( !source_vertices )
        {
            return NULL;
        }
        if( source_primitives.empty() )
        {
            return NULL;
        }

#if 0
    {
        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        geom->setVertexArray(const_cast<osg::Vec3Array*>(source_vertices));
        geom->setPrimitiveSetList(source_primitives);

        osgDB::writeNodeFile(*geom, "test.osgt");
    }
#endif

        osg::ref_ptr<osg::Vec3Array>    orig_vertices = new osg::Vec3Array;
        osg::Geometry::PrimitiveSetList orig_primitives;

        orig_vertices->reserve( source_vertices->size() );
        orig_primitives.reserve( source_primitives.size() );

        typedef std::vector<int> Indices;
        Indices                  remappedIndices( source_vertices->size(), -1 );

        float                    convexCornerInsertionWidth = 0.02F;
        float                    convexCornerCutoffAngle    = osg::radians( 45.0F );

        int                      num_vertices_on_web        = 10;

        for( osg::Geometry::PrimitiveSetList::const_iterator itr =
                 source_primitives.begin();
             itr != source_primitives.end();
             ++itr )
        {
            const osg::DrawElementsUShort* elements =
                dynamic_cast<const osg::DrawElementsUShort*>( itr->get() );
            if( elements && elements->size() > 2 )
            {
                osg::ref_ptr<osg::DrawElementsUShort> new_elements =
                    new osg::DrawElementsUShort( elements->getMode() );
                orig_primitives.push_back( new_elements.get() );

                int num_indices = static_cast<int>( elements->size() );
                for( int ei = 0; ei < num_indices - 1; ++ei )
                {
                    int vi_before =
                        ( ei == 0 ) ? ( *elements )[( elements->size() - 2 )]
                                    : ( *elements )[static_cast<std::size_t>( ei - 1 )];
                    int vi_curr  = ( *elements )[static_cast<std::size_t>( ei )];
                    int vi_after = ( *elements )[static_cast<std::size_t>( ei + 1 )];

                    osg::vec3 va =
                        ( *source_vertices )[static_cast<std::size_t>( vi_before )];
                    osg::vec3 vb =
                        ( *source_vertices )[static_cast<std::size_t>( vi_curr )];
                    osg::vec3 vc =
                        ( *source_vertices )[static_cast<std::size_t>( vi_after )];

                    // OSG_NOTICE<<"   "<<vi_before<<", "<<vi_curr<<",
                    // "<<vi_after<<std::endl;

                    if( vi_curr >= static_cast<int>( remappedIndices.size() ) )
                    {
                        remappedIndices.resize( static_cast<std::size_t>( vi_curr ) + 1,
                                                -1 );
                    }

                    int new_index = remappedIndices[static_cast<std::size_t>( vi_curr )];
                    if( new_index < 0 )
                    {
                        remappedIndices[static_cast<std::size_t>( vi_curr )] =
                            new_index = static_cast<int>( orig_vertices->size() );
                        orig_vertices->push_back( vb );
                    }
                    new_elements->push_back( static_cast<unsigned short>( new_index ) );

                    if( roundedWebs )
                    {
                        osg::vec3 vab     = vb - va;
                        osg::vec3 vbc     = vc - vb;
                        float     len_vab = osg::length( vab );
                        vab               = osg::normalize( vab );
                        float len_vbc     = osg::length( vbc );
                        vbc               = osg::normalize( vbc );

                        if( len_vab * len_vbc == 0.0F )
                        {
                            OSG_NOTICE << "Warning: len_vab=" << len_vab
                                       << ", len_vbc=" << len_vbc << std::endl;
                            continue;
                        }

                        osg::vec3 crossv = osg::cross( vab, vbc );
                        if( crossv.z > 0.0 )
                        {
                            float dot   = osg::dot( vab, vbc );

                            float theta = static_cast<float>( atan2( crossv.z, dot ) );

                            // OSG_NOTICE<<"  convex segment  vab=("<<vab<<")
                            // vbc=("<<vbc<<") theta"<<osg::degrees(theta)<<std::endl;

                            if( theta > convexCornerCutoffAngle )
                            {
                                vab               = osg::normalize( vab );
                                vbc               = osg::normalize( vbc );

                                float     min_len = std::min( len_vab, len_vbc );

                                osg::vec3 v_before =
                                    vb - vab * ( min_len * convexCornerInsertionWidth );
                                osg::vec3 v_after =
                                    vb + vbc * ( min_len * convexCornerInsertionWidth );
                                osg::vec3 v_mid =
                                    v_before + ( v_after - v_before ) * 0.5F;

                                float h = osg::length( vb - v_mid );
                                float w = osg::length( v_after - v_before ) * 0.5F;
                                float l = w * w / h;
                                float r = static_cast<float>( sqrt( l * l + w * w ) );
                                float alpha = static_cast<float>( atan2( w, h ) );
                                float beta =
                                    static_cast<float>( osg::PI ) - alpha * 2.0F;

                                // OSG_NOTICE<<"    h = "<<h<<", w = "<<w<<", l =
                                // "<<l<<", r="<<r<<", alpha="<<osg::degrees(alpha)<<",
                                // beta="<<osg::degrees(beta)<<std::endl;

                                osg::vec3 vertical = ( vb - v_mid ) * ( 1.0F / h );
                                osg::vec3 horizontal =
                                    ( v_after - v_before ) * ( r / ( w * 2.0F ) );

                                vertical            = osg::normalize( vertical );

                                osg::vec3 v_center  = v_mid - vertical * l;
                                vertical           *= r;

                                ( *orig_vertices )[static_cast<std::size_t>(
                                    new_index
                                )]                  = v_before;

                                for( int i = 1; i <= num_vertices_on_web - 1; ++i )
                                {
                                    float gamma =
                                        ( ( static_cast<float>( i ) /
                                            static_cast<float>( num_vertices_on_web ) ) -
                                          0.5F ) *
                                        beta;
                                    // OSG_NOTICE<<"     gamma =
                                    // "<<osg::degrees(gamma)<<"
                                    // sin(gamma)="<<sin(gamma)<<",
                                    // cos(gamma)="<<cos(gamma)<<std::endl;

                                    osg::vec3 v = v_center +
                                                  horizontal *
                                                  static_cast<float>( sin( gamma ) ) +
                                                  vertical *
                                                  static_cast<float>( cos( gamma ) );

                                    new_elements->push_back( static_cast<unsigned short>(
                                        orig_vertices->size()
                                    ) );
                                    orig_vertices->push_back( v );
                                }
                                new_elements->push_back(
                                    static_cast<unsigned short>( orig_vertices->size() )
                                );
                                orig_vertices->push_back( v_after );
                            }
                        }
                    }
                }

                // add the first element to create the loop.
                new_elements->push_back( new_elements->front() );
            }
        }

        if( !orig_vertices )
        {
            return 0;
        }
        if( orig_primitives.empty() )
        {
            return 0;
        }

        osg::ref_ptr<osg::Geometry> new_geometry = new osg::Geometry;

#define HANDLE_SHELL 0

        typedef std::vector<osg::ref_ptr<Boundary>> Boundaries;
        Boundaries                                  innerBoundaries;
        Boundaries                                  outerBoundaries;

        for( osg::Geometry::PrimitiveSetList::const_iterator itr =
                 orig_primitives.begin();
             itr != orig_primitives.end();
             ++itr )
        {
            if( ( *itr )->getMode() == GL_POLYGON )
            {
                osg::ref_ptr<Boundary> boundaryInner =
                    new Boundary( orig_vertices.get(), itr->get(), bevelThickness );
                // boundaryInner->removeAllSegmentsBelowThickness(bevelThickness);
                innerBoundaries.push_back( boundaryInner.get() );
#if HANDLE_SHELL
                osg::ref_ptr<Boundary> boundaryOuter =
                    new Boundary( orig_vertices.get(), itr->get(), -shellThickness );
                boundaryOuter->removeAllSegmentsAboveThickness( -shellThickness );
                outerBoundaries.push_back( boundaryOuter.get() );
#endif
            }
        }

        OSG_INFO << "Handling bevel" << std::endl;
        for( unsigned int i = 0; i < innerBoundaries.size(); ++i )
        {
            for( unsigned int j = 0; j < innerBoundaries.size(); ++j )
            {
                innerBoundaries[i]->checkBoundaries( *innerBoundaries[j] );
            }
        }

        float smallest = FLT_MAX, largest = -FLT_MAX;
        for( unsigned int i = 0; i < innerBoundaries.size(); ++i )
        {
            innerBoundaries[i]->getSuggestedThicknessRange( smallest, largest );
        }

        OSG_INFO << "Smallest = " << smallest << std::endl;
        OSG_INFO << "Largest = " << largest << std::endl;

        float targetThickness = largest * 0.75F;

#if 1
        for( unsigned int i = 0; i < innerBoundaries.size(); ++i )
        {
            innerBoundaries[i]->applyThickness( targetThickness );
        }

        for( Boundaries::iterator itr = innerBoundaries.begin();
             itr != innerBoundaries.end();
             ++itr )
        {
            ( *itr )->removeAllSegmentsBelowThickness( targetThickness * 0.75F );
        }

    #if 1
        for( unsigned int i = 0; i < innerBoundaries.size(); ++i )
        {
            for( unsigned int j = 0; j < innerBoundaries.size(); ++j )
            {
                innerBoundaries[i]->checkBoundaries( *innerBoundaries[j] );
            }
        }
    #endif
#endif

        for( Boundaries::iterator itr = innerBoundaries.begin();
             itr != innerBoundaries.end();
             ++itr )
        {
            ( *itr )->applySuggestedThickness();
            ( *itr )->addBoundaryToGeometry( new_geometry.get(),
                                             bevelThickness,
                                             "face",
                                             "bevel" );
        }

        OSG_INFO << "Handling shell" << std::endl;
        for( unsigned int i = 0; i < outerBoundaries.size(); ++i )
        {
            for( unsigned int j = 0; j < outerBoundaries.size(); ++j )
            {
                outerBoundaries[i]->checkBoundaries( *outerBoundaries[j] );
            }
        }

        for( Boundaries::iterator itr = outerBoundaries.begin();
             itr != outerBoundaries.end();
             ++itr )
        {
            ( *itr )->applySuggestedThickness();
            ( *itr )->addBoundaryToGeometry( new_geometry.get(),
                                             -shellThickness,
                                             "",
                                             "shell" );
        }

        osg::Vec3Array* vertices =
            dynamic_cast<osg::Vec3Array*>( new_geometry->getVertexArray() );

        // need to tessellate the inner boundary
        {
            osg::ref_ptr<osg::Geometry> face_geometry = new osg::Geometry;
            face_geometry->setVertexArray( vertices );

            osg::CopyOp                     copyop( osg::CopyOp::DEEP_COPY_ALL );

            osg::Geometry::PrimitiveSetList primitiveSets;

            for( osg::Geometry::PrimitiveSetList::iterator itr =
                     new_geometry->getPrimitiveSetList().begin();
                 itr != new_geometry->getPrimitiveSetList().end();
                 ++itr )
            {
                osg::PrimitiveSet* prim = itr->get();
                if( prim->getName() == "face" )
                {
                    face_geometry->addPrimitiveSet( copyop( itr->get() ) );
                }
                else
                {
                    primitiveSets.push_back( prim );
                }
            }

            osgUtil::Tessellator ts;
            ts.setWindingType( osgUtil::Tessellator::TESS_WINDING_POSITIVE );
            ts.setTessellationType( osgUtil::Tessellator::TESS_TYPE_GEOMETRY );
            ts.retessellatePolygons( *face_geometry );

            osg::TriangleIndexFunctor<CollectTriangleIndicesFunctor> ctif;
            face_geometry->accept( ctif );
            CollectTriangleIndicesFunctor::Indices& indices = ctif._indices;

            // remove the previous primitive sets
            new_geometry->getPrimitiveSetList().clear();

            // create a front face using triangle indices
            osg::DrawElementsUShort* front_face =
                new osg::DrawElementsUShort( GL_TRIANGLES );
            front_face->setName( "face" );
            new_geometry->addPrimitiveSet( front_face );
            for( unsigned int i = 0; i < indices.size(); ++i )
            {
                front_face->push_back( static_cast<unsigned short>( indices[i] ) );
            }

            for( osg::Geometry::PrimitiveSetList::iterator itr = primitiveSets.begin();
                 itr != primitiveSets.end();
                 ++itr )
            {
                osg::PrimitiveSet* prim = itr->get();
                if( prim->getName() != "face" )
                {
                    new_geometry->addPrimitiveSet( prim );
                }
            }
        }

#if REPORT_TIME
        OSG_NOTICE << "Time to compute 3d glyp geometry: " << timer.elapsedTime_m()
                   << "ms" << std::endl;
#endif

        return new_geometry.release();
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    //
    // computeTextGeometry
    //
    OSGTEXT_EXPORT osg::Geometry*
                   computeTextGeometry( const osgText::Glyph3D* glyph,
                                        float                   width )
    {
        const osg::Vec3Array* orig_vertices = glyph->getRawVertexArray();
        const osg::Geometry::PrimitiveSetList& orig_primitives =
            glyph->getRawFacePrimitiveSetList();

        osg::ref_ptr<osg::Geometry>  text_geometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array( *orig_vertices );

        text_geometry->setVertexArray( vertices.get() );
        text_geometry->setPrimitiveSetList( orig_primitives );

        osgUtil::Tessellator ts;
        ts.setWindingType( osgUtil::Tessellator::TESS_WINDING_POSITIVE );
        ts.setTessellationType( osgUtil::Tessellator::TESS_TYPE_GEOMETRY );
        ts.retessellatePolygons( *text_geometry );

        osg::TriangleIndexFunctor<CollectTriangleIndicesFunctor> ctif;
        text_geometry->accept( ctif );
        CollectTriangleIndicesFunctor::Indices& indices = ctif._indices;

        // remove the previous primitive sets
        text_geometry->getPrimitiveSetList().clear();

        if( indices.empty() )
        {
            return 0;
        }

        // create a front face using triangle indices
        osg::DrawElementsUShort* frontFace = new osg::DrawElementsUShort( GL_TRIANGLES );
        frontFace->setName( "front" );
        text_geometry->addPrimitiveSet( frontFace );
        for( unsigned int i = 0; i < indices.size(); ++i )
        {
            frontFace->push_back( static_cast<unsigned short>( indices[i] ) );
        }

        typedef std::vector<unsigned int> Indices;
        const unsigned int                NULL_VALUE = UINT_MAX;
        Indices                           back_indices;
        back_indices.resize( vertices->size(), NULL_VALUE );
        osg::vec3                forward( 0, 0, -width );

        // build up the vertices primitives for the back face, and record the indices
        // for later use, and to ensure sharing of vertices in the face primitive set
        // the order of the triangle indices are flipped to make sure that the triangles
        // are back face
        osg::DrawElementsUShort* backFace = new osg::DrawElementsUShort( GL_TRIANGLES );
        backFace->setName( "back" );
        text_geometry->addPrimitiveSet( backFace );
        for( unsigned int i = 0; i < indices.size() - 2; )
        {
            unsigned int p1 = indices[i++];
            unsigned int p2 = indices[i++];
            unsigned int p3 = indices[i++];
            if( back_indices[p1] == NULL_VALUE )
            {
                back_indices[p1] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *vertices )[p1] + forward );
            }

            if( back_indices[p2] == NULL_VALUE )
            {
                back_indices[p2] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *vertices )[p2] + forward );
            }

            if( back_indices[p3] == NULL_VALUE )
            {
                back_indices[p3] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *vertices )[p3] + forward );
            }

            backFace->push_back( static_cast<unsigned short>( back_indices[p1] ) );
            backFace->push_back( static_cast<unsigned short>( back_indices[p3] ) );
            backFace->push_back( static_cast<unsigned short>( back_indices[p2] ) );
        }

        unsigned int orig_size = static_cast<unsigned int>( orig_vertices->size() );
        Indices      frontedge_indices, backedge_indices;
        frontedge_indices.resize( orig_size, NULL_VALUE );
        backedge_indices.resize( orig_size, NULL_VALUE );

        for( osg::Geometry::PrimitiveSetList::const_iterator itr =
                 orig_primitives.begin();
             itr != orig_primitives.end();
             ++itr )
        {
            osg::DrawElementsUShort* edging =
                new osg::DrawElementsUShort( osg::PrimitiveSet::QUAD_STRIP );
            edging->setName( "wall" );
            text_geometry->addPrimitiveSet( edging );

            osg::DrawElementsUShort* elements =
                dynamic_cast<osg::DrawElementsUShort*>( itr->get() );
            if( elements )
            {
                for( unsigned int i = 0; i < elements->size(); ++i )
                {
                    unsigned int ei = ( *elements )[i];
                    if( frontedge_indices[ei] == NULL_VALUE )
                    {
                        frontedge_indices[ei] =
                            static_cast<unsigned int>( vertices->size() );
                        vertices->push_back( ( *orig_vertices )[ei] );
                    }
                    if( backedge_indices[ei] == NULL_VALUE )
                    {
                        backedge_indices[ei] =
                            static_cast<unsigned int>( vertices->size() );
                        vertices->push_back( ( *orig_vertices )[ei] + forward );
                    }

                    edging->push_back(
                        static_cast<unsigned short>( backedge_indices[ei] )
                    );
                    edging->push_back(
                        static_cast<unsigned short>( frontedge_indices[ei] )
                    );
                }
            }
        }

        return text_geometry.release();
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    //
    // computeTextGeometry
    //
    OSGTEXT_EXPORT osg::Geometry*
                   computeTextGeometry( osg::Geometry*        glyphGeometry,
                                        const osgText::Bevel& profile,
                                        float                 width )
    {
        if( !glyphGeometry )
        {
            OSG_NOTICE << "Warning: computeTextGeometry(..) error, glyphGeometry="
                       << glyphGeometry << std::endl;
            return 0;
        }

        osg::Vec3Array* orig_vertices =
            dynamic_cast<osg::Vec3Array*>( glyphGeometry->getVertexArray() );
        if( !orig_vertices )
        {
            OSG_INFO << "Warning: computeTextGeometry(..): No vertices on glyphGeometry."
                     << std::endl;
            return 0;
        }

        osg::ref_ptr<osg::Geometry>  text_geometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> vertices      = new osg::Vec3Array;
        text_geometry->setVertexArray( vertices.get() );

        typedef std::vector<unsigned int> Indices;
        const unsigned int                NULL_VALUE = UINT_MAX;
        Indices                           front_indices, back_indices;
        front_indices.resize( orig_vertices->size(), NULL_VALUE );
        back_indices.resize( orig_vertices->size(), NULL_VALUE );

        osg::DrawElementsUShort*        face = 0;
        osg::Geometry::PrimitiveSetList bevelPrimitiveSets;
        osg::vec3                       forward( 0, 0, -width );

        // collect bevels and face primitive sets
        for( osg::Geometry::PrimitiveSetList::iterator itr =
                 glyphGeometry->getPrimitiveSetList().begin();
             itr != glyphGeometry->getPrimitiveSetList().end();
             ++itr )
        {
            osg::PrimitiveSet* prim = itr->get();
            if( prim->getName() == "face" )
            {
                face = dynamic_cast<osg::DrawElementsUShort*>( prim );
            }
            else if( prim->getName() == "bevel" )
            {
                bevelPrimitiveSets.push_back( prim );
            }
        }

        // if we don't have a face we can't create any 3d text
        if( !face )
        {
            return 0;
        }

        // face doesn't have enough vertices on it to represent a polygon.
        if( face->size() < 3 )
        {
            OSG_NOTICE << "Face does not have enough elements to be able to represent a "
                          "polygon, face->size() = "
                       << face->size() << std::endl;
            return 0;
        }

        // build up the vertices primitives for the front face, and record the indices
        // for later use, and to ensure sharing of vertices in the face primitive set
        osg::DrawElementsUShort* frontFace = new osg::DrawElementsUShort( GL_TRIANGLES );
        frontFace->setName( "front" );
        text_geometry->addPrimitiveSet( frontFace );
        for( unsigned int i = 0; i < face->size(); )
        {
            unsigned int pi = ( *face )[i++];
            if( front_indices[pi] == NULL_VALUE )
            {
                front_indices[pi] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *orig_vertices )[pi] );
            }
            frontFace->push_back( static_cast<unsigned short>( front_indices[pi] ) );
        }

        // build up the vertices primitives for the back face, and record the indices
        // for later use, and to ensure sharing of vertices in the face primitive set
        // the order of the triangle indices are flipped to make sure that the triangles
        // are back face
        osg::DrawElementsUShort* backFace = new osg::DrawElementsUShort( GL_TRIANGLES );
        backFace->setName( "back" );
        text_geometry->addPrimitiveSet( backFace );
        for( unsigned int i = 0; i < face->size() - 2; )
        {
            unsigned int p1 = ( *face )[i++];
            unsigned int p2 = ( *face )[i++];
            unsigned int p3 = ( *face )[i++];
            if( back_indices[p1] == NULL_VALUE )
            {
                back_indices[p1] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *orig_vertices )[p1] + forward );
            }

            if( back_indices[p2] == NULL_VALUE )
            {
                back_indices[p2] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *orig_vertices )[p2] + forward );
            }

            if( back_indices[p3] == NULL_VALUE )
            {
                back_indices[p3] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *orig_vertices )[p3] + forward );
            }

            backFace->push_back( static_cast<unsigned short>( back_indices[p1] ) );
            backFace->push_back( static_cast<unsigned short>( back_indices[p3] ) );
            backFace->push_back( static_cast<unsigned short>( back_indices[p2] ) );
        }

        // now build up the bevel
        for( osg::Geometry::PrimitiveSetList::iterator itr = bevelPrimitiveSets.begin();
             itr != bevelPrimitiveSets.end();
             ++itr )
        {
            osg::DrawElementsUShort* bevel =
                dynamic_cast<osg::DrawElementsUShort*>( itr->get() );
            if( !bevel )
            {
                continue;
            }

            unsigned int no_vertices_on_boundary =
                static_cast<unsigned int>( bevel->size() ) / 2;

            const osgText::Bevel::Vertices& profileVertices = profile.getVertices();
            unsigned int                    no_vertices_on_bevel =
                static_cast<unsigned int>( profileVertices.size() );

            Indices bevelIndices;
            bevelIndices.resize( no_vertices_on_boundary * no_vertices_on_bevel,
                                 NULL_VALUE );

            // populate vertices
            for( unsigned int i = 0; i < no_vertices_on_boundary; ++i )
            {
                unsigned int topi        = ( *bevel )[i * 2];
                unsigned int basei       = ( *bevel )[i * 2 + 1];

                osg::vec3&   top_vertex  = ( *orig_vertices )[topi];
                osg::vec3&   base_vertex = ( *orig_vertices )[basei];
                osg::vec3    up          = top_vertex - base_vertex;

                if( front_indices[basei] == NULL_VALUE )
                {
                    front_indices[basei] = static_cast<unsigned int>( vertices->size() );
                    vertices->push_back( base_vertex );
                }

                bevelIndices[i * no_vertices_on_bevel + 0] = front_indices[basei];

                for( unsigned int j = 1; j < no_vertices_on_bevel - 1; ++j )
                {
                    const osg::vec2& pv = profileVertices[j];
                    osg::vec3 pos( base_vertex + ( forward * pv.x ) + ( up * pv.y ) );
                    bevelIndices[i * no_vertices_on_bevel + j] =
                        static_cast<unsigned int>( vertices->size() );
                    vertices->push_back( pos );
                }

                if( back_indices[basei] == NULL_VALUE )
                {
                    back_indices[basei] = static_cast<unsigned int>( vertices->size() );
                    vertices->push_back( base_vertex + forward );
                }

                bevelIndices[i * no_vertices_on_bevel + no_vertices_on_bevel - 1] =
                    back_indices[basei];
            }

            osg::DrawElementsUShort* elements =
                new osg::DrawElementsUShort( GL_TRIANGLES );
            elements->setName( "wall" );
            unsigned int base, next;
            for( unsigned int i = 0; i < no_vertices_on_boundary - 1; ++i )
            {
                for( unsigned int j = 0; j < no_vertices_on_bevel - 1; ++j )
                {
                    base = i * no_vertices_on_bevel + j;
                    next = base + no_vertices_on_bevel;

                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[base] )
                    );
                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[next] )
                    );
                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[base + 1] )
                    );

                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[base + 1] )
                    );
                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[next] )
                    );
                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[next + 1] )
                    );
                }
            }

            text_geometry->addPrimitiveSet( elements );
        }

        return text_geometry.release();
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    //
    // computeShellGeometry
    //
    OSGTEXT_EXPORT osg::Geometry*
                   computeShellGeometry( osg::Geometry*        glyphGeometry,
                                         const osgText::Bevel& profile,
                                         float                 width )
    {
        if( !glyphGeometry )
        {
            OSG_NOTICE << "Warning: computeShellGeometry(..) error, glyphGeometry="
                       << glyphGeometry << std::endl;
            return 0;
        }

        osg::Vec3Array* orig_vertices =
            dynamic_cast<osg::Vec3Array*>( glyphGeometry->getVertexArray() );
        if( !orig_vertices )
        {
            OSG_NOTICE << "computeTextGeometry(..): No vertices on glyphGeometry."
                       << std::endl;
            return 0;
        }

        osg::ref_ptr<osg::Geometry>  text_geometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> vertices      = new osg::Vec3Array;
        text_geometry->setVertexArray( vertices.get() );

        typedef std::vector<unsigned int> Indices;
        const unsigned int                NULL_VALUE = UINT_MAX;
        Indices                           front_indices, back_indices;
        front_indices.resize( orig_vertices->size(), NULL_VALUE );
        back_indices.resize( orig_vertices->size(), NULL_VALUE );

        osg::DrawElementsUShort*        face = 0;
        osg::Geometry::PrimitiveSetList bevelPrimitiveSets;
        osg::Geometry::PrimitiveSetList shellPrimitiveSets;
        osg::vec3                       frontOffset( 0, 0, width );
        osg::vec3                       backOffset( 0, 0, -2.0F * width );
        osg::vec3                       forward( backOffset - frontOffset );

        // collect bevels and face primitive sets
        for( osg::Geometry::PrimitiveSetList::iterator itr =
                 glyphGeometry->getPrimitiveSetList().begin();
             itr != glyphGeometry->getPrimitiveSetList().end();
             ++itr )
        {
            osg::PrimitiveSet* prim = itr->get();
            if( prim->getName() == "face" )
            {
                face = dynamic_cast<osg::DrawElementsUShort*>( prim );
            }
            else if( prim->getName() == "bevel" )
            {
                bevelPrimitiveSets.push_back( prim );
            }
            else if( prim->getName() == "shell" )
            {
                shellPrimitiveSets.push_back( prim );
            }
        }

        // if we don't have a face we can't create any 3d text
        if( !face )
        {
            return 0;
        }

        // build up the vertices primitives for the front face, and record the indices
        // for later use, and to ensure sharing of vertices in the face primitive set
        // the order of the triangle indices are flipped to make sure that the triangles
        // are back face
        osg::DrawElementsUShort* frontFace = new osg::DrawElementsUShort( GL_TRIANGLES );
        text_geometry->addPrimitiveSet( frontFace );
        for( unsigned int i = 0; i < face->size() - 2; )
        {
            unsigned int p1 = ( *face )[i++];
            unsigned int p2 = ( *face )[i++];
            unsigned int p3 = ( *face )[i++];
            if( front_indices[p1] == NULL_VALUE )
            {
                front_indices[p1] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *orig_vertices )[p1] + frontOffset );
            }

            if( front_indices[p2] == NULL_VALUE )
            {
                front_indices[p2] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *orig_vertices )[p2] + frontOffset );
            }

            if( front_indices[p3] == NULL_VALUE )
            {
                front_indices[p3] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *orig_vertices )[p3] + frontOffset );
            }

            frontFace->push_back( static_cast<unsigned short>( front_indices[p1] ) );
            frontFace->push_back( static_cast<unsigned short>( front_indices[p3] ) );
            frontFace->push_back( static_cast<unsigned short>( front_indices[p2] ) );
        }

        // build up the vertices primitives for the back face, and record the indices
        // for later use, and to ensure sharing of vertices in the face primitive set
        osg::DrawElementsUShort* backFace = new osg::DrawElementsUShort( GL_TRIANGLES );
        text_geometry->addPrimitiveSet( backFace );
        for( unsigned int i = 0; i < face->size(); )
        {
            unsigned int pi = ( *face )[i++];
            if( back_indices[pi] == NULL_VALUE )
            {
                back_indices[pi] = static_cast<unsigned int>( vertices->size() );
                vertices->push_back( ( *orig_vertices )[pi] + backOffset );
            }
            backFace->push_back( static_cast<unsigned short>( back_indices[pi] ) );
        }

        for( osg::Geometry::PrimitiveSetList::iterator itr = bevelPrimitiveSets.begin();
             itr != bevelPrimitiveSets.end();
             ++itr )
        {
            osg::DrawElementsUShort* strip =
                dynamic_cast<osg::DrawElementsUShort*>( itr->get() );
            if( !strip )
            {
                continue;
            }

            osg::CopyOp              copyop( osg::CopyOp::DEEP_COPY_ALL );

            osg::DrawElementsUShort* front_strip =
                new osg::DrawElementsUShort( *strip, copyop );
            text_geometry->addPrimitiveSet( front_strip );
            for( unsigned int i = 0; i < front_strip->size(); ++i )
            {
                unsigned short& pi = ( *front_strip )[i];
                if( front_indices[pi] == NULL_VALUE )
                {
                    front_indices[pi] = static_cast<unsigned int>( vertices->size() );
                    vertices->push_back( ( *orig_vertices )[pi] + frontOffset );
                }
                pi = static_cast<unsigned short>( front_indices[pi] );
            }

            for( unsigned int i = 0; i < front_strip->size() - 1; )
            {
                unsigned short& p1 = ( *front_strip )[i++];
                unsigned short& p2 = ( *front_strip )[i++];
                std::swap( p1, p2 );
            }

            osg::DrawElementsUShort* back_strip =
                new osg::DrawElementsUShort( *strip, copyop );
            text_geometry->addPrimitiveSet( back_strip );
            for( unsigned int i = 0; i < back_strip->size(); ++i )
            {
                unsigned short& pi = ( *back_strip )[i];
                if( back_indices[pi] == NULL_VALUE )
                {
                    back_indices[pi] = static_cast<unsigned int>( vertices->size() );
                    vertices->push_back( ( *orig_vertices )[pi] + backOffset );
                }
                pi = static_cast<unsigned short>( back_indices[pi] );
            }
        }

        // now build up the shell
        for( osg::Geometry::PrimitiveSetList::iterator itr = shellPrimitiveSets.begin();
             itr != shellPrimitiveSets.end();
             ++itr )
        {
            osg::DrawElementsUShort* bevel =
                dynamic_cast<osg::DrawElementsUShort*>( itr->get() );
            if( !bevel )
            {
                continue;
            }

            unsigned int no_vertices_on_boundary =
                static_cast<unsigned int>( bevel->size() ) / 2;

            const osgText::Bevel::Vertices& profileVertices = profile.getVertices();
            unsigned int                    no_vertices_on_bevel =
                static_cast<unsigned int>( profileVertices.size() );

            Indices bevelIndices;
            bevelIndices.resize( no_vertices_on_boundary * no_vertices_on_bevel,
                                 NULL_VALUE );

            // populate vertices
            for( unsigned int i = 0; i < no_vertices_on_boundary; ++i )
            {
                unsigned int topi        = ( *bevel )[i * 2 + 1];
                unsigned int basei       = ( *bevel )[i * 2];

                osg::vec3    top_vertex  = ( *orig_vertices )[topi] + frontOffset;
                osg::vec3    base_vertex = ( *orig_vertices )[basei] + frontOffset;
                osg::vec3    up          = top_vertex - base_vertex;

                if( front_indices[basei] == NULL_VALUE )
                {
                    front_indices[basei] = static_cast<unsigned int>( vertices->size() );
                    vertices->push_back( base_vertex );
                }

                bevelIndices[i * no_vertices_on_bevel + 0] = front_indices[basei];

                for( unsigned int j = 1; j < no_vertices_on_bevel - 1; ++j )
                {
                    const osg::vec2& pv = profileVertices[j];
                    osg::vec3 pos( base_vertex + ( forward * pv.x ) + ( up * pv.y ) );
                    bevelIndices[i * no_vertices_on_bevel + j] =
                        static_cast<unsigned int>( vertices->size() );
                    vertices->push_back( pos );
                }

                if( back_indices[basei] == NULL_VALUE )
                {
                    back_indices[basei] = static_cast<unsigned int>( vertices->size() );
                    vertices->push_back( base_vertex + forward );
                }

                bevelIndices[i * no_vertices_on_bevel + no_vertices_on_bevel - 1] =
                    back_indices[basei];
            }

            osg::DrawElementsUShort* elements =
                new osg::DrawElementsUShort( GL_TRIANGLES );
            unsigned int base, next;
            for( unsigned int i = 0; i < no_vertices_on_boundary - 1; ++i )
            {
                for( unsigned int j = 0; j < no_vertices_on_bevel - 1; ++j )
                {
                    base = i * no_vertices_on_bevel + j;
                    next = base + no_vertices_on_bevel;

                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[base] )
                    );
                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[base + 1] )
                    );
                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[next] )
                    );

                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[base + 1] )
                    );
                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[next + 1] )
                    );
                    elements->push_back(
                        static_cast<unsigned short>( bevelIndices[next] )
                    );
                }
            }

            text_geometry->addPrimitiveSet( elements );
        }

#if 1
        osg::Vec4Array* new_colours = new osg::Vec4Array;
        new_colours->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 0.2F ) );
        text_geometry->setColorArray( new_colours, osg::Array::BIND_OVERALL );

        osg::StateSet* stateset = text_geometry->getOrCreateStateSet();
        stateset->setMode( GL_BLEND, osg::StateAttribute::ON );
        // GL_LIGHTING removed: not in core profile
        stateset->setAttributeAndModes( new osg::CullFace, osg::StateAttribute::ON );
        // stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        stateset->setRenderBinDetails( 11, "SORT_FRONT_TO_BACK" );
#endif
        return text_geometry.release();
    }

}
