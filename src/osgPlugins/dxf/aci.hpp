/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * aci — osgPlugins library implementation.
 */
#pragma once

// lookup table for autocad color index
struct aci
{
        // some color positions
        enum
        {
            BLACK,
            RED,
            YELLOW,
            GREEN,
            CYAN,
            BLUE,
            MAGENTA,
            WHITE,
            USER_2,
            USER_3,
            BYLAYER = 256,
            MIN     = 1,
            MAX     = 255,
        };

        static double table[256 * 3];
};
