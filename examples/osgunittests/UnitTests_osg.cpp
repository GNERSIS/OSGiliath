/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * UnitTests_osg example application
 */
#include "UnitTestFramework.hpp"

#include <osg/maths/mat4.hpp>
#include <osg/maths/vec3.hpp>
#include <sstream>

namespace osg
{

    ///////////////////////////////////////////////////////////////////////////////
    //
    //  vec3 Tests
    //
    class Vec3TestFixture
    {
        public:

            Vec3TestFixture();

            void
            testAddition( const osgUtx::TestContext& ctx );
            void
            testSubtraction( const osgUtx::TestContext& ctx );
            void
            testScalarMultiplication( const osgUtx::TestContext& ctx );
            void
            testDotProduct( const osgUtx::TestContext& ctx );

        private:

            // Some convenience variables for use in the tests
            vec3 v1_, v2_, v3_;
    };

    Vec3TestFixture::Vec3TestFixture() :
        v1_( 1.0F,
             1.0F,
             1.0F ),
        v2_( 2.0F,
             2.0F,
             2.0F ),
        v3_( 3.0F,
             3.0F,
             3.0F )
    {
    }

    void
    Vec3TestFixture::testAddition( const osgUtx::TestContext& )
    {
        OSGUTX_TEST_F( v1_ + v2_ == v3_ )
    }

    void
    Vec3TestFixture::testSubtraction( const osgUtx::TestContext& )
    {
        OSGUTX_TEST_F( v3_ - v1_ == v2_ )
    }

    void
    Vec3TestFixture::testScalarMultiplication( const osgUtx::TestContext& )
    {
        OSGUTX_TEST_F( v1_ * 3 == v3_ )
    }

    void
    Vec3TestFixture::testDotProduct( const osgUtx::TestContext& )
    {
    }

    OSGUTX_BEGIN_TESTSUITE( vec3 )
    OSGUTX_ADD_TESTCASE( Vec3TestFixture,
                         testAddition )
    OSGUTX_ADD_TESTCASE( Vec3TestFixture,
                         testSubtraction )
    OSGUTX_ADD_TESTCASE( Vec3TestFixture,
                         testScalarMultiplication )
    OSGUTX_ADD_TESTCASE( Vec3TestFixture,
                         testDotProduct )
    OSGUTX_END_TESTSUITE

    OSGUTX_AUTOREGISTER_TESTSUITE_AT( vec3,
                                      root.osg )

    ///////////////////////////////////////////////////////////////////////////////
    //
    //  dmat4 Tests
    //
    class MatrixTestFixture
    {
        public:

            MatrixTestFixture();

            void
            testPreMultTranslate( const osgUtx::TestContext& ctx );
            void
            testPostMultTranslate( const osgUtx::TestContext& ctx );
            void
            testPreMultScale( const osgUtx::TestContext& ctx );
            void
            testPostMultScale( const osgUtx::TestContext& ctx );
            void
            testPreMultRotate( const osgUtx::TestContext& ctx );
            void
            testPostMultRotate( const osgUtx::TestContext& ctx );

        private:

            // Some convenience variables for use in the tests
            dmat4 _md;
            mat4  _mf;
            dvec3 _v3d;
            vec3  _v3;
            quat  _q1;
            quat  _q2;
            quat  _q3;
            quat  _q4;
    };

    MatrixTestFixture::MatrixTestFixture() :
        _md( 1,
             2,
             3,
             4,
             5,
             6,
             7,
             8,
             9,
             10,
             11,
             12,
             13,
             14,
             15,
             16 ),
        _mf( 1,
             2,
             3,
             4,
             5,
             6,
             7,
             8,
             9,
             10,
             11,
             12,
             13,
             14,
             15,
             16 ),
        _v3d( 1,
              2,
              3 ),
        _v3( 1,
             2,
             3 ),
        _q1( 1,
             0,
             0,
             0 ),
        _q2( 0,
             1,
             0,
             0 ),
        _q3( 0,
             0,
             1,
             0 ),
        _q4( 0,
             0,
             0,
             1 )
    {
    }

    void
    MatrixTestFixture::testPreMultTranslate( const osgUtx::TestContext& )
    {
        osg::dmat4 tdo;
        osg::dmat4 tdn;
        osg::mat4  tfo;
        osg::mat4  tfn;

        tdo = _md;
        tdn = _md;
        osg::preMult( tdo, osg::translate( _v3d ) );
        osg::preMultTranslate( tdn, _v3d );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::preMult( tdo, osg::translate( _v3 ) );
        osg::preMultTranslate( tdn, _v3 );
        OSGUTX_TEST_F( tdo == tdn )

        tfo = _mf;
        tfn = _mf;
        osg::preMult( tfo, osg::translate( _v3d ) );
        osg::preMultTranslate( tfn, _v3d );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::preMult( tfo, osg::translate( _v3 ) );
        osg::preMultTranslate( tfn, _v3 );
        OSGUTX_TEST_F( tfo == tfn )
    }

    void
    MatrixTestFixture::testPostMultTranslate( const osgUtx::TestContext& )
    {
        osg::dmat4 tdo;
        osg::dmat4 tdn;
        osg::mat4  tfo;
        osg::mat4  tfn;

        tdo = _md;
        tdn = _md;
        osg::postMult( tdo, osg::translate( _v3d ) );
        osg::postMultTranslate( tdn, _v3d );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::postMult( tdo, osg::translate( _v3 ) );
        osg::postMultTranslate( tdn, _v3 );
        OSGUTX_TEST_F( tdo == tdn )

        tfo = _mf;
        tfn = _mf;
        osg::postMult( tfo, osg::translate( _v3d ) );
        osg::postMultTranslate( tfn, _v3d );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::postMult( tfo, osg::translate( _v3 ) );
        osg::postMultTranslate( tfn, _v3 );
        OSGUTX_TEST_F( tfo == tfn )
    }

