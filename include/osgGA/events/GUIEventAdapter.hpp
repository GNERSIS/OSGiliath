/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Event data container carrying input event details (key, button,
 * coordinates, scroll, touch). Immutable once dispatched.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/rendering/GraphicsContext.hpp>
#include <osgGA/events/Event.hpp>

namespace osgGA
{

    struct PointerData : public osg::Referenced
    {
            PointerData() :
                object( 0 ),
                x( 0.0F ),
                xMin( -1.0F ),
                xMax( 1.0F ),
                y( 0.0F ),
                yMin( -1.0F ),
                yMax( 1.0F )
            {
            }

            PointerData( osg::Object* obj,
                         float        in_x,
                         float        in_xMin,
                         float        in_xMax,
                         float        in_y,
                         float        in_yMin,
                         float        in_yMax ) :
                object( obj ),
                x( in_x ),
                xMin( in_xMin ),
                xMax( in_xMax ),
                y( in_y ),
                yMin( in_yMin ),
                yMax( in_yMax )
            {
            }

            PointerData( const PointerData& pd ) :
                osg::Referenced(),
                object( pd.object ),
                x( pd.x ),
                xMin( pd.xMin ),
                xMax( pd.xMax ),
                y( pd.y ),
                yMin( pd.yMin ),
                yMax( pd.yMax )
            {
            }

            PointerData&
            operator=( const PointerData& pd )
            {
                if( &pd == this )
                {
                    return *this;
                }

                object = pd.object;
                x      = pd.x;
                xMin   = pd.xMin;
                xMax   = pd.xMax;
                y      = pd.y;
                yMin   = pd.yMin;
                yMax   = pd.yMax;

                return *this;
            }

            osg::observer_ptr<osg::Object> object;
            float                          x, xMin, xMax;
            float                          y, yMin, yMax;

            float
            getXnormalized() const
            {
                return ( x - xMin ) / ( xMax - xMin ) * 2.0F - 1.0F;
            }

            float
            getYnormalized() const
            {
                return ( y - yMin ) / ( yMax - yMin ) * 2.0F - 1.0F;
            }
    };

    /** Event class for storing Keyboard, mouse and window events.
     */
    class OSGGA_EXPORT GUIEventAdapter : public osg::Inherit<Event, GUIEventAdapter>
    {
        public:

            enum MouseButtonMask
            {
                LEFT_MOUSE_BUTTON   = 1 << 0,
                MIDDLE_MOUSE_BUTTON = 1 << 1,
                RIGHT_MOUSE_BUTTON  = 1 << 2,
            };

            enum EventType
            {
                NONE                = 0,
                PUSH                = 1 << 0,
                RELEASE             = 1 << 1,
                DOUBLECLICK         = 1 << 2,
                DRAG                = 1 << 3,
                MOVE                = 1 << 4,
                KEYDOWN             = 1 << 5,
                KEYUP               = 1 << 6,
                FRAME               = 1 << 7,
                RESIZE              = 1 << 8,
                SCROLL              = 1 << 9,
                PEN_PRESSURE        = 1 << 10,
                PEN_ORIENTATION     = 1 << 11,
                PEN_PROXIMITY_ENTER = 1 << 12,
                PEN_PROXIMITY_LEAVE = 1 << 13,
                CLOSE_WINDOW        = 1 << 14,
                QUIT_APPLICATION    = 1 << 15,
                USER                = 1 << 16,
            };

            enum KeySymbol
            {
                KEY_Space        = 0X20,

                KEY_0            = '0',
                KEY_1            = '1',
                KEY_2            = '2',
                KEY_3            = '3',
                KEY_4            = '4',
                KEY_5            = '5',
                KEY_6            = '6',
                KEY_7            = '7',
                KEY_8            = '8',
                KEY_9            = '9',
                KEY_A            = 'a',
                KEY_B            = 'b',
                KEY_C            = 'c',
                KEY_D            = 'd',
                KEY_E            = 'e',
                KEY_F            = 'f',
                KEY_G            = 'g',
                KEY_H            = 'h',
                KEY_I            = 'i',
                KEY_J            = 'j',
                KEY_K            = 'k',
                KEY_L            = 'l',
                KEY_M            = 'm',
                KEY_N            = 'n',
                KEY_O            = 'o',
                KEY_P            = 'p',
                KEY_Q            = 'q',
                KEY_R            = 'r',
                KEY_S            = 's',
                KEY_T            = 't',
                KEY_U            = 'u',
                KEY_V            = 'v',
                KEY_W            = 'w',
                KEY_X            = 'x',
                KEY_Y            = 'y',
                KEY_Z            = 'z',

