#include <osgFX/SpecularHighlights>
#include <osgFX/Registry>

#include <osg/TextureCubeMap>

#include <osgUtil/HighlightMapGenerator>

using namespace osgFX;

namespace
{

    class AutoTextureMatrix: public osg::StateAttribute {
    public:
        AutoTextureMatrix()
        :    osg::StateAttribute(),
            _lightnum(0),
            _active(false)
        {
        }

        AutoTextureMatrix(const AutoTextureMatrix& copy, const osg::CopyOp& copyop)
        :    osg::StateAttribute(copy, copyop),
            _lightnum(copy._lightnum),
            _active(copy._active)
        {
        }

        AutoTextureMatrix(int lightnum, bool active = true)
        :    osg::StateAttribute(),
            _lightnum(lightnum),
            _active(active)
        {
        }

        META_StateAttribute(osgFX, AutoTextureMatrix, osg::StateAttribute::TEXMAT);

        virtual bool isTextureAttribute() const { return true; }

        int compare(const osg::StateAttribute &sa) const
        {
            COMPARE_StateAttribute_Types(AutoTextureMatrix, sa);
            if (_lightnum < rhs._lightnum) return -1;
            if (_lightnum > rhs._lightnum) return 1;
            return 0;
        }

        void apply(osg::State& state) const
        {
            OSG_NOTICE<<"Warning: osgFX::SpecularHighlights unable to set texture matrix."<<std::endl;
        }

    private:
        int _lightnum;
        bool _active;
    };

}

namespace
{

    Registry::Proxy proxy(new SpecularHighlights);

    class DefaultTechnique: public Technique {
    public:

        DefaultTechnique(int lightnum, int unit, const osg::Vec4& color, float sexp)
        :    Technique(),
            _lightnum(lightnum),
            _unit(unit),
            _color(color),
            _sexp(sexp)
        {
        }

        virtual void getRequiredExtensions(std::vector<std::string>& /*extensions*/) const
        {
        }

        bool validate(osg::State& state) const
        {
            if (!Technique::validate(state)) return false;

            osg::GLExtensions *ext = state.get<osg::GLExtensions>();
            return ext ?  ext->isCubeMapSupported : false;
        }

    protected:

        void define_passes()
        {
            osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

            ss->setTextureAttributeAndModes(_unit, new AutoTextureMatrix(_lightnum), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            osg::ref_ptr<osgUtil::HighlightMapGenerator> hmg = new osgUtil::HighlightMapGenerator(osg::Vec3(0, 0, -1), _color, _sexp);
            hmg->generateMap(false);

            osg::ref_ptr<osg::TextureCubeMap> texture = new osg::TextureCubeMap;
            texture->setImage(osg::TextureCubeMap::POSITIVE_X, hmg->getImage(osg::TextureCubeMap::POSITIVE_X));
            texture->setImage(osg::TextureCubeMap::POSITIVE_Y, hmg->getImage(osg::TextureCubeMap::POSITIVE_Y));
            texture->setImage(osg::TextureCubeMap::POSITIVE_Z, hmg->getImage(osg::TextureCubeMap::POSITIVE_Z));
            texture->setImage(osg::TextureCubeMap::NEGATIVE_X, hmg->getImage(osg::TextureCubeMap::NEGATIVE_X));
            texture->setImage(osg::TextureCubeMap::NEGATIVE_Y, hmg->getImage(osg::TextureCubeMap::NEGATIVE_Y));
            texture->setImage(osg::TextureCubeMap::NEGATIVE_Z, hmg->getImage(osg::TextureCubeMap::NEGATIVE_Z));
            texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
            texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
            texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
            ss->setTextureAttributeAndModes(_unit, texture.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            addPass(ss.get());
        }

    private:
        int _lightnum;
        int _unit;
        osg::Vec4 _color;
        float _sexp;
    };

}


SpecularHighlights::SpecularHighlights()
:    Effect(),
    _lightnum(0),
    _unit(0),
    _color(1, 1, 1, 1),
    _sexp(16)
{
}

SpecularHighlights::SpecularHighlights(const SpecularHighlights& copy, const osg::CopyOp& copyop)
:    Effect(copy, copyop),
    _lightnum(copy._lightnum),
    _unit(copy._unit),
    _color(copy._color),
    _sexp(copy._sexp)
{
}

bool SpecularHighlights::define_techniques()
{
    addTechnique(new DefaultTechnique(_lightnum, _unit, _color, _sexp));
    return true;
}