    void
    MatrixTestFixture::testPreMultScale( const osgUtx::TestContext& )
    {
        osg::dmat4 tdo;
        osg::dmat4 tdn;
        osg::mat4  tfo;
        osg::mat4  tfn;

        tdo = _md;
        tdn = _md;
        osg::preMult( tdo, osg::scale( _v3d ) );
        osg::preMultScale( tdn, _v3d );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::preMult( tdo, osg::scale( _v3 ) );
        osg::preMultScale( tdn, _v3 );
        OSGUTX_TEST_F( tdo == tdn )

        tfo = _mf;
        tfn = _mf;
        osg::preMult( tfo, osg::scale( _v3d ) );
        osg::preMultScale( tfn, _v3d );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::preMult( tfo, osg::scale( _v3 ) );
        osg::preMultScale( tfn, _v3 );
        OSGUTX_TEST_F( tfo == tfn )
    }

    void
    MatrixTestFixture::testPostMultScale( const osgUtx::TestContext& )
    {
        osg::dmat4 tdo;
        osg::dmat4 tdn;
        osg::mat4  tfo;
        osg::mat4  tfn;

        tdo = _md;
        tdn = _md;
        osg::postMult( tdo, osg::scale( _v3d ) );
        osg::postMultScale( tdn, _v3d );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::postMult( tdo, osg::scale( _v3 ) );
        osg::postMultScale( tdn, _v3 );
        OSGUTX_TEST_F( tdo == tdn )

        tfo = _mf;
        tfn = _mf;
        osg::postMult( tfo, osg::scale( _v3d ) );
        osg::postMultScale( tfn, _v3d );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::postMult( tfo, osg::scale( _v3 ) );
        osg::postMultScale( tfn, _v3 );
        OSGUTX_TEST_F( tfo == tfn )
    }

    void
    MatrixTestFixture::testPreMultRotate( const osgUtx::TestContext& )
    {
        osg::dmat4 tdo;
        osg::dmat4 tdn;
        osg::mat4  tfo;
        osg::mat4  tfn;

        tdo = _md;
        tdn = _md;
        osg::preMult( tdo, osg::rotate( _q1 ) );
        osg::preMultRotate( tdn, _q1 );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::preMult( tdo, osg::rotate( _q2 ) );
        osg::preMultRotate( tdn, _q2 );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::preMult( tdo, osg::rotate( _q3 ) );
        osg::preMultRotate( tdn, _q3 );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::preMult( tdo, osg::rotate( _q4 ) );
        osg::preMultRotate( tdn, _q4 );
        OSGUTX_TEST_F( tdo == tdn )

        tfo = _mf;
        tfn = _mf;
        osg::preMult( tfo, osg::rotate( _q1 ) );
        osg::preMultRotate( tfn, _q1 );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::preMult( tfo, osg::rotate( _q2 ) );
        osg::preMultRotate( tfn, _q2 );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::preMult( tfo, osg::rotate( _q3 ) );
        osg::preMultRotate( tfn, _q3 );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::preMult( tfo, osg::rotate( _q4 ) );
        osg::preMultRotate( tfn, _q4 );
        OSGUTX_TEST_F( tfo == tfn )
    }

    void
    MatrixTestFixture::testPostMultRotate( const osgUtx::TestContext& )
    {
        osg::dmat4 tdo;
        osg::dmat4 tdn;
        osg::mat4  tfo;
        osg::mat4  tfn;

        tdo = _md;
        tdn = _md;
        osg::postMult( tdo, osg::rotate( _q1 ) );
        osg::postMultRotate( tdn, _q1 );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::postMult( tdo, osg::rotate( _q2 ) );
        osg::postMultRotate( tdn, _q2 );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::postMult( tdo, osg::rotate( _q3 ) );
        osg::postMultRotate( tdn, _q3 );
        OSGUTX_TEST_F( tdo == tdn )

        tdo = _md;
        tdn = _md;
        osg::postMult( tdo, osg::rotate( _q4 ) );
        osg::postMultRotate( tdn, _q4 );
        OSGUTX_TEST_F( tdo == tdn )

        tfo = _mf;
        tfn = _mf;
        osg::postMult( tfo, osg::rotate( _q1 ) );
        osg::postMultRotate( tfn, _q1 );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::postMult( tfo, osg::rotate( _q2 ) );
        osg::postMultRotate( tfn, _q2 );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::postMult( tfo, osg::rotate( _q3 ) );
        osg::postMultRotate( tfn, _q3 );
        OSGUTX_TEST_F( tfo == tfn )

        tfo = _mf;
        tfn = _mf;
        osg::postMult( tfo, osg::rotate( _q4 ) );
        osg::postMultRotate( tfn, _q4 );
        OSGUTX_TEST_F( tfo == tfn )
    }

    OSGUTX_BEGIN_TESTSUITE( dmat4 )
    OSGUTX_ADD_TESTCASE( MatrixTestFixture,
                         testPreMultTranslate )
    OSGUTX_ADD_TESTCASE( MatrixTestFixture,
                         testPostMultTranslate )
    OSGUTX_ADD_TESTCASE( MatrixTestFixture,
                         testPreMultScale )
    OSGUTX_ADD_TESTCASE( MatrixTestFixture,
                         testPostMultScale )
    OSGUTX_ADD_TESTCASE( MatrixTestFixture,
                         testPreMultRotate )
    OSGUTX_ADD_TESTCASE( MatrixTestFixture,
                         testPostMultRotate )
    OSGUTX_END_TESTSUITE

    OSGUTX_AUTOREGISTER_TESTSUITE_AT( dmat4,
                                      root.osg )

}