                KEY_Exclaim      = 0X21,
                KEY_Quotedbl     = 0X22,
                KEY_Hash         = 0X23,
                KEY_Dollar       = 0X24,
                KEY_Ampersand    = 0X26,
                KEY_Quote        = 0X27,
                KEY_Leftparen    = 0X28,
                KEY_Rightparen   = 0X29,
                KEY_Asterisk     = 0X2A,
                KEY_Plus         = 0X2B,
                KEY_Comma        = 0X2C,
                KEY_Minus        = 0X2D,
                KEY_Period       = 0X2E,
                KEY_Slash        = 0X2F,
                KEY_Colon        = 0X3A,
                KEY_Semicolon    = 0X3B,
                KEY_Less         = 0X3C,
                KEY_Equals       = 0X3D,
                KEY_Greater      = 0X3E,
                KEY_Question     = 0X3F,
                KEY_At           = 0X40,
                KEY_Leftbracket  = 0X5B,
                KEY_Backslash    = 0X5C,
                KEY_Rightbracket = 0X5D,
                KEY_Caret        = 0X5E,
                KEY_Underscore   = 0X5F,
                KEY_Backquote    = 0X60,

                KEY_BackSpace    = 0XFF'08, /* back space, back char */
                KEY_Tab          = 0XFF'09,
                KEY_Linefeed     = 0XFF'0A, /* Linefeed, LF */
                KEY_Clear        = 0XFF'0B,
                KEY_Return       = 0XFF'0D, /* Return, enter */
                KEY_Pause        = 0XFF'13, /* Pause, hold */
                KEY_Scroll_Lock  = 0XFF'14,
                KEY_Sys_Req      = 0XFF'15,
                KEY_Escape       = 0XFF'1B,
                KEY_Delete       = 0XFF'FF, /* Delete, rubout */

                /* Cursor control & motion */

                KEY_Home      = 0XFF'50,
                KEY_Left      = 0XFF'51, /* Move left, left arrow */
                KEY_Up        = 0XFF'52, /* Move up, up arrow */
                KEY_Right     = 0XFF'53, /* Move right, right arrow */
                KEY_Down      = 0XFF'54, /* Move down, down arrow */
                KEY_Prior     = 0XFF'55, /* Prior, previous */
                KEY_Page_Up   = 0XFF'55,
                KEY_Next      = 0XFF'56, /* Next */
                KEY_Page_Down = 0XFF'56,
                KEY_End       = 0XFF'57, /* EOL */
                KEY_Begin     = 0XFF'58, /* BOL */

                /* Misc Functions */

                KEY_Select  = 0XFF'60, /* Select, mark */
                KEY_Print   = 0XFF'61,
                KEY_Execute = 0XFF'62, /* Execute, run, do */
                KEY_Insert  = 0XFF'63, /* Insert, insert here */
                KEY_Undo    = 0XFF'65, /* Undo, oops */
                KEY_Redo    = 0XFF'66, /* redo, again */
                KEY_Menu =
                    0XFF'67, /* On Windows, this is VK_APPS, the context-menu key */
                KEY_Find          = 0XFF'68, /* Find, search */
                KEY_Cancel        = 0XFF'69, /* Cancel, stop, abort, exit */
                KEY_Help          = 0XFF'6A, /* Help */
                KEY_Break         = 0XFF'6B,
                KEY_Mode_switch   = 0XFF'7E, /* Character set switch */
                KEY_Script_switch = 0XFF'7E, /* Alias for mode_switch */
                KEY_Num_Lock      = 0XFF'7F,

                /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

