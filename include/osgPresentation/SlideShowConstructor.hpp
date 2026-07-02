/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Builds slide show presentations from script descriptions.
 * Creates scene graph slides with text, images, models, and transitions.
 */
#pragma once

#include <osg/images/ImageUtils.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/nodes/ClearNode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Switch.hpp>
#include <osg/textures/ImageSequence.hpp>
#include <osg/textures/ImageStream.hpp>
#include <osg/textures/TransferFunction.hpp>
#include <osg/traversal/AnimationPath.hpp>
#include <osg/traversal/ScriptEngine.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgGA/events/GUIEventAdapter.hpp>
#include <osgPresentation/AnimationMaterial.hpp>
#include <osgPresentation/PropertyManager.hpp>
#include <osgPresentation/SlideEventHandler.hpp>
#include <osgPresentation/Timeout.hpp>
#include <osgText/Text.hpp>
#include <osgVolume/VolumeSettings.hpp>
#include <osgVolume/VolumeTile.hpp>

namespace osgPresentation
{

    class OSGPRESENTATION_EXPORT HUDTransform : public osg::Transform
    {
        public:

            HUDTransform( HUDSettings* hudSettings );

            virtual bool
            computeLocalToWorldMatrix( osg::dmat4&       matrix,
                                       osg::NodeVisitor* nv ) const;

            virtual bool
            computeWorldToLocalMatrix( osg::dmat4& matrix,
                                       osg::NodeVisitor* ) const;

        protected:

            virtual ~HUDTransform();

            osg::ref_ptr<HUDSettings> _hudSettings;
    };

    class OSGPRESENTATION_EXPORT SlideShowConstructor
    {
        public:

            enum CoordinateFrame
            {
                SLIDE,
                MODEL
            };

            LayerAttributes*
            getOrCreateLayerAttributes( osg::Node* node );

            void
            setDuration( osg::Node* node,
                         double     duration )
            {
                getOrCreateLayerAttributes( node )->setDuration( duration );
            }

            void
            addKey( osg::Node*         node,
                    const KeyPosition& kp )
            {
                getOrCreateLayerAttributes( node )->addKey( kp );
            }

            void
            addRunString( osg::Node*         node,
                          const std::string& runString )
            {
                getOrCreateLayerAttributes( node )->addRunString( runString );
            }

            void
            setJump( osg::Node*      node,
                     const JumpData& jumpData )
            {
                getOrCreateLayerAttributes( node )->setJump( jumpData );
            }

            void
            addPresentationKey( const KeyPosition& kp )
            {
                if( !_presentationSwitch )
                {
                    createPresentation();
                }
                if( _presentationSwitch.valid() )
                {
                    addKey( _presentationSwitch.get(), kp );
                }
            }

            void
            addPresentationRunString( const std::string& runString )
            {
                if( !_presentationSwitch )
                {
                    createPresentation();
                }
                if( _presentationSwitch.valid() )
                {
                    addRunString( _presentationSwitch.get(), runString );
                }
            }

            void
            addSlideKey( const KeyPosition& kp )
            {
                if( !_slide )
                {
                    addSlide();
                }
                if( _slide.valid() )
                {
                    addKey( _slide.get(), kp );
                }
            }

            void
            addSlideRunString( const std::string& runString )
            {
                if( !_slide )
                {
                    addSlide();
                }
                if( _slide.valid() )
                {
                    addRunString( _slide.get(), runString );
                }
            }

            void
            setSlideJump( const JumpData& jumpData )
            {
                if( !_slide )
                {
                    addSlide();
                }
                if( _slide.valid() )
                {
                    setJump( _slide.get(), jumpData );
                }
            }

            void
            addLayerKey( const KeyPosition& kp )
            {
                if( !_currentLayer )
                {
                    addLayer();
                }
                if( _currentLayer.valid() )
                {
                    addKey( _currentLayer.get(), kp );
                }
            }

            void
            addLayerRunString( const std::string& runString )
            {
                if( !_currentLayer )
                {
                    addLayer();
                }
                if( _currentLayer.valid() )
                {
                    addRunString( _currentLayer.get(), runString );
                }
            }

