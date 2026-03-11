#include <osgFX/Technique>

#include <osg/rendering/GLExtensions.hpp>
#include <osgFX/Effect>
#include <osgUtil/culling/CullVisitor.hpp>

using namespace osgFX;

Technique::Technique() :
    osg::Referenced(),
    _passesDefined( 0 )
{
}

void
Technique::addPass( osg::StateSet* ss )
{
    if( ss )
    {
        _passes.push_back( ss );
        ss->setRenderBinDetails( static_cast<int>( _passes.size() ), "RenderBin" );
    }
}

bool
Technique::validate( osg::State& state ) const
{
    typedef std::vector<std::string> String_list;
    String_list                      extensions;

    getRequiredExtensions( extensions );

    for( String_list::const_iterator i = extensions.begin(); i != extensions.end(); ++i )
    {
        if( !osg::isGLExtensionSupported( state.getContextID(), i->c_str() ) )
        {
            return false;
        }
    }

    return true;
}

void
Technique::traverse_implementation( osg::NodeVisitor& nv,
                                    Effect*           fx )
{
    // define passes if necessary
    if( _passesDefined == 0 )
    {
        std::lock_guard<std::mutex> lock( _mutex );

        if( _passesDefined == 0 )
        {
            define_passes();

            _passesDefined.exchange( 1 );
        }
    }

    // special actions must be taken if the node visitor is actually a CullVisitor
    osgUtil::CullVisitor* cv = nv.asCullVisitor();
    if( cv )
    {
        // traverse all passes
        for( size_t i = 0; i < _passes.size(); ++i )
        {
            // push the i-th pass' StateSet
            cv->pushStateSet( _passes[i].get() );

            // traverse the override node if defined, otherwise
            // traverse children as a Group would do
            osg::Node* override = getOverrideChild( static_cast<int>( i ) );
            if( override )
            {
                override->accept( nv );
            }
            else
            {
                fx->inherited_traverse( nv );
            }

            // pop the StateSet
            cv->popStateSet();
        }
    }
    else
    {
        fx->inherited_traverse( nv );
    }
}
