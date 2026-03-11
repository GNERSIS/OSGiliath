/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Modal or modeless dialog container for UI widgets.
 * Provides title bar and layout management.
 */
#include <osgUI/Dialog>

#include <osg/core/Notify.hpp>
#include <osgText/Font>
#include <osgText/String>
#include <osgText/Text>
#include <osgUI/Callbacks>
#include <osgUI/Label>
#include <osgUI/PushButton>

using namespace osgUI;

Dialog::Dialog()
{
}

Dialog::Dialog( const osgUI::Dialog& dialog,
                const osg::CopyOp&   copyop ) :
    Inherit( dialog,
             copyop ),
    _title( dialog._title )
{
}

bool
Dialog::handleImplementation( osgGA::EventVisitor* /*ev*/,
                              osgGA::Event* event )
{
    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if( !ea )
    {
        return false;
    }

    switch( ea->getEventType() )
    {
        // case(osgGA::GUIEventAdapter::KEYDOWN):
        case( osgGA::GUIEventAdapter::KEYUP ) :
            OSG_NOTICE << "Key pressed : " << ea->getKey() << std::endl;

            break;
        default :
            break;
    }

    return false;
}

void
Dialog::createGraphicsImplementation()
{
    _group                = new osg::Group;

    Style*    style       = ( getStyle() != 0 ) ? getStyle() : Style::instance().get();

    float     titleHeight = 10.0;
    osg::box  titleBarExtents( _extents.xMin(),
                               _extents.yMax(),
                               _extents.zMin(),
                               _extents.xMax() - titleHeight,
                               _extents.yMax() + titleHeight,
                               _extents.zMin() );
    osg::box  closeButtonExtents( _extents.xMax() - titleHeight,
                                  _extents.yMax(),
                                  _extents.zMin(),
                                  _extents.xMax(),
                                  _extents.yMax() + titleHeight,
                                  _extents.zMin() );

    osg::vec4 dialogBackgroundColor( 0.84F, 0.82F, 0.82F, 1.0F );
    osg::vec4 dialogTitleBackgroundColor( 0.5, 0.5, 1.0, 1.0 );

    _group->addChild( style->createPanel( _extents, dialogBackgroundColor ) );

    _group->addChild( style->createPanel( titleBarExtents,
                                          dialogTitleBackgroundColor ) );

    osg::box dialogWithTitleExtents( _extents );
    dialogWithTitleExtents.expandBy( titleBarExtents );
    dialogWithTitleExtents.expandBy( closeButtonExtents );

    bool requiresFrame =
        ( getFrameSettings() &&
          getFrameSettings()->getShape() != osgUI::FrameSettings::NO_FRAME );
    if( requiresFrame )
    {
        _group->addChild( style->createFrame( dialogWithTitleExtents,
                                              getFrameSettings(),
                                              dialogBackgroundColor ) );

        titleBarExtents.min.x    += getFrameSettings()->getLineWidth();
        titleBarExtents.max.y    -= getFrameSettings()->getLineWidth();
        closeButtonExtents.max.x -= getFrameSettings()->getLineWidth();
        closeButtonExtents.max.y -= getFrameSettings()->getLineWidth();
    }

    OSG_NOTICE << "Dialog::_extents (" << _extents.xMin() << ", " << _extents.yMin()
               << ", " << _extents.zMin() << "), (" << _extents.xMax() << ", "
               << _extents.yMax() << ", " << _extents.zMax() << ")" << std::endl;
    OSG_NOTICE << "Dialog::titleBarExtents (" << titleBarExtents.xMin() << ", "
               << titleBarExtents.yMin() << ", " << titleBarExtents.zMin() << "), ("
               << titleBarExtents.xMax() << ", " << titleBarExtents.yMax() << ", "
               << titleBarExtents.zMax() << ")" << std::endl;
    OSG_NOTICE << "Dialog::dialogWithTitleExtents (" << dialogWithTitleExtents.xMin()
               << ", " << dialogWithTitleExtents.yMin() << ", "
               << dialogWithTitleExtents.zMin() << "), ("
               << dialogWithTitleExtents.xMax() << ", " << dialogWithTitleExtents.yMax()
               << ", " << dialogWithTitleExtents.zMax() << ")" << std::endl;

#if 0
    osg::ref_ptr<Node> node = style->createText(titleBarExtents, getAlignmentSettings(), getTextSettings(), _title);
    _titleDrawable = dynamic_cast<osgText::Text*>(node.get());
    _titleDrawable->setDataVariance(osg::Object::DataVariance::DYNAMIC);
    _group->addChild(_titleDrawable.get());
#endif

    osg::ref_ptr<PushButton> closeButton = new osgUI::PushButton;
    closeButton->setExtents( closeButtonExtents );
    closeButton->setText( "x" );
    closeButton->setAlignmentSettings( getAlignmentSettings() );
    closeButton->setTextSettings( getTextSettings() );
    // closeButton->setFrameSettings(getFrameSettings());
    closeButton->getOrCreateUserDataContainer()->addUserObject(
        new osgUI::CloseCallback( "released", this )
    );

    osg::ref_ptr<Label> titleLabel = new osgUI::Label;
    titleLabel->setExtents( titleBarExtents );
    titleLabel->setText( _title );
    titleLabel->setAlignmentSettings( getAlignmentSettings() );
    titleLabel->setTextSettings( getTextSettings() );
    titleLabel->setFrameSettings( getFrameSettings() );
    titleLabel->getOrCreateUserDataContainer()->addUserObject( new osgUI::DragCallback );

    _group->addChild( closeButton.get() );
    _group->addChild( titleLabel.get() );

    style->setupDialogStateSet( getOrCreateWidgetStateSet(), 5 );
    style->setupClipStateSet( dialogWithTitleExtents, getOrCreateWidgetStateSet() );

    // render before the subgraph
    setGraphicsSubgraph( -1, _group.get() );

    // render after the subgraph
    setGraphicsSubgraph( 1, style->createDepthSetPanel( dialogWithTitleExtents ) );
}
