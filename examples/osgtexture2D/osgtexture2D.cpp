/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgtexture2D example application
 */
#include <osg/core/Notify.hpp>
#include <osg/geometry/DrawPixels.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgText/Text>
#include <osgViewer/core/Viewer.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// filter wall and animation callback.
//

class FilterCallback : public osg::NodeCallback
{
    public:

        FilterCallback( osg::Texture2D* texture,
                        osgText::Text*  text,
                        double          delay = 1.0 ) :
            _texture( texture ),
            _text( text ),
            _delay( delay ),
            _currPos( 0 ),
            _prevTime( 0.0 )
        {
            // start with a mip mapped mode to ensure that
            _minFilterList.push_back( osg::Texture2D::LINEAR_MIPMAP_LINEAR );
            _magFilterList.push_back( osg::Texture2D::LINEAR );
            _textList.push_back( "Tri-linear mip mapping (default "
                                 "filtering)\nsetFilter(MIN_FILTER,LINEAR_MIP_LINEAR)"
                                 "\nsetFilter(MAG_FILTER,LINEAR)" );

            _minFilterList.push_back( osg::Texture2D::NEAREST );
            _magFilterList.push_back( osg::Texture2D::NEAREST );
            _textList.push_back(
                "Nearest "
                "filtering\nsetFilter(MIN_FILTER,NEAREST)\nsetFilter(MAG_FILTER,NEAREST)"
            );

            _minFilterList.push_back( osg::Texture2D::LINEAR );
            _magFilterList.push_back( osg::Texture2D::LINEAR );
            _textList.push_back(
                "Linear "
                "filtering\nsetFilter(MIN_FILTER,LINEAR)\nsetFilter(MAG_FILTER,LINEAR)"
            );

            _minFilterList.push_back( osg::Texture2D::NEAREST_MIPMAP_NEAREST );
            _magFilterList.push_back( osg::Texture2D::LINEAR );
            _textList.push_back(
                "nearest mip mapping (default "
                "filtering)\nsetFilter(MIN_FILTER,)\nsetFilter(MAG_FILTER,LINEAR)"
            );

            _minFilterList.push_back( osg::Texture2D::LINEAR_MIPMAP_NEAREST );
            _magFilterList.push_back( osg::Texture2D::LINEAR );
            _textList.push_back( "bi-linear mip "
                                 "mapping\nsetFilter(MIN_FILTER,LINEAR_MIPMAP_NEAREST)"
                                 "\nsetFilter(MAG_FILTER,LINEAR)" );

            _minFilterList.push_back( osg::Texture2D::NEAREST_MIPMAP_LINEAR );
            _magFilterList.push_back( osg::Texture2D::LINEAR );
            _textList.push_back( "bi-linear mip "
                                 "mapping\nsetFilter(MIN_FILTER,NEAREST_MIPMAP_LINEAR)"
                                 "\nsetFilter(MAG_FILTER,LINEAR)" );

            setValues();
        }

        virtual void
        operator()( osg::Node*,
                    osg::NodeVisitor* nv )
        {
            if( nv->getFrameStamp() )
            {
                double currTime = nv->getFrameStamp()->getSimulationTime();
                if( currTime - _prevTime > _delay )
                {
                    // update filter modes and text.
                    setValues();

                    // advance the current position, wrap round if required.
                    _currPos++;
                    if( _currPos >= _minFilterList.size() )
                    {
                        _currPos = 0;
                    }

                    // record time
                    _prevTime = currTime;
                }
            }
        }

        void
        setValues()
        {
            _texture->setFilter( osg::Texture2D::MIN_FILTER, _minFilterList[_currPos] );
            _texture->setFilter( osg::Texture2D::MAG_FILTER, _magFilterList[_currPos] );

            _text->setText( _textList[_currPos] );
        }

    protected:

        typedef std::vector<osg::Texture2D::FilterMode> FilterList;
        typedef std::vector<std::string>                TextList;

        osg::ref_ptr<osg::Texture2D>                    _texture;
        osg::ref_ptr<osgText::Text>                     _text;
        double                                          _delay;

        FilterList                                      _minFilterList;
        FilterList                                      _magFilterList;
        TextList                                        _textList;

