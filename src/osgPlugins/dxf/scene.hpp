/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * bounds class.
 * Provides: expandBy, makeMinValid, createPtGeometry, createLnGeometry,
 * createTriGeometry, createQuadGeometry.
 */
/** Simulate the scene with double precision before passing it back to osg.
    this permits us to scale down offsets from 0,0,0 with a few matrixtransforms,
    in case the objects are too far from that center.
    */

#pragma once

#include <osg/geometry/Geometry.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/state/Hint.hpp>
#include <osg/state/LineWidth.hpp>
#include <osgText/Text>
#include <osgUtil/optimization/SmoothingVisitor.hpp>

class dxfLayerTable;

class bounds
{
    public:

        bounds() :
            _min( DBL_MAX,
                  DBL_MAX,
                  DBL_MAX ),
            _max( -DBL_MAX,
                  -DBL_MAX,
                  -DBL_MAX )
        {
        }

        inline void
        expandBy( const osg::dvec3& v )
        {
            if( v.x < _min.x )
            {
                _min.x = v.x;
            }
            if( v.x > _max.x )
            {
                _max.x = v.x;
            }

            if( v.y < _min.y )
            {
                _min.y = v.y;
            }
            if( v.y > _max.y )
            {
                _max.y = v.y;
            }

            if( v.z < _min.z )
            {
                _min.z = v.z;
            }
            if( v.z > _max.z )
            {
                _max.z = v.z;
            }
        }

        inline void
        makeMinValid()
        {
            // we count on _min to offset the whole scene
            // so, we make sure its at 0,0,0 if
            // bounds are not set (anyway, the scene should be empty,
            // if we need to set any value of _min to 0).
            if( _min.x == DBL_MAX )
            {
                _min.x = 0;
            }
            if( _min.y == DBL_MAX )
            {
                _min.y = 0;
            }
            if( _min.z == DBL_MAX )
            {
                _min.z = 0;
            }
        }

        osg::dvec3 _min;
        osg::dvec3 _max;
};

static inline osg::Geometry*
createPtGeometry( osg::PrimitiveSet::Mode pointType,
                  osg::Vec3Array*         vertices,
                  const osg::vec4&        color )
{
    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( vertices );
    geom->addPrimitiveSet( new osg::DrawArrays( pointType, 0, vertices->size() ) );
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( color );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );
    osg::Vec3Array* norms = new osg::Vec3Array;
    norms->push_back( osg::vec3( 0, 0, 1 ) );
    geom->setNormalArray( norms, osg::Array::BIND_OVERALL );
    return geom;
}

static inline osg::Geometry*
createLnGeometry( osg::PrimitiveSet::Mode lineType,
                  osg::Vec3Array*         vertices,
                  const osg::vec4&        color )
{
    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( vertices );
    geom->addPrimitiveSet( new osg::DrawArrays( lineType, 0, vertices->size() ) );
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( color );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );
    osg::Vec3Array* norms = new osg::Vec3Array;
    norms->push_back( osg::vec3( 0, 0, 1 ) );
    geom->setNormalArray( norms, osg::Array::BIND_OVERALL );
    return geom;
}

static inline osg::Geometry*
createTriGeometry( osg::Vec3Array*  vertices,
                   osg::Vec3Array*  normals,
                   const osg::vec4& color )
{
    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( vertices );
    geom->addPrimitiveSet(
        new osg::DrawArrays( osg::PrimitiveSet::TRIANGLES, 0, vertices->size() )
    );
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( color );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );
    geom->setNormalArray( normals, osg::Array::BIND_PER_VERTEX );
    return geom;
}

static inline osg::Geometry*
createQuadGeometry( osg::Vec3Array*  vertices,
                    osg::Vec3Array*  normals,
                    const osg::vec4& color )
{
    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( vertices );
    geom->addPrimitiveSet(
        new osg::DrawArrays( osg::PrimitiveSet::TRIANGLE_FAN, 0, vertices->size() )
    );
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( color );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );
    geom->setNormalArray( normals, osg::Array::BIND_PER_VERTEX );
    return geom;
}

