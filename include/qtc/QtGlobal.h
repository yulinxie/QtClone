// QtClone - a Qt-style UI framework for C++ (VS2010 / MSVC 10 compatible)
// QtGlobal.h : global types, Qt:: enums, QFlags, debug helpers.
#ifndef QTC_QTGlobal_H
#define QTC_QTGlobal_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <wchar.h>
#include <string>

#define QTC_VERSION_STR "1.0.0"
#define QTC_VERSION 0x010000

// ---------------------------------------------------------------------------
// basic types
// ---------------------------------------------------------------------------
typedef signed char   qint8;
typedef unsigned char quint8;
typedef short         qint16;
typedef unsigned short quint16;
typedef int           qint32;
typedef unsigned int  quint32;
typedef __int64       qint64;
typedef unsigned __int64 quint64;
typedef double        qreal;
typedef unsigned char uchar;
typedef unsigned int  uint;
typedef unsigned long ulong;
typedef void*         WId;

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------
#define Q_UNUSED(x) (void)x;
#define Q_ASSERT(cond) assert(cond)
#define Q_ASSERT_X(cond, where, what) assert(cond)
#define Q_EMIT
#define emit
#define signals public
#define slots
#define Q_SIGNALS public
#define Q_SLOTS

template <class T> inline const T& qMin(const T& a, const T& b) { return (a < b) ? a : b; }
template <class T> inline const T& qMax(const T& a, const T& b) { return (a < b) ? b : a; }
template <class T> inline const T& qBound(const T& min, const T& val, const T& max)
{ return qMax(min, qMin(max, val)); }
template <class T> inline T qAbs(const T& t) { return t >= T(0) ? t : -t; }
template <class T> inline void qSwap(T& a, T& b) { T t = a; a = b; b = t; }
inline int qRound(qreal d) { return d >= 0.0 ? int(d + 0.5) : int(d - 0.5); }
inline int qFloor(qreal d) { int i = int(d); return (d < 0.0 && qreal(i) != d) ? i - 1 : i; }
inline int qCeil(qreal d) { int i = int(d); return (d > 0.0 && qreal(i) != d) ? i + 1 : i; }
inline bool qFuzzyCompare(qreal p1, qreal p2) { return qAbs(p1 - p2) <= 0.000000000001 * qMin(qAbs(p1), qAbs(p2)); }
inline bool qIsNull(qreal d) { return d >= -0.000000000001 && d <= 0.000000000001; }
inline void qUnusedParameter(...) {}

// debug output -> OutputDebugStringW + stderr (framework internal)
void qtcDebugMessage(const wchar_t* msg);

#define qDebug() qtcDebugStream()
#define qWarning() qtcDebugStream()
#define qCritical() qtcDebugStream()

class qtcDebugStream {
public:
    qtcDebugStream();
    ~qtcDebugStream();
    qtcDebugStream& operator<<(const wchar_t* s);
    qtcDebugStream& operator<<(const std::wstring& s);
    qtcDebugStream& operator<<(const char* s);
    qtcDebugStream& operator<<(int v);
    qtcDebugStream& operator<<(unsigned int v);
    qtcDebugStream& operator<<(qint64 v);
    qtcDebugStream& operator<<(double v);
private:
    std::wstring buf_;
};

// ---------------------------------------------------------------------------
// QFlags
// ---------------------------------------------------------------------------
template <typename Enum>
class QFlags {
public:
    typedef int Int;
    QFlags() : i(0) {}
    QFlags(Enum e) : i(Int(e)) {}
    QFlags(Int i_) : i(i_) {}
    operator Int() const { return i; }
    QFlags& operator|=(Enum e) { i |= Int(e); return *this; }
    QFlags& operator|=(QFlags f) { i |= f.i; return *this; }
    QFlags& operator&=(Enum e) { i &= Int(e); return *this; }
    QFlags& operator&=(QFlags f) { i &= f.i; return *this; }
    bool testFlag(Enum e) const { return (i & Int(e)) == Int(e); }
    Int toInt() const { return i; }
    Int i;
};

