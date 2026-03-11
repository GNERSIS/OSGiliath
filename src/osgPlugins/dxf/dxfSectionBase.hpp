/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * dxfSectionBase, derived from Referenced.
 * Provides: assign.
 */
#pragma once

#include <osg/core/Referenced.hpp>

class dxfFile;
class codeValue;

/// abstract base class for sections. see dxfSection.h
class dxfSectionBase : public osg::Referenced
{
    public:

        dxfSectionBase()
        {
        }

        virtual ~dxfSectionBase()
        {
        }

        virtual void
        assign( dxfFile*   dxf,
                codeValue& cv ) = 0;
};