            void
            setLayerJump( const JumpData& jumpData )
            {
                if( !_currentLayer )
                {
                    addLayer();
                }
                if( _currentLayer.valid() )
                {
                    setJump( _currentLayer.get(), jumpData );
                }
            }

            struct PositionData
            {
                    PositionData() :
                        frame( SlideShowConstructor::SLIDE ),
                        position( 0.0F,
                                  1.0F,
                                  0.0F ),
                        // position(0.5f,0.5f,0.0f),
                        scale( 1.0F,
                               1.0F,
                               1.0F ),
                        rotate( 0.0F,
                                0.0F,
                                0.0F,
                                1.0F ),
                        rotation( 0.0F,
                                  0.0F,
                                  1.0F,
                                  0.0F ),
                        absolute_path( false ),
                        inverse_path( false ),
                        path_time_offset( 0.0 ),
                        path_time_multiplier( 1.0F ),
                        path_loop_mode( osg::AnimationPath::NO_LOOPING ),
                        animation_material_time_offset( 0.0 ),
                        animation_material_time_multiplier( 1.0 ),
                        animation_material_loop_mode( AnimationMaterial::NO_LOOPING ),
                        autoRotate( false ),
                        autoScale( false ),
                        hud( false )
                    {
                    }

                    bool
                    requiresPosition() const
                    {
                        return ( position[0] !=
                                 0.0F ||
                                 position[1] !=
                                 1.0F ||
                                 position[2] != 1.0F );
                    }

                    bool
                    requiresScale() const
                    {
                        return (
                            scale[0] != 1.0F || scale[1] != 1.0F || scale[2] != 1.0F
                        );
                    }

                    bool
                    requiresRotate() const
                    {
                        return rotate[0] != 0.0F;
                    }

                    bool
                    requiresAnimation() const
                    {
                        return ( rotation[0] != 0.0F || !path.empty() );
                    }

                    bool
                    requiresMaterialAnimation() const
                    {
                        return !animation_material_filename.empty() || !fade.empty();
                    }

                    CoordinateFrame              frame;
                    osg::vec3                    position;
                    osg::vec3                    scale;
                    osg::vec4                    rotate;
                    osg::vec4                    rotation;
                    std::string                  animation_name;
                    bool                         absolute_path;
                    bool                         inverse_path;
                    double                       path_time_offset;
                    double                       path_time_multiplier;
                    osg::AnimationPath::LoopMode path_loop_mode;
                    std::string                  path;
                    double                       animation_material_time_offset;
                    double                       animation_material_time_multiplier;
                    AnimationMaterial::LoopMode  animation_material_loop_mode;
                    std::string                  animation_material_filename;
                    std::string                  fade;
                    bool                         autoRotate;
                    bool                         autoScale;
                    bool                         hud;
            };

            struct ModelData
            {
                    ModelData()
                    {
                    }

                    std::string region;
                    std::string effect;
                    std::string options;
            };

            struct ImageData
            {
                    ImageData() :
                        width( 1.0F ),
                        height( 1.0F ),
                        region( 0.0F,
                                0.0F,
                                1.0F,
                                1.0F ),
                        region_in_pixel_coords( false ),
                        texcoord_rotate( 0.0F ),
                        loopingMode( osg::ImageStream::NO_LOOPING ),
                        page( -1 ),
                        backgroundColor( 1.0F,
                                         1.0F,
                                         1.0F,
                                         1.0F ),
                        fps( 30.0 ),
                        duration( -1.0 ),
                        imageSequence( false ),
                        imageSequencePagingMode(
                            osg::ImageSequence::PAGE_AND_DISCARD_USED_IMAGES
                        ),
                        imageSequenceInteractionMode( PLAY_AUTOMATICALLY_LIKE_MOVIE ),
                        blendingHint( USE_IMAGE_ALPHA ),
                        delayTime( 0.0 ),
                        startTime( 0.0 ),
                        stopTime( -1.0 ),
                        volume( "" )
                    {
                    }

                    std::string                   options;
                    float                         width;
                    float                         height;
                    osg::vec4                     region;
                    bool                          region_in_pixel_coords;
                    float                         texcoord_rotate;
                    osg::ImageStream::LoopingMode loopingMode;
                    int                           page;
                    osg::vec4                     backgroundColor;
                    double                        fps;
                    double                        duration;

