/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * TexturePaletteManager, derived from Referenced.
 * Provides: add, write.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#pragma once

#include "ExportOptions.hpp"

#include <fstream>
#include <map>

namespace osg
{

    class Texture2D;

}

namespace flt
{

    class DataOutputStream;
    class FltExportVisitor;

    class TexturePaletteManager : public osg::Referenced
    {
        public:

            TexturePaletteManager( const FltExportVisitor& nv,
                                   const ExportOptions&    fltOpt );

            int
            add( int                   unit,
                 const osg::Texture2D* texture );

            void
            write( DataOutputStream& dos ) const;

        protected:

            TexturePaletteManager&
            operator=( const TexturePaletteManager& )
            {
                return *this;
            }

            int                                          _currIndex;

            typedef std::map<const osg::Texture2D*, int> TextureIndexMap;
            TextureIndexMap                              _indexMap;

            const FltExportVisitor&                      _nv;

            const ExportOptions&                         _fltOpt;
    };

}
