/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Legacy .osg format wrapper. Maps class names to read/write
 * functions for the deprecated ASCII scene format.
 */
#pragma once

#include <osg/core/Object.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osgDB/serialization/Input.hpp>
#include <osgDB/serialization/Output.hpp>
#include <string>
#include <vector>

namespace osgDB
{

    /** deprecated. */
    class OSGDB_EXPORT DotOsgWrapper : public osg::Referenced
    {
        public:

            typedef std::vector<std::string> Associates;
            typedef bool                     ( *ReadFunc )( osg::Object&,
                                                            osgDB::Input& );
            typedef bool                     ( *WriteFunc )( const osg::Object&,
                                                             osgDB::Output& );

            enum ReadWriteMode
            {
                READ_AND_WRITE,
                READ_ONLY,
            };

            DotOsgWrapper( osg::Object*       proto,
                           const std::string& name,
                           const std::string& associates,
                           ReadFunc           readFunc,
                           WriteFunc          writeFunc,
                           ReadWriteMode      readWriteMode = READ_AND_WRITE );

            inline const osg::Object*
            getPrototype() const
            {
                return _prototype.get();
            }

            inline const std::string&
            getName() const
            {
                return _name;
            }

            inline const Associates&
            getAssociates() const
            {
                return _associates;
            }

            inline ReadFunc
            getReadFunc() const
            {
                return _readFunc;
            }

            inline WriteFunc
            getWriteFunc() const
            {
                return _writeFunc;
            }

            inline ReadWriteMode
            getReadWriteMode() const
            {
                return _readWriteMode;
            }

        protected:

            /// protected to prevent inappropriate creation of wrappers.
            DotOsgWrapper()
            {
            }

            /// protected to prevent inappropriate creation of wrappers.
            DotOsgWrapper( DotOsgWrapper& ) :
                osg::Referenced()
            {
            }

            /// protected to prevent wrapper being created on stack.
            virtual ~DotOsgWrapper()
            {
            }

            osg::ref_ptr<osg::Object> _prototype;

            std::string               _name;
            Associates                _associates;

            ReadFunc                  _readFunc;
            WriteFunc                 _writeFunc;

            ReadWriteMode             _readWriteMode;
    };

    /** deprecated. */
    class OSGDB_EXPORT DeprecatedDotOsgWrapperManager : public osg::Referenced
    {
        public:

            DeprecatedDotOsgWrapperManager()
            {
            }

            void
            addDotOsgWrapper( DotOsgWrapper* wrapper );
            void
            removeDotOsgWrapper( DotOsgWrapper* wrapper );

            osg::Object*
            readObjectOfType( const osg::Object& compObj,
                              Input&             fr );
            osg::Object*
            readObjectOfType( const basic_type_wrapper& btw,
                              Input&                    fr );

            osg::Object*
            readObject( Input& fr );
            osg::Image*
            readImage( Input& fr );
            osg::Drawable*
            readDrawable( Input& fr );
            osg::Uniform*
            readUniform( Input& fr );
            osg::StateAttribute*
            readStateAttribute( Input& fr );
            osg::Node*
            readNode( Input& fr );
            osg::Shader*
            readShader( Input& fr );

            bool
                                           writeObject( const osg::Object& obj,
                                                        Output&            fw );

            typedef std::list<std::string> FileNames;
            bool
            getLibraryFileNamesToTry( const std::string& name,
                                      FileNames&         fileNames );

        private:

            virtual ~DeprecatedDotOsgWrapperManager()
            {
            }

            typedef std::map<std::string, osg::ref_ptr<DotOsgWrapper>> DotOsgWrapperMap;

            osg::Object*
            readObject( DotOsgWrapperMap& dowMap,
                        Input&            fr );
            void
                             eraseWrapper( DotOsgWrapperMap& wrappermap,
                                           DotOsgWrapper*    wrapper );

            DotOsgWrapperMap _objectWrapperMap;
            DotOsgWrapperMap _imageWrapperMap;
            DotOsgWrapperMap _drawableWrapperMap;
            DotOsgWrapperMap _stateAttrWrapperMap;
            DotOsgWrapperMap _uniformWrapperMap;
            DotOsgWrapperMap _nodeWrapperMap;
            DotOsgWrapperMap _shaderWrapperMap;

            DotOsgWrapperMap _classNameWrapperMap;
    };

    /** deprecated. */
    class OSGDB_EXPORT RegisterDotOsgWrapperProxy
    {
        public:

            RegisterDotOsgWrapperProxy( osg::Object*                 proto,
                                        const std::string&           name,
                                        const std::string&           associates,
                                        DotOsgWrapper::ReadFunc      readFunc,
                                        DotOsgWrapper::WriteFunc     writeFunc,
                                        DotOsgWrapper::ReadWriteMode readWriteMode =
                                            DotOsgWrapper::READ_AND_WRITE );

            ~RegisterDotOsgWrapperProxy();

        protected:

            osg::ref_ptr<DotOsgWrapper> _wrapper;
    };

    /** deprecated. */
    template<class T>
    class TemplateRegisterDotOsgWrapperProxy : public RegisterDotOsgWrapperProxy,
                                               public T
    {
        public:

            TemplateRegisterDotOsgWrapperProxy(
                osg::Object*                 proto,
                const std::string&           name,
                const std::string&           associates,
                DotOsgWrapper::ReadFunc      readFunc,
                DotOsgWrapper::WriteFunc     writeFunc,
                DotOsgWrapper::ReadWriteMode readWriteMode =
                    DotOsgWrapper::READ_AND_WRITE
            ) :
                RegisterDotOsgWrapperProxy( proto,
                                            name,
                                            associates,
                                            readFunc,
                                            writeFunc,
                                            readWriteMode )
            {
            }
    };

#define REGISTER_DOTOSGWRAPPER( classname )                                  \
    extern "C" void dotosgwrapper_##classname( void )                        \
    {                                                                        \
    }                                                                        \
    static osgDB::RegisterDotOsgWrapperProxy dotosgwrapper_proxy_##classname

}
