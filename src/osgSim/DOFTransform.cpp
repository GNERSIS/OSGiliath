/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Degree-of-freedom transform node for articulated models.
 * Supports translate, rotate, and scale with limits per axis.
 */
#include <osgSim/DOFTransform.hpp>

using namespace osgSim;
using namespace osg;

static const unsigned int TRANSLATION_X_LIMIT_BIT  = 0X80'00'00'00U >> 0;
static const unsigned int TRANSLATION_Y_LIMIT_BIT  = 0X80'00'00'00U >> 1;
static const unsigned int TRANSLATION_Z_LIMIT_BIT  = 0X80'00'00'00U >> 2;
static const unsigned int ROTATION_PITCH_LIMIT_BIT = 0X80'00'00'00U >> 3;
static const unsigned int ROTATION_ROLL_LIMIT_BIT  = 0X80'00'00'00U >> 4;
static const unsigned int ROTATION_YAW_LIMIT_BIT   = 0X80'00'00'00U >> 5;
static const unsigned int SCALE_X_LIMIT_BIT        = 0X80'00'00'00U >> 6;
static const unsigned int SCALE_Y_LIMIT_BIT        = 0X80'00'00'00U >> 7;
static const unsigned int SCALE_Z_LIMIT_BIT        = 0X80'00'00'00U >> 8;

