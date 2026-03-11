/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Node that selectively enables/disables children by index.
 * Used for toggling visibility of scene graph branches.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Group.hpp>

namespace osg
{

    /** Switch is a Group node that allows switching between children.
     * Typical uses would be for objects which might need to be rendered
     * differently at different times, for instance a switch could be used
     * to represent the different states of a traffic light.
     */
    class OSG_EXPORT Switch : public osg::Inherit<Group, Switch>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               Switch )

            Switch();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Switch( const Switch&,
                    const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            Switch*
            asSwitch() override
            {
                return this;
            }

            const Switch*
            asSwitch() const override
            {
                return this;
            }

            void
            traverse( NodeVisitor& nv ) override;
            void
            traverse( ConstNodeVisitor& nv ) const override;

            void
            setNewChildDefaultValue( bool value )
            {
                _newChildDefaultValue = value;
            }

            bool
            getNewChildDefaultValue() const
            {
                return _newChildDefaultValue;
            }

            using osg::Group::addChild;
            using osg::Group::insertChild;

            bool
            addChild( Node* child ) override;

            virtual bool
            addChild( Node* child,
                      bool  value );

            bool
            insertChild( unsigned int index,
                         Node*        child ) override;

            virtual bool
            insertChild( unsigned int index,
                         Node*        child,
                         bool         value );

            bool
            removeChildren( unsigned int pos,
                            unsigned int numChildrenToRemove ) override;

            void
            setValue( unsigned int pos,
                      bool         value );

            bool
            getValue( unsigned int pos ) const;

            void
            setChildValue( const Node* child,
                           bool        value );

            bool
            getChildValue( const Node* child ) const;

            /** Set all the children off (false), and set the new default child
             * value to off (false). */
            bool
            setAllChildrenOff();

            /** Set all the children on (true), and set the new default child
             * value to on (true). */
            bool
            setAllChildrenOn();

            /** Set a single child on, switch off all other children. */
            bool
                                      setSingleChildOn( unsigned int pos );

            typedef std::vector<bool> ValueList;

            void
            setValueList( const ValueList& values )
            {
                _values = values;
            }

            const ValueList&
            getValueList() const
            {
                return _values;
            }

            sphere
            computeBound() const override;

        protected:

            virtual ~Switch()
            {
            }

            // This is effectively a bit mask.
            bool      _newChildDefaultValue;
            ValueList _values;
    };

}
