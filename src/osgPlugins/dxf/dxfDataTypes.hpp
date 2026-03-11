/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * dxfDataType class.
 * Provides: between, between, between, between, between, between.
 */
#pragma once

#include <osg/nodes/Group.hpp>

typedef std::map<std::string, osg::ref_ptr<osg::Group>> LayerTable;

class dxfDataType
{
    public:

        enum TYPE
        {
            UNKNOWN,
            STRING,
            HEX,
            BOOL,
            SHORT,
            INT,
            LONG,
            DOUBLE,
        };

        inline static bool
        between( int a,
                 int m,
                 int x )
        {
            return ( a >= m && a <= x );
        }

        inline static int
        typeForCode( int gc )
        {
            if( between( gc, 0, 9 ) ||
                gc ==
                100 ||
                gc ==
                102 ||
                between( gc, 300, 309 ) ||
                between( gc, 410, 419 ) ||
                between( gc, 430, 439 ) ||
                between( gc, 470, 479 ) ||
                gc ==
                999 ||
                between( gc, 1'000, 1'009 ) )
            {
                return STRING;
            }
            else if( gc ==
                     105 ||
                     between( gc, 310, 319 ) ||
                     between( gc, 320, 329 ) ||
                     between( gc, 330, 369 ) ||
                     between( gc, 390, 399 ) )
            {
                return HEX;
            }
            else if( between( gc, 290, 299 ) )
            {
                return BOOL;
            }
            else if( between(
                         gc,
                         70,
                         78
                     ) )    // 2005.12.13 PdR 70 to 78 should be INT, not U_SHORT
            {
                return INT;
            }
            else if( between( gc, 60, 79 ) ||
                     between( gc, 170, 179 ) ||
                     between( gc, 270, 279 ) ||
                     between( gc, 280, 289 ) ||
                     between( gc, 370, 379 ) ||
                     between( gc, 380, 389 ) ||
                     between( gc, 400, 409 ) )
            {
                return SHORT;
            }
            else if( between( gc, 90, 99 ) ||
                     between( gc, 450, 459 ) ||
                     between( gc, 1'060, 1'070 ) )
            {
                return LONG;
            }
            else if( between( gc, 420, 429 ) || between( gc, 440, 449 ) || gc == 1'071 )
            {
                return INT;
            }
            else if( between( gc, 10, 39 ) ||
                     between( gc, 40, 59 ) ||
                     between( gc, 110, 119 ) ||
                     between( gc, 120, 129 ) ||
                     between( gc, 130, 139 ) ||
                     between( gc, 140, 149 ) ||
                     between( gc, 210, 239 ) ||
                     between( gc, 460, 469 ) ||
                     between( gc, 1'010, 1'019 ) )
            {
                return DOUBLE;
            }
            else
            {
                return UNKNOWN;
            }
        }
};
