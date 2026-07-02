/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Volume rendering parameters. Controls sample density,
 * transfer function, and rendering quality settings.
 */
#include <osgVolume/VolumeSettings.hpp>

using namespace osgVolume;

VolumeSettings::VolumeSettings() :
    _technique( MultiPass ),
    _shadingModel( Standard ),
    _sampleRatioProperty( new SampleRatioProperty( 1.0F ) ),
    _sampleRatioWhenMovingProperty( new SampleRatioWhenMovingProperty( 1.0F ) ),
    _cutoffProperty( new AlphaFuncProperty( 0.0F ) ),
    _transparencyProperty( new TransparencyProperty( 1.0F ) ),
    _isoSurfaceProperty( new IsoSurfaceProperty( 0.0F ) )
{
}

VolumeSettings::VolumeSettings( const VolumeSettings& vs,
                                const osg::CopyOp&    copyop ) :
    Inherit( vs,
             copyop ),
    _filename( vs._filename ),
    _technique( vs._technique ),
    _shadingModel( vs._shadingModel ),
    _sampleRatioProperty( osg::clone( vs._sampleRatioProperty.get(),
                                      copyop ) ),
    _sampleRatioWhenMovingProperty( osg::clone( vs._sampleRatioWhenMovingProperty.get(),
                                                copyop ) ),
    _cutoffProperty( osg::clone( vs._cutoffProperty.get(),
                                 copyop ) ),
    _transparencyProperty( osg::clone( vs._transparencyProperty.get(),
                                       copyop ) ),
    _isoSurfaceProperty( osg::clone( vs._isoSurfaceProperty.get(),
                                     copyop ) )
{
}

void
VolumeSettings::accept( PropertyVisitor& pv )
{
    pv.apply( *this );
}

void
VolumeSettings::traverse( PropertyVisitor& pv )
{
    if( _sampleRatioProperty.valid() )
    {
        _sampleRatioProperty->accept( pv );
    }
    if( _sampleRatioWhenMovingProperty.valid() )
    {
        _sampleRatioWhenMovingProperty->accept( pv );
    }
    if( _cutoffProperty.valid() )
    {
        _cutoffProperty->accept( pv );
    }
    if( _transparencyProperty.valid() )
    {
        _transparencyProperty->accept( pv );
    }
    if( _isoSurfaceProperty.valid() && _shadingModel == Isosurface )
    {
        _isoSurfaceProperty->accept( pv );
    }
}

void
VolumeSettings::setCutoff( float co )
{
    _cutoffProperty->setValue( co );
    if( _isoSurfaceProperty.valid() )
    {
        OSG_NOTICE << "Setting IsoSurface value to " << co << std::endl;
        _isoSurfaceProperty->setValue( co );
    }

    dirty();
}
