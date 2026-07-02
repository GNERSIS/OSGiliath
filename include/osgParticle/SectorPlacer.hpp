/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Places particles randomly within an annular sector.
 * Configurable center, radii range, and angle range.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/vec3.hpp>
#include <osgParticle/CenteredPlacer.hpp>
#include <osgParticle/Particle.hpp>
#include <osgParticle/range.hpp>

namespace osgParticle
{

    /**     A sector-shaped particle placer.
        This placer sets the initial position of incoming particle by choosing a random
       position within a circular sector; this sector is defined by three parameters: a
       <I>center point</I>, which is inherited directly from
       <CODE>osgParticle::CenteredPlacer</CODE>, a range of values for <I>radius</I>, and
       a range of values for the <I>central angle</I> (sometimes called    <B>phi</B>).
    */
    class SectorPlacer : public osg::Inherit<CenteredPlacer, SectorPlacer>
    {
        public:

            inline SectorPlacer();
            inline SectorPlacer( const SectorPlacer& copy,
                                 const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            /// Get the range of possible values for radius.
            inline const rangef&
            getRadiusRange() const;

            /// Set the range of possible values for radius.
            inline void
            setRadiusRange( const rangef& r );

            /// Set the range of possible values for radius.
            inline void
            setRadiusRange( float r1,
                            float r2 );

            /// Get the range of possible values for the central angle.
            inline const rangef&
            getPhiRange() const;

            /// Set the range of possible values for the central angle.
            inline void
            setPhiRange( const rangef& r );

            /// Set the range of possible values for the central angle.
            inline void
            setPhiRange( float r1,
                         float r2 );

            OSG_REGISTER_TYPE( osgParticle,
                               SectorPlacer )

            /// Place a particle. Do not call it manually.
            inline void
            place( Particle* P ) const;

            /// return the area of the sector
            inline float
            volume() const;

            /// return the control position
            inline osg::vec3
            getControlPosition() const;

        protected:

            virtual ~SectorPlacer()
            {
            }

            SectorPlacer&
            operator=( const SectorPlacer& )
            {
                return *this;
            }

        private:

            rangef _rad_range;
            rangef _phi_range;
    };

    // INLINE FUNCTIONS

    inline SectorPlacer::SectorPlacer() :
        _rad_range( 0,
                    1 ),
        _phi_range( 0,
                    static_cast<float>( osg::PI * 2.0 ) )
    {
    }

    inline SectorPlacer::SectorPlacer( const SectorPlacer& copy,
                                       const osg::CopyOp&  copyop ) :
        Inherit( copy,
                 copyop ),
        _rad_range( copy._rad_range ),
        _phi_range( copy._phi_range )
    {
    }

    inline const rangef&
    SectorPlacer::getRadiusRange() const
    {
        return _rad_range;
    }

    inline const rangef&
    SectorPlacer::getPhiRange() const
    {
        return _phi_range;
    }

    inline void
    SectorPlacer::setRadiusRange( const rangef& r )
    {
        _rad_range = r;
    }

    inline void
    SectorPlacer::setRadiusRange( float r1,
                                  float r2 )
    {
        _rad_range.minimum = r1;
        _rad_range.maximum = r2;
    }

    inline void
    SectorPlacer::setPhiRange( const rangef& r )
    {
        _phi_range = r;
    }

    inline void
    SectorPlacer::setPhiRange( float r1,
                               float r2 )
    {
        _phi_range.minimum = r1;
        _phi_range.maximum = r2;
    }

    inline void
    SectorPlacer::place( Particle* P ) const
    {
        float     rad = _rad_range.get_random_sqrtf();
        float     phi = _phi_range.get_random();

        osg::vec3 pos( getCenter().x + rad * cosf( phi ),
                       getCenter().y + rad * sinf( phi ),
                       getCenter().z );

        P->setPosition( pos );
    }

    inline float
    SectorPlacer::volume() const
    {
        return 0.5F *
               ( _phi_range.maximum - _phi_range.minimum ) *
               ( _rad_range.maximum *
                 _rad_range.maximum -
                 _rad_range.minimum *
                 _rad_range.minimum );
    }

    inline osg::vec3
    SectorPlacer::getControlPosition() const
    {
        return getCenter();
    }

}
