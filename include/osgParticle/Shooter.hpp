/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract shooter defining initial particle velocities.
 * Subclasses provide directional, radial, or random emission.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Object.hpp>

namespace osgParticle
{

    class Particle;

    /**     An abstract base class used by ModularEmitter to "shoot" the particles after
       they have been placed. Descendants of this class must override the
       <CODE>shoot()</CODE> method.
    */
    class Shooter : public osg::Object
    {
        public:

            inline Shooter();
            inline Shooter( const Shooter&     copy,
                            const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            virtual const char*
            libraryName() const
            {
                return "osgParticle";
            }

            virtual const char*
            className() const
            {
                return "Shooter";
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const Shooter*>( obj ) != 0;
            }

            /**     Shoot a particle. Must be overridden by descendants.
                This method should only set the velocity vector of particle
               <CODE>P</CODE>, leaving other attributes unchanged.
            */
            virtual void
            shoot( Particle* P ) const = 0;

        protected:

            virtual ~Shooter()
            {
            }

            Shooter&
            operator=( const Shooter& )
            {
                return *this;
            }
    };

    // INLINE FUNCTIONS

    inline Shooter::Shooter() :
        osg::Object()
    {
    }

    inline Shooter::Shooter( const Shooter&     copy,
                             const osg::CopyOp& copyop ) :
        osg::Object( copy,
                     copyop )
    {
    }

}
