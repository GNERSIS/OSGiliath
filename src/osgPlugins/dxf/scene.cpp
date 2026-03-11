/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: c, _layerTable, a, pscene, ti.
 */
#include "aci.hpp"
#include "dxfTable.hpp"
#include "scene.hpp"

#include <osg/maths/compat.hpp>
#include <osg/maths/Matrix.hpp>

using namespace osg;
using namespace std;

osg::vec4
sceneLayer::getColor( unsigned short color )
{
    // you're supposed to have a correct color in hand
    unsigned short r = color * 3;
    unsigned short g = color * 3 + 1;
    unsigned short b = color * 3 + 2;
    vec4           c( aci::table[r], aci::table[g], aci::table[b], 1.0F );
    return c;
}

scene::scene( dxfLayerTable* lt ) :
    _layerTable( lt )
{
    _m = osg::dmat4();
    _r = osg::dmat4();
}

void
scene::setLayerTable( dxfLayerTable* lt )
{
    _layerTable = lt;
}

dvec3
scene::addVertex( dvec3 v )
{
    v            += _t;
    v             = preMultd( _r, v );
    osg::dmat4 m  = osg::translate( v.x, v.y, v.z );
    m             = m * _m;
    dvec3 a       = preMultd( m, dvec3( 0, 0, 0 ) );
    _b.expandBy( a );
    return a;
}

dvec3
scene::addNormal( dvec3 v )
{
    // to do: vertices are not always listed in order. find why.
    return v;
}

void
scene::addPoint( const std::string& l,
                 unsigned short     color,
                 dvec3&             s )
{
    dxfLayer* layer = _layerTable->findOrCreateLayer( l );
    if( layer->getFrozen() )
    {
        return;
    }
    sceneLayer* ly = findOrCreateSceneLayer( l );
    dvec3       a( addVertex( s ) );
    ly->_points[correctedColorIndex( l, color )].push_back( a );
}

void
scene::addLine( const std::string& l,
                unsigned short     color,
                dvec3&             s,
                dvec3&             e,
                double             lineWidth )
{
    dxfLayer* layer = _layerTable->findOrCreateLayer( l );
    if( layer->getFrozen() )
    {
        return;
    }
    sceneLayer* ly = findOrCreateSceneLayer( l );
    dvec3       a( addVertex( s ) ), b( addVertex( e ) );
    if( lineWidth <= 0 )
    {
        lineWidth = 0;
    }
    ly->_maplines[lineWidth][correctedColorIndex( l, color )].push_back( a );
    ly->_maplines[lineWidth][correctedColorIndex( l, color )].push_back( b );
}

void
scene::addLineStrip( const std::string&  l,
                     unsigned short      color,
                     std::vector<dvec3>& vertices,
                     double              lineWidth )
{
    dxfLayer* layer = _layerTable->findOrCreateLayer( l );
    if( layer->getFrozen() )
    {
        return;
    }
    sceneLayer*        ly = findOrCreateSceneLayer( l );
    std::vector<dvec3> converted;
    for( std::vector<dvec3>::iterator itr = vertices.begin(); itr != vertices.end();
         ++itr )
    {
        converted.push_back( addVertex( *itr ) );
    }
    if( lineWidth <= 0 )
    {
        lineWidth = 0;
    }
    ly->_maplinestrips[lineWidth][correctedColorIndex( l, color )].push_back(
        converted
    );
}

void
scene::addLineLoop( const std::string&  l,
                    unsigned short      color,
                    std::vector<dvec3>& vertices,
                    double              lineWidth )
{
    dxfLayer* layer = _layerTable->findOrCreateLayer( l );
    if( layer->getFrozen() )
    {
        return;
    }
    sceneLayer*        ly = findOrCreateSceneLayer( l );
    std::vector<dvec3> converted;
    for( std::vector<dvec3>::iterator itr = vertices.begin(); itr != vertices.end();
         ++itr )
    {
        converted.push_back( addVertex( *itr ) );
    }
    converted.push_back( addVertex( vertices.front() ) );
    if( lineWidth <= 0 )
    {
        lineWidth = 0;
    }
    ly->_maplinestrips[lineWidth][correctedColorIndex( l, color )].push_back(
        converted
    );
}

