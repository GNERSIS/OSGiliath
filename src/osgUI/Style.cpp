/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Visual styling for UI widgets. Defines colors, fonts,
 * borders, and geometry generation for widget rendering.
 */
#include <osgUI/Style.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/state/Depth.hpp>
#include <osg/traversal/ComputeBoundsVisitor.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgText/Text.hpp>
#include <osgUtil/optimization/Optimizer.hpp>

using namespace osgUI;

osg::ref_ptr<Style>&
Style::instance()
{
    static osg::ref_ptr<Style> s_style = new Style;
    return s_style;
}

OSG_INIT_SINGLETON_PROXY( StyleSingletonProxy,
                          Style::instance() )

Style::Style()
{
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage( 1, 1, 1, GL_RGBA, GL_FLOAT );
    *( reinterpret_cast<osg::vec4*>( image->data( 0, 0, 0 ) ) ) =
        osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F );

    _clipTexture = new osg::Texture2D;
    _clipTexture->setImage( image.get() );
    _clipTexture->setBorderColor( osg::dvec4( 1.0, 1.0, 1.0, 0.0 ) );
    _clipTexture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER );
    _clipTexture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER );
    _clipTexture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    _clipTexture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );

    // image = osgDB::readImageFile("Images/lz.rgb");
    //_clipTexture->setImage(image.get());

    _disabledDepthWrite = new osg::Depth( osg::Depth::Function::LESS, 0.0, 1.0, false );
    _enabledDepthWrite  = new osg::Depth( osg::Depth::Function::LESS, 0.0, 1.0, true );
    _disableColorWriteMask = new osg::ColorMask( false, false, false, false );
}

Style::Style( const Style&       style,
              const osg::CopyOp& copyop ) :
    Inherit( style,
             copyop ),
    _clipTexture( style._clipTexture )
{
}