                KEY_KP_Space     = 0XFF'80, /* space */
                KEY_KP_Tab       = 0XFF'89,
                KEY_KP_Enter     = 0XFF'8D, /* enter */
                KEY_KP_F1        = 0XFF'91, /* PF1, KP_A, ... */
                KEY_KP_F2        = 0XFF'92,
                KEY_KP_F3        = 0XFF'93,
                KEY_KP_F4        = 0XFF'94,
                KEY_KP_Home      = 0XFF'95,
                KEY_KP_Left      = 0XFF'96,
                KEY_KP_Up        = 0XFF'97,
                KEY_KP_Right     = 0XFF'98,
                KEY_KP_Down      = 0XFF'99,
                KEY_KP_Prior     = 0XFF'9A,
                KEY_KP_Page_Up   = 0XFF'9A,
                KEY_KP_Next      = 0XFF'9B,
                KEY_KP_Page_Down = 0XFF'9B,
                KEY_KP_End       = 0XFF'9C,
                KEY_KP_Begin     = 0XFF'9D,
                KEY_KP_Insert    = 0XFF'9E,
                KEY_KP_Delete    = 0XFF'9F,
                KEY_KP_Equal     = 0XFF'BD, /* equals */
                KEY_KP_Multiply  = 0XFF'AA,
                KEY_KP_Add       = 0XFF'AB,
                KEY_KP_Separator = 0XFF'AC, /* separator, often comma */
                KEY_KP_Subtract  = 0XFF'AD,
                KEY_KP_Decimal   = 0XFF'AE,
                KEY_KP_Divide    = 0XFF'AF,

                KEY_KP_0         = 0XFF'B0,
                KEY_KP_1         = 0XFF'B1,
                KEY_KP_2         = 0XFF'B2,
                KEY_KP_3         = 0XFF'B3,
                KEY_KP_4         = 0XFF'B4,
                KEY_KP_5         = 0XFF'B5,
                KEY_KP_6         = 0XFF'B6,
                KEY_KP_7         = 0XFF'B7,
                KEY_KP_8         = 0XFF'B8,
                KEY_KP_9         = 0XFF'B9,

                /*
                 * Auxiliary Functions; note the duplicate definitions for left and right
                 * function keys;  Sun keyboards and a few other manufactures have such
                 * function key groups on the left and/or right sides of the keyboard.
                 * We've not found a keyboard with more than 35 function keys total.
                 */

                KEY_F1  = 0XFF'BE,
                KEY_F2  = 0XFF'BF,
                KEY_F3  = 0XFF'C0,
                KEY_F4  = 0XFF'C1,
                KEY_F5  = 0XFF'C2,
                KEY_F6  = 0XFF'C3,
                KEY_F7  = 0XFF'C4,
                KEY_F8  = 0XFF'C5,
                KEY_F9  = 0XFF'C6,
                KEY_F10 = 0XFF'C7,
                KEY_F11 = 0XFF'C8,
                KEY_F12 = 0XFF'C9,
                KEY_F13 = 0XFF'CA,
                KEY_F14 = 0XFF'CB,
                KEY_F15 = 0XFF'CC,
                KEY_F16 = 0XFF'CD,
                KEY_F17 = 0XFF'CE,
                KEY_F18 = 0XFF'CF,
                KEY_F19 = 0XFF'D0,
                KEY_F20 = 0XFF'D1,
                KEY_F21 = 0XFF'D2,
                KEY_F22 = 0XFF'D3,
                KEY_F23 = 0XFF'D4,
                KEY_F24 = 0XFF'D5,
                KEY_F25 = 0XFF'D6,
                KEY_F26 = 0XFF'D7,
                KEY_F27 = 0XFF'D8,
                KEY_F28 = 0XFF'D9,
                KEY_F29 = 0XFF'DA,
                KEY_F30 = 0XFF'DB,
                KEY_F31 = 0XFF'DC,
                KEY_F32 = 0XFF'DD,
                KEY_F33 = 0XFF'DE,
                KEY_F34 = 0XFF'DF,
                KEY_F35 = 0XFF'E0,

                /* Modifiers */

