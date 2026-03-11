/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgthreadedterrain example application
 */
#include <iostream>
#include <osg/core/ArgumentParser.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/threading/Thread.hpp>
#include <osg/traversal/ClusterCullingCallback.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/fstream.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgTerrain/GeometryTechnique>
#include <osgTerrain/Layer>
#include <osgTerrain/TerrainTile>
#include <osgText/FadeText>
#include <osgUtil/utils/IncrementalCompileOperation.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

struct ReleaseBlockOnCompileCompleted
    : public osgUtil::IncrementalCompileOperation::CompileCompletedCallback
{

        ReleaseBlockOnCompileCompleted( osg::RefBlockCount* block ) :
            _block( block )
        {
        }

        virtual bool
        compileCompleted( osgUtil::IncrementalCompileOperation::CompileSet* compileSet )
        {
            if( _block.valid() )
            {
                _block->completed();
            }

            // tell IncrementalCompileOperation that it's now safe to remove the
            // compileSet

            osg::notify( osg::NOTICE )
                << "compileCompleted(" << compileSet << ")" << std::endl;

            return true;
        }

        osg::ref_ptr<osg::RefBlockCount> _block;
};

class LoadAndCompileOperation : public osg::Operation
{
    public:

        LoadAndCompileOperation( const std::string&                    filename,
                                 osgUtil::IncrementalCompileOperation* ico,
                                 osg::RefBlockCount*                   block ) :
            osg::Referenced( true ),
            Operation( "Load and compile Operation",
                       false ),
            _filename( filename ),
            _incrementalCompileOperation( ico ),
            _block( block )
        {
        }

        virtual void
        operator()( osg::Object* /*object*/ )
        {
            // osg::notify(osg::NOTICE)<<"LoadAndCompileOperation
            // "<<_filename<<std::endl;

            _loadedModel = osgDB::readRefNodeFile( _filename );

            if( _loadedModel.valid() && _incrementalCompileOperation.valid() )
            {
                osg::ref_ptr<osgUtil::IncrementalCompileOperation::CompileSet>
                    compileSet = new osgUtil::IncrementalCompileOperation::CompileSet(
                        _loadedModel.get()
                    );

                compileSet->_compileCompletedCallback =
                    new ReleaseBlockOnCompileCompleted( _block.get() );

                _incrementalCompileOperation->add( compileSet.get() );
            }
            else
            {
                if( _block.valid() )
                {
                    _block->completed();
                }
            }

            // osg::notify(osg::NOTICE)<<"done LoadAndCompileOperation
            // "<<_filename<<std::endl;
        }

        std::string                                        _filename;
        osg::ref_ptr<osg::Node>                            _loadedModel;
        osg::ref_ptr<osgUtil::IncrementalCompileOperation> _incrementalCompileOperation;
        osg::ref_ptr<osg::RefBlockCount>                   _block;
};

class MasterOperation : public osg::Operation
{
    public:

        typedef std::set<std::string>                          Files;
        typedef std::map<std::string, osg::ref_ptr<osg::Node>> FilenameNodeMap;
        typedef std::vector<osg::ref_ptr<osg::Node>>           Nodes;

        MasterOperation( const std::string&                    filename,
                         osgUtil::IncrementalCompileOperation* ico ) :
            osg::Referenced( true ),
            Operation( "Master reading operation",
                       true ),
            _filename( filename ),
            _incrementalCompileOperation( ico )
        {
        }

        /** Set the OperationQueue that the MasterOperation can use to place tasks like
         * file loading on for other processes to handle.*/
        void
        setOperationQueue( osg::OperationQueue* oq )
        {
            _operationQueue = oq;
        }

        osg::OperationQueue*
        getOperationQueue()
        {
            return _operationQueue.get();
        }

        bool
        readMasterFile( Files& files ) const
        {
            osgDB::ifstream fin( _filename.c_str() );
            if( fin )
            {
                osgDB::Input fr;
                fr.attach( &fin );

                bool readFilename = false;

                while( !fr.eof() )
                {
                    bool itrAdvanced = false;
                    if( fr.matchSequence( "file %s" ) || fr.matchSequence( "file %w" ) )
                    {
                        files.insert( fr[1].getStr() );
                        fr           += 2;
                        itrAdvanced   = true;
                        readFilename  = true;
                    }

                    if( !itrAdvanced )
                    {
                        ++fr;
                    }
                }

                return readFilename;
            }
            return false;
        }

