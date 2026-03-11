/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgunittests example application
 */
#include "MultiThreadRead.hpp"
#include "performance.hpp"
#include "UnitTestFramework.hpp"

#include <iostream>
#include <osg/core/ApplicationUsage.hpp>
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/io_utils.hpp>
#include <osg/core/Timer.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/threading/Thread.hpp>
#include <osg/traversal/Polytope.hpp>

extern void
runFileNameUtilsTest( osg::ArgumentParser& arguments );

void
testFrustum( double left,
             double right,
             double bottom,
             double top,
             double zNear,
             double zFar )
{
    osg::dmat4 f;
    f               = osg::frustum( left, right, bottom, top, zNear, zFar );

    double c_left   = 0;
    double c_right  = 0;
    double c_top    = 0;
    double c_bottom = 0;
    double c_zNear  = 0;
    double c_zFar   = 0;

    std::cout << "testFrustum"
              << osg::getFrustum( f, c_left, c_right, c_bottom, c_top, c_zNear, c_zFar )
              << std::endl;
    std::cout << "  left = " << left << " compute " << c_left << std::endl;
    std::cout << "  right = " << right << " compute " << c_right << std::endl;

    std::cout << "  bottom = " << bottom << " compute " << c_bottom << std::endl;
    std::cout << "  top = " << top << " compute " << c_top << std::endl;

    std::cout << "  zNear = " << zNear << " compute " << c_zNear << std::endl;
    std::cout << "  zFar = " << zFar << " compute " << c_zFar << std::endl;

    std::cout << std::endl;
}

void
testOrtho( double left,
           double right,
           double bottom,
           double top,
           double zNear,
           double zFar )
{
    osg::dmat4 f;
    f               = osg::ortho( left, right, bottom, top, zNear, zFar );

    double c_left   = 0;
    double c_right  = 0;
    double c_top    = 0;
    double c_bottom = 0;
    double c_zNear  = 0;
    double c_zFar   = 0;

    std::cout << "testOrtho "
              << osg::getOrtho( f, c_left, c_right, c_bottom, c_top, c_zNear, c_zFar )
              << std::endl;
    std::cout << "  left = " << left << " compute " << c_left << std::endl;
    std::cout << "  right = " << right << " compute " << c_right << std::endl;

    std::cout << "  bottom = " << bottom << " compute " << c_bottom << std::endl;
    std::cout << "  top = " << top << " compute " << c_top << std::endl;

    std::cout << "  zNear = " << zNear << " compute " << c_zNear << std::endl;
    std::cout << "  zFar = " << zFar << " compute " << c_zFar << std::endl;

    std::cout << std::endl;
}

void
testPerspective( double fovy,
                 double aspect,
                 double zNear,
                 double zFar )
{
    osg::dmat4 f;
    f               = osg::perspective( fovy, aspect, zNear, zFar );

    double c_fovy   = 0;
    double c_aspect = 0;
    double c_zNear  = 0;
    double c_zFar   = 0;

    std::cout << "testPerspective "
              << osg::getPerspective( f, c_fovy, c_aspect, c_zNear, c_zFar )
              << std::endl;
    std::cout << "  fovy = " << fovy << " compute " << c_fovy << std::endl;
    std::cout << "  aspect = " << aspect << " compute " << c_aspect << std::endl;

    std::cout << "  zNear = " << zNear << " compute " << c_zNear << std::endl;
    std::cout << "  zFar = " << zFar << " compute " << c_zFar << std::endl;

    std::cout << std::endl;
}

void
testLookAt( const osg::vec3& eye,
            const osg::vec3& center,
            const osg::vec3& up )
{
    osg::dmat4 mv;
    mv = osg::lookAt( eye, center, up );

    osg::dvec3 c_eye, c_center, c_up;
    osg::getLookAt( mv, c_eye, c_center, c_up );

    std::cout << "testLookAt" << std::endl;
    std::cout << "  eye " << eye << " compute " << c_eye << std::endl;
    std::cout << "  center " << center << " compute " << c_center << std::endl;
    std::cout << "  up " << up << " compute " << c_up << std::endl;

    std::cout << std::endl;
}

