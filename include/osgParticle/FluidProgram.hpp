/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Combined fluid simulation program. Applies wind vector and
 * fluid friction to all particles in the system.
 */
#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgParticle/Export.hpp>
#include <osgParticle/Program.hpp>

namespace osgParticle
{

    /**    A program class for performing operations on particles using a sequence of
       <I>operators</I>. To use a <CODE>FluidProgram</CODE> you have to create some
       <CODE>Operator</CODE> objects and add them to the program. All operators will be
       applied to each particle in the same order they've been added to the program.
    */
    class OSGPARTICLE_EXPORT FluidProgram : public osg::Inherit<Program, FluidProgram>
    {
        public:

            FluidProgram();
            FluidProgram( const FluidProgram& copy,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               FluidProgram )

            /// Set the viscosity of the fluid.
            inline void
            setFluidViscosity( float v )
            {
                _viscosity            = v;
                _viscosityCoefficient = static_cast<float>( 6.0 * osg::PI ) * _viscosity;
            }

            /// Get the viscosity of the fluid.
            inline float
            getFluidViscosity() const
            {
                return _viscosity;
            }

            /// Set the density of the fluid.
            inline void
            setFluidDensity( float d )
            {
                _density            = d;
                _densityCoefficient = static_cast<float>( 0.2 * osg::PI ) * _density;
            }

            /// Get the density of the fluid.
            inline float
            getFluidDensity() const
            {
                return _density;
            }

            /// Set the wind vector.
            inline void
            setWind( const osg::vec3& wind )
            {
                _wind = wind;
            }

            /// Get the wind vector.
            inline const osg::vec3&
            getWind() const
            {
                return _wind;
            }

            /// Set the acceleration vector.
            inline void
            setAcceleration( const osg::vec3& v )
            {
                _acceleration = v;
            }

            /// Get the acceleration vector.
            inline const osg::vec3&
            getAcceleration() const
            {
                return _acceleration;
            }

            /** Set the acceleration vector to the gravity on earth (0, 0, -9.81).
                The acceleration will be multiplied by the <CODE>scale</CODE> parameter.
            */
            inline void
            setToGravity( float scale = 1.0F )
            {
                _acceleration.set( 0, 0, -9.81F * scale );
            }

            /// Set the fluid parameters as for air (20�C temperature).
            inline void
            setFluidToAir()
            {
                setToGravity( 1.0F );
                setFluidDensity( 1.2929F );
                setFluidViscosity( 1.8E-5F );
            }

            /// Set the fluid parameters as for pure water (20�C temperature).
            inline void
            setFluidToWater()
            {
                setToGravity( 1.0F );
                setFluidDensity( 1.0F );
                setFluidViscosity( 1.002E-3F );
            }

        protected:

            virtual ~FluidProgram()
            {
            }

            FluidProgram&
            operator=( const FluidProgram& )
            {
                return *this;
            }

            virtual void
                      execute( double dt );

            osg::vec3 _acceleration;
            float     _viscosity;
            float     _density;
            osg::vec3 _wind;

            float     _viscosityCoefficient;
            float     _densityCoefficient;
    };

}
