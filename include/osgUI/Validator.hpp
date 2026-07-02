/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract input validator for text input widgets.
 * Subclasses enforce numeric ranges or regex patterns.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osgUI/Export.hpp>

namespace osgUI
{

    class OSGUI_EXPORT Validator : public osg::Inherit<osg::Object, Validator>
    {
        public:

            Validator();
            Validator( const Validator&   validator,
                       const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               Validator )

            enum State
            {
                INVALID,
                INTERMEDIATE,
                ACCEPTABLE
            };

            /** entry point to validate(..) method, checks for "validate" CallbackObject
               and calls it if present, otherwise calls validateImplementation(..) str
               parameter is the string that needs to be validated cursorpos is the
               position of the cursor within the str string. return validatidy State. */
            virtual State
            validate( std::string& /*str*/,
                      int& cursorpos ) const;

            /// override in subclasses to proviude the validate implementation.
            virtual State
            validateImplementation( std::string& /*str*/,
                                    int& /*cursorpos*/ ) const;

            /** entry point to fixup, checks for "validate" Callbac Object and calls it
               if present, otherwise calls validateImplementation(..) fixup(..) is called
               when user pressers return/enter in a field being edited. str parameter is
               string that needs to be corrected.*/
            virtual void
            fixup( std::string& /*str*/ ) const;

            /// override in subclass to provide the fixup implementation.
            virtual void
            fixupImplementation( std::string& /*str*/ ) const;

        protected:

            virtual ~Validator()
            {
            }
    };

    class OSGUI_EXPORT IntValidator : public osg::Inherit<Validator, IntValidator>
    {
        public:

            IntValidator();
            IntValidator( const IntValidator& widget,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               IntValidator )

            /// set the bottom value that is accepted as valid, default -INT_MAX
            void
            setBottom( int bottom )
            {
                _bottom = bottom;
            }

            int
            getBottom() const
            {
                return _bottom;
            }

            /// set the top value that is accepted as valid, default INT_MAX
            void
            setTop( int top )
            {
                _top = top;
            }

            int
            getTop() const
            {
                return _top;
            }

            /// override validate implementation.
            virtual State
            validateImplementation( std::string& str,
                                    int&         cursorpos ) const;
            /// override validate implementation.
            virtual void
            fixupImplementation( std::string& str ) const;

        protected:

            virtual ~IntValidator()
            {
            }

            int _bottom;
            int _top;
    };

    class OSGUI_EXPORT DoubleValidator : public osg::Inherit<Validator, DoubleValidator>
    {
        public:

            DoubleValidator();
            DoubleValidator( const DoubleValidator& widget,
                             const osg::CopyOp&     copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               DoubleValidator )

            /** set the number of decimal places to accept, default is -1,
                all negative values disable validation against maximum number of places
               thus allows any number of decimals places. */
            void
            setDecimals( int numDecimals )
            {
                _decimals = numDecimals;
            }

            int
            getDecimals() const
            {
                return _decimals;
            }

            /// set the bottom value that is accepted as valid, default -DBL_MAX
            void
            setBottom( double bottom )
            {
                _bottom = bottom;
            }

            double
            getBottom() const
            {
                return _bottom;
            }

            /// set the top value that is accepted as valid, default DBL_MAX
            void
            setTop( double top )
            {
                _top = top;
            }

            double
            getTop() const
            {
                return _top;
            }

            /// override validate implementation.
            virtual State
            validateImplementation( std::string& str,
                                    int&         cursorpos ) const;
            /// override validate implementation.
            virtual void
            fixupImplementation( std::string& str ) const;

        protected:

            virtual ~DoubleValidator()
            {
            }

            int    _decimals;
            double _bottom;
            double _top;
    };

}
