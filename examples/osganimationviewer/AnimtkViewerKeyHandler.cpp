/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * AnimtkViewerKeyHandler example application
 */
#include "AnimtkViewerKeyHandler.hpp"

AnimtkKeyEventHandler::AnimtkKeyEventHandler()
{
    _actionKeys[List] = 'l';
    _actionKeys[Help] = 'h';
    _actionKeys[Play] = 'p';
    _actionKeys[Next] = ']';
    _actionKeys[Prev] = '[';
}

void
AnimtkKeyEventHandler::printUsage() const
{
    std::cout << ( char )_actionKeys.find( Help )->second << " for Help" << std::endl;
    std::cout << ( char )_actionKeys.find( List )->second << " for List" << std::endl;
    std::cout << ( char )_actionKeys.find( Play )->second << " for Play" << std::endl;
    std::cout << ( char )_actionKeys.find( Next )->second << " for select Next item"
              << std::endl;
    std::cout << ( char )_actionKeys.find( Prev )->second << " for select Previous item"
              << std::endl;
}

bool
AnimtkKeyEventHandler::handle( const osgGA::GUIEventAdapter& ea,
                               osgGA::GUIActionAdapter&,
                               osg::Object*,
                               osg::NodeVisitor* )
{
    AnimtkViewerModelController& mc = AnimtkViewerModelController::instance();
    if( ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN )
    {
        if( ea.getKey() == _actionKeys[List] )
        {
            return mc.list();
        }
        else if( ea.getKey() == _actionKeys[Play] )
        {
            return mc.play();
        }
        else if( ea.getKey() == _actionKeys[Next] )
        {
            return mc.next();
        }
        else if( ea.getKey() == _actionKeys[Prev] )
        {
            return mc.previous();
        }
        else if( ea.getKey() == _actionKeys[Help] )
        {
            printUsage();
            return true;
        }
    }

    return false;
}
