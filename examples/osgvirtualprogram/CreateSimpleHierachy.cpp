#include <iostream>
#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "VirtualProgram.h"

using osgCandidate::VirtualProgram;

////////////////////////////////////////////////////////////////////////////////
osg::Node * CreateSimpleHierarchy( osg::Node * node )
{
    if( !node ) return NULL;
    float r = node->getBound().radius(); // diameter
    osg::Group * root = new osg::Group();
    osg::Group * group = new osg::Group();

    // Create four matrices for translated instances of the cow
    osg::MatrixTransform * transform0  = new osg::MatrixTransform( );
    transform0->setMatrix( osg::Matrix::translate( 0,0,r ) );

    osg::MatrixTransform * transform1  = new osg::MatrixTransform( );
    transform1->setMatrix( osg::Matrix::translate( 0,0,0 ) );

    osg::MatrixTransform * transform2  = new osg::MatrixTransform( );
    transform2->setMatrix( osg::Matrix::translate( -r,0,-r ) );

    osg::MatrixTransform * transform3  = new osg::MatrixTransform( );
    transform3->setMatrix( osg::Matrix::translate(  r,0,-r ) );

    root->addChild( transform0 );
    root->addChild( group );
    group->addChild( transform1 );
    group->addChild( transform2 );
    group->addChild( transform3 );

    transform0->addChild( node );
    transform1->addChild( node );
    transform2->addChild( node );
    transform3->addChild( node );

    // At the scene root apply standard program
    if( 1 )
    {
        osg::Program * program = new osg::Program;

        osg::Shader * vert = new osg::Shader( osg::Shader::VERTEX );
        vert->setShaderSource(
            "#version 460 core\n"
            "layout(location = 0) in vec4 osg_Vertex;\n"
            "layout(location = 3) in vec4 osg_Color;\n"
            "layout(location = 8) in vec4 osg_MultiTexCoord0;\n"
            "uniform mat4 osg_ModelViewProjectionMatrix;\n"
            "out vec4 vColor;\n"
            "out vec4 vTexCoord;\n"
            "void main(void)\n"
            "{\n"
            "    vColor = osg_Color;\n"
            "    vTexCoord = osg_MultiTexCoord0;\n"
            "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
            "}\n"
            );
        vert->setName( "White Vertex" );
        program->addShader( vert );

        osg::Shader * main = new osg::Shader( osg::Shader::FRAGMENT );
        main->setShaderSource(
            "#version 460 core\n"
            "uniform sampler2D base; \n"
            "in vec4 vColor;\n"
            "in vec4 vTexCoord;\n"
            "out vec4 fragColor;\n"
            "void main(void) \n"
            "{\n"
            "    fragColor = vColor * textureProj( base, vTexCoord );\n"
            "    fragColor *= vec4( 1.0, 1.0, 1.0, 0.5 ); \n"
            "}\n"
            );
        program->addShader( main );

        main->setName( "White" );

        root->getOrCreateStateSet( )->setAttributeAndModes( program );
    }

    // Now override root program with default VirtualProgram for three bottom cows
    if( 1 )
    {
        VirtualProgram * virtualProgram = new VirtualProgram( );

        // Create main shader which declares and calls ColorFilter function
        osg::Shader * main = new osg::Shader( osg::Shader::FRAGMENT );

        main->setShaderSource(
            "#version 460 core\n"
            "vec4 ColorFilter( in vec4 color ); \n"
            "uniform sampler2D base; \n"
            "in vec4 vColor;\n"
            "in vec4 vTexCoord;\n"
            "out vec4 fragColor;\n"
            "void main(void) \n"
            "{ \n"
            "    fragColor = vColor * textureProj( base, vTexCoord ); \n"
            "    fragColor = ColorFilter( fragColor ); \n"
            "}\n"
            );

        virtualProgram->setShader( "main", main );

        main->setName( "Virtual Main" );

        // Create vertex shader for VirtualProgram
        osg::Shader * vpVert = new osg::Shader( osg::Shader::VERTEX );
        vpVert->setShaderSource(
            "#version 460 core\n"
            "layout(location = 0) in vec4 osg_Vertex;\n"
            "layout(location = 3) in vec4 osg_Color;\n"
            "layout(location = 8) in vec4 osg_MultiTexCoord0;\n"
            "uniform mat4 osg_ModelViewProjectionMatrix;\n"
            "out vec4 vColor;\n"
            "out vec4 vTexCoord;\n"
            "void main(void)\n"
            "{\n"
            "    vColor = osg_Color;\n"
            "    vTexCoord = osg_MultiTexCoord0;\n"
            "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
            "}\n"
            );
        vpVert->setName( "Virtual Vertex" );
        virtualProgram->setShader( "vertex_main", vpVert );

        // Create filter shader which implements green ColorFilter function 
        osg::Shader * colorFilter = new osg::Shader( osg::Shader::FRAGMENT );

        colorFilter->setShaderSource(
            "#version 460 core\n"
            "vec4 ColorFilter( in vec4 color ) \n"
            "{ \n"
            "    return color * vec4( 0.0, 1.0, 0.0, 1.0 ); \n"
            "}\n"
            );

        colorFilter->setName( "Virtual Green" );

        virtualProgram->setShader( "ColorFilter", colorFilter );

        group->getOrCreateStateSet( )->setAttributeAndModes( virtualProgram );
    }

    // Create "incomplete" VirtualProgram overriding ColorFilter function
    // Lower left cow drawn will be red
    if( 1 )
    {
        VirtualProgram * virtualProgram = new VirtualProgram();
      
        osg::Shader * redFilter = new osg::Shader( osg::Shader::FRAGMENT );
        redFilter->setShaderSource(
            "#version 460 core\n"
            "vec4 ColorFilter( in vec4 color ) \n"
            "{ \n"
            "    return color * vec4( 1, 0, 0, 1 ); \n"
            "}\n"
            );
        virtualProgram->setShader( "ColorFilter", redFilter );

        redFilter->setName( "Virtual Red" );

        transform2->getOrCreateStateSet( )->setAttribute( virtualProgram );
    }   

    // Create "incomplete" VirtualProgram overriding ColorFilter function
    // Lower right cow will be drawn with grid pattern on yellow background
    if( 1 )
    {
        VirtualProgram * virtualProgram = new VirtualProgram();

        osg::Shader * gridFilter = new osg::Shader( osg::Shader::FRAGMENT );
        gridFilter->setShaderSource(
            "#version 460 core\n"
            "vec4 ColorFilter( in vec4 color ) \n"
            "{ \n"
            "    vec2 grid = clamp( mod( gl_FragCoord.xy, 16.0 ), 0.0, 1.0 ); \n"
            "    return color * vec4( grid, 0.0, 1.0 ); \n"
            "}\n"
            );
        virtualProgram->setShader( "ColorFilter", gridFilter );

        gridFilter->setName( "Virtual Grid" );
       
        transform3->getOrCreateStateSet( )->setAttribute( virtualProgram );
    }

    return root;
}//////////////////////////////////////////
