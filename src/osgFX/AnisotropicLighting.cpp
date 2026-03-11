#include <osgFX/AnisotropicLighting>
#include <osgFX/Registry>

#include <osg/Texture2D>

#include <osgDB/ReadFile>

#include <sstream>

using namespace osgFX;

namespace
{

    osg::Image* create_default_image()
    {
        const int _texturesize = 16;
        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->setImage(_texturesize, _texturesize, 1, 3, GL_RGB, GL_UNSIGNED_BYTE, new unsigned char[3*_texturesize*_texturesize], osg::Image::USE_NEW_DELETE);
        for (int i=0; i<_texturesize; ++i) {
            for (int j=0; j<_texturesize; ++j) {
                float s = static_cast<float>(j) / (_texturesize-1);
                float t = static_cast<float>(i) / (_texturesize-1);
                float lum = t * 0.75f;
                float red = lum + 0.2f * powf(cosf(s*10), 3.0f);
                float green = lum;
                float blue = lum + 0.2f * powf(sinf(s*10), 3.0f);
                if (red > 1) red = 1;
                if (red < 0) red = 0;
                if (blue > 1) blue = 1;
                if (blue < 0) blue = 0;
                *(image->data(j, i)+0) = static_cast<unsigned char>(red * 255);
                *(image->data(j, i)+1) = static_cast<unsigned char>(green * 255);
                *(image->data(j, i)+2) = static_cast<unsigned char>(blue * 255);
            }
        }
        return image.release();
    }

}

namespace
{

    Registry::Proxy proxy(new AnisotropicLighting);

}


AnisotropicLighting::AnisotropicLighting()
:    Effect(),
    _lightnum(0),
    _texture(new osg::Texture2D)
{
    _texture->setImage(create_default_image());
    _texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    _texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
}

AnisotropicLighting::AnisotropicLighting(const AnisotropicLighting& copy, const osg::CopyOp& copyop)
:    Effect(copy, copyop),
    _lightnum(copy._lightnum),
    _texture(static_cast<osg::Texture2D *>(copyop(copy._texture.get())))
{
}

bool AnisotropicLighting::define_techniques()
{
    // ARB assembly program technique has been removed.
    // TODO: implement GLSL-based anisotropic lighting technique.
    return true;
}
