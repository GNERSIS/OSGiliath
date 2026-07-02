/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Tabbed container widget. Manages multiple pages with
 * a tab bar for switching between content panels.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Switch.hpp>
#include <osgText/Text.hpp>
#include <osgUI/Popup.hpp>

namespace osgUI
{

    class OSGUI_EXPORT Tab : public osg::Inherit<osg::Object, Tab>
    {
        public:

            Tab()
            {
            }

            Tab( const std::string& str ) :
                _text( str )
            {
            }

            Tab( const Tab&         item,
                 const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( item,
                         copyop ),
                _text( item._text )
            {
            }

            OSG_REGISTER_TYPE( osgUI,
                               Tab )

            void
            setText( const std::string& text )
            {
                _text = text;
            }

            std::string&
            getText()
            {
                return _text;
            }

            const std::string&
            getText() const
            {
                return _text;
            }

            void
            setWidget( osgUI::Widget* widget )
            {
                _widget = widget;
            }

            osgUI::Widget*
            getWidget()
            {
                return _widget.get();
            }

            const osgUI::Widget*
            getWidget() const
            {
                return _widget.get();
            }

        protected:

            virtual ~Tab()
            {
            }

            std::string                 _text;
            osg::ref_ptr<osgUI::Widget> _widget;
    };

    class OSGUI_EXPORT TabWidget : public osg::Inherit<osgUI::Widget, TabWidget>
    {
        public:

            TabWidget();
            TabWidget( const TabWidget&   combobox,
                       const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               TabWidget )

            void
            addTab( Tab* item )
            {
                _tabs.push_back( item );
                dirty();
            }

            void
            setTab( unsigned int i,
                    Tab*         item )
            {
                _tabs[i] = item;
                dirty();
            }

            Tab*
            getTab( unsigned int i )
            {
                return _tabs[i].get();
            }

            const Tab*
            getTab( unsigned int i ) const
            {
                return _tabs[i].get();
            }

            void
            clear()
            {
                _tabs.clear();
                dirty();
            }

            void
            removeTab( unsigned int i )
            {
                _tabs.erase( _tabs.begin() + i );
                dirty();
            }

            unsigned int
            getNumTabs()
            {
                return static_cast<unsigned int>( _tabs.size() );
            }

            typedef std::vector<osg::ref_ptr<Tab>> Tabs;

            void
            setTabs( const Tabs& items )
            {
                _tabs = items;
            }

            Tabs&
            getTabs()
            {
                return _tabs;
            }

            const Tabs&
            getTabs() const
            {
                return _tabs;
            }

            void
            setCurrentIndex( unsigned int i );

            unsigned int
            getCurrentIndex() const
            {
                return _currentIndex;
            }

            virtual void
            currrentIndexChanged( unsigned int i );
            virtual void
            currentIndexChangedImplementation( unsigned int i );

            virtual bool
            handleImplementation( osgGA::EventVisitor* ev,
                                  osgGA::Event*        event );
            virtual void
            createGraphicsImplementation();
            virtual void
            enterImplementation();
            virtual void
            leaveImplementation();

        protected:

            virtual ~TabWidget()
            {
            }

            void
            _activateWidgets();

            osg::Node*
            _createTabFrame( const osg::box&       extents,
                             osgUI::FrameSettings* fs,
                             const osg::vec4&      color );
            osg::Node*
                                      _createTabHeader( const osg::box&       extents,
                                                        osgUI::FrameSettings* fs,
                                                        const osg::vec4&      color );

            Tabs                      _tabs;
            unsigned int              _currentIndex;

            osg::ref_ptr<osg::Switch> _inactiveHeaderSwitch;
            osg::ref_ptr<osg::Switch> _activeHeaderSwitch;
            osg::ref_ptr<osg::Switch> _tabWidgetSwitch;
    };

}
