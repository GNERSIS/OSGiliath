/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Configuration for view frustum culling behavior: small feature
 * threshold, near/far computation mode, and culling mask.
 */
#pragma once

#include <cfloat>
#include <iosfwd>
#include <osg/maths/mat4.hpp>
#include <osg/nodes/ClearNode.hpp>

namespace osg
{

    // forward declare
    class ArgumentParser;
    class ApplicationUsage;

    class OSG_EXPORT CullSettings
    {
        public:

            CullSettings()
            {
                setDefaults();
                readEnvironmentalVariables();
            }

            CullSettings( ArgumentParser& arguments )
            {
                setDefaults();
                readEnvironmentalVariables();
                readCommandLine( arguments );
            }

            CullSettings( const CullSettings& cs );

            virtual ~CullSettings()
            {
            }

            CullSettings&
            operator=( const CullSettings& settings )
            {
                if( this == &settings )
                {
                    return *this;
                }
                setCullSettings( settings );
                return *this;
            }

            virtual void
            setDefaults();

            enum VariablesMask
            {
                COMPUTE_NEAR_FAR_MODE                = ( 0X1 << 0 ),
                CULLING_MODE                         = ( 0X1 << 1 ),
                LOD_SCALE                            = ( 0X1 << 2 ),
                SMALL_FEATURE_CULLING_PIXEL_SIZE     = ( 0X1 << 3 ),
                CLAMP_PROJECTION_MATRIX_CALLBACK     = ( 0X1 << 4 ),
                NEAR_FAR_RATIO                       = ( 0X1 << 5 ),
                IMPOSTOR_ACTIVE                      = ( 0X1 << 6 ),
                DEPTH_SORT_IMPOSTOR_SPRITES          = ( 0X1 << 7 ),
                IMPOSTOR_PIXEL_ERROR_THRESHOLD       = ( 0X1 << 8 ),
                NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES = ( 0X1 << 9 ),
                CULL_MASK                            = ( 0X1 << 10 ),
                CULL_MASK_LEFT                       = ( 0X1 << 11 ),
                CULL_MASK_RIGHT                      = ( 0X1 << 12 ),
                CLEAR_COLOR                          = ( 0X1 << 13 ),
                CLEAR_MASK                           = ( 0X1 << 14 ),
                LIGHTING_MODE                        = ( 0X1 << 15 ),
                LIGHT                                = ( 0X1 << 16 ),
                DRAW_BUFFER                          = ( 0X1 << 17 ),
                READ_BUFFER                          = ( 0X1 << 18 ),

                NO_VARIABLES                         = 0X00'00'00'00,
                ALL_VARIABLES                        = 0X7F'FF'FF'FF,
            };

            typedef int InheritanceMask;

            /** Set the inheritance mask used in inheritCullSettings to control which
             * variables get overwritten by the passed in CullSettings object.*/
            void
            setInheritanceMask( InheritanceMask mask )
            {
                _inheritanceMask = mask;
            }

            /** Get the inheritance mask used in inheritCullSettings to control which
             * variables get overwritten by the passed in CullSettings object.*/
            InheritanceMask
            getInheritanceMask() const
            {
                return _inheritanceMask;
            }

            /** Set the local cull settings values from specified CullSettings object.*/
            void
            setCullSettings( const CullSettings& settings );

            /** Inherit the local cull settings variable from specified CullSettings
             * object, according to the inheritance mask.*/
            virtual void
            inheritCullSettings( const CullSettings& settings )
            {
                inheritCullSettings( settings,
                                     static_cast<unsigned int>( _inheritanceMask ) );
            }

            /** Inherit the local cull settings variable from specified CullSettings
             * object, according to the inheritance mask.*/
            virtual void
            inheritCullSettings( const CullSettings& settings,
                                 unsigned int        inheritanceMask );

            /** read the environmental variables.*/
            void
            readEnvironmentalVariables();

            /** read the commandline arguments.*/
            void
            readCommandLine( ArgumentParser& arguments );

            enum InheritanceMaskActionOnAttributeSetting
            {
                DISABLE_ASSOCIATED_INHERITANCE_MASK_BIT,
                DO_NOT_MODIFY_INHERITANCE_MASK,
            };

            void
            setInheritanceMaskActionOnAttributeSetting(
                InheritanceMaskActionOnAttributeSetting action
            )
            {
                _inheritanceMaskActionOnAttributeSetting = action;
            }