        unsigned int                                    _currPos;
        double                                          _prevTime;
};

osg::Node*
createFilterWall( osg::box&          bb,
                  const std::string& filename )
{
    osg::Group*     group = new osg::Group;

    // left hand side of bounding box.
    osg::vec3       top_left( bb.xMin(), bb.yMin(), bb.zMax() );
    osg::vec3       bottom_left( bb.xMin(), bb.yMin(), bb.zMin() );
    osg::vec3       bottom_right( bb.xMin(), bb.yMax(), bb.zMin() );
    osg::vec3       top_right( bb.xMin(), bb.yMax(), bb.zMax() );
    osg::vec3       center( bb.xMin(),
                            ( bb.yMin() + bb.yMax() ) * 0.5F,
                            ( bb.zMin() + bb.zMax() ) * 0.5F );
    float           height = bb.zMax() - bb.zMin();

    // create the geometry for the wall.
    osg::Geometry*  geom     = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array( 4 );
    ( *vertices )[0]         = top_left;
    ( *vertices )[1]         = bottom_left;
    ( *vertices )[2]         = bottom_right;
    ( *vertices )[3]         = top_right;
    geom->setVertexArray( vertices );

    osg::Vec2Array* texcoords = new osg::Vec2Array( 4 );
    ( *texcoords )[0].set( 0.0F, 1.0F );
    ( *texcoords )[1].set( 0.0F, 0.0F );
    ( *texcoords )[2].set( 1.0F, 0.0F );
    ( *texcoords )[3].set( 1.0F, 1.0F );
    geom->setTexCoordArray( 0, texcoords );

    osg::Vec3Array* normals = new osg::Vec3Array( 1 );
    ( *normals )[0].set( 1.0F, 0.0F, 0.0F );
    geom->setNormalArray( normals, osg::Array::BIND_OVERALL );

    osg::Vec4Array* colors = new osg::Vec4Array( 1 );
    ( *colors )[0].set( 1.0F, 1.0F, 1.0F, 1.0F );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );

    {
        osg::DrawElementsUShort* indices = new osg::DrawElementsUShort( GL_TRIANGLES );
        indices->push_back( 0 );
        indices->push_back( 1 );
        indices->push_back( 2 );
        indices->push_back( 0 );
        indices->push_back( 2 );
        indices->push_back( 3 );
        geom->addPrimitiveSet( indices );
    }

    osg::Geode* geom_geode = new osg::Geode;
    geom_geode->addDrawable( geom );
    group->addChild( geom_geode );

    // set up the texture state.
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setDataVariance(
        osg::Object::DataVariance::DYNAMIC
    );    // protect from being optimized away as static state.
    texture->setImage( osgDB::readRefImageFile( filename ) );

    osg::StateSet* stateset = geom->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

    // create the text label.

    osgText::Text* text = new osgText::Text;
    text->setDataVariance( osg::Object::DataVariance::DYNAMIC );

    text->setFont( "fonts/arial.ttf" );
    text->setPosition( center );
    text->setCharacterSize( height * 0.03F );
    text->setAlignment( osgText::Text::CENTER_CENTER );
    text->setAxisAlignment( osgText::Text::YZ_PLANE );

    osg::Geode* text_geode = new osg::Geode;
    text_geode->addDrawable( text );

    osg::StateSet* text_stateset = text_geode->getOrCreateStateSet();
    text_stateset->setAttributeAndModes( new osg::PolygonOffset( -1.0F, -1.0F ),
                                         osg::StateAttribute::ON );

    group->addChild( text_geode );

    // set the update callback to cycle through the various min and mag filter modes.
    group->setUpdateCallback( new FilterCallback( texture, text ) );

    return group;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// anisotropic wall and animation callback.
//

class AnisotropicCallback : public osg::NodeCallback
{
    public:

