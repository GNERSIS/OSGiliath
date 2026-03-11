/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * FileNameUtils example application
 */
#include <osgDB/io/FileNameUtils.hpp>

#include <list>
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>

void
runFileNameUtilsTest( osg::ArgumentParser& )
{
    typedef std::list<std::string> Strings;
    Strings                        strings;
    strings.push_back( std::string( "" ) );
    strings.push_back( std::string( "myfile" ) );
    strings.push_back( std::string( ".osgt" ) );
    strings.push_back( std::string( "myfile.osgt" ) );
    strings.push_back( std::string( "/myfile.osgt" ) );
    strings.push_back( std::string( "home/robert/myfile.osgt" ) );
    strings.push_back( std::string( "/home/robert/myfile.osgt" ) );
    strings.push_back( std::string( "\\myfile.osgt" ) );
    strings.push_back( std::string( "home\\robert\\myfile.osgt" ) );
    strings.push_back( std::string( "\\home\\robert\\myfile.osgt" ) );
    strings.push_back( std::string( "\\home/robert\\myfile.osgt" ) );
    strings.push_back( std::string( "\\home\\robert/myfile.osgt" ) );
    strings.push_back( std::string( "home/robert/" ) );
    strings.push_back( std::string( "\\home\\robert\\" ) );
    strings.push_back( std::string( "home/robert/myfile" ) );
    strings.push_back( std::string( "\\home\\robert\\myfile" ) );
    strings.push_back( std::string( "home/robert/.osgt" ) );
    strings.push_back( std::string( "\\home\\robert\\.osgt" ) );
    strings.push_back( std::string( "home/robert/myfile.ext.osgt" ) );
    strings.push_back( std::string( "home\\robert\\myfile.ext.osgt" ) );

    for( Strings::iterator itr = strings.begin(); itr != strings.end(); ++itr )
    {
        std::string& str = *itr;
        OSG_NOTICE << "string=" << str;
        OSG_NOTICE << "\n\tosgDB::getFilePath(str)=" << osgDB::getFilePath( str );
        OSG_NOTICE << "\n\tosgDB::getSimpleFileName(str)="
                   << osgDB::getSimpleFileName( str );
        OSG_NOTICE << "\n\tosgDB::getStrippedName(str)="
                   << osgDB::getStrippedName( str );
        OSG_NOTICE << "\n\tosgDB::getFileExtension(str)="
                   << osgDB::getFileExtension( str );
        OSG_NOTICE << "\n\tosgDB::getFileExtensionIncludingDot(str)="
                   << osgDB::getFileExtensionIncludingDot( str );
        OSG_NOTICE << "\n\tosgDB::getNameLessExtension(str)="
                   << osgDB::getNameLessExtension( str );
        OSG_NOTICE << "\n\tosgDB::getNameLessAllExtensions(str)="
                   << osgDB::getNameLessAllExtensions( str );
        OSG_NOTICE << std::endl << std::endl;
    }
}
