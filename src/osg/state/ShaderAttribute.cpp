/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * State attribute combining a shader program with associated uniforms
 * and modes. Simplifies per-technique shader setup.
 */
#include <osg/state/ShaderAttribute.hpp>

#include <osg/core/Notify.hpp>
#include <osg/state/State.hpp>
#include <osg/state/StateSet.hpp>

using namespace osg;

ShaderAttribute::ShaderAttribute() :
    _type( osg::StateAttribute::Type( -1 ) )
{
    _shaderComponent = new osg::ShaderComponent;
}

ShaderAttribute::ShaderAttribute( const ShaderAttribute& sa,
                                  const CopyOp&          copyop ) :
    StateAttribute( sa,
                    copyop ),
    _type( sa._type ),
    _uniforms( sa._uniforms )
{
}

ShaderAttribute::~ShaderAttribute()
{
}

osg::Object*
ShaderAttribute::cloneType() const
{
    ShaderAttribute* sa = new ShaderAttribute;
    sa->setType( getType() );
    return sa;
}

int
ShaderAttribute::compare( const StateAttribute& sa ) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types( ShaderAttribute, sa )

        // check if types are same
        if( _type < rhs._type ) return -1;
    if( _type > rhs._type )
    {
        return 1;
    }

    // all properties are the equal so return 0.
    return 0;
}

void
ShaderAttribute::setType( Type type )
{
    _type = type;
}

unsigned int
ShaderAttribute::addUniform( Uniform* uniform )
{
    // check to see if uniform already add, if so return the index of it
    for( unsigned int i = 0; i < _uniforms.size(); ++i )
    {
        if( _uniforms[i] == uniform )
        {
            return i;
        }
    }

    // add uniform and return it's position
    _uniforms.push_back( uniform );
    return static_cast<unsigned int>( _uniforms.size() ) - 1;
}

void
ShaderAttribute::removeUniform( unsigned int i )
{
    _uniforms.erase( _uniforms.begin() + i );
}

bool
ShaderAttribute::getModeUsage( StateAttribute::ModeUsage& /*usage*/ ) const
{
    return false;
}

void
ShaderAttribute::apply( State& /*state*/ ) const
{
    // ShaderComposer has been removed; ShaderAttribute uniforms
    // were only consumed by the deprecated composition path.
}

void
ShaderAttribute::compileGLObjects( State& state ) const
{
    if( _shaderComponent.valid() )
    {
        _shaderComponent->compileGLObjects( state );
    }
}

void
ShaderAttribute::resizeGLObjectBuffers( unsigned int maxSize )
{
    if( _shaderComponent.valid() )
    {
        _shaderComponent->resizeGLObjectBuffers( maxSize );
    }
}

void
ShaderAttribute::releaseGLObjects( State* state ) const
{
    if( _shaderComponent.valid() )
    {
        _shaderComponent->releaseGLObjects( state );
    }
}
