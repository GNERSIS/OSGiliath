/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for emitters and programs. Provides reference
 * frame handling and lifetime enable/disable control.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Object.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgParticle/Export.hpp>
#include <osgParticle/ParticleSystem.hpp>

namespace osgParticle
{

    /** A common base interface for those classes which need to do something on
     * particles. Such classes are, for example, Emitter (particle generation) and
     * Program (particle animation). This class holds some properties, like a
     * <I>reference frame</I> and a reference to a ParticleSystem; descendant classes
     * should process the particles taking into account the reference frame, computing
     * the right transformations when needed.
     */
    class OSGPARTICLE_EXPORT ParticleProcessor : public osg::Node
    {
        public:

            enum ReferenceFrame
            {
                RELATIVE_RF,
                ABSOLUTE_RF
            };

            ParticleProcessor();
            ParticleProcessor( const ParticleProcessor& copy,
                               const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            virtual const char*
            libraryName() const
            {
                return "osgParticle";
            }

            virtual const char*
            className() const
            {
                return "ParticleProcessor";
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const ParticleProcessor*>( obj ) != 0;
            }

            virtual void
            accept( osg::NodeVisitor& nv )
            {
                if( nv.validNodeMask( *this ) )
                {
                    nv.pushOntoNodePath( this );
                    nv.apply( *this );
                    nv.popFromNodePath();
                }
            }

            /// Get the reference frame.
            inline ReferenceFrame
            getReferenceFrame() const;

            /// Set the reference frame.
            inline void
            setReferenceFrame( ReferenceFrame rf );

            /// Get whether this processor is enabled or not.
            bool
            getEnabled() const
            {
                return _enabled;
            }

            inline bool
            isEnabled() const;

            /// Set whether this processor is enabled or not.
            inline void
            setEnabled( bool v );

            /// Get a pointer to the destination particle system.
            inline ParticleSystem*
            getParticleSystem();

            /// Get a const pointer to the destination particle system.
            inline const ParticleSystem*
            getParticleSystem() const;

            /// Set the destination particle system.
            virtual void
            setParticleSystem( ParticleSystem* ps );

            /// Set the endless flag of this processor.
            inline void
            setEndless( bool type );

            /// Check whether this processor is endless.
            bool
            getEndless() const
            {
                return _endless;
            }

            inline bool
            isEndless() const;

            /// Set the lifetime of this processor.
            inline void
            setLifeTime( double t );

            /// Get the lifetime of this processor.
            inline double
            getLifeTime() const;

            /// Set the start time of this processor.
            inline void
            setStartTime( double t );

            /// Get the start time of this processor.
            inline double
            getStartTime() const;

            /// Set the current time of this processor.
            inline void
            setCurrentTime( double t );

            /// Get the current time of this processor.
            inline double
            getCurrentTime() const;

            /// Set the reset time of this processor. A value of 0 disables reset.
            inline void
            setResetTime( double t );

            /// Get the reset time of this processor.
            inline double
            getResetTime() const;

            /**
              Check whether the processor is alive with respect to start time and
              life duration. Note that this method may return true even if the
              processor has been disabled by calling setEnabled(false). To test
              whether the processor is actually processing particles or not, you
              should evaluate (isEnabled() && isAlive()).
             */
            inline bool
            isAlive() const;

            void
            traverse( osg::NodeVisitor& nv );

            /// Get the current local-to-world transformation matrix (valid only during
            /// cull traversal).
            inline const osg::dmat4&
            getLocalToWorldMatrix();

            /// Get the current world-to-local transformation matrix (valid only during
            /// cull traversal).
            inline const osg::dmat4&
            getWorldToLocalMatrix();

            /// Get the previous local-to-world transformation matrix (valid only during
            /// cull traversal).
            inline const osg::dmat4&
            getPreviousLocalToWorldMatrix();

            /// Get the previous world-to-local transformation matrix (valid only during
            /// cull traversal).
            inline const osg::dmat4&
            getPreviousWorldToLocalMatrix();

            /// Transform a point from local to world coordinates (valid only during cull
            /// traversal).
            inline osg::vec3
            transformLocalToWorld( const osg::vec3& P );

            /// Transform a vector from local to world coordinates, discarding
            /// translation (valid only during cull traversal).
            inline osg::vec3
            rotateLocalToWorld( const osg::vec3& P );

            /// Transform a point from world to local coordinates (valid only during cull
            /// traversal).
            inline osg::vec3
            transformWorldToLocal( const osg::vec3& P );

            /// Transform a vector from world to local coordinates, discarding
            /// translation (valid only during cull traversal).
            inline osg::vec3
            rotateWorldToLocal( const osg::vec3& P );

            virtual osg::sphere
            computeBound() const;

        protected:

            virtual ~ParticleProcessor()
            {
            }

            ParticleProcessor&
            operator=( const ParticleProcessor& )
            {
                return *this;
            }

            virtual void
                                         process( double dt ) = 0;

            ReferenceFrame               _rf;
            bool                         _enabled;
            double                       _t0;
            osg::ref_ptr<ParticleSystem> _ps;
            bool                         _first_ltw_compute;
            bool                         _need_ltw_matrix;
            bool                         _first_wtl_compute;
            bool                         _need_wtl_matrix;
            osg::dmat4                   _ltw_matrix;
            osg::dmat4                   _wtl_matrix;
            osg::dmat4                   _previous_ltw_matrix;
            osg::dmat4                   _previous_wtl_matrix;
            osg::NodeVisitor*            _current_nodevisitor;

            bool                         _endless;

            double                       _lifeTime;
            double                       _startTime;
            double                       _currentTime;
            double                       _resetTime;

            // added- 1/17/06- bgandere@nps.edu
            // a var to keep from doing multiple updates
            unsigned int                 _frameNumber;
    };

