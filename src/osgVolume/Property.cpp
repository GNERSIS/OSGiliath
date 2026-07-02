/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Volume rendering properties (transfer function, sample density,
 * transparency, IsoSurface value) controlling appearance.
 */
#include <osgVolume/Property.hpp>

#include <osgVolume/RayTracedTechnique.hpp>
#include <osgVolume/VolumeSettings.hpp>
#include <osgVolume/VolumeTile.hpp>

using namespace osgVolume;

Property::Property() :
    _modifiedCount( 0 )
{
}

Property::Property( const Property&    property,
                    const osg::CopyOp& copyop ) :
    Inherit( property,
             copyop ),
    _modifiedCount( 0 )
{
}

Property::~Property()
{
}

/////////////////////////////////////////////////////////////////////////////
//
// CompositeProperty
//
CompositeProperty::CompositeProperty()
{
}

CompositeProperty::CompositeProperty( const CompositeProperty& compositeProperty,
                                      const osg::CopyOp&       copyop ) :
    Inherit( compositeProperty,
             copyop )
{
}

void
CompositeProperty::clear()
{
    _properties.clear();
    dirty();
}

/////////////////////////////////////////////////////////////////////////////
//
// SwitchProperty
//
SwitchProperty::SwitchProperty() :
    _activeProperty( 0 )
{
}

SwitchProperty::SwitchProperty( const SwitchProperty& switchProperty,
                                const osg::CopyOp&    copyop ) :
    Inherit( switchProperty,
             copyop ),
    _activeProperty( switchProperty._activeProperty )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// TransferFunctionProperty
//
TransferFunctionProperty::TransferFunctionProperty( osg::TransferFunction* tf ) :
    _tf( tf )
{
}

TransferFunctionProperty::TransferFunctionProperty( const TransferFunctionProperty& tfp,
                                                    const osg::CopyOp& copyop ) :
    Inherit( tfp,
             copyop ),
    _tf( tfp._tf )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// ScalarProperty
//
ScalarProperty::ScalarProperty()
{
    _uniform = new osg::Uniform;
}

ScalarProperty::ScalarProperty( const std::string& scalarName,
                                float              value )
{
    setName( scalarName );
    _uniform = new osg::Uniform( scalarName.c_str(), value );
}

ScalarProperty::ScalarProperty( const ScalarProperty& sp,
                                const osg::CopyOp&    copyop ) :
    Inherit( sp,
             copyop )
{
    _uniform = new osg::Uniform( *sp._uniform.get(), copyop );
}

/////////////////////////////////////////////////////////////////////////////
//
// IsoSurfaceProperty
//
IsoSurfaceProperty::IsoSurfaceProperty( float value ) :
    Inherit( "IsoSurfaceValue",
             value )
{
}

IsoSurfaceProperty::IsoSurfaceProperty( const IsoSurfaceProperty& isp,
                                        const osg::CopyOp&        copyop ) :
    Inherit( isp,
             copyop )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// AlphaFuncProperty
//
AlphaFuncProperty::AlphaFuncProperty( float value ) :
    Inherit( "AlphaFuncValue",
             value )
{
}

AlphaFuncProperty::AlphaFuncProperty( const AlphaFuncProperty& afp,
                                      const osg::CopyOp&       copyop ) :
    Inherit( afp,
             copyop )
{
}

void
AlphaFuncProperty::setValue( float v )
{
    dirty();
    _uniform->set( v );
}

/////////////////////////////////////////////////////////////////////////////
//
// MaximumIntensityProjectionProperty
//
MaximumIntensityProjectionProperty::MaximumIntensityProjectionProperty()
{
}

MaximumIntensityProjectionProperty::MaximumIntensityProjectionProperty(
    const MaximumIntensityProjectionProperty& isp,
    const osg::CopyOp&                        copyop
) :
    Inherit( isp,
             copyop )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// LightingProperty
//
LightingProperty::LightingProperty()
{
}

LightingProperty::LightingProperty( const LightingProperty& isp,
                                    const osg::CopyOp&      copyop ) :
    Inherit( isp,
             copyop )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// SampleDensityProperty
//
SampleDensityProperty::SampleDensityProperty( float value ) :
    Inherit( "SampleDensityValue",
             value )
{
}

SampleDensityProperty::SampleDensityProperty( const SampleDensityProperty& isp,
                                              const osg::CopyOp&           copyop ) :
    Inherit( isp,
             copyop )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// SampleDensityWhenMovingProperty
