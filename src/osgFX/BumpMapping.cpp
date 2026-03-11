#include <osgFX/BumpMapping>
#include <osgFX/Registry>

#include <osg/Texture2D>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/Geometry>
#include <osg/Notify>

#include <osgUtil/TangentSpaceGenerator>

#include <osgDB/ReadFile>

#include <sstream>

using namespace osgFX;

namespace
{

    using osg::NodeVisitor;

     // this is a visitor class that prepares all geometries in a subgraph
    // by calling prepareGeometry() which in turn generates tangent-space
    // basis vectors
    class TsgVisitor: public NodeVisitor {
    public:
        TsgVisitor(BumpMapping* bm): NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN), _bm(bm) {}

        META_NodeVisitor("osgFX","TsgVisitor")

        void apply(osg::Geode& geode)
        {
            for (unsigned i=0; i<geode.getNumDrawables(); ++i) {
                osg::Geometry* geo = dynamic_cast<osg::Geometry* >(geode.getDrawable(i));
                if (geo) {
                    _bm->prepareGeometry(geo);
                }
            }
            NodeVisitor::apply(geode);
        }
    private:
        BumpMapping* _bm;
    };


    // this visitor generates texture coordinates for all geometries in a
    // subgraph. It is used only for demo purposes.
    class TexCoordGenerator: public osg::NodeVisitor {
    public:
        TexCoordGenerator(int du, int nu): NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN), du_(du), nu_(nu) {}

        META_NodeVisitor("osgFX","TexCoordGenerator")

        void apply(osg::Geode& geode)
        {
            const osg::BoundingSphere &bsphere = geode.getBound();
            float scale = 10;
            if (bsphere.radius() != 0) {
                scale = 5 / bsphere.radius();
            }
            for (unsigned i=0; i<geode.getNumDrawables(); ++i) {
                osg::Geometry* geo = dynamic_cast<osg::Geometry* >(geode.getDrawable(i));
                if (geo) {
                    osg::ref_ptr<osg::Vec2Array> tc = generate_coords(geo->getVertexArray(), geo->getNormalArray(), scale);
                    geo->setTexCoordArray(du_, tc.get());
                    geo->setTexCoordArray(nu_, tc.get());
                }
            }
            NodeVisitor::apply(geode);
        }

    protected:
        osg::Vec2Array* generate_coords(osg::Array* vx, osg::Array* nx, float scale)
        {
            osg::Vec2Array* v2a = dynamic_cast<osg::Vec2Array*>(vx);
            osg::Vec3Array* v3a = dynamic_cast<osg::Vec3Array*>(vx);
            osg::Vec4Array* v4a = dynamic_cast<osg::Vec4Array*>(vx);
            osg::Vec2Array* n2a = dynamic_cast<osg::Vec2Array*>(nx);
            osg::Vec3Array* n3a = dynamic_cast<osg::Vec3Array*>(nx);
            osg::Vec4Array* n4a = dynamic_cast<osg::Vec4Array*>(nx);

            osg::ref_ptr<osg::Vec2Array> tc = new osg::Vec2Array;
            for (unsigned i=0; i<vx->getNumElements(); ++i) {

                osg::Vec3 P;
                if (v2a) P.set((*v2a)[i].x(), (*v2a)[i].y(), 0);
                if (v3a) P.set((*v3a)[i].x(), (*v3a)[i].y(), (*v3a)[i].z());
                if (v4a) P.set((*v4a)[i].x(), (*v4a)[i].y(), (*v4a)[i].z());

                osg::Vec3 N(0, 0, 1);
                if (n2a) N.set((*n2a)[i].x(), (*n2a)[i].y(), 0);
                if (n3a) N.set((*n3a)[i].x(), (*n3a)[i].y(), (*n3a)[i].z());
                if (n4a) N.set((*n4a)[i].x(), (*n4a)[i].y(), (*n4a)[i].z());

                int axis = 0;
                if (N.y() > N.x() && N.y() > N.z()) axis = 1;
                if (-N.y() > N.x() && -N.y() > N.z()) axis = 1;
                if (N.z() > N.x() && N.z() > N.y()) axis = 2;
                if (-N.z() > N.x() && -N.z() > N.y()) axis = 2;

                osg::Vec2 uv;

                switch (axis) {
                    case 0: uv.set(P.y(), P.z()); break;
                    case 1: uv.set(P.x(), P.z()); break;
                    case 2: uv.set(P.x(), P.y()); break;
                    default: ;
                }

                tc->push_back(uv * scale);
            }
            return tc.release();
        }

    private:
        int du_;
        int nu_;
    };

}


namespace
{
    // let's register this cool effect! :)
    Registry::Proxy proxy(new BumpMapping);
}


BumpMapping::BumpMapping()
:   Effect(),
    _lightnum(0),
    _diffuse_unit(1),
    _normal_unit(0)
{
}

BumpMapping::BumpMapping(const BumpMapping& copy, const osg::CopyOp& copyop)
:   Effect(copy, copyop),
    _lightnum(copy._lightnum),
    _diffuse_unit(copy._diffuse_unit),
    _normal_unit(copy._normal_unit),
    _diffuse_tex(static_cast<osg::Texture2D* >(copyop(copy._diffuse_tex.get()))),
    _normal_tex(static_cast<osg::Texture2D* >(copyop(copy._normal_tex.get())))
{
}

bool BumpMapping::define_techniques()
{
    // ARB assembly program techniques have been removed.
    // TODO: implement GLSL-based bump mapping technique.
    return true;
}

void BumpMapping::prepareGeometry(osg::Geometry* geo)
{
    osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator;
    tsg->generate(geo, _normal_unit);
    if (!geo->getVertexAttribArray(6))
        geo->setVertexAttribArray(6, tsg->getTangentArray());
    if (!geo->getVertexAttribArray(7))
        geo->setVertexAttribArray(7, tsg->getBinormalArray());
    if (!geo->getVertexAttribArray(15))
        geo->setVertexAttribArray(15, tsg->getNormalArray());
}

void BumpMapping::prepareNode(osg::Node* node)
{
    osg::ref_ptr<TsgVisitor> tv = new TsgVisitor(this);
    node->accept(*tv.get());
}

void BumpMapping::prepareChildren()
{
    for (unsigned i=0; i<getNumChildren(); ++i)
        prepareNode(getChild(i));
}

void BumpMapping::setUpDemo()
{
    // generate texture coordinates
    TexCoordGenerator tcg(_diffuse_unit, _normal_unit);
    for (unsigned i=0; i<getNumChildren(); ++i)
        getChild(i)->accept(tcg);

    // set up diffuse texture
    if (!_diffuse_tex.valid()) {
        _diffuse_tex = new osg::Texture2D;
        _diffuse_tex->setImage(osgDB::readRefImageFile("Images/whitemetal_diffuse.jpg"));
        _diffuse_tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        _diffuse_tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        _diffuse_tex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
        _diffuse_tex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
        _diffuse_tex->setMaxAnisotropy(8);
    }

    // set up normal map texture
    if (!_normal_tex.valid()) {
        _normal_tex = new osg::Texture2D;
        _normal_tex->setImage(osgDB::readRefImageFile("Images/whitemetal_normal.jpg"));
        _normal_tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        _normal_tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        _normal_tex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
        _normal_tex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
        _normal_tex->setMaxAnisotropy(8);
    }

    // generate tangent-space basis vector
    prepareChildren();

    // recreate techniques on next step
    dirtyTechniques();
}
