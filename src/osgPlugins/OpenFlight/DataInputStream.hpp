/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DataInputStream, derived from istream.
 * Provides: readInt8, readUInt8, readInt16, readUInt16, readInt32, readUInt32.
 */
//
// OpenFlight� loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#pragma once

#include "Types.hpp"

#include <istream>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <string>

namespace flt
{

    class Record;

    class DataInputStream : public std::istream
    {
        public:

            explicit DataInputStream( std::streambuf* sb );

            int8
            readInt8( int8 def = 0 );
            uint8
            readUInt8( uint8 def = 0 );
            int16
            readInt16( int16 def = 0 );
            uint16
            readUInt16( uint16 def = 0 );
            int32
            readInt32( int32 def = 0 );
            uint32
            readUInt32( uint32 def = 0 );
            float32
            readFloat32( float32 def = 0 );
            float64
            readFloat64( float64 def = 0 );
            void
            readCharArray( char* data,
                           int   size );
            std::string
            readString( int size );
            osg::vec4
            readColor32();
            osg::vec2
            readVec2f();
            osg::vec3
            readVec3f();
            osg::vec4
            readVec4f();
            osg::dvec3
            readVec3d();

            std::istream&
            forward( std::istream::off_type off );

            int16
            peekInt16();

        protected:

            bool _byteswap;
    };

}    // end namespace
