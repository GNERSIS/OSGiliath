/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traversal visitor that tests intersections against the scene.
 * Drives Intersector objects (line, polytope, plane) through nodes.
 */
#include <osgUtil/intersection/IntersectionVisitor.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Billboard.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/PagedLOD.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/nodes/Transform.hpp>
#include <osgUtil/intersection/LineSegmentIntersector.hpp>

using namespace osgUtil;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  IntersectorGroup
//

IntersectorGroup::IntersectorGroup()
{
}

void
IntersectorGroup::addIntersector( Intersector* intersector )
{
    _intersectors.push_back( intersector );
}

void
IntersectorGroup::clear()
{
    _intersectors.clear();
}

Intersector*
IntersectorGroup::clone( osgUtil::IntersectionVisitor& iv )
{
    IntersectorGroup* ig = new IntersectorGroup;

    // now copy across all intersectors that aren't disabled.
    for( Intersectors::iterator itr = _intersectors.begin(); itr != _intersectors.end();
         ++itr )
    {
        if( !( *itr )->disabled() )
        {
            ig->addIntersector( ( *itr )->clone( iv ) );
        }
    }

    return ig;
}

bool
IntersectorGroup::enter( const osg::Node& node )
{
    if( disabled() )
    {
        return false;
    }

    bool foundIntersections = false;

    for( Intersectors::iterator itr = _intersectors.begin(); itr != _intersectors.end();
         ++itr )
    {
        if( ( *itr )->disabled() )
        {
            ( *itr )->incrementDisabledCount();
        }
        else if( ( *itr )->enter( node ) )
        {
            foundIntersections = true;
        }
        else
        {
            ( *itr )->incrementDisabledCount();
        }
    }

    if( !foundIntersections )
    {
        // need to call leave to clean up the DisabledCount's.
        leave();
        return false;
    }

    // we have found at least one suitable intersector, so return true
    return true;
}

void
IntersectorGroup::leave()
{
    for( Intersectors::iterator itr = _intersectors.begin(); itr != _intersectors.end();
         ++itr )
    {
        if( ( *itr )->disabled() )
        {
            ( *itr )->decrementDisabledCount();
        }
    }
}

void
IntersectorGroup::intersect( osgUtil::IntersectionVisitor& iv,
                             osg::Drawable*                drawable )
{
    if( disabled() )
    {
        return;
    }

    [[maybe_unused]]
    unsigned int numTested = 0;
    for( Intersectors::iterator itr = _intersectors.begin(); itr != _intersectors.end();
         ++itr )
    {
        if( !( *itr )->disabled() )
        {
            ( *itr )->intersect( iv, drawable );

            ++numTested;
        }
    }

    // OSG_NOTICE<<"Number testing "<<numTested<<std::endl;
}

void
IntersectorGroup::reset()
{
    Intersector::reset();

    for( Intersectors::iterator itr = _intersectors.begin(); itr != _intersectors.end();
         ++itr )
    {
        ( *itr )->reset();
    }
}

