/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: _currIndex, here, flag.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#include "DataOutputStream.hpp"
#include "LightSourcePaletteManager.hpp"
#include "Opcodes.hpp"

#include <cassert>
#include <osg/core/Notify.hpp>
#include <osg/lighting/Light.hpp>
#include <osg/maths/vec4.hpp>
#include <sstream>
#include <stdio.h>

namespace flt
{

    LightSourcePaletteManager::LightSourcePaletteManager() :
        _currIndex( -1 )
    {
        // TODO: Pay attention to the version here(?)
    }

    int
    LightSourcePaletteManager::add( const osg::Light* light )
    {
        int index = -1;
        if( light == NULL )
        {
            return -1;
        }

        // If this light has already been cached, set 'index' to the cached value
        LightPalette::const_iterator it = _lightPalette.find( light );
        if( it != _lightPalette.end() )
        {
            index = it->second.Index;
        }

        // New light? Add it to the cache...
        else
        {
            index = ++_currIndex;
            _lightPalette.insert( std::make_pair( light, LightRecord( light, index ) ) );
        }

        return index;
    }

    void
    LightSourcePaletteManager::write( DataOutputStream& dos ) const
    {
        using osg::vec4;

        static const int             INFINITE_LIGHT = 0;
        static const int             LOCAL_LIGHT    = 1;
        static const int             SPOT_LIGHT     = 2;

        LightPalette::const_iterator it             = _lightPalette.begin();
        for( ; it != _lightPalette.end(); ++it )
        {
            LightRecord m = it->second;

            static char lightName[64];
            sprintf( lightName, "Light%02d", m.Light->getLightNum() );

            int         lightType = INFINITE_LIGHT;
            const vec4& lightPos  = m.Light->getPosition();
            if( lightPos.w != 0 )
            {
                if( m.Light->getSpotCutoff() < 180 )
                {
                    lightType = SPOT_LIGHT;
                }
                else
                {
                    lightType = LOCAL_LIGHT;
                }
            }

            dos.writeInt16( ( int16 )LIGHT_SOURCE_PALETTE_OP );
            dos.writeInt16( 240 );
            dos.writeInt32( m.Index );
            dos.writeFill( 2 * 4, '\0' );    // Reserved
            dos.writeString( lightName, 20 );
            dos.writeFill( 4, '\0' );        // Reserved

            dos.writeVec4f( m.Light->getAmbient() );
            dos.writeVec4f( m.Light->getDiffuse() );
            dos.writeVec4f( m.Light->getSpecular() );
            dos.writeInt32( lightType );
            dos.writeFill( 4 * 10, '\0' );    // Reserved
            dos.writeFloat32( m.Light->getSpotExponent() );
            dos.writeFloat32( m.Light->getSpotCutoff() );
            dos.writeFloat32( 0 );            // Yaw (N/A)
            dos.writeFloat32( 0 );            // Pitch (N/A)
            dos.writeFloat32( m.Light->getConstantAttenuation() );
            dos.writeFloat32( m.Light->getLinearAttenuation() );
            dos.writeFloat32( m.Light->getQuadraticAttenuation() );
            dos.writeInt32( 0 );              // Modeling flag (N/A)
            dos.writeFill( 4 * 19, '\0' );    // Reserved
        }
    }

}    // End namespace fltexp