            InheritanceMaskActionOnAttributeSetting
            getInheritanceMaskActionOnAttributeSetting() const
            {
                return _inheritanceMaskActionOnAttributeSetting;
            }

            /** Apply the action, specified by the
             * InheritanceMaskActionOnAttributeSetting, to apply to the inheritance bit
             * mask. This method is called by CullSettings::set*() parameter methods to
             * ensure that CullSettings inheritance mechanisms doesn't overwrite the
             * local parameter settings.*/
            inline void
            applyMaskAction( unsigned int maskBit )
            {
                if( _inheritanceMaskActionOnAttributeSetting ==
                    DISABLE_ASSOCIATED_INHERITANCE_MASK_BIT )
                {
                    _inheritanceMask = _inheritanceMask & static_cast<int>( ~maskBit );
                }
            }

            /** Switch the creation of Impostors on or off.
             * Setting active to false forces the CullVisitor to use the Impostor
             * LOD children for rendering. Setting active to true forces the
             * CullVisitor to create the appropriate pre-rendering stages which
             * render to the ImpostorSprite's texture.*/
            void
            setImpostorsActive( bool active )
            {
                _impostorActive = active;
                applyMaskAction( IMPOSTOR_ACTIVE );
            }

            /** Get whether impostors are active or not. */
            bool
            getImpostorsActive() const
            {
                return _impostorActive;
            }

            /** Set the impostor error threshold.
             * Used in calculation of whether impostors remain valid.*/
            void
            setImpostorPixelErrorThreshold( float numPixels )
            {
                _impostorPixelErrorThreshold = numPixels;
                applyMaskAction( IMPOSTOR_PIXEL_ERROR_THRESHOLD );
            }

            /** Get the impostor error threshold.*/
            float
            getImpostorPixelErrorThreshold() const
            {
                return _impostorPixelErrorThreshold;
            }

            /** Set whether ImpostorSprite's should be placed in a depth sorted bin for
             * rendering.*/
            void
            setDepthSortImpostorSprites( bool doDepthSort )
            {
                _depthSortImpostorSprites = doDepthSort;
                applyMaskAction( DEPTH_SORT_IMPOSTOR_SPRITES );
            }

            /** Get whether ImpostorSprite's are depth sorted bin for rendering.*/
            bool
            getDepthSortImpostorSprites() const
            {
                return _depthSortImpostorSprites;
            }

            /** Set the number of frames that an ImpostorSprite is kept whilst not being
             * beyond, before being recycled.*/
            void
            setNumberOfFrameToKeepImpostorSprites( int numFrames )
            {
                _numFramesToKeepImpostorSprites = numFrames;
                applyMaskAction( NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES );
            }

            /** Get the number of frames that an ImpostorSprite is kept whilst not being
             * beyond, before being recycled.*/
            int
            getNumberOfFrameToKeepImpostorSprites() const
            {
                return _numFramesToKeepImpostorSprites;
            }

            enum ComputeNearFarMode
            {
                DO_NOT_COMPUTE_NEAR_FAR = 0,
                COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES,
                COMPUTE_NEAR_FAR_USING_PRIMITIVES,
                COMPUTE_NEAR_USING_PRIMITIVES,
            };

            void
            setComputeNearFarMode( ComputeNearFarMode cnfm )
            {
                _computeNearFar = cnfm;
                applyMaskAction( COMPUTE_NEAR_FAR_MODE );
            }

            ComputeNearFarMode
            getComputeNearFarMode() const
            {
                return _computeNearFar;
            }

            void
            setNearFarRatio( double ratio )
            {
                _nearFarRatio = ratio;
                applyMaskAction( NEAR_FAR_RATIO );
            }

            double
            getNearFarRatio() const
            {
                return _nearFarRatio;
            }

            enum CullingModeValues
            {
                NO_CULLING                 = 0X0,
                VIEW_FRUSTUM_SIDES_CULLING = 0X1,
                NEAR_PLANE_CULLING         = 0X2,
                FAR_PLANE_CULLING          = 0X4,
                VIEW_FRUSTUM_CULLING       = VIEW_FRUSTUM_SIDES_CULLING |
                    NEAR_PLANE_CULLING |
                    FAR_PLANE_CULLING,
                SMALL_FEATURE_CULLING    = 0X8,
                SHADOW_OCCLUSION_CULLING = 0X10,
                CLUSTER_CULLING          = 0X20,
                DEFAULT_CULLING          = VIEW_FRUSTUM_SIDES_CULLING |
                    SMALL_FEATURE_CULLING |
                    SHADOW_OCCLUSION_CULLING |
                    CLUSTER_CULLING,
                ENABLE_ALL_CULLING = VIEW_FRUSTUM_CULLING |
                    SMALL_FEATURE_CULLING |
                    SHADOW_OCCLUSION_CULLING |
                    CLUSTER_CULLING,
            };