void
testMatrixInvert( const osg::dmat4& matrix )
{
    // Invert it twice using the two inversion functions and view the results
    osg::notify( osg::NOTICE ) << "testMatrixInvert(" << std::endl;
    osg::notify( osg::NOTICE ) << matrix << std::endl;
    osg::notify( osg::NOTICE ) << ")" << std::endl;

    osg::dmat4 invM1_0;
    invM1_0 = osg::inverse( matrix );
    osg::notify( osg::NOTICE ) << "dmat4::invert" << std::endl;
    osg::notify( osg::NOTICE ) << invM1_0 << std::endl;
    osg::dmat4 default_result = matrix * invM1_0;
    osg::notify( osg::NOTICE ) << "matrix * invert=" << std::endl;
    osg::notify( osg::NOTICE ) << default_result << std::endl;
    ;
}

void
sizeOfTest()
{
    std::cout << "sizeof(bool)==" << sizeof( bool ) << std::endl;
    std::cout << "sizeof(char)==" << sizeof( char ) << std::endl;
    std::cout << "sizeof(short)==" << sizeof( short ) << std::endl;
    std::cout << "sizeof(short int)==" << sizeof( short int ) << std::endl;
    std::cout << "sizeof(int)==" << sizeof( int ) << std::endl;
    std::cout << "sizeof(long)==" << sizeof( long ) << std::endl;
    std::cout << "sizeof(long int)==" << sizeof( long int ) << std::endl;

#if defined( _MSC_VER )
    // long long isn't supported on VS6.0...
    std::cout << "sizeof(__int64)==" << sizeof( __int64 ) << std::endl;
#else
    std::cout << "sizeof(long long)==" << sizeof( long long ) << std::endl;
#endif
    std::cout << "sizeof(float)==" << sizeof( float ) << std::endl;
    std::cout << "sizeof(double)==" << sizeof( double ) << std::endl;

    std::cout << "sizeof(std::istream::pos_type)==" << sizeof( std::istream::pos_type )
              << std::endl;
    std::cout << "sizeof(std::istream::off_type)==" << sizeof( std::istream::off_type )
              << std::endl;
    std::cout << "sizeof(std::mutex)==" << sizeof( std::mutex ) << std::endl;

    std::cout << "sizeof(std::string)==" << sizeof( std::string ) << std::endl;
}

