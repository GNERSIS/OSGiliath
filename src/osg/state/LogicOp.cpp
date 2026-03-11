/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Logical pixel operation between fragment and framebuffer.
 * Supports AND, OR, XOR, and other bitwise operations.
 */
#include <osg/state/LogicOp.hpp>

#include <osg/core/Notify.hpp>

using namespace osg;

LogicOp::LogicOp() :
    _opcode( Opcode::COPY )
{
}

LogicOp::LogicOp( Opcode opcode ) :
    _opcode( opcode )
{
}

LogicOp::~LogicOp()
{
}

void
LogicOp::apply( State& ) const
{
    glLogicOp( static_cast<GLenum>( _opcode ) );
}
