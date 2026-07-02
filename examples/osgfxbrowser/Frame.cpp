/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Frame example application
 */
#include "Frame.hpp"

#include <osgText/Text.hpp>

namespace osgfxbrowser
{

    Frame::Frame() :
        bgcolor_( 0.5F,
                  0.5F,
                  0.5F,
                  1.0F ),
        rect_( 0,
               0,
               100,
               100 ),
        caption_( "Frame" )
    {
    }

    Frame::Frame( const Frame&       copy,
                  const osg::CopyOp& copyop ) :
        Inherit( copy,
                 copyop ),
        bgcolor_( copy.bgcolor_ ),
        rect_( copy.rect_ ),
        caption_( copy.caption_ )
    {
    }

    void
    Frame::rebuild()
    {
        float zPos = -0.1F;

        removeDrawables( 0, getNumDrawables() );
        addDrawable( build_quad( rect_, bgcolor_ ) );
        addDrawable(
            build_quad( Rect( rect_.x0 + 4, rect_.y1 - 24, rect_.x1 - 4, rect_.y1 - 4 ),
                        osg::vec4( 0, 0, 0, bgcolor_.w ),
                        false,
                        zPos )
        );

        osg::ref_ptr<osgText::Text> caption_text = new osgText::Text;
        caption_text->setText( caption_ );
        caption_text->setColor( osg::vec4( 1, 1, 1, 1 ) );
        caption_text->setAlignment( osgText::Text::CENTER_CENTER );
        caption_text->setFont( "fonts/arial.ttf" );
        caption_text->setCharacterSize( 16 );
        caption_text->setFontResolution( 16, 16 );
        caption_text->setPosition(
            osg::vec3( ( rect_.x0 + rect_.x1 ) / 2, rect_.y1 - 15, zPos * 2.0F )
        );
        addDrawable( caption_text.get() );

        rebuild_client_area(
            Rect( rect_.x0 + 4, rect_.y0 + 4, rect_.x1 - 4, rect_.y1 - 28 )
        );
    }

    osg::Geometry*
    Frame::build_quad( const Rect&      rect,
                       const osg::vec4& color,
                       bool             shadow,
                       float            z )
    {
        const float                  shadow_space = 8;
        const float                  shadow_size  = 10;

        osg::ref_ptr<osg::Geometry>  geo          = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> vx           = new osg::Vec3Array;

        vx->push_back( osg::vec3( rect.x0, rect.y0, z ) );
        vx->push_back( osg::vec3( rect.x1, rect.y0, z ) );
        vx->push_back( osg::vec3( rect.x1, rect.y1, z ) );
        vx->push_back( osg::vec3( rect.x0, rect.y1, z ) );

        if( shadow )
        {
            vx->push_back(
                osg::vec3( rect.x0 + shadow_space, rect.y0 - shadow_size, z )
            );
            vx->push_back(
                osg::vec3( rect.x1 + shadow_size, rect.y0 - shadow_size, z )
            );
            vx->push_back( osg::vec3( rect.x1, rect.y0, z ) );
            vx->push_back( osg::vec3( rect.x0 + shadow_space, rect.y0, z ) );

            vx->push_back( osg::vec3( rect.x1, rect.y1 - shadow_space, z ) );
            vx->push_back( osg::vec3( rect.x1, rect.y0, z ) );
            vx->push_back(
                osg::vec3( rect.x1 + shadow_size, rect.y0 - shadow_size, z )
            );
            vx->push_back(
                osg::vec3( rect.x1 + shadow_size, rect.y1 - shadow_space, z )
            );
        }

        geo->setVertexArray( vx.get() );

        osg::ref_ptr<osg::Vec4Array> clr = new osg::Vec4Array;
        clr->push_back( color );
        clr->push_back( color );
        clr->push_back( color );
        clr->push_back( color );

        if( shadow )
        {

            float           alpha = color.w * 0.5F;
            const osg::vec3 black( 0, 0, 0 );

            clr->push_back( osg::vec4( black, 0 ) );
            clr->push_back( osg::vec4( black, 0 ) );
            clr->push_back( osg::vec4( black, alpha ) );
            clr->push_back( osg::vec4( black, alpha ) );

            clr->push_back( osg::vec4( black, alpha ) );
            clr->push_back( osg::vec4( black, alpha ) );
            clr->push_back( osg::vec4( black, 0 ) );
            clr->push_back( osg::vec4( black, 0 ) );
        }

        geo->setColorArray( clr.get(), osg::Array::BIND_PER_VERTEX );

        unsigned int             numVertices = shadow ? 12 : 4;
        osg::DrawElementsUShort* indices = new osg::DrawElementsUShort( GL_TRIANGLES );
        for( unsigned int i = 0; i < numVertices / 4; ++i )
        {
            indices->push_back( i * 4 + 0 );
            indices->push_back( i * 4 + 1 );
            indices->push_back( i * 4 + 3 );
            indices->push_back( i * 4 + 1 );
            indices->push_back( i * 4 + 2 );
            indices->push_back( i * 4 + 3 );
        }
        geo->addPrimitiveSet( indices );

        return geo.release();
    }

}
