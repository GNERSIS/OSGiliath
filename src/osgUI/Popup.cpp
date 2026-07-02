/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Popup overlay widget. Appears above the scene for
 * transient menus and tooltip displays.
 */
#include <osgUI/Popup.hpp>

#include <osg/core/Notify.hpp>
#include <osgText/Font.hpp>
#include <osgText/String.hpp>
#include <osgText/Text.hpp>

using namespace osgUI;

Popup::Popup()
{
}

Popup::Popup( const osgUI::Popup& dialog,
              const osg::CopyOp&  copyop ) :
    Inherit( dialog,
             copyop ),
    _title( dialog._title )
{
}

bool
Popup::handleImplementation( osgGA::EventVisitor* /*ev*/,
                             osgGA::Event* event )
{
    // OSG_NOTICE<<"Popup::handleImplementation"<<std::endl;

    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if( !ea )
    {
        return false;
    }

    switch( ea->getEventType() )
    {
        // case(osgGA::GUIEventAdapter::KEYDOWN):
        case( osgGA::GUIEventAdapter::KEYUP ) :
            OSG_NOTICE << "Key pressed : " << ( char )ea->getKey() << std::endl;

            if( ea->getKey() == 'c' )
            {
                setVisible( false );
                ea->setHandled( true );

                return true;
            }

            break;
        default :
            break;
    }

    return false;
}

void
Popup::leaveImplementation()
{
    // setVisible(false);
}

void
Popup::createGraphicsImplementation()
{
    _transform      = new osg::PositionAttitudeTransform;

    Style*    style = ( getStyle() != 0 ) ? getStyle() : Style::instance().get();

    osg::vec4 dialogBackgroundColor( 0.9F, 0.9F, 0.9F, 1.0F );

    _transform->addChild( style->createPanel( _extents, dialogBackgroundColor ) );

    bool requiresFrame =
        ( getFrameSettings() &&
          getFrameSettings()->getShape() != osgUI::FrameSettings::NO_FRAME );
    if( requiresFrame )
    {
        _transform->addChild(
            style->createFrame( _extents, getFrameSettings(), dialogBackgroundColor )
        );
    }
#if 1
    style->setupDialogStateSet( getOrCreateWidgetStateSet(), 6 );
#else
    style->setupPopupStateSet( getOrCreateWidgetStateSet(), 6 );
#endif
    style->setupClipStateSet( _extents, getOrCreateWidgetStateSet() );

    // render before the subgraph
    setGraphicsSubgraph( -1, _transform.get() );

    // render after the subgraph
    setGraphicsSubgraph( 1, style->createDepthSetPanel( _extents ) );
}
