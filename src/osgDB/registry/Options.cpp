/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Read/write options passed to plugins. Carries file path lists,
 * option strings, database path, and callbacks.
 */
#include <osgDB/registry/Options.hpp>

#include <osgDB/registry/Registry.hpp>

using namespace osgDB;

Options::Options() :
    Inherit( true ),
    _objectCacheHint( CACHE_ARCHIVES ),
    _precisionHint( FLOAT_PRECISION_ALL ),
    _buildKdTreesHint( NO_PREFERENCE )
{
}

Options::Options( const std::string& str ) :
    Inherit( true ),
    _str( str ),
    _objectCacheHint( CACHE_ARCHIVES ),
    _precisionHint( FLOAT_PRECISION_ALL ),
    _buildKdTreesHint( NO_PREFERENCE )
{
    parsePluginStringData( str );
}

Options::Options( const Options&     options,
                  const osg::CopyOp& copyop ) :
    Inherit( options,
             copyop ),
    _str( options._str ),
    _databasePaths( options._databasePaths ),
    _objectCacheHint( options._objectCacheHint ),
    _objectCache( options._objectCache ),
    _precisionHint( options._precisionHint ),
    _buildKdTreesHint( options._buildKdTreesHint ),
    _pluginData( options._pluginData ),
    _pluginStringData( options._pluginStringData ),
    _findFileCallback( options._findFileCallback ),
    _readFileCallback( options._readFileCallback ),
    _writeFileCallback( options._writeFileCallback ),
    _fileLocationCallback( options._fileLocationCallback ),
    _fileCache( options._fileCache ),
    _terrain( options._terrain ),
    _parentGroup( options._parentGroup )
{
}

Options::~Options()
{
}

void
Options::parsePluginStringData( const std::string& str,
                                char               separator1,
                                char               separator2 )
{
    StringList valueList;
    split( str, valueList, separator1 );
    if( valueList.size() > 0 )
    {
        StringList keyAndValue;
        for( StringList::iterator itr = valueList.begin(); itr != valueList.end();
             ++itr )
        {
            split( *itr, keyAndValue, separator2 );
            if( keyAndValue.size() > 1 )
            {
                setPluginStringData( keyAndValue.front(), keyAndValue.back() );
            }
            else if( keyAndValue.size() > 0 )
            {
                setPluginStringData( keyAndValue.front(), "true" );
            }
            keyAndValue.clear();
        }
    }
}

bool
Options::operator<( const Options& rhs ) const
{
    // TODO add better compare
    // OSG_DEBUG << "comparing <'" << _str << "' with '" << rhs._str << "'" << std::endl;
    return _str.compare( rhs._str ) < 0;
}

bool
Options::operator==( const Options& rhs ) const
{
    // TODO add better compare
    // OSG_DEBUG << "comparing == '" << _str << "' with '" << rhs._str << "'" <<
    // std::endl;
    return _str.compare( rhs._str ) == 0;
}