        AnisotropicCallback( osg::Texture2D* texture,
                             osgText::Text*  text,
                             double          delay = 1.0 ) :
            _texture( texture ),
            _text( text ),
            _delay( delay ),
            _currPos( 0 ),
            _prevTime( 0.0 )
        {

            _maxAnisotropyList.push_back( 1.0F );
            _textList.push_back(
                "No anisotropic filtering (default)\nsetMaxAnisotropy(1.0f)"
            );

            _maxAnisotropyList.push_back( 2.0F );
            _textList.push_back( "Anisotropic filtering\nsetMaxAnisotropy(2.0f)" );

            _maxAnisotropyList.push_back( 4.0F );
            _textList.push_back( "Anisotropic filtering\nsetMaxAnisotropy(4.0f)" );

            _maxAnisotropyList.push_back( 8.0F );
            _textList.push_back( "Anisotropic filtering\nsetMaxAnisotropy(8.0f)" );

            _maxAnisotropyList.push_back( 16.0F );
            _textList.push_back(
                "Highest quality anisotropic filtering\nsetMaxAnisotropy(16.0f)"
            );

            setValues();
        }

        virtual void
        operator()( osg::Node*,
                    osg::NodeVisitor* nv )
        {
            if( nv->getFrameStamp() )
            {
                double currTime = nv->getFrameStamp()->getSimulationTime();
                if( currTime - _prevTime > _delay )
                {
                    // update filter modes and text.
                    setValues();

                    // advance the current position, wrap round if required.
                    _currPos++;
                    if( _currPos >= _maxAnisotropyList.size() )
                    {
                        _currPos = 0;
                    }

                    // record time
                    _prevTime = currTime;
                }
            }
        }

        void
        setValues()
        {
            _texture->setMaxAnisotropy( _maxAnisotropyList[_currPos] );

            _text->setText( _textList[_currPos] );
        }

    protected:

        typedef std::vector<float>       AnisotropyList;
        typedef std::vector<std::string> TextList;

        osg::ref_ptr<osg::Texture2D>     _texture;
        osg::ref_ptr<osgText::Text>      _text;
        double                           _delay;

        AnisotropyList                   _maxAnisotropyList;
        TextList                         _textList;

        unsigned int                     _currPos;
        double                           _prevTime;
};

osg::Node*
createAnisotripicWall( osg::box&          bb,
                       const std::string& filename )
{
    osg::Group*     group = new osg::Group;

    // left hand side of bounding box.
    osg::vec3       top_left( bb.xMin(), bb.yMax(), bb.zMin() );
    osg::vec3       bottom_left( bb.xMin(), bb.yMin(), bb.zMin() );
    osg::vec3       bottom_right( bb.xMax(), bb.yMin(), bb.zMin() );
    osg::vec3       top_right( bb.xMax(), bb.yMax(), bb.zMin() );
    osg::vec3       center( ( bb.xMin() + bb.xMax() ) * 0.5F,
                            ( bb.yMin() + bb.yMax() ) * 0.5F,
                            bb.zMin() );
    float           height = bb.yMax() - bb.yMin();

    // create the geometry for the wall.
    osg::Geometry*  geom     = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array( 4 );
    ( *vertices )[0]         = top_left;
    ( *vertices )[1]         = bottom_left;
    ( *vertices )[2]         = bottom_right;
    ( *vertices )[3]         = top_right;
    geom->setVertexArray( vertices );

    osg::Vec2Array* texcoords = new osg::Vec2Array( 4 );
    ( *texcoords )[0].set( 0.0F, 1.0F );
    ( *texcoords )[1].set( 0.0F, 0.0F );
    ( *texcoords )[2].set( 1.0F, 0.0F );
    ( *texcoords )[3].set( 1.0F, 1.0F );
    geom->setTexCoordArray( 0, texcoords );

    osg::Vec3Array* normals = new osg::Vec3Array( 1 );
    ( *normals )[0].set( 0.0F, 0.0F, 1.0F );
    geom->setNormalArray( normals, osg::Array::BIND_OVERALL );

    osg::Vec4Array* colors = new osg::Vec4Array( 1 );
    ( *colors )[0].set( 1.0F, 1.0F, 1.0F, 1.0F );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );

    {
        osg::DrawElementsUShort* indices = new osg::DrawElementsUShort( GL_TRIANGLES );
        indices->push_back( 0 );
        indices->push_back( 1 );
        indices->push_back( 2 );
        indices->push_back( 0 );
        indices->push_back( 2 );
        indices->push_back( 3 );
        geom->addPrimitiveSet( indices );
    }

    osg::Geode* geom_geode = new osg::Geode;
    geom_geode->addDrawable( geom );
    group->addChild( geom_geode );

    // set up the texture state.
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setDataVariance(
        osg::Object::DataVariance::DYNAMIC
    );    // protect from being optimized away as static state.
    texture->setImage( osgDB::readRefImageFile( filename ) );

    osg::StateSet* stateset = geom->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

    // create the text label.

    osgText::Text* text = new osgText::Text;
    text->setDataVariance( osg::Object::DataVariance::DYNAMIC );
    text->setFont( "fonts/arial.ttf" );
    text->setPosition( center );
    text->setCharacterSize( height * 0.03F );
    text->setColor( osg::vec4( 1.0F, 0.0F, 1.0F, 1.0F ) );
    text->setAlignment( osgText::Text::CENTER_CENTER );
    text->setAxisAlignment( osgText::Text::XY_PLANE );

    osg::Geode* text_geode = new osg::Geode;
    text_geode->addDrawable( text );

    osg::StateSet* text_stateset = text_geode->getOrCreateStateSet();
    text_stateset->setAttributeAndModes( new osg::PolygonOffset( -1.0F, -1.0F ),
                                         osg::StateAttribute::ON );

    group->addChild( text_geode );

    // set the update callback to cycle through the various min and mag filter modes.
    group->setUpdateCallback( new AnisotropicCallback( texture, text ) );

    return group;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// wrap wall and animation callback.
