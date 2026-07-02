/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Single light point definition with position, color, intensity,
 * and sector parameters for directional visibility.
 */
#pragma once

#include <osg/maths/quat.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osgSim/BlinkSequence.hpp>
#include <osgSim/Export.hpp>
#include <osgSim/Sector.hpp>

namespace osgSim
{

    class OSGSIM_EXPORT LightPoint
    {
        public:

            enum BlendingMode
            {
                ADDITIVE,
                BLENDED
            };

            LightPoint();

            LightPoint( const osg::vec3& position,
                        const osg::vec4& color );

            LightPoint( bool             on,
                        const osg::vec3& position,
                        const osg::vec4& color,
                        float            intensity     = 1.0F,
                        float            radius        = 1.0F,
                        Sector*          sector        = 0,
                        BlinkSequence*   blinkSequence = 0,
                        BlendingMode     blendingMode  = BLENDED );

            LightPoint( const LightPoint& lp );

            LightPoint&
                                        operator=( const LightPoint& lp );

            bool                        _on;
            osg::vec3                   _position;
            osg::vec4                   _color;
            float                       _intensity;
            float                       _radius;

            osg::ref_ptr<Sector>        _sector;
            osg::ref_ptr<BlinkSequence> _blinkSequence;

            BlendingMode                _blendingMode;
    };

}
