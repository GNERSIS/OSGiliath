/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ReaderWriterSDL, derived from ReaderWriter.
 * Provides: supportsExtension, className, readObject, REGISTER_OSGPLUGIN.
 */
#include "JoystickDevice.hpp"

#include <osg/core/Notify.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/registry/Registry.hpp>

class ReaderWriterSDL : public osgDB::ReaderWriter
{
    public:

        ReaderWriterSDL()
        {
            supportsExtension( "sdl", "SDL Device Integration" );
        }

        virtual const char*
        className() const
        {
            return "SDL Device Integration plugin";
        }

        virtual ReadResult
        readObject( const std::string& file,
                    const osgDB::ReaderWriter::Options* = NULL ) const
        {
            if( file == "joystick.sdl" )
            {
                return new JoystickDevice;
            }
            return ReadResult::FILE_NOT_HANDLED;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN( sdl,
                    ReaderWriterSDL )
