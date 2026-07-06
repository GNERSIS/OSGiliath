/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "FullscreenQuad.hpp"

#include <osg/geometry/Geometry.hpp>
#include <osg/GL>
#include <osg/nodes/Geode.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osg/state/StateSet.hpp>

namespace
{

    constexpr char fullscreenVertexShader[] = R"glsl(
#version 460 core

layout(location = 0) in vec4 osg_Vertex;
out vec2 vUV;

void main()
{
    vUV = osg_Vertex.xy * 0.5 + 0.5;
    gl_Position = vec4(osg_Vertex.xy, 0.0, 1.0);
}
)glsl";

}

namespace sponza
{

    osg::ref_ptr<osg::Geometry>
    createFullscreenQuadGeometry()
    {
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
        vertices->reserve( 6 );
        vertices->push_back( osg::vec3( -1.0F, -1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( 1.0F, -1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( 1.0F, 1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( -1.0F, -1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( 1.0F, 1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( -1.0F, 1.0F, 0.0F ) );

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setUseVertexBufferObjects( true );
        geometry->setVertexArray( vertices.get() );
        geometry->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 0, 6 ) );
        return geometry;
    }

    osg::ref_ptr<osg::Geode>
    makeFullscreenPassGeode( const char* fragmentShader )
    {
        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addBindAttribLocation( "osg_Vertex", 0U );
        program->addShader( new osg::Shader( osg::Shader::VERTEX,
                                             fullscreenVertexShader ) );
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragmentShader ) );

        osg::ref_ptr<osg::Geometry> geometry = createFullscreenQuadGeometry();
        osg::ref_ptr<osg::Geode>    geode    = new osg::Geode;
        geode->setCullingActive( false );
        geode->addDrawable( geometry.get() );

        osg::StateSet* stateSet = geode->getOrCreateStateSet();
        stateSet->setAttributeAndModes( program.get(), osg::StateAttribute::ON );
        stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
        stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        return geode;
    }

}
