/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ReaderWriterdxf, derived from ReaderWriter.
 * Provides: supportsExtension, supportsOption, supportsOption, supportsOption,
 * supportsOption, supportsOption.
 */
#include "dxfFile.hpp"
#include "DXFWriterNodeVisitor.hpp"

#include <iostream>
#include <map>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <sstream>
#include <string.h>
#include <string>
#include <utility>

using namespace osg;
using namespace osgDB;
using namespace std;

class ReaderWriterdxf : public osgDB::ReaderWriter
{
    public:

        ReaderWriterdxf()
        {
            supportsExtension( "dxf", "Autodesk DXF format" );

            supportsOption( "UTF8", "Assuming UTF8 encoding of dxf text" );
            supportsOption( "UTF16", "Assuming UTF16 encoding of dxf text" );
            supportsOption( "UTF32", "Assuming UTF32 encoding of dxf text" );
            supportsOption( "SIGNATURE",
                            "Determine encoding of dxf text from it's signative" );
            supportsOption(
                "WideChar | CurrentCodePage",
                "Determine encoding of dxf text using CurrentCodePage (Windows only.)"
            );
            supportsOption( "FontFile=<fontfile>", "Set the font file for dxf text" );
        }

        virtual const char*
        className() const
        {
            return "Autodesk DXF Reader/Writer";
        }

        virtual ReadResult
        readObject( const std::string&                  fileName,
                    const osgDB::ReaderWriter::Options* options ) const
        {
            return readNode( fileName, options );
        }

        virtual ReadResult
        readNode( const std::string& fileName,
                  const osgDB::ReaderWriter::Options* ) const;

        virtual WriteResult
        writeObject( const osg::Object& obj,
                     const std::string& fileName,
                     const Options*     options = NULL ) const
        {
            const osg::Node* node = dynamic_cast<const osg::Node*>( &obj );
            if( node )
            {
                return writeNode( *node, fileName, options );
            }
            else
            {
                return WriteResult( WriteResult::FILE_NOT_HANDLED );
            }
        }

        virtual WriteResult
        writeObject( const osg::Object& obj,
                     std::ostream&      fout,
                     const Options*     options = NULL ) const
        {
            const osg::Node* node = dynamic_cast<const osg::Node*>( &obj );
            if( node )
            {
                return writeNode( *node, fout, options );
            }
            else
            {
                return WriteResult( WriteResult::FILE_NOT_HANDLED );
            }
        }

        virtual WriteResult
        writeNode( const osg::Node& node,
                   std::ostream&    fout,
                   const Options* = NULL ) const
        {

            DXFWriterNodeVisitor nv( fout );

            node.accept(
                static_cast<osg::ConstNodeVisitor&>( nv )
            );    // first pass is to get all node names and types -> layers

            if( nv.writeHeader( node.getBound() ) )
            {
                node.accept(
                    static_cast<osg::ConstNodeVisitor&>( nv )
                );    // second pass outputs data
                nv.writeFooter();
            }

            return WriteResult( WriteResult::FILE_SAVED );
        }

        virtual WriteResult
        writeNode( const osg::Node&   node,
                   const std::string& fileName,
                   const Options* /*options*/ = NULL ) const
        {
            if( !acceptsExtension( osgDB::getFileExtension( fileName ) ) )
            {
                return WriteResult( WriteResult::FILE_NOT_HANDLED );
            }

            osgDB::ofstream f( fileName.c_str() );

            if( !f.is_open() )
            {
                return WriteResult( WriteResult::ERROR_IN_WRITING_FILE );
            }
            DXFWriterNodeVisitor nv( f );

            node.accept(
                static_cast<osg::ConstNodeVisitor&>( nv )
            );    // first pass is to get all node names and types -> layers

            if( nv.writeHeader( node.getBound() ) )
            {
                node.accept(
                    static_cast<osg::ConstNodeVisitor&>( nv )
                );    // second pass outputs data
                nv.writeFooter();
            }

            return WriteResult( WriteResult::FILE_SAVED );
        }

    protected:
};

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN( dxf,
                    ReaderWriterdxf )

// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult
ReaderWriterdxf::readNode( const std::string&                  filename,
                           const osgDB::ReaderWriter::Options* options ) const
{
    std::string ext = osgDB::getFileExtension( filename );
    if( !acceptsExtension( ext ) )
    {
        return ReadResult::FILE_NOT_HANDLED;
    }

    // extract accuracy options if available
    if( options )
    {
        bool useAccuracy = false;    // if we specify accuracy of curve rendering or not
        double maxError =
            0.0;      // if useAccuracy - the accuracy (max deviation) from the arc
        bool improveAccuracyOnly =
            false;    // if true only use the given accuracy if it would improve the
                      // curve compared to the previous implementation Thus you can
                      // ensure that large curves get rendered better but small ones
                      // don't get worse

        std::string optionsstring = options->getOptionString();

        size_t      accstart      = optionsstring.find( "Accuracy(" );
        if( accstart != std::string::npos )
        {
            const char* start = optionsstring.c_str() + accstart + strlen( "Accuracy(" );
            if( sscanf( start, "%lf", &maxError ) == 1 )
            {
                useAccuracy = true;
            }
        }
        if( useAccuracy )
        {
            // Option to only use the new accuracy code when it would improve on the
            // accuracy of the old method
            if( optionsstring.find( "ImproveAccuracyOnly" ) != std::string::npos )
            {
                improveAccuracyOnly = true;
            }
            // Pull out the initial dxfArc copy from the registry and set accuracy there.
            // When actual dxfArcs/Circles are created they will inherit these parameters
            // from the exemplar
            dxfEntity::getRegistryEntity( "ARC" )->setAccuracy( true,
                                                                maxError,
                                                                improveAccuracyOnly );
            dxfEntity::getRegistryEntity( "CIRCLE" )
                ->setAccuracy( true, maxError, improveAccuracyOnly );
        }    // accuracy options exists

        {
            std::istringstream iss( options->getOptionString() );
            std::string        opt;
            while( iss >> opt )
            {
                // split opt into pre= and post=
                std::string pre_equals;
                std::string post_equals;

                size_t      found = opt.find( "=" );
                if( found != std::string::npos )
                {
                    pre_equals  = opt.substr( 0, found );
                    post_equals = opt.substr( found + 1 );
                }
                else
                {
                    pre_equals = opt;
                }

                if( pre_equals == "FontFile" )
                {
                    std::string fontFile = post_equals.c_str();
                    if( !fontFile.empty() )
                    {
                        dynamic_cast<dxfText*>( dxfEntity::getRegistryEntity( "TEXT" ) )
                            ->font = fontFile;

                        OSG_INFO << "ReaderWriteDXF : Set fontFile to " << fontFile
                                 << std::endl;
                    }
                    else
                    {
                        OSG_NOTICE << "Warning: invalid FontFile value: " << post_equals
                                   << std::endl;
                    }
                }
                else if( pre_equals == "UTF8" )
                {
                    dynamic_cast<dxfText*>( dxfEntity::getRegistryEntity( "TEXT" ) )
                        ->encoding = osgText::String::ENCODING_UTF8;
                    OSG_INFO << "ReaderWriteDXF : Set encoding to "
                                "osgText::String::ENCODING_UTF8"
                             << std::endl;
                }
                else if( pre_equals == "UTF16" )
                {
                    dynamic_cast<dxfText*>( dxfEntity::getRegistryEntity( "TEXT" ) )
                        ->encoding = osgText::String::ENCODING_UTF16;
                    OSG_INFO << "ReaderWriteDXF : Set encoding to "
                                "osgText::String::ENCODING_UTF16"
                             << std::endl;
                }
                else if( pre_equals == "UTF32" )
                {
                    dynamic_cast<dxfText*>( dxfEntity::getRegistryEntity( "TEXT" ) )
                        ->encoding = osgText::String::ENCODING_UTF32;
                    OSG_INFO << "ReaderWriteDXF : Set encoding to "
                                "osgText::String::ENCODING_UTF32"
                             << std::endl;
                }
                else if( pre_equals == "SIGNATURE" )
                {
                    dynamic_cast<dxfText*>( dxfEntity::getRegistryEntity( "TEXT" ) )
                        ->encoding = osgText::String::ENCODING_SIGNATURE;
                    OSG_INFO << "ReaderWriteDXF : Set encoding to "
                                "osgText::String::ENCODING_SIGNATURE"
                             << std::endl;
                }
                else if( pre_equals == "WideChar" || pre_equals == "CurrentCodePage" )
                {
                    dynamic_cast<dxfText*>( dxfEntity::getRegistryEntity( "TEXT" ) )
                        ->encoding = osgText::String::ENCODING_CURRENT_CODE_PAGE;
                    OSG_INFO << "ReaderWriteDXF : Set encoding to "
                                "osgText::String::ENCODING_CURRENT_CODE_PAGE"
                             << std::endl;
                }
            }
        }
    }    // options exist

    // Open
    dxfFile df( filename );
    if( df.parseFile() )
    {
        // convert to OSG
        osg::Group* osg_top = df.dxf2osg();
        return ( osg_top );
    }
    return ReadResult::FILE_NOT_HANDLED;
}
