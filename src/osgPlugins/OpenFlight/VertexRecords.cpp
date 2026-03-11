/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * VertexC, derived from Record.
 * Provides: META_Record, readRecord, REGISTER_FLTRECORD, VertexCN, META_Record,
 * VertexCT.
 */
//
// OpenFlight� loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "Document.hpp"
#include "Record.hpp"
#include "RecordInputStream.hpp"
#include "Registry.hpp"

#include <osg/maths/compat.hpp>

namespace flt
{

    // Color from ColorPool.
    osg::vec4
    getColorFromPool( int              index,
                      const ColorPool* colorPool )
    {
        osg::vec4 color( 1, 1, 1, 1 );
        if( colorPool )
        {
            color = colorPool->getColor( index );
        }
        return color;
    }

    // Vertex flags
    enum Flags
    {
        START_HARD_EDGE = ( 0X80'00 >> 0 ),
        NORMAL_FROZEN   = ( 0X80'00 >> 1 ),
        NO_COLOR        = ( 0X80'00 >> 2 ),
        PACKED_COLOR    = ( 0X80'00 >> 3 ),
    };

    class VertexC : public Record
    {
        public:

            VertexC()
            {
            }

            META_Record( VertexC ) virtual ~VertexC()
            {
            }

            virtual void
            readRecord( RecordInputStream& in,
                        Document&          document )
            {
                /*int colorNameIndex =*/in.readInt16();
                uint16     flags       = in.readUInt16();
                osg::dvec3 coord       = in.readVec3d();
                osg::vec4  packedColor = in.readColor32();
                int        colorIndex  = in.readInt32( -1 );

                Vertex     vertex;
                vertex.setCoord( osg::vec3( coord * document.unitScale() ) );

                // color
                if( flags & PACKED_COLOR )
                {
                    vertex.setColor( packedColor );    // Packed color
                }
                else if( ( ( flags & NO_COLOR ) == 0 ) && ( colorIndex >= 0 ) )
                {
                    // Only use the color index if the NO_COLOR bit is _not_ set
                    // and the index isn't negative.
                    vertex.setColor(
                        getColorFromPool( colorIndex, document.getColorPool() )
                    );    // Color from pool
                }

                if( _parent.valid() )
                {
                    _parent->addVertex( vertex );
                }
            }
    };

    REGISTER_FLTRECORD( VertexC,
                        VERTEX_C_OP )

    class VertexCN : public Record
    {
        public:

            VertexCN()
            {
            }

        META_Record( VertexCN ) protected : virtual ~VertexCN()
            {
            }

            virtual void
            readRecord( RecordInputStream& in,
                        Document&          document )
            {
                /*int colorNameIndex =*/in.readInt16();
                uint16     flags       = in.readUInt16();
                osg::dvec3 coord       = in.readVec3d();
                osg::vec3  normal      = in.readVec3f();
                osg::vec4  packedColor = in.readColor32();
                int        colorIndex  = in.readInt32( -1 );

                Vertex     vertex;
                vertex.setCoord( osg::vec3( coord * document.unitScale() ) );
                vertex.setNormal( normal );

                // color
                if( flags & PACKED_COLOR )
                {
                    vertex.setColor( packedColor );    // Packed color
                }
                else if( ( ( flags & NO_COLOR ) == 0 ) && ( colorIndex >= 0 ) )
                {
                    // Only use the color index if the NO_COLOR bit is _not_ set
                    // and the index isn't negative.
                    vertex.setColor(
                        getColorFromPool( colorIndex, document.getColorPool() )
                    );    // Color from pool
                }

                if( _parent.valid() )
                {
                    _parent->addVertex( vertex );
                }
            }
    };

    REGISTER_FLTRECORD( VertexCN,
                        VERTEX_CN_OP )

    class VertexCT : public Record
    {
        public:

            VertexCT()
            {
            }

        META_Record( VertexCT ) protected : virtual ~VertexCT()
            {
            }

            virtual void
            readRecord( RecordInputStream& in,
                        Document&          document )
            {
                /*int colorNameIndex =*/in.readInt16();
                uint16     flags       = in.readUInt16();
                osg::dvec3 coord       = in.readVec3d();
                osg::vec2  uv          = in.readVec2f();
                osg::vec4  packedColor = in.readColor32();
                int        colorIndex  = in.readInt32( -1 );

                Vertex     vertex;
                vertex.setCoord( osg::vec3( coord * document.unitScale() ) );
                vertex.setUV( 0, uv );

                // color
                if( flags & PACKED_COLOR )
                {
                    vertex.setColor( packedColor );    // Packed color
                }
                else if( ( ( flags & NO_COLOR ) == 0 ) && ( colorIndex >= 0 ) )
                {
                    // Only use the color index if the NO_COLOR bit is _not_ set
                    // and the index isn't negative.
                    vertex.setColor(
                        getColorFromPool( colorIndex, document.getColorPool() )
                    );    // Color from pool
                }

                if( _parent.valid() )
                {
                    _parent->addVertex( vertex );
                }
            }
    };

    REGISTER_FLTRECORD( VertexCT,
                        VERTEX_CT_OP )

    class VertexCNT : public Record
    {
        public:

            VertexCNT()
            {
            }

        META_Record( VertexCNT ) protected : virtual ~VertexCNT()
            {
            }

            virtual void
            readRecord( RecordInputStream& in,
                        Document&          document )
            {
                /*int colorNameIndex =*/in.readInt16();
                uint16     flags       = in.readUInt16();
                osg::dvec3 coord       = in.readVec3d();
                osg::vec3  normal      = in.readVec3f();
                osg::vec2  uv          = in.readVec2f();
                osg::vec4  packedColor = in.readColor32();
                int        colorIndex  = in.readInt32( -1 );

                Vertex     vertex;
                vertex.setCoord( osg::vec3( coord * document.unitScale() ) );
                vertex.setNormal( normal );
                vertex.setUV( 0, uv );

                if( !osg::valid( coord ) )
                {
                    OSG_NOTICE
                        << "Warning: data error detected in VertexCNT::readRecord coord="
                        << coord.x << " " << coord.y << " " << coord.z << std::endl;
                }

                if( !osg::valid( normal ) )
                {
                    OSG_NOTICE << "Warning: data error detected in "
                                  "VertexCNT::readRecord normal="
                               << normal.x << " " << normal.y << " " << normal.z
                               << std::endl;
                }

                if( !osg::valid( uv ) )
                {
                    OSG_NOTICE
                        << "Warning: data error detected in VertexCNT::readRecord uv="
                        << uv.x << " " << uv.y << std::endl;
                }

                // color
                if( flags & PACKED_COLOR )
                {
                    vertex.setColor( packedColor );    // Packed color
                }
                else if( ( ( flags & NO_COLOR ) == 0 ) && ( colorIndex >= 0 ) )
                {
                    // Only use the color index if the NO_COLOR bit is _not_ set
                    // and the index isn't negative.
                    vertex.setColor(
                        getColorFromPool( colorIndex, document.getColorPool() )
                    );    // Color from pool
                }

                if( _parent.valid() )
                {
                    _parent->addVertex( vertex );
                }
            }
    };

    REGISTER_FLTRECORD( VertexCNT,
                        VERTEX_CNT_OP )

    /** Absolute Vertex -
     * version < 13
     */
    class AbsoluteVertex : public Record
    {
        public:

            AbsoluteVertex()
            {
            }

        META_Record( AbsoluteVertex ) protected : virtual ~AbsoluteVertex()
            {
            }

            virtual void
            readRecord( RecordInputStream& in,
                        Document&          document )
            {
                int32  x = in.readInt32();
                int32  y = in.readInt32();
                int32  z = in.readInt32();

                Vertex vertex;

                // coord
                vertex.setCoord( osg::vec3( x, y, z ) * document.unitScale() );

                // optional texture coordinates
                if( in.getRecordBodySize() > ( 4 + 4 + 4 ) )
                {
                    osg::vec2 uv = in.readVec2f();
                    vertex.setUV( 0, uv );
                }

                if( _parent.valid() )
                {
                    _parent->addVertex( vertex );
                }
            }
    };

    REGISTER_FLTRECORD( AbsoluteVertex,
                        OLD_ABSOLUTE_VERTEX_OP )

    /** Shaded Vertex
     * version < 13
     */
    class ShadedVertex : public Record
    {
        public:

            ShadedVertex()
            {
            }

        META_Record( ShadedVertex ) protected : virtual ~ShadedVertex()
            {
            }

            virtual void
            readRecord( RecordInputStream& in,
                        Document&          document )
            {
                int32 x = in.readInt32();
                int32 y = in.readInt32();
                int32 z = in.readInt32();
                /*uint8 edgeFlag =*/in.readUInt8();
                /*uint8 shadingFlag =*/in.readUInt8();
                int    colorIndex = ( int )in.readInt16();

                Vertex vertex;

                // coord
                vertex.setCoord( osg::vec3( x, y, z ) * document.unitScale() );

                // color
                if( colorIndex >= 0 )
                {
                    vertex.setColor(
                        getColorFromPool( colorIndex, document.getColorPool() )
                    );    // Color from pool
                }

                // optional texture coordinates
                if( in.getRecordBodySize() > ( 4 + 4 + 4 + 1 + 1 + 2 ) )
                {
                    osg::vec2 uv = in.readVec2f();
                    vertex.setUV( 0, uv );
                }

                if( _parent.valid() )
                {
                    _parent->addVertex( vertex );
                }
            }
    };

    REGISTER_FLTRECORD( ShadedVertex,
                        OLD_SHADED_VERTEX_OP )

    /** Normal Vertex
     * version < 13
     */
    class NormalVertex : public Record
    {
        public:

            NormalVertex()
            {
            }

        META_Record( NormalVertex ) protected : virtual ~NormalVertex()
            {
            }

            virtual void
            readRecord( RecordInputStream& in,
                        Document&          document )
            {
                int32 x = in.readInt32();
                int32 y = in.readInt32();
                int32 z = in.readInt32();
                /*uint8 edgeFlag =*/in.readUInt8();
                /*uint8 shadingFlag =*/in.readUInt8();
                int       colorIndex = ( int )in.readInt16();
                osg::vec3 normal( in.readVec3d() );

                Vertex    vertex;
                vertex.setCoord( osg::vec3( x, y, z ) * document.unitScale() );
                vertex.setNormal( normal / ( float )( 1L << 30 ) );

                // color
                if( colorIndex >= 0 )
                {
                    vertex.setColor(
                        getColorFromPool( colorIndex, document.getColorPool() )
                    );    // Color from pool
                }

                // optional texture coordinates
                if( in.getRecordBodySize() > ( 4 + 4 + 4 + 1 + 1 + 2 + 3 * 8 ) )
                {
                    osg::vec2 uv = in.readVec2f();
                    vertex.setUV( 0, uv );
                }

                if( _parent.valid() )
                {
                    _parent->addVertex( vertex );
                }
            }
    };

    REGISTER_FLTRECORD( NormalVertex,
                        OLD_NORMAL_VERTEX_OP )

}    // end namespace