                KEY_Shift_L    = 0XFF'E1, /* Left shift */
                KEY_Shift_R    = 0XFF'E2, /* Right shift */
                KEY_Control_L  = 0XFF'E3, /* Left control */
                KEY_Control_R  = 0XFF'E4, /* Right control */
                KEY_Caps_Lock  = 0XFF'E5, /* Caps lock */
                KEY_Shift_Lock = 0XFF'E6, /* Shift lock */

                KEY_Meta_L     = 0XFF'E7, /* Left meta */
                KEY_Meta_R     = 0XFF'E8, /* Right meta */
                KEY_Alt_L      = 0XFF'E9, /* Left alt */
                KEY_Alt_R      = 0XFF'EA, /* Right alt */
                KEY_Super_L    = 0XFF'EB, /* Left super */
                KEY_Super_R    = 0XFF'EC, /* Right super */
                KEY_Hyper_L    = 0XFF'ED, /* Left hyper */
                KEY_Hyper_R    = 0XFF'EE  /* Right hyper */
            };

            enum ModKeyMask
            {
                MODKEY_LEFT_SHIFT  = 0X00'01,
                MODKEY_RIGHT_SHIFT = 0X00'02,
                MODKEY_LEFT_CTRL   = 0X00'04,
                MODKEY_RIGHT_CTRL  = 0X00'08,
                MODKEY_LEFT_ALT    = 0X00'10,
                MODKEY_RIGHT_ALT   = 0X00'20,
                MODKEY_LEFT_META   = 0X00'40,
                MODKEY_RIGHT_META  = 0X00'80,
                MODKEY_LEFT_SUPER  = 0X01'00,
                MODKEY_RIGHT_SUPER = 0X02'00,
                MODKEY_LEFT_HYPER  = 0X04'00,
                MODKEY_RIGHT_HYPER = 0X08'00,
                MODKEY_NUM_LOCK    = 0X10'00,
                MODKEY_CAPS_LOCK   = 0X20'00,
                MODKEY_CTRL        = ( MODKEY_LEFT_CTRL | MODKEY_RIGHT_CTRL ),
                MODKEY_SHIFT       = ( MODKEY_LEFT_SHIFT | MODKEY_RIGHT_SHIFT ),
                MODKEY_ALT         = ( MODKEY_LEFT_ALT | MODKEY_RIGHT_ALT ),
                MODKEY_META        = ( MODKEY_LEFT_META | MODKEY_RIGHT_META ),
                MODKEY_SUPER       = ( MODKEY_LEFT_SUPER | MODKEY_RIGHT_SUPER ),
                MODKEY_HYPER       = ( MODKEY_LEFT_HYPER | MODKEY_RIGHT_HYPER ),
            };

            enum MouseYOrientation
            {
                Y_INCREASING_UPWARDS,
                Y_INCREASING_DOWNWARDS,
            };

            enum ScrollingMotion
            {
                SCROLL_NONE,
                SCROLL_LEFT,
                SCROLL_RIGHT,
                SCROLL_UP,
                SCROLL_DOWN,
                SCROLL_2D,
            };

            enum TabletPointerType
            {
                UNKNOWN = 0,
                PEN,
                PUCK,
                ERASER,
            };

            enum TouchPhase
            {
                TOUCH_UNKNOWN,
                TOUCH_BEGAN,
                TOUCH_MOVED,
                TOUCH_STATIONERY,
                TOUCH_ENDED,
            };

            class TouchData : public osg::Inherit<osg::Object, TouchData>
            {
                public:

                    struct TouchPoint
                    {
                            unsigned int id;
                            TouchPhase   phase;
                            float        x, y;

                            unsigned int tapCount;

                            TouchPoint() :
                                id( 0 ),
                                phase( TOUCH_UNKNOWN ),
                                x( 0.0F ),
                                y( 0.0F ),
                                tapCount( 0 )
                            {
                            }

                            TouchPoint( unsigned int in_id,
                                        TouchPhase   in_phase,
                                        float        in_x,
                                        float        in_y,
                                        unsigned int in_tap_count ) :
                                id( in_id ),
                                phase( in_phase ),
                                x( in_x ),
                                y( in_y ),
                                tapCount( in_tap_count )
                            {
                            }
                    };