        bool
        open( osg::Group* group )
        {
            Files files;
            readMasterFile( files );
            for( Files::iterator itr = files.begin(); itr != files.end(); ++itr )
            {
                osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile( *itr );
                if( model )
                {
                    osg::notify( osg::NOTICE )
                        << "open: Loaded file " << *itr << std::endl;
                    group->addChild( model );
                    _existingFilenameNodeMap[*itr] = model;
                }
            }

            return true;
        }

        virtual void
        operator()( osg::Object* callingObject )
        {
            // decided which method to call according to whole has called me.
            osgViewer::Viewer* viewer =
                dynamic_cast<osgViewer::Viewer*>( callingObject );

            if( viewer )
            {
                update( viewer->getSceneData() );
            }
            else
            {
                load();
            }
        }

        void
        load()
        {
            // osg::notify(osg::NOTICE)<<"void load(Object)"<<std::endl;
            Files filesA;
            Files filesB;

            readMasterFile( filesB );
            // osg::notify(osg::NOTICE)<<"First read "<<filesA.size()<<std::endl;

            // itererate until the master file is stable
            do
            {
                osg::Thread::microSleep( 100'000 );

                filesB.swap( filesA );
                filesB.clear();
                readMasterFile( filesB );

                // osg::notify(osg::NOTICE)<<"second read "<<filesB.size()<<std::endl;

            } while( filesA != filesB );

            Files files;
            files.swap( filesB );

            // osg::notify(osg::NOTICE)<<"Now equal "<<files.size()<<std::endl;

            Files newFiles;
            Files removedFiles;

            // find out which files are new, and which ones have been removed.
            {
                std::lock_guard<std::mutex> lock( _mutex );

                for( Files::iterator fitr = files.begin(); fitr != files.end(); ++fitr )
                {
                    if( _existingFilenameNodeMap.count( *fitr ) == 0 )
                    {
                        newFiles.insert( *fitr );
                    }
                }

                for( FilenameNodeMap::iterator litr = _existingFilenameNodeMap.begin();
                     litr != _existingFilenameNodeMap.end();
                     ++litr )
                {
                    if( files.count( litr->first ) == 0 )
                    {
                        removedFiles.insert( litr->first );
                    }
                }
            }

#if 0
        if (!newFiles.empty() || !removedFiles.empty())
        {
            osg::notify(osg::NOTICE)<<std::endl<<std::endl<<"void operator () files.size()="<<files.size()<<std::endl;
        }
#endif

            // first load the new files.
            FilenameNodeMap nodesToAdd;
            if( !newFiles.empty() )
            {

                typedef std::vector<osg::ref_ptr<osg::GraphicsThread>> GraphicsThreads;
                GraphicsThreads                                        threads;

                for( unsigned int i = 0; i <= osg::GraphicsContext::getMaxContextID();
                     ++i )
                {
                    osg::GraphicsContext* gc =
                        osg::GraphicsContext::getCompileContext( i );
                    osg::GraphicsThread* gt = gc ? gc->getGraphicsThread() : 0;
                    if( gt )
                    {
                        threads.push_back( gt );
                    }
                }

                if( _operationQueue.valid() )
                {
                    // osg::notify(osg::NOTICE)<<"Using OperationQueue"<<std::endl;

                    _endOfLoadBlock = new osg::RefBlockCount( newFiles.size() );

                    _endOfLoadBlock->reset();

                    typedef std::list<osg::ref_ptr<LoadAndCompileOperation>>
                                       LoadAndCompileList;
                    LoadAndCompileList loadAndCompileList;

                    for( Files::iterator nitr = newFiles.begin(); nitr != newFiles.end();
                         ++nitr )
                    {
                        // osg::notify(osg::NOTICE)<<"Adding LoadAndCompileOperation
                        // "<<*nitr<<std::endl;

                        osg::ref_ptr<LoadAndCompileOperation> loadAndCompile =
                            new LoadAndCompileOperation(
                                *nitr,
                                _incrementalCompileOperation.get(),
                                _endOfLoadBlock.get()
                            );
                        loadAndCompileList.push_back( loadAndCompile );
                        _operationQueue->add( loadAndCompile.get() );
                    }

#if 1
                    osg::ref_ptr<osg::Operation> operation;
                    while( ( operation = _operationQueue->getNextOperation() ).valid() )
                    {
                        // osg::notify(osg::NOTICE)<<"Local running of
                        // operation"<<std::endl;
                        ( *operation )( 0 );
                    }
#endif
                    // osg::notify(osg::NOTICE)<<"Waiting for completion of
                    // LoadAndCompile operations"<<std::endl;
                    _endOfLoadBlock->block();
                    // osg::notify(osg::NOTICE)<<"done ... Waiting for completion of
                    // LoadAndCompile operations"<<std::endl;

                    for( LoadAndCompileList::iterator litr = loadAndCompileList.begin();
                         litr != loadAndCompileList.end();
                         ++litr )
                    {
                        if( ( *litr )->_loadedModel.valid() )
                        {
                            nodesToAdd[( *litr )->_filename] = ( *litr )->_loadedModel;
                        }
                    }
                }

                else
                {

                    _endOfLoadBlock = new osg::RefBlockCount( newFiles.size() );

                    _endOfLoadBlock->reset();

                    for( Files::iterator nitr = newFiles.begin(); nitr != newFiles.end();
                         ++nitr )
                    {
                        osg::ref_ptr<osg::Node> loadedModel =
                            osgDB::readRefNodeFile( *nitr );

                        if( loadedModel.get() )
                        {
                            nodesToAdd[*nitr] = loadedModel;

                            if( _incrementalCompileOperation.valid() )
                            {
                                osg::ref_ptr<
                                    osgUtil::IncrementalCompileOperation::CompileSet>
                                    compileSet =
                                        new osgUtil::IncrementalCompileOperation::
                                            CompileSet( loadedModel.get() );

                                compileSet->_compileCompletedCallback =
                                    new ReleaseBlockOnCompileCompleted(
                                        _endOfLoadBlock.get()
                                    );

                                _incrementalCompileOperation->add( compileSet.get() );
                            }
                            else
                            {
                                _endOfLoadBlock->completed();
                            }
                        }
                        else
                        {
                            _endOfLoadBlock->completed();
                        }
                    }

                    _endOfLoadBlock->block();
                }
            }

            bool requiresBlock = false;

            // pass the locally peppared data to MasterOperations shared data
            // so that updated thread can merge these changes with the main scene
            // graph.  This merge is carried out via the update(..) method.
            if( !removedFiles.empty() || !nodesToAdd.empty() )
            {
                std::lock_guard<std::mutex> lock( _mutex );
                _nodesToRemove.swap( removedFiles );
                _nodesToAdd.swap( nodesToAdd );
                requiresBlock = true;
            }

            // now block so we don't try to load anything till the new data has been
            // merged otherwise _existingFilenameNodeMap will get out of sync.
            if( requiresBlock )
            {
                _updatesMergedBlock.block();
            }
            else
            {
                std::this_thread::yield();
            }
        }

        // merge the changes with the main scene graph.
        void
        update( osg::Node* scene )
        {
            // osg::notify(osg::NOTICE)<<"void update(Node*)"<<std::endl;

            osg::Group* group = dynamic_cast<osg::Group*>( scene );
            if( !group )
            {
                osg::notify( osg::NOTICE )
                    << "Error, MasterOperation::update(Node*) can only work with a "
                       "Group as Viewer::getSceneData()."
                    << std::endl;
                return;
            }

            std::lock_guard<std::mutex> lock( _mutex );

            if( !_nodesToRemove.empty() || !_nodesToAdd.empty() )
            {
                osg::notify( osg::NOTICE ) << "update().................. " << std::endl;
            }

            if( !_nodesToRemove.empty() )
            {
                for( Files::iterator itr = _nodesToRemove.begin();
                     itr != _nodesToRemove.end();
                     ++itr )
                {
                    FilenameNodeMap::iterator fnmItr =
                        _existingFilenameNodeMap.find( *itr );
                    if( fnmItr != _existingFilenameNodeMap.end() )
                    {
                        osg::notify( osg::NOTICE )
                            << "  update():removing " << *itr << std::endl;

                        group->removeChild( fnmItr->second.get() );
                        _existingFilenameNodeMap.erase( fnmItr );
                    }
                }

                _nodesToRemove.clear();
            }

            if( !_nodesToAdd.empty() )
            {
                for( FilenameNodeMap::iterator itr = _nodesToAdd.begin();
                     itr != _nodesToAdd.end();
                     ++itr )
                {
                    osg::notify( osg::NOTICE )
                        << "  update():inserting " << itr->first << std::endl;
                    group->addChild( itr->second.get() );
                    _existingFilenameNodeMap[itr->first] = itr->second;
                }

                _nodesToAdd.clear();
            }

            _updatesMergedBlock.release();
        }

        // add release implementation so that any thread cancellation can
        // work even when blocks and barriers are used.
        virtual void
        release()
        {
            if( _operationQueue.valid() )
            {
                _operationQueue->removeAllOperations();
            }

            _updatesMergedBlock.release();
            if( _endOfCompilebarrier.valid() )
            {
                _endOfCompilebarrier.release();
            }
            if( _endOfLoadBlock.valid() )
            {
                _endOfLoadBlock.release();
            }
        }

        std::string                                        _filename;

        std::mutex                                         _mutex;
        FilenameNodeMap                                    _existingFilenameNodeMap;
        Files                                              _nodesToRemove;
        FilenameNodeMap                                    _nodesToAdd;
        osg::Block                                         _updatesMergedBlock;

        osg::ref_ptr<osgUtil::IncrementalCompileOperation> _incrementalCompileOperation;
        osg::ref_ptr<osg::BarrierOperation>                _endOfCompilebarrier;
        osg::ref_ptr<osg::RefBlockCount>                   _endOfLoadBlock;

        osg::ref_ptr<osg::OperationQueue>                  _operationQueue;
};

class FilterHandler : public osgGA::GUIEventHandler
{
    public:

