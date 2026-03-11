/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osguserdata example application
 */
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/core/io_utils.hpp>
#include <osg/core/UserDataContainer.hpp>
#include <osg/core/ValueObject.hpp>
#include <osg/maths/compat.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgViewer/core/Viewer.hpp>

namespace MyNamespace
{

    /** Provide an simple example of customizing the default UserDataContainer.*/
    class MyUserDataContainer
        : public osg::Inherit<osg::DefaultUserDataContainer, MyUserDataContainer>
    {
        public:

            OSG_REGISTER_TYPE( MyNamespace,
                               MyUserDataContainer )

            MyUserDataContainer()
            {
            }

            MyUserDataContainer( const MyUserDataContainer& udc,
                                 const osg::CopyOp&         copyop =
                                     osg::CopyOp::SHALLOW_COPY ) :
                Inherit( udc,
                         copyop )
            {
            }

            virtual Object*
            getUserObject( unsigned int i )
            {
                OSG_NOTICE << "MyUserDataContainer::getUserObject(" << i << ")"
                           << std::endl;
                return osg::DefaultUserDataContainer::getUserObject( i );
            }

            virtual const Object*
            getUserObject( unsigned int i ) const
            {
                OSG_NOTICE << "MyUserDataContainer::getUserObject(" << i << ") const"
                           << std::endl;
                return osg::DefaultUserDataContainer::getUserObject( i );
            }

        protected:

            virtual ~MyUserDataContainer()
            {
            }
    };

}

/** Provide basic example of providing serialization support for the
 * MyUserDataContainer.*/
REGISTER_OBJECT_WRAPPER(
    MyUserDataContainer,
    new MyNamespace::MyUserDataContainer,
    MyNamespace::MyUserDataContainer,
    "osg::Object osg::UserDataContainer osg::DefaultUserDataContainer "
    "MyNamespace::MyUserDataContainer"
)
{
}

class MyGetValueVisitor : public osg::ValueObject::GetValueVisitor
{
    public:

        virtual void
        apply( bool value )
        {
            OSG_NOTICE << " bool " << value;
        }

        virtual void
        apply( char value )
        {
            OSG_NOTICE << " char " << value;
        }

        virtual void
        apply( unsigned char value )
        {
            OSG_NOTICE << " uchar " << value;
        }

        virtual void
        apply( short value )
        {
            OSG_NOTICE << " short " << value;
        }

        virtual void
        apply( unsigned short value )
        {
            OSG_NOTICE << " ushort " << value;
        }

        virtual void
        apply( int value )
        {
            OSG_NOTICE << " int " << value;
        }

        virtual void
        apply( unsigned int value )
        {
            OSG_NOTICE << " uint " << value;
        }

        virtual void
        apply( float value )
        {
            OSG_NOTICE << " float " << value;
        }

        virtual void
        apply( double value )
        {
            OSG_NOTICE << " double " << value;
        }

        virtual void
        apply( const std::string& value )
        {
            OSG_NOTICE << " string " << value;
        }

        virtual void
        apply( const osg::vec2& value )
        {
            OSG_NOTICE << " vec2 " << value;
        }

        virtual void
        apply( const osg::vec3& value )
        {
            OSG_NOTICE << " vec3 " << value;
        }

        virtual void
        apply( const osg::vec4& value )
        {
            OSG_NOTICE << " vec4 " << value;
        }

        virtual void
        apply( const osg::dvec2& value )
        {
            OSG_NOTICE << " dvec2 " << value;
        }

        virtual void
        apply( const osg::dvec3& value )
        {
            OSG_NOTICE << " dvec3 " << value;
        }

        virtual void
        apply( const osg::dvec4& value )
        {
            OSG_NOTICE << " dvec4 " << value;
        }

        virtual void
        apply( const osg::quat& value )
        {
            OSG_NOTICE << " quat " << value;
        }

        virtual void
        apply( const osg::Plane& value )
        {
            OSG_NOTICE << " Plane " << value;
        }

        virtual void
        apply( const osg::mat4& value )
        {
            OSG_NOTICE << " mat4 " << value;
        }

        virtual void
        apply( const osg::dmat4& value )
        {
            OSG_NOTICE << " dmat4 " << value;
        }
};

template<typename T>
class GetNumeric : public osg::ValueObject::GetValueVisitor
{
    public:

        GetNumeric() :
            _set( false ),
            _value( 0 )
        {
        }

        virtual void
        apply( bool value )
        {
            _value = value;
            _set   = true;
        }

        virtual void
        apply( char value )
        {
            _value = value;
            _set   = true;
        }

        virtual void
        apply( unsigned char value )
        {
            _value = value;
            _set   = true;
        }

        virtual void
        apply( short value )
        {
            _value = value;
            _set   = true;
        }

        virtual void
        apply( unsigned short value )
        {
            _value = value;
            _set   = true;
        }

