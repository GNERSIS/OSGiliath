/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Kerning type enumeration for text rendering.
 * Selects between no kerning, freetype kerning, and font-native kerning.
 */
#pragma once

namespace osgText
{

    typedef std::pair<unsigned int, unsigned int> FontResolution;

    enum KerningType
    {
        KERNING_DEFAULT,     // default locked to integer kerning values
        KERNING_UNFITTED,    // use floating point value for kerning
        KERNING_NONE         // no kerning
    };

}
