/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgpersistentbufferstorage example application
 */
/* file:        examples/osgpersistentbufferstorage/osgpersistentbufferstorage.cpp
 * author:      Julien Valentin 2020-10-01
 * copyright:   (C) 2013
 * license:     OpenSceneGraph Public License (OSGPL)
 *
 * A test of BufferStorage and PersistantBufferStorage features
 *
 */

#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>

///////////////////////////////////////////////////////////////////////////

class SineAnimation : public osg::Drawable::DrawCallback
{
    public:

        SineAnimation( osg::Vec4Array* dyn,
                       float           scale  = 1.0F,
                       float           offset = 0.0F ) :
            _dyn( dyn ),
            _rate( 0 ),
            _scale( scale ),
            _offset( offset )
        {
        }

        virtual void
        drawImplementation( osg::RenderInfo&     renderInfo,
                            const osg::Drawable* drawable ) const
        {
            unsigned int         contextId = renderInfo.getContextID();
            osg::GLBufferObject* glbo =
                _dyn->getBufferObject()->getOrCreateGLBufferObject( contextId );
            glbo->bindBuffer();
            GLfloat* data = ( GLfloat* )glbo->_persistentDMA +
                            glbo->getOffset( _dyn->getBufferIndex() );
            if( data )
            {
                _rate       += 0.01;
                float value  = sinf( _rate ) * _scale + _offset;
                for( int i = 0; i < 4; i++ )
                {
                    data[i * 4 + 0] = float( i ) * 0.25 * value;
                }
                glbo->commitDMA( _dyn->getBufferIndex() );
            }
            drawable->drawImplementation( renderInfo );
        }

    private:

        osg::ref_ptr<osg::Vec4Array> _dyn;
        mutable float                _rate;
        const float                  _scale;
        const float                  _offset;
};

///////////////////////////////////////////////////////////////////////////

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser      arguments( &argc, argv );
    osg::ref_ptr<osg::Group> root = new osg::Group;
    /// a first geom to demonstrate how to use usage to enable immutable buffer storage
    {
        osg::ref_ptr<osg::Vec4Array> vAry = new osg::Vec4Array;
        vAry->push_back( osg::vec4( 2, 0, 0, 1 ) );
        vAry->push_back( osg::vec4( 2, 0, 1, 1 ) );
        vAry->push_back( osg::vec4( 3, 0, 0, 1 ) );
        vAry->push_back( osg::vec4( 3, 0, 1, 1 ) );
        osg::ref_ptr<osg::VertexBufferObject> vbo = new osg::VertexBufferObject;
        vbo->setUsage( GL_MAP_WRITE_BIT );              // enable bufferStorage
        vbo->setMappingBitfield( GL_MAP_WRITE_BIT );    // set mapping flags
        vAry->setBufferObject( vbo.get() );

        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        geom->setUseVertexBufferObjects( true );
        geom->setVertexArray( vAry.get() );
        geom->addPrimitiveSet( new osg::DrawArrays( GL_LINES, 0, vAry->size() ) );
        root->addChild( geom );
    }

    // second geometry buffer is persistent mapped and modified in camera callback
    {
        osg::ref_ptr<osg::Vec4Array> vAry = new osg::Vec4Array;
        vAry->setDataVariance( osg::Object::DataVariance::STATIC );
        vAry->push_back( osg::vec4( 0, 0, 0, 1 ) );
        vAry->push_back( osg::vec4( 0, 0, 1, 1 ) );
        vAry->push_back( osg::vec4( 1, 0, 0, 1 ) );
        vAry->push_back( osg::vec4( 1, 0, 1, 1 ) );
        osg::ref_ptr<osg::VertexBufferObject> vbo = new osg::VertexBufferObject;
        vbo->setDataVariance( osg::Object::DataVariance::STATIC );
        vbo->setUsage( GL_MAP_WRITE_BIT |
                       GL_MAP_PERSISTENT_BIT );    // enable persistent bufferStorage
        vbo->setMappingBitfield( GL_MAP_WRITE_BIT |
                                 GL_MAP_PERSISTENT_BIT |
                                 GL_MAP_FLUSH_EXPLICIT_BIT );    // set mapping flags
        vAry->setBufferObject( vbo.get() );

        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        geom->setUseVertexBufferObjects( true );
        geom->setVertexArray( vAry.get() );
        osg::DrawElementsUShort* indices = new osg::DrawElementsUShort( GL_TRIANGLES );
        indices->push_back( 0 );
        indices->push_back( 1 );
        indices->push_back( 2 );
        indices->push_back( 2 );
        indices->push_back( 1 );
        indices->push_back( 3 );
        geom->addPrimitiveSet( indices );
        geom->setDrawCallback( new SineAnimation( vAry.get() ) );
        root->addChild( geom );
    }

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( root );
    return viewer.run();
}
