/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Scalar-to-color mapping function. Maps float values to RGBA
 * colors for data visualization and terrain coloring.
 */
#pragma once

#include <osg/core/Referenced.hpp>
#include <osg/maths/vec4.hpp>
#include <osgSim/Export.hpp>

namespace osgSim
{

    /**
    ScalarsToColors defines the interface to map a scalar value to a color,
    and provides a default implementation of the mapping functionaltity,
    with colors ranging from black to white across the min - max scalar
    range.
    */
    class OSGSIM_EXPORT ScalarsToColors : public osg::Referenced
    {
        public:

            ScalarsToColors( float scalarMin,
                             float scalarMax );

            virtual ~ScalarsToColors()
            {
            }

            /** Get the color for a given scalar value. */
            virtual osg::vec4
            getColor( float scalar ) const;

            /** Get the minimum scalar value. */
            float
            getMin() const;

            /** Get the maximum scalar value. */
            float
            getMax() const;

        private:

            float _min, _max;
    };

}
