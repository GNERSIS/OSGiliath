/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * PythonScriptEngine, derived from osg.
 * Provides: OSG_REGISTER_TYPE, getLanguage, run, getPythonMain.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/traversal/ScriptEngine.hpp>
#include <Python.h>

namespace python
{

    class PythonScriptEngine : public osg::Inherit<osg::ScriptEngine, PythonScriptEngine>
    {
        public:

            OSG_REGISTER_TYPE( lua,
                               PythonScriptEngine )

            PythonScriptEngine();
            PythonScriptEngine( const PythonScriptEngine& rhs,
                                const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

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

            /** get the Python main object.*/
            PyObject*
            getPythonMain()
            {
                return _py_main;
            }

        protected:

            void
            initialize();

            virtual ~PythonScriptEngine();

            PyObject* _py_main;
    };

}