/// Exercise the dmat4.getRotate function.
/// Compare the output of:
///  q1 * q2
/// versus
///  (mat(q1)*mat(q2)*scale).getRotate()
/// for a range of rotations
void
testGetQuatFromMatrix( const osg::dvec3& scale )
{

    // Options

    // acceptable error range
    double     eps = 1E-6;

    // scale matrix
    // To not test with scale, use 1,1,1
    // Not sure if 0's or negative values are acceptable
    osg::dmat4 scalemat;
    scalemat = osg::scale( scale );

    // range of rotations
#if 1
    // wide range
    double rol1start = 0.0;
    double rol1stop  = 360.0;
    double rol1step  = 20.0;

    double pit1start = 0.0;
    double pit1stop  = 90.0;
    double pit1step  = 20.0;

    double yaw1start = 0.0;
    double yaw1stop  = 360.0;
    double yaw1step  = 20.0;

    double rol2start = 0.0;
    double rol2stop  = 360.0;
    double rol2step  = 20.0;

    double pit2start = 0.0;
    double pit2stop  = 90.0;
    double pit2step  = 20.0;

    double yaw2start = 0.0;
    double yaw2stop  = 360.0;
    double yaw2step  = 20.0;
#else
    // focused range
    double rol1start = 0.0;
    double rol1stop  = 0.0;
    double rol1step  = 0.1;

    double pit1start = 0.0;
    double pit1stop  = 5.0;
    double pit1step  = 5.0;

    double yaw1start = 89.0;
    double yaw1stop  = 91.0;
    double yaw1step  = 0.1;

    double rol2start = 0.0;
    double rol2stop  = 0.0;
    double rol2step  = 0.1;

    double pit2start = 0.0;
    double pit2stop  = 0.0;
    double pit2step  = 0.1;

    double yaw2start = 89.0;
    double yaw2stop  = 91.0;
    double yaw2step  = 0.1;
#endif

    std::cout << std::endl
              << "Starting testGetQuatFromMatrix, it can take a while ..." << std::endl;

    osg::Timer_t tstart, tstop;
    tstart    = osg::Timer::instance()->tick();
    int count = 0;
    for( double rol1 = rol1start; rol1 <= rol1stop; rol1 += rol1step )
    {
        for( double pit1 = pit1start; pit1 <= pit1stop; pit1 += pit1step )
        {
            for( double yaw1 = yaw1start; yaw1 <= yaw1stop; yaw1 += yaw1step )
            {
                for( double rol2 = rol2start; rol2 <= rol2stop; rol2 += rol2step )
                {
                    for( double pit2 = pit2start; pit2 <= pit2stop; pit2 += pit2step )
                    {
                        for( double yaw2  = yaw2start; yaw2 <= yaw2stop;
                             yaw2        += yaw2step )
                        {
                            count++;
                            // create two quats based on the roll, pitch and yaw values
                            osg::dquat rot_quat1 = osg::dquat( osg::radians( rol1 ),
                                                               osg::dvec3( 1, 0, 0 ) ) *
                                                   osg::dquat( osg::radians( pit1 ),
                                                               osg::dvec3( 0, 1, 0 ) ) *
                                                   osg::dquat( osg::radians( yaw1 ),
                                                               osg::dvec3( 0, 0, 1 ) );

                            osg::dquat rot_quat2 = osg::dquat( osg::radians( rol2 ),
                                                               osg::dvec3( 1, 0, 0 ) ) *
                                                   osg::dquat( osg::radians( pit2 ),
                                                               osg::dvec3( 0, 1, 0 ) ) *
                                                   osg::dquat( osg::radians( yaw2 ),
                                                               osg::dvec3( 0, 0, 1 ) );

                            // create an output quat using quaternion math
                            osg::dquat out_quat1;
                            out_quat1 = rot_quat2 * rot_quat1;

                            // create two matrices based on the input quats
                            osg::dmat4 mat1, mat2;
                            mat1 = osg::rotate( rot_quat1 );
                            mat2 = osg::rotate( rot_quat2 );

                            // create an output quat by matrix multiplication and
                            // getRotate
                            osg::dmat4 out_mat;
                            out_mat = mat2 * mat1;
                            // add matrix scale for even more nastiness
                            out_mat = out_mat * scalemat;
                            osg::dquat out_quat2;
                            out_quat2 = osg::getRotate( out_mat );

                            // If the quaternion W is <0, osg::vec3(then we should
                            // reflect to get it into the positive W. Unfortunately, when
                            // W is very small (close to 0)), the sign does not really
                            // make sense because of precision problems and the
                            // reflection might not work.
                            if( out_quat1.w < 0 )
                            {
                                out_quat1 = out_quat1 * -1.0;
                            }
                            if( out_quat2.w < 0 )
                            {
                                out_quat2 = out_quat2 * -1.0;
                            }

                            // if the output quat length is not one
                            // or if the components do not match,
                            // something is amiss

                            bool componentsOK = false;
                            if( ( ( fabs( out_quat1.x - out_quat2.x ) ) < eps ) &&
                                ( ( fabs( out_quat1.y - out_quat2.y ) ) < eps ) &&
                                ( ( fabs( out_quat1.z - out_quat2.z ) ) < eps ) &&
                                ( ( fabs( out_quat1.w - out_quat2.w ) ) < eps ) )
                            {
                                componentsOK = true;
                            }
                            // We should also test for q = -q which is valid, so reflect
                            // one quat.
                            out_quat2 = out_quat2 * -1.0;
                            if( ( ( fabs( out_quat1.x - out_quat2.x ) ) < eps ) &&
                                ( ( fabs( out_quat1.y - out_quat2.y ) ) < eps ) &&
                                ( ( fabs( out_quat1.z - out_quat2.z ) ) < eps ) &&
                                ( ( fabs( out_quat1.w - out_quat2.w ) ) < eps ) )
                            {
                                componentsOK = true;
                            }

                            bool lengthOK = false;
                            if( fabs( 1.0 - osg::length( out_quat2 ) ) < eps )
                            {
                                lengthOK = true;
                            }

                            if( !lengthOK || !componentsOK )
                            {
                                std::cout << "testGetQuatFromMatrix problem at: \n"
                                          << " r1=" << rol1 << " p1=" << pit1
                                          << " y1=" << yaw1 << " r2=" << rol2
                                          << " p2=" << pit2 << " y2=" << yaw2 << "\n";
                                std::cout << "quats:        " << osg::quat( out_quat1 )
                                          << " length: " << osg::length( out_quat1 )
                                          << "\n";
                                std::cout << "mats and get: " << osg::quat( out_quat2 )
                                          << " length: " << osg::length( out_quat2 )
                                          << "\n\n";
                            }
                        }
                    }
                }
            }
        }
    }
    tstop           = osg::Timer::instance()->tick();
    double duration = osg::Timer::instance()->delta_s( tstart, tstop );
    std::cout << "Time for testGetQuatFromMatrix with " << count
              << " iterations: " << duration << std::endl
              << std::endl;
}