            typedef int CullingMode;

            /** Set the culling mode for the CullVisitor to use.*/
            void
            setCullingMode( CullingMode mode )
            {
                _cullingMode = mode;
                applyMaskAction( CULLING_MODE );
            }

            /** Returns the current CullingMode.*/
            CullingMode
            getCullingMode() const
            {
                return _cullingMode;
            }

            void
            setCullMask( osg::Node::NodeMask nm )
            {
                _cullMask = nm;
                applyMaskAction( CULL_MASK );
            }

            osg::Node::NodeMask
            getCullMask() const
            {
                return _cullMask;
            }

            void
            setCullMaskLeft( osg::Node::NodeMask nm )
            {
                _cullMaskLeft = nm;
                applyMaskAction( CULL_MASK_LEFT );
            }

            osg::Node::NodeMask
            getCullMaskLeft() const
            {
                return _cullMaskLeft;
            }

            void
            setCullMaskRight( osg::Node::NodeMask nm )
            {
                _cullMaskRight = nm;
                applyMaskAction( CULL_MASK_RIGHT );
            }

            osg::Node::NodeMask
            getCullMaskRight() const
            {
                return _cullMaskRight;
            }

            /** Set the LOD bias for the CullVisitor to use.*/
            void
            setLODScale( float scale )
            {
                _LODScale = scale;
                applyMaskAction( LOD_SCALE );
            }

            /** Get the LOD bias.*/
            float
            getLODScale() const
            {
                return _LODScale;
            }

            /** Threshold at which small features are culled.
            \param value Bounding volume size in screen space. Default is 2.0. */
            void
            setSmallFeatureCullingPixelSize( float value )
            {
                _smallFeatureCullingPixelSize = value;
                applyMaskAction( SMALL_FEATURE_CULLING_PIXEL_SIZE );
            }

            /** Get the Small Feature Culling Pixel Size.*/
            float
            getSmallFeatureCullingPixelSize() const
            {
                return _smallFeatureCullingPixelSize;
            }

            /** Callback for overriding the CullVisitor's default clamping of the
             * projection matrix to computed near and far values. Note, both mat4 and
             * dmat4 versions of clampProjectionMatrixImplementation must be implemented
             * as the CullVisitor can target either dmat4 data type, configured at
             * compile time.*/
            struct ClampProjectionMatrixCallback : public osg::Referenced
            {
                    virtual bool
                    clampProjectionMatrixImplementation( osg::mat4& projection,
                                                         double&    znear,
                                                         double&    zfar ) const = 0;
                    virtual bool
                    clampProjectionMatrixImplementation( osg::dmat4& projection,
                                                         double&     znear,
                                                         double&     zfar ) const = 0;
            };

            /** set the ClampProjectionMatrixCallback.*/
            void
            setClampProjectionMatrixCallback( ClampProjectionMatrixCallback* cpmc )
            {
                _clampProjectionMatrixCallback = cpmc;
                applyMaskAction( CLAMP_PROJECTION_MATRIX_CALLBACK );
            }

            /** get the non const ClampProjectionMatrixCallback.*/
            ClampProjectionMatrixCallback*
            getClampProjectionMatrixCallback()
            {
                return _clampProjectionMatrixCallback.get();
            }

            /** get the const ClampProjectionMatrixCallback.*/
            const ClampProjectionMatrixCallback*
            getClampProjectionMatrixCallback() const
            {
                return _clampProjectionMatrixCallback.get();
            }

            /** Write out internal settings of CullSettings. */
            void
            write( std::ostream& out );

        protected:

            InheritanceMask _inheritanceMask;
            InheritanceMaskActionOnAttributeSetting
                               _inheritanceMaskActionOnAttributeSetting;

            ComputeNearFarMode _computeNearFar;
            CullingMode        _cullingMode;
            float              _LODScale;
            float              _smallFeatureCullingPixelSize;

