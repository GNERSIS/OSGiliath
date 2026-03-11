/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: _currIndex, here, warning.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#include "DataOutputStream.hpp"
#include "MaterialPaletteManager.hpp"
#include "Opcodes.hpp"

#include <cassert>
#include <osg/core/Notify.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/state/Material.hpp>
#include <sstream>

namespace flt
{

    MaterialPaletteManager::MaterialPaletteManager( ExportOptions& fltOpt ) :
        _currIndex( -1 ),
        _fltOpt( fltOpt )
    {
        // TODO: Pay attention to the version here(?)
    }

    int
    MaterialPaletteManager::add( const osg::Material* material )
    {
        int index = -1;
        if( material == NULL )
        {
            return -1;
        }

        // If this material has already been cached, set 'index' to the cached value
        MaterialPalette::const_iterator it = _materialPalette.find( material );
        if( it != _materialPalette.end() )
        {
            index = it->second.Index;
        }

        // New material? Add it to the cache...
        else
        {
            index = ++_currIndex;
            _materialPalette.insert(
                std::make_pair( material, MaterialRecord( material, index ) )
            );
        }

        return index;
    }

    void
    MaterialPaletteManager::write( DataOutputStream& dos ) const
    {
        using osg::vec4;

        MaterialPalette::const_iterator it = _materialPalette.begin();
        for( ; it != _materialPalette.end(); ++it )
        {
            MaterialRecord m         = it->second;
            const vec4&    ambient   = m.Material->getAmbient( osg::Material::FRONT );
            const vec4&    diffuse   = m.Material->getDiffuse( osg::Material::FRONT );
            const vec4&    specular  = m.Material->getSpecular( osg::Material::FRONT );
            const vec4&    emissive  = m.Material->getEmission( osg::Material::FRONT );
            float          shininess = m.Material->getShininess( osg::Material::FRONT );

            dos.writeInt16( ( int16 )MATERIAL_PALETTE_OP );
            dos.writeInt16( 84 );    // Length - FIXME: hard-code/FLT version?
            dos.writeInt32( m.Index );
            dos.writeString( m.Material->getName(), 12 );
            dos.writeInt32( 0 );     // Flags
            dos.writeFloat32( ambient.r );
            dos.writeFloat32( ambient.g );
            dos.writeFloat32( ambient.b );
            dos.writeFloat32( diffuse.r );
            dos.writeFloat32( diffuse.g );
            dos.writeFloat32( diffuse.b );
            dos.writeFloat32( specular.r );
            dos.writeFloat32( specular.g );
            dos.writeFloat32( specular.b );
            dos.writeFloat32( emissive.r );
            dos.writeFloat32( emissive.g );
            dos.writeFloat32( emissive.b );
            dos.writeFloat32( shininess );
            dos.writeFloat32( diffuse.a );    // alpha
            dos.writeFloat32( 1.0F );         // 'Reserved' - unused

            if( m.Material->getAmbientFrontAndBack() ==
                false ||
                m.Material->getDiffuseFrontAndBack() ==
                false ||
                m.Material->getSpecularFrontAndBack() ==
                false ||
                m.Material->getEmissionFrontAndBack() ==
                false ||
                m.Material->getShininessFrontAndBack() == false )

            {
                std::string warning( "fltexp: No support for different front and back "
                                     "material properties." );
                OSG_WARN << warning << std::endl;
                _fltOpt.getWriteResult().warn( warning );
            }
        }
    }

}    // End namespace fltexp
