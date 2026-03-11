/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgspacewarp example application
 */
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

float
random( float min,
        float max )
{
    return min + ( max - min ) * ( float )rand() / ( float )RAND_MAX;
}

struct DrawCallback : public osg::Drawable::DrawCallback
{

        DrawCallback() :
            _firstTime( true )
        {
        }

        virtual void
        drawImplementation( osg::RenderInfo&     renderInfo,
                            const osg::Drawable* drawable ) const
        {
            osg::State& state = *renderInfo.getState();

            if( !_firstTime )
            {
                _previousModelViewMatrix = _currentModelViewMatrix;
                _currentModelViewMatrix  = state.getModelViewMatrix();
                _inverseModelViewMatrix  = osg::inverse( _currentModelViewMatrix );

                osg::dmat4     T( _previousModelViewMatrix * _inverseModelViewMatrix );

                osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(
                    const_cast<osg::Drawable*>( drawable )
                );
                osg::Vec3Array* vertices =
                    dynamic_cast<osg::Vec3Array*>( geometry->getVertexArray() );
                for( unsigned int i = 0; i + 1 < vertices->size(); i += 2 )
                {
                    ( *vertices )[i + 1] = ( *vertices )[i] * T;
                }
            }
            else
            {
                _currentModelViewMatrix = state.getModelViewMatrix();
            }

            _firstTime = false;

            drawable->drawImplementation( renderInfo );
        }

        mutable bool       _firstTime;
        mutable osg::dmat4 _currentModelViewMatrix;
        mutable osg::dmat4 _inverseModelViewMatrix;
        mutable osg::dmat4 _previousModelViewMatrix;
};

osg::Node*
createScene( unsigned int noStars )
{

    osg::Geometry*  geometry = new osg::Geometry;

    // set up vertices
    osg::Vec3Array* vertices = new osg::Vec3Array( noStars * 2 );
    geometry->setVertexArray( vertices );

    float        min = -1.0F;
    float        max = 1.0F;
    unsigned int j   = 0;
    unsigned int i   = 0;
    for( i = 0; i < noStars; ++i, j += 2 )
    {
        ( *vertices )[j].set( random( min, max ),
                              random( min, max ),
                              random( min, max ) );
        ( *vertices )[j + 1] = ( *vertices )[j] + osg::vec3( 0.0F, 0.0F, 0.001F );
    }

    // set up colours
    osg::Vec4Array* colours = new osg::Vec4Array( 1 );
    geometry->setColorArray( colours, osg::Array::BIND_OVERALL );
    ( *colours )[0].set( 1.0F, 1.0F, 1.0F, 1.0F );

    // set up the primitive set to draw lines
    geometry->addPrimitiveSet( new osg::DrawArrays( GL_LINES, 0, noStars * 2 ) );

    // set up the points for the stars.
    osg::DrawElementsUShort* points = new osg::DrawElementsUShort( GL_POINTS, noStars );
    geometry->addPrimitiveSet( points );
    for( i = 0; i < noStars; ++i )
    {
        ( *points )[i] = i * 2;
    }

    geometry->setDrawCallback( new DrawCallback );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( geometry );
    // GL_LIGHTING removed for Core Profile compatibility

    osg::Group* group = new osg::Group;
    group->addChild( geode );

    return group;
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
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

    // set the scene to render
    viewer.setSceneData( createScene( 50'000 ) );
    return viewer.run();
}