//
SampleDensityWhenMovingProperty::SampleDensityWhenMovingProperty( float value ) :
    Inherit( "SampleDensityValue",
             value )
{
}

SampleDensityWhenMovingProperty::SampleDensityWhenMovingProperty(
    const SampleDensityWhenMovingProperty& isp,
    const osg::CopyOp&                     copyop
) :
    Inherit( isp,
             copyop )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// SampleRatioProperty
//
SampleRatioProperty::SampleRatioProperty( float value ) :
    Inherit( "SampleRatioValue",
             value )
{
}

SampleRatioProperty::SampleRatioProperty( const SampleRatioProperty& srp,
                                          const osg::CopyOp&         copyop ) :
    Inherit( srp,
             copyop )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// SampleRatioWhenMovingProperty
//
SampleRatioWhenMovingProperty::SampleRatioWhenMovingProperty( float value ) :
    Inherit( "SampleRatioValue",
             value )
{
}

SampleRatioWhenMovingProperty::SampleRatioWhenMovingProperty(
    const SampleRatioWhenMovingProperty& isp,
    const osg::CopyOp&                   copyop
) :
    Inherit( isp,
             copyop )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// TransparencyProperty
//
TransparencyProperty::TransparencyProperty( float value ) :
    Inherit( "TransparencyValue",
             value )
{
}

TransparencyProperty::TransparencyProperty( const TransparencyProperty& isp,
                                            const osg::CopyOp&          copyop ) :
    Inherit( isp,
             copyop )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// ExteriorTransparencyFactorProperty
//
ExteriorTransparencyFactorProperty::ExteriorTransparencyFactorProperty( float value ) :
    Inherit( "ExteriorTransparencyFactorValue",
             value )
{
}

ExteriorTransparencyFactorProperty::ExteriorTransparencyFactorProperty(
    const ExteriorTransparencyFactorProperty& etfp,
    const osg::CopyOp&                        copyop
) :
    Inherit( etfp,
             copyop )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// PropertyVisitor
//
PropertyVisitor::PropertyVisitor( bool traverseOnlyActiveChildren ) :
    _traverseOnlyActiveChildren( traverseOnlyActiveChildren )
{
}

void
PropertyVisitor::apply( Property& p )
{
    p.traverse( *this );
}