//
class WrapCallback : public osg::NodeCallback
{
    public:

        WrapCallback( osg::Texture2D* texture,
                      osgText::Text*  text,
                      double          delay = 1.0 ) :
            _texture( texture ),
            _text( text ),
            _delay( delay ),
            _currPos( 0 ),
            _prevTime( 0.0 )
        {

            _wrapList.push_back( osg::Texture2D::CLAMP );
            _textList.push_back( "Default tex coord clamp\nsetWrap(WRAP_S,CLAMP)" );

            _wrapList.push_back( osg::Texture2D::CLAMP_TO_EDGE );
            _textList.push_back(
                "Clamp to edge extension\nsetWrap(WRAP_S,CLAMP_TO_EDGE)"
            );

            _wrapList.push_back( osg::Texture2D::CLAMP_TO_BORDER );
            _textList.push_back(
                "Clamp to border color extension\nsetWrap(WRAP_S,CLAMP_TO_BORDER)"
            );

            _wrapList.push_back( osg::Texture2D::REPEAT );
            _textList.push_back( "Repeat wrap\nsetWrap(WRAP_S,REPEAT)" );

            _wrapList.push_back( osg::Texture2D::MIRROR );
            _textList.push_back( "Mirror wrap extension\nsetWrap(WRAP_S,MIRROR)" );

            setValues();
        }

        virtual void
        operator()( osg::Node*,
                    osg::NodeVisitor* nv )
        {
            if( nv->getFrameStamp() )
            {
                double currTime = nv->getFrameStamp()->getSimulationTime();
                if( currTime - _prevTime > _delay )
                {
                    // update filter modes and text.
                    setValues();

                    // advance the current position, wrap round if required.
                    _currPos++;
                    if( _currPos >= _wrapList.size() )
                    {
                        _currPos = 0;
                    }

                    // record time
                    _prevTime = currTime;
                }
            }
        }

        void
        setValues()
        {
            _texture->setWrap( osg::Texture2D::WRAP_S, _wrapList[_currPos] );
            _texture->setWrap( osg::Texture2D::WRAP_T, _wrapList[_currPos] );

            _text->setText( _textList[_currPos] );
        }

    protected:

        typedef std::vector<osg::Texture2D::WrapMode> WrapList;
        typedef std::vector<std::string>              TextList;

        osg::ref_ptr<osg::Texture2D>                  _texture;
        osg::ref_ptr<osgText::Text>                   _text;
        double                                        _delay;

        WrapList                                      _wrapList;
        TextList                                      _textList;

