// QtClone - QWidget.h : window system, widget base class, application object.
#ifndef QTC_QWIDGET_H
#define QTC_QWIDGET_H

#include <qtc/QtGlobal.h>
#include <qtc/QRect.h>
#include <qtc/QString.h>
#include <qtc/QList.h>
#include <qtc/QObject.h>
#include <qtc/QEvent.h>
#include <qtc/QtGui.h>

class QLayout;
class QStyle;
class QWidget;
class QWidgetWindow;
struct QWidgetPrivate;
QWidgetPrivate* qtcWidgetPrivate(QWidget* widget);
class QStyleSheet;

typedef QList<QWidget*> QWidgetList;

// ---------------------------------------------------------------------------
// QSizePolicy
// ---------------------------------------------------------------------------
class QSizePolicy {
public:
    enum Policy {
        Fixed = 0,
        Minimum = 1,
        Maximum = 4,
        Preferred = 5,
        MinimumExpanding = 3,
        Expanding = 7,
        Ignored = 13
    };

    QSizePolicy() : h_(Preferred), v_(Preferred), hStretch_(0), vStretch_(0) {}
    QSizePolicy(Policy h, Policy v, int hStretch = 0, int vStretch = 0)
        : h_(h), v_(v), hStretch_(hStretch), vStretch_(vStretch) {}

    Policy horizontalPolicy() const { return h_; }
    Policy verticalPolicy() const { return v_; }
    void setHorizontalPolicy(Policy p) { h_ = p; }
    void setVerticalPolicy(Policy p) { v_ = p; }
    int horizontalStretch() const { return hStretch_; }
    int verticalStretch() const { return vStretch_; }
    void setHorizontalStretch(int s) { hStretch_ = s; }
    void setVerticalStretch(int s) { vStretch_ = s; }
    Qt::Orientations expandingDirections() const;

    bool operator==(const QSizePolicy& other) const {
        return h_ == other.h_ && v_ == other.v_ &&
               hStretch_ == other.hStretch_ && vStretch_ == other.vStretch_;
    }

private:
    Policy h_;
    Policy v_;
    int hStretch_;
    int vStretch_;
};

// ---------------------------------------------------------------------------
// QClipboard
// ---------------------------------------------------------------------------
class QClipboard {
public:
    QString text() const;
    void setText(const QString& text);
    bool hasText() const { return !text().isEmpty(); }
};

// ---------------------------------------------------------------------------
// QWidget
// ---------------------------------------------------------------------------
class QWidget : public QObject {
    QTC_OBJECT(QWidget, QObject)
public:
    explicit QWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~QWidget();

    // ---- geometry ----
    int x() const;
    int y() const;
    int width() const;
    int height() const;
    QPoint pos() const;
    QSize size() const;
    QRect rect() const { return QRect(0, 0, width(), height()); }
    QRect geometry() const;
    QRect frameGeometry() const;
    void move(int x, int y);
    void move(const QPoint& p) { move(p.x(), p.y()); }
    void resize(int w, int h);
    void resize(const QSize& s) { resize(s.width(), s.height()); }
    void setGeometry(const QRect& r) { setGeometry(r.x(), r.y(), r.width(), r.height()); }
    void setGeometry(int x, int y, int w, int h);
    QRect contentsRect() const;
    QMargins contentsMargins() const;
    void setContentsMargins(int left, int top, int right, int bottom);
    void setContentsMargins(const QMargins& m) { setContentsMargins(m.left(), m.top(), m.right(), m.bottom()); }
    void getContentsMargins(int* left, int* top, int* right, int* bottom) const;

    QWidget* parentWidget() const;
    void setParentWidget(QWidget* parent);
    QWidget* window() const;
    bool isWindow() const;
    WId winId() const;
    QWidgetList topLevelWidgets() const;

    QPoint mapToGlobal(const QPoint& p) const;
    QPoint mapFromGlobal(const QPoint& p) const;
    QPoint mapTo(const QWidget* parent, const QPoint& p) const;
    QPoint mapFrom(const QWidget* parent, const QPoint& p) const;
    QWidget* childAt(int x, int y) const { return childAt(QPoint(x, y)); }
    QWidget* childAt(const QPoint& p) const;
    QList<QWidget*> childrenWidgets() const;

