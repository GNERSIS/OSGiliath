/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * LOD node that replaces distant geometry with a billboard
 * image captured from the current viewpoint.
 */
#include <osgSim/Impostor>

#include <algorithm>
#include <osg/maths/compat.hpp>
#include <osg/maths/Matrix.hpp>

using namespace osg;
using namespace osgSim;

// use this cull callback to allow the camera to traverse the Impostor's children without
// actuall having them assigned as children to the camea itself.  This make the camera a
// decorator without ever directly being assigned to it.
class ImpostorTraverseNodeCallback : public osg::NodeCallback
{
    public:

        ImpostorTraverseNodeCallback( osgSim::Impostor* node ) :
            _node( node )
        {
        }

        virtual void
        operator()( osg::Node*,
                    osg::NodeVisitor* nv )
        {
            _node->LOD::traverse( *nv );
        }

        osgSim::Impostor* _node;
};

Impostor::Impostor()
{
    _impostorThreshold = -1.0F;
}

ImpostorSprite*
Impostor::findBestImpostorSprite( unsigned int     contextID,
                                  const osg::vec3& currLocalEyePoint ) const
{
    ImpostorSpriteList& impostorSpriteList = _impostorSpriteListBuffer[contextID];

    float               min_distance2      = FLT_MAX;
    ImpostorSprite*     impostorSprite     = NULL;
    for( ImpostorSpriteList::iterator itr = impostorSpriteList.begin();
         itr != impostorSpriteList.end();
         ++itr )
    {
        float distance2 =
            osg::length2( currLocalEyePoint - ( *itr )->getStoredLocalEyePoint() );
        if( distance2 < min_distance2 )
        {
            min_distance2  = distance2;
            impostorSprite = itr->get();
        }
    }
    return impostorSprite;
}

void
Impostor::addImpostorSprite( unsigned int    contextID,
                             ImpostorSprite* is )
{
    if( is && is->getParent() != this )
    {
        ImpostorSpriteList& impostorSpriteList = _impostorSpriteListBuffer[contextID];

        // add it to my impostor list first, so it remains referenced
        // when its reference in the previous_owner is removed.
        impostorSpriteList.push_back( is );

        if( is->getParent() )
        {
            Impostor*           previous_owner = is->getParent();
            ImpostorSpriteList& isl =
                previous_owner->_impostorSpriteListBuffer[contextID];

            // find and erase reference to is.
            for( ImpostorSpriteList::iterator itr = isl.begin(); itr != isl.end();
                 ++itr )
            {
                if( ( *itr ) == is )
                {
                    isl.erase( itr );
                    break;
                }
            }
        }
        is->setParent( this );
    }
}

osg::sphere
Impostor::computeBound() const
{
    return LOD::computeBound();
}

inline osgUtil::CullVisitor::value_type
distance( const osg::vec3&  coord,
          const osg::dmat4& matrix )
{

    // std::cout << "distance("<<coord<<", "<<matrix<<")"<<std::endl;

    return -( ( osgUtil::CullVisitor::value_type )coord[0] *
              ( osgUtil::CullVisitor::value_type )matrix( 2, 0 ) +
              ( osgUtil::CullVisitor::value_type )coord[1] *
              ( osgUtil::CullVisitor::value_type )matrix( 2, 1 ) +
              ( osgUtil::CullVisitor::value_type )coord[2] *
              ( osgUtil::CullVisitor::value_type )matrix( 2, 2 ) +
              matrix( 2, 3 ) );
}

