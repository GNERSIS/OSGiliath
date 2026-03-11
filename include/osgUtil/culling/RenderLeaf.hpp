/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Single drawable + state + depth entry in a RenderBin.
 * The atomic unit of rendering in the draw traversal.
 */
#pragma once

#include <osg/core/ref_ptr.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/state/State.hpp>
#include <osgUtil/Export>

namespace osgUtil
{

#define OSGUTIL_RENDERBACKEND_USE_REF_PTR

    // Forward declare StateGraph
    class StateGraph;

    /** Container class for all data required for rendering of drawables.
     */
    class OSGUTIL_EXPORT RenderLeaf : public osg::Referenced
    {
        public:

            inline RenderLeaf( osg::Drawable*  drawable,
                               osg::RefMatrix* projection,
                               osg::RefMatrix* modelview,
                               float           depth                = 0.0F,
                               unsigned int    traversalOrderNumber = 0 ) :
                osg::Referenced( false ),
                _parent( 0 ),
                _drawable( drawable ),
                _projection( projection ),
                _modelview( modelview ),
                _depth( depth ),
                _traversalOrderNumber( traversalOrderNumber )
            {
                _dynamic = ( drawable->getDataVariance() ==
                             osg::Object::DataVariance::DYNAMIC );
            }

            inline void
            set( osg::Drawable*  drawable,
                 osg::RefMatrix* projection,
                 osg::RefMatrix* modelview,
                 float           depth                = 0.0F,
                 unsigned int    traversalOrderNumber = 0 )
            {
                _parent               = 0;
                _drawable             = drawable;
                _projection           = projection;
                _modelview            = modelview;
                _depth                = depth;
                _dynamic              = ( drawable->getDataVariance() ==
                                          osg::Object::DataVariance::DYNAMIC );
                _traversalOrderNumber = traversalOrderNumber;
            }

            inline void
            reset()
            {
                _parent               = 0;
                _drawable             = 0;
                _projection           = 0;
                _modelview            = 0;
                _depth                = 0.0F;
                _dynamic              = false;
                _traversalOrderNumber = 0;
            }

            virtual void
            render( osg::RenderInfo& renderInfo,
                    RenderLeaf*      previous );

            virtual void
            resizeGLObjectBuffers( unsigned int maxSize )
            {
                if( _drawable )
                {
                    _drawable->resizeGLObjectBuffers( maxSize );
                }
            }

            virtual void
            releaseGLObjects( osg::State* state = 0 ) const
            {
                if( _drawable )
                {
                    _drawable->releaseGLObjects( state );
                }
            }

            /// Allow StateGraph to change the RenderLeaf's _parent.
            friend class osgUtil::StateGraph;

        public:

            StateGraph* _parent;

#ifdef OSGUTIL_RENDERBACKEND_USE_REF_PTR
            osg::ref_ptr<osg::Drawable> _drawable;

            const osg::Drawable*
            getDrawable() const
            {
                return _drawable.get();
            }
#else
            osg::Drawable* _drawable;

            const osg::Drawable*
            getDrawable() const
            {
                return _drawable;
            }
#endif
            osg::ref_ptr<osg::RefMatrix> _projection;
            osg::ref_ptr<osg::RefMatrix> _modelview;
            float                        _depth;
            bool                         _dynamic;
            unsigned int                 _traversalOrderNumber;

        private:

            /// disallow creation of blank RenderLeaf as this isn't useful.
            RenderLeaf() :
                osg::Referenced( false ),
                _parent( 0 ),
                _drawable( 0 ),
                _projection( 0 ),
                _modelview( 0 ),
                _depth( 0.0F ),
                _traversalOrderNumber( 0 )
            {
            }

            /// disallow copy construction.
            RenderLeaf( const RenderLeaf& ) :
                osg::Referenced( false )
            {
            }

            /// disallow copy operator.
            RenderLeaf&
            operator=( const RenderLeaf& )
            {
                return *this;
            }
    };

}
