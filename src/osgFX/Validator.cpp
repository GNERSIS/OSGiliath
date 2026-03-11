#include <osgFX/Validator>

#include <osg/core/Notify.hpp>
#include <osgFX/Effect>

using namespace osgFX;

Validator::Validator() :
    _effect( 0 )
{
}

Validator::Validator( Effect* effect ) :
    _effect( effect )
{
}

Validator::Validator( const Validator&   copy,
                      const osg::CopyOp& copyop ) :
    Inherit( copy,
             copyop ),
    _effect( static_cast<Effect*>( copyop( copy._effect ) ) )
{
}

void
Validator::compileGLObjects( osg::State& state ) const
{
    apply( state );
}

void
Validator::apply( osg::State& state ) const
{
    if( !_effect )
    {
        return;
    }

    if( _effect->_tech_selected[state.getContextID()] == 0 )
    {
        Effect::Technique_list::iterator i;
        int                              j = 0;
        for( i = _effect->_techs.begin(); i != _effect->_techs.end(); ++i, ++j )
        {
            if( ( *i )->validate( state ) )
            {
                _effect->_sel_tech[state.getContextID()]      = j;
                _effect->_tech_selected[state.getContextID()] = 1;
                return;
            }
        }
        OSG_WARN << "Warning: osgFX::Validator: could not find any techniques "
                    "compatible with the current OpenGL context"
                 << std::endl;
    }
}
