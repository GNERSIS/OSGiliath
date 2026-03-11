/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * HDRLoaderResult class.
 * Provides: isHDRFile, load.
 */
#pragma once

class HDRLoaderResult
{
    public:

        int    width, height;
        // each pixel takes 3 float32, each component can be of any value...
        float* cols;    // this is to be freed using free() and not delete[]
};

class HDRLoader
{
    public:

        static bool
        isHDRFile( const char* fileName );
        static bool
        load( const char*      fileName,
              const bool       rawRGBE,
              HDRLoaderResult& res );
};
