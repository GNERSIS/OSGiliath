/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Deduplicates identical state attributes across loaded models.
 * Shares textures, state sets, and programs to reduce GPU memory.
 */
#pragma once

#include <mutex>
#include <osg/core/Inherit.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgDB/Export.hpp>
#include <set>

namespace osgDB
{

    class OSGDB_EXPORT SharedStateManager : public osg::DualModeVisitor
    {
        public:

            enum ShareMode
            {
                SHARE_NONE                  = 0,
                SHARE_STATIC_TEXTURES       = 1 << 0,
                SHARE_UNSPECIFIED_TEXTURES  = 1 << 1,
                SHARE_DYNAMIC_TEXTURES      = 1 << 2,
                SHARE_STATIC_STATESETS      = 1 << 3,
                SHARE_UNSPECIFIED_STATESETS = 1 << 4,
                SHARE_DYNAMIC_STATESETS     = 1 << 5,
                SHARE_TEXTURES  = SHARE_STATIC_TEXTURES | SHARE_UNSPECIFIED_TEXTURES,
                SHARE_STATESETS = SHARE_STATIC_STATESETS | SHARE_UNSPECIFIED_STATESETS,
                SHARE_ALL       = SHARE_TEXTURES | SHARE_STATESETS,
            };

            SharedStateManager( unsigned int mode = SHARE_ALL );

            OSG_REGISTER_TYPE( osgDB,
                               SharedStateManager )

            void
            setShareMode( unsigned int mode );

            unsigned int
            getShareMode()
            {
                return _shareMode;
            }

            // Call right after each unload and before Registry cache prune.
            void
            prune();

            // Call right after each load
            void
            share( osg::Node*  node,
                   std::mutex* mt = 0 );

            using osg::ConstNodeVisitor::apply;
            using osg::NodeVisitor::apply;

            void
            apply( osg::Node& node );

            // Answers the question "Will this state set be eliminated by
            // the SharedStateManager because an equivalent one has been
            // seen already?" Safe to call from the pager thread.
            bool
            isShared( osg::StateSet* stateSet );

            bool
            isShared( osg::Texture* texture );

            void
            releaseGLObjects( osg::State* state ) const;

        protected:

            inline bool
            shareTexture( osg::Object::DataVariance variance )
            {
                return _shareTexture[static_cast<int>( variance )];
            }

            inline bool
            shareStateSet( osg::Object::DataVariance variance )
            {
                return _shareStateSet[static_cast<int>( variance )];
            }

            void
            process( osg::StateSet* ss,
                     osg::Object*   parent );
            osg::StateAttribute*
            find( osg::StateAttribute* sa );
            osg::StateSet*
            find( osg::StateSet* ss );
            void
            setStateSet( osg::StateSet* ss,
                         osg::Object*   object );
            void
            shareTextures( osg::StateSet* ss );

            struct CompareStateAttributes
            {
                    bool
                    operator()( const osg::ref_ptr<osg::StateAttribute>& lhs,
                                const osg::ref_ptr<osg::StateAttribute>& rhs ) const
                    {
                        return *lhs < *rhs;
                    }
            };

            struct CompareStateSets
            {
                    bool
                    operator()( const osg::ref_ptr<osg::StateSet>& lhs,
                                const osg::ref_ptr<osg::StateSet>& rhs ) const
                    {
                        return lhs->compare( *rhs, true ) < 0;
                    }
            };

            // Lists of shared objects
            typedef std::set<osg::ref_ptr<osg::StateAttribute>, CompareStateAttributes>
                       TextureSet;
            TextureSet _sharedTextureList;

            typedef std::set<osg::ref_ptr<osg::StateSet>, CompareStateSets> StateSetSet;
            StateSetSet                                   _sharedStateSetList;

            // Temporary lists just to avoid unnecessary find calls
            typedef std::pair<osg::StateAttribute*, bool> TextureSharePair;
            typedef std::map<osg::StateAttribute*, TextureSharePair>
                                                    TextureTextureSharePairMap;
            TextureTextureSharePairMap              tmpSharedTextureList;

            typedef std::pair<osg::StateSet*, bool> StateSetSharePair;
            typedef std::map<osg::StateSet*, StateSetSharePair>
                                         StateSetStateSetSharePairMap;
            StateSetStateSetSharePairMap tmpSharedStateSetList;

            unsigned int                 _shareMode;
            bool                         _shareTexture[3];
            bool                         _shareStateSet[3];

            // Share connection mutex

            std::mutex*                  _mutex;
            // Mutex for doing isShared queries from other threads
            mutable std::mutex           _listMutex;
    };

}