                    typedef std::vector<TouchPoint>  TouchSet;

                    typedef TouchSet::iterator       iterator;
                    typedef TouchSet::const_iterator const_iterator;

                    TouchData()
                    {
                    }

                    TouchData( const TouchData&   td,
                               const osg::CopyOp& copyop ) :
                        Inherit( td,
                                 copyop ),
                        _touches( td._touches )
                    {
                    }

                    OSG_REGISTER_TYPE( osgGA,
                                       TouchData )

                    unsigned int
                    getNumTouchPoints() const
                    {
                        return static_cast<unsigned int>( _touches.size() );
                    }

                    iterator
                    begin()
                    {
                        return _touches.begin();
                    }

                    const_iterator
                    begin() const
                    {
                        return _touches.begin();
                    }

                    iterator
                    end()
                    {
                        return _touches.end();
                    }

                    const_iterator
                    end() const
                    {
                        return _touches.end();
                    }

                    const TouchPoint
                    get( unsigned int i ) const
                    {
                        return _touches[i];
                    }

                protected:

                    virtual ~TouchData()
                    {
                    }

                    void
                    addTouchPoint( unsigned int id,
                                   TouchPhase   phase,
                                   float        x,
                                   float        y,
                                   unsigned int tap_count )
                    {
                        _touches.push_back( TouchPoint( id, phase, x, y, tap_count ) );
                    }

                    TouchSet _touches;

                    friend class GUIEventAdapter;
            };

        public:

            GUIEventAdapter();

            GUIEventAdapter( const GUIEventAdapter& rhs,
                             const osg::CopyOp&     copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgGA,
                               GUIEventAdapter )

            virtual GUIEventAdapter*
            asGUIEventAdapter()
            {
                return this;
            }

            virtual const GUIEventAdapter*
            asGUIEventAdapter() const
            {
                return this;
            }

            /** Get the accumulated event state singleton.
             * Typically all EventQueue will share this single GUIEventAdapter object for
             * tracking the mouse position, keyboard and mouse masks. */
            static osg::ref_ptr<GUIEventAdapter>&
            getAccumulatedEventState();

            /** set the event type. */
            void
            setEventType( EventType Type )
            {
                _eventType = Type;
            }

            /** get the event type. */
            virtual EventType
            getEventType() const
            {
                return _eventType;
            }

            /** deprecated function for getting time of event. */
            double
            time() const
            {
                return _time;
            }

            void
            setGraphicsContext( osg::GraphicsContext* context )
            {
                _context = context;
            }

            osg::GraphicsContext*
            getGraphicsContext()
            {
                return _context.get();
            }

            const osg::GraphicsContext*
            getGraphicsContext() const
            {
                return _context.get();
            }

            /** set window rectangle. */
            void
            setWindowRectangle( int  x,
                                int  y,
                                int  width,
                                int  height,
                                bool updateMouseRange = true );

            /** set window x origin.*/
            void
            setWindowX( int v )
            {
                _windowX = v;
            }

            /** get window x origin.*/
            int
            getWindowX() const
            {
                return _windowX;
            }

            /** set window x origin.*/
            void
            setWindowY( int v )
            {
                _windowY = v;
            }

            /** get window y origin.*/
            int
            getWindowY() const
            {
                return _windowY;
            }

            /** set window width.*/
            void
            setWindowWidth( int v )
            {
                _windowWidth = v;
            }

            /** get window width.*/
            int
            getWindowWidth() const
            {
                return _windowWidth;
            }

            /** set window height.*/
            void
            setWindowHeight( int v )
            {
                _windowHeight = v;
            }

            /** get window height.*/
            int
            getWindowHeight() const
            {
                return _windowHeight;
            }

            /** set key pressed. */
            inline void
            setKey( int key )
            {
                _key = key;
            }

            /** get key pressed, return -1 if inappropriate for this GUIEventAdapter. */
            virtual int
            getKey() const
            {
                return _key;
            }

            /** set virtual key pressed. */
            void
            setUnmodifiedKey( int key )
            {
                _unmodifiedKey = key;
            }