        unsigned int                                  _currPos;
        double                                        _prevTime;
};

osg::Node*
createWrapWall( osg::box&          bb,
                const std::string& filename )
{
    osg::Group*     group = new osg::Group;

    // left hand side of bounding box.
    osg::vec3       top_left( bb.xMax(), bb.yMax(), bb.zMax() );
    osg::vec3       bottom_left( bb.xMax(), bb.yMax(), bb.zMin() );
    osg::vec3       bottom_right( bb.xMax(), bb.yMin(), bb.zMin() );
    osg::vec3       top_right( bb.xMax(), bb.yMin(), bb.zMax() );
    osg::vec3       center( bb.xMax(),
                            ( bb.yMin() + bb.yMax() ) * 0.5F,
                            ( bb.zMin() + bb.zMax() ) * 0.5F );
    float           height = bb.zMax() - bb.zMin();

    // create the geometry for the wall.
    osg::Geometry*  geom     = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array( 4 );
    ( *vertices )[0]         = top_left;
    ( *vertices )[1]         = bottom_left;
    ( *vertices )[2]         = bottom_right;
    ( *vertices )[3]         = top_right;
    geom->setVertexArray( vertices );

    osg::Vec2Array* texcoords = new osg::Vec2Array( 4 );
    ( *texcoords )[0].set( -1.0F, 2.0F );
    ( *texcoords )[1].set( -1.0F, -1.0F );
    ( *texcoords )[2].set( 2.0F, -1.0F );
    ( *texcoords )[3].set( 2.0F, 2.0F );
    geom->setTexCoordArray( 0, texcoords );

    osg::Vec3Array* normals = new osg::Vec3Array( 1 );
    ( *normals )[0].set( -1.0F, 0.0F, 0.0F );
    geom->setNormalArray( normals, osg::Array::BIND_OVERALL );

    osg::Vec4Array* colors = new osg::Vec4Array( 1 );
    ( *colors )[0].set( 1.0F, 1.0F, 1.0F, 1.0F );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );

    {
        osg::DrawElementsUShort* indices = new osg::DrawElementsUShort( GL_TRIANGLES );
        indices->push_back( 0 );
        indices->push_back( 1 );
        indices->push_back( 2 );
        indices->push_back( 0 );
        indices->push_back( 2 );
        indices->push_back( 3 );
        geom->addPrimitiveSet( indices );
    }

    osg::Geode* geom_geode = new osg::Geode;
    geom_geode->addDrawable( geom );
    group->addChild( geom_geode );

    // set up the texture state.
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setDataVariance(
        osg::Object::DataVariance::DYNAMIC
    );    // protect from being optimized away as static state.
    texture->setBorderColor(
        osg::dvec4( 1.0F, 1.0F, 1.0F, 0.5F )
    );    // only used when wrap is set to CLAMP_TO_BORDER
    texture->setImage( osgDB::readRefImageFile( filename ) );

    osg::StateSet* stateset = geom->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
    stateset->setMode( GL_BLEND, osg::StateAttribute::ON );
    stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    // create the text label.

    osgText::Text* text = new osgText::Text;
    text->setDataVariance( osg::Object::DataVariance::DYNAMIC );
    text->setFont( "fonts/arial.ttf" );
    text->setPosition( center );
    text->setCharacterSize( height * 0.03F );
    text->setAlignment( osgText::Text::CENTER_CENTER );
    text->setAxisAlignment( osgText::Text::YZ_PLANE );

    osg::Geode* text_geode = new osg::Geode;
    text_geode->addDrawable( text );

    osg::StateSet* text_stateset = text_geode->getOrCreateStateSet();
    text_stateset->setAttributeAndModes( new osg::PolygonOffset( -1.0F, -1.0F ),
                                         osg::StateAttribute::ON );

    group->addChild( text_geode );

    // set the update callback to cycle through the various min and mag filter modes.
    group->setUpdateCallback( new WrapCallback( texture, text ) );

    return group;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// sublooad wall and animation callback.
//

class ImageUpdateCallback : public osg::NodeCallback
{
    public:

