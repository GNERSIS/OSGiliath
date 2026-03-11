/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * TextNode example application
 */
#ifndef OSGTEXT_TEXTNODE
    #define OSGTEXT_TEXTNODE 1

    #include <osg/core/Inherit.hpp>
    #include <osg/maths/compat.hpp>
    #include <osg/maths/quat.hpp>
    #include <osg/nodes/Group.hpp>
    #include <osgText/Font>
    #include <osgText/Glyph>
    #include <osgText/String>
    #include <osgText/Style>
    #include <osgUtil/culling/CullVisitor.hpp>

namespace osgText
{

    // forward declare
    class TextNode;

    class /*OSGTEXT_EXPORT*/ Layout : public osg::Inherit<osg::Object, Layout>
    {
        public:

            OSG_REGISTER_TYPE( osgText,
                               Layout )

            Layout();
            Layout( const Layout&      layout,
                    const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            /// default Layout implementation used if no other is specified on TextNode
            static osg::ref_ptr<Layout>&
            getDefaultLayout();

            virtual void
            layout( TextNode& text ) const;

        protected:
    };

    class /*OSGTEXT_EXPORT*/ TextTechnique
        : public osg::Inherit<osg::Object, TextTechnique>
    {
        public:

            OSG_REGISTER_TYPE( osgText,
                               TextTechnique )

            TextTechnique();
            TextTechnique( const TextTechnique& technique,
                           const osg::CopyOp&   copyop = osg::CopyOp::SHALLOW_COPY );

            TextNode*
            getTextNode()
            {
                return _textNode;
            }

            const TextNode*
            getTextNode() const
            {
                return _textNode;
            }

            /// default TextTechnique implementation used if no other is specified on
            /// TextNode
            static osg::ref_ptr<TextTechnique>&
            getDefaultTextTechinque();

            /// start building a new character layout
            virtual void
            start();

            /// called by Layout engine to place individual characters
            virtual void
            addCharacter( const osg::vec3& position,
                          const osg::vec3& size,
                          Glyph*           glyph,
                          Style*           style );

            /// called by Layout engine to place individual characters
            virtual void
            addCharacter( const osg::vec3& position,
                          const osg::vec3& size,
                          Glyph3D*         glyph,
                          Style*           style );

            /// finish building new character layout
            virtual void
            finish();

            /// provide traversal control
            virtual void
            traverse( osg::NodeVisitor& nv );

        protected:

            friend class TextNode;

            void
            setTextNode( TextNode* textNode )
            {
                _textNode = textNode;
            }

            TextNode* _textNode;
    };

    class /*OSGTEXT_EXPORT*/ TextNode : public osg::Inherit<osg::Group, TextNode>
    {
        public:

            OSG_REGISTER_TYPE( osgText,
                               TextNode )

            TextNode();
            TextNode( const TextNode&    text,
                      const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            void
            accept( osg::NodeVisitor& nv ) override
            {
                if( nv.validNodeMask( *this ) )
                {
                    nv.pushOntoNodePath( this );
                    nv.apply( *this );
                    nv.popFromNodePath();
                }
            }

            virtual void
            traverse( osg::NodeVisitor& nv );

            void
            setFont( Font* font )
            {
                _font = font;
            }

            Font*
            getFont()
            {
                return _font.get();
            }

            const Font*
            getFont() const
            {
                return _font.get();
            }

            Font*
            getActiveFont()
            {
                return _font.valid() ? _font.get() : Font::getDefaultFont().get();
            }

            const Font*
            getActiveFont() const
            {
                return _font.valid() ? _font.get() : Font::getDefaultFont().get();
            }

            void
            setStyle( Style* style )
            {
                _style = style;
            }

            Style*
            getStyle()
            {
                return _style.get();
            }

            const Style*
            getStyle() const
            {
                return _style.get();
            }

            Style*
            getActiveStyle()
            {
                return _style.valid() ? _style.get() : Style::getDefaultStyle().get();
            }

            const Style*
            getActiveStyle() const
            {
                return _style.valid() ? _style.get() : Style::getDefaultStyle().get();
            }

            void
            setLayout( Layout* layout )
            {
                _layout = layout;
            }

            Layout*
            getLayout()
            {
                return _layout.get();
            }

            const Layout*
            getLayout() const
            {
                return _layout.get();
            }

            const Layout*
            getActiveLayout() const
            {
                return _layout.valid() ? _layout.get()
                                       : Layout::getDefaultLayout().get();
            }

            void
            setTextTechnique( TextTechnique* technique );

            TextTechnique*
            getTextTechnique()
            {
                return _technique.get();
            }

            const TextTechnique*
            getTextTechnique() const
            {
                return _technique.get();
            }

            void
            setText( const std::string& str );

            void
            setText( const String& str )
            {
                _string = str;
            }

            String&
            getText()
            {
                return _string;
            }

            const String&
            getText() const
            {
                return _string;
            }

            void
            setPosition( const osg::dvec3& position )
            {
                _position = position;
            }

            const osg::dvec3&
            getPosition() const
            {
                return _position;
            }

            void
            setRotation( const osg::quat& rotation )
            {
                _rotation = rotation;
            }

            const osg::quat&
            getRotation() const
            {
                return _rotation;
            }

            void
            setCharacterSize( float characterSize )
            {
                _characterSize = characterSize;
            }

            float
            getCharacterSize() const
            {
                return _characterSize;
            }

            /// force a regeneration of the rendering backend required to represent the
            /// text.
            virtual void
            update();

        protected:

            virtual ~TextNode();

            osg::ref_ptr<Font>          _font;
            osg::ref_ptr<Style>         _style;
            osg::ref_ptr<Layout>        _layout;
            osg::ref_ptr<TextTechnique> _technique;

            String                      _string;
            osg::dvec3                  _position;
            osg::quat                   _rotation;
            float                       _characterSize;
    };

}

#endif