            /** get virtual key pressed. */
            int
            getUnmodifiedKey() const
            {
                return _unmodifiedKey;
            }

            /** set button pressed/released.*/
            void
            setButton( int button )
            {
                _button = button;
            }

            /** button pressed/released, return -1 if inappropriate for this
             * GUIEventAdapter.*/
            int
            getButton() const
            {
                return _button;
            }

            /** set mouse input range. */
            void
            setInputRange( float Xmin,
                           float Ymin,
                           float Xmax,
                           float Ymax );

            /** set mouse minimum x. */
            void
            setXmin( float v )
            {
                _Xmin = v;
            }

            /** get mouse minimum x. */
            float
            getXmin() const
            {
                return _Xmin;
            }

            /** set mouse maximum x. */
            void
            setXmax( float v )
            {
                _Xmax = v;
            }

            /** get mouse maximum x. */
            float
            getXmax() const
            {
                return _Xmax;
            }

            /** set mouse minimum x. */
            void
            setYmin( float v )
            {
                _Ymin = v;
            }

            /** get mouse minimum y. */
            float
            getYmin() const
            {
                return _Ymin;
            }

            /** set mouse maximum y. */
            void
            setYmax( float v )
            {
                _Ymax = v;
            }

            /** get mouse maximum y. */
            float
            getYmax() const
            {
                return _Ymax;
            }

            /** set current mouse x position.*/
            void
            setX( float x )
            {
                _mx = x;
            }

            /** get current mouse x position.*/
            float
            getX() const
            {
                return _mx;
            }

            /** set current mouse y position.*/
            void
            setY( float y )
            {
                _my = y;
            }

            /** get current mouse y position.*/
            float
            getY() const
            {
                return _my;
            }

#if 1
            inline float
            getXnormalized() const
            {
                return _pointerDataList.size() >= 1
                         ? _pointerDataList[_pointerDataList.size() - 1]
                               ->getXnormalized()
                         : 2.0F *
                               ( getX() - getXmin() ) /
                               ( getXmax() - getXmin() ) -
                               1.0F;
            }

            inline float
            getYnormalized() const
            {
                if( _pointerDataList.size() >= 1 )
                {
                    return _pointerDataList[_pointerDataList.size() - 1]
                        ->getYnormalized();
                }
                if( _mouseYOrientation == Y_INCREASING_UPWARDS )
                {
                    return 2.0F *
                           ( getY() - getYmin() ) /
                           ( getYmax() - getYmin() ) -
                           1.0F;
                }
                else
                {
                    return -(
                        2.0F * ( getY() - getYmin() ) / ( getYmax() - getYmin() ) - 1.0F
                    );
                }
            }
#else
            /**
             * return the current mouse x value normalized to the range of -1 to 1.
             * -1 would be the left hand side of the window.
             * 0.0 would be the middle of the window.
             * +1 would be the right hand side of the window.
             */
            inline float
            getXnormalized() const
            {
                return 2.0F * ( getX() - getXmin() ) / ( getXmax() - getXmin() ) - 1.0F;
            }

            /**
             * return the current mouse y value normalized to the range of -1 to 1.
             * -1 would be the bottom of the window.
             * 0.0 would be the middle of the window.
             * +1 would be the top of the window.
             */
            inline float
            getYnormalized() const
            {
                if( _mouseYOrientation == Y_INCREASING_UPWARDS )
                {
                    return 2.0F *
                           ( getY() - getYmin() ) /
                           ( getYmax() - getYmin() ) -
                           1.0F;
                }
                else
                {
                    return -(
                        2.0F * ( getY() - getYmin() ) / ( getYmax() - getYmin() ) - 1.0F
                    );
                }
            }
#endif

            /// set mouse-Y orientation (mouse-Y increases upwards or downwards).
            void
            setMouseYOrientation( MouseYOrientation myo )
            {
                _mouseYOrientation = myo;
            }

            /// get mouse-Y orientation (mouse-Y increases upwards or downwards).
            MouseYOrientation
            getMouseYOrientation() const
            {
                return _mouseYOrientation;
            }