        ImageUpdateCallback( osg::Texture2D* texture,
                             osgText::Text*  text,
                             double          delay = 1.0 ) :
            _texture( texture ),
            _text( text ),
            _delay( delay ),
            _currPos( 0 ),
            _prevTime( 0.0 )
        {

#if 1
            osg::ref_ptr<osg::Image> originalImage =
                osgDB::readRefImageFile( "Images/dog_left_eye.jpg" );

            osg::ref_ptr<osg::Image> subImage = new osg::Image;
            subImage->setUserData(
                originalImage.get()
            );    // attach the originalImage as user data to prevent it being deleted.

            // now assign the appropriate portion data from the originalImage
            subImage->setImage(
                originalImage->s() / 2,
                originalImage->t() / 2,
                originalImage->r(),                  // half the width and height
                originalImage
                    ->getInternalTextureFormat(),    // same internal texture format
                originalImage->getPixelFormat(),
                originalImage->getDataType(),        // same pixel format and data type
                originalImage->data(
                    originalImage->s() / 4,
                    originalImage->t() / 4
                ),                        // offset the start point to 1/4 into the image
                osg::Image::NO_DELETE,    // don't attempt to delete the image data,
                                          // leave this to the originalImage
                originalImage->getPacking(),    // use the same packing
                originalImage->s()
            );    // use the width of the original image as the row width

            subImage->setPixelBufferObject(
                new osg::PixelBufferObject( subImage.get() )
            );

    #if 0
        OSG_NOTICE<<"orignalImage iterator"<<std::endl;
        for(osg::Image::DataIterator itr(originalImage.get()); itr.valid(); ++itr)
        {
            OSG_NOTICE<<"  "<<(void*)itr.data()<<", "<<itr.size()<<std::endl;
        }

        OSG_NOTICE<<"subImage iterator, size "<<subImage->s()<<", "<<subImage->t()<<std::endl;
        unsigned int i=0;
        for(osg::Image::DataIterator itr(subImage.get()); itr.valid(); ++itr, ++i)
        {
            OSG_NOTICE<<"  "<<i<<", "<<(void*)itr.data()<<", "<<itr.size()<<std::endl;

            for(unsigned char* d=const_cast<unsigned char*>(itr.data()); d<(itr.data()+itr.size()); ++d)
            {
                *d = 255-*d;
            }
        }
    #endif

            _imageList.push_back( subImage );

#else
            _imageList.push_back( osgDB::readRefImageFile( "Images/dog_left_eye.jpg" ) );
#endif
            _textList.push_back( "Subloaded Image 1 - dog_left_eye.jpg" );

            _imageList.push_back(
                osgDB::readRefImageFile( "Images/dog_right_eye.jpg" )
            );
            _textList.push_back( "Subloaded Image 2 - dog_right_eye.jpg" );

            setValues();
        }

        virtual void
        operator()( osg::Node*,
                    osg::NodeVisitor* nv )
        {
            if( nv->getFrameStamp() )
            {
                double currTime = nv->getFrameStamp()->getSimulationTime();
                if( currTime - _prevTime > _delay )
                {
                    // update filter modes and text.
                    setValues();

                    // advance the current position, wrap round if required.
                    _currPos++;
                    if( _currPos >= _imageList.size() )
                    {
                        _currPos = 0;
                    }

                    // record time
                    _prevTime = currTime;
                }
            }
        }

        void
        setValues()
        {
            // Note, as long as the images are the same dimensions subloading will be
            // used to update the textures.  If dimensions change then the texture
            // objects have to be deleted and re-recreated.
            //
            // The load/subload happens during the draw traversal so doesn't happen on
            // the setImage which just updates internal pointers and modified flags.

            _texture->setImage( _imageList[_currPos].get() );

            //_texture->dirtyTextureObject();

            _text->setText( _textList[_currPos] );
        }

    protected:

        typedef std::vector<osg::ref_ptr<osg::Image>> ImageList;
        typedef std::vector<std::string>              TextList;

        osg::ref_ptr<osg::Texture2D>                  _texture;
        osg::ref_ptr<osgText::Text>                   _text;
        double                                        _delay;

        ImageList                                     _imageList;
        TextList                                      _textList;

        unsigned int                                  _currPos;
        double                                        _prevTime;
};

