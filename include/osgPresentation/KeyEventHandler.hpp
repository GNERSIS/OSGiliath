/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Keyboard event handler for presentation control.
 * Maps keys to presentation actions (forward, back, home, quit).
 */
#pragma once

#include <osg/state/Point.hpp>
#include <osg/state/StateSet.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgPresentation/SlideEventHandler.hpp>

namespace osgPresentation
{

    class OSGPRESENTATION_EXPORT KeyEventHandler : public osgGA::GUIEventHandler
    {
        public:

            KeyEventHandler( int                        key,
                             osgPresentation::Operation operation,
                             const JumpData&            jumpData = JumpData() );
            KeyEventHandler( int                        key,
                             const std::string&         str,
                             osgPresentation::Operation operation,
                             const JumpData&            jumpData = JumpData() );
            KeyEventHandler( int                                 key,
                             const osgPresentation::KeyPosition& keyPos,
                             const JumpData&                     jumpData = JumpData() );

            void
            setKey( int key )
            {
                _key = key;
            }

            int
            getKey() const
            {
                return _key;
            }

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

            int                          _key;

            std::string                  _command;
            osgPresentation::KeyPosition _keyPos;
            osgPresentation::Operation   _operation;

            JumpData                     _jumpData;
    };

}
