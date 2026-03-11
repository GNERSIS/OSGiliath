/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Legacy .osg format output helper. Provides indented writing
 * for the deprecated ASCII scene format writer.
 */
#pragma once

#include <map>
#include <osg/core/Object.hpp>
#include <osgDB/io/fstream.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <string>

namespace osgDB
{

    /** deprecated. */
    class OSGDB_EXPORT Output : public osgDB::ofstream
    {
        public:

            Output();
            Output( const char* name );

            virtual ~Output();

            void
            setOptions( const Options* options );

            const Options*
            getOptions() const
            {
                return _options.get();
            }

            void
            setWriteOutDefaultValues( bool flag )
            {
                _writeOutDefaultValues = flag;
            }

            bool
            getWriteOutDefaultValues() const
            {
                return _writeOutDefaultValues;
            }

            void
            open( const char* name );

            // comment out temporarily to avoid compilation problems, RO Jan 2002.
            // void open(const char *name,int mode);

            Output&
            indent();

            /** wrap a string with "" quotes and use \" for any internal quotes.*/
            std::string
            wrapString( const char* str );

            /** wrap a string with "" quotes and use \" for any internal quotes.*/
            std::string
            wrapString( const std::string& str );

            inline void
            setIndentStep( int step )
            {
                _indentStep = step;
            }

            inline int
            getIndentStep() const
            {
                return _indentStep;
            }

            inline void
            setIndent( int indent )
            {
                _indent = indent;
            }

            inline int
            getIndent() const
            {
                return _indent;
            }

            inline void
            setNumIndicesPerLine( int num )
            {
                _numIndicesPerLine = num;
            }

            inline int
            getNumIndicesPerLine() const
            {
                return _numIndicesPerLine;
            }

            void
            moveIn();
            void
            moveOut();

            virtual bool
            writeObject( const osg::Object& obj );
            virtual void
            writeBeginObject( const std::string& name );
            virtual void
            writeEndObject();
            virtual void
            writeUseID( const std::string& id );
            virtual void
            writeUniqueID( const std::string& id );

            bool
            getUniqueIDForObject( const osg::Object* obj,
                                  std::string&       uniqueID );
            bool
            createUniqueIDForObject( const osg::Object* obj,
                                     std::string&       uniqueID );
            bool
            registerUniqueIDForObject( const osg::Object* obj,
                                       std::string&       uniqueID );

            enum PathNameHint
            {
                AS_IS,
                FULL_PATH,
                RELATIVE_PATH,
                FILENAME_ONLY,
            };

            inline void
            setPathNameHint( const PathNameHint pnh )
            {
                _pathNameHint = pnh;
            }

            inline PathNameHint
            getPathNameHint() const
            {
                return _pathNameHint;
            }

            virtual std::string
            getFileNameForOutput( const std::string& filename ) const;

            const std::string&
            getFileName() const
            {
                return _filename;
            }

            // Set and get if export texture files during write
            void
            setOutputTextureFiles( bool flag )
            {
                _outputTextureFiles = flag;
            }

            bool
            getOutputTextureFiles() const
            {
                return _outputTextureFiles;
            }

            // support code for OutputTextureFiles
            virtual std::string
            getTextureFileNameForOutput();

            void
            setOutputShaderFiles( bool flag )
            {
                _outputShaderFiles = flag;
            }

            bool
            getOutputShaderFiles() const
            {
                return _outputShaderFiles;
            }

            virtual std::string
            getShaderFileNameForOutput();

            void
            setExternalFileWritten( const std::string& filename,
                                    bool               hasBeenWritten = true );
            bool
            getExternalFileWritten( const std::string& filename ) const;

        protected:

            virtual void
                                                              init();

            osg::ref_ptr<const Options>                       _options;

            int                                               _indent;
            int                                               _indentStep;

            int                                               _numIndicesPerLine;

            typedef std::map<const osg::Object*, std::string> UniqueIDToLabelMapping;
            UniqueIDToLabelMapping                            _objectToUniqueIDMap;

            std::string                                       _filename;

            PathNameHint                                      _pathNameHint;

            bool                                              _outputTextureFiles;
            unsigned int                                      _textureFileNameNumber;

            bool                                              _outputShaderFiles;
            unsigned int                                      _shaderFileNameNumber;

            bool                                              _writeOutDefaultValues;

            typedef std::map<std::string, bool>               ExternalFileWrittenMap;
            ExternalFileWrittenMap                            _externalFileWritten;
    };

}