static inline osg::Geode*
createModel( const std::string& name,
             osg::Drawable*     drawable )
{
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( drawable );
    geode->setName( name );
    return geode;
}

static inline osg::dvec3
preMultd( const osg::dmat4& m,
          const osg::dvec3& v )
{
    double d =
        1.0F / ( m( 3, 0 ) * v.x + m( 3, 1 ) * v.y + m( 3, 2 ) * v.z + m( 3, 3 ) );
    return osg::dvec3(
        ( m( 0, 0 ) * v.x + m( 1, 0 ) * v.y + m( 2, 0 ) * v.z + m( 3, 0 ) ) * d,
        ( m( 0, 1 ) * v.x + m( 1, 1 ) * v.y + m( 2, 1 ) * v.z + m( 3, 1 ) ) * d,
        ( m( 0, 2 ) * v.x + m( 1, 2 ) * v.y + m( 2, 2 ) * v.z + m( 3, 2 ) ) * d
    );
}

static inline osg::dvec3
postMultd( const osg::dmat4& m,
           const osg::dvec3& v )
{
    double d =
        1.0F / ( m( 3, 0 ) * v.x + m( 3, 1 ) * v.y + m( 3, 2 ) * v.z + m( 3, 3 ) );
    return osg::dvec3(
        ( m( 0, 0 ) * v.x + m( 0, 1 ) * v.y + m( 0, 2 ) * v.z + m( 0, 3 ) ) * d,
        ( m( 1, 0 ) * v.x + m( 1, 1 ) * v.y + m( 1, 2 ) * v.z + m( 1, 3 ) ) * d,
        ( m( 2, 0 ) * v.x + m( 2, 1 ) * v.y + m( 2, 2 ) * v.z + m( 2, 3 ) ) * d
    );
}

typedef std::vector<osg::dvec3>             VList;
typedef std::map<unsigned short, VList>     MapVList;
typedef std::vector<VList>                  VListList;
typedef std::map<unsigned short, VListList> MapVListList;
typedef std::map<double, MapVList>          MapMapVList;
typedef std::map<double, MapVListList>      MapMapVListList;

class sceneLayer : public osg::Referenced
{
    public:

        sceneLayer( std::string name ) :
            _name( name )
        {
        }

        virtual ~sceneLayer()
        {
        }

        void
        layer2osg( osg::Group* root,
                   bounds&     b )
        {
            osgPoints( root, b );
            osgLines( root, b );
            osgTriangles( root, b );
            osgQuads( root, b );
            osgText( root, b );
        }

        MapMapVListList _maplinestrips;
        MapMapVList     _maplines;
        MapVList        _points;
        MapVList        _triangles;
        MapVList        _trinorms;
        MapVList        _quads;
        MapVList        _quadnorms;

        struct textInfo
        {
                textInfo( short int      color,
                          osg::dvec3     point,
                          osgText::Text* text ) :
                    _color( color ),
                    _point( point ),
                    _text( text ) {};
                short int                   _color;
                osg::dvec3                  _point;
                osg::ref_ptr<osgText::Text> _text;
        };

        typedef std::vector<textInfo> TextList;
        TextList                      _textList;

    protected:

        std::string _name;

        osg::vec4
        getColor( unsigned short color );

        void
        osgPoints( osg::Group* root,
                   bounds&     b )
        {

            for( MapVList::iterator mitr = _points.begin(); mitr != _points.end();
                 ++mitr )
            {
                osg::Vec3Array* coords = new osg::Vec3Array;
                for( VList::iterator itr = mitr->second.begin();
                     itr != mitr->second.end();
                     ++itr )
                {
                    osg::vec3 v( itr->x - b._min.x,
                                 itr->y - b._min.y,
                                 itr->z - b._min.z );
                    coords->push_back( v );
                }
                root->addChild(
                    createModel( _name,
                                 createPtGeometry( osg::PrimitiveSet::POINTS,
                                                   coords,
                                                   getColor( mitr->first ) ) )
                );
            }
        }