void
Impostor::traverse( osg::NodeVisitor& nv )
{
    if( nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR )
    {
        LOD::traverse( nv );
        return;
    }

    osgUtil::CullVisitor* cv = nv.asCullVisitor();
    if( !cv )
    {
        LOD::traverse( nv );
        return;
    }

    osg::vec3     eyeLocal  = nv.getEyePoint();
    const sphere& bs        = getBound();

    unsigned int  contextID = cv->getState() ? cv->getState()->getContextID() : 0;

    float         distance2 = osg::length2( eyeLocal - bs.center );
    float         LODScale  = cv->getLODScale();
    if( !cv->getImpostorsActive() ||
        distance2 *
        LODScale *
        LODScale <
        osg::square( getImpostorThreshold() ) ||
        distance2 <
        bs.radius2() *
        2.0F )
    {
        // outwith the impostor distance threshold therefore simple
        // traverse the appropriate child of the LOD.
        LOD::traverse( nv );
    }
    else
    {

        // within the impostor distance threshold therefore attempt
        // to use impostor instead.

        RefMatrix&      matrix = *cv->getModelViewMatrix();

        // search for the best fit ImpostorSprite;
        ImpostorSprite* impostorSprite = findBestImpostorSprite( contextID, eyeLocal );

        if( impostorSprite )
        {
            // impostor found, now check to see if it is good enough to use
            float error = impostorSprite->calcPixelError( *( cv->getMVPW() ) );

            if( error > cv->getImpostorPixelErrorThreshold() )
            {
                // chosen impostor sprite pixel error is too great to use
                // from this eye point, therefore invalidate it.
                impostorSprite = NULL;
            }
        }

        // need to think about sprite reuse and support for multiple context's.

        if( impostorSprite == NULL )
        {
            // no appropriate sprite has been found therefore need to create
            // one for use

            // create the impostor sprite.
            impostorSprite = createImpostorSprite( cv );

            // if (impostorSprite) impostorSprite->_color.set(0.0f,0.0f,1.0f,1.0f);
        }
        // else impostorSprite->_color.set(1.0f,1.0f,1.0f,1.0f);

        if( impostorSprite )
        {

            // update frame number to show that impostor is in action.
            impostorSprite->setLastFrameUsed( cv->getTraversalNumber() );

            if( cv->getComputeNearFarMode() )
            {
                cv->updateCalculatedNearFar( matrix, *impostorSprite, false );
            }

            StateSet* stateset = impostorSprite->getStateSet();

            if( stateset )
            {
                cv->pushStateSet( stateset );
            }

            cv->addDrawableAndDepth( impostorSprite,
                                     &matrix,
                                     static_cast<float>( distance( getCenter(),
                                                                   matrix ) ) );

            if( stateset )
            {
                cv->popStateSet();
            }
        }
        else
        {
            // no impostor has been selected or created so default to
            // traversing the usual LOD selected child.
            LOD::traverse( nv );
        }
    }
}

