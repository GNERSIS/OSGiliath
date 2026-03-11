/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: DataInputStream, readRecordBody, good.
 */
//
// OpenFlight� loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "Document.hpp"
#include "Opcodes.hpp"
#include "RecordInputStream.hpp"
#include "Registry.hpp"

#include <iostream>

using namespace flt;
using namespace std;

RecordInputStream::RecordInputStream( std::streambuf* sb ) :
    DataInputStream( sb ),
    _recordSize( 0 )
{
}

bool
RecordInputStream::readRecord( Document& document )
{
    opcode_type opcode = ( opcode_type )readUInt16();
    size_type   size   = ( size_type )readUInt16();

    return readRecordBody( opcode, size, document );
}

bool
RecordInputStream::readRecordBody( opcode_type opcode,
                                   size_type   size,
                                   Document&   document )
{
    // Correct endian error in Creator v2.5 gallery models.
    // Last pop level record in little-endian.
    const uint16 LITTLE_ENDIAN_POP_LEVEL_OP = 0X0B'00;
    if( opcode == LITTLE_ENDIAN_POP_LEVEL_OP )
    {
        OSG_INFO << "Little endian pop-level record" << std::endl;
        opcode = POP_LEVEL_OP;
        size   = 4;
    }

    _recordSize = size;

    // Get prototype record
    Record* prototype = Registry::instance()->getPrototype( ( int )opcode );

    if( prototype )
    {
#if 0    // for debugging
        {
            for (int i=0; i<document.level(); i++)
                cout << "   ";
            cout << "opcode=" << opcode << " size=" << size;
            if (prototype) std::cout << " " << typeid(*prototype).name();
            cout << endl;
        }
#endif
        // Create from prototype.
        osg::ref_ptr<Record> record = prototype->cloneType();

        // Read record
        record->read( *this, document );
    }
    else    // prototype not found
    {
        OSG_WARN << "Unknown record, opcode=" << opcode << " size=" << size << std::endl;

        // Add to registry so we only have to see this error message once.
        Registry::instance()->addPrototype( opcode, new DummyRecord );
    }

    return good();
}
