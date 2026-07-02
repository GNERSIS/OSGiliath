/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Lua scripting integration for osgWidget.
 * Binds widget events to Lua callback functions.
 */
// Code by: Jeremy Moles (cubicool) 2007-2008

#pragma once

#include <osgWidget/WindowManager.hpp>

namespace osgWidget
{

    // Externally defined; does this work in Windows?
    struct LuaEngineData;

    // The actual Engine itself. Every attempt is made to export the implementation into
    // the source file, rather than having it here.
    class OSGWIDGET_EXPORT LuaEngine : public ScriptEngine
    {
        public:

            LuaEngine( WindowManager* = 0 );

            bool
            initialize();
            bool
            close();
            bool
            eval( const std::string& );
            bool
            runFile( const std::string& );

        protected:

            LuaEngineData* _data;
            WindowManager* _wm;
    };

}
