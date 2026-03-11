/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * LightSourcePaletteManager, derived from Referenced.
 * Provides: add, write.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#pragma once

#include "ExportOptions.hpp"

#include <set>

namespace osg
{

    class Light;

}

namespace flt
{

    class DataOutputStream;

    class LightSourcePaletteManager : public osg::Referenced
    {
        public:

            LightSourcePaletteManager();

            // Add a light to the palette and auto-assign it the next available index
            int
            add( const osg::Light* light );

            // Write the light palette records out to a DataOutputStream
            void
            write( DataOutputStream& dos ) const;

        private:

            int _currIndex;

            // Helper struct to hold light palette records
            struct LightRecord
            {
                    LightRecord( const osg::Light* light,
                                 int               i ) :
                        Light( light ),
                        Index( i )
                    {
                    }

                    const osg::Light* Light;
                    int               Index;
            };

            // Map to allow lookups by Light pointer
            typedef std::map<const osg::Light*, LightRecord> LightPalette;
            LightPalette                                     _lightPalette;

        protected:

            LightSourcePaletteManager&
            operator=( const LightSourcePaletteManager& )
            {
                return *this;
            }
    };

}    // End namespace fltexp
