/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Drawable that renders procedural shapes (sphere, box, cylinder,
 * cone, capsule) by tessellating them into triangle geometry.
 */
#pragma once

#include <osg/geometry/Geometry.hpp>

namespace osg
{

    /** Allow the use of <tt>Shape</tt>s as <tt>Drawable</tt>s, so that they can
     *  be rendered with reduced effort. The implementation of \c ShapeDrawable is
     *  not geared to efficiency; it's better to think of it as a convenience to
     *  render <tt>Shape</tt>s easily (perhaps for test or debugging purposes) than
     *  as the right way to render basic shapes in some efficiency-critical section
     *  of code.
     */
    class OSG_EXPORT ShapeDrawable : public osg::Geometry
    {
        public:

            ShapeDrawable();

            ShapeDrawable( Shape*             shape,
                           TessellationHints* hints = 0 );

            template<class T>
            ShapeDrawable( const ref_ptr<T>&  shape,
                           TessellationHints* hints = 0 ) :
                _tessellationHints( hints )
            {
                setShape( shape.get() );
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            ShapeDrawable( const ShapeDrawable& pg,
                           const CopyOp&        copyop = CopyOp::SHALLOW_COPY );

            virtual Object*
            cloneType() const
            {
                return new ShapeDrawable();
            }

            virtual Object*
            clone( const CopyOp& copyop ) const
            {
                return new ShapeDrawable( *this, copyop );
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const ShapeDrawable*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "ShapeDrawable";
            }

            virtual void
            setShape( Shape* shape );

            /** Set the color of the shape.*/
            void
            setColor( const vec4& color );

            /** Get the color of the shape.*/
            const vec4&
            getColor() const
            {
                return _color;
            }

            void
            setTessellationHints( TessellationHints* hints );

            TessellationHints*
            getTessellationHints()
            {
                return _tessellationHints.get();
            }

            const TessellationHints*
            getTessellationHints() const
            {
                return _tessellationHints.get();
            }

            /** method to invoke to rebuild the geometry that renders the shape.*/
            void
            build();

        protected:

            ShapeDrawable&
            operator=( const ShapeDrawable& )
            {
                return *this;
            }

            virtual ~ShapeDrawable();

            vec4                       _color;

            ref_ptr<TessellationHints> _tessellationHints;
    };

}