void
scene::addTriangles( const std::string&  l,
                     unsigned short      color,
                     std::vector<dvec3>& vertices,
                     bool                inverted )
{
    dxfLayer* layer = _layerTable->findOrCreateLayer( l );
    if( layer->getFrozen() )
    {
        return;
    }
    sceneLayer* ly = findOrCreateSceneLayer( l );
    for( VList::iterator itr = vertices.begin(); itr != vertices.end(); )
    {
        VList::iterator a;
        VList::iterator b;
        VList::iterator c;
        if( inverted )
        {
            c = itr++;
            b = itr++;
            a = itr++;
        }
        else
        {
            a = itr++;
            b = itr++;
            c = itr++;
        }
        if( a != vertices.end() && b != vertices.end() && c != vertices.end() )
        {
            dvec3 n = ( ( *b - *a ) ^ ( *c - *a ) );
            n       = osg::normalize( n );
            ly->_trinorms[correctedColorIndex( l, color )].push_back( n );
            ly->_triangles[correctedColorIndex( l, color )].push_back( addVertex( *a ) );
            ly->_triangles[correctedColorIndex( l, color )].push_back( addVertex( *b ) );
            ly->_triangles[correctedColorIndex( l, color )].push_back( addVertex( *c ) );
        }
    }
}

void
scene::addQuads( const std::string&  l,
                 unsigned short      color,
                 std::vector<dvec3>& vertices,
                 bool                inverted )
{
    dxfLayer* layer = _layerTable->findOrCreateLayer( l );
    if( layer->getFrozen() )
    {
        return;
    }

    sceneLayer* ly = findOrCreateSceneLayer( l );
    for( VList::iterator itr = vertices.begin(); itr != vertices.end(); )
    {
        VList::iterator a = vertices.end();
        VList::iterator b = vertices.end();
        VList::iterator c = vertices.end();
        VList::iterator d = vertices.end();
        if( inverted )
        {
            d = itr++;
            if( itr != vertices.end() )
            {
                c = itr++;
            }
            if( itr != vertices.end() )
            {
                b = itr++;
            }
            if( itr != vertices.end() )
            {
                a = itr++;
            }
        }
        else
        {
            a = itr++;
            if( itr != vertices.end() )
            {
                b = itr++;
            }
            if( itr != vertices.end() )
            {
                c = itr++;
            }
            if( itr != vertices.end() )
            {
                d = itr++;
            }
        }
        if( a !=
            vertices.end() &&
            b !=
            vertices.end() &&
            c !=
            vertices.end() &&
            d != vertices.end() )
        {
            dvec3 n      = ( ( *b - *a ) ^ ( *c - *a ) );
            n            = osg::normalize( n );
            short cindex = correctedColorIndex( l, color );
            ly->_quadnorms[cindex].push_back( n );
            VList& vl = ly->_quads[cindex];
            vl.push_back( addVertex( *a ) );
            vl.push_back( addVertex( *b ) );
            vl.push_back( addVertex( *c ) );
            vl.push_back( addVertex( *d ) );
        }
    }
}

void
scene::addText( const std::string& l,
                unsigned short     color,
                dvec3&             point,
                osgText::Text*     text )
{
    dxfLayer* layer = _layerTable->findOrCreateLayer( l );
    if( layer->getFrozen() )
    {
        return;
    }
    sceneLayer* ly = findOrCreateSceneLayer( l );

    // Apply the scene settings to the text size and rotation

    dvec3       pscene( addVertex( point ) );
    dvec3       xvec =
        addVertex( point + ( text->getRotation() * osg::dvec3( 1.0, 0.0, 0.0 ) ) ) -
        pscene;
    dvec3 yvec =
        addVertex( point + ( text->getRotation() * osg::dvec3( 0.0, 1.0, 0.0 ) ) ) -
        pscene;
    text->setCharacterSize(
        text->getCharacterHeight() * osg::length( yvec ),
        text->getCharacterAspectRatio() * osg::length( yvec ) / osg::length( xvec )
    );

    dmat4 qm = _r * _m;
    dvec3 t, s;
    dquat ro, so;
    osg::decompose( qm, t, ro, s, so );
    text->setRotation( text->getRotation() * quat( static_cast<float>( ro.x ),
                                                   static_cast<float>( ro.y ),
                                                   static_cast<float>( ro.z ),
                                                   static_cast<float>( ro.w ) ) );

    sceneLayer::textInfo ti( correctedColorIndex( l, color ), pscene, text );
    ly->_textList.push_back( ti );
}

unsigned short
scene::correctedColorIndex( const std::string& l,
                            unsigned short     color )
{
    if( color >= aci::MIN && color <= aci::MAX )
    {
        return color;
    }
    else if( !color || color == aci::BYLAYER )
    {
        dxfLayer*      layer  = _layerTable->findOrCreateLayer( l );
        unsigned short lcolor = layer->getColor();
        if( lcolor >= aci::MIN && lcolor <= aci::MAX )
        {
            return lcolor;
        }
    }
    return aci::WHITE;
}

double
scene::correctedLineWidth( const std::string& l,
                           double             defaultLineWidth )
{
    if( defaultLineWidth <= 0 )
    {
        dxfLayer* layer = _layerTable->findOrCreateLayer( l );
        return layer->getLineWidth();
    }

    return defaultLineWidth;
}
