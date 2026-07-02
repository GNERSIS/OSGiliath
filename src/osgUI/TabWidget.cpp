/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Tabbed container widget. Manages multiple pages with
 * a tab bar for switching between content panels.
 */
#include <osgUI/TabWidget.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/ValueObject.hpp>
#include <osgText/Font.hpp>
#include <osgText/String.hpp>
#include <osgText/Text.hpp>

using namespace osgUI;

TabWidget::TabWidget() :
    _currentIndex( 0 )
{
}

TabWidget::TabWidget( const osgUI::TabWidget& tabwidget,
                      const osg::CopyOp&      copyop ) :
    Inherit( tabwidget,
             copyop ),
    _tabs( tabwidget._tabs ),
    _currentIndex( tabwidget._currentIndex )
{
}

bool
TabWidget::handleImplementation( osgGA::EventVisitor* ev,
                                 osgGA::Event*        event )
{
    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if( !ea )
    {
        return false;
    }

    osgGA::GUIActionAdapter* aa = ev ? ev->getActionAdapter() : 0;
    if( !aa )
    {
        return false;
    }

    if( !getHasEventFocus() )
    {
        return false;
    }

    unsigned int tabHeaderContainsPointer = static_cast<unsigned int>( _tabs.size() );

    // test if active tab header contains pointer
    {
        osg::NodePath nodePath = ev->getNodePath();
        nodePath.push_back( _activeHeaderSwitch.get() );

        osgUtil::LineSegmentIntersector::Intersections intersections;
        if( aa->computeIntersections( *ea, nodePath, intersections ) )
        {
            tabHeaderContainsPointer = _currentIndex;
        }
    }

    // test if inactive tab header contains pointer
    {
        osg::NodePath nodePath = ev->getNodePath();
        nodePath.push_back( _inactiveHeaderSwitch.get() );

        osgUtil::LineSegmentIntersector::Intersections intersections;
        if( aa->computeIntersections( *ea, nodePath, intersections ) )
        {
            const osgUtil::LineSegmentIntersector::Intersection& Intersection =
                *intersections.begin();
            for( osg::NodePath::const_iterator itr = Intersection.nodePath.begin();
                 itr != Intersection.nodePath.end();
                 ++itr )
            {
                if( ( *itr )->getUserValue( "index", tabHeaderContainsPointer ) )
                {
                    break;
                }
            }
        }
    }

    if( tabHeaderContainsPointer >= _tabs.size() )
    {
        return false;
    }

    switch( ea->getEventType() )
    {
        case( osgGA::GUIEventAdapter::SCROLL ) :
            if( ea->getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN )
            {
                if( getCurrentIndex() < _tabs.size() - 1 )
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
            if( ea->getKey() ==
                osgGA::GUIEventAdapter::KEY_Down ||
                ea->getKey() == osgGA::GUIEventAdapter::KEY_Right )
            {
                if( getCurrentIndex() < _tabs.size() - 1 )
                {
                    setCurrentIndex( getCurrentIndex() + 1 );
                }
                return true;
            }
            else if( ea->getKey() ==
                     osgGA::GUIEventAdapter::KEY_Up ||
                     ea->getKey() == osgGA::GUIEventAdapter::KEY_Left )
            {
                if( getCurrentIndex() > 0 )
                {
                    setCurrentIndex( getCurrentIndex() - 1 );
                }
                return true;
            }

            break;

        case( osgGA::GUIEventAdapter::RELEASE ) :
            {
                setCurrentIndex( tabHeaderContainsPointer );
                return true;

                break;
            }
        default :
            break;
    }

    return false;
}

void
TabWidget::enterImplementation()
{
    OSG_NOTICE << "TabWidget enter" << std::endl;
}

void
TabWidget::leaveImplementation()
{
    OSG_NOTICE << "TabWidget leave" << std::endl;
}

void
TabWidget::setCurrentIndex( unsigned int i )
{
    // OSG_NOTICE << "TabWidget::setCurrentIndex("<<i<<")"<<std::endl;
    if( _currentIndex == i )
    {
        return;
    }

    _currentIndex = i;
    _activateWidgets();

    currrentIndexChanged( _currentIndex );
}

void
TabWidget::currrentIndexChanged( unsigned int i )
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
TabWidget::currentIndexChangedImplementation( unsigned int i )
{
    OSG_NOTICE << "TabWidget::currentIndexChangedImplementation(" << i << ")"
               << std::endl;
}

void
TabWidget::createGraphicsImplementation()
{
    Style* style = ( getStyle() != 0 ) ? getStyle() : Style::instance().get();

    // bool requiresFrame = (getFrameSettings() &&
    // getFrameSettings()->getShape()!=osgUI::FrameSettings::NO_FRAME);

    _inactiveHeaderSwitch        = new osg::Switch;
    _activeHeaderSwitch          = new osg::Switch;
    _tabWidgetSwitch             = new osg::Switch;

    float        active          = 0.84F;
    float        inactive        = 0.80F;
    float        titleHeight     = 10.0F;
    float        characterWidth  = titleHeight * 0.7F;
    float        margin          = titleHeight * 0.2F;

    unsigned int tabIndex        = 0;

    osg::box     centerExtents   = _extents;
    centerExtents.max.y         -= titleHeight;

    osg::ref_ptr<osgUI::AlignmentSettings> textAlignment =
        new osgUI::AlignmentSettings( osgUI::AlignmentSettings::LEFT_CENTER );

    osg::ref_ptr<FrameSettings> fs = getFrameSettings();
    if( !fs )
    {
        fs = new osgUI::FrameSettings;
        fs->setShadow( osgUI::FrameSettings::RAISED );
        fs->setLineWidth( 1.0F );
    }

    osg::vec4 dialogBackgroundColor( active, active, active, 1.0 );
    osg::vec4 inactiveColor( inactive, inactive, inactive, 1.0F );

    float     xPos = _extents.xMin();
    float     yMin = _extents.yMax() - titleHeight - fs->getLineWidth();
    float     yMax = _extents.yMax();
    float     zMin = _extents.zMin();
    float     zMax = _extents.zMax();

    for( Tabs::iterator itr = _tabs.begin(); itr != _tabs.end(); ++itr, ++tabIndex )
    {
        Tab*     tab   = itr->get();

        float    width = static_cast<float>( tab->getText().size() ) * characterWidth;

        osg::box headerExtents( xPos, yMin, zMin, xPos + width, yMax, zMax );
        osg::box textExtents( xPos + margin,
                              yMin,
                              zMin,
                              xPos + width - margin,
                              yMax,
                              zMax );

        osg::ref_ptr<osg::Node>     textNode = style->createText( textExtents,
                                                                  textAlignment.get(),
                                                                  getTextSettings(),
                                                                  tab->getText() );

        osg::ref_ptr<osgText::Text> text =
            dynamic_cast<osgText::Text*>( textNode.get() );
        if( text.valid() )
        {
            textExtents = text->getBoundingBox();
        }

        // adjust position of size of text.
        float textWidth     = ( textExtents.xMax() - textExtents.xMin() );

        headerExtents.max.x = textExtents.xMin() + textWidth + margin;

        osg::ref_ptr<osg::Node> inactive_panel =
            _createTabHeader( headerExtents, fs.get(), inactiveColor );
        osg::ref_ptr<osg::Node> selected_panel =
            _createTabHeader( headerExtents, fs.get(), dialogBackgroundColor );

        osg::ref_ptr<osg::Group> selected_group = new osg::Group;
        selected_group->setUserValue( "index", tabIndex );
        selected_group->addChild( selected_panel.get() );
        selected_group->addChild( text.get() );

        osg::ref_ptr<osg::Group> inactive_group = new osg::Group;
        inactive_group->setUserValue( "index", tabIndex );
        inactive_group->addChild( inactive_panel.get() );
        inactive_group->addChild( text.get() );

        _inactiveHeaderSwitch->addChild( inactive_group.get() );
        _activeHeaderSwitch->addChild( selected_group.get() );
        _tabWidgetSwitch->addChild( tab->getWidget() );

        xPos += textWidth + 3.0F * margin;
    }

    setGraphicsSubgraph( -4, _inactiveHeaderSwitch.get() );

    osg::ref_ptr<osg::Node> backgroundPanel =
        _createTabFrame( centerExtents, fs.get(), dialogBackgroundColor );
    setGraphicsSubgraph( -3, backgroundPanel.get() );

    setGraphicsSubgraph( -2, _activeHeaderSwitch.get() );
    setGraphicsSubgraph( -1, _tabWidgetSwitch.get() );

    _activateWidgets();
}

void
TabWidget::_activateWidgets()
{
    if( _graphicsInitialized && _currentIndex < _tabs.size() )
    {
        OSG_NOTICE << "Activating widget " << _currentIndex << std::endl;

        _inactiveHeaderSwitch->setAllChildrenOn();
        _inactiveHeaderSwitch->setValue( _currentIndex, false );

        _activeHeaderSwitch->setAllChildrenOff();
        _activeHeaderSwitch->setValue( _currentIndex, true );

        _tabWidgetSwitch->setAllChildrenOff();
        _tabWidgetSwitch->setValue( _currentIndex, true );
    }
}

osg::Node*
TabWidget::_createTabFrame( const osg::box&       extents,
                            osgUI::FrameSettings* fs,
                            const osg::vec4&      color )
{
    Style* style = ( getStyle() != 0 ) ? getStyle() : Style::instance().get();
    osg::ref_ptr<osg::Group> group = new osg::Group;

    group->addChild( style->createPanel( extents, color ) );
    group->addChild( style->createFrame( extents, fs, color ) );

    return group.release();
}

osg::Node*
TabWidget::_createTabHeader( const osg::box&       extents,
                             osgUI::FrameSettings* frameSettings,
                             const osg::vec4&      color )
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setName( "Frame" );

    float topScale    = 1.0F;
    float bottomScale = 1.0F;
    float leftScale   = 1.0F;
    float rightScale  = 1.0F;

    if( frameSettings )
    {
        switch( frameSettings->getShadow() )
        {
            case( FrameSettings::PLAIN ) :
                // default settings are appropriate for PLAIN
                break;
            case( FrameSettings::SUNKEN ) :
                topScale    = 0.6F;
                bottomScale = 1.2F;
                leftScale   = 0.8F;
                rightScale  = 0.8F;
                break;
            case( FrameSettings::RAISED ) :
                topScale    = 1.2F;
                bottomScale = 0.6F;
                leftScale   = 0.8F;
                rightScale  = 0.8F;
                break;
        }
    }

    osg::vec4 topColor( std::min( color.r * topScale, 1.0F ),
                        std::min( color.g * topScale, 1.0F ),
                        std::min( color.b * topScale, 1.0F ),
                        color.a );
    osg::vec4 bottomColor( std::min( color.r * bottomScale, 1.0F ),
                           std::min( color.g * bottomScale, 1.0F ),
                           std::min( color.b * bottomScale, 1.0F ),
                           color.a );
    osg::vec4 leftColor( std::min( color.r * leftScale, 1.0F ),
                         std::min( color.g * leftScale, 1.0F ),
                         std::min( color.b * leftScale, 1.0F ),
                         color.a );
    osg::vec4 rightColor( std::min( color.r * rightScale, 1.0F ),
                          std::min( color.g * rightScale, 1.0F ),
                          std::min( color.b * rightScale, 1.0F ),
                          color.a );

    float     lineWidth = frameSettings ? frameSettings->getLineWidth() : 1.0F;

    osg::vec3 outerBottomLeft( extents.xMin(),
                               extents.yMin() + lineWidth,
                               extents.zMin() );
    osg::vec3 outerBottomRight( extents.xMax(),
                                extents.yMin() + lineWidth,
                                extents.zMin() );
    osg::vec3 outerTopLeft( extents.xMin(), extents.yMax(), extents.zMin() );
    osg::vec3 outerTopRight( extents.xMax(), extents.yMax(), extents.zMin() );

    osg::vec3 innerBottomLeft( extents.xMin() + lineWidth,
                               extents.yMin(),
                               extents.zMin() );
    osg::vec3 innerBottomRight( extents.xMax() - lineWidth,
                                extents.yMin(),
                                extents.zMin() );
    osg::vec3 innerTopLeft( extents.xMin() + lineWidth,
                            extents.yMax() - lineWidth,
                            extents.zMin() );
    osg::vec3 innerTopRight( extents.xMax() - lineWidth,
                             extents.yMax() - lineWidth,
                             extents.zMin() );

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray( vertices.get() );

    vertices->push_back( outerBottomLeft );     // 0
    vertices->push_back( outerBottomRight );    // 1
    vertices->push_back( outerTopLeft );        // 2
    vertices->push_back( outerTopRight );       // 3

    vertices->push_back( innerBottomLeft );     // 4
    vertices->push_back( innerBottomRight );    // 5
    vertices->push_back( innerTopLeft );        // 6
    vertices->push_back( innerTopRight );       // 7

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    geometry->setColorArray( colours.get(), osg::Array::BIND_PER_PRIMITIVE_SET );

    // bottom
    {
        colours->push_back( bottomColor );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
        geometry->addPrimitiveSet( primitives.get() );
        primitives->push_back( 4 );
        primitives->push_back( 0 );
        primitives->push_back( 5 );
        primitives->push_back( 1 );
    }

    // top
    {
        colours->push_back( topColor );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
        geometry->addPrimitiveSet( primitives.get() );
        primitives->push_back( 2 );
        primitives->push_back( 6 );
        primitives->push_back( 3 );
        primitives->push_back( 7 );
    }

    // left
    {
        colours->push_back( leftColor );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
        geometry->addPrimitiveSet( primitives.get() );
        primitives->push_back( 2 );
        primitives->push_back( 0 );
        primitives->push_back( 6 );
        primitives->push_back( 4 );
    }

    // right
    {
        colours->push_back( rightColor );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
        geometry->addPrimitiveSet( primitives.get() );
        primitives->push_back( 7 );
        primitives->push_back( 5 );
        primitives->push_back( 3 );
        primitives->push_back( 1 );
    }

    // center
    {
        colours->push_back( color );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
        geometry->addPrimitiveSet( primitives.get() );
        primitives->push_back( 6 );
        primitives->push_back( 4 );
        primitives->push_back( 7 );
        primitives->push_back( 5 );
    }

    return geometry.release();
}
