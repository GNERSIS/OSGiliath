/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * dxfTable — osgPlugins library implementation.
 */
#include "codeValue.hpp"
#include "dxfFile.hpp"
#include "dxfTable.hpp"

void
dxfLayer::assign( dxfFile*,
                  codeValue& cv )
{
    switch( cv._groupCode )
    {
        case 2 :
            _name = cv._string;
            break;
        case 62 :
            _color = cv._short;
            if( ( short )_color < 0 )
            {
                _frozen = true;
            }
            break;
        // Thickness
        case 370 :
        case 39 :
            if( cv._double > 0 )
            {
                _lineThickness = cv._double;
                if( _lineWidth <= 0 )
                {
                    _lineWidth = _lineThickness;
                }
            }
            break;
        // width
        case 43 :
            if( cv._double > 0 )
            {
                _lineWidth = cv._double;
            }
            break;
        case 70 :
            _frozen = ( bool )( cv._short & 1 );
            break;
    }
}

void
dxfLayerTable::assign( dxfFile*   dxf,
                       codeValue& cv )
{
    std::string s = cv._string;
    if( cv._groupCode == 0 )
    {
        if( _currentLayer.get() )
        {
            _layers[_currentLayer->getName()] = _currentLayer.get();
        }
        if( s == "LAYER" )
        {
            _currentLayer = new dxfLayer;
        }    // otherwise it's the close call from ENDTAB
    }
    else if( _currentLayer.get() )
    {
        _currentLayer->assign( dxf, cv );
    }
}