osg::Node*
Style::createFrame( const osg::box&      extents,
                    const FrameSettings* frameSettings,
                    const osg::vec4&     color )
{
    // OSG_NOTICE<<"createFrame"<<std::endl;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setName( "Frame" );

    float topScale    = 1.0F;
    float bottomScale = 1.0F;
    float leftScale   = 1.0F;
    float rightScale  = 1.0F;

    if( frameSettings )
    {
        switch( frameSettings->getShadow() )
        {
            case( FrameSettings::PLAIN ) :
                // default settings are appropriate for PLAIN
                break;
            case( FrameSettings::SUNKEN ) :
                topScale    = 0.6F;
                bottomScale = 1.2F;
                leftScale   = 0.8F;
                rightScale  = 0.8F;
                break;
            case( FrameSettings::RAISED ) :
                topScale    = 1.2F;
                bottomScale = 0.6F;
                leftScale   = 0.8F;
                rightScale  = 0.8F;
                break;
        }
    }

    osg::vec4 topColor( std::min( color.r * topScale, 1.0F ),
                        std::min( color.g * topScale, 1.0F ),
                        std::min( color.b * topScale, 1.0F ),
                        color.a );
    osg::vec4 bottomColor( std::min( color.r * bottomScale, 1.0F ),
                           std::min( color.g * bottomScale, 1.0F ),
                           std::min( color.b * bottomScale, 1.0F ),
                           color.a );
    osg::vec4 leftColor( std::min( color.r * leftScale, 1.0F ),
                         std::min( color.g * leftScale, 1.0F ),
                         std::min( color.b * leftScale, 1.0F ),
                         color.a );
    osg::vec4 rightColor( std::min( color.r * rightScale, 1.0F ),
                          std::min( color.g * rightScale, 1.0F ),
                          std::min( color.b * rightScale, 1.0F ),
                          color.a );

    float     lineWidth = frameSettings ? frameSettings->getLineWidth() : 1.0F;

    osg::vec3 outerBottomLeft( extents.xMin(), extents.yMin(), extents.zMin() );
    osg::vec3 outerBottomRight( extents.xMax(), extents.yMin(), extents.zMin() );
    osg::vec3 outerTopLeft( extents.xMin(), extents.yMax(), extents.zMin() );
    osg::vec3 outerTopRight( extents.xMax(), extents.yMax(), extents.zMin() );

    osg::vec3 innerBottomLeft( extents.xMin() + lineWidth,
                               extents.yMin() + lineWidth,
                               extents.zMin() );
    osg::vec3 innerBottomRight( extents.xMax() - lineWidth,
                                extents.yMin() + lineWidth,
                                extents.zMin() );
    osg::vec3 innerTopLeft( extents.xMin() + lineWidth,
                            extents.yMax() - lineWidth,
                            extents.zMin() );
    osg::vec3 innerTopRight( extents.xMax() - lineWidth,
                             extents.yMax() - lineWidth,
                             extents.zMin() );

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray( vertices.get() );

    vertices->push_back( outerBottomLeft );     // 0
    vertices->push_back( outerBottomRight );    // 1
    vertices->push_back( outerTopLeft );        // 2
    vertices->push_back( outerTopRight );       // 3

    vertices->push_back( innerBottomLeft );     // 4
    vertices->push_back( innerBottomRight );    // 5
    vertices->push_back( innerTopLeft );        // 6
    vertices->push_back( innerTopRight );       // 7

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    geometry->setColorArray( colours.get(), osg::Array::BIND_PER_PRIMITIVE_SET );

    // bottom
    {
        colours->push_back( bottomColor );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
        geometry->addPrimitiveSet( primitives.get() );
        primitives->push_back( 4 );
        primitives->push_back( 0 );
        primitives->push_back( 5 );
        primitives->push_back( 1 );
    }

    // top
    {
        colours->push_back( topColor );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
        geometry->addPrimitiveSet( primitives.get() );
        primitives->push_back( 2 );
        primitives->push_back( 6 );
        primitives->push_back( 3 );
        primitives->push_back( 7 );
    }

    // left
    {
        colours->push_back( leftColor );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
        geometry->addPrimitiveSet( primitives.get() );
        primitives->push_back( 2 );
        primitives->push_back( 0 );
        primitives->push_back( 6 );
        primitives->push_back( 4 );
    }

    // right
    {
        colours->push_back( rightColor );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLE_STRIP );
        geometry->addPrimitiveSet( primitives.get() );
        primitives->push_back( 7 );
        primitives->push_back( 5 );
        primitives->push_back( 3 );
        primitives->push_back( 1 );
    }

    return geometry.release();
}