void
testQuatRotate( const osg::dvec3& from,
                const osg::dvec3& to )
{
    osg::quat q_nicolas;
    q_nicolas = osg::quat( osg::vec3( from ), osg::vec3( to ) );

    osg::quat q_original;
    q_original = osg::quat( osg::vec3( from ), osg::vec3( to ) );

    std::cout << "osg::quat::makeRotate(" << from << ", " << to << ")" << std::endl;
    std::cout << "  q_nicolas = " << q_nicolas << std::endl;
    std::cout << "  q_original = " << q_original << std::endl;
    std::cout << "  from * M4x4(q_nicolas) = "
              << from * osg::dmat4( osg::rotate( q_nicolas ) ) << std::endl;
    std::cout << "  from * M4x4(q_original) = "
              << from * osg::dmat4( osg::rotate( q_original ) ) << std::endl;
}

void
testQuat( const osg::dvec3& quat_scale )
{
    osg::quat q1;
    q1 = osg::quat( osg::radians( 30.0 ), osg::vec3( 0.0F, 0.0F, 1.0F ) );

    osg::quat q2;
    q2              = osg::quat( osg::radians( 133.0 ), osg::vec3( 0.0F, 1.0F, 1.0F ) );

    osg::quat  q1_2 = q1 * q2;
    osg::quat  q2_1 = q2 * q1;

    osg::dmat4 m1   = osg::dmat4( osg::rotate( q1 ) );
    osg::dmat4 m2   = osg::dmat4( osg::rotate( q2 ) );

    osg::dmat4 m1_2 = m1 * m2;
    osg::dmat4 m2_1 = m2 * m1;

    osg::quat  qm1_2;
    qm1_2 = osg::quat( osg::getRotate( m1_2 ) );

    osg::quat qm2_1;
    qm2_1 = osg::quat( osg::getRotate( m2_1 ) );

    std::cout << "q1*q2 = " << q1_2 << std::endl;
    std::cout << "q2*q1 = " << q2_1 << std::endl;
    std::cout << "m1*m2 = " << qm1_2 << std::endl;
    std::cout << "m2*m1 = " << qm2_1 << std::endl;

    testQuatRotate( osg::dvec3( 1.0, 0.0, 0.0 ), osg::dvec3( 0.0, 1.0, 0.0 ) );
    testQuatRotate( osg::dvec3( 0.0, 1.0, 0.0 ), osg::dvec3( 1.0, 0.0, 0.0 ) );
    testQuatRotate( osg::dvec3( 0.0, 0.0, 1.0 ), osg::dvec3( 0.0, 1.0, 0.0 ) );
    testQuatRotate( osg::dvec3( 1.0, 1.0, 1.0 ), osg::dvec3( 1.0, 0.0, 0.0 ) );
    testQuatRotate( osg::dvec3( 1.0, 0.0, 0.0 ), osg::dvec3( 1.0, 0.0, 0.0 ) );
    testQuatRotate( osg::dvec3( 1.0, 0.0, 0.0 ), osg::dvec3( -1.0, 0.0, 0.0 ) );
    testQuatRotate( osg::dvec3( -1.0, 0.0, 0.0 ), osg::dvec3( 1.0, 0.0, 0.0 ) );
    testQuatRotate( osg::dvec3( 0.0, 1.0, 0.0 ), osg::dvec3( 0.0, -1.0, 0.0 ) );
    testQuatRotate( osg::dvec3( 0.0, -1.0, 0.0 ), osg::dvec3( 0.0, 1.0, 0.0 ) );
    testQuatRotate( osg::dvec3( 0.0, 0.0, 1.0 ), osg::dvec3( 0.0, 0.0, -1.0 ) );
    testQuatRotate( osg::dvec3( 0.0, 0.0, -1.0 ), osg::dvec3( 0.0, 0.0, 1.0 ) );

    // Test a range of rotations
    testGetQuatFromMatrix( quat_scale );

    // This is a specific test case for a matrix containing scale and rotation
    osg::dmat4 matrix( 0.5,
                       0.0,
                       0.0,
                       0.0,
                       0.0,
                       0.5,
                       0.0,
                       0.0,
                       0.0,
                       0.0,
                       0.5,
                       0.0,
                       1.0,
                       1.0,
                       1.0,
                       1.0 );

    osg::quat  quat( osg::getRotate( matrix ) );

    osg::notify( osg::NOTICE ) << "dmat4 = " << matrix << "rotation = " << quat
                               << ", expected quat = (0,0,0,1)" << std::endl;
}

