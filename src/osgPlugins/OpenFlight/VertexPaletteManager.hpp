/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * VertexPaletteManager, derived from Referenced.
 * Provides: add, add, write, asVec2Array, asVec3Array, asVec3dArray.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#pragma once

#include "DataOutputStream.hpp"
#include "ExportOptions.hpp"

#include <map>
#include <osg/geometry/Array.hpp>
#include <osgDB/io/fstream.hpp>

namespace osg
{

    class Geometry;

}

namespace flt
{

    /*!
       Manages writing the Vertex Palette record during export.
       Maintains a map to ensure that instanced VertexArray data is only
       written once to the palette. Writes the palette record to a temp
       file and copies it to FltExportVisitor::_dos after the scene graph
       has been completely walked.
     */
    class VertexPaletteManager : public osg::Referenced
    {
        public:

            VertexPaletteManager( const ExportOptions& fltOpt );

            void
            add( const osg::Geometry& geom );
            void
            add( const osg::Array*      key,
                 const osg::Vec3dArray* v,
                 const osg::Vec4Array*  c,
                 const osg::Vec3Array*  n,
                 const osg::Vec2Array*  t,
                 bool                   colorPerVertex,
                 bool                   normalPerVertex,
                 bool                   allowSharing = true );

            unsigned int
            byteOffset( unsigned int idx ) const;

            void
            write( DataOutputStream& dos ) const;

            /*!
               Static utility routines for handling the morass of array
               types that could be found in a Geometry object's vertex/
               normal/texcoord/color data. */
            static osg::ref_ptr<const osg::Vec2Array>
            asVec2Array( const osg::Array*  in,
                         const unsigned int n );
            static osg::ref_ptr<const osg::Vec3Array>
            asVec3Array( const osg::Array*  in,
                         const unsigned int n );
            static osg::ref_ptr<const osg::Vec3dArray>
            asVec3dArray( const osg::Array*  in,
                          const unsigned int n );
            static osg::ref_ptr<const osg::Vec4Array>
            asVec4Array( const osg::Array*  in,
                         const unsigned int n );

        protected:

            virtual ~VertexPaletteManager();

            typedef enum
            {
                VERTEX_C,
                VERTEX_CN,
                VERTEX_CNT,
                VERTEX_CT,
            } PaletteRecordType;

            static PaletteRecordType
            recordType( const osg::Array* v,
                        const osg::Array* c,
                        const osg::Array* n,
                        const osg::Array* t );
            unsigned int
            recordSize( PaletteRecordType recType );

            void
                         writeRecords( const osg::Vec3dArray* v,
                                       const osg::Vec4Array*  c,
                                       const osg::Vec3Array*  n,
                                       const osg::Vec2Array*  t,
                                       bool                   colorPerVertex,
                                       bool                   normalPerVertex );

            unsigned int _currentSizeBytes;

            struct ArrayInfo
            {
                    ArrayInfo();

                    unsigned int _byteStart;
                    unsigned int _idxSizeBytes;
                    unsigned int _idxCount;
            };

            ArrayInfo*                                     _current;
            ArrayInfo                                      _nonShared;

            typedef std::map<const osg::Array*, ArrayInfo> ArrayMap;
            ArrayMap                                       _arrayMap;

            mutable osgDB::ofstream                        _verticesStr;
            DataOutputStream*                              _vertices;
            std::string                                    _verticesTempName;

            const ExportOptions&                           _fltOpt;
    };

}
