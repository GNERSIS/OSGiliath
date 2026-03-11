/* -*-c++-*-
 */
//****************************************************************************//
// loader.cpp                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

/*****************************************************************************/
/* Loads a core compressed keyframe instance.
 *
 * This function loads a core compressed keyframe instance from a data source.
 *
 * @param dataSrc The data source to load the core compressed keyframe instance from.
 *
 * @return One of the following values:
 *         \li a pointer to the core keyframe
 *         \li \b 0 if an error happened
 * Authors
 *         Igor Kravchenko
 *         Cedric Pinson <mornifle@plopbyte.net>
 *****************************************************************************/

#pragma once

#include <float.h>
#include <osg/maths/Math.hpp>
#include <osg/maths/vec3.hpp>
#include <vector>

namespace osgAnimation
{

    struct Vec3Packed
    {
            typedef unsigned int uint32_t;
            uint32_t             m32bits;

            Vec3Packed( uint32_t val ) :
                m32bits( val )
            {
            }

            Vec3Packed() :
                m32bits( 0 )
            {
            }

            void
            uncompress( const osg::vec3& scale,
                        const osg::vec3& min,
                        osg::vec3&       result ) const
            {
                uint32_t pt[3];
                pt[0]     = m32bits & 0X7'FF;
                pt[1]     = ( m32bits >> 11 ) & 0X7'FF;
                pt[2]     = m32bits >> 22;
                result[0] = scale[0] * static_cast<float>( pt[0] ) + min[0];
                result[1] = scale[1] * static_cast<float>( pt[1] ) + min[1];
                result[2] = scale[2] * static_cast<float>( pt[2] ) + min[2];
            }

            void
            compress( const osg::vec3& src,
                      const osg::vec3& min,
                      const osg::vec3& scaleInv )
            {
                uint32_t srci[3];
                srci[0] =
                    std::min( static_cast<uint32_t>( ( src[0] - min[0] ) * scaleInv[0] ),
                              uint32_t( 2'047 ) );
                srci[1] =
                    std::min( static_cast<uint32_t>( ( src[1] - min[1] ) * scaleInv[1] ),
                              uint32_t( 2'047 ) );
                srci[2] =
                    std::min( static_cast<uint32_t>( ( src[2] - min[2] ) * scaleInv[2] ),
                              uint32_t( 1'023 ) );
                m32bits = srci[0] + ( srci[1] << 11 ) + ( srci[2] << 22 );
            }
    };

    struct Vec3ArrayPacked
    {
            std::vector<Vec3Packed> mVecCompressed;
            osg::vec3               mMin;
            osg::vec3               mScale;
            osg::vec3               mScaleInv;

            void
            analyze( const std::vector<osg::vec3>& src )
            {
                // analyze the keys
                mMin.set( FLT_MAX, FLT_MAX, FLT_MAX );
                osg::vec3 maxp( -FLT_MAX, -FLT_MAX, -FLT_MAX );
                int       nb = ( int )src.size();
                for( std::size_t i = 0; i < static_cast<std::size_t>( nb ); i++ )
                {
                    const osg::vec3& pos = src[i];
                    for( std::size_t j = 0; j < 3; j++ )
                    {
                        maxp[j] = std::max( pos[j], maxp[j] );
                        mMin[j] = std::min( pos[j], mMin[j] );
                    }
                }

                osg::vec3 diff = maxp - mMin;
                mScaleInv.set( 0, 0, 0 );
                if( diff[0] != 0 )
                {
                    mScaleInv[0] = 2047.0F / diff[0];
                }

                if( diff[1] != 0 )
                {
                    mScaleInv[1] = 2047.0F / diff[1];
                }

                if( diff[2] != 0 )
                {
                    mScaleInv[2] = 1023.0F / diff[2];
                }

                mScale[0] = diff[0] / 2'047;
                mScale[1] = diff[1] / 2'047;
                mScale[2] = diff[2] / 1'023;
            }

            void
            compress( const std::vector<osg::vec3>& src )
            {
                mVecCompressed.resize( src.size() );
                // save all core keyframes
                for( std::size_t i = 0; i < src.size(); i++ )
                {
                    mVecCompressed[i].compress( src[i], mMin, mScaleInv );
                }
            }
    };

}
