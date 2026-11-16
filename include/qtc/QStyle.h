// QtClone - QStyle.h : widget style (drawing primitives) and style sheets.
#ifndef QTC_QSTYLE_H
#define QTC_QSTYLE_H

#include <qtc/QWidget.h>

// ---------------------------------------------------------------------------
// style options (kept deliberately small and explicit)
// ---------------------------------------------------------------------------
struct QStyleOption {
    QStyleOption() : state(0), rect() {}
    int state;
    QRect rect;
};

struct QStyleOptionButton : QStyleOption {
    QStyleOptionButton() : checked(false), down(false), defaultButton(false), hasIcon(false) {}
    QString text;
    QIcon icon;
    bool checked;
    bool down;
    bool defaultButton;
    bool hasIcon;
};

struct QStyleOptionToolButton : QStyleOptionButton {
    Qt::ToolButtonStyle toolStyle;
    QStyleOptionToolButton() : toolStyle(Qt::ToolButtonTextBesideIcon) {}
};

struct QStyleOptionFrame : QStyleOption {
    QStyleOptionFrame() : lineWidth(1), rounded(false) {}
    int lineWidth;
    bool rounded;
    QString placeholder;
    QString text;
    int alignment;
};

struct QStyleOptionProgressBar : QStyleOption {
    QStyleOptionProgressBar() : minimum(0), maximum(100), value(0), textVisible(true), vertical(false) {}
    int minimum;
    int maximum;
    int value;
    bool textVisible;
    bool vertical;
    QString text;
};

struct QStyleOptionSlider : QStyleOption {
    QStyleOptionSlider() : minimum(0), maximum(100), value(0), orientation(Qt::Horizontal),
                           singleStep(1), pageStep(10) {}
    int minimum;
    int maximum;
    int value;
    Qt::Orientation orientation;
    int singleStep;
    int pageStep;
};

struct QStyleOptionScrollBar : QStyleOption {
    QStyleOptionScrollBar() : minimum(0), maximum(100), value(0), pageStep(10),
                              orientation(Qt::Vertical), pressedPart(0), hoverPart(0) {}
    int minimum;
    int maximum;
    int value;
    int pageStep;
    Qt::Orientation orientation;
    int pressedPart;
    int hoverPart;
};

struct QStyleOptionComboBox : QStyleOption {
    QStyleOptionComboBox() : open(false), editable(false) {}
    QString text;
    bool open;
    bool editable;
};

struct QStyleOptionTab : QStyleOption {
    enum TabPosition { Beginning, Middle, End, OnlyOne };
    QStyleOptionTab() : selected(false), position(OnlyOne), hovered(false) {}
    QString text;
    bool selected;
    TabPosition position;
    bool hovered;
};

struct QStyleOptionHeader : QStyleOption {
    QStyleOptionHeader() : sortIndicator(0), pressed(false), hovered(false) {}
    QString text;
    int sortIndicator;   // 0 none, 1 ascending, 2 descending
    bool pressed;
    bool hovered;
};

struct QStyleOptionMenuItem : QStyleOption {
    QStyleOptionMenuItem()
        : checked(false), separator(false), subMenu(false), active(false), enabled(true),
          checkable(false), shortcut(QString()), iconVisible(false) {}
    QString text;
    QString shortcut;
    bool checked;
    bool separator;
    bool subMenu;
    bool active;
    bool enabled;
    bool checkable;
    bool iconVisible;
};

struct QStyleOptionViewItem : QStyleOption {
    QStyleOptionViewItem() : selected(false), hovered(false), alternate(false),
                             hasIcon(false), checkState(0) {}
    QString text;
    bool selected;
    bool hovered;
    bool alternate;
    Qt::Alignment alignment;
    bool hasIcon;
    QIcon icon;
    int checkState;     // 0 none, 1 checked, 2 partial
};

struct QStyleOptionGroupBox : QStyleOption {
    QString text;
};

// ---------------------------------------------------------------------------
// QStyle
// ---------------------------------------------------------------------------
class QStyle {
public:
    enum StateFlag {
        State_None = 0,
        State_Enabled = 0x01,
        State_Hover = 0x02,
        State_Sunken = 0x04,
        State_On = 0x08,
        State_Off = 0x10,
        State_NoChange = 0x20,
        State_HasFocus = 0x40,
        State_Selected = 0x80,
        State_Default = 0x100,
        State_Raised = 0x400,
        State_Disabled = 0x200
    };

    QStyle();
    virtual ~QStyle();

    virtual QString name() const { return QString(L"QtClone Fusion"); }