        FilterHandler( osgTerrain::GeometryTechnique* gt ) :
            _gt( gt )
        {
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& /*aa*/ )
        {
            if( !_gt )
            {
                return false;
            }

            switch( ea.getEventType() )
            {
                case( osgGA::GUIEventAdapter::KEYDOWN ) :
                    {
                        if( ea.getKey() == 'g' )
                        {
                            osg::notify( osg::NOTICE ) << "Gaussian" << std::endl;
                            _gt->setFilterMatrixAs(
                                osgTerrain::GeometryTechnique::GAUSSIAN
                            );
                            return true;
                        }
                        else if( ea.getKey() == 's' )
                        {
                            osg::notify( osg::NOTICE ) << "Smooth" << std::endl;
                            _gt->setFilterMatrixAs(
                                osgTerrain::GeometryTechnique::SMOOTH
                            );
                            return true;
                        }
                        else if( ea.getKey() == 'S' )
                        {
                            osg::notify( osg::NOTICE ) << "Sharpen" << std::endl;
                            _gt->setFilterMatrixAs(
                                osgTerrain::GeometryTechnique::SHARPEN
                            );
                            return true;
                        }
                        else if( ea.getKey() == '+' )
                        {
                            _gt->setFilterWidth( _gt->getFilterWidth() * 1.1 );
                            osg::notify( osg::NOTICE )
                                << "Filter width = " << _gt->getFilterWidth()
                                << std::endl;
                            return true;
                        }
                        else if( ea.getKey() == '-' )
                        {
                            _gt->setFilterWidth( _gt->getFilterWidth() / 1.1 );
                            osg::notify( osg::NOTICE )
                                << "Filter width = " << _gt->getFilterWidth()
                                << std::endl;
                            return true;
                        }
                        else if( ea.getKey() == '>' )
                        {
                            _gt->setFilterBias( _gt->getFilterBias() + 0.1 );
                            osg::notify( osg::NOTICE )
                                << "Filter bias = " << _gt->getFilterBias() << std::endl;
                            return true;
                        }
                        else if( ea.getKey() == '<' )
                        {
                            _gt->setFilterBias( _gt->getFilterBias() - 0.1 );
                            osg::notify( osg::NOTICE )
                                << "Filter bias = " << _gt->getFilterBias() << std::endl;
                            return true;
                        }
                        break;
                    }
                default :
                    break;
            }
            return false;
        }

