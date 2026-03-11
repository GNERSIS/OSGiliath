/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Tessellation patch parameter attribute. Sets the number of
 * control points per patch and inner/outer tessellation levels.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Class which encapsulates glPatchParameter(..).
     */
    class OSG_EXPORT PatchParameter : public osg::Inherit<StateAttribute, PatchParameter>
    {
        public:

            PatchParameter( GLint vertices = 3 );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            PatchParameter( const PatchParameter& rhs,
                            const CopyOp&         copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( rhs,
                         copyop ),
                _vertices( rhs._vertices ),
                _patchDefaultInnerLevel( rhs._patchDefaultInnerLevel ),
                _patchDefaultOuterLevel( rhs._patchDefaultOuterLevel )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               PatchParameter )

            Type
            getType() const override
            {
                return Type::PATCH_PARAMETER;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( PatchParameter, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _vertices )
                        COMPARE_StateAttribute_Parameter( _patchDefaultInnerLevel )
                            COMPARE_StateAttribute_Parameter(
                                _patchDefaultOuterLevel
                            ) return 0;    // passed all the above comparison macros,
                                           // must be equal.
            }

            /** Set GL_PATCH_VERTICES parameter.*/
            void
            setVertices( GLint vertices )
            {
                _vertices = vertices;
            }

            /** Get GL_PATCH_VERTICES parameter.*/
            GLint
            getVertices() const
            {
                return _vertices;
            }

            /** Set GL_PATCH_DEFAULT_INNER_LEVEL parameter.*/
            void
            setPatchDefaultInnerLevel( const osg::vec2& level )
            {
                _patchDefaultInnerLevel = level;
            }

            /** Get GL_PATCH_DEFAULT_INNER_LEVEL parameter.*/
            const osg::vec2&
            getPatchDefaultInnerLevel() const
            {
                return _patchDefaultInnerLevel;
            }

            /** Set GL_PATCH_DEFAULT_OUTER_LEVEL parameter.*/
            void
            setPatchDefaultOuterLevel( const osg::vec4& level )
            {
                _patchDefaultOuterLevel = level;
            }

            /** Get GL_PATCH_DEFAULT_INNER_LEVEL parameter.*/
            const osg::vec4&
            getPatchDefaultOuterLevel() const
            {
                return _patchDefaultOuterLevel;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~PatchParameter();

            GLint     _vertices;
            osg::vec2 _patchDefaultInnerLevel;
            osg::vec4 _patchDefaultOuterLevel;
    };

}