        void
        osgLines( osg::Group* root,
                  bounds&     b )
        {
            std::map<double, osg::ref_ptr<osg::StateSet>> mapStateSet;
            for( MapMapVListList::iterator mmlitr = _maplinestrips.begin();
                 mmlitr != _maplinestrips.end();
                 ++mmlitr )
            {
                osg::ref_ptr<osg::StateSet> spStateSet = NULL;
                short                       lineWidth  = mmlitr->first;
                if( lineWidth > 0 )
                {
                    spStateSet = new osg::StateSet;
                    spStateSet->setAttributeAndModes(
                        new osg::LineWidth( lineWidth * 96.0F / 254.0F ),
                        osg::StateAttribute::ON
                    );
                    mapStateSet[lineWidth] = spStateSet;
                }
                for( MapVListList::iterator mlitr = mmlitr->second.begin();
                     mlitr != mmlitr->second.end();
                     ++mlitr )
                {
                    for( VListList::iterator itr = mlitr->second.begin();
                         itr != mlitr->second.end();
                         ++itr )
                    {
                        if( itr->size() )
                        {
                            osg::Vec3Array* coords = new osg::Vec3Array;
                            for( VList::iterator vitr = itr->begin(); vitr != itr->end();
                                 ++vitr )
                            {
                                osg::vec3 v( vitr->x - b._min.x,
                                             vitr->y - b._min.y,
                                             vitr->z - b._min.z );
                                coords->push_back( v );
                            }

                            osg::Geode* geode = createModel(
                                _name,
                                createLnGeometry( osg::PrimitiveSet::LINE_STRIP,
                                                  coords,
                                                  getColor( mlitr->first ) )
                            );
                            if( spStateSet.valid() )
                            {
                                geode->setStateSet( spStateSet.get() );
                            }
                            root->addChild( geode );
                        }
                    }
                }
            }

            for( MapMapVList::iterator mmitr = _maplines.begin();
                 mmitr != _maplines.end();
                 ++mmitr )
            {
                osg::ref_ptr<osg::StateSet> spStateSet = NULL;
                double                      lineWidth  = mmitr->first;
                if( lineWidth > 0 )
                {
                    std::map<double, osg::ref_ptr<osg::StateSet>>::iterator it =
                        mapStateSet.find( lineWidth );
                    if( it != mapStateSet.end() )
                    {
                        spStateSet = it->second;
                    }
                    else
                    {
                        spStateSet = new osg::StateSet;
                        spStateSet->setAttributeAndModes(
                            new osg::LineWidth( lineWidth * 96.0F / 254.0F ),
                            osg::StateAttribute::ON
                        );
                    }
                }
                for( MapVList::iterator mitr = mmitr->second.begin();
                     mitr != mmitr->second.end();
                     ++mitr )
                {
                    osg::Vec3Array* coords = new osg::Vec3Array;
                    for( VList::iterator itr = mitr->second.begin();
                         itr != mitr->second.end();
                         ++itr )
                    {
                        osg::vec3 v( itr->x - b._min.x,
                                     itr->y - b._min.y,
                                     itr->z - b._min.z );
                        coords->push_back( v );
                    }

                    osg::Geode* geode =
                        createModel( _name,
                                     createLnGeometry( osg::PrimitiveSet::LINES,
                                                       coords,
                                                       getColor( mitr->first ) ) );
                    if( spStateSet.valid() )
                    {
                        geode->setStateSet( spStateSet.get() );
                    }
                    root->addChild( geode );
                }
            }
        }

