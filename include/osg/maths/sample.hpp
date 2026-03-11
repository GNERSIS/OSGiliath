/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2022 Robert Osfield */

#pragma once

#include <osg/maths/clamp.hpp>
#include <osg/maths/color.hpp>

namespace osg
{

    /// Filter mode for texture sampling
    enum class FilterMode
    {
        NEAREST,
        LINEAR,
    };

    /// Address mode for texture sampling
    enum class AddressMode
    {
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER,
    };

    /// clamp coord to range using the given address mode, return true if succeeds.
    inline bool
    clamp( AddressMode mode,
           float&      coord )
    {
        switch( mode )
        {
            case AddressMode::REPEAT :
                coord = repeat( coord );
                return true;
            case AddressMode::MIRRORED_REPEAT :
                coord = mirror_repeat( coord );
                return true;
            case AddressMode::CLAMP_TO_EDGE :
                coord = clamp_to_edge( coord );
                return true;
            case AddressMode::CLAMP_TO_BORDER :
                if( coord < 0.0F )
                {
                    return false;
                }
                if( coord > 1.0F )
                {
                    return false;
                }
                return true;
            default :
                // not supported, fallback to clamp_to_edge
                coord = clamp_to_edge( coord );
                break;
        }
        return true;
    }

}    // namespace osg