ImpostorSprite*
Impostor::createImpostorSprite( osgUtil::CullVisitor* cv )
{
    unsigned int contextID = cv->getState() ? cv->getState()->getContextID() : 0;

    osgSim::ImpostorSpriteManager* impostorSpriteManager =
        dynamic_cast<osgSim::ImpostorSpriteManager*>( cv->getUserData() );
    if( !impostorSpriteManager )
    {
        impostorSpriteManager = new osgSim::ImpostorSpriteManager;
        cv->setUserData( impostorSpriteManager );
    }

    // default to true right now, will dertermine if perspective from the
    // projection matrix...
    bool          isPerspectiveProjection = true;

    const dmat4&  matrix                  = *( cv->getModelViewMatrix() );
    const sphere& bs                      = getBound();
    osg::vec3     eye_local               = cv->getEyeLocal();

    if( !bs.valid() )
    {
        OSG_WARN << "bb invalid" << std::endl;
        return NULL;
    }

    vec3  center_local     = bs.center;
    vec3  camera_up_local  = cv->getUpLocal();
    vec3  lv_local         = center_local - eye_local;

    float distance_local   = osg::length( lv_local );
    lv_local              /= distance_local;

    vec3 sv_local          = lv_local ^ camera_up_local;
    sv_local               = osg::normalize( sv_local );

    vec3  up_local         = sv_local ^ lv_local;

    float width            = bs.radius;
    if( isPerspectiveProjection )
    {
        // expand the width to account for projection onto sprite.
        width *=
            ( distance_local / sqrtf( distance_local * distance_local - bs.radius2() ) );
    }

    // scale up and side vectors to sprite width.
    up_local *= width;
    sv_local *= width;

    // create the corners of the sprite.
    vec3              c00( center_local - sv_local - up_local );
    vec3              c10( center_local + sv_local - up_local );
    vec3              c01( center_local - sv_local + up_local );
    vec3              c11( center_local + sv_local + up_local );

    // calc texture size for eye, bs.

    // convert the corners of the sprite (in world coords) into their
    // equivalent window coordinates by using the camera's project method.
    const osg::dmat4& MVPW    = *( cv->getMVPW() );
    vec3              c00_win = vec3( c00 * MVPW );
    vec3              c11_win = vec3( c11 * MVPW );

    // adjust texture size to be nearest power of 2.

    float             s = c11_win.x - c00_win.x;
    float             t = c11_win.y - c00_win.y;

    // may need to reverse sign of width or height if a matrix has
    // been applied which flips the orientation of this subgraph.
    if( s < 0.0F )
    {
        s = -s;
    }
    if( t < 0.0F )
    {
        t = -t;
    }

    // bias value used to assist the rounding up or down of
    // the texture dimensions to the nearest power of two.
    // bias near 0.0 will almost always round down.
    // bias near 1.0 will almost always round up.
    float                bias        = 0.7F;

    float                sp2         = logf( ( float )s ) / logf( 2.0F );
    float                rounded_sp2 = floorf( sp2 + bias );
    int                  new_s       = ( int )( powf( 2.0F, rounded_sp2 ) );

    float                tp2         = logf( ( float )t ) / logf( 2.0F );
    float                rounded_tp2 = floorf( tp2 + bias );
    int                  new_t       = ( int )( powf( 2.0F, rounded_tp2 ) );

    const osg::Viewport& viewport    = *( cv->getViewport() );

    // if dimension is bigger than window divide it down.
    while( new_s > viewport.width() )
    {
        new_s /= 2;
    }

    // if dimension is bigger than window divide it down.
    while( new_t > viewport.height() )
    {
        new_t /= 2;
    }

    // create the impostor sprite.
    ImpostorSprite* impostorSprite = impostorSpriteManager->createOrReuseImpostorSprite(
        new_s,
        new_t,
        cv->getTraversalNumber() -
            static_cast<unsigned int>( cv->getNumberOfFrameToKeepImpostorSprites() )
    );

    if( impostorSprite == NULL )
    {
        OSG_WARN << "Warning: unable to create required impostor sprite." << std::endl;
        return NULL;
    }

    // update frame number to show that impostor is in action.
    impostorSprite->setLastFrameUsed( cv->getTraversalNumber() );

    // have successfully created an impostor sprite so now need to
    // add it into the impostor.
    addImpostorSprite( contextID, impostorSprite );

    if( cv->getDepthSortImpostorSprites() )
    {
        // the depth sort bin should probably be user definable,
        // will look into this later. RO July 2001.
        StateSet* stateset = impostorSprite->getStateSet();
        stateset->setRenderBinDetails( 10, "DepthSortedBin" );
    }

    osg::Texture2D* texture = impostorSprite->getTexture();

    texture->setTextureSize( new_s, new_t );
    texture->setInternalFormat( GL_RGBA );
    texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );

    // update frame number to show that impostor is in action.
    impostorSprite->setLastFrameUsed( cv->getTraversalNumber() );

    vec3* coords    = impostorSprite->getCoords();
    vec2* texcoords = impostorSprite->getTexCoords();

    coords[0]       = c01;
    texcoords[0].set( 0.0F, 1.0F );

    coords[1] = c00;
    texcoords[1].set( 0.0F, 0.0F );

    coords[2] = c10;
    texcoords[2].set( 1.0F, 0.0F );

    coords[3] = c11;
    texcoords[3].set( 1.0F, 1.0F );

    impostorSprite->dirty();

    vec3* controlcoords = impostorSprite->getControlCoords();

    if( isPerspectiveProjection )
    {
        // deal with projection issue by moving the coorners of the quad
        // towards the eye point.
        float ratio           = width / osg::length( center_local - eye_local );
        float one_minus_ratio = 1.0F - ratio;
        vec3  eye_local_ratio = eye_local * ratio;

        controlcoords[0]      = coords[0] * one_minus_ratio + eye_local_ratio;
        controlcoords[1]      = coords[1] * one_minus_ratio + eye_local_ratio;
        controlcoords[2]      = coords[2] * one_minus_ratio + eye_local_ratio;
        controlcoords[3]      = coords[3] * one_minus_ratio + eye_local_ratio;
    }
    else
    {
        // project the control points forward towards the eyepoint,
        // but since this an othographics projection this projection is
        // parallel.
        vec3 dv          = lv_local * width;

        controlcoords[0] = coords[0] - dv;
        controlcoords[1] = coords[1] - dv;
        controlcoords[2] = coords[2] - dv;
        controlcoords[3] = coords[3] - dv;
    }

    impostorSprite->setStoredLocalEyePoint( eye_local );

    vec3         eye_world( 0.0, 0.0, 0.0 );
    vec3         center_world = vec3( matrix * dvec3( bs.center ) );

    osg::Camera* camera       = impostorSprite->getCamera();
    if( !camera )
    {
        camera = new osg::Camera;
        impostorSprite->setCamera( camera );
    }

    camera->setCullCallback( new ImpostorTraverseNodeCallback( this ) );

    osgUtil::RenderStage* previous_stage = cv->getRenderStage();

    // set up the background color and clear mask.
    osg::vec4             clear_color = previous_stage->getClearColor();
    clear_color[3]                    = 0.0F;    // set thae alpha to zero.
    camera->setClearColor( clear_color );
    camera->setClearMask( previous_stage->getClearMask() );

    // adjust camera left,right,up,down to fit (in world coords)

    vec3  near_local( center_local - lv_local * width );
    vec3  far_local( center_local + lv_local * width );
    vec3  top_local( center_local + up_local );
    vec3  right_local( center_local + sv_local );

    vec3  near_world   = vec3( matrix * dvec3( near_local ) );
    vec3  far_world    = vec3( matrix * dvec3( far_local ) );
    vec3  top_world    = vec3( matrix * dvec3( top_local ) );
    vec3  right_world  = vec3( matrix * dvec3( right_local ) );

    float znear        = osg::length( near_world - eye_world );
    float zfar         = osg::length( far_world - eye_world );

    float top          = osg::length( top_world - center_world );
    float right        = osg::length( right_world - center_world );

    znear             *= 0.9F;
    zfar              *= 1.1F;

    // set up projection.
    if( isPerspectiveProjection )
    {
        // deal with projection issue move the top and right points
        // onto the near plane.
        float ratio  = znear / osg::length( center_world - eye_world );
        top         *= ratio;
        right       *= ratio;
        camera->setProjectionMatrixAsFrustum( -right, right, -top, top, znear, zfar );
    }
    else
    {
        camera->setProjectionMatrixAsOrtho( -right, right, -top, top, znear, zfar );
    }

    vec3       rotate_from = bs.center - eye_local;
    vec3       rotate_to   = cv->getLookVectorLocal();

    osg::dvec3 eye_local_d( eye_local.x, eye_local.y, eye_local.z );
    osg::dvec3 rotate_from_d( rotate_from.x, rotate_from.y, rotate_from.z );
    osg::dvec3 rotate_to_d( rotate_to.x, rotate_to.y, rotate_to.z );
    osg::dmat4 rotate_matrix = osg::translate( -eye_local_d ) *
                               osg::rotate( rotate_from_d, rotate_to_d ) *
                               osg::translate( eye_local_d ) *
                             *cv->getModelViewMatrix();

    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setViewMatrix( rotate_matrix );

    camera->setViewport( 0, 0, new_s, new_t );

    // tell the camera to use OpenGL frame buffer object where supported.
    camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT,
                                           osg::Camera::FRAME_BUFFER );

    // set the camera to render before the main camera.
    camera->setRenderOrder( osg::Camera::PRE_RENDER );

    // attach the texture and use it as the color buffer.
    camera->attach( osg::Camera::COLOR_BUFFER, texture );

    // do the cull traversal on the subgraph
    camera->accept( *cv );

    return impostorSprite;
}