                    bool                          imageSequence;
                    osg::ImageSequence::Mode      imageSequencePagingMode;

                    enum ImageSequenceInteractionMode
                    {
                        PLAY_AUTOMATICALLY_LIKE_MOVIE,
                        USE_MOUSE_X_POSITION,
                        USE_MOUSE_Y_POSITION
                    };

                    ImageSequenceInteractionMode imageSequenceInteractionMode;

                    enum BlendingHint
                    {
                        USE_IMAGE_ALPHA,
                        OFF,
                        ON
                    };

                    BlendingHint blendingHint;

                    double       delayTime;
                    double       startTime;
                    double       stopTime;
                    std::string  volume;
            };

            struct VolumeData
            {
                    typedef osgVolume::VolumeSettings::ShadingModel ShadingModel;
                    typedef osgVolume::VolumeSettings::Technique    Technique;

                    VolumeData() :
                        shadingModel( osgVolume::VolumeSettings::Standard ),
                        useTabbedDragger( false ),
                        useTrackballDragger( false ),
                        region_in_pixel_coords( false ),
                        alphaValue(),
                        cutoffValue(),
                        exteriorTransparencyFactorValue(),
                        sampleDensityValue(),
                        sampleRatioValue(),
                        colorSpaceOperation( osg::NO_COLOR_SPACE_OPERATION ),
                        colorModulate( 1.0F,
                                       1.0F,
                                       1.0F,
                                       1.0F ),
                        technique( osgVolume::VolumeSettings::RayTraced )
                    {
                        hullPositionData.position = osg::vec3( 0.0, 0.0, 0.0 );
                        hullPositionData.frame =
                            osgPresentation::SlideShowConstructor::MODEL;
                    }

                    osg::ref_ptr<osgVolume::VolumeSettings> volumeSettings;

                    std::string                             options;
                    ShadingModel                            shadingModel;
                    osg::ref_ptr<osg::TransferFunction1D>   transferFunction;
                    bool                                    useTabbedDragger;
                    bool                                    useTrackballDragger;
                    std::string                             region;
                    bool                                    region_in_pixel_coords;
                    std::string                             alphaValue;
                    std::string                             cutoffValue;
                    std::string              exteriorTransparencyFactorValue;
                    std::string              sampleDensityValue;
                    std::string              sampleDensityWhenMovingValue;

                    std::string              sampleRatioValue;
                    std::string              sampleRatioWhenMovingValue;

                    osg::ColorSpaceOperation colorSpaceOperation;
                    osg::vec4                colorModulate;
                    Technique                technique;
                    std::string              hull;
                    PositionData             hullPositionData;
            };

            struct FontData
            {
                    FontData() :
                        font( "fonts/arial.ttf" ),
                        layout( osgText::Text::LEFT_TO_RIGHT ),
                        alignment( osgText::Text::LEFT_BASE_LINE ),
                        axisAlignment( osgText::Text::XZ_PLANE ),
                        characterSizeMode( osgText::Text::OBJECT_COORDS ),
                        characterSize( 0.04F ),
                        maximumHeight( 1.0F ),
                        maximumWidth( 1.0F ),
                        color( 1.0F,
                               1.0F,
                               1.0F,
                               1.0F )
                    {
                    }

                    std::string                      font;
                    osgText::Text::Layout            layout;
                    osgText::Text::AlignmentType     alignment;
                    osgText::Text::AxisAlignment     axisAlignment;
                    osgText::Text::CharacterSizeMode characterSizeMode;
                    float                            characterSize;
                    float                            maximumHeight;
                    float                            maximumWidth;
                    osg::vec4                        color;
            };

            enum ScriptCallbackType
            {
                UPDATE_SCRIPT,
                EVENT_SCRIPT
            };

            typedef std::pair<ScriptCallbackType, std::string> ScriptPair;

            struct ScriptData
            {
                    ScriptData()
                    {
                    }

                    typedef std::vector<ScriptPair> Scripts;
                    Scripts                         scripts;

                    bool
                    hasScripts() const
                    {
                        return !scripts.empty();
                    }
            };

            SlideShowConstructor( osgDB::Options* options );

            void
            createPresentation();