void
PropertyVisitor::apply( CompositeProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( SwitchProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( TransferFunctionProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( ScalarProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( IsoSurfaceProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( AlphaFuncProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( MaximumIntensityProjectionProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( LightingProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( SampleRatioProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( SampleRatioWhenMovingProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( SampleDensityProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( SampleDensityWhenMovingProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( TransparencyProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( ExteriorTransparencyFactorProperty& p )
{
    apply( static_cast<Property&>( p ) );
}

void
PropertyVisitor::apply( VolumeSettings& p )
{
    apply( static_cast<Property&>( p ) );
}

/////////////////////////////////////////////////////////////////////////////
//
// CollectPropertiesVisitor
//
CollectPropertiesVisitor::CollectPropertiesVisitor( bool traverseOnlyActiveChildren ) :
    PropertyVisitor( traverseOnlyActiveChildren )
{
}

void
CollectPropertiesVisitor::apply( TransferFunctionProperty& tf )
{
    _tfProperty = &tf;
}

void
CollectPropertiesVisitor::apply( ScalarProperty& )
{
}

void
CollectPropertiesVisitor::apply( IsoSurfaceProperty& iso )
{
    _isoProperty = &iso;
}

void
CollectPropertiesVisitor::apply( AlphaFuncProperty& af )
{
    _afProperty = &af;
}

void
CollectPropertiesVisitor::apply( MaximumIntensityProjectionProperty& mip )
{
    _mipProperty = &mip;
}

void
CollectPropertiesVisitor::apply( LightingProperty& lp )
{
    _lightingProperty = &lp;
}

void
CollectPropertiesVisitor::apply( SampleDensityProperty& sdp )
{
    _sampleDensityProperty = &sdp;
}

void
CollectPropertiesVisitor::apply( SampleDensityWhenMovingProperty& sdp )
{
    _sampleDensityWhenMovingProperty = &sdp;
}

void
CollectPropertiesVisitor::apply( SampleRatioProperty& srp )
{
    _sampleRatioProperty = &srp;
}

void
CollectPropertiesVisitor::apply( SampleRatioWhenMovingProperty& srp )
{
    _sampleRatioWhenMovingProperty = &srp;
}

void
CollectPropertiesVisitor::apply( TransparencyProperty& tp )
{
    _transparencyProperty = &tp;
}

void
CollectPropertiesVisitor::apply( ExteriorTransparencyFactorProperty& etfp )
{
    _exteriorTransparencyFactorProperty = &etfp;
}

class CycleSwitchVisitor : public osgVolume::PropertyVisitor
{
    public:

        CycleSwitchVisitor( int delta ) :
            PropertyVisitor( false ),
            _delta( delta ),
            _switchModified( false )
        {
        }

        virtual void
        apply( VolumeSettings& vs )
        {
            int newValue = static_cast<int>( vs.getShadingModel() ) + _delta;
            if( newValue < 0 )
            {
                newValue = VolumeSettings::MaximumIntensityProjection;
            }
            else if( newValue > VolumeSettings::MaximumIntensityProjection )
            {
                newValue = 0;
            }
            vs.setShadingModel( static_cast<VolumeSettings::ShadingModel>( newValue ) );
            OSG_NOTICE << "CycleSwitchVisitor::apply(VolumeSettings&) " << newValue
                       << std::endl;

            _switchModified = true;

            PropertyVisitor::apply( vs );
        }

        virtual void
        apply( SwitchProperty& sp )
        {
            if( sp.getNumProperties() > 1 )
            {
                int newValue = static_cast<int>( sp.getActiveProperty() ) + _delta;
                if( newValue >= static_cast<int>( sp.getNumProperties() ) )
                {
                    newValue = 0;
                }
                if( newValue < 0 )
                {
                    newValue = static_cast<int>( sp.getNumProperties() ) - 1;
                }

                sp.setActiveProperty( newValue );

                OSG_NOTICE << "CycleSwitchVisitor::apply(SwitchProperty&) " << newValue
                           << std::endl;

                _switchModified = true;
            }

            PropertyVisitor::apply( sp );
        }

        int  _delta;
        bool _switchModified;
};

/////////////////////////////////////////////////////////////////////////////
//
// PropertyAdjustmentCallback
//
PropertyAdjustmentCallback::PropertyAdjustmentCallback() :
    _cyleForwardKey( 'v' ),
    _cyleBackwardKey( 'V' ),
    _transparencyKey( 't' ),
    _exteriorTransparencyFactorKey( 'y' ),
    _alphaFuncKey( 'a' ),
    _sampleDensityKey( 'd' ),
    _updateTransparency( false ),
    _updateExteriorTransparencyFactor( false ),
    _updateAlphaCutOff( false ),
    _updateSampleDensity( false )
{
}

PropertyAdjustmentCallback::PropertyAdjustmentCallback(
    const PropertyAdjustmentCallback& pac,
    const osg::CopyOp&                copyop
) :
    osg::Object( pac,
                 copyop ),
    osg::Callback( pac,
                   copyop ),
    Inherit( pac,
             copyop ),
    osg::StateSet::Callback( pac,
                             copyop ),
    _cyleForwardKey( pac._cyleForwardKey ),
    _cyleBackwardKey( pac._cyleBackwardKey ),
    _transparencyKey( pac._transparencyKey ),
    _exteriorTransparencyFactorKey( pac._exteriorTransparencyFactorKey ),
    _alphaFuncKey( pac._alphaFuncKey ),
    _sampleDensityKey( pac._sampleDensityKey ),
    _updateTransparency( false ),
    _updateExteriorTransparencyFactor( false ),
    _updateAlphaCutOff( false ),
    _updateSampleDensity( false )
{
}

bool
PropertyAdjustmentCallback::handle( const osgGA::GUIEventAdapter& ea,
                                    osgGA::GUIActionAdapter&,
                                    osg::Object* object,
                                    osg::NodeVisitor* )
{
    if( ea.getHandled() )
    {
        return false;
    }

    osgVolume::VolumeTile* tile     = dynamic_cast<osgVolume::VolumeTile*>( object );
    osgVolume::Layer*      layer    = tile ? tile->getLayer() : 0;
    osgVolume::Property*   property = layer ? layer->getProperty() : 0;
    if( !property )
    {
        return false;
    }

    osgVolume::CollectPropertiesVisitor cpv;
    property->accept( cpv );

    bool passOnUpdates = false;

    switch( ea.getEventType() )
    {
        case( osgGA::GUIEventAdapter::MOVE ) :
        case( osgGA::GUIEventAdapter::DRAG ) :
            {
                passOnUpdates = true;
                break;
            }
        case( osgGA::GUIEventAdapter::KEYDOWN ) :
            {
                if( ea.getKey() == _cyleForwardKey || ea.getKey() == _cyleBackwardKey )
                {
                    CycleSwitchVisitor csv( ( ea.getKey() == _cyleForwardKey ) ? 1
                                                                               : -1 );
                    property->accept( csv );
                    if( csv._switchModified )
                    {
                        if( dynamic_cast<osgVolume::RayTracedTechnique*>(
                                tile->getVolumeTechnique()
                            ) )
                        {
                            tile->setDirty( true );
                            tile->init();
                        }
                    }
                }
                else if( ea.getKey() == _transparencyKey )
                {
                    _updateTransparency = passOnUpdates = true;
                }
                else if( ea.getKey() == _exteriorTransparencyFactorKey )
                {
                    _updateExteriorTransparencyFactor = passOnUpdates = true;
                }
                else if( ea.getKey() == _alphaFuncKey )
                {
                    _updateAlphaCutOff = passOnUpdates = true;
                }
                else if( ea.getKey() == _sampleDensityKey )
                {
                    _updateSampleDensity = passOnUpdates = true;
                }
                break;
            }
        case( osgGA::GUIEventAdapter::KEYUP ) :
            {
                if( ea.getKey() == _transparencyKey )
                {
                    _updateTransparency = false;
                }
                else if( ea.getKey() == _exteriorTransparencyFactorKey )
                {
                    _updateExteriorTransparencyFactor = false;
                }
                else if( ea.getKey() == _alphaFuncKey )
                {
                    _updateAlphaCutOff = false;
                }
                else if( ea.getKey() == _sampleDensityKey )
                {
                    _updateSampleDensity = false;
                }
                break;
            }
        default :
            break;
    }

    if( passOnUpdates )
    {
        float v = ( ea.getY() - ea.getYmin() ) / ( ea.getYmax() - ea.getYmin() );
        if( ea.getMouseYOrientation() == osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS )
        {
            v = 1.0F - v;
        }

        float v2            = v * v;
        float sampleRatio   = powf( ( 1.0F - v ) * 2.0F, 3.0F );
        float sampleDensity = static_cast<float>( 1.0 / sampleRatio ) / 512.0F;

        if( _updateAlphaCutOff && cpv._isoProperty.valid() )
        {
            OSG_NOTICE << "Setting isoProperty to " << v << std::endl;
            cpv._isoProperty->setValue( v );
        }

        if( _updateAlphaCutOff && cpv._afProperty.valid() )
        {
            OSG_NOTICE << "Setting afProperty to " << v2 << std::endl;
            cpv._afProperty->setValue( v2 );
        }

        if( _updateTransparency && cpv._transparencyProperty.valid() )
        {
            cpv._transparencyProperty->setValue( ( 1.0F - v2 ) * 2.0F );
            OSG_NOTICE << "Setting transparency to "
                       << cpv._transparencyProperty->getValue() << std::endl;
        }

        if( _updateExteriorTransparencyFactor &&
            cpv._exteriorTransparencyFactorProperty.valid() )
        {
            cpv._exteriorTransparencyFactorProperty->setValue( 1.0F - v );
            OSG_NOTICE << "Setting exterior transparency factor to "
                       << cpv._exteriorTransparencyFactorProperty->getValue()
                       << std::endl;
        }

        if( _updateSampleDensity && cpv._sampleDensityProperty.valid() )
        {
            OSG_NOTICE << "Setting sample density to " << sampleDensity << std::endl;
            cpv._sampleDensityProperty->setValue( sampleDensity );
        }
        if( _updateSampleDensity && cpv._sampleDensityWhenMovingProperty.valid() )
        {
            OSG_INFO << "Setting sample density when moving to " << sampleDensity
                     << std::endl;
            cpv._sampleDensityWhenMovingProperty->setValue( sampleDensity );
        }

        if( _updateSampleDensity && cpv._sampleRatioProperty.valid() )
        {
            OSG_NOTICE << "Setting sample ratio to " << sampleRatio << std::endl;
            cpv._sampleRatioProperty->setValue( sampleRatio );
        }

        if( _updateSampleDensity && cpv._sampleRatioWhenMovingProperty.valid() )
        {
            OSG_NOTICE << "Setting sample ratio to " << sampleRatio << std::endl;
            cpv._sampleRatioWhenMovingProperty->setValue( sampleRatio );
        }
    }

    return false;
}