bool
IntersectorGroup::containsIntersections()
{
    for( Intersectors::iterator itr = _intersectors.begin(); itr != _intersectors.end();
         ++itr )
    {
        if( ( *itr )->containsIntersections() )
        {
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  IntersectionVisitor
//

IntersectionVisitor::IntersectionVisitor( Intersector*  intersector,
                                          ReadCallback* readCallback ) :
    osg::DualModeVisitor( osg::DualModeVisitor::INTERSECTION_VISITOR,
                          osg::DualModeVisitor::TRAVERSE_ACTIVE_CHILDREN )
{
    _useKdTreesWhenAvailable   = true;
    _dummyTraversal            = false;

    _lodSelectionMode          = USE_HIGHEST_LEVEL_OF_DETAIL;
    _eyePointDirty             = true;

    LineSegmentIntersector* ls = dynamic_cast<LineSegmentIntersector*>( intersector );
    if( ls )
    {
        setReferenceEyePoint( osg::vec3( ls->getStart() ) );
        setReferenceEyePointCoordinateFrame( ls->getCoordinateFrame() );
    }
    else
    {
        setReferenceEyePoint( osg::vec3( 0.0F, 0.0F, 0.0F ) );
        setReferenceEyePointCoordinateFrame( Intersector::VIEW );
    }

    setIntersector( intersector );

    setReadCallback( readCallback );
}

void
IntersectionVisitor::setIntersector( Intersector* intersector )
{
    // keep reference around just in case intersector is already in the
    // _intersectorStack, otherwise the clear could delete it.
    osg::ref_ptr<Intersector> temp = intersector;

    _intersectorStack.clear();

    if( intersector )
    {
        _intersectorStack.push_back( intersector );
    }
}

void
IntersectionVisitor::reset()
{
    if( !_intersectorStack.empty() )
    {
        osg::ref_ptr<Intersector> intersector = _intersectorStack.front();
        intersector->reset();

        _intersectorStack.clear();
        _intersectorStack.push_back( intersector );
    }
}

void
IntersectionVisitor::apply( osg::Node& node )
{
    // OSG_NOTICE<<"apply(Node&)"<<std::endl;

    if( !enter( node ) )
    {
        return;
    }

    // OSG_NOTICE<<"inside apply(Node&)"<<std::endl;

    traverse( node );

    leave();
}

void
IntersectionVisitor::apply( osg::Group& group )
{
    if( !enter( group ) )
    {
        return;
    }

    traverse( group );

    leave();
}

void
IntersectionVisitor::apply( osg::Drawable& drawable )
{
    intersect( &drawable );
}

void
IntersectionVisitor::apply( osg::Geode& geode )
{
    // OSG_NOTICE<<"apply(Geode&)"<<std::endl;

    if( !enter( geode ) )
    {
        return;
    }

    // OSG_NOTICE<<"inside apply(Geode&)"<<std::endl;

    for( unsigned int i = 0; i < geode.getNumChildren(); ++i )
    {
        geode.getChild( i )->accept( *this );
    }

    leave();
}

void
IntersectionVisitor::apply( osg::Billboard& billboard )
{
    if( !enter( billboard ) )
    {
        return;
    }

#if 1
    // IntersectVisitor doesn't have getEyeLocal(), can we use
    // NodeVisitor::getEyePoint()?
    osg::vec3 eye_local = getEyePoint();

    for( unsigned int i = 0; i < billboard.getNumChildren(); i++ )
    {
        const osg::vec3&             pos              = billboard.getPosition( i );
        osg::ref_ptr<osg::RefMatrix> billboard_matrix = new osg::RefMatrix;
        if( getViewMatrix() )
        {
            if( getModelMatrix() )
            {
                static_cast<osg::dmat4&>( *billboard_matrix ) =
                    *getViewMatrix() * *getModelMatrix();
            }
            else
            {
                static_cast<osg::dmat4&>( *billboard_matrix ) = *getViewMatrix();
            }
        }
        else if( getModelMatrix() )
        {
            static_cast<osg::dmat4&>( *billboard_matrix ) = *getModelMatrix();
        }

        billboard.computeMatrix( *billboard_matrix, eye_local, pos );

        if( getViewMatrix() )
        {
            static_cast<osg::dmat4&>( *billboard_matrix ) =
                osg::inverse( static_cast<const osg::dmat4&>( *getViewMatrix() ) ) *
                *billboard_matrix;
        }
        pushModelMatrix( billboard_matrix.get() );

        // now push an new intersector clone transform to the new local coordinates
        push_clone();

        billboard.getChild( i )->accept( *this );

        // now push an new intersector clone transform to the new local coordinates
        pop_clone();

        popModelMatrix();
    }
#else

    for( unsigned int i = 0; i < billboard.getNumDrawables(); ++i )
    {
        intersect( billboard.getDrawable( i ) );
    }
#endif

    leave();
}

void
IntersectionVisitor::apply( osg::LOD& lod )
{
    if( !enter( lod ) )
    {
        return;
    }

    traverse( lod );

    leave();
}

void
IntersectionVisitor::apply( osg::PagedLOD& plod )
{
    if( !enter( plod ) )
    {
        return;
    }

    if( plod.getNumFileNames() > 0 )
    {
#if 1
        // Identify the range value for the highest res child
        float targetRangeValue;
        if( plod.getRangeMode() == osg::LOD::DISTANCE_FROM_EYE_POINT )
        {
            targetRangeValue = 1E6;    // Init high to find min value
        }
        else
        {
            targetRangeValue = 0;    // Init low to find max value
        }

        const osg::LOD::RangeList           rl = plod.getRangeList();
        osg::LOD::RangeList::const_iterator rit;
        for( rit = rl.begin(); rit != rl.end(); rit++ )
        {
            if( plod.getRangeMode() == osg::LOD::DISTANCE_FROM_EYE_POINT )
            {
                if( rit->first < targetRangeValue )
                {
                    targetRangeValue = rit->first;
                }
            }
            else
            {
                if( rit->first > targetRangeValue )
                {
                    targetRangeValue = rit->first;
                }
            }
        }

        // Perform an intersection test only on children that display
        // at the maximum resolution.
        unsigned int childIndex;
        for( rit = rl.begin(), childIndex = 0; rit != rl.end(); rit++, childIndex++ )
        {
            if( rit->first != targetRangeValue )
            {
                // This is not one of the highest res children
                continue;
            }

            osg::ref_ptr<osg::Node> child( NULL );
            if( plod.getNumChildren() > childIndex )
            {
                child = plod.getChild( childIndex );
            }

            if( ( !child.valid() ) && ( _readCallback.valid() ) )
            {
                // Child is NULL; attempt to load it, if we have a readCallback...
                unsigned int validIndex( childIndex );
                if( plod.getNumFileNames() <= childIndex )
                {
                    validIndex = plod.getNumFileNames() - 1;
                }

                child = _readCallback->readNodeFile( plod.getDatabasePath() +
                                                     plod.getFileName( validIndex ) );
            }

            if( !child.valid() && plod.getNumChildren() > 0 )
            {
                // Child is still NULL, so just use the one at the end of the list.
                child = plod.getChild( plod.getNumChildren() - 1 );
            }

            if( child.valid() )
            {
                child->accept( *this );
            }
        }
#else
        // older code than above block, that assumes that the PagedLOD is ordered
        // correctly i.e. low res children first, no duplicate ranges.

        osg::ref_ptr<osg::Node> highestResChild;

        if( plod.getNumFileNames() != plod.getNumChildren() && _readCallback.valid() )
        {
            highestResChild = _readCallback->readNodeFile(
                plod.getDatabasePath() + plod.getFileName( plod.getNumFileNames() - 1 )
            );
        }

        if( !highestResChild.valid() && plod.getNumChildren() > 0 )
        {
            highestResChild = plod.getChild( plod.getNumChildren() - 1 );
        }

        if( highestResChild.valid() )
        {
            highestResChild->accept( *this );
        }
#endif
    }

    leave();
}

void
IntersectionVisitor::apply( osg::Transform& transform )
{
    if( !enter( transform ) )
    {
        return;
    }

    osg::ref_ptr<osg::RefMatrix> matrix = _modelStack.empty()
                                            ? new osg::RefMatrix()
                                            : new osg::RefMatrix( *_modelStack.back() );
    transform.computeLocalToWorldMatrix( *matrix, this );

    // We want to ignore the view matrix if the transform is an absolute reference
    if( transform.getReferenceFrame() != osg::Transform::RELATIVE_RF )
    {
        pushViewMatrix( new osg::RefMatrix() );
    }

    pushModelMatrix( matrix.get() );

    // now push an new intersector clone transform to the new local coordinates
    push_clone();

    traverse( transform );

    // pop the clone.
    pop_clone();

    popModelMatrix();

    if( transform.getReferenceFrame() != osg::Transform::RELATIVE_RF )
    {
        popViewMatrix();
    }

    // tidy up an cached cull variables in the current intersector.
    leave();
}

void
IntersectionVisitor::apply( osg::Projection& projection )
{
    if( !enter( projection ) )
    {
        return;
    }

    pushProjectionMatrix( new osg::RefMatrix( projection.getMatrix() ) );

    // now push an new intersector clone transform to the new local coordinates
    push_clone();

    traverse( projection );

    // pop the clone.
    pop_clone();

    popProjectionMatrix();

    leave();
}

void
IntersectionVisitor::apply( osg::Camera& camera )
{
    // OSG_NOTICE<<"apply(Camera&)"<<std::endl;

    // note, commenting out right now because default Camera setup is with the culling
    // active.  Should this be changed? if (!enter(camera)) return;

    // OSG_NOTICE<<"inside apply(Camera&)"<<std::endl;

    osg::RefMatrix* projection = NULL;
    osg::RefMatrix* view       = NULL;
    osg::RefMatrix* model      = NULL;

    if( camera.getReferenceFrame() ==
        osg::Transform::RELATIVE_RF &&
        getProjectionMatrix() &&
        getViewMatrix() )
    {
        if( camera.getTransformOrder() == osg::Camera::POST_MULTIPLY )
        {
            projection = new osg::RefMatrix( *getProjectionMatrix() *
                                             camera.getProjectionMatrix() );
            view       = new osg::RefMatrix( *getViewMatrix() * camera.getViewMatrix() );
            model      = new osg::RefMatrix( *getModelMatrix() );
        }
        else    // pre multiply
        {
            projection = new osg::RefMatrix( camera.getProjectionMatrix() *
                                             ( *getProjectionMatrix() ) );
            view       = new osg::RefMatrix( *getViewMatrix() );
            model = new osg::RefMatrix( camera.getViewMatrix() * ( *getModelMatrix() ) );
        }
    }
    else
    {
        // an absolute reference frame
        projection = new osg::RefMatrix( camera.getProjectionMatrix() );
        view       = new osg::RefMatrix( camera.getViewMatrix() );
        model      = new osg::RefMatrix();
    }

    if( camera.getViewport() )
    {
        pushWindowMatrix( camera.getViewport() );
    }
    pushProjectionMatrix( projection );
    pushViewMatrix( view );
    pushModelMatrix( model );

    // now push an new intersector clone transform to the new local coordinates
    push_clone();

    traverse( camera );

    // pop the clone.
    pop_clone();

    popModelMatrix();
    popViewMatrix();
    popProjectionMatrix();
    if( camera.getViewport() )
    {
        popWindowMatrix();
    }

    // leave();
}

osg::vec3
IntersectionVisitor::getEyePoint() const
{
    if( !_eyePointDirty )
    {
        return _eyePoint;
    }

    osg::dmat4 matrix;
    switch( _referenceEyePointCoordinateFrame )
    {
        case( Intersector::WINDOW ) :
            if( getModelMatrix() )
            {
                matrix = *getModelMatrix();
            }
            if( getViewMatrix() )
            {
                matrix = *getViewMatrix() * matrix;
            }
            if( getProjectionMatrix() )
            {
                matrix = *getProjectionMatrix() * matrix;
            }
            if( getWindowMatrix() )
            {
                matrix = *getWindowMatrix() * matrix;
            }
            break;
        case( Intersector::PROJECTION ) :
            if( getModelMatrix() )
            {
                matrix = *getModelMatrix();
            }
            if( getViewMatrix() )
            {
                matrix = *getViewMatrix() * matrix;
            }
            if( getProjectionMatrix() )
            {
                matrix = *getProjectionMatrix() * matrix;
            }
            break;
        case( Intersector::VIEW ) :
            if( getModelMatrix() )
            {
                matrix = *getModelMatrix();
            }
            if( getViewMatrix() )
            {
                matrix = *getViewMatrix() * matrix;
            }
            break;
        case( Intersector::MODEL ) :
            if( getModelMatrix() )
            {
                matrix = *getModelMatrix();
            }
            break;
    }

    osg::dmat4 inv = osg::inverse( matrix );

    _eyePoint      = osg::vec3( inv * osg::dvec3( _referenceEyePoint ) );
    _eyePointDirty = false;

    return _eyePoint;
}

float
IntersectionVisitor::getDistanceToEyePoint( const osg::vec3& pos,
                                            bool /*withLODScale*/ ) const
{
    if( _lodSelectionMode == USE_EYE_POINT_FOR_LOD_LEVEL_SELECTION )
    {
        return osg::length( pos - getEyePoint() );
    }
    else
    {
        return 0.0F;
    }
}
