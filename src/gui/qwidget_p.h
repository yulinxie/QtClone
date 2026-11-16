// QtClone - internal widget/window structures (not installed).
#ifndef QTC_QWIDGET_P_H
#define QTC_QWIDGET_P_H

#include <qtc/QWidget.h>
#include <qtc/QStyle.h>
#include <qtc/QLayout.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <set>
#include <vector>

// ---------------------------------------------------------------------------
// QWidgetPrivate
// ---------------------------------------------------------------------------
struct QWidgetPrivate {
    QWidget* q;
    QWidget* parent;
    QWidget* focusProxy;
    QList<QWidget*> children;

    QRect geometry;            // relative to the parent's client area
    QSize minSize;
    QSize maxSize;
    QSizePolicy sizePolicy;
    QMargins margins;

    Qt::WindowFlags flags;
    QFlags<Qt::WidgetAttribute> attrs;

    bool visible;
    bool explicitHide;
    bool enabled;
    bool underMouse;
    bool mouseTracking;
    bool down;
    bool autoFill;
    bool hasFocus;
    bool hasFont;
    bool hasPalette;
    bool hasStyleSheet;
    bool hasStyle;
    bool hasCursor;
    bool created;
    bool inPaint;
    bool inLayout;
    bool layoutRequested;
    bool activeWindow;
    bool modal;
    bool showPending;
    bool stretchWidth;

    QFont font;
    QPalette palette;
    QString styleSheet;
    QString toolTip;
    QString statusTip;
    QString windowTitle;
    QIcon windowIcon;
    QCursor cursor;
    Qt::FocusPolicy focusPolicy;
    QLayout* layout;
    QStyle* style;
    QWidgetWindow* window;
    QWidgetWindow* ownerWindow;

    QStyleSheet::Box qss;
    bool qssDirty;

    QWidgetPrivate()
        : q(0), parent(0), focusProxy(0), minSize(0, 0), maxSize(0x7fffffff, 0x7fffffff),
          visible(false), explicitHide(false), enabled(true), underMouse(false), mouseTracking(false), down(false),
          autoFill(false), hasFocus(false), hasFont(false), hasPalette(false),
          hasStyleSheet(false), hasStyle(false), hasCursor(false), created(false),
          inPaint(false), inLayout(false), layoutRequested(false), activeWindow(false),
          modal(false), showPending(false), stretchWidth(false),
          focusPolicy(Qt::NoFocus), layout(0), style(0), window(0), ownerWindow(0),
          qssDirty(true) {}
};

QWidgetPrivate* qtcWidgetPrivate(QWidget* widget);
void qtcRegisterWidget(QWidget* widget);
void qtcUnregisterWidget(QWidget* widget);

// ---------------------------------------------------------------------------
// QWidgetWindow : one native HWND per top-level widget
// ---------------------------------------------------------------------------
class QWidgetWindow {
public:
    static QWidgetWindow* fromHwnd(HWND hwnd);
    static int windowCount();

    QWidgetWindow(QWidget* widget);
    ~QWidgetWindow();

    bool create();
    void destroy();
    void show();
    void hide();
    void setTitle(const QString& title);
    void setWindowIcon(const QIcon& icon);
    void updateWindowFlags();

    void updateArea(const QRect& clientRect);
    void updateAll();
    void paint(HDC targetDc = 0);
    void flush(const QRect& clientRect);

    void resizeBackingStore(int w, int h);
    QPoint mapToGlobal(const QPoint& client) const;
    QPoint mapFromGlobal(const QPoint& global) const;

    QWidget* widgetAt(const QPoint& clientPos, bool includeDisabled = false);
    void setFocusWidget(QWidget* widget, Qt::FocusReason reason = Qt::OtherFocusReason);
    QWidget* focusWidget() const { return focus; }

    void handleMouse(QEvent::Type type, const QPoint& clientPos, Qt::MouseButton button,
                     Qt::MouseButtons buttons, Qt::KeyboardModifiers mods);
    void handleWheel(const QPoint& clientPos, int delta, Qt::MouseButtons buttons,
                     Qt::KeyboardModifiers mods);
    void handleKey(QEvent::Type type, int key, Qt::KeyboardModifiers mods, const QString& text);
    void handleIme(const QString& commit, const QString& preedit);
    void setHoverWidget(QWidget* widget);

    void installMessageHooks();

    HWND hwnd;
    QWidget* widget;
    QPixmap back;
    QRegion dirty;
    QWidget* hover;
    QWidget* grabber;
    QWidget* focus;
    QWidget* defaultWidget;
    bool visible;
    bool inPaint;
    bool popup;
    bool trackingLeave;
    QPoint lastMousePos;
    Qt::MouseButtons lastButtons;
    bool clickDebug;
    int debugClicks;
    QPoint debugPos;
    QString debugTarget;
    QString windowClass;
    ulong style;
    ulong exStyle;

private:
    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT handleMessage(HWND, UINT, WPARAM, LPARAM);
    void applyCursor();
    void computeDirtyForPaint();
};

// helpers shared by gui sources
Qt::KeyboardModifiers qtcQueryModifiers();
Qt::MouseButtons qtcQueryMouseButtons();
int qtcKeyFromVirtualKey(WPARAM vk, LPARAM lp, bool* isPrintable);
bool qtcIsSystemDark();
void qtcWatchSystemTheme(HWND hwnd);
void qtcUnwatchSystemTheme(HWND hwnd);
QString qtcMakeBackendWindowClass();

// font / metric helpers
int qtcTextHeight(const QFont& font);
int qtcFontLineSpacing(const QFont& font);

#endif // QTC_QWIDGET_P_H