    // ---- visibility / window state ----
    bool isVisible() const;
    bool isHidden() const;
    bool isEnabled() const;
    bool isEnabledTo(const QWidget* ancestor) const;
    virtual void show();
    virtual void hide();
    void setVisible(bool visible);
    void setEnabled(bool enabled);
    void close();
    bool isActiveWindow() const;
    void activateWindow();
    void raise();
    void lower();
    void setWindowTitle(const QString& title);
    QString windowTitle() const;
    void setWindowIcon(const QIcon& icon);
    QIcon windowIcon() const;
    void setWindowFlags(Qt::WindowFlags flags);
    Qt::WindowFlags windowFlags() const;
    void setWindowModality(bool modal);

    // ---- appearance ----
    void setFont(const QFont& font);
    QFont font() const;
    void setPalette(const QPalette& palette);
    QPalette palette() const;
    void setStyleSheet(const QString& sheet);
    QString styleSheet() const;
    QStyle* style() const;
    void setStyle(QStyle* style);
    void setToolTip(const QString& tip);
    QString toolTip() const;
    void setStatusTip(const QString& tip);
    QString statusTip() const;
    void setCursor(const QCursor& cursor);
    QCursor cursor() const;
    void unsetCursor();
    void setMouseTracking(bool enable);
    bool hasMouseTracking() const;
    bool underMouse() const;

    // ---- focus / keyboard ----
    void setFocusPolicy(Qt::FocusPolicy policy);
    Qt::FocusPolicy focusPolicy() const;
    void setFocus(Qt::FocusReason reason = Qt::OtherFocusReason);
    void clearFocus();
    bool hasFocus() const;
    QWidget* focusWidget() const;
    bool focusNextPrevChild(bool next);

    // ---- sizes / layout ----
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    void setMinimumSize(const QSize& s);
    void setMinimumSize(int w, int h) { setMinimumSize(QSize(w, h)); }
    void setMaximumSize(const QSize& s);
    void setMaximumSize(int w, int h) { setMaximumSize(QSize(w, h)); }
    void setFixedSize(const QSize& s) { setMinimumSize(s); setMaximumSize(s); }
    void setFixedSize(int w, int h) { setFixedSize(QSize(w, h)); }
    void setFixedWidth(int w) { setMinimumSize(w, minimumSize().height()); setMaximumSize(w, maximumSize().height()); }
    void setFixedHeight(int h) { setMinimumSize(minimumSize().width(), h); setMaximumSize(maximumSize().width(), h); }
    QSizePolicy sizePolicy() const;
    void setSizePolicy(const QSizePolicy& policy);
    void setSizePolicy(QSizePolicy::Policy h, QSizePolicy::Policy v) { setSizePolicy(QSizePolicy(h, v)); }
    void adjustSize();
    void setLayout(QLayout* layout);
    QLayout* layout() const;

    // ---- attributes ----
    void setAttribute(Qt::WidgetAttribute attribute, bool on = true);
    bool testAttribute(Qt::WidgetAttribute attribute) const;

    // ---- painting ----
    void update();
    void update(const QRect& r);
    void update(const QRegion& region);
    void repaint();
    virtual void render(QPainter* painter, const QPoint& targetOffset = QPoint());
    QPixmap grab();


signals:
    Signal<void(const QString&)> windowTitleChanged;
    Signal<void()> themeChanged;

protected:
    // event handlers (Qt naming)
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void moveEvent(QMoveEvent* event);
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);
    virtual void closeEvent(QCloseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);
    virtual void enterEvent(QEvent* event);
    virtual void leaveEvent(QEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void changeEvent(QEvent* event);
    bool event(QEvent* event);

    // helpers for subclasses
    void paintBackground(QPainter* painter);
    void paintFrame(QPainter* painter);
    QRect styleRect() const;
    int styleState() const;
    bool isDown() const;
    bool isHovered() const;
    void setDown(bool down);
    void setFocusProxy(QWidget* w);
    QWidget* focusProxy() const;
    void updateGeometry();
    void ensurePolished();

    QWidgetPrivate* d_func() const { return d_; }

private:
    friend class QApplication;
    friend class QWidgetWindow;
    friend class QLayout;
    friend class QStyle;
    friend class QStyleSheet;
    friend class QWidgetItem;
    friend class QPainter;
    friend struct QWidgetPrivate;
    friend QWidgetPrivate* qtcWidgetPrivate(QWidget* widget);

    void createWindow();
    void destroyWindow();
    void propagateOwnerWindow(QWidgetWindow* window);
    void clearOwnerWindow();
    void renderInternal(QPainter* painter, const QPoint& origin, const QRect& clip);
    void paintSelf(QPainter* painter);
    void layoutChildren();
    void notifyThemeChanged();
    QWidgetWindow* windowHandle() const;
    QRect localDirtyRect() const;

    QWidgetPrivate* d_;
    QWidget(const QWidget&);
    QWidget& operator=(const QWidget&);
};

