/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DataOutputStream, derived from ostream.
 * Provides: writeInt8, writeUInt8, writeInt16, writeUInt16, writeInt32, writeUInt32.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#pragma once

#include "Types.hpp"

#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <ostream>
#include <string>

// #include "Export.h"

namespace flt
{

    class Record;

    class DataOutputStream : public std::ostream
    {
        public:

            explicit DataOutputStream( std::streambuf* sb,
                                       bool            validate = false );

            void
            writeInt8( const int8 val );
            void
            writeUInt8( const uint8 val );
            void
            writeInt16( const int16 val );
            void
            writeUInt16( const uint16 val );
            void
            writeInt32( const int32 val );
            void
            writeUInt32( const uint32 val );
            void
            writeFloat32( const float32 val );
            void
            writeFloat64( const float64 val );

            // Write the entire string. If nullTerminate is true, write an additional
            // NULL. Always writes either 'val.size()' bytes or 'val.size()+1' bytes.
            void
            writeString( const std::string& val,
                         bool               nullTerminate = true );

            // Never write more than 'size-1' bytes from 'val', and write 'fill' so that
            // 'size' bytes total are written. Always writes 'size' bytes..
            void
            writeString( const std::string& val,
                         int                size,
                         char               fill = '\0' );

            void
            writeID( const std::string& val );
            void
            writeVec2f( const osg::vec2& val );
            void
            writeVec3f( const osg::vec3& val );
            void
            writeVec4f( const osg::vec4& val );
            void
            writeVec3d( const osg::dvec3& val );

            void
            writeFill( int        sizeBytes,
                       const char val = '\0' );

        protected:

            virtual std::ostream&
                        vwrite( char_type*      str,
                                std::streamsize count );

            bool        _byteswap;
            bool        _validate;

            static char _null;
    };

}
