/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Geometry node that auto-rotates its drawables to face the camera.
 * Supports axial and point-to-eye rotation modes.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/nodes/Geode.hpp>

namespace osg
{

    /** Billboard is a derived form of Geode that orients its osg::Drawable
     * children to face the eye point. Typical uses include trees and
     * particle explosions.
     */
    class OSG_EXPORT Billboard : public osg::Inherit<Geode, Billboard>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               Billboard )

            enum Mode
            {
                POINT_ROT_EYE,
                POINT_ROT_WORLD,
                AXIAL_ROT,
            };

            Billboard();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Billboard( const Billboard&,
                       const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            /** Set the billboard rotation mode. */
            void
            setMode( Mode mode );

            /** Get the billboard rotation mode. */
            inline Mode
            getMode() const
            {
                return _mode;
            }

            /** Set the rotation axis for the billboard's child Drawables.
             * Only utilized when mode==AXIAL_ROT. */
            void
            setAxis( const vec3& axis );

            /** Get the rotation axis. */
            inline const vec3&
            getAxis() const
            {
                return _axis;
            }

            /** This normal defines child Drawables' front face direction when unrotated.
             */
            void
            setNormal( const vec3& normal );

            /** Get the front face direction normal. */
            inline const vec3&
            getNormal() const
            {
                return _normal;
            }

            /** Set the specified child Drawable's position. */
            inline void
            setPosition( unsigned int i,
                         const vec3&  pos )
            {
                _positionList[i] = pos;
            }

            /** Get the specified child Drawable's position. */
            inline const vec3&
            getPosition( unsigned int i ) const
            {
                return _positionList[i];
            }

            /** Type definition for pivot point position list. */
            typedef std::vector<vec3> PositionList;

            /** Set the list of pivot point positions. */
            inline void
            setPositionList( PositionList& pl )
            {
                _positionList = pl;
            }

            /** Get the list of pivot point positions. */
            inline PositionList&
            getPositionList()
            {
                return _positionList;
            }

            /** Get a const list of pivot point positions. */
            inline const PositionList&
            getPositionList() const
            {
                return _positionList;
            }

            /** Add a Drawable with a default position of vec3(0,0,0).
             * Call the base-class Geode::addDrawble() to add the given Drawable
             * gset as a child. If Geode::addDrawable() returns true, add the
             * default position to the pivot point position list and return true.
             * Otherwise, return false. */
            virtual bool
            addDrawable( Drawable* gset );

            /** Add a Drawable with a specified position.
             * Call the base-class Geode::addDrawble() to add the given Drawable
             * gset as a child. If Geode::addDrawable() returns true, add the
             * given position pos to the pivot point position list and return true.
             * Otherwise, return false. */
            virtual bool
            addDrawable( Drawable*   gset,
                         const vec3& pos );

            /** Remove a Drawable and its associated position.
             * If gset is a child, remove it, decrement its reference count,
             * remove its pivot point position. and return true.
             * Otherwise, return false. */
            virtual bool
            removeDrawable( Drawable* gset );

            bool
            computeMatrix( dmat4&      modelview,
                           const vec3& eye_local,
                           const vec3& pos_local ) const;

            virtual sphere
            computeBound() const;

        protected:

            virtual ~Billboard();

            enum AxisAligned
            {
                AXIAL_ROT_X_AXIS = AXIAL_ROT + 1,
                AXIAL_ROT_Y_AXIS,
                AXIAL_ROT_Z_AXIS,
                POINT_ROT_WORLD_Z_AXIS,
                CACHE_DIRTY,
            };

            Mode         _mode;
            vec3         _axis;
            vec3         _normal;
            dmat4        _rotateNormalToZAxis;
            PositionList _positionList;

            // used internally as cache of which what _axis is aligned to help
            // decide which method of rotation to use.
            int          _cachedMode;
            vec3         _side;
            void
            updateCache();
    };

}
