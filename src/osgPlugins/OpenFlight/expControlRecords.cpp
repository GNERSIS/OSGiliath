/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * expControlRecords — osgPlugins library implementation.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#include "DataOutputStream.hpp"
#include "FltExportVisitor.hpp"
#include "Opcodes.hpp"

namespace flt
{

    void
    FltExportVisitor::writePush()
    {
        _records->writeInt16( ( int16 )PUSH_LEVEL_OP );
        _records->writeInt16( 4 );
    }

    void
    FltExportVisitor::writePop()
    {
        _records->writeInt16( ( int16 )POP_LEVEL_OP );
        _records->writeInt16( 4 );
    }

    void
    FltExportVisitor::writePushSubface()
    {
        _records->writeInt16( ( int16 )PUSH_SUBFACE_OP );
        _records->writeInt16( 4 );
    }

    void
    FltExportVisitor::writePopSubface()
    {
        _records->writeInt16( ( int16 )POP_SUBFACE_OP );
        _records->writeInt16( 4 );
    }

}
