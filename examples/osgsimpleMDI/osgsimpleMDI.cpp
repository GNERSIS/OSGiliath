/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgsimpleMDI example application
 */
/* file:        examples/osgsimpleMDI/osgsimpleMDI.cpp
 * author:      Julien Valentin 2017-08-01
 * copyright:   (C) 2013
 * license:     OpenSceneGraph Public License (OSGPL)
 *
 * A simple example of mdi with basevertex
 *
 */

#include <iostream>
#include <osg/core/Notify.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/PrimitiveSetIndirect.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/BufferIndexBinding.hpp>
#include <osg/state/Point.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

int
main( int    argc,
      char** argv )
{

    osg::ArgumentParser arguments( &argc, argv );
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the example which demonstrates Multi Indirect Draw with basevertex"
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] "
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption( "--numX",
                                                           "square count on X" );
    arguments.getApplicationUsage()->addCommandLineOption( "--numY",
                                                           "square count on Y" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--classic",
        "disable MDI and use classic DrawElements"
    );

    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    int MAXX = 200;
    int MAXY = 200;
    arguments.read( "--numX", MAXX );
    arguments.read( "--numY", MAXY );

    enum PrimtiveSetUsage
    {
        MultiDraw,
        MultiplePrimitiveSets,
        SinglePrimitiveSet,
    };

    PrimtiveSetUsage usage = MultiDraw;
    if( arguments.read( "--classic" ) )
    {
        usage = MultiplePrimitiveSets;
        OSG_WARN << "disabling MDI, using multiple PrimitiveSet" << std::endl;
    }

    if( arguments.read( "--single" ) )
    {
        usage = SinglePrimitiveSet;
        OSG_WARN << "disabling MDI, using single PrimitiveSet" << std::endl;
    }

    osg::Geode*                            root( new osg::Geode );

    osg::ref_ptr<osg::ElementBufferObject> ebo = new osg::ElementBufferObject;

    /// create empty mdi
    osg::MultiDrawElementsIndirectUShort*  mdi =
        new osg::MultiDrawElementsIndirectUShort( osg::PrimitiveSet::TRIANGLE_STRIP );
    osg::DefaultIndirectCommandDrawElements* mdicommands =
        new osg::DefaultIndirectCommandDrawElements();
    mdi->setIndirectCommandArray( mdicommands );

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    geom->setUseVertexBufferObjects( true );

    osg::box bb;
    bb.set( 0, 0, 0, MAXX, 0, MAXY );
    // set bounds by hand cause of the lack of support of basevertex in PrimitiveFunctors
    geom->setInitialBound( bb );

    osg::vec3 myCoords[] = {
        osg::vec3( 0, 0.0F, 0.7F ),
        osg::vec3( 0, 0.0F, 0 ),
        osg::vec3( 0.7, 0.0F, 0 ),
        osg::vec3( 0.7F, 0.0F, 0.7F )
    };

    unsigned short  myIndices[]   = { 0, 1, 3, 2 };
    unsigned int    myIndicesUI[] = { 0, 1, 3, 2 };

    osg::Vec3Array* verts         = new osg::Vec3Array();

    for( int j = 0; j < MAXY; ++j )
    {
        for( int i = 0; i < MAXX; ++i )
        {
            /// create indirect command
            osg::DrawElementsIndirectCommand cmd;
            cmd.count         = 4;
            cmd.instanceCount = 1;
            cmd.firstIndex    = verts->size();
            cmd.baseVertex    = verts->size();
            mdicommands->push_back( cmd );

            for( int z = 0; z < 4; z++ )
            {
                verts->push_back( osg::vec3( i, 0, j ) + myCoords[z] );
                mdi->addElement( myIndices[z] );
            }
        }
    }

    geom->setVertexArray( verts );

    switch( usage )
    {
        case( MultiDraw ) :
            {
                geom->addPrimitiveSet( mdi );
                break;
            }
        case( MultiplePrimitiveSets ) :
            {
                for( int i = 0; i < MAXY * MAXX; ++i )
                {
                    osg::ref_ptr<osg::DrawElementsUInt> dre =
                        new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLE_STRIP,
                                                   4,
                                                   myIndicesUI );
                    dre->setElementBufferObject( ebo.get() );
                    geom->addPrimitiveSet( dre.get() );
                    for( int z = 0; z < 4; z++ )
                    {
                        myIndicesUI[z] += 4;
                    }
                }
                break;
            }
        case( SinglePrimitiveSet ) :
            {
                osg::ref_ptr<osg::DrawElementsUInt> primitives =
                    new osg::DrawElementsUInt( GL_TRIANGLES );
                primitives->setElementBufferObject( ebo.get() );
                geom->addPrimitiveSet( primitives.get() );

                unsigned int vi = 0;
                for( int i = 0; i < MAXY * MAXX; ++i )
                {
                    primitives->push_back( vi );
                    primitives->push_back( vi + 2 );
                    primitives->push_back( vi + 1 );
                    primitives->push_back( vi + 1 );
                    primitives->push_back( vi + 2 );
                    primitives->push_back( vi + 3 );
                    vi += 4;
                }
                break;
            }
    }

    root->addChild( geom );

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
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.setSceneData( root );
    return viewer.run();
}