osg::Node*
Style::createText( const osg::box&          extents,
                   const AlignmentSettings* as,
                   const TextSettings*      ts,
                   const std::string&       text )
{
    // OSG_NOTICE<<"createText"<<std::endl;

    osg::vec4                   textColor( 0.0F, 0.0, 0.0F, 1.0 );

    osg::ref_ptr<osgText::Text> textDrawable = new osgText::Text;
    textDrawable->setName( "Text" );

    textDrawable->setText( text );
    textDrawable->setEnableDepthWrites( false );
    textDrawable->setColor( textColor );

    if( ts )
    {
        textDrawable->setFont( ts->getFont() );
        textDrawable->setCharacterSize( ts->getCharacterSize() );
    }

    AlignmentSettings::Alignment alignment =
        as ? as->getAlignment() : AlignmentSettings::CENTER_CENTER;
    textDrawable->setAlignment(
        static_cast<osgText::TextBase::AlignmentType>( alignment )
    );

    switch( alignment )
    {
        case( AlignmentSettings::LEFT_TOP ) :
            textDrawable->setPosition(
                osg::vec3( extents.xMin(), extents.yMax(), extents.zMin() )
            );
            break;
        case( AlignmentSettings::LEFT_CENTER ) :
            textDrawable->setPosition( osg::vec3( extents.xMin(),
                                                  ( extents.yMin() + extents.yMax() ) *
                                                      0.5F,
                                                  extents.zMin() ) );
            break;
        case( AlignmentSettings::LEFT_BOTTOM ) :
            textDrawable->setPosition(
                osg::vec3( extents.xMin(), extents.yMin(), extents.zMin() )
            );
            break;

        case( AlignmentSettings::CENTER_TOP ) :
            textDrawable->setPosition( osg::vec3( ( extents.xMin() + extents.xMax() ) *
                                                      0.5F,
                                                  extents.yMax(),
                                                  extents.zMin() ) );
            break;
        case( AlignmentSettings::CENTER_CENTER ) :
            textDrawable->setPosition(
                osg::vec3( ( extents.xMin() + extents.xMax() ) * 0.5F,
                           ( extents.yMin() + extents.yMax() ) * 0.5F,
                           extents.zMin() )
            );
            break;
        case( AlignmentSettings::CENTER_BOTTOM ) :
            textDrawable->setPosition( osg::vec3( ( extents.xMin() + extents.xMax() ) *
                                                      0.5F,
                                                  extents.yMin(),
                                                  extents.zMin() ) );
            break;

        case( AlignmentSettings::RIGHT_TOP ) :
            textDrawable->setPosition(
                osg::vec3( extents.xMax(), extents.yMax(), extents.zMin() )
            );
            break;
        case( AlignmentSettings::RIGHT_CENTER ) :
            textDrawable->setPosition( osg::vec3( extents.xMax(),
                                                  ( extents.yMin() + extents.yMax() ) *
                                                      0.5F,
                                                  extents.zMin() ) );
            break;
        case( AlignmentSettings::RIGHT_BOTTOM ) :
            textDrawable->setPosition(
                osg::vec3( extents.xMax(), extents.yMin(), extents.zMin() )
            );
            break;

        case( AlignmentSettings::LEFT_BASE_LINE ) :
            OSG_NOTICE << "Text : LEFT_BASE_LINE" << std::endl;
            textDrawable->setPosition(
                osg::vec3( extents.xMin(),
                           ( extents.yMin() + extents.yMax() ) *
                               0.5F -
                               textDrawable->getCharacterHeight() *
                               0.5F,
                           extents.zMin() )
            );
            break;
        case( AlignmentSettings::CENTER_BASE_LINE ) :
            textDrawable->setPosition(
                osg::vec3( ( extents.xMin() + extents.xMax() ) * 0.5F,
                           ( extents.yMin() + extents.yMax() ) *
                               0.5F -
                               textDrawable->getCharacterHeight() *
                               0.5F,
                           extents.zMin() )
            );
            break;
        case( AlignmentSettings::RIGHT_BASE_LINE ) :
            textDrawable->setPosition(
                osg::vec3( extents.xMax(),
                           ( extents.yMin() + extents.yMax() ) *
                               0.5F -
                               textDrawable->getCharacterHeight() *
                               0.5F,
                           extents.zMin() )
            );
            break;

        case( AlignmentSettings::LEFT_BOTTOM_BASE_LINE ) :
        case( AlignmentSettings::CENTER_BOTTOM_BASE_LINE ) :
        case( AlignmentSettings::RIGHT_BOTTOM_BASE_LINE ) :

        default :
            textDrawable->setPosition(
                osg::vec3( extents.xMin(), extents.yMin(), extents.zMin() )
            );
            break;
    }

    return textDrawable.release();
}

