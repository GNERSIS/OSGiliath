/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: dxfEntity.
 */
#include "codeValue.hpp"
#include "dxfBlock.hpp"
#include "dxfEntity.hpp"
#include "dxfFile.hpp"

using namespace std;

void
dxfBlock::assign( dxfFile*   dxf,
                  codeValue& cv )
{
    string s = cv._string;
    if( cv._groupCode == 0 )
    {
        if( _currentEntity && _currentEntity->done() )
        {
            _currentEntity = new dxfEntity( s );
            _entityList.push_back( _currentEntity );
        }
        else if( _currentEntity )
        {
            _currentEntity->assign( dxf, cv );
        }
        else
        {
            _currentEntity = new dxfEntity( s );
            _entityList.push_back( _currentEntity );
        }
    }
    else if( _currentEntity )
    {
        _currentEntity->assign( dxf, cv );
    }
    else if( cv._groupCode != 0 )
    {
        switch( cv._groupCode )
        {
            case 2 :
                _name = s;
                break;
            case 10 :
                _position.x = cv._double;
                break;
            case 20 :
                _position.y = cv._double;
                break;
            case 30 :
                _position.z = cv._double;
                break;
            default :
                // dxf garble
                break;
        }
    }
}

const osg::dvec3&
dxfBlock::getPosition() const
{
    return _position;
}