            /// set mouse-Y orientation (mouse-Y increases upwards or downwards) and
            /// recompute variables
            void
            setMouseYOrientationAndUpdateCoords( MouseYOrientation myo );

            /// set current mouse button state.
            void
            setButtonMask( int mask )
            {
                _buttonMask = mask;
            }

            /// get current mouse button state.
            int
            getButtonMask() const
            {
                return _buttonMask;
            }

            /// set modifier key mask.
            void
            setModKeyMask( int mask )
            {
                _modKeyMask = mask;
            }

            /// get modifier key mask.
            int
            getModKeyMask() const
            {
                return _modKeyMask;
            }

            /// set scrolling motion (for EventType::SCROLL).
            void
            setScrollingMotion( ScrollingMotion motion )
            {
                _scrolling.motion = motion;
            }

            /// get scrolling motion (for EventType::SCROLL).
            ScrollingMotion
            getScrollingMotion() const
            {
                return _scrolling.motion;
            }

            /// set the scrolling delta to x,y and the scrolling motion to SCROLL_2D.
            void
            setScrollingMotionDelta( float x,
                                     float y )
            {
                _scrolling.motion = SCROLL_2D;
                _scrolling.deltaX = x;
                _scrolling.deltaY = y;
            }

            /// set the scrolling x-delta.
            void
            setScrollingDeltaX( float v )
            {
                _scrolling.deltaX = v;
            }

            /// get the scrolling x-delta.
            float
            getScrollingDeltaX() const
            {
                return _scrolling.deltaX;
            }

            /// set the scrolling y-delta.
            void
            setScrollingDeltaY( float v )
            {
                _scrolling.deltaY = v;
            }

            /// get the scrolling y-delta.
            float
            getScrollingDeltaY() const
            {
                return _scrolling.deltaY;
            }

            /// set the tablet pen pressure (range 0..1).
            void
            setPenPressure( float pressure )
            {
                _tabletPen.pressure = pressure;
            }

            /// get the tablet pen pressure (range 0..1).
            float
            getPenPressure() const
            {
                return _tabletPen.pressure;
            }

            /// set the tablet pen tiltX in degrees.
            void
            setPenTiltX( float tiltX )
            {
                _tabletPen.tiltX = tiltX;
            }

            /// get the tablet pen tiltX in degrees.
            float
            getPenTiltX() const
            {
                return _tabletPen.tiltX;
            }

            /// set the tablet pen tiltY in degrees.
            void
            setPenTiltY( float tiltY )
            {
                _tabletPen.tiltY = tiltY;
            }

            /// get the tablet pen tiltY in degrees.
            float
            getPenTiltY() const
            {
                return _tabletPen.tiltY;
            }

            /// set the tablet pen rotation around the Z-axis in degrees.
            void
            setPenRotation( float rotation )
            {
                _tabletPen.rotation = rotation;
            }

            /// get the tablet pen rotation around the Z-axis in degrees.
            float
            getPenRotation() const
            {
                return _tabletPen.rotation;
            }

            /// set the tablet pointer type.
            void
            setTabletPointerType( TabletPointerType pt )
            {
                _tabletPen.tabletPointerType = pt;
            }

            /// get the tablet pointer type.
            TabletPointerType
            getTabletPointerType() const
            {
                return _tabletPen.tabletPointerType;
            }

            /// set the orientation from a tablet input device as a matrix.
            const osg::dmat4
            getPenOrientation() const;

            void
            addTouchPoint( unsigned int id,
                           TouchPhase   phase,
                           float        x,
                           float        y,
                           unsigned int tapCount = 0 );

            void
            setTouchData( TouchData* td )
            {
                _touchData = td;
            }

            TouchData*
            getTouchData() const
            {
                return _touchData.get();
            }

            bool
            isMultiTouchEvent() const
            {
                return ( _touchData.valid() );
            }

            inline float
            getTouchPointNormalizedX( unsigned int ndx ) const
            {
                return ( getTouchData()->get( ndx ).x - _Xmin ) /
                       ( _Xmax - _Xmin ) *
                       2.0F -
                       1.0F;
            }