osg::Node*
Style::createIcon( const osg::box&    extents,
                   const std::string& filename,
                   const osg::vec4&   color )
{
    osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile( filename );
    if( !object )
    {
        // OSG_NOTICE<<"Warning: Style::createIcon(.., "<<filename<<") could not find
        // icon file."<<std::endl; return 0;
    }

    osg::ref_ptr<osg::Image> image = dynamic_cast<osg::Image*>( object.get() );
    if( image.valid() )
    {
        osg::vec3 center( extents.center() );
        float     width              = extents.xMax() - extents.xMin();
        float     height             = extents.yMax() - extents.yMin();
        float     extentsAspectRatio = height / width;

        float     imageAspectRatio =
            static_cast<float>( image->t() ) / static_cast<float>( image->s() );
        if( imageAspectRatio > extentsAspectRatio )
        {
            width *= ( extentsAspectRatio / imageAspectRatio );
        }
        else
        {
            height *= ( imageAspectRatio / extentsAspectRatio );
        }

        osg::ref_ptr<osg::Geometry> geometry = osg::createTexturedQuadGeometry(
            osg::vec3( center.x - width * 0.5F, center.y - height * 0.5F, center.z ),
            osg::vec3( width, 0.0F, 0.0F ),
            osg::vec3( 0.0F, height, 0.0F )
        );

        osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
        colors->push_back( color );
        geometry->setColorArray( colors.get(), osg::Array::BIND_OVERALL );

        osg::ref_ptr<osg::Texture2D> texture  = new osg::Texture2D( image.get() );

        osg::ref_ptr<osg::StateSet>  stateset = geometry->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes( 0,
                                               texture.get(),
                                               osg::StateAttribute::ON );

        if( image->isImageTranslucent() )
        {
            stateset->setMode( GL_BLEND, osg::StateAttribute::ON );
        }

        return geometry.release();
    }

    osg::ref_ptr<osg::Node> node = dynamic_cast<osg::Node*>( object.get() );
    if( !node )
    {
        OSG_NOTICE << "Warning: Style::createIcon(.., " << filename
                   << ") could not find icon file." << std::endl;

        osg::ref_ptr<osg::ShapeDrawable> ds =
            new osg::ShapeDrawable( new osg::Sphere( osg::vec3( 0.0, 0.0, 0.0 ), 1.0 ) );

        node = ds.get();

        // return 0;
    }

    osg::ComputeBoundsVisitor cbv;
    node->accept( cbv );
    osg::box  bb = cbv.getBoundingBox();
    osg::vec3 bb_size( bb.xMax() - bb.xMin(),
                       bb.zMax() - bb.zMin(),
                       bb.zMax() - bb.zMin() );

    osg::vec3 scale(
        ( bb_size.x > 0 ) ? ( extents.xMax() - extents.xMin() ) / bb_size.x : 1.0F,
        ( bb_size.y > 0 ) ? ( extents.yMax() - extents.yMin() ) / bb_size.y : 1.0F,
        ( bb_size.z > 0 ) ? ( extents.zMax() - extents.zMin() ) / bb_size.z : 1.0F
    );

    float minNonZeroScale = scale.x;
    if( scale.y != 0.0 && scale.y < minNonZeroScale )
    {
        minNonZeroScale = scale.y;
    }
    if( scale.z != 0.0 && scale.z < minNonZeroScale )
    {
        minNonZeroScale = scale.z;
    }

    scale.set( minNonZeroScale, minNonZeroScale, minNonZeroScale );

    // create Transform to rescale subgraph
    osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
    transform->setMatrix( osg::dmat4( osg::translate( -bb.center() ) *
                                      osg::scale( scale ) *
                                      osg::translate( extents.center() ) ) );

    transform->setDataVariance( osg::Object::DataVariance::STATIC );
    transform->addChild( node.get() );

    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->addChild( transform.get() );

    {
        osgUtil::Optimizer::FlattenStaticTransformsVisitor fstv;
        group->accept( fstv );
        fstv.removeTransforms( group.get() );
    }

    if( group->getNumChildren() == 1 )
    {
        node = group->getChild( 0 );

        // remove references to avoid node from node being unreferenced after the node
        // ref_ptr<> is released().
        group     = 0;
        transform = 0;

        return node.release();
    }
    else
    {
        OSG_NOTICE << "Warning: Style::createIcon(.., " << filename
                   << "), error in creation of icon." << std::endl;
        return 0;
    }
}

