/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Compensates for non-uniform scale in parent transforms.
 * Prevents draggers from appearing distorted.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/AntiSquish>

#include <osg/maths/compat.hpp>

using namespace osgManipulator;

AntiSquish::AntiSquish() :
    _usePivot( true ),
    _usePosition( false ),
    _cacheDirty( true )
{
}

AntiSquish::AntiSquish( const osg::dvec3& pivot ) :
    _pivot( pivot ),
    _usePivot( true ),
    _usePosition( false ),
    _cacheDirty( true )
{
}

AntiSquish::AntiSquish( const osg::dvec3& pivot,
                        const osg::dvec3& pos ) :
    _pivot( pivot ),
    _usePivot( true ),
    _position( pos ),
    _usePosition( true ),
    _cacheDirty( true )
{
}

AntiSquish::AntiSquish( const AntiSquish&  pat,
                        const osg::CopyOp& copyop ) :
    Transform( pat,
               copyop ),
    _pivot( pat._pivot ),
    _usePivot( pat._usePivot ),
    _position( pat._position ),
    _usePosition( pat._usePosition ),
    _cacheDirty( pat._cacheDirty ),
    _cacheLocalToWorld( pat._cacheLocalToWorld ),
    _cache( pat._cache )
{
}

AntiSquish::~AntiSquish()
{
}

bool
AntiSquish::computeLocalToWorldMatrix( osg::dmat4& matrix,
                                       osg::NodeVisitor* /*nv*/ ) const
{
    osg::dmat4 unsquishedMatrix;
    if( !computeUnSquishedMatrix( unsquishedMatrix ) )
    {
        return false;
    }

    if( _referenceFrame == RELATIVE_RF )
    {
        osg::preMult( matrix, unsquishedMatrix );
    }
    else    // absolute
    {
        matrix = unsquishedMatrix;
    }

    return true;
}

bool
AntiSquish::computeWorldToLocalMatrix( osg::dmat4& matrix,
                                       osg::NodeVisitor* ) const
{
    osg::dmat4 unsquishedMatrix;
    if( !computeUnSquishedMatrix( unsquishedMatrix ) )
    {
        return false;
    }

    osg::dmat4 inverse = osg::inverse( unsquishedMatrix );

    if( _referenceFrame == RELATIVE_RF )
    {
        osg::postMult( matrix, inverse );
    }
    else    // absolute
    {
        matrix = inverse;
    }
    return true;
}

bool
AntiSquish::computeUnSquishedMatrix( osg::dmat4& unsquished ) const
{
    std::lock_guard<std::mutex> lock( _cacheLock );

    osg::NodePathList           nodePaths = getParentalNodePaths();
    if( nodePaths.empty() )
    {
        return false;
    }

    osg::NodePath np = nodePaths.front();
    if( np.empty() )
    {
        return false;
    }

    // Remove the last node which is the anti squish node itself.
    np.pop_back();

    // Get the accumulated modeling matrix.
    const osg::dmat4 localToWorld = osg::computeLocalToWorld( np );

    // reuse cached value
    if( !_cacheDirty && _cacheLocalToWorld == localToWorld )
    {
        unsquished = _cache;
        return true;
    }

    osg::dvec3 t, s;
    osg::dquat r, so;

    osg::decompose( localToWorld, t, r, s, so );

    // Let's take an average of the scale.
    double av = ( s[0] + s[1] + s[2] ) / 3.0;
    s[0]      = av;
    s[1]      = av;
    s[2]      = av;

    if( av == 0 )
    {
        return false;
    }

    //
    // Final dmat4: [-Pivot][SO]^[S][SO][R][T][Pivot][LOCALTOWORLD]^[position]
    // OR [SO]^[S][SO][R][T][LOCALTOWORLD]^
    //
    if( _usePivot )
    {
        osg::postMultTranslate( unsquished, -_pivot );

        osg::dmat4 tmps = osg::rotate( so );
        osg::dmat4 invtmps;
        bool       invertible = true;
        double     det        = osg::determinant( tmps );
        if( std::abs( det ) < 1E-10 )
        {
            invertible = false;
        }
        else
        {
            invtmps = osg::inverse( tmps );
        }
        if( !invertible )
        {
            return false;
        }

        // SO^
        osg::postMult( unsquished, invtmps );
        // S
        osg::postMultScale( unsquished, s );
        // SO
        osg::postMult( unsquished, tmps );
        // R
        osg::postMultRotate( unsquished, r );
        // T
        osg::postMultTranslate( unsquished, t );

        osg::dmat4 invltw = osg::inverse( localToWorld );

        // LTW^
        osg::postMult( unsquished, invltw );

        // Position
        if( _usePosition )
        {
            osg::postMultTranslate( unsquished, _position );
        }
        else
        {
            osg::postMultTranslate( unsquished, _pivot );
        }
    }
    else
    {
        osg::dmat4 tmps = osg::rotate( so );
        osg::dmat4 invtmps;
        bool       invertible = true;
        double     det        = osg::determinant( tmps );
        if( std::abs( det ) < 1E-10 )
        {
            invertible = false;
        }
        else
        {
            invtmps = osg::inverse( tmps );
        }
        if( !invertible )
        {
            return false;
        }

        osg::postMult( unsquished, invtmps );
        osg::postMultScale( unsquished, s );
        osg::postMult( unsquished, tmps );
        osg::postMultRotate( unsquished, r );
        osg::postMultTranslate( unsquished, t );
        osg::dmat4 invltw = osg::inverse( localToWorld );
        osg::postMult( unsquished, invltw );
    }

    // Check for NaN
    const double* ptr = &unsquished.value[0].x;
    for( int i = 0; i < 16; ++i )
    {
        if( std::isnan( ptr[i] ) )
        {
            return false;
        }
    }

    _cache             = unsquished;
    _cacheLocalToWorld = localToWorld;
    _cacheDirty        = false;

    // As Transform::computeBounde calls us without a node-path it relies on
    // The cache. Hence a new _cache affects the bound.
    const_cast<AntiSquish*>( this )->dirtyBound();

    return true;
}
