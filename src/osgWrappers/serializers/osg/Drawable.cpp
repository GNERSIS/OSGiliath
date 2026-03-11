#include <osg/Drawable>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkInitialBound( const osg::Drawable& drawable )
{
    return drawable.getInitialBound().valid();
}

static bool readInitialBound( osgDB::InputStream& is, osg::Drawable& drawable )
{
    osg::Vec3d min, max;
    is >> is.BEGIN_BRACKET;
    is >> is.PROPERTY("Minimum") >> min;
    is >> is.PROPERTY("Maximum") >> max;
    is >> is.END_BRACKET;
    drawable.setInitialBound( osg::BoundingBox(min, max) );
    return true;
}

static bool writeInitialBound( osgDB::OutputStream& os, const osg::Drawable& drawable )
{
    const osg::BoundingBox& bb = drawable.getInitialBound();
    os << os.BEGIN_BRACKET << std::endl;
    os << os.PROPERTY("Minimum") << osg::Vec3d(bb._min) << std::endl;
    os << os.PROPERTY("Maximum") << osg::Vec3d(bb._max) << std::endl;
    os << os.END_BRACKET;
    os << std::endl;
    return true;
}

// Backward-compatibility stubs for display list serializers removed in Core Profile.
// Old .osgt files (version < current) may contain SupportsDisplayList and UseDisplayList
// properties. These stubs allow the parser to read and discard them so that subsequent
// properties (PrimitiveSetList, VertexData, etc.) are parsed correctly.
static bool checkSupportsDisplayList( const osg::Drawable& ) { return false; }
static bool readSupportsDisplayList( osgDB::InputStream& is, osg::Drawable& )
{
    bool value; is >> value; return true;
}
static bool writeSupportsDisplayList( osgDB::OutputStream&, const osg::Drawable& )
{
    return true;
}

static bool checkUseDisplayList( const osg::Drawable& ) { return false; }
static bool readUseDisplayList( osgDB::InputStream& is, osg::Drawable& )
{
    bool value; is >> value; return true;
}
static bool writeUseDisplayList( osgDB::OutputStream&, const osg::Drawable& )
{
    return true;
}

REGISTER_OBJECT_WRAPPER( Drawable,
                         /*new osg::Drawable*/NULL,
                         osg::Drawable,
                         "osg::Object osg::Node osg::Drawable" )
{
    {
         UPDATE_TO_VERSION_SCOPED( 154 )
         ADDED_ASSOCIATE("osg::Node")
    }

    ADD_OBJECT_SERIALIZER( StateSet, osg::StateSet, NULL );  // _stateset
    ADD_USER_SERIALIZER( InitialBound );  // _initialBound
    ADD_OBJECT_SERIALIZER( ComputeBoundingBoxCallback,
                           osg::Drawable::ComputeBoundingBoxCallback, NULL );  // _computeBoundCallback
    ADD_OBJECT_SERIALIZER( Shape, osg::Shape, NULL );  // _shape
    ADD_USER_SERIALIZER( SupportsDisplayList );  // backward compat: read and discard
    ADD_USER_SERIALIZER( UseDisplayList );  // backward compat: read and discard
    ADD_BOOL_SERIALIZER( UseVertexBufferObjects, false );  // _useVertexBufferObjects
    ADD_OBJECT_SERIALIZER( UpdateCallback, osg::Callback, NULL );  // _updateCallback
    ADD_OBJECT_SERIALIZER( EventCallback, osg::Callback, NULL );  // _eventCallback
    ADD_OBJECT_SERIALIZER( CullCallback, osg::Callback, NULL );  // _cullCallback
    ADD_OBJECT_SERIALIZER( DrawCallback, osg::Drawable::DrawCallback, NULL );  // _drawCallback
    {
         //now provided by Node's serialization
         UPDATE_TO_VERSION_SCOPED( 156 )
         REMOVE_SERIALIZER( StateSet )
         REMOVE_SERIALIZER( UpdateCallback )
         REMOVE_SERIALIZER( EventCallback )
         REMOVE_SERIALIZER( CullCallback )
         REMOVE_SERIALIZER( DrawCallback )
    }

    {
        UPDATE_TO_VERSION_SCOPED( 142 )
        ADD_HEXINT_SERIALIZER( NodeMask, 0xffffffff );  // _nodeMask
    }

    {
        UPDATE_TO_VERSION_SCOPED( 145 )
        ADD_BOOL_SERIALIZER( CullingActive, true );  // _cullingActive
    }

}
