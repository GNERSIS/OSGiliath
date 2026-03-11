/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: warning, length.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#include "DataOutputStream.hpp"
#include "FltExportVisitor.hpp"
#include "Opcodes.hpp"

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/RefMatrix.hpp>
#include <osg/nodes/MatrixTransform.hpp>

namespace flt
{

    /** If the DataOutputStream parameter is NULL, write to the _records
        member variable. Otherwise, write to the specified DataOutputStream.
        */
    void
    FltExportVisitor::writeComment( const osg::Node&  node,
                                    DataOutputStream* dos )
    {
        if( dos == NULL )
        {
            dos = _records;
        }

        // Write all descriptions as Comment records.
        unsigned int nd  = node.getNumDescriptions();
        unsigned int idx = 0;
        while( idx < nd )
        {
            const std::string& com  = node.getDescription( idx );
            unsigned int       iLen = com.length() + 5;
            if( iLen > 0XFF'FF )
            {
                // short overrun
                std::string warning( "fltexp: writeComment: Descriptions too long, "
                                     "resorts in short overrun. Skipping." );
                _fltOpt->getWriteResult().warn( warning );
                OSG_WARN << warning << std::endl;
                continue;
            }
            uint16 length( ( uint16 )iLen );

            dos->writeInt16( ( int16 )COMMENT_OP );
            dos->writeInt16( length );
            dos->writeString( com );

            idx++;
        }
    }

    /** If the DataOutputStream parameter is NULL, write to the _records
        member variable. Otherwise, write to the specified DataOutputStream.
        */
    void
    FltExportVisitor::writeLongID( const std::string& id,
                                   DataOutputStream*  dos )
    {
        if( dos == NULL )
        {
            dos = _records;
        }

        uint16 length( 2 + 2 + id.length() + 1 );    // +1 for terminating '\0'

        dos->writeInt16( ( int16 )LONG_ID_OP );
        dos->writeUInt16( length );
        dos->writeString( id );
    }

    void
    FltExportVisitor::writeMatrix( const osg::Referenced* ref )
    {
        const osg::RefMatrix* rm = dynamic_cast<const osg::RefMatrix*>( ref );
        if( !rm )
        {
            return;
        }

        uint16 length( 4 + ( 16 * sizeof( float32 ) ) );

        _records->writeInt16( ( int16 )MATRIX_OP );
        _records->writeUInt16( length );

        int idx, jdx;
        for( idx = 0; idx < 4; idx++ )
        {
            for( jdx = 0; jdx < 4; jdx++ )
            {
                _records->writeFloat32( ( *rm )( idx, jdx ) );
            }
        }
    }

    void
    FltExportVisitor::writeContinuationRecord( const unsigned short length )
    {
        OSG_DEBUG << "fltexp: Continuation record length: " << length + 4 << std::endl;
        _records->writeInt16( ( int16 )CONTINUATION_OP );
        _records->writeUInt16( length + 4 );
    }

}
