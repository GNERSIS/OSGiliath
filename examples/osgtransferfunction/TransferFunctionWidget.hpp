/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * TransferFunctionWidget example application
 */
#ifndef OSGUI_TRANSFERFUNCTIONWIDGET
#define OSGUI_TRANSFERFUNCTIONWIDGET

#include <osg/core/Inherit.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/textures/TransferFunction.hpp>
#include <osgUI/Widget>

namespace osgUI
{

    class TransferFunctionWidget
        : public osg::Inherit<osgUI::Widget, TransferFunctionWidget>
    {
        public:

            OSG_REGISTER_TYPE( osgUI,
                               TransferFunctionWidget )

            TransferFunctionWidget( osg::TransferFunction1D* tf = 0 );
            TransferFunctionWidget( const TransferFunctionWidget& tfw,
                                    const osg::CopyOp&            copyop =
                                        osg::CopyOp::SHALLOW_COPY );

            void
            accept( osg::NodeVisitor& nv ) override
            {
                if( nv.validNodeMask( *this ) )
                {
                    nv.pushOntoNodePath( this );
                    nv.apply( *this );
                    nv.popFromNodePath();
                }
            }

            virtual void
            traverseImplementation( osg::NodeVisitor& nv );

            virtual bool
            handleImplementation( osgGA::EventVisitor* ev,
                                  osgGA::Event*        event );

            void
            setTransferFunction( const osg::TransferFunction1D* tf );

            osg::TransferFunction1D*
            getTransferFunction()
            {
                return _transferFunction.get();
            }

            const osg::TransferFunction1D*
            getTransferFunction() const
            {
                return _transferFunction.get();
            }

            void
            resetVisibleRange();
            void
            setVisibleRange( float left,
                             float right );
            void
            translateVisibleRange( float delta );
            void
            scaleVisibleRange( float center,
                               float delta );

            virtual void
            createGraphicsImplementation();

        protected:

            virtual ~TransferFunctionWidget()
            {
            }

            osg::ref_ptr<osg::TransferFunction1D> _transferFunction;

            osg::ref_ptr<osg::Geode>              _geode;
            osg::ref_ptr<osg::Geometry>           _geometry;
            osg::ref_ptr<osg::Vec3Array>          _vertices;
            osg::ref_ptr<osg::Vec4Array>          _colours;
            osg::ref_ptr<osg::DrawElementsUShort> _background_primitives;
            osg::ref_ptr<osg::DrawElementsUShort> _historgram_primitives;
            osg::ref_ptr<osg::DrawElementsUShort> _outline_primitives;

            float                                 _min;
            float                                 _max;
            float                                 _left;
            float                                 _right;

            bool                                  _startedDrag;
            float                                 _previousDragPosition;
    };

}

#endif