    protected:

        osg::observer_ptr<osgTerrain::GeometryTechnique> _gt;
};

class LayerHandler : public osgGA::GUIEventHandler
{
    public:

        LayerHandler( osgTerrain::Layer* layer ) :
            _layer( layer )
        {
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& /*aa*/ )
        {
            if( !_layer )
            {
                return false;
            }

            float scale = 1.2;

            switch( ea.getEventType() )
            {
                case( osgGA::GUIEventAdapter::KEYDOWN ) :
                    {
                        if( ea.getKey() == 'q' )
                        {
                            _layer->transform( 0.0, scale );
                            return true;
                        }
                        else if( ea.getKey() == 'a' )
                        {
                            _layer->transform( 0.0, 1.0F / scale );
                            return true;
                        }
                        break;
                    }
                default :
                    break;
            }
            return false;
        }

    protected:

        osg::observer_ptr<osgTerrain::Layer> _layer;
};

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    double              x              = 0.0;
    double              y              = 0.0;
    double              w              = 1.0;
    double              h              = 1.0;

    unsigned int        numLoadThreads = 1;
    while( arguments.read( "--load-threads", numLoadThreads ) )
    {
    }

    std::string masterFilename;
    while( arguments.read( "-m", masterFilename ) )
    {
    }

    osg::ref_ptr<osgTerrain::TerrainTile> terrainTile = new osgTerrain::TerrainTile;
    osg::ref_ptr<osgTerrain::Locator>     locator     = new osgTerrain::Locator;
    osg::ref_ptr<osgTerrain::ValidDataOperator> validDataOperator =
        new osgTerrain::NoDataValue( 0.0 );
    osg::ref_ptr<osgTerrain::Layer> lastAppliedLayer;

