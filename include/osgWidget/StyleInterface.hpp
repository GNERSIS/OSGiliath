/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Mixin interface providing style name property.
 * Widgets use this to participate in StyleManager lookups.
 */
// Code by: Jeremy Moles (cubicool) 2007-2008

#pragma once

#include <osgWidget/Export.hpp>

namespace osgWidget
{

    class OSGWIDGET_EXPORT StyleInterface
    {
        public:

            StyleInterface() :
                _style( "" )
            {
            }

            StyleInterface( const StyleInterface& si ) :
                _style( si._style )
            {
            }

            void
            setStyle( const std::string& style )
            {
                _style = style;
            }

            std::string&
            getStyle()
            {
                return _style;
            }

            const std::string&
            getStyle() const
            {
                return _style;
            }

        private:

            std::string _style;
    };

}
