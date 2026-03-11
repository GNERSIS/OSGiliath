/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traverses the scene graph to build a render graph. Performs view
 * frustum culling, state sorting, and transparency ordering.
 */
#pragma once

#include <map>
#include <osg/core/Inherit.hpp>
#include <osg/core/Notify.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/sphere.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/ClearNode.hpp>
#include <osg/state/State.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/traversal/CullStack.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgUtil/culling/RenderStage.hpp>
#include <osgUtil/culling/StateGraph.hpp>
#include <vector>

namespace osgUtil
{

    /**
     * Basic NodeVisitor implementation for rendering a scene.
     * This visitor traverses the scene graph, collecting transparent and
     * opaque osg::Drawables into a depth sorted transparent bin and a state
     * sorted opaque bin.  The opaque bin is rendered first, and then the
     * transparent bin is rendered in order from the furthest osg::Drawable
     * from the eye to the one nearest the eye.
     */
    class OSGUTIL_EXPORT CullVisitor : public osg::DualModeVisitor,
                                       public osg::CullStack
    {
        public:

            typedef osg::dmat4::value_type value_type;

            CullVisitor();

            /// Copy constructor that does a shallow copy.
            CullVisitor( const CullVisitor& );

            OSG_REGISTER_TYPE( osgUtil,
                               CullVisitor )

            /** Convert 'this' into a osgUtil::CullVisitor pointer if Object is a
             * osgUtil::CullVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<osgUtil::CullVisitor*>(this).*/
            virtual osgUtil::CullVisitor*
            asCullVisitor()
            {
                return this;
            }

            /** convert 'const this' into a const osgUtil::CullVisitor pointer if Object
             * is a osgUtil::CullVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<const osgUtil::CullVisitor*>(this).*/
            virtual const osgUtil::CullVisitor*
            asCullVisitor() const
            {
                return this;
            }

            /** Convert 'this' into a osg::CullStack pointer if Object is a
             * osg::CullStack, otherwise return 0. Equivalent to
             * dynamic_cast<osg::CullStack*>(this).*/
            virtual osg::CullStack*
            asCullStack()
            {
                return static_cast<osg::CullStack*>( this );
            }

            /** convert 'const this' into a const osg::CullStack pointer if Object is a
             * osg::CullStack, otherwise return 0. Equivalent to dynamic_cast<const
             * osg::CullStack*>(this).*/
            virtual const osg::CullStack*
            asCullStack() const
            {
                return static_cast<const osg::CullStack*>( this );
            }

            using osg::NodeVisitor::clone;

            /** Create a shallow copy of the CullVisitor, used by CullVisitor::create()
             * to clone the prototype. */
            virtual CullVisitor*
            clone() const
            {
                return new CullVisitor( *this );
            }

            /** get the prototype singleton used by CullVisitor::create().*/
            static osg::ref_ptr<CullVisitor>&
            prototype();

            /** create a CullVisitor by cloning CullVisitor::prototype().*/
            static CullVisitor*
            create();

            virtual void
            reset();

            struct Identifier : public osg::Referenced
            {
                    Identifier()
                    {
                    }

                    virtual ~Identifier()
                    {
                    }
            };

            void
            setIdentifier( Identifier* identifier )
            {
                _identifier = identifier;
            }

            Identifier*
            getIdentifier()
            {
                return _identifier.get();
            }

            const Identifier*
            getIdentifier() const
            {
                return _identifier.get();
            }

            virtual osg::vec3
            getEyePoint() const
            {
                return getEyeLocal();
            }

            virtual osg::vec3
            getViewPoint() const
            {
                return getViewPointLocal();
            }

            virtual float
            getDistanceToEyePoint( const osg::vec3& pos,
                                   bool             withLODScale ) const;
            virtual float
            getDistanceFromEyePoint( const osg::vec3& pos,
                                     bool             withLODScale ) const;

            virtual float
            getDistanceToViewPoint( const osg::vec3& pos,
                                    bool             withLODScale ) const;

            using osg::ConstNodeVisitor::apply;
            using osg::NodeVisitor::apply;

            virtual void
            apply( osg::Node& );
            virtual void
            apply( osg::Geode& node );
            virtual void
            apply( osg::Drawable& drawable );
            virtual void
            apply( osg::Billboard& node );
            virtual void
            apply( osg::LightSource& node );

            virtual void
            apply( osg::Group& node );
            virtual void
            apply( osg::Transform& node );
            virtual void
            apply( osg::Projection& node );
            virtual void
            apply( osg::Switch& node );
            virtual void
            apply( osg::LOD& node );
            virtual void
            apply( osg::ClearNode& node );
            virtual void
            apply( osg::Camera& node );
            virtual void
            apply( osg::OccluderNode& node );
            virtual void
            apply( osg::OcclusionQueryNode& node );

            /** Push state set on the current state group.
             * If the state exists in a child state group of the current
             * state group then move the current state group to that child.
             * Otherwise, create a new state group for the state set, add
             * it to the current state group then move the current state
             * group pointer to the new state group.
             */
            inline void
            pushStateSet( const osg::StateSet* ss )
            {
                _currentStateGraph = _currentStateGraph->find_or_insert( ss );

                bool useRenderBinDetails =
                    ( ss->useRenderBinDetails() && !ss->getBinName().empty() ) &&
                    ( _numberOfEncloseOverrideRenderBinDetails ==
                      0 ||
                      ( ss->getRenderBinMode() &
                        osg::StateSet::PROTECTED_RENDERBIN_DETAILS ) != 0 );

                if( useRenderBinDetails )
                {
                    _renderBinStack.push_back( _currentRenderBin );

                    _currentRenderBin =
                        ss->getNestRenderBins()
                            ? _currentRenderBin->find_or_insert( ss->getBinNumber(),
                                                                 ss->getBinName() )
                            : _currentRenderBin->getStage()->find_or_insert(
                                  ss->getBinNumber(),
                                  ss->getBinName()
                              );
                }

                if( ( ss->getRenderBinMode() &
                      osg::StateSet::OVERRIDE_RENDERBIN_DETAILS ) != 0 )
                {
                    ++_numberOfEncloseOverrideRenderBinDetails;
                }
            }

            /** Pop the top state set and hence associated state group.
             * Move the current state group to the parent of the popped
             * state group.
             */
            inline void
            popStateSet()
            {
                const osg::StateSet* ss = _currentStateGraph->getStateSet();
                if( ( ss->getRenderBinMode() &
                      osg::StateSet::OVERRIDE_RENDERBIN_DETAILS ) != 0 )
                {
                    --_numberOfEncloseOverrideRenderBinDetails;
                }

                bool useRenderBinDetails =
                    ( ss->useRenderBinDetails() && !ss->getBinName().empty() ) &&
                    ( _numberOfEncloseOverrideRenderBinDetails ==
                      0 ||
                      ( ss->getRenderBinMode() &
                        osg::StateSet::PROTECTED_RENDERBIN_DETAILS ) != 0 );

                if( useRenderBinDetails )
                {
                    if( _renderBinStack.empty() )
                    {
                        _currentRenderBin = _currentRenderBin->getStage();
                    }
                    else
                    {
                        _currentRenderBin = _renderBinStack.back();
                        _renderBinStack.pop_back();
                    }
                }
                _currentStateGraph = _currentStateGraph->_parent;
            }

            inline void
            setStateGraph( StateGraph* rg )
            {
                _rootStateGraph    = rg;
                _currentStateGraph = rg;
            }

            inline StateGraph*
            getRootStateGraph()
            {
                return _rootStateGraph.get();
            }

            inline StateGraph*
            getCurrentStateGraph()
            {
                return _currentStateGraph;
            }

            inline void
            setRenderStage( RenderStage* rg )
            {
                _rootRenderStage  = rg;
                _currentRenderBin = rg;
            }

            inline RenderStage*
            getRenderStage()
            {
                return _rootRenderStage.get();
            }

            inline RenderStage*
            getCurrentRenderStage()
            {
                return _currentRenderBin->getStage();
            }

            inline osg::Camera*
            getCurrentCamera()
            {
                return getCurrentRenderStage()->getCamera();
            }

            inline RenderBin*
            getCurrentRenderBin()
            {
                return _currentRenderBin;
            }

            inline void
            setCurrentRenderBin( RenderBin* rb )
            {
                _currentRenderBin = rb;
            }

            void
            setCalculatedNearPlane( value_type value )
            {
                _computed_znear = value;
            }

            inline value_type
            getCalculatedNearPlane() const
            {
                return _computed_znear;
            }

            void
            setCalculatedFarPlane( value_type value )
            {
                _computed_zfar = value;
            }

            inline value_type
            getCalculatedFarPlane() const
            {
                return _computed_zfar;
            }

            value_type
            computeNearestPointInFrustum( const osg::dmat4&               matrix,
                                          const osg::Polytope::PlaneList& planes,
                                          const osg::Drawable&            drawable );
            value_type
            computeFurthestPointInFrustum( const osg::dmat4&               matrix,
                                           const osg::Polytope::PlaneList& planes,
                                           const osg::Drawable&            drawable );

            bool
            updateCalculatedNearFar( const osg::dmat4& matrix,
                                     const osg::box&   bb );

            bool
            updateCalculatedNearFar( const osg::dmat4&    matrix,
                                     const osg::Drawable& drawable,
                                     bool                 isBillboard = false );

            void
            updateCalculatedNearFar( const osg::vec3& pos );

            /** Add a drawable to current render graph.*/
            inline void
            addDrawable( osg::Drawable*  drawable,
                         osg::RefMatrix* matrix );

            /** Add a drawable and depth to current render graph.*/
            inline void
            addDrawableAndDepth( osg::Drawable*  drawable,
                                 osg::RefMatrix* matrix,
                                 float           depth );

            /** Add an attribute which is positioned relative to the modelview matrix.*/
            inline void
            addPositionedAttribute( osg::RefMatrix*            matrix,
                                    const osg::StateAttribute* attr );

            /** Add an attribute which is positioned relative to the modelview matrix.*/
            inline void
            addPositionedTextureAttribute( unsigned int               textureUnit,
                                           osg::RefMatrix*            matrix,
                                           const osg::StateAttribute* attr );

            /** compute near plane based on the polgon intersection of primtives in near
             * plane candidate list of drawables. Note, you have to set
             * ComputeNearFarMode to COMPUTE_NEAR_FAR_USING_PRIMITIVES to be able to near
             * plane candidate drawables to be recorded by the cull traversal. */
            void
            computeNearPlane();

            /** Re-implement CullStack's popProjectionMatrix() adding clamping of the
             * projection matrix to the computed near and far.*/
            virtual void
            popProjectionMatrix();

            /** CullVisitor's default clamping of the projection float matrix to computed
             * near and far values. Note, do not call this method directly, use
             * clampProjectionMatrix(..) instead, unless you want to bypass the
             * callback.*/
            virtual bool
            clampProjectionMatrixImplementation( osg::mat4& projection,
                                                 double&    znear,
                                                 double&    zfar ) const;

            /** CullVisitor's default clamping of the projection double matrix to
             * computed near and far values. Note, do not call this method directly, use
             * clampProjectionMatrix(..) instead, unless you want to bypass the
             * callback.*/
            virtual bool
            clampProjectionMatrixImplementation( osg::dmat4& projection,
                                                 double&     znear,
                                                 double&     zfar ) const;

            /** Clamp the projection float matrix to computed near and far values, use
             * callback if it exists, otherwise use default CullVisitor implementation.*/
            inline bool
            clampProjectionMatrix( osg::mat4&  projection,
                                   value_type& znear,
                                   value_type& zfar ) const
            {
                double zn     = znear;
                double zf     = zfar;
                bool   result = false;
                if( _clampProjectionMatrixCallback.valid() )
                {
                    result =
                        _clampProjectionMatrixCallback
                            ->clampProjectionMatrixImplementation( projection, zn, zf );
                }
                else
                {
                    result = clampProjectionMatrixImplementation( projection, zn, zf );
                }

                if( result )
                {
                    znear = zn;
                    zfar  = zf;
                    return true;
                }
                else
                {
                    return false;
                }
            }

            /** Clamp the projection double matrix to computed near and far values, use
             * callback if it exists, otherwise use default CullVisitor implementation.*/
            inline bool
            clampProjectionMatrix( osg::dmat4& projection,
                                   value_type& znear,
                                   value_type& zfar ) const
            {
                double zn     = znear;
                double zf     = zfar;
                bool   result = false;

                if( _clampProjectionMatrixCallback.valid() )
                {
                    result =
                        _clampProjectionMatrixCallback
                            ->clampProjectionMatrixImplementation( projection, zn, zf );
                }
                else
                {
                    result = clampProjectionMatrixImplementation( projection, zn, zf );
                }

                if( result )
                {
                    znear = zn;
                    zfar  = zf;
                    return true;
                }
                else
                {
                    return false;
                }
            }

            void
            setState( osg::State* state )
            {
                _renderInfo.setState( state );
            }

            osg::State*
            getState()
            {
                return _renderInfo.getState();
            }

            const osg::State*
            getState() const
            {
                return _renderInfo.getState();
            }

            void
            setRenderInfo( osg::RenderInfo& renderInfo )
            {
                _renderInfo = renderInfo;
            }

            osg::RenderInfo&
            getRenderInfo()
            {
                return _renderInfo;
            }

            const osg::RenderInfo&
            getRenderInfo() const
            {
                return _renderInfo;
            }

        protected:

            virtual ~CullVisitor();

            /** Prevent unwanted copy operator.*/
            CullVisitor&
            operator=( const CullVisitor& )
            {
                return *this;
            }

            inline void
            handle_cull_callbacks_and_traverse( osg::Node& node )
            {
                osg::Callback* callback = node.getCullCallback();
                if( callback )
                {
                    callback->run( &node, this );
                }
                else
                {
                    traverse( node );
                }
            }

            inline void
            handle_cull_callbacks_and_accept( osg::Node& node,
                                              osg::Node* acceptNode )
            {
                osg::Callback* callback = node.getCullCallback();
                if( callback )
                {
                    callback->run( &node, this );
                }
                else
                {
                    acceptNode->accept( *this );
                }
            }

            osg::ref_ptr<StateGraph>                      _rootStateGraph;
            StateGraph*                                   _currentStateGraph;

            osg::ref_ptr<RenderStage>                     _rootRenderStage;
            RenderBin*                                    _currentRenderBin;
            std::vector<RenderBin*>                       _renderBinStack;

            value_type                                    _computed_znear;
            value_type                                    _computed_zfar;

            unsigned int                                  _traversalOrderNumber;

            typedef std::vector<osg::ref_ptr<RenderLeaf>> RenderLeafList;
            RenderLeafList                                _reuseRenderLeafList;
            unsigned int                                  _currentReuseRenderLeafIndex;

            inline RenderLeaf*
                            createOrReuseRenderLeaf( osg::Drawable*  drawable,
                                                     osg::RefMatrix* projection,
                                                     osg::RefMatrix* matrix,
                                                     float           depth = 0.0F );

            unsigned int    _numberOfEncloseOverrideRenderBinDetails;

            osg::RenderInfo _renderInfo;

            struct MatrixPlanesDrawables
            {
                    MatrixPlanesDrawables() :
                        _drawable( 0 )
                    {
                    }

                    void
                    set( const osg::dmat4&    matrix,
                         const osg::Drawable* drawable,
                         const osg::Polytope& frustum )
                    {
                        _matrix   = matrix;
                        _drawable = drawable;
                        if( !_planes.empty() )
                        {
                            _planes.clear();
                        }

                        // create a new list of planes from the active walls of the
                        // frustum.
                        osg::Polytope::ClippingMask result_mask =
                            frustum.getResultMask();
                        osg::Polytope::ClippingMask selector_mask = 0X1;
                        for( osg::Polytope::PlaneList::const_iterator itr =
                                 frustum.getPlaneList().begin();
                             itr != frustum.getPlaneList().end();
                             ++itr )
                        {
                            if( result_mask & selector_mask )
                            {
                                _planes.push_back( *itr );
                            }
                            selector_mask <<= 1;
                        }
                    }

                    MatrixPlanesDrawables( const MatrixPlanesDrawables& mpd ) :
                        _matrix( mpd._matrix ),
                        _drawable( mpd._drawable ),
                        _planes( mpd._planes )
                    {
                    }

                    MatrixPlanesDrawables&
                    operator=( const MatrixPlanesDrawables& mpd )
                    {
                        _matrix   = mpd._matrix;
                        _drawable = mpd._drawable;
                        _planes   = mpd._planes;
                        return *this;
                    }

                    osg::dmat4               _matrix;
                    const osg::Drawable*     _drawable;
                    osg::Polytope::PlaneList _planes;
            };

            typedef std::multimap<value_type, MatrixPlanesDrawables>
                                      DistanceMatrixDrawableMap;
            DistanceMatrixDrawableMap _nearPlaneCandidateMap;
            DistanceMatrixDrawableMap _farPlaneCandidateMap;

            osg::ref_ptr<Identifier>  _identifier;
    };

