#include <stdio.h>
#include <osg/Geode>
#include <osg/Drawable>
#include <osg/BlendFunc>
#include <osg/StateSet>
#include <osg/Notify>
#include <osg/Viewport>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osgUtil/CullVisitor>

using namespace osg;
using namespace osgDB;


class Logos: public osg::Drawable
{
    public:
        enum RelativePosition{
            Center,
            UpperLeft,
            UpperRight,
            LowerLeft,
            LowerRight,
            UpperCenter,
            LowerCenter,
            last_position
        };

        struct logosCullCallback : public osg::DrawableCullCallback
        {
            virtual bool cull(osg::NodeVisitor *visitor, osg::Drawable* drawable, osg::State*) const
            {
                Logos *logos = dynamic_cast<Logos *>(drawable);
                if (!logos) return true;

                osgUtil::CullVisitor *cv = visitor->asCullVisitor();
                if (!cv) return true;

                osg::State* state = cv->getState();

                unsigned int contextID = state!=0 ? state->getContextID() : 0;
                if(contextID != logos->getContextID())
                {
                    // logo not appropriate for window assigned to the cull visitor so cull it.
                    return true;
                }

                osg::Viewport *vp = cv->getViewport();
                if( vp != NULL )
                {
                    if( vp->width() != logos->getViewport()->width() ||
                        vp->height() != logos->getViewport()->height() )
                    {
                        logos->getViewport()->setViewport( vp->x(), vp->y(), vp->width(), vp->height() );
                        logos->dirtyGLObjects();
                    }
                }
                return false;
            }
        };

        Logos()
        {
            osg::StateSet *sset = new osg::StateSet;
            osg::BlendFunc *transp = new osg::BlendFunc;
            transp->setFunction(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            sset->setAttribute( transp );
            sset->setMode( GL_BLEND, osg::StateAttribute::ON );
            sset->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
#if 1
            // for now we'll crudely set the bin number to 100 to force it to draw later and ontop of the scene
            sset->setRenderBinDetails( 100 , "RenderBin" );
#else
            sset->setRenderBinDetails( StateSet::TRANSPARENT_BIN + 1 , "RenderBin" );
#endif
            setStateSet( sset );
            _viewport = new osg::Viewport;
            setCullCallback( new logosCullCallback );
            _contextID = 0;
        }

        Logos(const Logos& logo, const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
            Drawable( logo, copyop ),
            _contextID(0) {}

        virtual Object* cloneType() const { return new Logos(); }
        virtual Object* clone( const CopyOp& copyop) const { return new Logos(*this, copyop ); }
        virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const Logos*>(obj)!=NULL; }
        virtual const char* className() const { return "Logos"; }

        virtual void drawImplementation(osg::RenderInfo& renderInfo) const
        {
            OSG_NOTICE<<"Warning: Logos::drawImplementation(..) not supported."<<std::endl;
        }

        void addLogo( RelativePosition pos, std::string name )
        {
            osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile( name.c_str() );
            if( image.valid())
            {
                _logos[pos].push_back( image );
            }
            else
            {
                OSG_WARN<< "Logos::addLogo image file not found : " << name << ".\n";
            }
        }

        osg::Viewport *getViewport() { return _viewport.get(); }

        void setContextID( unsigned int id ) { _contextID = id; }
        unsigned int getContextID() { return _contextID; }

        bool hasLogos()
        {
            int n = 0;
            for( int i = Center; i < last_position; i++ )
                n += _logos[i].size();
            return (n != 0);
        }

        virtual osg::BoundingBox computeBoundingBox() const
        {
            return osg::BoundingBox( -1, -1, -1, 1, 1, 1);
        }

    protected:
        Logos& operator = (const Logos&) { return *this;}

        virtual ~Logos() {}
    private :
        typedef std::vector < osg::ref_ptr<osg::Image> >  Images;

        Images _logos[last_position];
        osg::ref_ptr<osg::Viewport> _viewport;
        unsigned int _contextID;
};


class LOGOReaderWriter : public osgDB::ReaderWriter
{
    public:
        LOGOReaderWriter()
        {
            supportsExtension("logo","Ascii logo placement format");
        }

        virtual const char* className() const { return "Logo Database Reader/Writer"; }

        virtual ReadResult readObject(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
        {
            return readNode(filename, options);
        }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty())
                return ReadResult::FILE_NOT_FOUND;

            OSG_INFO<< "ReaderWriterLOGO::readNode( "<<fileName.c_str()<<" )\n";

            std::string filePath = osgDB::getFilePath(fileName);
            if (!filePath.empty()) {
                OSG_DEBUG<< "Adding : " << filePath << " to the file data path\n";
                osgDB::getDataFilePathList().push_back(filePath);
            }


            osg::ref_ptr<osg::Geode> geode = new osg::Geode;

            unsigned int screen = 0;

            Logos* ld = new Logos;
            ld->setContextID( screen );

            Logos::RelativePosition pos = Logos::LowerRight;


            std::ifstream fin(fileName.c_str());
            if (!fin) return NULL;

            while(fin)
            {
                std::string str;
                fin >> str;

                if( str == "Center" )
                    pos = Logos::Center;
                else if( str == "UpperLeft" )
                    pos = Logos::UpperLeft;
                else if( str == "UpperRight" )
                    pos = Logos::UpperRight;
                else if( str == "LowerLeft" )
                    pos = Logos::LowerLeft;
                else if( str == "LowerRight" )
                    pos = Logos::LowerRight;
                else if( str == "UpperCenter" )
                    pos = Logos::UpperCenter;
                else if( str == "LowerCenter" )
                    pos = Logos::LowerCenter;
                else if( str == "Camera" )
                {
                    int tn;
                    fin >> tn;
                    if (fin.fail())
                    {
                        OSG_WARN << "Error... Camera requires an integer argument\n";
                        break;
                    }

                    if (tn < 0)
                    {
                        OSG_WARN << "Error... Camera requires an positive or null value argument\n";
                        break;
                    }

                    unsigned int n = static_cast<unsigned int>(tn);
                    if( screen != n )
                    {
                        screen = n;
                        if( ld->hasLogos() )
                        {
                        geode->addDrawable( ld );
                        ld = new Logos;
                        ld->setContextID( screen );
                    }
                    else
                        ld->setContextID( screen );
                    }
                }
                else
                {
                    if( str.length() )
                    ld->addLogo( pos, str );
                }
            }

            if( ld->hasLogos() )
                geode->addDrawable( ld );

            ld->setCullingActive(false);
            return geode;
        }
};


// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(logo, LOGOReaderWriter)