        void
        osgTriangles( osg::Group* root,
                      bounds&     b )
        {
            if( _triangles.size() )
            {
                for( MapVList::iterator mitr = _triangles.begin();
                     mitr != _triangles.end();
                     ++mitr )
                {
                    osg::Vec3Array* coords = new osg::Vec3Array;
                    VList::iterator itr;
                    for( itr = mitr->second.begin(); itr != mitr->second.end(); ++itr )
                    {
                        osg::vec3 v( itr->x - b._min.x,
                                     itr->y - b._min.y,
                                     itr->z - b._min.z );
                        coords->push_back( v );
                    }
                    osg::Vec3Array* norms    = new osg::Vec3Array;
                    VList&          normlist = _trinorms[mitr->first];
                    for( itr = normlist.begin(); itr != normlist.end(); ++itr )
                    {
                        osg::vec3 norm( itr->x, itr->y, itr->z );
                        for( int i = 0; i < 3; ++i )
                        {
                            norms->push_back( norm );
                        }
                    }
                    root->addChild( createModel(
                        _name,
                        createTriGeometry( coords, norms, getColor( mitr->first ) )
                    ) );
                }
            }
        }

        void
        osgQuads( osg::Group* root,
                  bounds&     b )
        {
            if( _quads.size() )
            {
                for( MapVList::iterator mitr = _quads.begin(); mitr != _quads.end();
                     ++mitr )
                {
                    osg::Vec3Array* coords = new osg::Vec3Array;
                    VList::iterator itr;
                    for( itr = mitr->second.begin(); itr != mitr->second.end(); ++itr )
                    {
                        osg::vec3 v( itr->x - b._min.x,
                                     itr->y - b._min.y,
                                     itr->z - b._min.z );
                        coords->push_back( v );
                    }
                    osg::Vec3Array* norms    = new osg::Vec3Array;
                    VList&          normlist = _quadnorms[mitr->first];
                    for( itr = normlist.begin(); itr != normlist.end(); ++itr )
                    {
                        osg::vec3 norm( itr->x, itr->y, itr->z );
                        for( int i = 0; i < 4; ++i )
                        {
                            norms->push_back( norm );
                        }
                    }
                    root->addChild( createModel(
                        _name,
                        createQuadGeometry( coords, norms, getColor( mitr->first ) )
                    ) );
                }
            }
        }

        void
        osgText( osg::Group* root,
                 bounds&     b )
        {
            if( _textList.size() )
            {
                for( TextList::iterator titr = _textList.begin();
                     titr != _textList.end();
                     ++titr )
                {
                    titr->_text->setColor( getColor( titr->_color ) );
                    osg::dvec3 v1 = titr->_point;
                    osg::vec3  v2( v1.x - b._min.x, v1.y - b._min.y, v1.z - b._min.z );
                    titr->_text->setPosition( v2 );
                    root->addChild( createModel( _name, titr->_text.get() ) );
                }
            }
        }
};

class scene : public osg::Referenced
{
    public:

        scene( dxfLayerTable* lt = NULL );

        virtual ~scene()
        {
        }

        void
        setLayerTable( dxfLayerTable* lt );

        void
        pushMatrix( const osg::dmat4& m,
                    bool              protect = false )
        {
            _mStack.push_back( _m );
            if( protect )    // equivalent to setMatrix
            {
                _m = m;
            }
            else
            {
                _m = _m * m;
            }
        }

        void
        popMatrix()
        {
            _mStack.pop_back();
            if( _mStack.size() )
            {
                _m = _mStack.back();
            }
            else
            {
                _m = osg::dmat4();
            }
        }

        void
        ocs( const osg::dmat4& r )
        {
            _r = r;
        }

        void
        blockOffset( const osg::dvec3& t )
        {
            _t = t;
        }

        void
        ocs_clear()
        {
            _r = osg::dmat4();
        }

        osg::dmat4&
        backMatrix()
        {
            if( _mStack.size() )
            {
                return _mStack.back();
            }
            else
            {
                return _m;
            }
        }

        osg::dvec3
        addVertex( osg::dvec3 v );
        osg::dvec3
        addNormal( osg::dvec3 v );

