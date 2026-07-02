/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Text rendering settings for UI widgets. Specifies
 * font, character size, and text color for widget text.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/vec4.hpp>
#include <osgUI/Export.hpp>

namespace osgUI
{

    class OSGUI_EXPORT TextSettings : public osg::Inherit<osg::Object, TextSettings>
    {
        public:

            TextSettings();
            TextSettings( const TextSettings& textSettings,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               TextSettings )

            void
            setFont( const std::string& font )
            {
                _font = font;
            }

            const std::string&
            getFont() const
            {
                return _font;
            }

            void
            setCharacterSize( float characterSize )
            {
                _characterSize = characterSize;
            }

            float
            getCharacterSize() const
            {
                return _characterSize;
            }

        protected:

            virtual ~TextSettings()
            {
            }

            std::string _font;
            float       _characterSize;
    };

}
