/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Callback interfaces for UI widget events.
 * Provides slots for pressed, released, and value-changed.
 */
#include <osgUI/Callbacks>

#include <osg/core/io_utils.hpp>
#include <osg/core/ValueObject.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osgUI/Dialog>
#include <osgUI/Widget>

using namespace osgUI;

CloseCallback::CloseCallback( const std::string& callbackName,
                              osgUI::Widget*     closeWidget ) :
    _closeWidget( closeWidget )
{
    setName( callbackName );
}

CloseCallback::CloseCallback( const CloseCallback& hc,
                              const osg::CopyOp&   copyop ) :
    Inherit( hc,
             copyop )
{
}

bool
CloseCallback::run( osg::Object* object,
                    osg::Parameters&,
                    osg::Parameters& ) const
{
    if( _closeWidget.valid() )
    {
        _closeWidget->setVisible( false );
    }

    osg::Node* node = object->asNode();
    if( node )
    {
        osg::NodePathList nodePathList = node->getParentalNodePaths();
        for( osg::NodePathList::iterator itr = nodePathList.begin();
             itr != nodePathList.end();
             ++itr )
        {
            osg::NodePath& nodePath = *itr;
            for( osg::NodePath::reverse_iterator ritr = nodePath.rbegin();
                 ritr != nodePath.rend();
                 ++ritr )
            {
                osgUI::Dialog* dialog = dynamic_cast<osgUI::Dialog*>( *ritr );
                if( dialog )
                {
                    dialog->setVisible( false );
                    break;
                }
            }
        }
        return true;
    }
    return false;
}

HandleCallback::HandleCallback()
{
    setName( "handle" );
}

HandleCallback::HandleCallback( const HandleCallback& hc,
                                const osg::CopyOp&    copyop ) :
    Inherit( hc,
             copyop )
{
}

bool
HandleCallback::run( osg::Object* /*object*/,
                     osg::Parameters& inputParameters,
                     osg::Parameters& outputParameters ) const
{
    if( inputParameters.size() >= 2 )
    {
        osgGA::EventVisitor* ev =
            dynamic_cast<osgGA::EventVisitor*>( inputParameters[0].get() );
        osgGA::Event* event = dynamic_cast<osgGA::Event*>( inputParameters[1].get() );
        if( ev && event )
        {
            outputParameters.push_back(
                new osg::BoolValueObject( "return", handle( ev, event ) )
            );
            return true;
        }
    }
    return false;
}

bool
HandleCallback::handle( osgGA::EventVisitor* /*ev*/,
                        osgGA::Event* /*event*/ ) const
{
    return false;
}

DragCallback::DragCallback() :
    _dragging( false )
{
}

DragCallback::DragCallback( const DragCallback& hc,
                            const osg::CopyOp&  copyop ) :
    osg::Object( hc,
                 copyop ),
    osg::Callback( hc,
                   copyop ),
    Inherit( hc,
             copyop ),
    _dragging( false )
{
}

osg::Transform*
findNearestTransform( const osg::NodePath& nodePath )
{
    osg::Transform* transform = 0;
    for( osg::NodePath::const_reverse_iterator itr = nodePath.rbegin();
         itr != nodePath.rend();
         ++itr )
    {
        if( ( *itr )->asTransform() )
        {
            transform = ( *itr )->asTransform();
            break;
        }
    }
    return transform;
}

bool
DragCallback::handle( osgGA::EventVisitor* ev,
                      osgGA::Event*        event ) const
{
    osgGA::GUIEventAdapter* ea = event ? event->asGUIEventAdapter() : 0;
    if( !ev || !ea )
    {
        return false;
    }

    osgUI::Widget* widget = dynamic_cast<osgUI::Widget*>(
        ev->getNodePath().empty() ? 0 : ev->getNodePath().back()
    );
    if( widget && widget->getHasEventFocus() )
    {
        DragCallback* dc = const_cast<DragCallback*>( this );
        switch( ea->getEventType() )
        {
            case( osgGA::GUIEventAdapter::SCROLL ) :
                {
                    osg::dvec3 localPosition;
                    if( !widget->computeExtentsPositionInLocalCoordinates(
                            ev,
                            ea,
                            localPosition
                        ) )
                    {
                        break;
                    }

                    OSG_NOTICE << "Scroll motion: " << ea->getScrollingMotion() << ", "
                               << localPosition << std::endl;
                    double scale = 1.0;

                    switch( ea->getScrollingMotion() )
                    {
                        case( osgGA::GUIEventAdapter::SCROLL_UP ) :
                            scale = 0.9;
                            break;
                        case( osgGA::GUIEventAdapter::SCROLL_DOWN ) :
                            scale = 1.0 / 0.9;
                            break;
                        default :
                            break;
                    }

                    if( scale != 1.0 )
                    {
                        osg::MatrixTransform* transform =
                            dynamic_cast<osg::MatrixTransform*>(
                                findNearestTransform( ev->getNodePath() )
                            );
                        if( transform )
                        {
                            transform->setMatrix(
                                osg::translate( -localPosition ) *
                                osg::scale( osg::dvec3( scale, scale, scale ) ) *
                                osg::translate( localPosition ) *
                                transform->getMatrix()
                            );
                        }
                    }

                    break;
                }
            case( osgGA::GUIEventAdapter::PUSH ) :
                {
                    dc->_dragging = ( ea->getButtonMask() ==
                                      osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON );
                    if( dc->_dragging )
                    {
                        osg::dvec3 localPosition;
                        if( widget->computeExtentsPositionInLocalCoordinates(
                                ev,
                                ea,
                                localPosition
                            ) )

                        {
                            dc->_previousPosition = localPosition;
                        }
                    }
                    break;
                }
            case( osgGA::GUIEventAdapter::DRAG ) :
                {
                    if( dc->_dragging )
                    {
                        osg::MatrixTransform* transform =
                            dynamic_cast<osg::MatrixTransform*>(
                                findNearestTransform( ev->getNodePath() )
                            );
                        if( transform )
                        {
                            osg::dvec3 position;
                            if( widget->computeExtentsPositionInLocalCoordinates(
                                    ev,
                                    ea,
                                    position,
                                    false
                                ) )
                            {
                                osg::dvec3 delta = position - _previousPosition;
                                osg::MatrixTransform* mt =
                                    transform->asMatrixTransform();
                                mt->setMatrix( osg::translate( delta ) *
                                               mt->getMatrix() );
                                // OSG_NOTICE<<"Move to local "<<position<<",
                                // "<<position-_previousPosition<<std::endl;
                            }
                        }
                        else
                        {
                            OSG_NOTICE << "Failed to drag, No Transform to move"
                                       << std::endl;
                        }
                    }
                    break;
                }
            case( osgGA::GUIEventAdapter::RELEASE ) :
                dc->_dragging = false;
            default :
                break;
        }
    }
    return false;
}
