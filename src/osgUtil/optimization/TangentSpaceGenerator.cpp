#include <osgUtil/optimization/TangentSpaceGenerator.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/common.hpp>

using namespace osgUtil;

TangentSpaceGenerator::TangentSpaceGenerator() :
    osg::Referenced(),
    T_( new osg::Vec4Array ),
    B_( new osg::Vec4Array ),
    N_( new osg::Vec4Array )
{
    T_->setBinding( osg::Array::BIND_PER_VERTEX );
    T_->setNormalize( false );
    B_->setBinding( osg::Array::BIND_PER_VERTEX );
    B_->setNormalize( false );
    N_->setBinding( osg::Array::BIND_PER_VERTEX );
    N_->setNormalize( false );
}

TangentSpaceGenerator::TangentSpaceGenerator( const TangentSpaceGenerator& copy,
                                              const osg::CopyOp&           copyop ) :
    osg::Referenced( copy ),
    T_( static_cast<osg::Vec4Array*>( copyop( copy.T_.get() ) ) ),
    B_( static_cast<osg::Vec4Array*>( copyop( copy.B_.get() ) ) ),
    N_( static_cast<osg::Vec4Array*>( copyop( copy.N_.get() ) ) )
{
}

void
TangentSpaceGenerator::generate( osg::Geometry* geo,
                                 int            normal_map_tex_unit )
{
    const osg::Array* vx = geo->getVertexArray();
    const osg::Array* nx = geo->getNormalArray();
    const osg::Array* tx =
        geo->getTexCoordArray( static_cast<unsigned int>( normal_map_tex_unit ) );

    if( !vx || !tx )
    {
        return;
    }

    unsigned int vertex_count = vx->getNumElements();
    T_->assign( vertex_count, osg::vec4() );
    B_->assign( vertex_count, osg::vec4() );
    N_->assign( vertex_count, osg::vec4() );

    int i;    // VC6 doesn't like for-scoped variables

    for( unsigned int pri = 0; pri < geo->getNumPrimitiveSets(); ++pri )
    {
        osg::PrimitiveSet* pset = geo->getPrimitiveSet( pri );

        int                N    = static_cast<int>( pset->getNumIndices() );

        switch( pset->getMode() )
        {

            case osg::PrimitiveSet::TRIANGLES :
                for( i = 0; i < N; i += 3 )
                {
                    compute( pset, vx, nx, tx, i, i + 1, i + 2 );
                }
                break;

            case osg::PrimitiveSet::QUADS :
                for( i = 0; i < N; i += 4 )
                {
                    compute( pset, vx, nx, tx, i, i + 1, i + 2 );
                    compute( pset, vx, nx, tx, i + 2, i + 3, i );
                }
                break;

            case osg::PrimitiveSet::TRIANGLE_STRIP :
                if( pset->getType() == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType )
                {
                    osg::DrawArrayLengths* dal =
                        static_cast<osg::DrawArrayLengths*>( pset );
                    int j = 0;
                    for( osg::DrawArrayLengths::const_iterator pi = dal->begin();
                         pi != dal->end();
                         ++pi )
                    {
                        int iN = *pi - 2;
                        for( i = 0; i < iN; ++i, ++j )
                        {
                            if( ( i % 2 ) == 0 )
                            {
                                compute( pset, vx, nx, tx, j, j + 1, j + 2 );
                            }
                            else
                            {
                                compute( pset, vx, nx, tx, j + 1, j, j + 2 );
                            }
                        }
                        j += 2;
                    }
                }
                else
                {
                    for( i = 0; i < N - 2; ++i )
                    {
                        if( ( i % 2 ) == 0 )
                        {
                            compute( pset, vx, nx, tx, i, i + 1, i + 2 );
                        }
                        else
                        {
                            compute( pset, vx, nx, tx, i + 1, i, i + 2 );
                        }
                    }
                }
                break;

            case osg::PrimitiveSet::QUAD_STRIP :
                if( pset->getType() == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType )
                {
                    osg::DrawArrayLengths* dal =
                        static_cast<osg::DrawArrayLengths*>( pset );
                    int j = 0;
                    for( osg::DrawArrayLengths::const_iterator pi = dal->begin();
                         pi != dal->end();
                         ++pi )
                    {
                        int iN = *pi - 2;
                        for( i = 0; i < iN; ++i, ++j )
                        {
                            if( ( i % 2 ) == 0 )
                            {
                                compute( pset, vx, nx, tx, j, j + 2, j + 1 );
                            }
                            else
                            {
                                compute( pset, vx, nx, tx, j, j + 1, j + 2 );
                            }
                        }
                        j += 2;
                    }
                }
                else
                {
                    for( i = 0; i < N - 2; ++i )
                    {
                        if( ( i % 2 ) == 0 )
                        {
                            compute( pset, vx, nx, tx, i, i + 2, i + 1 );
                        }
                        else
                        {
                            compute( pset, vx, nx, tx, i, i + 1, i + 2 );
                        }
                    }
                }
                break;

            case osg::PrimitiveSet::TRIANGLE_FAN :
            case osg::PrimitiveSet::POLYGON :
                if( pset->getType() == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType )
                {
                    osg::DrawArrayLengths* dal =
                        static_cast<osg::DrawArrayLengths*>( pset );
                    int j = 0;
                    for( osg::DrawArrayLengths::const_iterator pi = dal->begin();
                         pi != dal->end();
                         ++pi )
                    {
                        int iN = *pi - 2;
                        for( i = 0; i < iN; ++i )
                        {
                            compute( pset, vx, nx, tx, 0, j + 1, j + 2 );
                        }
                        j += 2;
                    }
                }
                else
                {
                    for( i = 0; i < N - 2; ++i )
                    {
                        compute( pset, vx, nx, tx, 0, i + 1, i + 2 );
                    }
                }
                break;

            case osg::PrimitiveSet::POINTS :
            case osg::PrimitiveSet::LINES :
            case osg::PrimitiveSet::LINE_STRIP :
            case osg::PrimitiveSet::LINE_LOOP :
            case osg::PrimitiveSet::LINES_ADJACENCY :
            case osg::PrimitiveSet::LINE_STRIP_ADJACENCY :
                break;

            default :
                OSG_WARN << "Warning: TangentSpaceGenerator: unknown primitive mode "
                         << pset->getMode() << "\n";
        }
    }

    // normalize basis vectors and force the normal vector to match
    // the triangle normal's direction
    unsigned int attrib_count = vx->getNumElements();
    for( i = 0; static_cast<unsigned int>( i ) < attrib_count; ++i )
    {
        osg::vec4& vT = ( *T_ )[static_cast<std::size_t>( i )];
        osg::vec4& vB = ( *B_ )[static_cast<std::size_t>( i )];
        osg::vec4& vN = ( *N_ )[static_cast<std::size_t>( i )];

        osg::vec3  txN =
            osg::cross( osg::vec3( vT.x, vT.y, vT.z ), osg::vec3( vB.x, vB.y, vB.z ) );
        bool flipped = osg::dot( txN, osg::vec3( vN.x, vN.y, vN.z ) ) < 0;

        if( flipped )
        {
            vN = osg::vec4( -txN, 0 );
        }
        else
        {
            vN = osg::vec4( txN, 0 );
        }

        vT    = osg::normalize( vT );
        vB    = osg::normalize( vB );
        vN    = osg::normalize( vN );

        vT[3] = flipped ? -1.0F : 1.0F;
    }
    /* TO-DO: if indexed, compress the attributes to have only one
     * version of each (different indices for each one?) */
}