DOFTransform::DOFTransform() :
    _previousTraversalNumber( osg::UNINITIALIZED_FRAME_NUMBER ),
    _previousTime( 0.0 ),
    _limitationFlags( 0 ),
    _animationOn( false ),
    _increasingFlags( 0XFF'FF ),
    _multOrder( PRH )
{
}

DOFTransform::DOFTransform( const DOFTransform& dof,
                            const osg::CopyOp&  copyop ) :
    Inherit( dof,
             copyop ),
    _previousTraversalNumber( dof._previousTraversalNumber ),
    _previousTime( dof._previousTime ),
    _minHPR( dof._minHPR ),
    _maxHPR( dof._maxHPR ),
    _currentHPR( dof._currentHPR ),
    _incrementHPR( dof._incrementHPR ),
    _minTranslate( dof._minTranslate ),
    _maxTranslate( dof._maxTranslate ),
    _currentTranslate( dof._currentTranslate ),
    _incrementTranslate( dof._incrementTranslate ),
    _minScale( dof._minScale ),
    _maxScale( dof._maxScale ),
    _currentScale( dof._currentScale ),
    _incrementScale( dof._incrementScale ),
    _Put( dof._Put ),
    _inversePut( dof._inversePut ),
    _limitationFlags( dof._limitationFlags ),
    _animationOn( dof._animationOn ),
    _increasingFlags( dof._increasingFlags ),
    _multOrder( dof._multOrder )
{
    if( _animationOn )
    {
        setNumChildrenRequiringUpdateTraversal(
            getNumChildrenRequiringUpdateTraversal() + 1
        );
    }
}

void
DOFTransform::traverse( osg::NodeVisitor& nv )
{
    if( nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
    {
        // ensure that we do not operate on this node more than
        // once during this traversal.  This is an issue since node
        // can be shared between multiple parents.
        if( ( nv.getTraversalNumber() != _previousTraversalNumber ) &&
            nv.getFrameStamp() )
        {
            double newTime = nv.getFrameStamp()->getSimulationTime();

            animate( ( float )( newTime - _previousTime ) );

            _previousTraversalNumber = nv.getTraversalNumber();
            _previousTime            = newTime;
        }
    }

    Transform::traverse( nv );
}

bool
DOFTransform::computeLocalToWorldMatrix( osg::dmat4& matrix,
                                         osg::NodeVisitor* ) const
{
    // put the PUT matrix first:
    osg::dmat4 l2w( getPutMatrix() );

    // now the current matrix:
    osg::dmat4 current = osg::translate( osg::dvec3( getCurrentTranslate() ) );

    // now create the local rotation:
    if( _multOrder == PRH )
    {
        current = current * osg::rotate( ( double )getCurrentHPR()[1],
                                         osg::dvec3( 1.0, 0.0, 0.0 ) );    // pitch
        current = current * osg::rotate( ( double )getCurrentHPR()[2],
                                         osg::dvec3( 0.0, 1.0, 0.0 ) );    // roll
        current = current * osg::rotate( ( double )getCurrentHPR()[0],
                                         osg::dvec3( 0.0, 0.0, 1.0 ) );    // heading
    }
    else if( _multOrder == PHR )
    {
        current = current * osg::rotate( ( double )getCurrentHPR()[1],
                                         osg::dvec3( 1.0, 0.0, 0.0 ) );    // pitch
        current = current * osg::rotate( ( double )getCurrentHPR()[0],
                                         osg::dvec3( 0.0, 0.0, 1.0 ) );    // heading
        current = current * osg::rotate( ( double )getCurrentHPR()[2],
                                         osg::dvec3( 0.0, 1.0, 0.0 ) );    // roll
    }
    else if( _multOrder == HPR )
    {
        current = current * osg::rotate( ( double )getCurrentHPR()[0],
                                         osg::dvec3( 0.0, 0.0, 1.0 ) );    // heading
        current = current * osg::rotate( ( double )getCurrentHPR()[1],
                                         osg::dvec3( 1.0, 0.0, 0.0 ) );    // pitch
        current = current * osg::rotate( ( double )getCurrentHPR()[2],
                                         osg::dvec3( 0.0, 1.0, 0.0 ) );    // roll
    }
    else if( _multOrder == HRP )
    {
        current = current * osg::rotate( ( double )getCurrentHPR()[0],
                                         osg::dvec3( 0.0, 0.0, 1.0 ) );    // heading
        current = current * osg::rotate( ( double )getCurrentHPR()[2],
                                         osg::dvec3( 0.0, 1.0, 0.0 ) );    // roll
        current = current * osg::rotate( ( double )getCurrentHPR()[1],
                                         osg::dvec3( 1.0, 0.0, 0.0 ) );    // pitch
    }
    else if( _multOrder == RHP )
    {
        current = current * osg::rotate( ( double )getCurrentHPR()[2],
                                         osg::dvec3( 0.0, 1.0, 0.0 ) );    // roll
        current = current * osg::rotate( ( double )getCurrentHPR()[0],
                                         osg::dvec3( 0.0, 0.0, 1.0 ) );    // heading
        current = current * osg::rotate( ( double )getCurrentHPR()[1],
                                         osg::dvec3( 1.0, 0.0, 0.0 ) );    // pitch
    }
    else    // _multOrder == RPH
    {
        current = current * osg::rotate( ( double )getCurrentHPR()[2],
                                         osg::dvec3( 0.0, 1.0, 0.0 ) );    // roll
        current = current * osg::rotate( ( double )getCurrentHPR()[1],
                                         osg::dvec3( 1.0, 0.0, 0.0 ) );    // pitch
        current = current * osg::rotate( ( double )getCurrentHPR()[0],
                                         osg::dvec3( 0.0, 0.0, 1.0 ) );    // heading
    }

    // and scale:
    current = current * osg::scale( osg::dvec3( getCurrentScale() ) );

    l2w     = current * l2w;

    // and impose inverse put:
    l2w = getInversePutMatrix() * l2w;

    // finally.
    if( _referenceFrame == RELATIVE_RF )
    {
        matrix = matrix * l2w;
    }
    else
    {
        matrix = l2w;
    }

    return true;
}

bool
DOFTransform::computeWorldToLocalMatrix( osg::dmat4& matrix,
                                         osg::NodeVisitor* ) const
{
    // put the PUT matrix first:
    osg::dmat4 w2l( getInversePutMatrix() );

    // now the current matrix:
    osg::dmat4 current = osg::translate( osg::dvec3( -getCurrentTranslate() ) );

    // now create the local rotation:
    if( _multOrder == PRH )
    {
        current =
            osg::rotate( ( double )-getCurrentHPR()[0], osg::dvec3( 0.0, 0.0, 1.0 ) ) *
            current;    // heading
        current =
            osg::rotate( ( double )-getCurrentHPR()[2], osg::dvec3( 0.0, 1.0, 0.0 ) ) *
            current;    // roll
        current =
            osg::rotate( ( double )-getCurrentHPR()[1], osg::dvec3( 1.0, 0.0, 0.0 ) ) *
            current;    // pitch
    }
    else if( _multOrder == PHR )
    {
        current =
            osg::rotate( ( double )-getCurrentHPR()[2], osg::dvec3( 0.0, 1.0, 0.0 ) ) *
            current;    // roll
        current =
            osg::rotate( ( double )-getCurrentHPR()[0], osg::dvec3( 0.0, 0.0, 1.0 ) ) *
            current;    // heading
        current =
            osg::rotate( ( double )-getCurrentHPR()[1], osg::dvec3( 1.0, 0.0, 0.0 ) ) *
            current;    // pitch
    }
    else if( _multOrder == HPR )
    {
        current =
            osg::rotate( ( double )-getCurrentHPR()[2], osg::dvec3( 0.0, 1.0, 0.0 ) ) *
            current;    // roll
        current =
            osg::rotate( ( double )-getCurrentHPR()[1], osg::dvec3( 1.0, 0.0, 0.0 ) ) *
            current;    // pitch
        current =
            osg::rotate( ( double )-getCurrentHPR()[0], osg::dvec3( 0.0, 0.0, 1.0 ) ) *
            current;    // heading
    }
    else if( _multOrder == HRP )
    {
        current =
            osg::rotate( ( double )-getCurrentHPR()[1], osg::dvec3( 1.0, 0.0, 0.0 ) ) *
            current;    // pitch
        current =
            osg::rotate( ( double )-getCurrentHPR()[2], osg::dvec3( 0.0, 1.0, 0.0 ) ) *
            current;    // roll
        current =
            osg::rotate( ( double )-getCurrentHPR()[0], osg::dvec3( 0.0, 0.0, 1.0 ) ) *
            current;    // heading
    }
    else if( _multOrder == RHP )
    {
        current =
            osg::rotate( ( double )-getCurrentHPR()[1], osg::dvec3( 1.0, 0.0, 0.0 ) ) *
            current;    // pitch
        current =
            osg::rotate( ( double )-getCurrentHPR()[0], osg::dvec3( 0.0, 0.0, 1.0 ) ) *
            current;    // heading
        current =
            osg::rotate( ( double )-getCurrentHPR()[2], osg::dvec3( 0.0, 1.0, 0.0 ) ) *
            current;    // roll
    }
    else                // _multOrder == MultOrder::RPH
    {
        current =
            osg::rotate( ( double )-getCurrentHPR()[0], osg::dvec3( 0.0, 0.0, 1.0 ) ) *
            current;    // heading
        current =
            osg::rotate( ( double )-getCurrentHPR()[1], osg::dvec3( 1.0, 0.0, 0.0 ) ) *
            current;    // pitch
        current =
            osg::rotate( ( double )-getCurrentHPR()[2], osg::dvec3( 0.0, 1.0, 0.0 ) ) *
            current;    // roll
    }

    // and scale:
    current = osg::scale( osg::dvec3( 1. / getCurrentScale()[0],
                                      1. / getCurrentScale()[1],
                                      1. / getCurrentScale()[2] ) ) *
              current;

    w2l     = current * w2l;

    // and impose inverse put:
    w2l = getPutMatrix() * w2l;

    if( _referenceFrame == RELATIVE_RF )
    {
        matrix = w2l * matrix;
    }
    else    // absolute
    {
        matrix = w2l;
    }
    return true;
}

void
DOFTransform::updateCurrentHPR( const osg::vec3& hpr )
{
    // if there is constrain on animation
    if( _limitationFlags & ROTATION_ROLL_LIMIT_BIT )
    {
        // if we have min == max, it is effective constrain, so don't change
        if( _minHPR[2] != _maxHPR[2] )
        {
            _currentHPR[2]           = hpr[2];
            unsigned short this_flag = ( unsigned short )1 << 4;    // roll

            if( _currentHPR[2] < _minHPR[2] )
            {
                _currentHPR[2] = _minHPR[2];
                // force increasing flag to 1
                _increasingFlags |= this_flag;
            }
            else if( _currentHPR[2] > _maxHPR[2] )
            {
                _currentHPR[2] = _maxHPR[2];
                // force increasing flag to 0
                _increasingFlags &= ~this_flag;
            }
        }
    }
    else
    {
        _currentHPR[2] = hpr[2];
    }

    if( _limitationFlags & ROTATION_PITCH_LIMIT_BIT )
    {
        if( _minHPR[1] != _maxHPR[1] )
        {
            _currentHPR[1]           = hpr[1];
            unsigned short this_flag = ( unsigned short )1 << 3;    // pitch

            if( _currentHPR[1] < _minHPR[1] )
            {
                _currentHPR[1]    = _minHPR[1];
                _increasingFlags |= this_flag;
            }
            else if( _currentHPR[1] > _maxHPR[1] )
            {
                _currentHPR[1]    = _maxHPR[1];
                _increasingFlags &= ~this_flag;
            }
        }
    }
    else
    {
        _currentHPR[1] = hpr[1];
    }

    if( _limitationFlags & ROTATION_YAW_LIMIT_BIT )
    {
        if( _minHPR[0] != _maxHPR[0] )
        {
            _currentHPR[0]           = hpr[0];

            unsigned short this_flag = ( unsigned short )1 << 5;    // heading

            if( _currentHPR[0] < _minHPR[0] )
            {
                _currentHPR[0]    = _minHPR[0];
                _increasingFlags |= this_flag;
            }
            else if( _currentHPR[0] > _maxHPR[0] )
            {
                _currentHPR[0]    = _maxHPR[0];
                _increasingFlags &= ~this_flag;
            }
        }
    }
    else
    {
        _currentHPR[0] = hpr[0];
    }

    dirtyBound();
}

void
DOFTransform::updateCurrentTranslate( const osg::vec3& translate )
{
    if( _limitationFlags & TRANSLATION_Z_LIMIT_BIT )
    {
        if( _minTranslate[2] != _maxTranslate[2] )
        {
            _currentTranslate[2]     = translate[2];
            unsigned short this_flag = ( unsigned short )1 << 2;

            if( _currentTranslate[2] < _minTranslate[2] )
            {
                _currentTranslate[2]  = _minTranslate[2];
                _increasingFlags     |= this_flag;
            }
            else if( _currentTranslate[2] > _maxTranslate[2] )
            {
                _currentTranslate[2]  = _maxTranslate[2];
                _increasingFlags     &= ~this_flag;
            }
        }
    }
    else
    {
        _currentTranslate[2] = translate[2];
    }

    if( _limitationFlags & TRANSLATION_Y_LIMIT_BIT )
    {
        if( _minTranslate[1] != _maxTranslate[1] )
        {
            _currentTranslate[1]     = translate[1];
            unsigned short this_flag = ( unsigned short )1 << 1;

            if( _currentTranslate[1] < _minTranslate[1] )
            {
                _currentTranslate[1]  = _minTranslate[1];
                _increasingFlags     |= this_flag;
            }
            else if( _currentTranslate[1] > _maxTranslate[1] )
            {
                _currentTranslate[1]  = _maxTranslate[1];
                _increasingFlags     &= ~this_flag;
            }
        }
    }
    else
    {
        _currentTranslate[1] = translate[1];
    }

    if( _limitationFlags & TRANSLATION_X_LIMIT_BIT )
    {
        if( _minTranslate[0] != _maxTranslate[0] )
        {
            _currentTranslate[0]     = translate[0];
            unsigned short this_flag = ( unsigned short )1;

            if( _currentTranslate[0] < _minTranslate[0] )
            {
                _currentTranslate[0]  = _minTranslate[0];
                _increasingFlags     |= this_flag;
            }
            else if( _currentTranslate[0] > _maxTranslate[0] )
            {
                _currentTranslate[0]  = _maxTranslate[0];
                _increasingFlags     &= ~this_flag;
            }
        }
    }
    else
    {
        _currentTranslate[0] = translate[0];
    }

    dirtyBound();
}

void
DOFTransform::updateCurrentScale( const osg::vec3& scale )
{
    if( _limitationFlags & SCALE_Z_LIMIT_BIT )
    {
        if( _minScale[2] != _maxScale[2] )
        {
            _currentScale[2]         = scale[2];
            unsigned short this_flag = ( unsigned short )1 << 8;

            if( _currentScale[2] < _minScale[2] )
            {
                _currentScale[2]  = _minScale[2];
                _increasingFlags |= this_flag;
            }
            else if( _currentScale[2] > _maxScale[2] )
            {
                _currentScale[2]  = _maxScale[2];
                _increasingFlags &= ~this_flag;
            }
        }
    }
    else
    {
        _currentScale[2] = scale[2];
    }

    if( _limitationFlags & SCALE_Y_LIMIT_BIT )
    {
        if( _minScale[1] != _maxScale[1] )
        {
            _currentScale[1]         = scale[1];
            unsigned short this_flag = ( unsigned short )1 << 7;

            if( _currentScale[1] < _minScale[1] )
            {
                _currentScale[1]  = _minScale[1];
                _increasingFlags |= this_flag;
            }
            else if( _currentScale[1] > _maxScale[1] )
            {
                _currentScale[1]  = _maxScale[1];
                _increasingFlags &= ~this_flag;
            }
        }
    }
    else
    {
        _currentScale[1] = scale[1];
    }

    if( _limitationFlags & SCALE_X_LIMIT_BIT )
    {
        if( _minScale[0] != _maxScale[0] )
        {
            _currentScale[0]         = scale[0];
            unsigned short this_flag = ( unsigned short )1 << 6;

            if( _currentScale[0] < _minScale[0] )
            {
                _currentScale[0]  = _minScale[0];
                _increasingFlags |= this_flag;
            }
            else if( _currentScale[0] > _maxScale[0] )
            {
                _currentScale[0]  = _maxScale[0];
                _increasingFlags &= ~this_flag;
            }
        }
    }
    else
    {
        _currentScale[0] = scale[0];
    }

    dirtyBound();
}

void
DOFTransform::setAnimationOn( bool do_animate )
{
    if( _animationOn == do_animate )
    {
        return;
    }

    int delta = 0;

    if( _animationOn )
    {
        --delta;
    }
    if( do_animate )
    {
        ++delta;
    }

    _animationOn = do_animate;

    if( _animationOn )
    {
        setNumChildrenRequiringUpdateTraversal( static_cast<unsigned int>(
            static_cast<int>( getNumChildrenRequiringUpdateTraversal() ) + delta
        ) );
    }
}

void
DOFTransform::animate( float deltaTime )
{
    if( !_animationOn )
    {
        return;
    }
    // this will increment or decrement all allowed values

    osg::vec3 new_value = _currentTranslate;

    if( _increasingFlags & 1 )
    {
        new_value[0] += _incrementTranslate[0] * deltaTime;
    }
    else
    {
        new_value[0] -= _incrementTranslate[0] * deltaTime;
    }

    if( _increasingFlags & 1 << 1 )
    {
        new_value[1] += _incrementTranslate[1] * deltaTime;
    }
    else
    {
        new_value[1] -= _incrementTranslate[1] * deltaTime;
    }

    if( _increasingFlags & 1 << 2 )
    {
        new_value[2] += _incrementTranslate[2] * deltaTime;
    }
    else
    {
        new_value[2] -= _incrementTranslate[2] * deltaTime;
    }

    updateCurrentTranslate( new_value );

    new_value = _currentHPR;

    if( _increasingFlags & ( ( unsigned short )1 << 3 ) )
    {
        new_value[1] += _incrementHPR[1] * deltaTime;
    }
    else
    {
        new_value[1] -= _incrementHPR[1] * deltaTime;
    }

    if( _increasingFlags & ( ( unsigned short )1 << 4 ) )
    {
        new_value[2] += _incrementHPR[2] * deltaTime;
    }
    else
    {
        new_value[2] -= _incrementHPR[2] * deltaTime;
    }

    if( _increasingFlags & ( ( unsigned short )1 << 5 ) )
    {
        new_value[0] += _incrementHPR[0] * deltaTime;
    }
    else
    {
        new_value[0] -= _incrementHPR[0] * deltaTime;
    }

    updateCurrentHPR( new_value );

    new_value = _currentScale;

    if( _increasingFlags & 1 << 6 )
    {
        new_value[0] += _incrementScale[0] * deltaTime;
    }
    else
    {
        new_value[0] -= _incrementScale[0] * deltaTime;
    }

    if( _increasingFlags & 1 << 7 )
    {
        new_value[1] += _incrementScale[1] * deltaTime;
    }
    else
    {
        new_value[1] -= _incrementScale[1] * deltaTime;
    }

    if( _increasingFlags & 1 << 8 )
    {
        new_value[2] += _incrementScale[2] * deltaTime;
    }
    else
    {
        new_value[2] -= _incrementScale[2] * deltaTime;
    }

    updateCurrentScale( new_value );
}