    inline void
    CullVisitor::addDrawable( osg::Drawable*  drawable,
                              osg::RefMatrix* matrix )
    {
        if( _currentStateGraph->leaves_empty() )
        {
            // this is first leaf to be added to StateGraph
            // and therefore should not already know to current render bin,
            // so need to add it.
            _currentRenderBin->addStateGraph( _currentStateGraph );
        }
        //_currentStateGraph->addLeaf(new RenderLeaf(drawable,matrix));
        _currentStateGraph->addLeaf(
            createOrReuseRenderLeaf( drawable, _projectionStack.back().get(), matrix )
        );
    }

    /** Add a drawable and depth to current render graph.*/
    inline void
    CullVisitor::addDrawableAndDepth( osg::Drawable*  drawable,
                                      osg::RefMatrix* matrix,
                                      float           depth )
    {
        if( _currentStateGraph->leaves_empty() )
        {
            // this is first leaf to be added to StateGraph
            // and therefore should not already know to current render bin,
            // so need to add it.
            _currentRenderBin->addStateGraph( _currentStateGraph );
        }
        //_currentStateGraph->addLeaf(new RenderLeaf(drawable,matrix,depth));
        _currentStateGraph->addLeaf(
            createOrReuseRenderLeaf( drawable,
                                     _projectionStack.back().get(),
                                     matrix,
                                     depth )
        );
    }