void
TangentSpaceGenerator::compute( osg::PrimitiveSet* pset,
                                const osg::Array*  vx,
                                const osg::Array*  nx,
                                const osg::Array*  tx,
                                int                iA,
                                int                iB,
                                int                iC )
{
    iA = static_cast<int>( pset->index( static_cast<unsigned int>( iA ) ) );
    iB = static_cast<int>( pset->index( static_cast<unsigned int>( iB ) ) );
    iC = static_cast<int>( pset->index( static_cast<unsigned int>( iC ) ) );

    osg::vec3 P1;
    osg::vec3 P2;
    osg::vec3 P3;

    int       i;    // VC6 doesn't like for-scoped variables

    switch( vx->getType() )
    {
        case osg::Array::Vec2ArrayType :
            for( i = 0; i < 2; ++i )
            {
                P1.data()[i] = static_cast<const osg::Vec2Array&>(
                                   *vx
                )[static_cast<std::size_t>( iA )]
                                   .data()[i];
                P2.data()[i] = static_cast<const osg::Vec2Array&>(
                                   *vx
                )[static_cast<std::size_t>( iB )]
                                   .data()[i];
                P3.data()[i] = static_cast<const osg::Vec2Array&>(
                                   *vx
                )[static_cast<std::size_t>( iC )]
                                   .data()[i];
            }
            break;

        case osg::Array::Vec3ArrayType :
            P1 = static_cast<const osg::Vec3Array&>(
                *vx
            )[static_cast<std::size_t>( iA )];
            P2 = static_cast<const osg::Vec3Array&>(
                *vx
            )[static_cast<std::size_t>( iB )];
            P3 = static_cast<const osg::Vec3Array&>(
                *vx
            )[static_cast<std::size_t>( iC )];
            break;

        case osg::Array::Vec4ArrayType :
            for( i = 0; i < 3; ++i )
            {
                P1.data()[i] = static_cast<const osg::Vec4Array&>(
                                   *vx
                )[static_cast<std::size_t>( iA )]
                                   .data()[i];
                P2.data()[i] = static_cast<const osg::Vec4Array&>(
                                   *vx
                )[static_cast<std::size_t>( iB )]
                                   .data()[i];
                P3.data()[i] = static_cast<const osg::Vec4Array&>(
                                   *vx
                )[static_cast<std::size_t>( iC )]
                                   .data()[i];
            }
            break;

        default :
            OSG_WARN << "Warning: TangentSpaceGenerator: vertex array must be "
                        "Vec2Array, Vec3Array or Vec4Array"
                     << std::endl;
    }

    osg::vec3 N1;
    osg::vec3 N2;
    osg::vec3 N3;

    if( nx )
    {
        switch( nx->getType() )
        {
            case osg::Array::Vec2ArrayType :
                for( i = 0; i < 2; ++i )
                {
                    N1.data()[i] = static_cast<const osg::Vec2Array&>(
                                       *nx
                    )[static_cast<std::size_t>( iA )]
                                       .data()[i];
                    N2.data()[i] = static_cast<const osg::Vec2Array&>(
                                       *nx
                    )[static_cast<std::size_t>( iB )]
                                       .data()[i];
                    N3.data()[i] = static_cast<const osg::Vec2Array&>(
                                       *nx
                    )[static_cast<std::size_t>( iC )]
                                       .data()[i];
                }
                break;

            case osg::Array::Vec3ArrayType :
                N1 = static_cast<const osg::Vec3Array&>(
                    *nx
                )[static_cast<std::size_t>( iA )];
                N2 = static_cast<const osg::Vec3Array&>(
                    *nx
                )[static_cast<std::size_t>( iB )];
                N3 = static_cast<const osg::Vec3Array&>(
                    *nx
                )[static_cast<std::size_t>( iC )];
                break;

            case osg::Array::Vec4ArrayType :
                for( i = 0; i < 3; ++i )
                {
                    N1.data()[i] = static_cast<const osg::Vec4Array&>(
                                       *nx
                    )[static_cast<std::size_t>( iA )]
                                       .data()[i];
                    N2.data()[i] = static_cast<const osg::Vec4Array&>(
                                       *nx
                    )[static_cast<std::size_t>( iB )]
                                       .data()[i];
                    N3.data()[i] = static_cast<const osg::Vec4Array&>(
                                       *nx
                    )[static_cast<std::size_t>( iC )]
                                       .data()[i];
                }
                break;

            default :
                OSG_WARN << "Warning: TangentSpaceGenerator: normal array must be "
                            "Vec2Array, Vec3Array or Vec4Array"
                         << std::endl;
        }
    }

    osg::vec2 uv1;
    osg::vec2 uv2;
    osg::vec2 uv3;

    switch( tx->getType() )
    {
        case osg::Array::Vec2ArrayType :
            uv1 = static_cast<const osg::Vec2Array&>(
                *tx
            )[static_cast<std::size_t>( iA )];
            uv2 = static_cast<const osg::Vec2Array&>(
                *tx
            )[static_cast<std::size_t>( iB )];
            uv3 = static_cast<const osg::Vec2Array&>(
                *tx
            )[static_cast<std::size_t>( iC )];
            break;

        case osg::Array::Vec3ArrayType :
            for( i = 0; i < 2; ++i )
            {
                uv1.data()[i] = static_cast<const osg::Vec3Array&>(
                                    *tx
                )[static_cast<std::size_t>( iA )]
                                    .data()[i];
                uv2.data()[i] = static_cast<const osg::Vec3Array&>(
                                    *tx
                )[static_cast<std::size_t>( iB )]
                                    .data()[i];
                uv3.data()[i] = static_cast<const osg::Vec3Array&>(
                                    *tx
                )[static_cast<std::size_t>( iC )]
                                    .data()[i];
            }
            break;

        case osg::Array::Vec4ArrayType :
            for( i = 0; i < 2; ++i )
            {
                uv1.data()[i] = static_cast<const osg::Vec4Array&>(
                                    *tx
                )[static_cast<std::size_t>( iA )]
                                    .data()[i];
                uv2.data()[i] = static_cast<const osg::Vec4Array&>(
                                    *tx
                )[static_cast<std::size_t>( iB )]
                                    .data()[i];
                uv3.data()[i] = static_cast<const osg::Vec4Array&>(
                                    *tx
                )[static_cast<std::size_t>( iC )]
                                    .data()[i];
            }
            break;

        default :
            OSG_WARN << "Warning: TangentSpaceGenerator: texture coord array must be "
                        "Vec2Array, Vec3Array or Vec4Array"
                     << std::endl;
    }

    osg::vec3 V, T1, T2, T3, B1, B2, B3;

    // no normal per vertex use the one by face
    if( !nx )
    {
        N1 = osg::cross( P2 - P1, P3 - P1 );
        N2 = N1;
        N3 = N1;
    }

    V = osg::cross( osg::vec3( P2.x - P1.x, uv2.x - uv1.x, uv2.y - uv1.y ),
                    osg::vec3( P3.x - P1.x, uv3.x - uv1.x, uv3.y - uv1.y ) );
    if( V.x != 0 )
    {
        V     = osg::normalize( V );
        T1.x += -V.y / V.x;
        B1.x += -V.z / V.x;
        T2.x += -V.y / V.x;
        B2.x += -V.z / V.x;
        T3.x += -V.y / V.x;
        B3.x += -V.z / V.x;
    }

    V = osg::cross( osg::vec3( P2.y - P1.y, uv2.x - uv1.x, uv2.y - uv1.y ),
                    osg::vec3( P3.y - P1.y, uv3.x - uv1.x, uv3.y - uv1.y ) );
    if( V.x != 0 )
    {
        V     = osg::normalize( V );
        T1.y += -V.y / V.x;
        B1.y += -V.z / V.x;
        T2.y += -V.y / V.x;
        B2.y += -V.z / V.x;
        T3.y += -V.y / V.x;
        B3.y += -V.z / V.x;
    }

    V = osg::cross( osg::vec3( P2.z - P1.z, uv2.x - uv1.x, uv2.y - uv1.y ),
                    osg::vec3( P3.z - P1.z, uv3.x - uv1.x, uv3.y - uv1.y ) );
    if( V.x != 0 )
    {
        V     = osg::normalize( V );
        T1.z += -V.y / V.x;
        B1.z += -V.z / V.x;
        T2.z += -V.y / V.x;
        B2.z += -V.z / V.x;
        T3.z += -V.y / V.x;
        B3.z += -V.z / V.x;
    }

    osg::vec3 tempvec;

    tempvec                                  = osg::cross( N1, T1 );
    ( *T_ )[static_cast<std::size_t>( iA )] += osg::vec4( osg::cross( tempvec, N1 ), 0 );

    tempvec                                  = osg::cross( B1, N1 );
    ( *B_ )[static_cast<std::size_t>( iA )] += osg::vec4( osg::cross( N1, tempvec ), 0 );

    tempvec                                  = osg::cross( N2, T2 );
    ( *T_ )[static_cast<std::size_t>( iB )] += osg::vec4( osg::cross( tempvec, N2 ), 0 );

    tempvec                                  = osg::cross( B2, N2 );
    ( *B_ )[static_cast<std::size_t>( iB )] += osg::vec4( osg::cross( N2, tempvec ), 0 );

    tempvec                                  = osg::cross( N3, T3 );
    ( *T_ )[static_cast<std::size_t>( iC )] += osg::vec4( osg::cross( tempvec, N3 ), 0 );

    tempvec                                  = osg::cross( B3, N3 );
    ( *B_ )[static_cast<std::size_t>( iC )] += osg::vec4( osg::cross( N3, tempvec ), 0 );

    ( *N_ )[static_cast<std::size_t>( iA )] += osg::vec4( N1, 0 );
    ( *N_ )[static_cast<std::size_t>( iB )] += osg::vec4( N2, 0 );
    ( *N_ )[static_cast<std::size_t>( iC )] += osg::vec4( N3, 0 );
}
