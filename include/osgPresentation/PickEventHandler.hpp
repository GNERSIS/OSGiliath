/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Pick (click) event handler for interactive slide elements.
 * Routes click events to embedded URLs, animations, or scripts.
 */
#pragma once

#include <osg/state/Point.hpp>
#include <osg/state/StateSet.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgPresentation/SlideEventHandler.hpp>

namespace osgPresentation
{

    class OSGPRESENTATION_EXPORT PickEventHandler : public osgGA::GUIEventHandler
    {
        public:

            PickEventHandler( osgPresentation::Operation operation,
                              const JumpData&            jumpData = JumpData() );
            PickEventHandler( const std::string&         str,
                              osgPresentation::Operation operation,
                              const JumpData&            jumpData = JumpData() );
            PickEventHandler( const osgPresentation::KeyPosition& keyPos,
                              const JumpData& jumpData = JumpData() );

            void
            setOperation( osgPresentation::Operation operation )
            {
                _operation = operation;
            }

            osgPresentation::Operation
            getOperation() const
            {
                return _operation;
            }

            void
            setCommand( const std::string& str )
            {
                _command = str;
            }

            const std::string&
            getCommand() const
            {
                return _command;
            }

            void
            setKeyPosition( const osgPresentation::KeyPosition& keyPos )
            {
                _keyPos = keyPos;
            }

            const osgPresentation::KeyPosition&
            getKeyPosition() const
            {
                return _keyPos;
            }

            void
            setJumpData( const JumpData& jumpData )
            {
                _jumpData = jumpData;
            }

            const JumpData&
            getJumpData() const
            {
                return _jumpData;
            }

            virtual bool
            handle( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter&      aa,
                    osg::Object*                  object,
                    osg::NodeVisitor*             nv );

            virtual void
            getUsage( osg::ApplicationUsage& usage ) const;

            void
                                         doOperation();

            std::string                  _command;
            osgPresentation::KeyPosition _keyPos;
            osgPresentation::Operation   _operation;

            JumpData                     _jumpData;
            std::set<osg::Drawable*>     _drawablesOnPush;
    };

}
