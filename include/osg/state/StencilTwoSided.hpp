/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Two-sided stencil operations. Configures independent stencil
 * test and ops for front and back faces in a single pass.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/Stencil.hpp>

namespace osg
{

    /** Provides OpenGL two sided stencil functionality, also known as separate stencil.
     *  It enables to specify different stencil function for front and back facing
     * polygons. Two sided stenciling is used usually to eliminate the need of two
     * rendering passes when using standard stenciling functions. See also \sa
     * osg::Stencil.
     *
     *  Uses core OpenGL 2.0+ glStencilFuncSeparate, glStencilOpSeparate, and
     *  glStencilMaskSeparate functions which are required in OpenGL 4.6 Core Profile.
     */
    class OSG_EXPORT StencilTwoSided
        : public osg::Inherit<StateAttribute, StencilTwoSided>
    {
        public:

            StencilTwoSided();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            StencilTwoSided( const StencilTwoSided& stencil,
                             const CopyOp&          copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               StencilTwoSided )

            Type
            getType() const override
            {
                return Type::STENCIL;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            int
            compare( const StateAttribute& sa ) const override;

            bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const override
            {
                usage.usesMode( GL_STENCIL_TEST );
                return true;
            }

            enum Face
            {
                FRONT = 0,
                BACK  = 1,
            };

            enum class Function
            {
                NEVER    = GL_NEVER,
                LESS     = GL_LESS,
                EQUAL    = GL_EQUAL,
                LEQUAL   = GL_LEQUAL,
                GREATER  = GL_GREATER,
                NOTEQUAL = GL_NOTEQUAL,
                GEQUAL   = GL_GEQUAL,
                ALWAYS   = GL_ALWAYS,
            };

            inline void
            setFunction( Face         face,
                         Function     func,
                         int          ref,
                         unsigned int mask )
            {
                _func[face]     = func;
                _funcRef[face]  = ref;
                _funcMask[face] = mask;
            }

            inline void
            setFunction( Face     face,
                         Function func )
            {
                _func[face] = func;
            }

            inline Function
            getFunction( Face face ) const
            {
                return _func[face];
            }

            inline void
            setFunctionRef( Face face,
                            int  ref )
            {
                _funcRef[face] = ref;
            }

            inline int
            getFunctionRef( Face face ) const
            {
                return _funcRef[face];
            }

            inline void
            setFunctionMask( Face         face,
                             unsigned int mask )
            {
                _funcMask[face] = mask;
            }

            inline unsigned int
            getFunctionMask( Face face ) const
            {
                return _funcMask[face];
            }

            enum class Operation
            {
                KEEP      = GL_KEEP,
                ZERO      = GL_ZERO,
                REPLACE   = GL_REPLACE,
                INCR      = GL_INCR,
                DECR      = GL_DECR,
                INVERT    = GL_INVERT,
                INCR_WRAP = GL_INCR_WRAP,
                DECR_WRAP = GL_DECR_WRAP,
            };

            /** set the operations to apply when the various stencil and depth
             * tests fail or pass.  First parameter is to control the operation
             * when the stencil test fails.  The second parameter is to control the
             * operation when the stencil test passes, but depth test fails. The
             * third parameter controls the operation when both the stencil test
             * and depth pass.  Ordering of parameter is the same as if using
             * glStencilOp(,,).*/
            inline void
            setOperation( Face      face,
                          Operation sfail,
                          Operation zfail,
                          Operation zpass )
            {
                _sfail[face] = sfail;
                _zfail[face] = zfail;
                _zpass[face] = zpass;
            }

            /** set the operation when the stencil test fails.*/
            inline void
            setStencilFailOperation( Face      face,
                                     Operation sfail )
            {
                _sfail[face] = sfail;
            }

            /** get the operation when the stencil test fails.*/
            inline Operation
            getStencilFailOperation( Face face ) const
            {
                return _sfail[face];
            }

            /** set the operation when the stencil test passes but the depth test
             * fails.*/
            inline void
            setStencilPassAndDepthFailOperation( Face      face,
                                                 Operation zfail )
            {
                _zfail[face] = zfail;
            }

            /** get the operation when the stencil test passes but the depth test
             * fails.*/
            inline Operation
            getStencilPassAndDepthFailOperation( Face face ) const
            {
                return _zfail[face];
            }

            /** set the operation when both the stencil test and the depth test pass.*/
            inline void
            setStencilPassAndDepthPassOperation( Face      face,
                                                 Operation zpass )
            {
                _zpass[face] = zpass;
            }

            /** get the operation when both the stencil test and the depth test pass.*/
            inline Operation
            getStencilPassAndDepthPassOperation( Face face ) const
            {
                return _zpass[face];
            }

            inline void
            setWriteMask( Face         face,
                          unsigned int mask )
            {
                _writeMask[face] = mask;
            }

            inline unsigned int
            getWriteMask( Face face ) const
            {
                return _writeMask[face];
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~StencilTwoSided();

            Function     _func[2];
            int          _funcRef[2];
            unsigned int _funcMask[2];

            Operation    _sfail[2];
            Operation    _zfail[2];
            Operation    _zpass[2];

            unsigned int _writeMask[2];
    };

}