            void
            setBackgroundColor( const osg::vec4& color,
                                bool             updateClearNode );

            const osg::vec4&
            getBackgroundColor() const
            {
                return _backgroundColor;
            }

            void
            setTextColor( const osg::vec4& color );

            const osg::vec4&
            getTextColor() const
            {
                return _textFontDataDefault.color;
            }

            void
            setPresentationName( const std::string& name );

            void
            setPresentationAspectRatio( float aspectRatio );

            void
            setPresentationAspectRatio( const std::string& str );

            void
            setPresentationDuration( double duration );

            void
            addScriptEngine( const std::string& scriptEngineName );

            void
            addScriptFile( const std::string& name,
                           const std::string& filename );

            void
            addScript( const std::string& name,
                       const std::string& language,
                       const std::string& script );

            void
            addSlide();

            void
            selectSlide( int slideNum );

            void
            setSlideTitle( const std::string& name,
                           PositionData&      positionData,
                           FontData&          fontData )
            {
                _titlePositionData = positionData;
                _titleFontData     = fontData;
                _slideTitle        = name;
            }

            void
            setSlideBackgrondHUD( bool hud )
            {
                _slideBackgroundAsHUD = hud;
            }

            void
            setSlideBackground( const std::string& name )
            {
                _slideBackgroundImageFileName = name;
            }

            void
            setSlideDuration( double duration );

            Timeout*
            addTimeout();

            void
            addLayer( bool inheritPreviousLayers = true,
                      bool defineAsBaseLayer     = false );

            void
            selectLayer( int layerNum );

            void
            setLayerDuration( double duration );

            // title settings
            FontData&
            getTitleFontData()
            {
                return _titleFontData;
            }

            FontData&
            getTitleFontDataDefault()
            {
                return _titleFontDataDefault;
            }

            PositionData&
            getTitlePositionData()
            {
                return _titlePositionData;
            }

            PositionData&
            getTitlePositionDataDefault()
            {
                return _titlePositionDataDefault;
            }

            // text settings
            FontData&
            getTextFontData()
            {
                return _textFontData;
            }

            FontData&
            getTextFontDataDefault()
            {
                return _textFontDataDefault;
            }

            PositionData&
            getTextPositionData()
            {
                return _textPositionData;
            }

            PositionData&
            getTextPositionDataDefault()
            {
                return _textPositionDataDefault;
            }

            void
            translateTextCursor( const osg::vec3& delta )
            {
                _textPositionData.position += delta;
            }

            // image settings
            PositionData&
            getImagePositionData()
            {
                return _imagePositionData;
            }

            PositionData&
            getImagePositionDataDefault()
            {
                return _imagePositionDataDefault;
            }

            // model settings
            PositionData&
            getModelPositionData()
            {
                return _modelPositionData;
            }

            PositionData&
            getModelPositionDataDefault()
            {
                return _modelPositionDataDefault;
            }

            enum PresentationContext
            {
                CURRENT_PRESENTATION,
                CURRENT_SLIDE,
                CURRENT_LAYER
            };

            void
            addEventHandler( PresentationContext                  presentationContext,
                             osg::ref_ptr<osgGA::GUIEventHandler> handler );

            void
            keyToDoOperation( PresentationContext presentationContext,
                              int                 key,
                              Operation           operation,
                              const JumpData&     jumpData = JumpData() );
            void
            keyToDoOperation( PresentationContext presentationContext,
                              int                 key,
                              const std::string&  command,
                              Operation           operation,
                              const JumpData&     jumpData = JumpData() );
            void
            keyEventOperation( PresentationContext presentationContext,
                               int                 key,
                               const KeyPosition&  keyPos,
                               const JumpData&     jumpData = JumpData() );

            void
            layerClickToDoOperation( Operation       operation,
                                     const JumpData& jumpData = JumpData() );
            void
            layerClickToDoOperation( const std::string& command,
                                     Operation          operation,
                                     const JumpData&    jumpData = JumpData() );
            void
            layerClickEventOperation( const KeyPosition& keyPos,
                                      const JumpData&    jumpData = JumpData() );

            void
            addPropertyAnimation( PresentationContext presentationContext,
                                  PropertyAnimation*  propertyAnimation );

