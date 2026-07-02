/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Collection of animated light points. Manages sequencing
 * and intensity animation for approach lighting systems.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>

namespace osgSim
{

    /*
     * LightPointSYSTEM encapsulates animation and intensity state in a single object
     *   that can be shared by several osgSim::LightPointNodes, thereby allowing an
     *   application to efficiently control the animation/intensity state of
     *   several LightPointNodes.
     */
    class LightPointSystem : public osg::Inherit<osg::Object, LightPointSystem>
    {
        public:

            LightPointSystem() :
                _intensity( 1.F ),
                _animationState( ANIMATION_ON )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            LightPointSystem( const LightPointSystem& lps,
                              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( lps,
                         copyop ),
                _intensity( lps._intensity ),
                _animationState( lps._animationState )
            {
            }

            OSG_REGISTER_TYPE( osgSim,
                               LightPointSystem )

            typedef enum
            {
                ANIMATION_ON,
                ANIMATION_OFF,
                ANIMATION_RANDOM
            } AnimationState;

            void
            setIntensity( float intensity )
            {
                _intensity = intensity;
            }

            float
            getIntensity() const
            {
                return _intensity;
            }

            void
            setAnimationState( LightPointSystem::AnimationState state )
            {
                _animationState = state;
            }

            LightPointSystem::AnimationState
            getAnimationState() const
            {
                return _animationState;
            }

        protected:

            ~LightPointSystem()
            {
            }

            float          _intensity;
            AnimationState _animationState;
    };

}
