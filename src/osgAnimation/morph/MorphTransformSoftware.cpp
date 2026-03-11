/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * CPU-based morph target blending. Interpolates vertex
 * positions on the CPU from morph target arrays.
 */
#include <osgAnimation/morph/MorphTransformSoftware.hpp>

#include <osgAnimation/morph/MorphGeometry.hpp>
#include <osgAnimation/skeletal/BoneMapVisitor.hpp>
#include <osgAnimation/skeletal/VertexInfluence.hpp>

using namespace osgAnimation;

bool
MorphTransformSoftware::init( MorphGeometry& morphGeometry )
{

    morphGeometry.setDataVariance( osg::Object::DataVariance::DYNAMIC );
    osg::Vec3Array* pos =
        dynamic_cast<osg::Vec3Array*>( morphGeometry.getVertexArray() );
    osg::Vec3Array* vertexSource = ( morphGeometry.getVertexSource() );
    osg::Vec3Array* normalSource = ( morphGeometry.getNormalSource() );

    // See if we have an internal optimized geometry

    if( pos )
    {
        if( !vertexSource || vertexSource->size() != pos->size() )
        {
            morphGeometry.setVertexSource( new osg::Vec3Array( pos->begin(),
                                                               pos->end() ) );
            pos->setDataVariance( osg::Object::DataVariance::DYNAMIC );
        }

        osg::Vec3Array* normal =
            dynamic_cast<osg::Vec3Array*>( morphGeometry.getNormalArray() );
        bool normalmorphable = morphGeometry.getMorphNormals() && normal;
        morphGeometry.setMorphNormals( normalmorphable );
        if( normalmorphable &&
            ( !normalSource || normalSource->size() != normal->size() ) )
        {
            morphGeometry.setNormalSource( new osg::Vec3Array( normal->begin(),
                                                               normal->end() ) );
            normal->setDataVariance( osg::Object::DataVariance::DYNAMIC );
        }
    }
    else
    {
        return false;
    }

    _needInit = false;
    return true;
}

void
MorphTransformSoftware::operator()( MorphGeometry& morphGeometry )
{
    if( _needInit )
    {
        if( !init( morphGeometry ) )
        {
            return;
        }
    }
    if( morphGeometry.isDirty() )
    {
        osg::Vec3Array* pos =
            static_cast<osg::Vec3Array*>( morphGeometry.getVertexArray() );
        osg::Vec3Array& vertexSource = *( morphGeometry.getVertexSource() );
        osg::Vec3Array& normalSource = *( morphGeometry.getNormalSource() );
        osg::Vec3Array* normal =
            static_cast<osg::Vec3Array*>( morphGeometry.getNormalArray() );
        bool normalmorphable = morphGeometry.getMorphNormals() && normal;

        if( !vertexSource.empty() )
        {
            bool initialized = false;
            if( morphGeometry.getMethod() == MorphGeometry::NORMALIZED )
            {
                // base * 1 - (sum of weights) + sum of (weight * target)
                float baseWeight = 0;
                for( unsigned int i = 0; i < morphGeometry.getMorphTargetList().size();
                     i++ )
                {
                    baseWeight += morphGeometry.getMorphTarget( i ).getWeight();
                }
                baseWeight = 1 - baseWeight;

                if( baseWeight != 0 )
                {
                    initialized = true;
                    for( unsigned int i = 0; i < pos->size(); i++ )
                    {
                        ( *pos )[i] = vertexSource[i] * baseWeight;
                    }
                    if( normalmorphable )
                    {
                        for( unsigned int i = 0; i < normal->size(); i++ )
                        {
                            ( *normal )[i] = normalSource[i] * baseWeight;
                        }
                    }
                }
            }
            else    // if (_method == RELATIVE)
            {
                // base + sum of (weight * target)
                initialized = true;
                for( unsigned int i = 0; i < pos->size(); i++ )
                {
                    ( *pos )[i] = vertexSource[i];
                }
                if( normalmorphable )
                {
                    for( unsigned int i = 0; i < normal->size(); i++ )
                    {
                        ( *normal )[i] = normalSource[i];
                    }
                }
            }

            for( unsigned int i = 0; i < morphGeometry.getMorphTargetList().size(); i++ )
            {
                if( morphGeometry.getMorphTarget( i ).getWeight() > 0 )
                {
                    // See if any the targets use the internal optimized geometry
                    osg::Geometry* targetGeometry =
                        morphGeometry.getMorphTarget( i ).getGeometry();

                    osg::Vec3Array* targetPos = dynamic_cast<osg::Vec3Array*>(
                        targetGeometry->getVertexArray()
                    );
                    osg::Vec3Array* targetNormals = dynamic_cast<osg::Vec3Array*>(
                        targetGeometry->getNormalArray()
                    );
                    normalmorphable = normalmorphable && targetNormals;
                    if( targetPos )
                    {
                        if( initialized )
                        {
                            // If vertices are initialized, add the morphtargets
                            for( unsigned int j = 0; j < pos->size(); j++ )
                            {
                                ( *pos )[j] +=
                                    ( *targetPos )[j] *
                                    morphGeometry.getMorphTarget( i ).getWeight();
                            }

                            if( normalmorphable )
                            {
                                for( unsigned int j = 0; j < normal->size(); j++ )
                                {
                                    ( *normal )[j] +=
                                        ( *targetNormals )[j] *
                                        morphGeometry.getMorphTarget( i ).getWeight();
                                }
                            }
                        }
                        else
                        {
                            // If not initialized, initialize with this morph target
                            initialized = true;
                            for( unsigned int j = 0; j < pos->size(); j++ )
                            {
                                ( *pos )[j] =
                                    ( *targetPos )[j] *
                                    morphGeometry.getMorphTarget( i ).getWeight();
                            }

                            if( normalmorphable )
                            {
                                for( unsigned int j = 0; j < normal->size(); j++ )
                                {
                                    ( *normal )[j] =
                                        ( *targetNormals )[j] *
                                        morphGeometry.getMorphTarget( i ).getWeight();
                                }
                            }
                        }
                    }
                }
            }

            pos->dirty();
            if( normalmorphable )
            {
                for( unsigned int j = 0; j < normal->size(); j++ )
                {
                    ( *normal )[j] = osg::normalize( ( *normal )[j] );
                }
                normal->dirty();
            }
        }

        morphGeometry.dirtyBound();

        morphGeometry.dirty( false );
    }
}