osg::Node*
createSubloadWall( osg::box& bb )
{
    osg::Group*     group = new osg::Group;

    // left hand side of bounding box.
    osg::vec3       top_left( bb.xMin(), bb.yMax(), bb.zMax() );
    osg::vec3       bottom_left( bb.xMin(), bb.yMax(), bb.zMin() );
    osg::vec3       bottom_right( bb.xMax(), bb.yMax(), bb.zMin() );
    osg::vec3       top_right( bb.xMax(), bb.yMax(), bb.zMax() );
    osg::vec3       center( ( bb.xMax() + bb.xMin() ) * 0.5F,
                            bb.yMax(),
                            ( bb.zMin() + bb.zMax() ) * 0.5F );
    float           height = bb.zMax() - bb.zMin();

    // create the geometry for the wall.
    osg::Geometry*  geom     = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array( 4 );
    ( *vertices )[0]         = top_left;
    ( *vertices )[1]         = bottom_left;
    ( *vertices )[2]         = bottom_right;
    ( *vertices )[3]         = top_right;
    geom->setVertexArray( vertices );

    osg::Vec2Array* texcoords = new osg::Vec2Array( 4 );
    ( *texcoords )[0].set( 0.0F, 1.0F );
    ( *texcoords )[1].set( 0.0F, 0.0F );
    ( *texcoords )[2].set( 1.0F, 0.0F );
    ( *texcoords )[3].set( 1.0F, 1.0F );
    geom->setTexCoordArray( 0, texcoords );

    osg::Vec3Array* normals = new osg::Vec3Array( 1 );
    ( *normals )[0].set( 0.0F, -1.0F, 0.0F );
    geom->setNormalArray( normals, osg::Array::BIND_OVERALL );

    osg::Vec4Array* colors = new osg::Vec4Array( 1 );
    ( *colors )[0].set( 1.0F, 1.0F, 1.0F, 1.0F );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );

    {
        osg::DrawElementsUShort* indices = new osg::DrawElementsUShort( GL_TRIANGLES );
        indices->push_back( 0 );
        indices->push_back( 1 );
        indices->push_back( 2 );
        indices->push_back( 0 );
        indices->push_back( 2 );
        indices->push_back( 3 );
        geom->addPrimitiveSet( indices );
    }

    osg::Geode* geom_geode = new osg::Geode;
    geom_geode->addDrawable( geom );
    group->addChild( geom_geode );

    // set up the texture state.
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setDataVariance(
        osg::Object::DataVariance::DYNAMIC
    );    // protect from being optimized away as static state.
    texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );

    osg::StateSet* stateset = geom->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

    // create the text label.

    osgText::Text* text = new osgText::Text;
    text->setDataVariance( osg::Object::DataVariance::DYNAMIC );
    text->setFont( "fonts/arial.ttf" );
    text->setPosition( center );
    text->setCharacterSize( height * 0.03F );
    text->setAlignment( osgText::Text::CENTER_CENTER );
    text->setAxisAlignment( osgText::Text::XZ_PLANE );

    osg::Geode* text_geode = new osg::Geode;
    text_geode->addDrawable( text );

    osg::StateSet* text_stateset = text_geode->getOrCreateStateSet();
    text_stateset->setAttributeAndModes( new osg::PolygonOffset( -1.0F, -1.0F ),
                                         osg::StateAttribute::ON );

    group->addChild( text_geode );

    // set the update callback to cycle through the various min and mag filter modes.
    group->setUpdateCallback( new ImageUpdateCallback( texture, text ) );

    return group;
}

osg::Node*
createModel()
{

    // create the root node which will hold the model.
    osg::Group* root = new osg::Group();

    osg::box    bb( 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F );

    root->addChild( createFilterWall( bb, "Images/lz.rgb" ) );
    root->addChild( createAnisotripicWall( bb, "Images/primitives.gif" ) );
    root->addChild( createWrapWall( bb, "Images/tree0.rgba" ) );
    root->addChild( createSubloadWall( bb ) );

    return root;
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;

    // add model to viewer.
    viewer.setSceneData( createModel() );
    return viewer.run();
}
