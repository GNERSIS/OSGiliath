/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: Inherit.
 */
#include "PythonScriptEngine.hpp"

using namespace python;

PythonScriptEngine::PythonScriptEngine() :
    Inherit( "python" ),
    _py_main( 0 )
{
    initialize();
}

PythonScriptEngine::PythonScriptEngine( const PythonScriptEngine& /*rhs*/,
                                        const osg::CopyOp& /*copyop*/ ) :
    Inherit( "python" ),
    _py_main( 0 )
{
    initialize();
}

PythonScriptEngine::~PythonScriptEngine()
{
    Py_Finalize();
}

void
PythonScriptEngine::initialize()
{
    Py_InitializeEx( 0 );

    _py_main = PyModule_GetDict( PyImport_AddModule( "__main__" ) );
}

bool
PythonScriptEngine::run( osg::Script*       script,
                         const std::string& entryPoint,
                         osg::Parameters&   inputParameters,
                         osg::Parameters&   outputParameters )
{
    if( !script || !_py_main )
    {
        return false;
    }

    PyObject* r =
        PyRun_String( script->getScript().c_str(), Py_file_input, _py_main, _py_main );

    if( !r )
    {
        r = PyErr_Occurred();

        if( r )
        {
            PyErr_Print();
            PyErr_Clear();
        }
    }

    return true;
}