            inline float
            getTouchPointNormalizedY( unsigned int ndx ) const
            {
                if( _mouseYOrientation == Y_INCREASING_UPWARDS )
                {
                    return ( getTouchData()->get( ndx ).y - _Ymin ) /
                           ( _Ymax - _Ymin ) *
                           2.0F -
                           1.0F;
                }
                else
                {
                    return -( ( getTouchData()->get( ndx ).y - _Ymin ) /
                              ( _Ymax - _Ymin ) *
                              2.0F -
                              1.0F );
                }
            }

            typedef std::vector<osg::ref_ptr<PointerData>> PointerDataList;

            void
            setPointerDataList( const PointerDataList& pdl )
            {
                _pointerDataList = pdl;
            }

            PointerDataList&
            getPointerDataList()
            {
                return _pointerDataList;
            }

            const PointerDataList&
            getPointerDataList() const
            {
                return _pointerDataList;
            }

            unsigned int
            getNumPointerData() const
            {
                return static_cast<unsigned int>( _pointerDataList.size() );
            }

            PointerData*
            getPointerData( unsigned int i )
            {
                return _pointerDataList[i].get();
            }

            const PointerData*
            getPointerData( unsigned int i ) const
            {
                return _pointerDataList[i].get();
            }

            PointerData*
            getPointerData( osg::Object* obj )
            {
                for( unsigned int i = 0; i < _pointerDataList.size(); ++i )
                {
                    if( _pointerDataList[i]->object == obj )
                    {
                        return _pointerDataList[i].get();
                    }
                }
                return 0;
            }

            const PointerData*
            getPointerData( osg::Object* obj ) const
            {
                for( unsigned int i = 0; i < _pointerDataList.size(); ++i )
                {
                    if( _pointerDataList[i]->object == obj )
                    {
                        return _pointerDataList[i].get();
                    }
                }
                return 0;
            }

            void
            addPointerData( PointerData* pd )
            {
                _pointerDataList.push_back( pd );
            }

            void
            copyPointerDataFrom( const osgGA::GUIEventAdapter& sourceEvent );

        protected:

            /** Force users to create on heap, so that multiple referencing is safe.*/
            virtual ~GUIEventAdapter();

            EventType                               _eventType;

            osg::observer_ptr<osg::GraphicsContext> _context;
            int                                     _windowX;
            int                                     _windowY;
            int                                     _windowWidth;
            int                                     _windowHeight;
            int                                     _key;
            int                                     _unmodifiedKey;
            int                                     _button;
            float                                   _Xmin, _Xmax;
            float                                   _Ymin, _Ymax;
            float                                   _mx;
            float                                   _my;
            int                                     _buttonMask;
            int                                     _modKeyMask;
            MouseYOrientation                       _mouseYOrientation;

            struct Scrolling
            {
                    ScrollingMotion motion;
                    float           deltaX;
                    float           deltaY;

                    Scrolling() :
                        motion( SCROLL_NONE ),
                        deltaX( 0 ),
                        deltaY( 0 )
                    {
                    }

                    Scrolling( const Scrolling& rhs ) :
                        motion( rhs.motion ),
                        deltaX( rhs.deltaX ),
                        deltaY( rhs.deltaY )
                    {
                    }
            };

            Scrolling _scrolling;

            struct TabletPen
            {
                    float             pressure;
                    float             tiltX;
                    float             tiltY;
                    float             rotation;
                    TabletPointerType tabletPointerType;

                    TabletPen() :
                        pressure( 0 ),
                        tiltX( 0 ),
                        tiltY( 0 ),
                        rotation( 0 ),
                        tabletPointerType( UNKNOWN )
                    {
                    }

                    TabletPen( const TabletPen& rhs ) :
                        pressure( rhs.pressure ),
                        tiltX( rhs.tiltX ),
                        tiltY( rhs.tiltY ),
                        rotation( rhs.rotation ),
                        tabletPointerType( rhs.tabletPointerType )
                    {
                    }
            };

            TabletPen               _tabletPen;

            osg::ref_ptr<TouchData> _touchData;

            PointerDataList         _pointerDataList;
    };

}