        virtual void
        apply( int value )
        {
            _value = value;
            _set   = true;
        }

        virtual void
        apply( unsigned int value )
        {
            _value = value;
            _set   = true;
        }

        virtual void
        apply( float value )
        {
            _value = value;
            _set   = true;
        }

        virtual void
        apply( double value )
        {
            _value = value;
            _set   = true;
        }

        bool _set;
        T    _value;
};

template<typename T>
T
getNumeric( osg::Object* object )
{
    osg::ValueObject* bvo = dynamic_cast<osg::ValueObject*>( object );
    if( bvo )
    {
        GetNumeric<T> gn;
        if( bvo->get( gn ) && gn._set )
        {
            return gn._value;
        }
    }
    return T( 0 );
}

void
testResults( osg::Node* node )
{
    int j = 0;
    if( node->getUserValue( "Int value", j ) )
    {
        OSG_NOTICE << "Int value=" << j << std::endl;
    }
    else
    {
        OSG_NOTICE << "Int value not found" << std::endl;
    }

    std::string readString;
    if( node->getUserValue( "Status", readString ) )
    {
        OSG_NOTICE << "Status=" << readString << std::endl;
    }
    else
    {
        OSG_NOTICE << "Status not found" << std::endl;
    }

    float height = 0.0F;
    if( node->getUserValue( "Height", height ) )
    {
        OSG_NOTICE << "Height=" << height << std::endl;
    }
    else
    {
        OSG_NOTICE << "Height not found" << std::endl;
    }

    osg::UserDataContainer* udc = node->getUserDataContainer();
    if( udc )
    {
        OSG_NOTICE << "udc->getNumUserObjects()=" << udc->getNumUserObjects()
                   << std::endl;
        for( unsigned int i = 0; i < udc->getNumUserObjects(); ++i )
        {
            MyGetValueVisitor mgvv;
            osg::Object*      userObject = udc->getUserObject( i );
            osg::ValueObject* valueObject =
                dynamic_cast<osg::ValueObject*>( userObject );
            OSG_NOTICE << "userObject=" << userObject
                       << ", className=" << userObject->className()
                       << ", getName()=" << userObject->getName()
                       << " valueObject=" << valueObject << " getNumeric "
                       << getNumeric<float>( userObject ) << " ";
            if( valueObject )
            {
                valueObject->get( mgvv );
            }
            OSG_NOTICE << std::endl;
        }
    }

    OSG_NOTICE << std::endl << std::endl;
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser      arguments( &argc, argv );

    osg::ref_ptr<osg::Group> node = new osg::Group;

    if( arguments.read( "--MyUserDataContainer" ) || arguments.read( "--mydc" ) )
    {
        node->setUserDataContainer( new MyNamespace::MyUserDataContainer );
    }

    int i = 10;
    node->setUserValue( "Int value", i );

    std::string testString( "All seems fine" );
    node->setUserValue( "Status", testString );

    node->setUserValue( "Height", float( 1.4 ) );

    osg::ref_ptr<osg::Drawable> drawable = new osg::Geometry;
    drawable->setName( "myDrawable" );
    node->getOrCreateUserDataContainer()->addUserObject( drawable.get() );

    node->setUserValue( "fred", 12 );
    node->setUserValue( "john", 1.1 );
    node->setUserValue( "david", 1.9F );
    node->setUserValue( "char", char( 65 ) );
    node->setUserValue( "matrixd", osg::translate( 1.0, 2.0, 3.0 ) );
    node->setUserValue( "flag-on", true );
    node->setUserValue( "flag-off", false );

    OSG_NOTICE << "Testing results for values set directly on scene graph" << std::endl;
    testResults( node.get() );

    {
        osgDB::writeNodeFile( *node, "results.osgt" );

        osg::ref_ptr<osg::Node> from_osgt = osgDB::readRefNodeFile( "results.osgt" );
        if( from_osgt.valid() )
        {
            OSG_NOTICE
                << std::endl
                << "Testing results for values from scene graph read from .osgt file"
                << std::endl;
            testResults( from_osgt.get() );
        }
    }

    {
        osgDB::writeNodeFile( *node, "results.osgb" );

        osg::ref_ptr<osg::Node> from_osgb = osgDB::readRefNodeFile( "results.osgb" );
        if( from_osgb.valid() )
        {
            OSG_NOTICE
                << std::endl
                << "Testing results for values from scene graph read from .osgb file"
                << std::endl;
            testResults( from_osgb.get() );
        }
    }

    {
        osgDB::writeNodeFile( *node, "results.osgx" );

        osg::ref_ptr<osg::Node> from_osgx = osgDB::readRefNodeFile( "results.osgx" );
        if( from_osgx.valid() )
        {
            OSG_NOTICE
                << std::endl
                << "Testing results for values from scene graph read from .osgx file"
                << std::endl;
            testResults( from_osgx.get() );
        }
    }
    return 0;
}
