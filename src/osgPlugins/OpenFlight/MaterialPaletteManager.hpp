/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * MaterialPaletteManager, derived from Referenced.
 * Provides: add, write.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#pragma once

#include "ExportOptions.hpp"

#include <map>

namespace osg
{

    class Material;

}

namespace flt
{

    class DataOutputStream;

    class MaterialPaletteManager : public osg::Referenced
    {
        public:

            MaterialPaletteManager( ExportOptions& fltOpt );

            // Add a material to the palette and auto-assign it the next available index
            int
            add( const osg::Material* material );

            // Write the material palette records out to a DataOutputStream
            void
            write( DataOutputStream& dos ) const;

        private:

            virtual ~MaterialPaletteManager()
            {
            }

            int _currIndex;

            // Helper struct to hold material palette records
            struct MaterialRecord
            {
                    MaterialRecord( const osg::Material* m,
                                    int                  i ) :
                        Material( m ),
                        Index( i )
                    {
                    }

                    const osg::Material* Material;
                    int                  Index;
            };

            // Map to allow lookups by Material pointer, and permit sorting by index
            typedef std::map<const osg::Material*, MaterialRecord> MaterialPalette;
            MaterialPalette                                        _materialPalette;

            ExportOptions&                                         _fltOpt;

        protected:

            MaterialPaletteManager&
            operator=( const MaterialPaletteManager& )
            {
                return *this;
            }
    };

}    // End namespace fltexp
