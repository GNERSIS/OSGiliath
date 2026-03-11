/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ReaderWriterTRANS, derived from ReaderWriter.
 * Provides: supportsExtension, className, readObject, readNode, readNode,
 * REGISTER_OSGPLUGIN.
 */
/* file:        src/osgPlugins/trans/ReaderWriterTRANS.cpp
 * author:      Mike Weiblen http://mew.cx/ 2004-07-15
 * copyright:   (C) 2004 Michael Weiblen
 * license:     OpenSceneGraph Public License (OSGPL)
 */

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <osgDB/registry/Registry.hpp>
#include <stdio.h>

#define EXTENSION_NAME "trans"

static bool
getFilenameAndParams( const std::string& input,
                      std::string&       filename,
                      std::string&       params )
{
    // find the start of the params list, accounting for nesting of [] and () brackets,
    // note, we are working backwards.
    int                    noNestedBrackets = 0;
    std::string::size_type pos              = input.size();
    for( ; pos > 0; )
    {
        --pos;
        char c = input[pos];
        if( c == ']' )
        {
            ++noNestedBrackets;
        }
        else if( c == '[' )
        {
            --noNestedBrackets;
        }
        else if( c == ')' )
        {
            ++noNestedBrackets;
        }
        else if( c == '(' )
        {
            --noNestedBrackets;
        }
        else if( c == '.' && noNestedBrackets == 0 )
        {
            break;
        }
    }

    // get the next "extension", which actually contains the pseudo-loader parameters
    params = input.substr( pos + 1, std::string::npos );
    if( params.empty() )
    {
        OSG_WARN << "Missing parameters for " EXTENSION_NAME " pseudo-loader"
                 << std::endl;
        return false;
    }

    // clear the params string of any brackets.
    std::string::size_type params_pos = params.size();
    for( ; params_pos > 0; )
    {
        --params_pos;
        char c = params[params_pos];
        if( c == ']' || c == '[' || c == ')' || c == '(' )
        {
            params.erase( params_pos, 1 );
        }
    }

    // strip the "params extension", which must leave a sub-filename.
    filename = input.substr( 0, pos );

    return true;
}

///////////////////////////////////////////////////////////////////////////

/**
 * An OSG reader plugin for the ".trans" pseudo-loader, which inserts a
 * translation transform above the loaded geometry.
 * This pseudo-loader make it simple to change the origin of a saved model
 * by specifying a correcting translation as part of the filename.
 *
 * Usage: <modelfile.ext>.<tx>,<ty>,<tz>.trans
 * where:
 *      <modelfile.ext> = an model filename.
 *      <tx> = translation along the X axis.
 *      <ty> = translation along the Y axis.
 *      <tz> = translation along the Z axis.
 *
 * example: osgviewer cow.osg.25,0,0.trans cessna.osg
 */

class ReaderWriterTRANS : public osgDB::ReaderWriter
{
    public:

        ReaderWriterTRANS()
        {
            supportsExtension( EXTENSION_NAME, "Translation Pseudo loader." );
        }

        virtual const char*
        className() const
        {
            return "translation pseudo-loader";
        }

        virtual ReadResult
        readObject( const std::string&                  fileName,
                    const osgDB::ReaderWriter::Options* options ) const
        {
            return readNode( fileName, options );
        }

        virtual ReadResult
        readNode( const std::string&                  fileName,
                  const osgDB::ReaderWriter::Options* options ) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension( fileName );
            if( !acceptsExtension( ext ) )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            OSG_INFO << "ReaderWriterTRANS( \"" << fileName << "\" )" << std::endl;

            // strip the pseudo-loader extension
            std::string tmpName = osgDB::getNameLessExtension( fileName );

            if( tmpName.empty() )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            std::string subFileName, params;
            if( !getFilenameAndParams( tmpName, subFileName, params ) )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            if( subFileName.empty() )
            {
                OSG_WARN << "Missing subfilename for " EXTENSION_NAME " pseudo-loader"
                         << std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }

            OSG_INFO << " params = \"" << params << "\"" << std::endl;
            OSG_INFO << " subFileName = \"" << subFileName << "\"" << std::endl;

            float tx, ty, tz;
            int   count = sscanf( params.c_str(), "%f,%f,%f", &tx, &ty, &tz );
            if( count != 3 )
            {
                OSG_WARN << "Bad parameters for " EXTENSION_NAME " pseudo-loader: \""
                         << params << "\"" << std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }

            // recursively load the subfile.
            osg::ref_ptr<osg::Node> node =
                osgDB::readRefNodeFile( subFileName, options );
            if( !node )
            {
                // propagate the read failure upwards
                OSG_WARN << "Subfile \"" << subFileName << "\" could not be loaded"
                         << std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }

            osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform;
            xform->setDataVariance( osg::Object::DataVariance::STATIC );
            xform->setMatrix( osg::translate( static_cast<double>( tx ),
                                              static_cast<double>( ty ),
                                              static_cast<double>( tz ) ) );
            xform->addChild( node );
            return xform;
        }
};

// Add ourself to the Registry to instantiate the reader/writer.
REGISTER_OSGPLUGIN( trans,
                    ReaderWriterTRANS )

/*EOF*/