// ---------------------------------------------------------------------------
// QTheme : light / dark / follow system
// ---------------------------------------------------------------------------
class QTheme {
public:
    enum Mode { Light, Dark, System };

    static void setMode(Mode mode);
    static Mode mode();
    static bool isDark();

    // semantic colours used by the built-in style and by widgets
    struct Colors {
        QColor window;        // window background
        QColor windowText;
        QColor base;          // input/list background
        QColor alternateBase;
        QColor surface;       // panels, toolbars, group boxes
        QColor surfaceAlt;    // hovered rows, striped areas
        QColor border;
        QColor borderStrong;
        QColor button;
        QColor buttonHover;
        QColor buttonPressed;
        QColor buttonText;
        QColor accent;
        QColor accentText;
        QColor accentHover;
        QColor disabledText;
        QColor gridLine;
        QColor selection;
        QColor selectionText;
        QColor toolTipBase;
        QColor toolTipText;
        QColor header;
        QColor headerText;
        QColor menuBar;
        QColor menuPopup;
        QColor scrollbar;
        QColor scrollbarHandle;
        QColor canvas;        // designer canvas
        QColor shadow;
    };
    static const Colors& colors();
    static void setSystemDark(bool dark);

signals:
    void changed();
};

// ---------------------------------------------------------------------------
// QApplication
// ---------------------------------------------------------------------------
class QApplication {
public:
    QApplication(int& argc, char** argv);
    ~QApplication();

    static QApplication* instance();

    static int exec();
    static void quit();
    static void exit(int code);
    static void processEvents();
    static void sendPostedEvents();
    static bool closingDown();

    static QWidget* activeWindow();
    static void setActiveWindow(QWidget* widget);
    static QWidget* focusWidget();
    static QWidget* widgetAt(const QPoint& globalPos);
    static QWidgetList topLevelWidgets();
    static QWidgetList allWidgets();

    static QFont font();
    static void setFont(const QFont& font);
    static QPalette palette();
    static void setPalette(const QPalette& palette);
    static QStyle* style();
    static void setStyle(QStyle* style);
    static QString styleSheet();
    static void setStyleSheet(const QString& sheet);

    static QTheme::Mode themeMode();
    static void setThemeMode(QTheme::Mode mode);
    static bool isDarkTheme();

    static QString buildInfo();
    static QClipboard* clipboard();
    static int doubleClickInterval();
    static int keyboardRepeatRate();
    static Qt::KeyboardModifiers keyboardModifiers();
    static Qt::MouseButtons mouseButtons();
    static QPoint cursorPos();
    static void setOverrideCursor(const QCursor& cursor);
    static void restoreOverrideCursor();
    static void beep();

    // internal
    static void qtcRegisterWindow(QWidgetWindow* window);
    static void qtcUnregisterWindow(QWidgetWindow* window);
    static void qtcRepaintAll();
    static void qtcNotifyThemeChange();
    static void qtcRebuildStyleSheet();
    static void qtcPolishedChanged();
    static void qtcSetMouseGrabber(QWidget* widget);
    static QWidget* qtcMouseGrabber();
    static void qtcPostQuit();
    static bool qtcInEventLoop();
    static void qtcAddWidgetToPolish(QWidget* widget);

private:
    static QApplication* self_;
};

// ---------------------------------------------------------------------------
// widget class registry (used by the designer and by .ui loading)
// ---------------------------------------------------------------------------
struct QWidgetClassInfo {
    const char* className;
    const char* group;
    const char* label;
    QWidget* (*create)();
    const char* header;      // header the class lives in
};

class QWidgetFactory {
public:
    static void registerClass(const QWidgetClassInfo& info);
    static QList<QWidgetClassInfo> classes();
    static QWidget* create(const char* className);
    static const QWidgetClassInfo* classInfo(const char* className);
    static bool isRegistered(const char* className);
};

template <class T>
struct QtcWidgetCreator {
    static QWidget* create() { return new T(0); }
};

#define QTC_REGISTER_WIDGET(CLASS, GROUP, LABEL, HEADER) \
    static struct QtCReg_##CLASS { \
        QtCReg_##CLASS() { \
            QWidgetClassInfo info; \
            info.className = #CLASS; \
            info.group = GROUP; \
            info.label = LABEL; \
            info.create = &QtcWidgetCreator<CLASS>::create; \
            info.header = HEADER; \
            QWidgetFactory::registerClass(info); \
        } \
    } qtc_reg_instance_##CLASS;

#endif // QTC_QWIDGET_H