osg::Node*
Style::createPanel( const osg::box&  extents,
                    const osg::vec4& colour )
{
    // OSG_NOTICE<<"createPanel"<<std::endl;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setName( "Panel" );

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray( vertices.get() );

    vertices->push_back( osg::vec3( extents.xMin(), extents.yMin(), extents.zMin() ) );
    vertices->push_back( osg::vec3( extents.xMin(), extents.yMax(), extents.zMin() ) );
    vertices->push_back( osg::vec3( extents.xMax(), extents.yMin(), extents.zMin() ) );
    vertices->push_back( osg::vec3( extents.xMax(), extents.yMax(), extents.zMin() ) );

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    geometry->setColorArray( colours.get(), osg::Array::BIND_OVERALL );

    colours->push_back( colour );

    geometry->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLE_STRIP, 0, 4 ) );

    return geometry.release();
}

osg::Node*
Style::createDepthSetPanel( const osg::box& extents )
{
    // OSG_NOTICE<<"createDepthSetPanel"<<std::endl;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setName( "DepthSetPanel" );

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray( vertices.get() );

    vertices->push_back( osg::vec3( extents.xMin(), extents.yMin(), extents.zMin() ) );
    vertices->push_back( osg::vec3( extents.xMin(), extents.yMax(), extents.zMin() ) );
    vertices->push_back( osg::vec3( extents.xMax(), extents.yMin(), extents.zMin() ) );
    vertices->push_back( osg::vec3( extents.xMax(), extents.yMax(), extents.zMin() ) );

    geometry->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLE_STRIP, 0, 4 ) );

    osg::ref_ptr<osg::StateSet> stateset = geometry->getOrCreateStateSet();
    stateset->setAttributeAndModes( _enabledDepthWrite.get(),
                                    osg::StateAttribute::ON |
                                        osg::StateAttribute::PROTECTED );
    stateset->setAttributeAndModes( _disableColorWriteMask.get(),
                                    osg::StateAttribute::ON |
                                        osg::StateAttribute::PROTECTED );
    stateset->setRenderBinDetails( 20,
                                   "TraversalOrderBin",
                                   osg::StateSet::OVERRIDE_PROTECTED_RENDERBIN_DETAILS );
    stateset->setNestRenderBins( false );

    return geometry.release();
}

void
Style::setupDialogStateSet( osg::StateSet* stateset,
                            int            binNum )
{
    stateset->setRenderBinDetails( binNum,
                                   "TraversalOrderBin",
                                   osg::StateSet::OVERRIDE_PROTECTED_RENDERBIN_DETAILS );
    stateset->setAttributeAndModes( _disabledDepthWrite.get(),
                                    osg::StateAttribute::ON |
                                        osg::StateAttribute::OVERRIDE );
    stateset->setNestRenderBins( false );
}

void
Style::setupPopupStateSet( osg::StateSet* /*stateset*/,
                           int /*binNum*/ )
{
}

void
Style::setupClipStateSet( const osg::box& extents,
                          osg::StateSet*  stateset )
{
    unsigned int clipTextureUnit = 1;

    stateset->setTextureAttributeAndModes( clipTextureUnit,
                                           _clipTexture.get(),
                                           osg::StateAttribute::ON |
                                               osg::StateAttribute::OVERRIDE );

    // Matrix computation intentionally unused (kept for reference)
    ( void )osg::dmat4(
        osg::translate(
            osg::vec3( -extents.xMin(), -extents.yMin(), -extents.zMin() )
        ) *
        osg::scale( osg::vec3( 1.0F / ( extents.xMax() - extents.xMin() ),
                               1.0F / ( extents.yMax() - extents.yMin() ),
                               1.0F ) )
    );

    OSG_NOTICE << "setupClipState(" << extents.xMin() << ", " << extents.yMin() << ", "
               << extents.zMin() << ", " << extents.xMax() << ", " << extents.yMax()
               << ", " << extents.zMax() << ")" << std::endl;
}