#define Q_DECLARE_FLAGS(Flags, Enum) typedef QFlags<Enum> Flags;

template <typename Enum>
inline QFlags<Enum> operator|(Enum a, Enum b) { return QFlags<Enum>(int(a) | int(b)); }
template <typename Enum>
inline QFlags<Enum> operator|(Enum a, QFlags<Enum> b) { return QFlags<Enum>(int(a) | b.i); }
template <typename Enum>
inline QFlags<Enum> operator|(QFlags<Enum> a, Enum b) { return QFlags<Enum>(a.i | int(b)); }
template <typename Enum>
inline QFlags<Enum> operator|(QFlags<Enum> a, QFlags<Enum> b) { return QFlags<Enum>(a.i | b.i); }
template <typename Enum>
inline QFlags<Enum> operator&(Enum a, Enum b) { return QFlags<Enum>(int(a) & int(b)); }
template <typename Enum>
inline QFlags<Enum> operator&(Enum a, QFlags<Enum> b) { return QFlags<Enum>(int(a) & b.i); }
template <typename Enum>
inline QFlags<Enum> operator&(QFlags<Enum> a, Enum b) { return QFlags<Enum>(a.i & int(b)); }
template <typename Enum>
inline QFlags<Enum> operator&(QFlags<Enum> a, QFlags<Enum> b) { return QFlags<Enum>(a.i & b.i); }
template <typename Enum>
inline QFlags<Enum> operator~(QFlags<Enum> a) { return QFlags<Enum>(~a.i); }
template <typename Enum>
inline QFlags<Enum> operator~(Enum a) { return QFlags<Enum>(~int(a)); }

