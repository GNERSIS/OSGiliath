/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Controls blending weights between multiple texture layers.
 * Used for terrain multi-texturing with smooth transitions.
 */
#include <osgFX/MultiTextureControl>

using namespace osg;
using namespace osgFX;

MultiTextureControl::MultiTextureControl() :
    _useTextureWeightsUniform( true )
{
    _textureWeights = new TextureWeights;
}

MultiTextureControl::MultiTextureControl( const MultiTextureControl& copy,
                                          const osg::CopyOp&         copyop ) :
    Inherit( copy,
             copyop ),
    _textureWeights( osg::clone( copy._textureWeights.get(),
                                 osg::CopyOp::DEEP_COPY_ALL ) ),
    _useTextureWeightsUniform( copy._useTextureWeightsUniform )
{
    updateStateSet();
}

void
MultiTextureControl::setTextureWeight( unsigned int unit,
                                       float        weight )
{
    if( unit >= _textureWeights->size() )
    {
        _textureWeights->resize( unit + 1, 0.0F );
    }
    ( *_textureWeights )[unit] = weight;

    updateStateSet();
}

void
MultiTextureControl::updateStateSet()
{
    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

    if( _useTextureWeightsUniform && _textureWeights->size() > 0 )
    {
        osg::ref_ptr<osg::Uniform> uniform =
            new osg::Uniform( osg::Uniform::FLOAT,
                              "TextureWeights",
                              static_cast<int>( _textureWeights->size() ) );
#if 1
        uniform->setArray( _textureWeights.get() );
#else
        for( unsigned int i = 0; i < _textureWeights->size(); ++i )
        {
            uniform->setElement( i, ( *_textureWeights )[i] );
        }
#endif
        stateset->addUniform( uniform.get() );
        stateset->setDefine( "TEXTURE_WEIGHTS" );
    }

    setStateSet( stateset.get() );
}
