/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Frame example application
 */
#ifndef FRAME_H_
#define FRAME_H_

#include <osg/core/Inherit.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>

namespace osgfxbrowser
{

    struct Rect
    {
            float x0, y0, x1, y1;

            Rect()
            {
            }

            Rect( float x0_,
                  float y0_,
                  float x1_,
                  float y1_ ) :
                x0( x0_ ),
                y0( y0_ ),
                x1( x1_ ),
                y1( y1_ )
            {
            }

            inline float
            width() const
            {
                return x1 - x0;
            }

            inline float
            height() const
            {
                return y0 - y1;
            }
    };

    class Frame : public osg::Inherit<osg::Geode, Frame>
    {
        public:

            OSG_REGISTER_TYPE( osgfxbrowser,
                               Frame )

            Frame();
            Frame( const Frame&       copy,
                   const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

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

            inline const std::string&
            getCaption() const
            {
                return caption_;
            }

            inline void
            setCaption( const std::string& caption )
            {
                caption_ = caption;
            }

            inline const osg::vec4&
            getBackgroundColor() const
            {
                return bgcolor_;
            }

            inline void
            setBackgroundColor( const osg::vec4& bgcolor )
            {
                bgcolor_ = bgcolor;
            }

            inline const Rect&
            getRect() const
            {
                return rect_;
            }

            inline void
            setRect( const Rect& rect )
            {
                rect_ = rect;
            }

            static osg::Geometry*
            build_quad( const Rect&      rect,
                        const osg::vec4& color,
                        bool             shadow = true,
                        float            z      = 0 );

            virtual void
            rebuild();

        protected:

            virtual ~Frame()
            {
            }

            Frame&
            operator()( const Frame& )
            {
                return *this;
            }

            virtual void
            rebuild_client_area( const Rect& /*client_rect*/ )
            {
            }

        private:

            osg::vec4   bgcolor_;
            Rect        rect_;
            std::string caption_;
    };

}

#endif
