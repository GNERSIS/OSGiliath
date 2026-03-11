#include <osg/lighting/LightSource.hpp>
#include <osgDB/io/ReadFile.hpp>

#if defined( _MSC_VER )
    #pragma warning( disable : 4'505 )
    #pragma warning( default : 4'996 )
#endif
#include "fbxReader.hpp"

#include <fbxsdk.h>

osgDB::ReaderWriter::ReadResult
OsgFbxReader::readFbxLight( FbxNode* pNode,
                            int&     nLightCount )
{
    const FbxLight* fbxLight = FbxCast<FbxLight>( pNode->GetNodeAttribute() );

    if( !fbxLight )
    {
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }

    osg::Light* osgLight = new osg::Light;
    osgLight->setLightNum( nLightCount++ );

    osg::LightSource* osgLightSource = new osg::LightSource;
    osgLightSource->setLight( osgLight );

    FbxLight::EType fbxLightType =
        fbxLight->LightType.IsValid() ? fbxLight->LightType.Get() : FbxLight::ePoint;

    osgLight->setPosition(
        osg::vec4( 0, 0, 0, fbxLightType != FbxLight::eDirectional )
    );

    if( fbxLightType == FbxLight::eSpot )
    {
        double      coneAngle   = fbxLight->OuterAngle.Get();
        double      hotSpot     = fbxLight->InnerAngle.Get();
        const float MIN_HOTSPOT = 0.467532F;

        osgLight->setSpotCutoff( static_cast<float>( coneAngle ) );

        // Approximate the hotspot using the GL light exponent.
        //  This formula maps a hotspot of 180 to exponent 0 (uniform light
        //  distribution) and a hotspot of 45 to exponent 1 (effective light
        //  intensity is attenuated by the cosine of the angle between the
        //  direction of the light and the direction from the light to the vertex
        //  being lighted). A hotspot close to 0 maps to exponent 128 (maximum).
        float exponent = ( 180.0F /
                           ( std::max )( static_cast<float>( hotSpot ), MIN_HOTSPOT ) -
                           1.0F ) /
                         3.0F;
        osgLight->setSpotExponent( exponent );
    }

    if( fbxLight->DecayType.IsValid() && fbxLight->DecayStart.IsValid() )
    {
        double fbxDecayStart = fbxLight->DecayStart.Get();

        switch( fbxLight->DecayType.Get() )
        {
            case FbxLight::eNone :
                break;
            case FbxLight::eLinear :
                osgLight->setLinearAttenuation( fbxDecayStart );
                break;
            case FbxLight::eQuadratic :
            case FbxLight::eCubic :
                osgLight->setQuadraticAttenuation( fbxDecayStart );
                break;
        }
    }

    osg::vec3 osgDiffuseSpecular( 1.0F, 1.0F, 1.0F );
    osg::vec3 osgAmbient( 0.0F, 0.0F, 0.0F );
    if( fbxLight->Color.IsValid() )
    {
        FbxDouble3 fbxColor = fbxLight->Color.Get();
        osgDiffuseSpecular.set( static_cast<float>( fbxColor[0] ),
                                static_cast<float>( fbxColor[1] ),
                                static_cast<float>( fbxColor[2] ) );
    }
    if( fbxLight->Intensity.IsValid() )
    {
        osgDiffuseSpecular *= static_cast<float>( fbxLight->Intensity.Get() ) * 0.01F;
    }
    if( fbxLight->ShadowColor.IsValid() )
    {
        FbxDouble3 fbxShadowColor = fbxLight->ShadowColor.Get();
        osgAmbient.set( static_cast<float>( fbxShadowColor[0] ),
                        static_cast<float>( fbxShadowColor[1] ),
                        static_cast<float>( fbxShadowColor[2] ) );
    }

    osgLight->setDiffuse( osg::vec4( osgDiffuseSpecular, 1.0F ) );
    osgLight->setSpecular( osg::vec4( osgDiffuseSpecular, 1.0F ) );
    osgLight->setAmbient( osg::vec4( osgAmbient, 1.0F ) );

    return osgDB::ReaderWriter::ReadResult( osgLightSource );
}
