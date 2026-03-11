/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Material class.
 * Provides: ambient, diffuse, specular, emissive, sharpness, illum.
 */
#pragma once

#include <istream>
#include <map>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <string>
#include <vector>

namespace obj
{

    class Material
    {
        public:

            Material() :
                ambient( 0.2F,
                         0.2F,
                         0.2F,
                         1.0F ),
                diffuse( 0.8F,
                         0.8F,
                         0.8F,
                         1.0F ),
                specular( 0.0F,
                          0.0F,
                          0.0F,
                          1.0F ),
                emissive( 0.0F,
                          0.0F,
                          0.0F,
                          1.0F ),
                sharpness( 0.0F ),
                illum( 2 ),
                Tf( 0.0F,
                    0.0F,
                    0.0F,
                    1.0F ),
                Ni( 0 ),
                Ns( -1 ),
                // textureReflection(false),
                alpha( 1.0F )
            {
            }

            std::string name;

            osg::vec4   ambient;
            osg::vec4   diffuse;
            osg::vec4   specular;
            osg::vec4   emissive;
            float       sharpness;
            int illum;    // read but not implemented (would need specific shaders or
                          // state manipulation)

            osg::vec4 Tf;
            int       Ni;
            int       Ns;    // shininess 0..1000

            // bool        textureReflection;
            float     alpha;

            class Map
            {
                    // -o and -s (offset and scale) options supported for the maps
                    // -clamp <on|off> is supported
                    // -blendu, -blendv, -imfchan, not supported
                    // -mm <base> <gain> is parsed but not actually used
                    // -bm <bump_multiplier> is parsed but not used

                public:

                    enum TextureMapType
                    {
                        DIFFUSE = 0,
                        OPACITY,
                        AMBIENT,
                        SPECULAR,
                        SPECULAR_EXPONENT,
                        BUMP,
                        DISPLACEMENT,
                        REFLECTION,    // read of a reflection map will also apply
                                       // spherical texgen coordinates
                        UNKNOWN,       // UNKNOWN has to be the last
                    };

                    Map() :
                        type( UNKNOWN ),
                        name( "" ),
                        uScale( 1.0F ),
                        vScale( 1.0F ),
                        uOffset( 0.0F ),
                        vOffset( 0.0F ),
                        clamp( false )
                    {
                    }

                    TextureMapType type;
                    std::string    name;

                    // Texture scale and offset, used for creating the texture matrix.
                    // Reader only picks u and v from -s u v w, although all u v and w
                    // all need to be specified! e.g. "map_Kd -s u v w <name>" is OK but
                    // "map_Kd -s u v <name>" is not, even though tex is only 2D
                    float          uScale;
                    float          vScale;
                    float          uOffset;
                    float          vOffset;

                    // According to the spec, if clamping is off (default), the effect is
                    // a texture repeat if clamping is on, then the effect is a decal
                    // texture; i.e. the border is transparent
                    bool           clamp;
            };

            std::vector<Map> maps;

        protected:
    };

    class Element : public osg::Referenced
    {
        public:

            typedef std::vector<int> IndexList;

            enum DataType
            {
                POINTS,
                POLYLINE,
                POLYGON,
            };

            Element( DataType type ) :
                dataType( type )
            {
            }

            enum CoordinateCombination
            {
                VERTICES,
                VERTICES_NORMALS,
                VERTICES_TEXCOORDS,
                VERTICES_NORMALS_TEXCOORDS,
            };

            CoordinateCombination
            getCoordinateCombination() const
            {
                if( vertexIndices.size() == normalIndices.size() )
                {
                    return ( vertexIndices.size() == texCoordIndices.size() )
                             ? VERTICES_NORMALS_TEXCOORDS
                             : VERTICES_NORMALS;
                }
                else
                {
                    return ( vertexIndices.size() == texCoordIndices.size() )
                             ? VERTICES_TEXCOORDS
                             : VERTICES;
                }
            }

            DataType  dataType;
            IndexList vertexIndices;
            IndexList normalIndices;
            IndexList texCoordIndices;
            IndexList colorsIndices;
    };

    class ElementState
    {
        public:

            ElementState() :
                coordinateCombination( Element::VERTICES ),
                smoothingGroup( 0 )
            {
            }

            bool
            operator<( const ElementState& rhs ) const
            {
                if( materialName < rhs.materialName )
                {
                    return true;
                }
                else if( rhs.materialName < materialName )
                {
                    return false;
                }

                if( objectName < rhs.objectName )
                {
                    return true;
                }
                else if( rhs.objectName < objectName )
                {
                    return false;
                }

                if( groupName < rhs.groupName )
                {
                    return true;
                }
                else if( rhs.groupName < groupName )
                {
                    return false;
                }

                if( coordinateCombination < rhs.coordinateCombination )
                {
                    return true;
                }
                else if( rhs.coordinateCombination < coordinateCombination )
                {
                    return false;
                }

                return ( smoothingGroup < rhs.smoothingGroup );
            }

            std::string                    objectName;
            std::string                    groupName;
            std::string                    materialName;
            Element::CoordinateCombination coordinateCombination;
            int                            smoothingGroup;
    };

    class Model
    {
        public:

            Model() :
                currentElementList( 0 )
            {
            }

            void
            setDatabasePath( const std::string& path )
            {
                databasePath = path;
            }

            const std::string&
            getDatabasePath() const
            {
                return databasePath;
            }

            std::string
            lastComponent( const char* linep );
            bool
            readMTL( std::istream& fin );
            bool
            readOBJ( std::istream&                       fin,
                     const osgDB::ReaderWriter::Options* options );

            bool
            readline( std::istream& fin,
                      char*         line,
                      const int     LINE_SIZE );
            void
            addElement( Element* element );

            osg::vec3
            averageNormal( const Element& element ) const;
            osg::vec3
            computeNormal( const Element& element ) const;
            bool
            needReverse( const Element& element ) const;

            int
            remapVertexIndex( int vi )
            {
                return ( vi < 0 ) ? vertices.size() + vi : vi - 1;
            }

            int
            remapNormalIndex( int vi )
            {
                return ( vi < 0 ) ? normals.size() + vi : vi - 1;
            }

            int
            remapTexCoordIndex( int vi )
            {
                return ( vi < 0 ) ? texcoords.size() + vi : vi - 1;
            }

            typedef std::map<std::string, Material>     MaterialMap;
            typedef std::vector<osg::vec2>              Vec2Array;
            typedef std::vector<osg::vec3>              Vec3Array;
            typedef std::vector<osg::vec4>              Vec4Array;
            typedef std::vector<osg::ref_ptr<Element>>  ElementList;
            typedef std::map<ElementState, ElementList> ElementStateMap;

            std::string                                 databasePath;
            MaterialMap                                 materialMap;

            Vec3Array                                   vertices;
            Vec4Array                                   colors;
            Vec3Array                                   normals;
            Vec2Array                                   texcoords;

            ElementState                                currentElementState;

            ElementStateMap                             elementStateMap;
            ElementList*                                currentElementList;
    };

}
