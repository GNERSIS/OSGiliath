#include "TileMapper.h"
#include "TXPArchive.h"
#include "TXPPagedLOD.h"
#include "TXPSeamLOD.h"

using namespace txp;
using namespace osg;

TXPSeamLOD::TXPSeamLOD()
{
    _dx = 0;
    _dy = 0;
}

TXPSeamLOD::TXPSeamLOD( int x,
                        int y,
                        int lod,
                        int dx,
                        int dy )
{
    _tid.x   = x;
    _tid.y   = y;
    _tid.lod = lod;
    _dx      = dx;
    _dy      = dy;
}

TXPSeamLOD::TXPSeamLOD( const TXPSeamLOD&  ttg,
                        const osg::CopyOp& copyop ) :
    Inherit( ttg,
             copyop )
{
    _tid = ttg._tid;
    _dx  = ttg._dx;
    _dy  = ttg._dy;
}

void
TXPSeamLOD::traverse( osg::NodeVisitor& nv )
{
    if( nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR && getNumChildren() == 2 )
    {

        TileMapper* tileMapper = dynamic_cast<TileMapper*>( nv.getUserData() );

        if( tileMapper && !tileMapper->isTileNeighbourALowerLODLevel( _tid, _dx, _dy ) )
        {
            getChild( 1 )->accept( nv );
        }
        else
        {
            getChild( 0 )->accept( nv );
        }
    }
    else
    {
        Group::traverse( nv );
    }
}
