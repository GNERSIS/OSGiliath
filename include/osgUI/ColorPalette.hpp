/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Color palette widget providing a grid of selectable colors.
 * Used for color picker UIs in 3D scenes.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/vec4.hpp>
#include <osgUI/Export.hpp>

namespace osgUI
{

    class OSGUI_EXPORT ColorPalette : public osg::Inherit<osg::Object, ColorPalette>
    {
        public:

            ColorPalette();
            ColorPalette( const ColorPalette& cp,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               ColorPalette )

            typedef std::vector<osg::vec4> Colors;

            void
            setColors( const Colors& colors )
            {
                _colors = colors;
            }

            Colors&
            getColors()
            {
                return _colors;
            }

            const Colors&
            getColors() const
            {
                return _colors;
            }

            typedef std::vector<std::string> Names;

            void
            setNames( const Names& names )
            {
                _names = names;
            }

            Names&
            getNames()
            {
                return _names;
            }

            const Names&
            getNames() const
            {
                return _names;
            }

        protected:

            virtual ~ColorPalette()
            {
            }

            Colors _colors;
            Names  _names;
    };

}