            ref_ptr<ClampProjectionMatrixCallback> _clampProjectionMatrixCallback;
            double                                 _nearFarRatio;
            bool                                   _impostorActive;
            bool                                   _depthSortImpostorSprites;
            float                                  _impostorPixelErrorThreshold;
            int                                    _numFramesToKeepImpostorSprites;

            Node::NodeMask                         _cullMask;
            Node::NodeMask                         _cullMaskLeft;
            Node::NodeMask                         _cullMaskRight;
    };

    template<class matrix_type,
             class value_type>
    bool
    clampProjectionMatrix( matrix_type& projection,
                           double&      znear,
                           double&      zfar,
                           value_type   nearFarRatio )
    {
        double epsilon = 1E-6;
        if( zfar < znear - epsilon )
        {
            if( zfar != -FLT_MAX || znear != FLT_MAX )
            {
                OSG_INFO << "_clampProjectionMatrix not applied, invalid depth range, "
                            "znear = "
                         << znear << "  zfar = " << zfar << std::endl;
            }
            return false;
        }

        if( zfar < znear + epsilon )
        {
            double average = ( znear + zfar ) * 0.5;
            znear          = average - epsilon;
            zfar           = average + epsilon;
        }

        // Column-major mat4: [col][row], operator()(col, row)
        // Perspective: [2][2]=z_scale, [2][3]=-1, [3][2]=nf_term, [3][3]=0
        // Ortho:       [2][2]=z_scale, [2][3]=0,  [3][2]=z_trans, [3][3]=1

        // Detect perspective vs ortho by checking if [2][3] (the perspective divide) is
        // non-zero
        bool isPerspective = fabs( projection[2][3] ) > epsilon;

        if( !isPerspective )
        {
            // Orthographic
            value_type delta_span = static_cast<value_type>( ( zfar - znear ) * 0.02 );
            if( delta_span < 1.0F )
            {
                delta_span = 1.0F;
            }
            value_type desired_znear = static_cast<value_type>( znear - delta_span );
            value_type desired_zfar  = static_cast<value_type>( zfar + delta_span );

            znear                    = desired_znear;
            zfar                     = desired_zfar;

            // Ortho Z elements: [2][2] = -2/(f-n), [3][2] = -(f+n)/(f-n)
            projection[2][2] = static_cast<typename matrix_type::value_type>(
                -2.0 / ( desired_zfar - desired_znear )
            );
            projection[3][2] = static_cast<typename matrix_type::value_type>(
                -( desired_zfar + desired_znear ) / ( desired_zfar - desired_znear )
            );
        }
        else
        {
            // Perspective
            value_type zfarPushRatio  = 1.02F;
            value_type znearPullRatio = 0.98F;

            value_type desired_znear  = znear * znearPullRatio;
            value_type desired_zfar   = zfar * zfarPushRatio;

            double     min_near_plane = zfar * nearFarRatio;
            if( desired_znear < min_near_plane )
            {
                desired_znear = min_near_plane;
            }

            znear = desired_znear;
            zfar  = desired_zfar;

            // Perspective Z elements: [2][2] = -(f+n)/(f-n), [2][3] = -1, [3][2] =
            // -2fn/(f-n), [3][3] = 0 Compute the NDC Z range for desired near/far
            [[maybe_unused]]
            value_type trans_near_plane = static_cast<value_type>(
                ( -desired_znear * projection[2][2] + projection[3][2] ) /
                ( -desired_znear * projection[2][3] + projection[3][3] )
            );
            [[maybe_unused]]
            value_type trans_far_plane = static_cast<value_type>(
                ( -desired_zfar * projection[2][2] + projection[3][2] ) /
                ( -desired_zfar * projection[2][3] + projection[3][3] )
            );

            // ratio and center were computed here but are unused

            // Directly replace Z column values for the desired near/far
            // [2][2] = -(f+n)/(f-n), [3][2] = -2fn/(f-n)
            // [2][3] = -1 (perspective divide, unchanged)
            projection[2][2] = static_cast<typename matrix_type::value_type>(
                -( desired_zfar + desired_znear ) / ( desired_zfar - desired_znear )
            );
            projection[3][2] = static_cast<typename matrix_type::value_type>(
                -2.0 * desired_zfar * desired_znear / ( desired_zfar - desired_znear )
            );
        }
        return true;
    }

}