    /** Add an attribute which is positioned relative to the modelview matrix.*/
    inline void
    CullVisitor::addPositionedAttribute( osg::RefMatrix*            matrix,
                                         const osg::StateAttribute* attr )
    {
        _currentRenderBin->getStage()->addPositionedAttribute( matrix, attr );
    }

    /** Add an attribute which is positioned relative to the modelview matrix.*/
    inline void
    CullVisitor::addPositionedTextureAttribute( unsigned int               textureUnit,
                                                osg::RefMatrix*            matrix,
                                                const osg::StateAttribute* attr )
    {
        _currentRenderBin->getStage()->addPositionedTextureAttribute( textureUnit,
                                                                      matrix,
                                                                      attr );
    }

    inline RenderLeaf*
    CullVisitor::createOrReuseRenderLeaf( osg::Drawable*  drawable,
                                          osg::RefMatrix* projection,
                                          osg::RefMatrix* matrix,
                                          float           depth )
    {
        // Skips any already reused renderleaf.
        while( _currentReuseRenderLeafIndex <
               _reuseRenderLeafList.size() &&
               _reuseRenderLeafList[_currentReuseRenderLeafIndex]->referenceCount() > 1 )
        {
            osg::notify( osg::INFO )
                << "CullVisitor:createOrReuseRenderLeaf() skipping multiply referenced "
                   "entry. _reuseRenderLeafList.size()="
                << _reuseRenderLeafList.size() << " _reuseRenderLeafList["
                << _currentReuseRenderLeafIndex << "]->referenceCount()="
                << _reuseRenderLeafList[_currentReuseRenderLeafIndex]->referenceCount()
                << std::endl;
            ++_currentReuseRenderLeafIndex;
        }

        // If still within list, element must be singularly referenced then return it to
        // be reused.
        if( _currentReuseRenderLeafIndex < _reuseRenderLeafList.size() )
        {
            RenderLeaf* renderleaf =
                _reuseRenderLeafList[_currentReuseRenderLeafIndex++].get();
            renderleaf->set( drawable,
                             projection,
                             matrix,
                             depth,
                             _traversalOrderNumber++ );
            return renderleaf;
        }

        // Otherwise need to create new renderleaf.
        RenderLeaf* renderleaf = new RenderLeaf( drawable,
                                                 projection,
                                                 matrix,
                                                 depth,
                                                 _traversalOrderNumber++ );
        _reuseRenderLeafList.push_back( renderleaf );

        ++_currentReuseRenderLeafIndex;
        return renderleaf;
    }

}
