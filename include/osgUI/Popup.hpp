/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Popup overlay widget. Appears above the scene for
 * transient menus and tooltip displays.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osgText/Text.hpp>
#include <osgUI/Widget.hpp>

namespace osgUI
{

    class OSGUI_EXPORT Popup : public osg::Inherit<osgUI::Widget, Popup>
    {
        public:

            Popup();
            Popup( const Popup&       dialog,
                   const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               Popup )

            virtual void
            leaveImplementation();

            bool
            handleImplementation( osgGA::EventVisitor* ev,
                                  osgGA::Event*        event );

            virtual void
            createGraphicsImplementation();

        protected:

            virtual ~Popup()
            {
            }

            std::string                                  _title;

            osg::ref_ptr<osg::PositionAttitudeTransform> _transform;
    };

}
