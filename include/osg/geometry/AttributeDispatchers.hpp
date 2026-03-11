/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Dispatches vertex attributes to GL during immediate-mode
 * fallback paths. Maps array types to glVertexAttrib calls.
 */
#pragma once

#include <osg/core/ref_ptr.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/maths/mat4.hpp>

namespace osg
{

    // forward declare
    class State;
    class AttributeDispatchMap;

    struct AttributeDispatch : public osg::Referenced
    {
            virtual void
            assign( const GLvoid* )
            {
            }

            virtual void
            operator()( unsigned int ) {};
    };

    /** Helper class for managing the dispatch to OpenGL of various attribute arrays such
     * as stored in osg::Geometry.*/
    class OSG_EXPORT AttributeDispatchers : public osg::Referenced
    {
        public:

            AttributeDispatchers();
            ~AttributeDispatchers();

            void
            setState( osg::State* state );

            void
            reset();

            void
            setUseVertexAttribAlias( bool flag )
            {
                _useVertexAttribAlias = flag;
            }

            bool
            getUseVertexAttribAlias() const
            {
                return _useVertexAttribAlias;
            }

#define DISPATCH_OR_ACTIVATE( array, dispatcher )                                \
    if( array )                                                                  \
    {                                                                            \
        unsigned int binding = static_cast<unsigned int>( array->getBinding() ); \
        if( binding == osg::Array::BIND_OVERALL )                                \
        {                                                                        \
            AttributeDispatch* at = dispatcher;                                  \
            if( at )                                                             \
                ( *at )( 0 );                                                    \
        }                                                                        \
        else if( binding == osg::Array::BIND_PER_PRIMITIVE_SET )                 \
        {                                                                        \
            AttributeDispatch* at = dispatcher;                                  \
            if( at )                                                             \
                _activeDispatchList.push_back( at );                             \
        }                                                                        \
    }

            void
            activateColorArray( osg::Array* array )
            {
                DISPATCH_OR_ACTIVATE( array, colorDispatcher( array ) );
            }

            void
            activateNormalArray( osg::Array* array )
            {
                DISPATCH_OR_ACTIVATE( array, normalDispatcher( array ) );
            }

            void
            activateSecondaryColorArray( osg::Array* array )
            {
                DISPATCH_OR_ACTIVATE( array, secondaryColorDispatcher( array ) );
            }

            void
            activateFogCoordArray( osg::Array* array )
            {
                DISPATCH_OR_ACTIVATE( array, fogCoordDispatcher( array ) );
            }

            void
            activateVertexAttribArray( unsigned int unit,
                                       osg::Array*  array )
            {
                DISPATCH_OR_ACTIVATE( array, vertexAttribDispatcher( unit, array ) );
            }

            AttributeDispatch*
            normalDispatcher( Array* array );
            AttributeDispatch*
            colorDispatcher( Array* array );
            AttributeDispatch*
            secondaryColorDispatcher( Array* array );
            AttributeDispatch*
            fogCoordDispatcher( Array* array );
            AttributeDispatch*
            vertexAttribDispatcher( unsigned int unit,
                                    Array*       array );

            void
            dispatch( unsigned int index )
            {
                for( AttributeDispatchList::iterator itr = _activeDispatchList.begin();
                     itr != _activeDispatchList.end();
                     ++itr )
                {
                    ( *( *itr ) )( index );
                }
            }

            bool
            active() const
            {
                return !_activeDispatchList.empty();
            }

        protected:

            void
            init();

            void
            assignTexCoordDispatchers( unsigned int unit );
            void
                                  assignVertexAttribDispatchers( unsigned int unit );

            bool                  _initialized;
            State*                _state;

            AttributeDispatchMap* _normalDispatchers;
            AttributeDispatchMap* _colorDispatchers;
            AttributeDispatchMap* _secondaryColorDispatchers;
            AttributeDispatchMap* _fogCoordDispatchers;

            typedef std::vector<AttributeDispatchMap*> AttributeDispatchMapList;
            AttributeDispatchMapList                   _vertexAttribDispatchers;

            typedef std::vector<AttributeDispatch*>    AttributeDispatchList;

            AttributeDispatchList                      _activeDispatchList;

            bool                                       _useVertexAttribAlias;
    };

}