    // INLINE FUNCTIONS

    inline ParticleProcessor::ReferenceFrame
    ParticleProcessor::getReferenceFrame() const
    {
        return _rf;
    }

    inline void
    ParticleProcessor::setReferenceFrame( ReferenceFrame rf )
    {
        _rf = rf;
    }

    inline bool
    ParticleProcessor::isEnabled() const
    {
        return _enabled;
    }

    inline void
    ParticleProcessor::setEnabled( bool v )
    {
        _enabled = v;
        if( _enabled )
        {
            _currentTime = 0;
        }
    }

    inline ParticleSystem*
    ParticleProcessor::getParticleSystem()
    {
        return _ps.get();
    }

    inline const ParticleSystem*
    ParticleProcessor::getParticleSystem() const
    {
        return _ps.get();
    }

    inline void
    ParticleProcessor::setEndless( bool type )
    {
        _endless = type;
    }

    inline bool
    ParticleProcessor::isEndless() const
    {
        return _endless;
    }

    inline void
    ParticleProcessor::setLifeTime( double t )
    {
        _lifeTime = t;
    }

    inline double
    ParticleProcessor::getLifeTime() const
    {
        return _lifeTime;
    }

    inline void
    ParticleProcessor::setStartTime( double t )
    {
        _startTime = t;
    }

    inline double
    ParticleProcessor::getStartTime() const
    {
        return _startTime;
    }

    inline void
    ParticleProcessor::setCurrentTime( double t )
    {
        _currentTime = t;
    }

    inline double
    ParticleProcessor::getCurrentTime() const
    {
        return _currentTime;
    }

    inline void
    ParticleProcessor::setResetTime( double t )
    {
        _resetTime = t;
    }

    inline double
    ParticleProcessor::getResetTime() const
    {
        return _resetTime;
    }

    inline const osg::dmat4&
    ParticleProcessor::getLocalToWorldMatrix()
    {
        if( _need_ltw_matrix )
        {
            _previous_ltw_matrix = _ltw_matrix;
            _ltw_matrix =
                osg::computeLocalToWorld( _current_nodevisitor->getNodePath() );
            if( _first_ltw_compute )
            {
                _previous_ltw_matrix = _ltw_matrix;
                _first_ltw_compute   = false;
            }
            _need_ltw_matrix = false;
        }
        return _ltw_matrix;
    }

    inline const osg::dmat4&
    ParticleProcessor::getWorldToLocalMatrix()
    {
        if( _need_wtl_matrix )
        {
            _previous_wtl_matrix = _wtl_matrix;
            _wtl_matrix =
                osg::computeWorldToLocal( _current_nodevisitor->getNodePath() );
            if( _first_wtl_compute )
            {
                _previous_wtl_matrix = _wtl_matrix;
                _first_wtl_compute   = false;
            }
            _need_wtl_matrix = false;
        }
        return _wtl_matrix;
    }

    inline const osg::dmat4&
    ParticleProcessor::getPreviousLocalToWorldMatrix()
    {
        if( _need_ltw_matrix )
        {
            getLocalToWorldMatrix();
        }
        return _previous_ltw_matrix;
    }

    inline const osg::dmat4&
    ParticleProcessor::getPreviousWorldToLocalMatrix()
    {
        if( _need_wtl_matrix )
        {
            getWorldToLocalMatrix();
        }
        return _previous_wtl_matrix;
    }

    inline osg::vec3
    ParticleProcessor::transformLocalToWorld( const osg::vec3& P )
    {
        return osg::vec3( getLocalToWorldMatrix() * P );
    }

    inline osg::vec3
    ParticleProcessor::transformWorldToLocal( const osg::vec3& P )
    {
        return osg::vec3( getWorldToLocalMatrix() * P );
    }

    inline osg::vec3
    ParticleProcessor::rotateLocalToWorld( const osg::vec3& P )
    {
        return osg::vec3( getLocalToWorldMatrix() * P ) -
               osg::vec3( getLocalToWorldMatrix() * osg::vec3( 0, 0, 0 ) );
    }

    inline osg::vec3
    ParticleProcessor::rotateWorldToLocal( const osg::vec3& P )
    {
        return osg::vec3( getWorldToLocalMatrix() * P ) -
               osg::vec3( getWorldToLocalMatrix() * osg::vec3( 0, 0, 0 ) );
    }

    inline bool
    ParticleProcessor::isAlive() const
    {
        return _currentTime < ( _lifeTime + _startTime );
    }

}
