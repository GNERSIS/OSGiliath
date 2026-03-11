/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Drop-down selection widget. Presents a list of items
 * with single-selection and change notification.
 */
#include <osgUI/ComboBox>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/ValueObject.hpp>
#include <osgText/Font>
#include <osgText/String>
#include <osgText/Text>

using namespace osgUI;

ComboBox::ComboBox() :
    _currentIndex( 0 )
{
}

ComboBox::ComboBox( const osgUI::ComboBox& combobox,
                    const osg::CopyOp&     copyop ) :
    Inherit( combobox,
             copyop ),
    _items( combobox._items ),
    _currentIndex( combobox._currentIndex )
{
}

bool
ComboBox::handleImplementation( osgGA::EventVisitor* ev,
                                osgGA::Event*        event )
{
    // OSG_NOTICE<<"ComboBox::handleImplementation"<<std::endl;

    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if( !ea )
    {
        return false;
    }

    bool hasFocus =
        getHasEventFocus() || ( _popup->getVisible() && _popup->getHasEventFocus() );

    if( !hasFocus )
    {
        if( ea->getEventType() == osgGA::GUIEventAdapter::PUSH && _popup->getVisible() )
        {
            _popup->setVisible( false );
        }

        return false;
    }

    switch( ea->getEventType() )
    {
        case( osgGA::GUIEventAdapter::SCROLL ) :
            if( ea->getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN )
            {
                if( getCurrentIndex() < getNumItems() - 1 )
                {
                    setCurrentIndex( getCurrentIndex() + 1 );
                }
                return true;
            }
            else if( ea->getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP )
            {
                if( getCurrentIndex() > 0 )
                {
                    setCurrentIndex( getCurrentIndex() - 1 );
                }
                return true;
            }
            break;

        case( osgGA::GUIEventAdapter::KEYDOWN ) :
            if( ea->getKey() == osgGA::GUIEventAdapter::KEY_Down )
            {
                if( getCurrentIndex() < getNumItems() - 1 )
                {
                    setCurrentIndex( getCurrentIndex() + 1 );
                }
                return true;
            }
            else if( ea->getKey() == osgGA::GUIEventAdapter::KEY_Up )
            {
                if( getCurrentIndex() > 0 )
                {
                    setCurrentIndex( getCurrentIndex() - 1 );
                }
                return true;
            }

            break;

        case( osgGA::GUIEventAdapter::PUSH ) :
            {
                if( _popup->getVisible() && _popup->getHasEventFocus() )
                {
                    osg::dvec3 position;
                    if( _popup->computeExtentsPositionInLocalCoordinates( ev,
                                                                          ea,
                                                                          position ) )
                    {
                        position   -= _popupItemOrigin;
                        position.x /= _popupItemSize.x;
                        position.y /= _popupItemSize.y;
                        int index   = static_cast<int>( position.y );
                        if( index >= 0 && index < static_cast<int>( _items.size() ) )
                        {
                            setCurrentIndex( static_cast<unsigned int>( index ) );
                        }
                    }
                    _popup->setVisible( false );
                }
                else if( getHasEventFocus() )
                {
                    _popup->setVisible( !_popup->getVisible() );
                }
                else
                {
                    _popup->setVisible( false );
                    return false;
                }
                break;
            }
        case( osgGA::GUIEventAdapter::RELEASE ) :
            OSG_NOTICE << "Button release " << std::endl;
            break;

        default :
            break;
    }

    return false;
}

void
ComboBox::enterImplementation()
{
    OSG_NOTICE << "ComboBox enter" << std::endl;
    if( _backgroundSwitch.valid() )
    {
        _backgroundSwitch->setSingleChildOn( 1 );
    }
}

void
ComboBox::leaveImplementation()
{
    OSG_NOTICE << "ComboBox leave" << std::endl;
    if( _backgroundSwitch.valid() )
    {
        _backgroundSwitch->setSingleChildOn( 0 );
    }
}

void
ComboBox::setCurrentIndex( unsigned int i )
{
    // OSG_NOTICE << "ComboBox::setCurrentIndex("<<i<<")"<<std::endl;
    if( _currentIndex == i )
    {
        return;
    }

    _currentIndex = i;
    if( _buttonSwitch.valid() )
    {
        _buttonSwitch->setSingleChildOn( _currentIndex );
    }

    currrentIndexChanged( _currentIndex );
}

void
ComboBox::currrentIndexChanged( unsigned int i )
{
    osg::CallbackObject* co = getCallbackObject( this, "currentIndexChanged" );
    if( co )
    {
        osg::Parameters inputParameters, outputParameters;
        inputParameters.push_back( new osg::UIntValueObject( "index", i ) );
        if( co->run( this, inputParameters, outputParameters ) )
        {
            return;
        }
    }
    currentIndexChangedImplementation( i );
}

void
ComboBox::currentIndexChangedImplementation( unsigned int i )
{
    OSG_NOTICE << "ComboBox::currentIndexChangedImplementation(" << i << ")"
               << std::endl;
}

