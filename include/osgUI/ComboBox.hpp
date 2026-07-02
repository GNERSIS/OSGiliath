/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Drop-down selection widget. Presents a list of items
 * with single-selection and change notification.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Switch.hpp>
#include <osgText/Text.hpp>
#include <osgUI/Popup.hpp>

namespace osgUI
{

    class OSGUI_EXPORT Item : public osg::Inherit<osg::Object, Item>
    {
        public:

            Item() :
                _color( 1.0F,
                        1.0F,
                        1.0F,
                        0.0F )
            {
            }

            Item( const std::string& str ) :
                _text( str ),
                _color( 1.0F,
                        1.0F,
                        1.0F,
                        0.0F )
            {
            }

            Item( const std::string& str,
                  const osg::vec4&   col ) :
                _text( str ),
                _color( col )
            {
            }

            Item( const osg::vec4& col ) :
                _color( col )
            {
            }

            Item( const Item&        item,
                  const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( item,
                         copyop ),
                _text( item._text ),
                _color( item._color )
            {
            }

            OSG_REGISTER_TYPE( osgUI,
                               Item )

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
            setColor( const osg::vec4& color )
            {
                _color = color;
            }

            osg::vec4&
            getColor()
            {
                return _color;
            }

            const osg::vec4&
            getColor() const
            {
                return _color;
            }

        protected:

            virtual ~Item()
            {
            }

            std::string _text;
            osg::vec4   _color;
    };

    class OSGUI_EXPORT ComboBox : public osg::Inherit<osgUI::Widget, ComboBox>
    {
        public:

            ComboBox();
            ComboBox( const ComboBox&    combobox,
                      const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               ComboBox )

            void
            addItem( Item* item )
            {
                _items.push_back( item );
                dirty();
            }

            void
            setItem( unsigned int i,
                     Item*        item )
            {
                _items[i] = item;
                dirty();
            }

            Item*
            getItem( unsigned int i )
            {
                return _items[i].get();
            }

            const Item*
            getItem( unsigned int i ) const
            {
                return _items[i].get();
            }

            void
            clear()
            {
                _items.clear();
                dirty();
            }

            void
            removeItem( unsigned int i )
            {
                _items.erase( _items.begin() + i );
                dirty();
            }

            unsigned int
            getNumItems()
            {
                return static_cast<unsigned int>( _items.size() );
            }

            typedef std::vector<osg::ref_ptr<Item>> Items;

            void
            setItems( const Items& items )
            {
                _items = items;
            }

            Items&
            getItems()
            {
                return _items;
            }

            const Items&
            getItems() const
            {
                return _items;
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

            virtual ~ComboBox()
            {
            }

            Items                      _items;
            unsigned int               _currentIndex;
            osg::dvec3                 _popupItemOrigin;
            osg::dvec3                 _popupItemSize;

            osg::ref_ptr<osg::Switch>  _buttonSwitch;
            osg::ref_ptr<osg::Switch>  _backgroundSwitch;
            osg::ref_ptr<osgUI::Popup> _popup;
    };

}