        sceneLayer*
        findOrCreateSceneLayer( const std::string& l )
        {
            sceneLayer* ly = _layers[l].get();
            if( !ly )
            {
                ly         = new sceneLayer( l );
                _layers[l] = ly;
            }
            return ly;
        }

        unsigned short
        correctedColorIndex( const std::string& l,
                             unsigned short     color );
        double
        correctedLineWidth( const std::string& l,
                            double             defaultLineWidth = -1.0 );

        void
        addPoint( const std::string& l,
                  unsigned short     color,
                  osg::dvec3&        s );
        void
        addLine( const std::string& l,
                 unsigned short     color,
                 osg::dvec3&        s,
                 osg::dvec3&        e,
                 double             lineWidth );
        void
        addLineStrip( const std::string&       l,
                      unsigned short           color,
                      std::vector<osg::dvec3>& vertices,
                      double                   lineWidth );
        void
        addLineLoop( const std::string&       l,
                     unsigned short           color,
                     std::vector<osg::dvec3>& vertices,
                     double                   lineWidth );
        void
        addTriangles( const std::string&       l,
                      unsigned short           color,
                      std::vector<osg::dvec3>& vertices,
                      bool                     inverted = false );
        void
        addQuads( const std::string&       l,
                  unsigned short           color,
                  std::vector<osg::dvec3>& vertices,
                  bool                     inverted = false );
        void
        addText( const std::string& l,
                 unsigned short     color,
                 osg::dvec3&        point,
                 osgText::Text*     text );

        osg::Group*
        scene2osg()
        {
            osg::Group* root  = NULL;
            osg::Group* child = NULL;
            _b.makeMinValid();
            osg::vec3  v = osg::vec3( _b._min.x, _b._min.y, _b._min.z );
            double     x = _b._min.x - ( double )v.x;
            double     y = _b._min.y - ( double )v.y;
            double     z = _b._min.z - ( double )v.z;
            osg::dmat4 m = osg::translate( osg::dvec3( v ) );
            root         = new osg::MatrixTransform( m );
            if( x || y || z )
            {
                m     = osg::translate( osg::dvec3( x, y, z ) );
                child = new osg::MatrixTransform( m );
                root->addChild( child );
            }
            else
            {
                child = root;
            }
            // root = mt;

            child->setName( "Layers" );

            std::map<double, osg::ref_ptr<osg::StateSet>> mapStateSet;
            for( std::map<std::string, osg::ref_ptr<sceneLayer>>::iterator litr =
                     _layers.begin();
                 litr != _layers.end();
                 ++litr )
            {
                sceneLayer* ly = ( *litr ).second.get();
                if( !ly )
                {
                    continue;
                }

                osg::ref_ptr<osg::StateSet> spStateSet = NULL;
                double lineWidth = correctedLineWidth( ( *litr ).first, -1 );
                if( lineWidth > 0 )
                {
                    std::map<double, osg::ref_ptr<osg::StateSet>>::iterator it =
                        mapStateSet.find( lineWidth );
                    if( it != mapStateSet.end() )
                    {
                        spStateSet = it->second;
                    }
                    else
                    {
                        spStateSet = new osg::StateSet;
                        spStateSet->setAttributeAndModes(
                            new osg::LineWidth( lineWidth * 96.0F / 254.0F ),
                            osg::StateAttribute::ON
                        );
                    }
                }
                osg::Group* lg = new osg::Group;
                lg->setName( ( *litr ).first );
                child->addChild( lg );
                ly->layer2osg( lg, _b );
                if( spStateSet.valid() )
                {
                    lg->setStateSet( spStateSet.get() );
                }
            }
            return root;
        }

    protected:

        osg::dmat4                                      _m;
        osg::dmat4                                      _r;
        osg::dvec3                                      _t;
        bounds                                          _b;
        std::map<std::string, osg::ref_ptr<sceneLayer>> _layers;
        std::vector<osg::dmat4>                         _mStack;
        dxfLayerTable*                                  _layerTable;
};