            void
            addScriptCallback( PresentationContext presentationContext,
                               ScriptCallbackType  scriptCallbackType,
                               const std::string&  functionName );

            void
            addScriptToNode( ScriptCallbackType scriptCallbackType,
                             const std::string& name,
                             osg::Node*         node );

            void
            addScriptsToNode( const ScriptData& scriptData,
                              osg::Node*        node );

            void
            addToCurrentLayer( osg::Node* subgraph );

            void
            addBullet( const std::string& bullet,
                       PositionData&      positionData,
                       FontData&          fontData,
                       const ScriptData&  scriptData );

            void
            addParagraph( const std::string& paragraph,
                          PositionData&      positionData,
                          FontData&          fontData,
                          const ScriptData&  scriptData );

            osg::ref_ptr<osg::Image>
            readImage( const std::string& filename,
                       const ImageData&   imageData );

            void
            addImage( const std::string&  filename,
                      const PositionData& positionData,
                      const ImageData&    imageData,
                      const ScriptData&   scriptData );

            void
            addStereoImagePair( const std::string&  filenameLeft,
                                const ImageData&    imageDataLeft,
                                const std::string&  filenameRight,
                                const ImageData&    imageDataRight,
                                const PositionData& positionData,
                                const ScriptData&   scriptData );

            void
            addGraph( const std::string&  filename,
                      const PositionData& positionData,
                      const ImageData&    imageData,
                      const ScriptData&   scriptData );
            void
            addVNC( const std::string&  filename,
                    const PositionData& positionData,
                    const ImageData&    imageData,
                    const std::string&  password,
                    const ScriptData&   scriptData );
            void
            addBrowser( const std::string&  filename,
                        const PositionData& positionData,
                        const ImageData&    imageData,
                        const ScriptData&   scriptData );
            void
            addPDF( const std::string&  filename,
                    const PositionData& positionData,
                    const ImageData&    imageData,
                    const ScriptData&   scriptData );
            osg::ref_ptr<osg::Image>
            addInteractiveImage( const std::string&  filename,
                                 const PositionData& positionData,
                                 const ImageData&    imageData,
                                 const ScriptData&   scriptData );

            void
            addModel( osg::Node*          subgraph,
                      const PositionData& positionData,
                      const ModelData&    modelData,
                      const ScriptData&   scriptData );

            void
            addModel( const std::string&  filename,
                      const PositionData& positionData,
                      const ModelData&    modelData,
                      const ScriptData&   scriptData );

            void
            setUpVolumeScalarProperty( osgVolume::VolumeTile*     tile,
                                       osgVolume::ScalarProperty* property,
                                       const std::string&         source );

            void
            addVolume( const std::string&  filename,
                       const PositionData& positionData,
                       const VolumeData&   volumeData,
                       const ScriptData&   scriptData );

            osg::Group*
            takePresentation()
            {
                return _root.release();
            }

            osg::Group*
            getPresentation()
            {
                return _root.get();
            }

            osg::Switch*
            getPresentationSwitch()
            {
                return _presentationSwitch.get();
            }

            osg::Switch*
            getCurrentSlide()
            {
                return _slide.get();
            }

            void
            pushCurrentLayer( osg::Group* newLayerGroup );
            void
            popCurrentLayer();

            osg::Group*
            getCurrentLayer()
            {
                return _currentLayer.get();
            }

            void
            setLoopPresentation( bool loop )
            {
                _loopPresentation = loop;
            }

            bool
            getLoopPresentation() const
            {
                return _loopPresentation;
            }

            void
            setAutoSteppingActive( bool flag = true )
            {
                _autoSteppingActive = flag;
            }

            bool
            getAutoSteppingActive() const
            {
                return _autoSteppingActive;
            }

            void
            setHUDSettings( HUDSettings* hudSettings )
            {
                _hudSettings = hudSettings;
            }

            HUDSettings*
            getHUDSettings()
            {
                return _hudSettings.get();
            }

            const HUDSettings*
            getHUDSettings() const
            {
                return _hudSettings.get();
            }

            osg::ScriptEngine*
            getOrCreateScriptEngine( const std::string& language );

        protected:

            void
            findImageStreamsAndAddCallbacks( osg::Node* node );

