/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract scripting engine interface. Provides the base for
 * Lua, Python, or JavaScript bindings to scene graph objects.
 */
#pragma once

#include <osg/core/Callback.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/core/UserDataContainer.hpp>
#include <osg/traversal/NodeVisitor.hpp>

namespace osg
{

    // forward declare
    class ScriptEngine;

    /* Script class for wrapping a script and the language used in the script.*/
    class Script : public osg::Inherit<osg::Object, Script>
    {
        public:

            Script() :
                _modifiedCount( 0 )
            {
            }

            Script( const std::string& language,
                    const std::string& str ) :
                _language( language ),
                _script( str ),
                _modifiedCount( 0 )
            {
            }

            Script( const Script&      rhs,
                    const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                osg::Inherit<osg::Object,
                             Script>( rhs,
                                      copyop ),
                _language( rhs._language ),
                _script( rhs._script ),
                _modifiedCount( 0 )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Script )

            void
            setLanguage( const std::string& language )
            {
                _language = language;
                dirty();
            }

            const std::string&
            getLanguage() const
            {
                return _language;
            }

            void
            setScript( const std::string& str )
            {
                _script = str;
                dirty();
            }

            const std::string&
            getScript() const
            {
                return _script;
            }

            void
            dirty()
            {
                ++_modifiedCount;
            }

            unsigned int
            getModifiedCount() const
            {
                return _modifiedCount;
            }

        protected:

            virtual ~Script()
            {
            }

            std::string  _language;
            std::string  _script;
            unsigned int _modifiedCount;
    };

    /** NodeCallback for attaching a script to a NodeCallback so that it can be called as
     * an update or event callback.*/
    class OSG_EXPORT ScriptNodeCallback
        : public osg::Inherit<osg::NodeCallback, ScriptNodeCallback>
    {
        public:

            ScriptNodeCallback( Script*            script     = 0,
                                const std::string& entryPoint = "" ) :
                _script( script ),
                _entryPoint( entryPoint )
            {
            }

            ScriptNodeCallback( const ScriptNodeCallback& rhs,
                                const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( rhs,
                         copyop ),
                _script( rhs._script )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               ScriptNodeCallback )

            /** Set the script to call.*/
            void
            setScript( osg::Script* script )
            {
                _script = script;
            }

            /** Get the script to call.*/
            osg::Script*
            getScript()
            {
                return _script.get();
            }

            /** Get the script to call.*/
            const osg::Script*
            getScript() const
            {
                return _script.get();
            }

            /** Set the entry point to call.*/
            void
            setEntryPoint( const std::string& script )
            {
                _entryPoint = script;
            }

            /** Get the script to call.*/
            const std::string&
            getEntryPoint() const
            {
                return _entryPoint;
            }

            /** find the ScriptEngine from looking at the UserDataContainers of nodes in
             * scene graph above the ScriptCallback.*/
            osg::ScriptEngine*
            getScriptEngine( osg::NodePath& nodePath );

            /** NodeCallback method, calls the Script.*/
            virtual void
            operator()( osg::Node*        node,
                        osg::NodeVisitor* nv );

        protected:

            virtual ~ScriptNodeCallback()
            {
            }

            osg::ref_ptr<Script> _script;
            std::string          _entryPoint;
    };

    /** ScriptEngine base class for integrating different scripting languages.
     *  Concrete ScriptEngine's are provided by osgDB::readFile<ScriptEngine> */
    class ScriptEngine : public osg::Object
    {
        public:

            /** get the scripting language supported by the ScriptEngine.*/
            inline const std::string&
            getLanguage() const
            {
                return _language;
            }

            /** run a Script.*/
            bool
            run( osg::Script* script )
            {
                // assumpt empty input and output parameters lists
                Parameters inputParameters, outputParameters;
                return run( script, "", inputParameters, outputParameters );
            }

            /** run a Script.*/
            virtual bool
            run( osg::Script*       script,
                 const std::string& entryPoint,
                 Parameters&        inputParameters,
                 Parameters&        outputParameters ) = 0;

        protected:

            ScriptEngine( const std::string& language ) :
                _language( language )
            {
                setName( language );
            }

            virtual ~ScriptEngine()
            {
            }

            std::string _language;
    };

}
