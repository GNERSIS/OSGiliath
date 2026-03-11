// The majority of the application is dedicated to building the
// current contributors list by parsing the ChangeLog, it just takes
// one line in the main itself to report the version number.

#include <iostream>
#include <osg/core/ApplicationUsage.hpp>
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/BoundingBox.hpp>
#include <osg/maths/BoundingSphere.hpp>
#include <osg/maths/Matrix.hpp>
#include <osg/maths/Plane.hpp>
#include <osg/maths/Quat.hpp>
#include <osg/Version>
#include <set>
#include <vector>

#ifdef BUILD_CONTRIBUTORS
extern void
printContributors( const std::string& changeLog,
                   bool               printNumEntries );
#endif

using namespace std;

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options]"
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--version-number",
        "Print out version number only"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--major-number",
        "Print out major version number only"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--minor-number",
        "Print out minor version number only"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--patch-number",
        "Print out patch version number only"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--so-number ",
        "Print out shared object version number only"
    );
    // OpenThreads has been removed; options retained for backward compatibility
    arguments.getApplicationUsage()->addCommandLineOption(
        "--openthreads-version-number",
        "(deprecated, OpenThreads removed)"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--openthreads-soversion-number",
        "(deprecated, OpenThreads removed)"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "Matrix::value_type",
        "Print the value of Matrix::value_type"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "Plane::value_type",
        "Print the value of Plane::value_type"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "BoundingSphere::value_type",
        "Print the value of BoundingSphere::value_type"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "BoundingBox::value_type",
        "Print the value of BoundingBox::value_type"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "Quat::value_type",
        "Print the value of Quat::value_type"
    );

#ifdef BUILD_CONTRIBUTORS
    arguments.getApplicationUsage()->addCommandLineOption(
        "-r <file> or --read <file>",
        "Read the ChangeLog to generate an estimated contributors list."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--entries",
        "Print out number of entries into the ChangeLog file for each contributor."
    );
#endif

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        cout << arguments.getApplicationUsage()->getCommandLineUsage() << endl;
        arguments.getApplicationUsage()->write(
            cout,
            arguments.getApplicationUsage()->getCommandLineOptions()
        );
        return 1;
    }

    if( arguments.read( "--version-number" ) )
    {
        cout << osgGetVersion() << endl;
        return 0;
    }

    if( arguments.read( "--major-number" ) )
    {
        cout << OSGILIATH_MAJOR_VERSION << endl;
        return 0;
    }

    if( arguments.read( "--minor-number" ) )
    {
        cout << OSGILIATH_MINOR_VERSION << endl;
        return 0;
    }

    if( arguments.read( "--patch-number" ) )
    {
        cout << OSGILIATH_PATCH_VERSION << endl;
        return 0;
    }

    if( arguments.read( "--soversion-number" ) || arguments.read( "--so-number" ) )
    {
        cout << osgGetSOVersion() << endl;
        return 0;
    }

    if( arguments.read( "--openthreads-version-number" ) )
    {
        cout << "(OpenThreads removed, using std::thread)" << endl;
        return 0;
    }

    if( arguments.read( "--openthreads-major-number" ) )
    {
        cout << 0 << endl;
        return 0;
    }

    if( arguments.read( "--openthreads-minor-number" ) )
    {
        cout << 0 << endl;
        return 0;
    }

    if( arguments.read( "--openthreads-patch-number" ) )
    {
        cout << 0 << endl;
        return 0;
    }

    if( arguments.read( "--openthreads-soversion-number" ) )
    {
        cout << "(OpenThreads removed, using std::thread)" << endl;
        return 0;
    }

    if( arguments.read( "Matrix::value_type" ) )
    {
        cout << ( ( sizeof( osg::dmat4::value_type ) == 4 ) ? "float" : "double" )
             << endl;
        return 0;
    }

    if( arguments.read( "Plane::value_type" ) )
    {
        cout << ( ( sizeof( osg::Plane::value_type ) == 4 ) ? "float" : "double" )
             << endl;
        return 0;
    }

    if( arguments.read( "BoundingSphere::value_type" ) )
    {
        cout << ( ( sizeof( osg::sphere::value_type ) == 4 ) ? "float" : "double" )
             << endl;
        return 0;
    }

    if( arguments.read( "BoundingBox::value_type" ) )
    {
        cout << ( ( sizeof( osg::box::value_type ) == 4 ) ? "float" : "double" ) << endl;
        return 0;
    }

    if( arguments.read( "Quat::value_type" ) )
    {
        cout << ( ( sizeof( osg::quat::value_type ) == 4 ) ? "float" : "double" )
             << endl;
        return 0;
    }

    cout << osgGetLibraryName() << " " << osgGetVersion() << endl << endl;

#ifdef BUILD_CONTRIBUTORS
    string changeLog;
    while( arguments.read( "-r", changeLog ) || arguments.read( "--read", changeLog ) )
    {
        printContributors( changeLog, arguments.read( "--entries" ) );
    }
#endif

    return 0;
}
