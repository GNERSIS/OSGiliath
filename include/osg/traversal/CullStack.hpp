/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stack-based traversal state for culling. Tracks modelview, projection,
 * viewport, and frustum through the scene graph hierarchy.
 */
#pragma once

#include <osg/core/fast_back_stack.hpp>
#include <osg/maths/RefMatrix.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/state/Viewport.hpp>
#include <osg/traversal/CullingSet.hpp>
#include <osg/traversal/CullSettings.hpp>

namespace osg
{

    /** A CullStack class which accumulates the current project, modelview matrices
    and the CullingSet. */
    class OSG_EXPORT CullStack : public osg::CullSettings
    {

        public:

            CullStack();
            CullStack( const CullStack& cs );

            ~CullStack();

            typedef std::vector<ShadowVolumeOccluder> OccluderList;

            void
            reset();

            void
            pushCullingSet();
            void
            popCullingSet();

            void
            setOccluderList( const ShadowVolumeOccluderList& svol )
            {
                _occluderList = svol;
            }

            ShadowVolumeOccluderList&
            getOccluderList()
            {
                return _occluderList;
            }

            const ShadowVolumeOccluderList&
            getOccluderList() const
            {
                return _occluderList;
            }

            void
            pushViewport( osg::Viewport* viewport );
            void
            popViewport();

            void
            pushProjectionMatrix( osg::RefMatrix* matrix );
            void
            popProjectionMatrix();

            void
            pushModelViewMatrix( osg::RefMatrix*           matrix,
                                 Transform::ReferenceFrame referenceFrame );
            void
            popModelViewMatrix();

            inline float
            getFrustumVolume()
            {
                if( _frustumVolume < 0.0F )
                {
                    computeFrustumVolume();
                }
                return _frustumVolume;
            }

            /** Compute the pixel size of an object at position v, with specified
             * radius.*/
            float
            pixelSize( const vec3& v,
                       float       radius ) const
            {
                return getCurrentCullingSet().pixelSize( v, radius );
            }

            /** Compute the pixel size of the bounding sphere.*/
            float
            pixelSize( const sphere& bs ) const
            {
                return pixelSize( bs.center, bs.radius );
            }

            /** Compute the pixel size of an object at position v, with specified radius.
             * fabs()ed to always be positive. */
            float
            clampedPixelSize( const vec3& v,
                              float       radius ) const
            {
                return getCurrentCullingSet().clampedPixelSize( v, radius );
            }

            /** Compute the pixel size of the bounding sphere. fabs()ed to always be
             * positive. */
            float
            clampedPixelSize( const sphere& bs ) const
            {
                return clampedPixelSize( bs.center, bs.radius );
            }

            inline void
            disableAndPushOccludersCurrentMask( NodePath& nodePath )
            {
                getCurrentCullingSet().disableAndPushOccludersCurrentMask( nodePath );
            }

            inline void
            popOccludersCurrentMask( NodePath& nodePath )
            {
                getCurrentCullingSet().popOccludersCurrentMask( nodePath );
            }

            inline bool
            isCulled( const std::vector<vec3>& vertices )
            {
                return getCurrentCullingSet().isCulled( vertices );
            }

            inline bool
            isCulled( const box& bb )
            {
                return bb.valid() && getCurrentCullingSet().isCulled( bb );
            }

            inline bool
            isCulled( const sphere& bs )
            {
                return getCurrentCullingSet().isCulled( bs );
            }

            inline bool
            isCulled( const osg::Node& node )
            {
                if( node.isCullingActive() )
                {
                    return getCurrentCullingSet().isCulled( node.getBound() );
                }
                else
                {
                    getCurrentCullingSet().resetCullingMask();
                    return false;
                }
            }

            inline void
            pushCurrentMask()
            {
                getCurrentCullingSet().pushCurrentMask();
            }

            inline void
            popCurrentMask()
            {
                getCurrentCullingSet().popCurrentMask();
            }

            typedef std::vector<CullingSet> CullingStack;

            inline CullingStack&
            getClipSpaceCullingStack()
            {
                return _clipspaceCullingStack;
            }

            inline CullingStack&
            getProjectionCullingStack()
            {
                return _projectionCullingStack;
            }

            inline CullingStack&
            getModelViewCullingStack()
            {
                return _modelviewCullingStack;
            }

            inline CullingSet&
            getCurrentCullingSet()
            {
                return *_back_modelviewCullingStack;
            }

            inline const CullingSet&
            getCurrentCullingSet() const
            {
                return *_back_modelviewCullingStack;
            }

            inline osg::Viewport*
            getViewport();
            inline const osg::Viewport*
            getViewport() const;

            inline osg::RefMatrix*
            getModelViewMatrix();
            inline const osg::RefMatrix*
            getModelViewMatrix() const;

            inline osg::RefMatrix*
            getProjectionMatrix();
            inline const osg::RefMatrix*
            getProjectionMatrix() const;

            inline osg::dmat4
            getWindowMatrix() const;
            inline const osg::RefMatrix*
            getMVPW();

            inline const osg::vec3&
            getReferenceViewPoint() const
            {
                return _referenceViewPoints.back();
            }

            inline void
            pushReferenceViewPoint( const osg::vec3& viewPoint )
            {
                _referenceViewPoints.push_back( viewPoint );
            }

            inline void
            popReferenceViewPoint()
            {
                _referenceViewPoints.pop_back();
            }

            inline const osg::vec3&
            getEyeLocal() const
            {
                return _eyePointStack.back();
            }

            inline const osg::vec3&
            getViewPointLocal() const
            {
                return _viewPointStack.back();
            }