// ---------------------------------------------------------------------------
// namespace Qt
// ---------------------------------------------------------------------------
namespace Qt {

enum GlobalColor {
    color0 = 0, color1 = 1,
    black = 2, white = 3,
    darkGray = 4, gray = 5, lightGray = 6,
    red = 7, green = 8, blue = 9,
    cyan = 10, magenta = 11, yellow = 12,
    darkRed = 13, darkGreen = 14, darkBlue = 15,
    darkCyan = 16, darkMagenta = 17, darkYellow = 18,
    transparent = 19
};

enum AlignmentFlag {
    AlignLeft     = 0x0001,
    AlignRight    = 0x0002,
    AlignHCenter  = 0x0004,
    AlignJustify  = 0x0008,
    AlignAbsolute = 0x0010,
    AlignTop      = 0x0020,
    AlignBottom   = 0x0040,
    AlignVCenter  = 0x0080,
    AlignCenter   = AlignHCenter | AlignVCenter,
    AlignHorizontal_Mask = AlignLeft | AlignRight | AlignHCenter | AlignJustify,
    AlignVertical_Mask = AlignTop | AlignBottom | AlignVCenter
};
Q_DECLARE_FLAGS(Alignment, AlignmentFlag)

enum Orientation { Horizontal = 0x1, Vertical = 0x2 };
Q_DECLARE_FLAGS(Orientations, Orientation)

enum TextElideMode { ElideLeft, ElideRight, ElideMiddle, ElideNone };
enum TextFormat { PlainText, RichText, AutoText };
enum TextFlag {
    TextSingleLine = 0x0100,
    TextDontClip = 0x0200,
    TextExpandTabs = 0x0400,
    TextShowMnemonic = 0x0800,
    TextWordWrap = 0x1000,
    TextWrapAnywhere = 0x2000,
    TextDontPrint = 0x4000,
    TextHideMnemonic = 0x8000,
    TextJustificationForced = 0x10000
};
enum AspectRatioMode { IgnoreAspectRatio, KeepAspectRatio, KeepAspectRatioByExpanding };
enum TransformationMode { FastTransformation, SmoothTransformation };

enum PenStyle {
    NoPen = 0, SolidLine = 1, DashLine = 2, DotLine = 3,
    DashDotLine = 4, DashDotDotLine = 5, CustomDashLine = 6
};
enum PenCapStyle { FlatCap = 0x00, SquareCap = 0x10, RoundCap = 0x20 };
enum PenJoinStyle { MiterJoin = 0x00, BevelJoin = 0x40, RoundJoin = 0x80, SvgMiterJoin = 0x100 };

enum BrushStyle {
    NoBrush = 0, SolidPattern = 1, Dense1Pattern = 2, Dense2Pattern = 3,
    Dense3Pattern = 4, Dense4Pattern = 5, Dense5Pattern = 6, Dense6Pattern = 7,
    Dense7Pattern = 8, HorPattern = 9, VerPattern = 10, CrossPattern = 11,
    BDiagPattern = 12, FDiagPattern = 13, DiagCrossPattern = 14,
    LinearGradientPattern = 15, RadialGradientPattern = 16, ConicalGradientPattern = 17,
    NoPattern = 0
};

enum BGMode { TransparentMode = 0, OpaqueMode = 1 };
enum FillRule { OddEvenFill = 0, WindingFill = 1 };
enum ArrowType { NoArrow, UpArrow, DownArrow, LeftArrow, RightArrow };

enum FocusPolicy {
    NoFocus = 0x0,
    TabFocus = 0x1,
    ClickFocus = 0x2,
    StrongFocus = TabFocus | ClickFocus | 0x8,
    WheelFocus = StrongFocus | 0x4
};

enum MouseButton {
    NoButton = 0,
    LeftButton = 1,
    RightButton = 2,
    MidButton = 4,
    MiddleButton = MidButton,
    XButton1 = 8,
    XButton2 = 16,
    MouseButtonMask = 0xff
};
Q_DECLARE_FLAGS(MouseButtons, MouseButton)

enum KeyboardModifier {
    NoModifier = 0x00000000,
    ShiftModifier = 0x02000000,
    ControlModifier = 0x04000000,
    AltModifier = 0x08000000,
    MetaModifier = 0x10000000,
    KeypadModifier = 0x20000000,
    GroupSwitchModifier = 0x40000000,
    KeyboardModifierMask = 0xfe000000
};
Q_DECLARE_FLAGS(KeyboardModifiers, KeyboardModifier)

enum Key {
    Key_Escape = 0x01000000,
    Key_Tab = 0x01000001,
    Key_Backtab = 0x01000002,
    Key_Backspace = 0x01000003,
    Key_Return = 0x01000004,
    Key_Enter = 0x01000005,
    Key_Insert = 0x01000006,
    Key_Delete = 0x01000007,
    Key_Pause = 0x01000008,
    Key_Print = 0x01000009,
    Key_SysReq = 0x0100000a,
    Key_Clear = 0x0100000b,
    Key_Home = 0x01000010,
    Key_End = 0x01000011,
    Key_Left = 0x01000012,
    Key_Up = 0x01000013,
    Key_Right = 0x01000014,
    Key_Down = 0x01000015,
    Key_PageUp = 0x01000016,
    Key_PageDown = 0x01000017,
    Key_Shift = 0x01000020,
    Key_Control = 0x01000021,
    Key_Meta = 0x01000022,
    Key_Alt = 0x01000023,
    Key_CapsLock = 0x01000024,
    Key_NumLock = 0x01000025,
    Key_ScrollLock = 0x01000026,
    Key_F1 = 0x01000030,
    Key_F2 = 0x01000031,
    Key_F3 = 0x01000032,
    Key_F4 = 0x01000033,
    Key_F5 = 0x01000034,
    Key_F6 = 0x01000035,
    Key_F7 = 0x01000036,
    Key_F8 = 0x01000037,
    Key_F9 = 0x01000038,
    Key_F10 = 0x01000039,
    Key_F11 = 0x0100003a,
    Key_F12 = 0x0100003b,
    Key_Space = 0x20,
    Key_Exclam = 0x21,
    Key_QuoteDbl = 0x22,
    Key_NumberSign = 0x23,
    Key_Dollar = 0x24,
    Key_Percent = 0x25,
    Key_Ampersand = 0x26,
    Key_Apostrophe = 0x27,
    Key_ParenLeft = 0x28,
    Key_ParenRight = 0x29,
    Key_Asterisk = 0x2a,
    Key_Plus = 0x2b,
    Key_Comma = 0x2c,
    Key_Minus = 0x2d,
    Key_Period = 0x2e,
    Key_Slash = 0x2f,
    Key_0 = 0x30, Key_1 = 0x31, Key_2 = 0x32, Key_3 = 0x33, Key_4 = 0x34,
    Key_5 = 0x35, Key_6 = 0x36, Key_7 = 0x37, Key_8 = 0x38, Key_9 = 0x39,
    Key_Colon = 0x3a,
    Key_Semicolon = 0x3b,
    Key_Less = 0x3c,
    Key_Equal = 0x3d,
    Key_Greater = 0x3e,
    Key_Question = 0x3f,
    Key_At = 0x40,
    Key_A = 0x41, Key_B = 0x42, Key_C = 0x43, Key_D = 0x44, Key_E = 0x45,
    Key_F = 0x46, Key_G = 0x47, Key_H = 0x48, Key_I = 0x49, Key_J = 0x4a,
    Key_K = 0x4b, Key_L = 0x4c, Key_M = 0x4d, Key_N = 0x4e, Key_O = 0x4f,
    Key_P = 0x50, Key_Q = 0x51, Key_R = 0x52, Key_S = 0x53, Key_T = 0x54,
    Key_U = 0x55, Key_V = 0x56, Key_W = 0x57, Key_X = 0x58, Key_Y = 0x59,
    Key_Z = 0x5a,
    Key_BracketLeft = 0x5b,
    Key_Backslash = 0x5c,
    Key_BracketRight = 0x5d,
    Key_AsciiCircum = 0x5e,
    Key_Underscore = 0x5f,
    Key_QuoteLeft = 0x60,
    Key_BraceLeft = 0x7b,
    Key_Bar = 0x7c,
    Key_BraceRight = 0x7d,
    Key_AsciiTilde = 0x7e,
    Key_unknown = 0x01ffffff
};

enum WindowType {
    Widget = 0x00000000,
    Window = 0x00000001,
    Dialog = 0x00000002 | Window,
    Sheet = 0x00000004 | Window,
    Drawer = Sheet | Dialog,
    Popup = 0x00000008 | Window,
    Tool = Popup | Dialog,
    ToolTip = Popup | Sheet,
    SplashScreen = ToolTip | Dialog,
    Desktop = 0x00000010 | Window,
    SubWindow = 0x00000012,
    ForeignWindow = 0x00000020 | Window,
    CoverWindow = 0x00000040 | Window,
    WindowType_Mask = 0x000000ff,
    MSWindowsFixedSizeDialogHint = 0x00000100,
    MSWindowsOwnDC = 0x00000200,
    X11BypassWindowManagerHint = 0x00000400,
    FramelessWindowHint = 0x00000800,
    NoDropShadowWindowHint = 0x40000000,
    WindowTitleHint = 0x00001000,
    WindowSystemMenuHint = 0x00002000,
    WindowMinimizeButtonHint = 0x00004000,
    WindowMaximizeButtonHint = 0x00008000,
    WindowMinMaxButtonsHint = WindowMinimizeButtonHint | WindowMaximizeButtonHint,
    WindowCloseButtonHint = 0x08000000,
    WindowContextHelpButtonHint = 0x00010000,
    WindowShadeButtonHint = 0x00020000,
    WindowStaysOnTopHint = 0x00040000,
    WindowStaysOnBottomHint = 0x04000000,
    CustomizeWindowHint = 0x02000000
};
Q_DECLARE_FLAGS(WindowFlags, WindowType)

enum WidgetAttribute {
    WA_None = 0,
    WA_DeleteOnClose = 1,
    WA_StaticContents,
    WA_OpaquePaintEvent,
    WA_NoSystemBackground,
    WA_TransparentForMouseEvents,
    WA_Resized,
    WA_Moved,
    WA_Hover,
    WA_UnderMouse,
    WA_Hidden,
    WA_Disabled,
    WA_SetFont,
    WA_SetPalette,
    WA_SetStyle,
    WA_StyledBackground,
    WA_KeyboardFocusChange,
    WA_AlwaysShowToolTips,
    WA_AttributeCount
};
Q_DECLARE_FLAGS(WidgetAttributes, WidgetAttribute)

enum ItemDataRole {
    DisplayRole = 0,
    DecorationRole = 1,
    EditRole = 2,
    ToolTipRole = 3,
    StatusTipRole = 4,
    WhatsThisRole = 5,
    FontRole = 6,
    TextAlignmentRole = 7,
    BackgroundRole = 8,
    ForegroundRole = 9,
    CheckStateRole = 10,
    UserRole = 0x0100
};

enum CheckState { Unchecked = 0, PartiallyChecked = 1, Checked = 2 };

enum ItemFlag {
    NoItemFlags = 0,
    ItemIsSelectable = 1,
    ItemIsEditable = 2,
    ItemIsDragEnabled = 4,
    ItemIsDropEnabled = 8,
    ItemIsUserCheckable = 16,
    ItemIsEnabled = 32,
    ItemIsTristate = 64
};
Q_DECLARE_FLAGS(ItemFlags, ItemFlag)

enum SortOrder { AscendingOrder, DescendingOrder };
enum ScrollBarPolicy { ScrollBarAsNeeded, ScrollBarAlwaysOff, ScrollBarAlwaysOn };
enum ButtonSymbols { NoSymbol, UpDownArrows, PlusMinus };
enum ToolButtonStyle {
    ToolButtonIconOnly, ToolButtonTextOnly,
    ToolButtonTextBesideIcon, ToolButtonTextUnderIcon,
    ToolButtonFollowStyle
};
enum CursorShape {
    ArrowCursor, UpArrowCursor, CrossCursor, WaitCursor, IBeamCursor,
    SizeVerCursor, SizeHorCursor, SizeBDiagCursor, SizeFDiagCursor,
    SizeAllCursor, BlankCursor, SplitVCursor, SplitHCursor,
    PointingHandCursor, OpenHandCursor, ClosedHandCursor,
    WhatsThisCursor, BusyCursor, DragCopyCursor, DragMoveCursor, DragLinkCursor
};

enum MethodType { Method = 0, Signal = 1, Slot = 2, Constructor = 3, MethodTypeCount };
enum ConnectionType { AutoConnection, DirectConnection, QueuedConnection, BlockingQueuedConnection, UniqueConnection = 0x80 };
enum ShortcutContext { WidgetShortcut, WindowShortcut, ApplicationShortcut };

enum SizeMode { AbsoluteSize = 0, RelativeSize = 1 };
enum FocusReason {
    MouseFocusReason,
    TabFocusReason,
    BacktabFocusReason,
    ActiveWindowFocusReason,
    PopupFocusReason,
    ShortcutFocusReason,
    MenuBarFocusReason,
    OtherFocusReason
};
enum SliderAction { SliderNoAction, SliderSingleStepAdd, SliderSingleStepSub, SliderPageStepAdd, SliderPageStepSub, SliderToMinimum, SliderToMaximum, SliderMove };

} // namespace Qt

#endif // QTC_QTGlobal_H