    locator->setCoordinateSystemType( osgTerrain::Locator::GEOCENTRIC );
    locator->setTransformAsExtents( -osg::PI, -osg::PI * 0.5, osg::PI, osg::PI * 0.5 );

    unsigned int             layerNum = 0;

    std::string              filterName;

    osg::Texture::FilterMode filter = osg::Texture::LINEAR;

    float                    minValue, maxValue;
    float                    scale  = 1.0F;
    float                    offset = 0.0F;

    int                      pos    = 1;
    while( pos < arguments.argc() )
    {
        std::string filename;

        if( arguments.read( pos, "--layer", layerNum ) )
        {
            osg::notify( osg::NOTICE )
                << "Set layer number to " << layerNum << std::endl;
        }

        else if( arguments.read( pos, "-b" ) )
        {
            terrainTile->setTreatBoundariesToValidDataAsDefaultValue( true );
        }

        else if( arguments.read( pos, "-e", x, y, w, h ) )
        {
            // define the extents.
            locator->setCoordinateSystemType( osgTerrain::Locator::GEOCENTRIC );
            locator->setTransformAsExtents( x, y, x + w, y + h );
        }

        else if( arguments.read( pos, "--transform", offset, scale ) ||
                 arguments.read( pos, "-t", offset, scale ) )
        {
            // define the extents.
        }

        else if( arguments.read( pos, "--cartesian", x, y, w, h ) )
        {
            // define the extents.
            locator->setCoordinateSystemType( osgTerrain::Locator::PROJECTED );
            locator->setTransformAsExtents( x, y, x + w, y + h );
        }

        else if( arguments.read( pos, "--hf", filename ) )
        {
            osg::notify( osg::NOTICE ) << "--hf " << filename << std::endl;

            osg::ref_ptr<osg::HeightField> hf =
                osgDB::readRefHeightFieldFile( filename );
            if( hf.valid() )
            {
                osg::ref_ptr<osgTerrain::HeightFieldLayer> hfl =
                    new osgTerrain::HeightFieldLayer;
                hfl->setHeightField( hf.get() );

                hfl->setLocator( locator.get() );
                hfl->setValidDataOperator( validDataOperator.get() );
                hfl->setMagFilter( filter );

                if( offset != 0.0F || scale != 1.0F )
                {
                    hfl->transform( offset, scale );
                }

                terrainTile->setElevationLayer( hfl.get() );

                lastAppliedLayer = hfl.get();

                osg::notify( osg::NOTICE )
                    << "created osgTerrain::HeightFieldLayer" << std::endl;
            }
            else
            {
                osg::notify( osg::NOTICE )
                    << "failed to create osgTerrain::HeightFieldLayer" << std::endl;
            }

            scale  = 1.0F;
            offset = 0.0F;
        }

        else if( arguments.read( pos, "-d", filename ) ||
                 arguments.read( pos, "--elevation-image", filename ) )
        {
            osg::notify( osg::NOTICE ) << "--elevation-image " << filename << std::endl;

            osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile( filename );
            if( image.valid() )
            {
                osg::ref_ptr<osgTerrain::ImageLayer> imageLayer =
                    new osgTerrain::ImageLayer;
                imageLayer->setImage( image.get() );
                imageLayer->setLocator( locator.get() );
                imageLayer->setValidDataOperator( validDataOperator.get() );
                imageLayer->setMagFilter( filter );

                if( offset != 0.0F || scale != 1.0F )
                {
                    imageLayer->transform( offset, scale );
                }

                terrainTile->setElevationLayer( imageLayer.get() );

                lastAppliedLayer = imageLayer.get();

                osg::notify( osg::NOTICE )
                    << "created Elevation osgTerrain::ImageLayer" << std::endl;
            }
            else
            {
                osg::notify( osg::NOTICE )
                    << "failed to create osgTerrain::ImageLayer" << std::endl;
            }

            scale  = 1.0F;
            offset = 0.0F;
        }

        else if( arguments.read( pos, "-c", filename ) ||
                 arguments.read( pos, "--image", filename ) )
        {
            osg::notify( osg::NOTICE )
                << "--image " << filename << " x=" << x << " y=" << y << " w=" << w
                << " h=" << h << std::endl;

            osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile( filename );
            if( image.valid() )
            {
                osg::ref_ptr<osgTerrain::ImageLayer> imageLayer =
                    new osgTerrain::ImageLayer;
                imageLayer->setImage( image.get() );
                imageLayer->setLocator( locator.get() );
                imageLayer->setValidDataOperator( validDataOperator.get() );
                imageLayer->setMagFilter( filter );

                if( offset != 0.0F || scale != 1.0F )
                {
                    imageLayer->transform( offset, scale );
                }

                terrainTile->setColorLayer( layerNum, imageLayer.get() );

                lastAppliedLayer = imageLayer.get();

                osg::notify( osg::NOTICE )
                    << "created Color osgTerrain::ImageLayer" << std::endl;
            }
            else
            {
                osg::notify( osg::NOTICE )
                    << "failed to create osgTerrain::ImageLayer" << std::endl;
            }

            scale  = 1.0F;
            offset = 0.0F;
        }

        else if( arguments.read( pos, "--filter", filterName ) )
        {
            if( filterName == "NEAREST" )
            {
                osg::notify( osg::NOTICE ) << "--filter " << filterName << std::endl;
                filter = osg::Texture::NEAREST;
            }
            else if( filterName == "LINEAR" )
            {
                filter = osg::Texture::LINEAR;
                osg::notify( osg::NOTICE ) << "--filter " << filterName << std::endl;
            }
            else
            {
                osg::notify( osg::NOTICE )
                    << "--filter " << filterName
                    << " unrecognized filter name, please use LINEAER or NEAREST."
                    << std::endl;
            }

            if( terrainTile->getColorLayer( layerNum ) )
            {
                terrainTile->getColorLayer( layerNum )->setMagFilter( filter );
            }
        }

        else if( arguments.read( pos, "--tf", minValue, maxValue ) )
        {
            osg::ref_ptr<osg::TransferFunction1D> tf       = new osg::TransferFunction1D;

            unsigned int                          numCells = 6;
            float delta = ( maxValue - minValue ) / float( numCells - 1 );
            float v     = minValue;

            tf->allocate( 6 );
            tf->setColor( v, osg::vec4( 1.0, 1.0, 1.0, 1.0 ) );
            v += delta;
            tf->setColor( v, osg::vec4( 1.0, 0.0, 1.0, 1.0 ) );
            v += delta;
            tf->setColor( v, osg::vec4( 1.0, 0.0, 0.0, 1.0 ) );
            v += delta;
            tf->setColor( v, osg::vec4( 1.0, 1.0, 0.0, 1.0 ) );
            v += delta;
            tf->setColor( v, osg::vec4( 0.0, 1.0, 1.0, 1.0 ) );
            v += delta;
            tf->setColor( v, osg::vec4( 0.0, 1.0, 0.0, 1.0 ) );

            osg::notify( osg::NOTICE )
                << "--tf " << minValue << " " << maxValue << std::endl;

            terrainTile->setColorLayer( layerNum,
                                        new osgTerrain::ContourLayer( tf.get() ) );
        }
        else
        {
            ++pos;
        }
    }

