/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Typed GPU-uploadable arrays (Vec3Array, Vec4Array, FloatArray, etc.).
 * Backed by BufferObject for VBO binding. Foundation for vertex data.
 */
#include <osg/geometry/Array.hpp>

#include <osg/core/Notify.hpp>

using namespace osg;

static const char* s_ArrayNames[] = {
    "Array",           // 0
    "ByteArray",       // 1
    "ShortArray",      // 2
    "IntArray",        // 3

    "UByteArray",      // 4
    "UShortArray",     // 5
    "UIntArray",       // 6

    "FloatArray",      // 7
    "DoubleArray",     // 8

    "Vec2bArray",      // 9
    "Vec3bArray",      // 10
    "Vec4bArray",      // 11

    "Vec2sArray",      // 12
    "Vec3sArray",      // 13
    "Vec4sArray",      // 14

    "Vec2iArray",      // 15
    "Vec3iArray",      // 16
    "Vec4iArray",      // 17

    "Vec2ubArray",     // 18
    "Vec3ubArray",     // 19
    "Vec4ubArray",     // 20

    "Vec2usArray",     // 21
    "Vec3usArray",     // 22
    "Vec4usArray",     // 23

    "Vec2uiArray",     // 24
    "Vec3uiArray",     // 25
    "Vec4uiArray",     // 26

    "Vec2Array",       // 27
    "Vec3Array",       // 28
    "Vec4Array",       // 29

    "Vec2dArray",      // 30
    "Vec3dArray",      // 31
    "Vec4dArray",      // 32

    "MatrixArray",     // 33
    "MatrixdArray",    // 34

    "QuatArray",       // 35

    "UInt64Array",     // 36
    "Int64Array",      // 37
};

const char*
Array::className() const
{
    if( _arrayType >= ArrayType && _arrayType <= LastArrayType )
    {
        return s_ArrayNames[_arrayType];
    }
    else
    {
        OSG_DEBUG << "Array::className(): Unknown array type " << _arrayType
                  << std::endl;
        return "UnknownArray";
    }
}