void
testDecompose()
{
    double    angx = osg::radians( 30.0 );
    double    angy = osg::radians( 30.0 );
    double    angz = osg::radians( 30.0 );

    osg::quat qx, qy, qz;
    qx                 = osg::quat( angx, osg::vec3( osg::vec3( 1.0F, 0.0F, 0.0F ) ) );
    qy                 = osg::quat( angy, osg::vec3( osg::vec3( 0.0F, 1.0F, 0.0F ) ) );
    qz                 = osg::quat( angz, osg::vec3( osg::vec3( 0.0F, 0.0F, 1.0F ) ) );

    osg::quat rotation = qx * qy * qz;

    osg::mat4 matf;
    matf = osg::rotate( rotation );

    printf( "Test - dmat4::decompose(), input rotation  : %f %f %f %f\n",
            rotation.x,
            rotation.y,
            rotation.z,
            rotation.w );

    osg::vec3 transf;
    osg::quat rotf;
    osg::vec3 sclf;
    osg::quat sof;
    osg::decompose( matf, transf, rotf, sclf, sof );
    printf( "mat4::decomposef\n" );
    printf( "Translation      : %f %f %f\n", transf.x, transf.y, transf.z );
    printf( "Rotation         : %f %f %f %f\n", rotf.x, rotf.y, rotf.z, rotf.w );
    printf( "Scale            : %f %f %f\n", sclf.x, sclf.y, sclf.z );
    printf( "Scale Orientation: %f %f %f %f\n", sof.x, sof.y, sof.z, sof.w );

    osg::dmat4 matd;
    matd = osg::dmat4( osg::rotate( rotation ) );

    osg::dvec3 transd;
    osg::dquat rotd;
    osg::dvec3 scld;
    osg::dquat sod;
    osg::decompose( matd, transd, rotd, scld, sod );
    printf( "dmat4::decompose\n" );
    printf( "Translation      : %f %f %f\n", transd.x, transd.y, transd.z );
    printf( "Rotation         : %f %f %f %f\n", rotd.x, rotd.y, rotd.z, rotd.w );
    printf( "Scale            : %f %f %f\n", scld.x, scld.y, scld.z );
    printf( "Scale Orientation: %f %f %f %f\n", sod.x, sod.y, sod.z, sod.w );

    osg::notify( osg::NOTICE ) << std::endl;
}

class MyThread : public osg::Thread
{
    public:

        void
        run( void )
        {
        }
};

class NotifyThread : public osg::Thread
{
    public:

