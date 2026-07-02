/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgText: computeGlyphGeometry, computeTextGeometry,
 * computeShellGeometry.
 */
#pragma once

#include <osgText/Text3D.hpp>

namespace osgText
{

    extern OSGTEXT_EXPORT osg::Geometry*
                          computeGlyphGeometry( const osgText::Glyph3D* glyph,
                                                const Bevel&            profile,
                                                float                   shellThickness );

    extern OSGTEXT_EXPORT osg::Geometry*
                          computeTextGeometry( const osgText::Glyph3D* glyph,
                                               float                   width );

    extern OSGTEXT_EXPORT osg::Geometry*
                          computeTextGeometry( osg::Geometry* glyphGeometry,
                                               const Bevel&   profile,
                                               float          width );

    extern OSGTEXT_EXPORT osg::Geometry*
                          computeShellGeometry( osg::Geometry* glyphGeometry,
                                                const Bevel&   profile,
                                                float          width );

}
