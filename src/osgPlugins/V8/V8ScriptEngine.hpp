/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * V8ScriptEngine, derived from osg.
 * Provides: OSG_REGISTER_TYPE, getLanguage, run, getIsolate.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/traversal/ScriptEngine.hpp>
#include <v8.h>

namespace v8
{

    class V8ScriptEngine : public osg::Inherit<osg::ScriptEngine, V8ScriptEngine>
    {
        public:

            OSG_REGISTER_TYPE( v8,
                               V8ScriptEngine )

            V8ScriptEngine();
            V8ScriptEngine( const V8ScriptEngine& rhs,
                            const osg::CopyOp&    copyop = osg::CopyOp::SHALLOW_COPY );

            virtual const std::string&
            getLanguage() const
            {
                return _language;
            }

            /** run a Script.*/
            virtual bool
            run( osg::Script*       script,
                 const std::string& entryPoint,
                 osg::Parameters&   inputParameters,
                 osg::Parameters&   outputParameters );

            v8::Isolate*
            getIsolate()
            {
                return _isolate;
            }

        protected:

            void
            initialize();

            virtual ~V8ScriptEngine();

            v8::Isolate*                       _isolate;
            v8::Persistent<v8::Context>        _globalContext;
            v8::Persistent<v8::ObjectTemplate> _globalTemplate;
    };

}