        NotifyThread( osg::NotifySeverity level,
                      const std::string&  message ) :
            _done( false ),
            _level( level ),
            _message( message )
        {
        }

        ~NotifyThread()
        {
            _done = true;
            if( isRunning() )
            {
                cancel();
                join();
            }
        }

        void
        run( void )
        {
            std::cout << "Entering thread ..." << _message << std::endl;

            unsigned int count = 0;

            while( !_done )
            {
                ++count;
#if 1
                osg::notify( _level ) << _message << this << "\n";
#else
                osg::notify( _level ) << _message << this << std::endl;
#endif
            }

            std::cout << "Leaving thread ..." << _message << " count=" << count
                      << std::endl;
        }

        bool                _done;
        osg::NotifySeverity _level;
        std::string         _message;
};

void
testThreadInitAndExit()
{
    std::cout << "******   Running thread start and delete test   ****** " << std::endl;

    {
        MyThread thread;
        thread.startThread();
    }

    // add a sleep to allow the thread start to fall over it its going to.
    osg::Thread::microSleep( 500'000 );

    std::cout << "pass    thread start and delete test" << std::endl << std::endl;

    std::cout << "******   Running notify thread test   ****** " << std::endl;

    {
        NotifyThread thread1( osg::INFO, "thread one:" );
        NotifyThread thread2( osg::INFO, "thread two:" );
        NotifyThread thread3( osg::INFO, "thread three:" );
        NotifyThread thread4( osg::INFO, "thread four:" );
        thread1.startThread();
        thread2.startThread();
        thread3.startThread();
        thread4.startThread();

        // add a sleep to allow the thread start to fall over it its going to.
        osg::Thread::microSleep( 5'000'000 );
    }

    std::cout << "pass    noitfy thread test." << std::endl << std::endl;
}

void
testPolytope()
{
    osg::Polytope pt;
    pt.setToBoundingBox( osg::box( -1'000, -1'000, -1'000, 1'000, 1'000, 1'000 ) );
    bool bContains = pt.contains( osg::vec3( 0, 0, 0 ) );
    if( bContains )
    {
        std::cout << "Polytope pt.contains(osg::vec3(0, 0, 0)) has succeeded."
                  << std::endl;
    }
    else
    {
        std::cout << "Polytope pt.contains(osg::vec3(0, 0, 0)) has failed." << std::endl;
    }
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() + " is the example which runs units tests."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options]"
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption( "qt",
                                                           "Display qualified tests." );
    arguments.getApplicationUsage()->addCommandLineOption(
        "quat",
        "Display extended quaternion tests."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "quat_scaled sx sy sz",
        "Display extended quaternion tests of pre scaled matrix."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "sizeof",
                                                           "Display sizeof tests." );
    arguments.getApplicationUsage()->addCommandLineOption( "matrix",
                                                           "Display qualified tests." );
    arguments.getApplicationUsage()->addCommandLineOption( "performance",
                                                           "Display qualified tests." );
    arguments.getApplicationUsage()->addCommandLineOption(
        "read-threads <numthreads>",
        "Run multi-thread reading test."
    );

    if( arguments.argc() <= 1 )
    {
        arguments.getApplicationUsage()->write(
            std::cout,
            osg::ApplicationUsage::COMMAND_LINE_OPTION
        );
        return 1;
    }

    bool printQualifiedTest = false;
    while( arguments.read( "qt" ) )
    {
        printQualifiedTest = true;
    }

    bool printMatrixTest = false;
    while( arguments.read( "matrix" ) )
    {
        printMatrixTest = true;
    }

    bool printSizeOfTest = false;
    while( arguments.read( "sizeof" ) )
    {
        printSizeOfTest = true;
    }

    bool printFileNameUtilsTests = false;
    while( arguments.read( "filenames" ) )
    {
        printFileNameUtilsTests = true;
    }

    bool printQuatTest = false;
    while( arguments.read( "quat" ) )
    {
        printQuatTest = true;
    }

    int numReadThreads = 0;
    while( arguments.read( "read-threads", numReadThreads ) )
    {
    }

    bool printPolytopeTest = false;
    while( arguments.read( "polytope" ) )
    {
        printPolytopeTest = true;
    }

    bool doTestThreadInitAndExit = false;
    while( arguments.read( "thread" ) )
    {
        doTestThreadInitAndExit = true;
    }

    osg::dvec3 quat_scale( 1.0, 1.0, 1.0 );
    while( arguments.read( "quat_scaled", quat_scale.x, quat_scale.y, quat_scale.z ) )
    {
        printQuatTest = true;
    }

    bool performanceTest = false;
    while( arguments.read( "p" ) || arguments.read( "performance" ) )
    {
        performanceTest = true;
    }

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        std::cout << arguments.getApplicationUsage()->getCommandLineUsage() << std::endl;
        arguments.getApplicationUsage()->write(
            std::cout,
            arguments.getApplicationUsage()->getCommandLineOptions()
        );
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if( arguments.errors() )
    {
        arguments.writeErrorMessages( std::cout );
        return 1;
    }

    if( printQuatTest )
    {
        testQuat( quat_scale );
    }

    if( printMatrixTest )
    {
        std::cout << "******   Running matrix tests   ******" << std::endl;

        testFrustum( -1, 1, -1, 1, 1, 1'000 );
        testFrustum( 0, 1, 1, 2, 2.5, 100'000 );

        testOrtho( 0, 1, 1, 2, 2.1, 1'000 );
        testOrtho( -1, 10, 1, 20, 2.5, 100'000 );

        testPerspective( 20, 1, 1, 1'000 );
        testPerspective( 90, 2, 1, 1'000 );

        testLookAt( osg::vec3( 10.0, 4.0, 2.0 ),
                    osg::vec3( 10.0, 4.0, 2.0 ) + osg::vec3( 0.0, 1.0, 0.0 ),
                    osg::vec3( 0.0, 0.0, 1.0 ) );
        testLookAt( osg::vec3( 10.0, 4.0, 2.0 ),
                    osg::vec3( 10.0, 4.0, 2.0 ) + osg::vec3( 1.0, 1.0, 0.0 ),
                    osg::vec3( 0.0, 0.0, 1.0 ) );

        testMatrixInvert( osg::dmat4( 0.999848,
                                      -0.002700,
                                      0.017242,
                                      -0.1715,
                                      0,
                                      0.987960,
                                      0.154710,
                                      0.207295,
                                      -0.017452,
                                      -0.154687,
                                      0.987809,
                                      -0.98239,
                                      0,
                                      0,
                                      0,
                                      1 ) );

        testMatrixInvert( osg::dmat4( 0.999848,
                                      -0.002700,
                                      0.017242,
                                      0.0,
                                      0.0,
                                      0.987960,
                                      0.154710,
                                      0.0,
                                      -0.017452,
                                      -0.154687,
                                      0.987809,
                                      0.0,
                                      -0.1715,
                                      0.207295,
                                      -0.98239,
                                      1.0 ) );

        testDecompose();
    }

    if( printSizeOfTest )
    {
        std::cout << "**** sizeof() tests  ******" << std::endl;

        sizeOfTest();

        std::cout << std::endl;
    }

    if( performanceTest )
    {
        std::cout << "**** performance tests  ******" << std::endl;

        runPerformanceTests();
    }

    if( numReadThreads > 0 )
    {
        runMultiThreadReadTests( numReadThreads, arguments );
        return 0;
    }

    if( printPolytopeTest )
    {
        testPolytope();
    }

    if( printQualifiedTest )
    {
        std::cout << "*****   Qualified Tests  ******" << std::endl;

        osgUtx::QualifiedTestPrinter printer;
        osgUtx::TestGraph::instance().root()->accept( printer );
        std::cout << std::endl;
    }

    if( printFileNameUtilsTests )
    {
        runFileNameUtilsTest( arguments );
    }

    if( doTestThreadInitAndExit )
    {
        testThreadInitAndExit();
    }

    std::cout << "******   Running tests   ******" << std::endl;

    // Global Data or Context
    osgUtx::TestContext ctx;
    osgUtx::TestRunner  runner( ctx );
    runner.specify( "root" );

    osgUtx::TestGraph::instance().root()->accept( runner );

    return 0;
}