            osg::Geometry*
            createTexturedQuadGeometry( const osg::vec3& pos,
                                        const osg::vec4& rotation,
                                        float            width,
                                        float            height,
                                        osg::Image*      image,
                                        bool&            usedTextureRectangle );
            void
            setUpMovieVolume( osg::Node*        subgraph,
                              osg::ImageStream* imageStream,
                              const ImageData&  imageData );

            osg::vec3
            computePositionInModelCoords( const PositionData& positionData ) const;
            void
            updatePositionFromInModelCoords( const osg::vec3& vertex,
                                             PositionData&    positionData ) const;

            osg::vec3
            convertSlideToModel( const osg::vec3& position ) const;
            osg::vec3
            convertModelToSlide( const osg::vec3& position ) const;

            osg::AnimationPathCallback*
            getAnimationPathCallback( const PositionData& positionData );

            osg::Node*
            attachMaterialAnimation( osg::Node*          model,
                                     const PositionData& positionData );

            osg::StateSet*
            createTransformStateSet()
            {
                osg::StateSet* stateset = new osg::StateSet;
                return stateset;
            }

            osg::Node*
            decorateSubgraphForPosition( osg::Node*    node,
                                         PositionData& positionData );
            osg::Node*
            decorateSubgraphForPositionAndAnimation( osg::Node*          node,
                                                     const PositionData& positionData );

            osg::ref_ptr<osgDB::Options>                   _options;

            osg::vec3                                      _slideOrigin;
            osg::vec3                                      _eyeOrigin;
            double                                         _slideWidth;
            double                                         _slideHeight;
            double                                         _slideDistance;
            unsigned int                                   _leftEyeMask;
            unsigned int                                   _rightEyeMask;

            osg::ref_ptr<HUDSettings>                      _hudSettings;

            // title settings
            FontData                                       _titleFontData;
            FontData                                       _titleFontDataDefault;

            PositionData                                   _titlePositionData;
            PositionData                                   _titlePositionDataDefault;

            // text settings
            FontData                                       _textFontData;
            FontData                                       _textFontDataDefault;

            PositionData                                   _textPositionData;
            PositionData                                   _textPositionDataDefault;

            // image settings
            PositionData                                   _imagePositionData;
            PositionData                                   _imagePositionDataDefault;

            // model settings
            PositionData                                   _modelPositionData;
            PositionData                                   _modelPositionDataDefault;

            bool                                           _loopPresentation;
            bool                                           _autoSteppingActive;
            osg::vec4                                      _backgroundColor;
            std::string                                    _presentationName;
            double                                         _presentationDuration;

            osg::ref_ptr<osgPresentation::PropertyManager> _propertyManager;
            osg::ref_ptr<osgPresentation::PropertyEventCallback> _propertyEventCallback;

            osg::ref_ptr<osg::Group>                             _root;
            osg::ref_ptr<osg::Switch>                            _presentationSwitch;

            typedef std::map<std::string, osg::ref_ptr<osg::ScriptEngine>>
                                                                     ScriptEngineMap;
            ScriptEngineMap                                          _scriptEngines;

            typedef std::map<std::string, osg::ref_ptr<osg::Script>> ScriptMap;
            ScriptMap                                                _scripts;

            osg::ref_ptr<osg::ClearNode>                             _slideClearNode;
            osg::ref_ptr<osg::Switch>                                _slide;
            std::string                                              _slideTitle;
            std::string                                   _slideBackgroundImageFileName;
            bool                                          _slideBackgroundAsHUD;

            osg::ref_ptr<osg::Group>                      _previousLayer;
            osg::ref_ptr<osg::Group>                      _currentLayer;
            typedef std::vector<osg::ref_ptr<osg::Group>> LayerStack;
            LayerStack                                    _layerStack;

            osg::ref_ptr<FilePathData>                    _filePathData;

            osg::ref_ptr<osg::Group>                      _layerToApplyEventCallbackTo;

            typedef std::list<osg::ref_ptr<osgGA::GUIEventHandler>> EventHandlerList;
            EventHandlerList _currentEventCallbacksToApply;

            std::string
            findFileAndRecordPath( const std::string& filename );

            void
            recordOptionsFilePath( const osgDB::Options* options );
    };

}
