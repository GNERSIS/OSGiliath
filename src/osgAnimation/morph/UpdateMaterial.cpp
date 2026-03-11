/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback animating Material properties (diffuse,
 * ambient, etc.) from animation channels.
 */
#include <osgAnimation/morph/UpdateMaterial.hpp>

#include <osg/traversal/NodeVisitor.hpp>

using namespace osgAnimation;

UpdateMaterial::UpdateMaterial( const UpdateMaterial& apc,
                                const osg::CopyOp&    copyop ) :
    Inherit( apc,
             copyop )
{
    _diffuse = new osgAnimation::Vec4Target( apc._diffuse->getValue() );
}

UpdateMaterial::UpdateMaterial( const std::string& name ) :
    Inherit( name )
{
    _diffuse = new osgAnimation::Vec4Target( osg::vec4( 1, 0, 1, 1 ) );
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void
UpdateMaterial::operator()( osg::StateAttribute* sa,
                            osg::NodeVisitor*    nv )
{
    if( nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
    {
        osg::Material* material = dynamic_cast<osg::Material*>( sa );
        if( material )
        {
            update( *material );
        }
    }
}

osgAnimation::Vec4Target*
UpdateMaterial::getDiffuse()
{
    return _diffuse.get();
}

void
UpdateMaterial::update( osg::Material& material )
{
    osg::vec4 diffuse = _diffuse->getValue();
    material.setDiffuse( osg::Material::FRONT_AND_BACK, diffuse );
}

bool
UpdateMaterial::link( osgAnimation::Channel* channel )
{
    if( channel->getName().find( "diffuse" ) != std::string::npos )
    {
        return channel->setTarget( _diffuse.get() );
    }
    else
    {
        OSG_WARN << "Channel " << channel->getName()
                 << " does not contain a valid symbolic name for this class "
                 << className() << std::endl;
    }
    return false;
}