void
ComboBox::createGraphicsImplementation()
{
    Style* style  = ( getStyle() != 0 ) ? getStyle() : Style::instance().get();

    _buttonSwitch = new osg::Switch;
    _popup        = new osgUI::Popup;
    _popup->setVisible( false );
    _popup->setFrameSettings( getFrameSettings() );

    osg::box                 extents( _extents );

    osg::ref_ptr<osg::Group> group = new osg::Group;
    bool                     requiresFrame =
        ( getFrameSettings() &&
          getFrameSettings()->getShape() != osgUI::FrameSettings::NO_FRAME );
    float     frameWidth = 0.0;
    float     unFocused  = 0.92F;
    float     withFocus  = 0.97F;
    osg::vec4 frameColor( unFocused, unFocused, unFocused, 1.0F );

    if( requiresFrame )
    {
        frameWidth = getFrameSettings()->getLineWidth();

        group->addChild(
            style->createFrame( _extents, getFrameSettings(), frameColor )
        );
        extents.min.x += frameWidth;
        extents.max.x -= frameWidth;
        extents.min.y += frameWidth;
        extents.max.y -= frameWidth;
    }

    bool itemsHaveColor = false;
    for( Items::iterator itr = _items.begin(); itr != _items.end(); ++itr )
    {
        Item* item = itr->get();
        if( item->getColor().a != 0.0F )
        {
            itemsHaveColor = true;
            break;
        }
    }

    // work out position of carat.
    float h            = extents.yMax() - extents.yMin();
    float w            = h * 0.7F;
    float minItemWidth = ( extents.xMax() - extents.xMin() ) * 0.5F;
    if( w > minItemWidth )
    {
        w = minItemWidth;
    }
    float    xDivision         = extents.xMax() - w;

    osg::box backgroundExtents = extents;
    osg::box iconExtents       = backgroundExtents;
    iconExtents.min.x          = xDivision;
    extents.max.x              = xDivision;

    if( itemsHaveColor )
    {
        backgroundExtents.min.x = xDivision;
    }

    OSG_NOTICE << "itemsHaveColor = " << itemsHaveColor << std::endl;

    // clear background of edit region
    _backgroundSwitch = new osg::Switch;
    _backgroundSwitch->addChild(
        style->createPanel( backgroundExtents,
                            osg::vec4( unFocused, unFocused, unFocused, 1.0 ) )
    );
    _backgroundSwitch->addChild(
        style->createPanel( backgroundExtents,
                            osg::vec4( withFocus, withFocus, withFocus, 1.0 ) )
    );
    _backgroundSwitch->setSingleChildOn( 0 );

    // assign carat
    group->addChild( _backgroundSwitch.get() );

    group->addChild( _buttonSwitch.get() );

    // group->addChild(style->createIcon(iconExtents, "cow.osgt", osg::vec4(withFocus,
    // withFocus, withFocus,1.0)));
    group->addChild(
        style->createIcon( iconExtents,
                           "Images/osg64.png",
                           osg::vec4( withFocus, withFocus, withFocus, 1.0 ) )
    );

    if( !_items.empty() )
    {

        float    margin = ( extents.yMax() - extents.yMin() ) * 0.1F;
        // float itemWidth = (_extents.xMax()-_extents.xMin()) - 2.0f*frameWidth;
        float    itemHeight  = ( _extents.yMax() - _extents.yMin() ) - 2.0F * frameWidth;
        float    popupHeight = ( itemHeight ) *
                               static_cast<float>( _items.size() ) +
                               margin *
                               static_cast<float>( _items.size() - 1 ) +
                               2.0F *
                               frameWidth;
        float    popupTop    = _extents.yMin() - frameWidth - margin * 1.0F;
        float    popupLeft   = _extents.xMin();
        float    popupRight  = _extents.xMax();

        osg::box popupExtents( popupLeft,
                               popupTop - popupHeight,
                               _extents.zMin(),
                               popupRight,
                               popupTop,
                               _extents.zMax() );
        _popup->setExtents( popupExtents );

        osg::box popupItemExtents( popupExtents.xMin() + frameWidth,
                                   popupTop - frameWidth - itemHeight,
                                   popupExtents.zMin(),
                                   popupExtents.xMax() - frameWidth,
                                   popupTop - frameWidth,
                                   popupExtents.zMax() );

        _popupItemOrigin.set( popupItemExtents.xMin(),
                              popupItemExtents.yMax(),
                              popupExtents.zMin() );
        _popupItemSize.set( popupItemExtents.xMax() - popupItemExtents.xMin(),
                            -( itemHeight + margin ),
                            0.0 );

        unsigned int index = 0;
        for( Items::iterator itr = _items.begin(); itr != _items.end(); ++itr, ++index )
        {
            Item* item = itr->get();
            OSG_NOTICE << "Creating item " << item->getText() << ", " << item->getColor()
                       << std::endl;

            // setup graphics for button
            {
                osg::ref_ptr<osg::Group> button_group = new osg::Group;
                if( item->getColor().a != 0.0F )
                {
                    button_group->addChild( style->createPanel( extents,
                                                                item->getColor() ) );
                }
                if( !item->getText().empty() )
                {
                    button_group->addChild( style->createText( extents,
                                                               getAlignmentSettings(),
                                                               getTextSettings(),
                                                               item->getText() ) );
                }
                _buttonSwitch->addChild( button_group.get() );
            }

            // setup graphics for popup
            {
                osg::ref_ptr<osg::Group> popup_group = new osg::Group;

                if( item->getColor().a != 0.0F )
                {
                    popup_group->addChild( style->createPanel( popupItemExtents,
                                                               item->getColor() ) );
                }
                if( !item->getText().empty() )
                {
                    popup_group->addChild( style->createText( popupItemExtents,
                                                              getAlignmentSettings(),
                                                              getTextSettings(),
                                                              item->getText() ) );
                }
                _popup->addChild( popup_group.get() );

                popupItemExtents.min.y -= ( itemHeight + margin );
                popupItemExtents.max.y -= ( itemHeight + margin );
            }
        }
    }
    else
    {
        _buttonSwitch->addChild( style->createPanel( _extents, frameColor ) );
    }

    _buttonSwitch->setSingleChildOn( _currentIndex );

    style->setupClipStateSet( _extents, getOrCreateWidgetStateSet() );

    setGraphicsSubgraph( 0, group.get() );
    addChild( _popup.get() );
}
