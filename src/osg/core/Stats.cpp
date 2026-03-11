/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Named statistics collector. Stores per-frame timing and count
 * values (draw time, GPU time, triangle count) as named attributes.
 */
#include <osg/core/Stats.hpp>

#include <osg/core/Notify.hpp>

using namespace osg;

Stats::Stats( const std::string& name ) :
    _name( name )
{
    allocate( 25 );
}

Stats::Stats( const std::string& name,
              unsigned int       numberOfFrames ) :
    _name( name )
{
    allocate( numberOfFrames );
}

void
Stats::allocate( unsigned int numberOfFrames )
{
    std::lock_guard<std::mutex> lock( _mutex );

    _baseFrameNumber   = 0;
    _latestFrameNumber = 0;
    _attributeMapList.clear();
    _attributeMapList.resize( numberOfFrames );
}

bool
Stats::setAttribute( unsigned int       frameNumber,
                     const std::string& attributeName,
                     double             value )
{
    if( frameNumber < getEarliestFrameNumber() )
    {
        return false;
    }

    std::lock_guard<std::mutex> lock( _mutex );

    if( frameNumber > _latestFrameNumber )
    {
        // need to advance

        // first clear the entries up to and including the new frameNumber
        for( unsigned int i = _latestFrameNumber + 1; i <= frameNumber; ++i )
        {
            unsigned int index = ( i - _baseFrameNumber ) % _attributeMapList.size();
            _attributeMapList[index].clear();
        }

        if( ( frameNumber - _baseFrameNumber ) >=
            static_cast<unsigned int>( _attributeMapList.size() ) )
        {
            _baseFrameNumber =
                static_cast<unsigned int>( ( frameNumber / _attributeMapList.size() ) *
                                           _attributeMapList.size() );
        }

        _latestFrameNumber = frameNumber;
    }

    int index = getIndex( frameNumber );
    if( index < 0 )
    {
        OSG_NOTICE << "Failed to assign valid index for Stats::setAttribute("
                   << frameNumber << "," << attributeName << "," << value << ")"
                   << std::endl;
        return false;
    }

    AttributeMap& attributeMap  = _attributeMapList[static_cast<size_t>( index )];
    attributeMap[attributeName] = value;

    return true;
}

bool
Stats::getAttributeNoMutex( unsigned int       frameNumber,
                            const std::string& attributeName,
                            double&            value ) const
{
    int index = getIndex( frameNumber );
    if( index < 0 )
    {
        return false;
    }

    const AttributeMap& attributeMap = _attributeMapList[static_cast<size_t>( index )];
    AttributeMap::const_iterator itr = attributeMap.find( attributeName );
    if( itr == attributeMap.end() )
    {
        return false;
    }

    value = itr->second;
    return true;
}

bool
Stats::getAveragedAttribute( const std::string& attributeName,
                             double&            value,
                             bool               averageInInverseSpace ) const
{
    return getAveragedAttribute( getEarliestFrameNumber(),
                                 getLatestFrameNumber(),
                                 attributeName,
                                 value,
                                 averageInInverseSpace );
}

bool
Stats::getAveragedAttribute( unsigned int       startFrameNumber,
                             unsigned int       endFrameNumber,
                             const std::string& attributeName,
                             double&            value,
                             bool               averageInInverseSpace ) const
{
    if( endFrameNumber < startFrameNumber )
    {
        std::swap( endFrameNumber, startFrameNumber );
    }

    std::lock_guard<std::mutex> lock( _mutex );

    double                      total           = 0.0;
    double                      numValidSamples = 0.0;
    for( unsigned int i = startFrameNumber; i <= endFrameNumber; ++i )
    {
        double v = 0.0;
        if( getAttributeNoMutex( i, attributeName, v ) )
        {
            if( averageInInverseSpace )
            {
                total += 1.0 / v;
            }
            else
            {
                total += v;
            }
            numValidSamples += 1.0;
        }
    }
    if( numValidSamples > 0.0 )
    {
        if( averageInInverseSpace )
        {
            value = numValidSamples / total;
        }
        else
        {
            value = total / numValidSamples;
        }
        return true;
    }
    else
    {
        return false;
    }
}

Stats::AttributeMap&
Stats::getAttributeMapNoMutex( unsigned int frameNumber )
{
    int index = getIndex( frameNumber );
    if( index < 0 )
    {
        return _invalidAttributeMap;
    }

    return _attributeMapList[static_cast<size_t>( index )];
}

const Stats::AttributeMap&
Stats::getAttributeMapNoMutex( unsigned int frameNumber ) const
{
    int index = getIndex( frameNumber );
    if( index < 0 )
    {
        return _invalidAttributeMap;
    }

    return _attributeMapList[static_cast<size_t>( index )];
}

void
Stats::report( std::ostream& out,
               const char*   indent ) const
{
    std::lock_guard<std::mutex> lock( _mutex );

    if( indent )
    {
        out << indent;
    }
    out << "Stats " << _name << std::endl;
    for( unsigned int i = getEarliestFrameNumber(); i <= getLatestFrameNumber(); ++i )
    {
        out << " FrameNumber " << i << std::endl;
        const osg::Stats::AttributeMap& attributes = getAttributeMapNoMutex( i );
        for( osg::Stats::AttributeMap::const_iterator itr = attributes.begin();
             itr != attributes.end();
             ++itr )
        {
            if( indent )
            {
                out << indent;
            }
            out << "    " << itr->first << "\t" << itr->second << std::endl;
        }
    }
}

void
Stats::report( std::ostream& out,
               unsigned int  frameNumber,
               const char*   indent ) const
{
    std::lock_guard<std::mutex> lock( _mutex );

    if( indent )
    {
        out << indent;
    }
    out << "Stats " << _name << " FrameNumber " << frameNumber << std::endl;
    const osg::Stats::AttributeMap& attributes = getAttributeMapNoMutex( frameNumber );
    for( osg::Stats::AttributeMap::const_iterator itr = attributes.begin();
         itr != attributes.end();
         ++itr )
    {
        if( indent )
        {
            out << indent;
        }
        out << "    " << itr->first << "\t" << itr->second << std::endl;
    }
}