    virtual void drawButton(QPainter* p, const QStyleOptionButton& opt, const QWidget* w) const;
    virtual void drawCheckBox(QPainter* p, const QStyleOptionButton& opt, const QWidget* w) const;
    virtual void drawRadioButton(QPainter* p, const QStyleOptionButton& opt, const QWidget* w) const;
    virtual void drawToolButton(QPainter* p, const QStyleOptionToolButton& opt, const QWidget* w) const;
    virtual void drawLineEditFrame(QPainter* p, const QStyleOptionFrame& opt, const QWidget* w) const;
    virtual void drawFrame(QPainter* p, const QStyleOptionFrame& opt, const QWidget* w) const;
    virtual void drawPanel(QPainter* p, const QStyleOption& opt, const QWidget* w) const;
    virtual void drawGroupBox(QPainter* p, const QStyleOptionGroupBox& opt, const QWidget* w) const;
    virtual void drawProgressBar(QPainter* p, const QStyleOptionProgressBar& opt, const QWidget* w) const;
    virtual void drawSlider(QPainter* p, const QStyleOptionSlider& opt, const QWidget* w) const;
    virtual void drawScrollBar(QPainter* p, const QStyleOptionScrollBar& opt, const QWidget* w) const;
    virtual void drawComboBox(QPainter* p, const QStyleOptionComboBox& opt, const QWidget* w) const;
    virtual void drawTab(QPainter* p, const QStyleOptionTab& opt, const QWidget* w) const;
    virtual void drawHeader(QPainter* p, const QStyleOptionHeader& opt, const QWidget* w) const;
    virtual void drawMenuItem(QPainter* p, const QStyleOptionMenuItem& opt, const QWidget* w) const;
    virtual void drawViewItem(QPainter* p, const QStyleOptionViewItem& opt, const QWidget* w) const;
    virtual void drawFocusRect(QPainter* p, const QRect& rect, const QWidget* w) const;
    virtual void drawToolTip(QPainter* p, const QStyleOption& opt, const QWidget* w) const;
    virtual void drawArrow(QPainter* p, const QRect& rect, Qt::ArrowType type, const QColor& color) const;
    virtual void drawCheckMark(QPainter* p, const QRect& rect, const QColor& color) const;
    virtual void drawCopyIcon(QPainter* p, const QRect& rect, const QColor& color) const;

    virtual int textHMargin() const { return 8; }
    virtual int textVMargin() const { return 3; }
    virtual QSize sizeFromContents(const QStyleOptionButton& opt, const QSize& textSize) const;
    virtual int sliderHandleWidth() const { return 14; }
    virtual int scrollBarExtent() const { return 13; }

    virtual void polish(QWidget* widget);
    virtual void unpolish(QWidget* widget);

    static QStyle* createDefault();
};

// ---------------------------------------------------------------------------
// QStyleSheet : a practical subset of Qt style sheets
// ---------------------------------------------------------------------------
class QStyleSheet {
public:
    struct Box {
        Box()
            : hasBackground(false), hasBorder(false), borderWidth(1), radius(0),
              hasColor(false), hasFont(false), hasPadding(false),
              padLeft(0), padTop(0), padRight(0), padBottom(0),
              hasMinWidth(false), hasMinHeight(false), minWidth(0), minHeight(0),
              hasMaxWidth(false), hasMaxHeight(false), maxWidth(0), maxHeight(0),
              hasSelection(false) {}
        bool hasBackground;
        QColor background;
        bool hasBorder;
        QColor borderColor;
        int borderWidth;
        int radius;
        bool hasColor;
        QColor color;
        bool hasFont;
        QFont font;
        bool hasPadding;
        int padLeft, padTop, padRight, padBottom;
        bool hasMinWidth, hasMinHeight;
        int minWidth, minHeight;
        bool hasMaxWidth, hasMaxHeight;
        int maxWidth, maxHeight;
        bool hasSelection;
        QColor selectionBackground;
        QColor selectionColor;
        bool empty() const {
            return !hasBackground && !hasBorder && !hasColor && !hasFont && !hasPadding &&
                   !hasMinWidth && !hasMinHeight && !hasMaxWidth && !hasMaxHeight && !hasSelection;
        }
    };

    static void setApplicationSheet(const QString& sheet);
    static const QString& applicationSheet();
    static void invalidate();
    static void invalidate(const QWidget* widget);
    static const Box& boxFor(const QWidget* widget);

    // draws background + border described by the box
    static void paintBackground(QPainter* painter, const QWidget* widget, const Box& box);

    // resolves a colour the way QSS would (falls back to the palette)
    static QColor resolveColor(const QWidget* widget, QPalette::ColorRole role);
};

#endif // QTC_QSTYLE_H
