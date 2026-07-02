/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Places all particles at a single point.
 * Simplest placer for point-source emission.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osgParticle/CenteredPlacer.hpp>
#include <osgParticle/Particle.hpp>

namespace osgParticle
{

    /**    A point-shaped particle placer.
        This placer class uses the center point defined in its base class
       <CODE>CenteredPlacer</CODE> to place there all incoming particles.
    */
    class PointPlacer : public osg::Inherit<CenteredPlacer, PointPlacer>
    {
        public:

            inline PointPlacer();
            inline PointPlacer( const PointPlacer& copy,
                                const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               PointPlacer )

            /** Place a particle.
                This method is called automatically by <CODE>ModularEmitter</CODE> and
               should not be called manually.
            */
            inline void
            place( Particle* P ) const;

            /// return the control position
            inline osg::vec3
            getControlPosition() const;

        protected:

            virtual ~PointPlacer()
            {
            }

            PointPlacer&
            operator=( const PointPlacer& )
            {
                return *this;
            }
    };

    // INLINE FUNCTIONS

    inline PointPlacer::PointPlacer()
    {
    }

    inline PointPlacer::PointPlacer( const PointPlacer& copy,
                                     const osg::CopyOp& copyop ) :
        Inherit( copy,
                 copyop )
    {
    }

    inline void
    PointPlacer::place( Particle* P ) const
    {
        P->setPosition( getCenter() );
    }

    inline osg::vec3
    PointPlacer::getControlPosition() const
    {
        return getCenter();
    }

}