            inline const osg::vec3
            getUpLocal() const
            {
                const osg::dmat4& matrix = *_modelviewStack.back();
                // Up in model space = row 1 of MV rotation: M(1,0), M(1,1), M(1,2)
                return osg::vec3( static_cast<float>( matrix( 1, 0 ) ),
                                  static_cast<float>( matrix( 1, 1 ) ),
                                  static_cast<float>( matrix( 1, 2 ) ) );
            }

            inline const osg::vec3
            getLookVectorLocal() const
            {
                const osg::dmat4& matrix = *_modelviewStack.back();
                // Look direction in model space = -row 2 of MV rotation: -M(2,0),
                // -M(2,1), -M(2,2)
                return osg::vec3( static_cast<float>( -matrix( 2, 0 ) ),
                                  static_cast<float>( -matrix( 2, 1 ) ),
                                  static_cast<float>( -matrix( 2, 2 ) ) );
            }

            typedef fast_back_stack<ref_ptr<RefMatrix>> MatrixStack;

            MatrixStack&
            getProjectionStack()
            {
                return _projectionStack;
            }

            const MatrixStack&
            getProjectionStack() const
            {
                return _projectionStack;
            }

            MatrixStack&
            getModelViewStack()
            {
                return _modelviewStack;
            }

            const MatrixStack&
            getModelViewStack() const
            {
                return _modelviewStack;
            }

            MatrixStack&
            getMVPWStack()
            {
                return _MVPW_Stack;
            }

            const MatrixStack&
            getMVPWStack() const
            {
                return _MVPW_Stack;
            }

        protected:

            // base set of shadow volume occluder to use in culling.
            ShadowVolumeOccluderList                   _occluderList;

            MatrixStack                                _projectionStack;

            MatrixStack                                _modelviewStack;
            MatrixStack                                _MVPW_Stack;

            typedef fast_back_stack<ref_ptr<Viewport>> ViewportStack;
            ViewportStack                              _viewportStack;

            typedef fast_back_stack<vec3>              EyePointStack;
            EyePointStack                              _referenceViewPoints;
            EyePointStack                              _eyePointStack;
            EyePointStack                              _viewPointStack;

            CullingStack                               _clipspaceCullingStack;
            CullingStack                               _projectionCullingStack;

            CullingStack                               _modelviewCullingStack;
            unsigned int                               _index_modelviewCullingStack;
            CullingSet*                                _back_modelviewCullingStack;

            void
                                                              computeFrustumVolume();
            float                                             _frustumVolume;

            unsigned int                                      _bbCornerNear;
            unsigned int                                      _bbCornerFar;

            ref_ptr<osg::RefMatrix>                           _identity;

            typedef std::vector<osg::ref_ptr<osg::RefMatrix>> MatrixList;
            MatrixList                                        _reuseMatrixList;
            unsigned int                                      _currentReuseMatrixIndex;

            inline osg::RefMatrix*
            createOrReuseMatrix( const osg::dmat4& value );
    };

    inline osg::Viewport*
    CullStack::getViewport()
    {
        return _viewportStack.empty() ? 0 : _viewportStack.back().get();
    }

    inline const osg::Viewport*
    CullStack::getViewport() const
    {
        return _viewportStack.empty() ? 0 : _viewportStack.back().get();
    }

    inline osg::RefMatrix*
    CullStack::getModelViewMatrix()
    {
        return _modelviewStack.empty() ? _identity.get() : _modelviewStack.back().get();
    }

    inline const osg::RefMatrix*
    CullStack::getModelViewMatrix() const
    {
        return _modelviewStack.empty() ? _identity.get() : _modelviewStack.back().get();
    }

    inline osg::RefMatrix*
    CullStack::getProjectionMatrix()
    {
        return _projectionStack.empty() ? _identity.get()
                                        : _projectionStack.back().get();
    }

    inline const osg::RefMatrix*
    CullStack::getProjectionMatrix() const
    {
        return _projectionStack.empty() ? _identity.get()
                                        : _projectionStack.back().get();
    }

    inline osg::dmat4
    CullStack::getWindowMatrix() const
    {
        if( !_viewportStack.empty() )
        {
            osg::Viewport* viewport = _viewportStack.back().get();
            return viewport->computeWindowMatrix();
        }
        else
        {
            return *_identity;
        }
    }

    inline const osg::RefMatrix*
    CullStack::getMVPW()
    {
        if( !_MVPW_Stack.empty() )
        {
            if( !_MVPW_Stack.back() )
            {
                _MVPW_Stack.back()       = createOrReuseMatrix( *getModelViewMatrix() );
                ( *_MVPW_Stack.back() ) *= *( getProjectionMatrix() );
                ( *_MVPW_Stack.back() ) *= getWindowMatrix();
            }
            return _MVPW_Stack.back().get();
        }
        else
        {
            return _identity.get();
        }
    }

    inline RefMatrix*
    CullStack::createOrReuseMatrix( const osg::dmat4& value )
    {
        // skip of any already reused matrix.
        while( _currentReuseMatrixIndex <
               _reuseMatrixList.size() &&
               _reuseMatrixList[_currentReuseMatrixIndex]->referenceCount() > 1 )
        {
            ++_currentReuseMatrixIndex;
        }

        // if still within list, element must be singularly referenced
        // there return it to be reused.
        if( _currentReuseMatrixIndex < _reuseMatrixList.size() )
        {
            RefMatrix* matrix = _reuseMatrixList[_currentReuseMatrixIndex++].get();
            matrix->set( value );
            return matrix;
        }

        // otherwise need to create new matrix.
        osg::RefMatrix* matrix = new RefMatrix( value );
        _reuseMatrixList.push_back( matrix );
        ++_currentReuseMatrixIndex;
        return matrix;
    }

}    // end of namespace