    osg::ref_ptr<osg::Group>                    scene = new osg::Group;

    osg::ref_ptr<osgTerrain::GeometryTechnique> geometryTechnique;
    if( terrainTile.valid() &&
        ( terrainTile->getElevationLayer() || terrainTile->getColorLayer( 0 ) ) )
    {
        osg::notify( osg::NOTICE ) << "Terrain created" << std::endl;

        scene->addChild( terrainTile.get() );

        geometryTechnique = new osgTerrain::GeometryTechnique;
        terrainTile->setTerrainTechnique( geometryTechnique.get() );
    }

    if( scene->getNumChildren() == 0 )
    {
        osg::notify( osg::NOTICE )
            << "No model created, please specify terrain or master file on command line."
            << std::endl;
        return 0;
    }

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    // construct the viewer.
    osgViewer::Viewer viewer( arguments );

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator =
            new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1',
                                                    "Trackball",
                                                    new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2',
                                                    "Flight",
                                                    new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3',
                                                    "Drive",
                                                    new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4',
                                                    "Terrain",
                                                    new osgGA::TerrainManipulator() );

        std::string pathfile;
        char        keyForAnimationPath = '5';
        while( arguments.read( "-p", pathfile ) )
        {
            osgGA::AnimationPathManipulator* apm =
                new osgGA::AnimationPathManipulator( pathfile );
            if( apm || !apm->valid() )
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath,
                                                            "Path",
                                                            apm );
                keyswitchManipulator->selectMatrixManipulator( num );
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    // add the record camera path handler
    viewer.addEventHandler( new osgViewer::RecordCameraPathHandler );

    // attach an IncrementaCompileOperation to allow the master loading
    // to be handled with an incremental compile to avoid frame drops when large objects
    // are added.
    viewer.setIncrementalCompileOperation( new osgUtil::IncrementalCompileOperation() );

    if( geometryTechnique.valid() )
    {
        viewer.addEventHandler( new FilterHandler( geometryTechnique.get() ) );
        viewer.addEventHandler( new LayerHandler( lastAppliedLayer.get() ) );
    }

    osg::ref_ptr<MasterOperation> masterOperation;
    if( !masterFilename.empty() )
    {
        masterOperation = new MasterOperation( masterFilename,
                                               viewer.getIncrementalCompileOperation() );
        osg::notify( osg::NOTICE ) << "Master operation created" << std::endl;
        masterOperation->open( scene.get() );
    }

    viewer.setSceneData( scene.get() );

    // start operation thread if a master file has been used.
    osg::ref_ptr<osg::OperationThread>                    masterOperationThread;

    typedef std::list<osg::ref_ptr<osg::OperationThread>> OperationThreadList;
    OperationThreadList                                   generalThreadList;

    if( masterOperation.valid() )
    {
        masterOperationThread = new osg::OperationThread;
        masterOperationThread->startThread();

        masterOperationThread->add( masterOperation.get() );

        // if (numLoadThreads>0)
        {
            osg::ref_ptr<osg::OperationQueue> operationQueue = new osg::OperationQueue;
            masterOperation->setOperationQueue( operationQueue.get() );

            for( unsigned int i = 0; i < numLoadThreads; ++i )
            {
                osg::ref_ptr<osg::OperationThread> thread = new osg::OperationThread;
                thread->setOperationQueue( operationQueue.get() );
                thread->startThread();
                generalThreadList.push_back( thread );
            }
        }

        viewer.addUpdateOperation( masterOperation.get() );
    }

    viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );

    // enable the use of compile contexts and associated threads.
    // osg::DisplaySettings::instance()->setCompileContextsHint(true);

    // realize the graphics windows.
    viewer.realize();

    // run the viewers main loop
    return viewer.run();
}
