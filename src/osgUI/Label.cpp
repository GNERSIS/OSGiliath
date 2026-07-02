/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Text label widget. Displays non-editable text with
 * configurable font, size, and alignment.
 */
#include <osgUI/Label.hpp>

#include <osgText/Font.hpp>
#include <osgText/String.hpp>
#include <osgText/Text.hpp>

using namespace osgUI;

Label::Label()
{
}

Label::Label( const osgUI::Label& label,
              const osg::CopyOp&  copyop ) :
    Inherit( label,
             copyop ),
    _text( label._text )
{
}

void
Label::createGraphicsImplementation()
{
    OSG_NOTICE << "Label::createGraphicsImplementation()" << std::endl;

    Style* style = ( getStyle() != 0 ) ? getStyle() : Style::instance().get();
    osg::ref_ptr<Node> node =
        style->createText( _extents, getAlignmentSettings(), getTextSettings(), _text );
    _textDrawable = dynamic_cast<osgText::Text*>( node.get() );

    style->setupClipStateSet( _extents, getOrCreateWidgetStateSet() );
    setGraphicsSubgraph( 0, node.get() );
}
