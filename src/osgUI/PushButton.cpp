/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Clickable push button widget. Emits pressed/released
 * events for user interaction in 3D UI.
 */
#include <osgUI/PushButton.hpp>

#include <osg/core/Notify.hpp>
#include <osgText/Font.hpp>
#include <osgText/String.hpp>
#include <osgText/Text.hpp>

using namespace osgUI;

PushButton::PushButton()
{
}

PushButton::PushButton( const osgUI::PushButton& pb,
                        const osg::CopyOp&       copyop ) :
    Inherit( pb,
             copyop ),
    _text( pb._text )
{
}

bool
PushButton::handleImplementation( osgGA::EventVisitor* /*ev*/,
                                  osgGA::Event* event )
{
    if( !getHasEventFocus() )
    {
        return false;
    }

    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if( !ea )
    {
        return false;
    }

    switch( ea->getEventType() )
    {
        case( osgGA::GUIEventAdapter::PUSH ) :
            if( _buttonSwitch.valid() )
            {
                pressed();
            }
            break;
        case( osgGA::GUIEventAdapter::RELEASE ) :
            if( _buttonSwitch.valid() )
            {
                released();
            }
            break;
        default :
            break;
    }

    return false;
}

void
PushButton::enterImplementation()
{
    OSG_NOTICE << "PushButton enter" << std::endl;
    if( _buttonSwitch.valid() )
    {
        _buttonSwitch->setSingleChildOn( 1 );
    }
}

void
PushButton::leaveImplementation()
{
    OSG_NOTICE << "PushButton leave" << std::endl;
    if( _buttonSwitch.valid() )
    {
        _buttonSwitch->setSingleChildOn( 0 );
    }
}

void
PushButton::createGraphicsImplementation()
{
    osg::ref_ptr<osg::Group> group = new osg::Group;

    Style*    style     = ( getStyle() != 0 ) ? getStyle() : Style::instance().get();

    float     pressed   = 0.88F;
    float     unFocused = 0.92F;
    float     withFocus = 0.97F;

    osg::vec4 frameColor( unFocused, unFocused, unFocused, 1.0F );

    osg::box  extents( _extents );

    bool      requiresFrame =
        ( getFrameSettings() &&
          getFrameSettings()->getShape() != osgUI::FrameSettings::NO_FRAME );
    if( requiresFrame )
    {
        group->addChild(
            style->createFrame( _extents, getFrameSettings(), frameColor )
        );
        extents.min.x += getFrameSettings()->getLineWidth();
        extents.max.x -= getFrameSettings()->getLineWidth();
        extents.min.y += getFrameSettings()->getLineWidth();
        extents.max.y -= getFrameSettings()->getLineWidth();
    }

    _buttonSwitch = new osg::Switch;
    _buttonSwitch->addChild(
        style->createPanel( extents, osg::vec4( unFocused, unFocused, unFocused, 1.0 ) )
    );
    _buttonSwitch->addChild(
        style->createPanel( extents, osg::vec4( withFocus, withFocus, withFocus, 1.0 ) )
    );
    _buttonSwitch->addChild(
        style->createPanel( extents, osg::vec4( pressed, pressed, pressed, 1.0 ) )
    );
    _buttonSwitch->setSingleChildOn( 0 );

    group->addChild( _buttonSwitch.get() );

    // create label.
    osg::ref_ptr<Node> node =
        style->createText( extents, getAlignmentSettings(), getTextSettings(), _text );

    _textDrawable = dynamic_cast<osgText::Text*>( node.get() );

    node->setDataVariance( osg::Object::DataVariance::DYNAMIC );

    group->addChild( node.get() );

    style->setupClipStateSet( _extents, getOrCreateWidgetStateSet() );

    setGraphicsSubgraph( 0, group.get() );
}

void
PushButton::pressedImplementation()
{
    _buttonSwitch->setSingleChildOn( 2 );
}

void
PushButton::releasedImplementation()
{
    _buttonSwitch->setSingleChildOn( 1 );
}
