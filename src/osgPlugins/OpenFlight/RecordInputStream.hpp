/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * RecordInputStream, derived from DataInputStream.
 * Provides: readRecord, readRecordBody, getRecordSize, getRecordBodySize.
 */
//
// OpenFlight� loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#pragma once

#include "DataInputStream.hpp"
#include "Record.hpp"

namespace flt
{

    class Document;

    typedef int             opcode_type;
    typedef std::streamsize size_type;

    class RecordInputStream : public DataInputStream
    {
        public:

            explicit RecordInputStream( std::streambuf* sb );

            bool
            readRecord( Document& );
            bool
            readRecordBody( opcode_type,
                            size_type,
                            Document& );

            inline std::streamsize
            getRecordSize() const
            {
                return _recordSize;
            }

            inline std::streamsize
            getRecordBodySize() const
            {
                return _recordSize - ( std::streamsize )4;
            }

        protected:

            std::streamsize _recordSize;
    };

}    // end namespace
