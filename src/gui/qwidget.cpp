// QtClone - qwidget.cpp : QWidget, theme, clipboard, widget factory.
#include "qwidget_p.h"
#include <qtc/QLayout.h>
#include <qtc/QStyle.h>
#include <stdio.h>

// ---------------------------------------------------------------------------
// small helpers
// ---------------------------------------------------------------------------
QWidgetPrivate* qtcWidgetPrivate(QWidget* widget) {
    return widget ? widget->d_func() : 0;
}

static QWidgetWindow* qtcUpdateWindow(QWidget* widget);

static bool qtcEffectivelyVisible(const QWidget* widget) {
    for (const QWidget* w = widget; w; w = w->parentWidget()) {
        if (!w->isVisible()) return false;
    }
    return true;
}

static QPoint qtcMapToWindowPos(QWidget* widget, const QPoint& p) {
    QPoint result = p;
    for (QWidget* w = widget; w; w = w->parentWidget()) {
        result += w->pos();
        if (w->isWindow()) break;
    }
    return result;
}

static QRect qtcMapToWindowRect(QWidget* widget, const QRect& r) {
    return QRect(qtcMapToWindowPos(widget, r.topLeft()), r.size());
}

// ---------------------------------------------------------------------------
// QTheme
// ---------------------------------------------------------------------------
namespace {

QTheme::Mode g_themeMode = QTheme::System;
bool g_systemDark = false;
bool g_systemDarkProbed = false;

QTheme::Colors makeLightColors() {
    QTheme::Colors c;
    c.window = QColor(0xF5, 0xF6, 0xFA);
    c.windowText = QColor(0x1F, 0x29, 0x37);
    c.base = QColor(0xFF, 0xFF, 0xFF);
    c.alternateBase = QColor(0xF9, 0xFA, 0xFB);
    c.surface = QColor(0xFF, 0xFF, 0xFF);
    c.surfaceAlt = QColor(0xEE, 0xF2, 0xF7);
    c.border = QColor(0xD8, 0xDE, 0xE8);
    c.borderStrong = QColor(0xB9, 0xC2, 0xCF);
    c.button = QColor(0xFF, 0xFF, 0xFF);
    c.buttonHover = QColor(0xF0, 0xF4, 0xFA);
    c.buttonPressed = QColor(0xE2, 0xE8, 0xF0);
    c.buttonText = QColor(0x1F, 0x29, 0x37);
    c.accent = QColor(0x00, 0x78, 0xD4);
    c.accentText = QColor(0xFF, 0xFF, 0xFF);
    c.accentHover = QColor(0x10, 0x6E, 0xBE);
    c.disabledText = QColor(0x9A, 0xA3, 0xAF);
    c.gridLine = QColor(0xE5, 0xE9, 0xF0);
    c.selection = QColor(0xCC, 0xE8, 0xFF);
    c.selectionText = QColor(0x1F, 0x29, 0x37);
    c.toolTipBase = QColor(0xFF, 0xFF, 0xE1);
    c.toolTipText = QColor(0x1F, 0x29, 0x37);
    c.header = QColor(0xF0, 0xF2, 0xF6);
    c.headerText = QColor(0x1F, 0x29, 0x37);
    c.menuBar = QColor(0xF7, 0xF8, 0xFA);
    c.menuPopup = QColor(0xFF, 0xFF, 0xFF);
    c.scrollbar = QColor(0xEF, 0xF1, 0xF5);
    c.scrollbarHandle = QColor(0xC4, 0xCB, 0xD6);
    c.canvas = QColor(0xE9, 0xED, 0xF3);
    c.shadow = QColor(0, 0, 0, 40);
    return c;
}

QTheme::Colors makeDarkColors() {
    QTheme::Colors c;
    c.window = QColor(0x12, 0x14, 0x17);
    c.windowText = QColor(0xE6, 0xE8, 0xEB);
    c.base = QColor(0x17, 0x19, 0x1D);
    c.alternateBase = QColor(0x1B, 0x1E, 0x23);
    c.surface = QColor(0x1A, 0x1D, 0x22);
    c.surfaceAlt = QColor(0x23, 0x27, 0x2E);
    c.border = QColor(0x2E, 0x33, 0x3B);
    c.borderStrong = QColor(0x3D, 0x44, 0x4E);
    c.button = QColor(0x22, 0x26, 0x2D);
    c.buttonHover = QColor(0x2A, 0x2F, 0x37);
    c.buttonPressed = QColor(0x1C, 0x20, 0x26);
    c.buttonText = QColor(0xE6, 0xE8, 0xEB);
    c.accent = QColor(0x4C, 0x8D, 0xFF);
    c.accentText = QColor(0x0B, 0x0D, 0x10);
    c.accentHover = QColor(0x6B, 0xA1, 0xFF);
    c.disabledText = QColor(0x6B, 0x72, 0x80);
    c.gridLine = QColor(0x27, 0x2B, 0x32);
    c.selection = QColor(0x23, 0x4E, 0x82);
    c.selectionText = QColor(0xEA, 0xF1, 0xFF);
    c.toolTipBase = QColor(0x22, 0x26, 0x2D);
    c.toolTipText = QColor(0xE6, 0xE8, 0xEB);
    c.header = QColor(0x1E, 0x21, 0x26);
    c.headerText = QColor(0xE6, 0xE8, 0xEB);
    c.menuBar = QColor(0x16, 0x18, 0x1C);
    c.menuPopup = QColor(0x1A, 0x1D, 0x22);
    c.scrollbar = QColor(0x17, 0x19, 0x1D);
    c.scrollbarHandle = QColor(0x3A, 0x40, 0x48);
    c.canvas = QColor(0x0E, 0x10, 0x13);
    c.shadow = QColor(0, 0, 0, 120);
    return c;
}

QTheme::Colors g_light = makeLightColors();
QTheme::Colors g_dark = makeDarkColors();

} // namespace

const QTheme::Colors& QTheme::colors() {
    return isDark() ? g_dark : g_light;
}

void QTheme::setSystemDark(bool dark) {
    if (g_systemDark == dark) return;
    g_systemDark = dark;
    g_systemDarkProbed = true;
    if (g_themeMode == System) QApplication::qtcNotifyThemeChange();
}

bool QTheme::isDark() {
    if (g_themeMode == Dark) return true;
    if (g_themeMode == Light) return false;
    if (!g_systemDarkProbed) {
        g_systemDark = qtcIsSystemDark();
        g_systemDarkProbed = true;
    }
    return g_systemDark;
}

QTheme::Mode QTheme::mode() { return g_themeMode; }

void QTheme::setMode(Mode mode) {
    if (g_themeMode == mode) return;
    g_themeMode = mode;
    if (mode == System) {
        g_systemDark = qtcIsSystemDark();
    }
    QApplication::qtcNotifyThemeChange();
}

// ---------------------------------------------------------------------------
// QSizePolicy
// ---------------------------------------------------------------------------
Qt::Orientations QSizePolicy::expandingDirections() const {
    Qt::Orientations result = 0;
    // Preferred carries the grow flag (5 & 7 != 0) but must not be treated as
    // expanding; only Expanding/MinimumExpanding may claim the surplus first.
    if (h_ == Expanding || h_ == MinimumExpanding) result = result | Qt::Horizontal;
    if (v_ == Expanding || v_ == MinimumExpanding) result = result | Qt::Vertical;
    return result;
}

// ---------------------------------------------------------------------------
// QClipboard
// ---------------------------------------------------------------------------
QString QClipboard::text() const {
    QString result;
    if (!OpenClipboard(0)) return result;
    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (data) {
        const wchar_t* text = (const wchar_t*)GlobalLock(data);
        if (text) {
            result = text;
            GlobalUnlock(data);
        }
    }
    CloseClipboard();
    return result;
}

void QClipboard::setText(const QString& text) {
    if (!OpenClipboard(0)) return;
    EmptyClipboard();
    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (mem) {
        void* dst = GlobalLock(mem);
        memcpy(dst, text.c_str(), bytes);
        GlobalUnlock(mem);
        SetClipboardData(CF_UNICODETEXT, mem);
    }
    CloseClipboard();
}

// ---------------------------------------------------------------------------
// QWidget construction / destruction
// ---------------------------------------------------------------------------
QWidget::QWidget(QWidget* parent, Qt::WindowFlags f)
    : QObject(0), d_(new QWidgetPrivate()) {
    d_->q = this;
    d_->flags = f;
    d_->geometry = QRect(0, 0, (f & Qt::WindowType_Mask) ? 800 : 120, (f & Qt::WindowType_Mask) ? 600 : 30);
    d_->focusPolicy = Qt::NoFocus;
    setWidgetType(true);
    if (f & Qt::WindowType_Mask) {
        d_->flags = f;
    }
    if (parent) setParentWidget(parent);
    if (f & Qt::WindowType_Mask) {
        d_->flags = f;
    }
    qtcRegisterWidget(this);
}

QWidget::~QWidget() {
    d_->visible = false;
    if (d_->window) {
        d_->window->widget = 0;
        destroyWindow();
    }
    if (d_->ownerWindow) {
        if (d_->ownerWindow->hover == this) d_->ownerWindow->hover = 0;
    }
    // Widget children live in QWidgetPrivate::children, not in QObject's
    // child list, so they have to be deleted explicitly.
    while (!d_->children.isEmpty()) {
        QWidget* child = d_->children.takeLast();
        child->d_func()->parent = 0;
        delete child;
    }
    // Null the pointer before deleting: QLayout's destructor calls
    // setLayout(0) on its parent, which would otherwise delete it again.
    QLayout* layout = d_->layout;
    d_->layout = 0;
    delete layout;
    // children are deleted by QObject's destructor; detach them from the window first
    for (int i = 0; i < d_->children.count(); ++i) {
        QWidget* child = d_->children[i];
        if (child->d_func() && child->d_func()->ownerWindow == d_->ownerWindow)
            child->d_func()->ownerWindow = 0;
    }
    if (parentWidget() && parentWidget()->d_func()) {
        QWidgetPrivate* pd = parentWidget()->d_func();
        pd->children.removeAll(this);
    }
    d_->parent = 0;
    qtcUnregisterWidget(this);
    delete d_;
    d_ = 0;
}

QWidget* QWidget::parentWidget() const { return d_->parent; }
bool QWidget::isWindow() const { return (d_->flags & Qt::WindowType_Mask) != 0; }

void QWidget::setParentWidget(QWidget* parent) {
    if (d_->parent == parent) return;
    if (d_->parent && d_->parent->d_func()) d_->parent->d_func()->children.removeAll(this);
    d_->parent = parent;
    if (parent) {
        parent->d_func()->children.append(this);
        d_->ownerWindow = isWindow() ? 0 : parent->windowHandle();
        if (!(d_->flags & Qt::WindowType_Mask)) d_->flags = Qt::Widget;
        if (!isWindow() && parent->d_func()->visible && !d_->explicitHide && !d_->visible) show();
    } else {
        d_->ownerWindow = 0;
        if (!(d_->flags & Qt::WindowType_Mask)) d_->flags = Qt::Window;
    }
}

WId QWidget::winId() const {
    QWidget* self = const_cast<QWidget*>(this);
    QWidget* top = self->isWindow() ? self : self->window();
    if (!top->d_->window) top->createWindow();
    return top->d_->window ? (WId)top->d_->window->hwnd : 0;
}

QWidgetWindow* QWidget::windowHandle() const {
    if (d_->window) return d_->window;
    return d_->ownerWindow;
}

QWidget* QWidget::window() const {
    QWidget* w = const_cast<QWidget*>(this);
    while (w->parentWidget()) w = w->parentWidget();
    return w;
}

QWidgetList QWidget::topLevelWidgets() const {
    QWidgetList out;
    QWidgetList all = QApplication::allWidgets();
    for (int i = 0; i < all.count(); ++i) if (all[i]->isWindow()) out.append(all[i]);
    return out;
}

QWidgetList QWidget::childrenWidgets() const { return d_->children; }

// ---------------------------------------------------------------------------
// window creation
// ---------------------------------------------------------------------------
void QWidget::propagateOwnerWindow(QWidgetWindow* window) {
    if (isWindow()) return;  // popups/dialogs own their window, never borrow one
    d_->ownerWindow = window;
    for (int i = 0; i < d_->children.count(); ++i)
        d_->children[i]->propagateOwnerWindow(window);
}

void QWidget::clearOwnerWindow() {
    if (isWindow()) {
        d_->ownerWindow = 0;
        return;
    }
    d_->ownerWindow = 0;
    for (int i = 0; i < d_->children.count(); ++i)
        d_->children[i]->clearOwnerWindow();
}

void QWidget::createWindow() {
    if (d_->window) return;
    if (!isWindow()) return;
    d_->window = new QWidgetWindow(this);
    if (!d_->window->create()) {
        delete d_->window;
        d_->window = 0;
        return;
    }
    d_->created = true;
    // Widgets parented before the window existed have no ownerWindow yet;
    // giving the whole subtree the window makes update()/show()/focus work.
    propagateOwnerWindow(d_->window);
}

void QWidget::destroyWindow() {
    if (!d_->window) return;
    clearOwnerWindow();
    d_->window->destroy();
    delete d_->window;
    d_->window = 0;
    d_->created = false;
}

// ---------------------------------------------------------------------------
// geometry
// ---------------------------------------------------------------------------
int QWidget::x() const { return d_->geometry.x(); }
int QWidget::y() const { return d_->geometry.y(); }
int QWidget::width() const { return d_->geometry.width(); }
int QWidget::height() const { return d_->geometry.height(); }
QPoint QWidget::pos() const { return d_->geometry.topLeft(); }
QSize QWidget::size() const { return d_->geometry.size(); }
QRect QWidget::geometry() const { return d_->geometry; }
QRect QWidget::frameGeometry() const { return d_->geometry; }

QRect QWidget::contentsRect() const {
    return rect().adjusted(d_->margins.left(), d_->margins.top(), -d_->margins.right(), -d_->margins.bottom());
}

QMargins QWidget::contentsMargins() const { return d_->margins; }

void QWidget::setContentsMargins(int left, int top, int right, int bottom) {
    d_->margins = QMargins(left, top, right, bottom);
    if (d_->layout) d_->layout->setContentsMargins(left, top, right, bottom);
    updateGeometry();
}

void QWidget::getContentsMargins(int* left, int* top, int* right, int* bottom) const {
    if (left) *left = d_->margins.left();
    if (top) *top = d_->margins.top();
    if (right) *right = d_->margins.right();
    if (bottom) *bottom = d_->margins.bottom();
}

void QWidget::move(int nx, int ny) {
    setGeometry(QRect(nx, ny, width(), height()));
}

void QWidget::resize(int w, int h) {
    setGeometry(QRect(x(), y(), w, h));
}

void QWidget::setGeometry(int nx, int ny, int nw, int nh) {
    if (nw < 0) nw = 0;
    if (nh < 0) nh = 0;
    if (d_->minSize.width() > 0 && nw < d_->minSize.width()) nw = d_->minSize.width();
    if (d_->minSize.height() > 0 && nh < d_->minSize.height()) nh = d_->minSize.height();
    if (d_->maxSize.width() < 0x7fffffff && nw > d_->maxSize.width()) nw = d_->maxSize.width();
    if (d_->maxSize.height() < 0x7fffffff && nh > d_->maxSize.height()) nh = d_->maxSize.height();

    QRect old = d_->geometry;
    QRect now(nx, ny, nw, nh);
    if (old == now) return;

    QWidgetWindow* win = windowHandle();
    if (isWindow()) {
        d_->geometry = now;
        if (d_->window && d_->window->hwnd) {
            SetWindowPos(d_->window->hwnd, 0, nx, ny, nw, nh,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (old.size() != now.size()) {
            QResizeEvent ev(now.size(), old.size());
            event(&ev);
        }
        if (old.topLeft() != now.topLeft()) {
            QMoveEvent ev(now.topLeft(), old.topLeft());
            event(&ev);
        }
        if (d_->layout) d_->layout->setGeometry(contentsRect());
        return;
    }

    QWidget* par = d_->parent;
    QRect oldR;
    if (win && par) oldR = QRect(qtcMapToWindowPos(par, old.topLeft()), old.size());
    d_->geometry = now;
    if (win) {
        if (par && !oldR.isEmpty()) win->updateArea(oldR);
        win->updateArea(qtcMapToWindowRect(this, rect()));
    }    if (old.size() != now.size()) {
        QResizeEvent ev(now.size(), old.size());
        event(&ev);
    }
    if (old.topLeft() != now.topLeft()) {
        QMoveEvent ev(now.topLeft(), old.topLeft());
        event(&ev);
    }
    if (d_->layout) d_->layout->setGeometry(contentsRect());
}

// ---------------------------------------------------------------------------
// visibility
// ---------------------------------------------------------------------------
bool QWidget::isVisible() const { return d_->visible; }
bool QWidget::isHidden() const { return !d_->visible; }
bool QWidget::isEnabled() const { return d_->enabled; }
bool QWidget::isEnabledTo(const QWidget* ancestor) const {
    for (const QWidget* w = this; w; w = w->parentWidget()) {
        if (!w->isEnabled()) return false;
        if (w == ancestor) break;
    }
    return true;
}

void QWidget::setEnabled(bool enabled) {
    if (d_->enabled == enabled) return;
    d_->enabled = enabled;
    update();
    for (int i = 0; i < d_->children.count(); ++i) d_->children[i]->update();
    if (d_->window) d_->window->updateAll();
}

void QWidget::show() {
    d_->explicitHide = false;
    if (d_->visible) {
        if (isWindow() && d_->window) {
            ShowWindow(d_->window->hwnd, SW_SHOW);
            SetForegroundWindow(d_->window->hwnd);
        }
        return;
    }
    d_->visible = true;
    if (isWindow()) {
        if (!d_->window) createWindow();
        if (d_->window) {
            d_->window->setTitle(d_->windowTitle);
            d_->window->setWindowIcon(d_->windowIcon);
            d_->window->show();
            d_->window->updateAll();
        }
    } else {
        QWidgetWindow* win = windowHandle();
        if (win && qtcEffectivelyVisible(this)) win->updateArea(qtcMapToWindowRect(this, rect()));
    }
    QShowEvent ev;
    event(&ev);
    for (int i = 0; i < d_->children.count(); ++i) {
        QWidget* child = d_->children[i];
        if (child->d_func()->explicitHide) continue;
        if (!child->d_func()->visible) child->show();
    }
    // Children are visible now, so the layout no longer skips them.
    if (d_->layout) {
        d_->layout->invalidate();
        d_->layout->activate();
    }
}

void QWidget::hide() {
    d_->explicitHide = true;
    if (!d_->visible) return;
    QWidgetWindow* win = windowHandle();
    QRect r = qtcMapToWindowRect(this, rect());
    d_->visible = false;
    QHideEvent ev;
    event(&ev);
    if (isWindow()) {
        if (d_->window) d_->window->hide();
    } else if (win) {
        win->updateArea(r);
    }
    for (int i = 0; i < d_->children.count(); ++i) {
        QWidget* child = d_->children[i];
        if (child->d_func()->visible) child->hide();
    }
}

void QWidget::setVisible(bool visible) {
    if (visible) show();
    else hide();
}

void QWidget::close() {
    if (!isWindow()) {
        hide();
        return;
    }
    QCloseEvent ev;
    event(&ev);
    if (!ev.isAccepted()) return;
    hide();

    // Closing the last visible top level window ends the application,
    // like Qt does by default. Without this the process survives with no
    // window left, which looks like "the app cannot be exited".
    QWidgetList tops = QApplication::topLevelWidgets();
    bool anyVisible = false;
    for (int i = 0; i < tops.count(); ++i) {
        if (tops[i]->isVisible()) { anyVisible = true; break; }
    }
    if (!anyVisible) QApplication::quit();
}

bool QWidget::isActiveWindow() const { return d_->activeWindow; }

void QWidget::activateWindow() {
    if (d_->ownerWindow && d_->ownerWindow->hwnd) {
        SetForegroundWindow(d_->ownerWindow->hwnd);
    }
}

void QWidget::setWindowTitle(const QString& title) {
    if (d_->windowTitle == title) return;
    d_->windowTitle = title;
    if (d_->window) d_->window->setTitle(title);
    windowTitleChanged(title);
}

QString QWidget::windowTitle() const { return d_->windowTitle; }

void QWidget::setWindowIcon(const QIcon& icon) {
    d_->windowIcon = icon;
    if (d_->window) d_->window->setWindowIcon(icon);
}

QIcon QWidget::windowIcon() const { return d_->windowIcon; }

void QWidget::setWindowFlags(Qt::WindowFlags flags) {
    bool wasWindow = isWindow();
    d_->flags = flags;
    if (d_->window) d_->window->updateWindowFlags();
    if (wasWindow != isWindow()) {
        if (d_->window) destroyWindow();
        if (d_->visible) show();
    }
}

Qt::WindowFlags QWidget::windowFlags() const { return d_->flags; }

void QWidget::setWindowModality(bool modal) { d_->modal = modal; }

// ---------------------------------------------------------------------------
// appearance
// ---------------------------------------------------------------------------
void QWidget::setFont(const QFont& font) {
    d_->font = font;
    d_->hasFont = true;
    updateGeometry();
    update();
    for (int i = 0; i < d_->children.count(); ++i) d_->children[i]->update();
}

QFont QWidget::font() const {
    if (d_->hasFont) return d_->font;
    if (d_->parent) return d_->parent->font();
    return QApplication::font();
}

void QWidget::setPalette(const QPalette& palette) {
    d_->palette = palette;
    d_->hasPalette = true;
    update();
}

QPalette QWidget::palette() const {
    if (d_->hasPalette) return d_->palette;
    if (d_->parent) return d_->parent->palette();
    return QApplication::palette();
}

void QWidget::setStyleSheet(const QString& sheet) {
    d_->styleSheet = sheet;
    d_->hasStyleSheet = true;
    QStyleSheet::invalidate(this);
    QStyleSheet::invalidate();
    ensurePolished();
    updateGeometry();
    for (int i = 0; i < d_->children.count(); ++i) {
        QStyleSheet::invalidate(d_->children[i]);
        d_->children[i]->updateGeometry();
    }
    update();
    if (d_->window) d_->window->updateAll();
}

QString QWidget::styleSheet() const { return d_->styleSheet; }

QStyle* QWidget::style() const {
    if (d_->style) return d_->style;
    if (d_->parent) return d_->parent->style();
    return QApplication::style();
}

void QWidget::setStyle(QStyle* style) {
    d_->style = style;
    d_->hasStyle = true;
    update();
}

void QWidget::setToolTip(const QString& tip) { d_->toolTip = tip; }
QString QWidget::toolTip() const {
    if (!d_->toolTip.isEmpty()) return d_->toolTip;
    return QString();
}

void QWidget::setStatusTip(const QString& tip) { d_->statusTip = tip; }
QString QWidget::statusTip() const { return d_->statusTip; }

void QWidget::setCursor(const QCursor& cursor) { d_->cursor = cursor; d_->hasCursor = true; }
QCursor QWidget::cursor() const {
    if (d_->hasCursor) return d_->cursor;
    if (d_->parent) return d_->parent->cursor();
    return QCursor();
}
void QWidget::unsetCursor() { d_->hasCursor = false; }

void QWidget::setMouseTracking(bool enable) { d_->mouseTracking = enable; }
bool QWidget::hasMouseTracking() const { return d_->mouseTracking; }
bool QWidget::underMouse() const { return d_->underMouse; }

void QWidget::setFocusPolicy(Qt::FocusPolicy policy) {
    d_->focusPolicy = policy;
    if (policy == Qt::NoFocus && hasFocus()) clearFocus();
}

Qt::FocusPolicy QWidget::focusPolicy() const {
    if (d_->focusProxy) return d_->focusProxy->focusPolicy();
    return d_->focusPolicy;
}

void QWidget::setFocusProxy(QWidget* w) { d_->focusProxy = w; }
QWidget* QWidget::focusProxy() const { return d_->focusProxy; }

void QWidget::setFocus(Qt::FocusReason reason) {
    if (d_->focusProxy) {
        d_->focusProxy->setFocus(reason);
        return;
    }
    QWidgetWindow* win = qtcUpdateWindow(this);
    if (win) win->setFocusWidget(this, reason);
    else d_->hasFocus = true;
}

void QWidget::clearFocus() {
    QWidgetWindow* win = qtcUpdateWindow(this);
    if (win && win->focusWidget() == this) win->setFocusWidget(0);
    d_->hasFocus = false;
}

bool QWidget::hasFocus() const { return d_->hasFocus; }

QWidget* QWidget::focusWidget() const {
    QWidgetWindow* win = qtcUpdateWindow(const_cast<QWidget*>(this));
    return win ? win->focusWidget() : 0;
}

bool QWidget::focusNextPrevChild(bool next) {
    QWidgetWindow* win = qtcUpdateWindow(this);
    if (!win) return false;
    QWidgetList chain;
    QWidget* root = win->widget;
    if (!root) return false;
    // depth first collection of focusable widgets
    QList<QWidget*> stack;
    stack.append(root);
    while (!stack.isEmpty()) {
        QWidget* w = stack.last();
        stack.removeAt(stack.count() - 1);
        QWidgetPrivate* wd = w->d_func();
        if (w != root && wd->visible && wd->enabled &&
            (wd->focusPolicy & (Qt::TabFocus | Qt::StrongFocus | Qt::ClickFocus)) &&
            (wd->focusPolicy & Qt::TabFocus))
            chain.append(w);
        for (int i = wd->children.count() - 1; i >= 0; --i) stack.append(wd->children[i]);
    }
    if (chain.isEmpty()) return false;
    int idx = chain.indexOf(win->focusWidget());
    int nextIdx;
    if (next) nextIdx = (idx + 1) % chain.count();
    else nextIdx = (idx <= 0 ? chain.count() - 1 : idx - 1);
    chain[nextIdx]->setFocus(next ? Qt::TabFocusReason : Qt::BacktabFocusReason);
    return true;
}

// ---------------------------------------------------------------------------
// sizes
// ---------------------------------------------------------------------------
QSize QWidget::sizeHint() const {
    if (d_->layout) {
        QSize s = d_->layout->sizeHint();
        return s + QSize(d_->margins.left() + d_->margins.right(), d_->margins.top() + d_->margins.bottom());
    }
    if (isWindow()) return QSize(640, 480);
    return QSize(120, 30);
}

QSize QWidget::minimumSizeHint() const {
    if (d_->layout) {
        QSize s = d_->layout->minimumSize();
        return s + QSize(d_->margins.left() + d_->margins.right(), d_->margins.top() + d_->margins.bottom());
    }
    return QSize(0, 0);
}

QSize QWidget::minimumSize() const { return d_->minSize; }
QSize QWidget::maximumSize() const { return d_->maxSize; }

void QWidget::setMinimumSize(const QSize& s) {
    d_->minSize = s;
    if (s.width() > 0 && width() < s.width()) resize(s.width(), height());
    if (s.height() > 0 && height() < s.height()) resize(width(), s.height());
    updateGeometry();
}

void QWidget::setMaximumSize(const QSize& s) {
    d_->maxSize = s;
    updateGeometry();
}

QSizePolicy QWidget::sizePolicy() const { return d_->sizePolicy; }

void QWidget::setSizePolicy(const QSizePolicy& policy) {
    d_->sizePolicy = policy;
    updateGeometry();
}

void QWidget::adjustSize() {
    QSize s = sizeHint();
    if (s.isValid()) resize(s);
}

void QWidget::setLayout(QLayout* layout) {
    if (d_->layout == layout) return;
    // Detach before deleting: ~QLayout calls setLayout(0) on its parent, which
    // would delete the same layout again and recurse until the stack overflows.
    QLayout* old = d_->layout;
    d_->layout = 0;
    delete old;
    d_->layout = layout;
    if (layout) {
        layout->setParentWidget(this);
        layout->setContentsMargins(d_->margins);
        layout->activate();
    }
    updateGeometry();
}

QLayout* QWidget::layout() const { return d_->layout; }

void QWidget::updateGeometry() {
    QWidget* w = d_->parent;
    if (w && w->d_func() && w->d_func()->layout) w->d_func()->layout->invalidate();
    if (d_->layout) d_->layout->invalidate();
}

void QWidget::setAttribute(Qt::WidgetAttribute attribute, bool on) {
    if (on) d_->attrs |= attribute;
    else d_->attrs &= ~attribute;
}

bool QWidget::testAttribute(Qt::WidgetAttribute attribute) const {
    return d_->attrs.testFlag(attribute);
}

// ---------------------------------------------------------------------------
// painting
// ---------------------------------------------------------------------------
// Widgets parented before their top-level window exists do not have an
// ownerWindow yet; resolve the window through the parent chain at update time.
static QWidgetWindow* qtcUpdateWindow(QWidget* widget) {
    QWidget* top = widget;
    while (top->parentWidget()) top = top->parentWidget();
    QWidgetPrivate* td = qtcWidgetPrivate(top);
    return td ? td->window : 0;
}

void QWidget::update() { update(rect()); }

void QWidget::update(const QRect& r) {
    if (r.isEmpty()) return;
    QWidgetWindow* win = qtcUpdateWindow(this);
    if (!win) return;
    if (!qtcEffectivelyVisible(this)) return;
    win->updateArea(qtcMapToWindowRect(this, r));
}

void QWidget::update(const QRegion& region) {
    if (!qtcUpdateWindow(this)) return;
    for (int i = 0; i < region.rectCount(); ++i) update(region.rects()[i]);
}

void QWidget::repaint() {
    QWidgetWindow* win = qtcUpdateWindow(this);
    if (!win) return;
    win->updateArea(qtcMapToWindowRect(this, rect()));
    win->flush(qtcMapToWindowRect(this, rect()));
}

void QWidget::paintBackground(QPainter* painter) {
    QStyleSheet::Box& box = const_cast<QStyleSheet::Box&>(QStyleSheet::boxFor(this));
    if (box.hasBackground) {
        QStyleSheet::paintBackground(painter, this, box);
    } else if (isWindow()) {
        painter->fillRect(rect(), palette().color(QPalette::Window));
    } else if (d_->autoFill) {
        painter->fillRect(rect(), palette().color(QPalette::Window));
    }
}

void QWidget::paintFrame(QPainter* painter) {
    const QStyleSheet::Box& box = QStyleSheet::boxFor(this);
    if (!box.hasBorder) return;
    QPen pen(box.borderColor, box.borderWidth);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    if (box.radius > 0)
        painter->drawRoundedRect(rect().adjusted(0, 0, -1, -1), box.radius, box.radius);
    else
        painter->drawRect(rect().adjusted(0, 0, -1, -1));
}

void QWidget::paintSelf(QPainter* painter) {
    paintBackground(painter);
    QPaintEvent ev(rect());
    paintEvent(&ev);
}

void QWidget::renderInternal(QPainter* painter, const QPoint& origin, const QRect& clip) {
    if (!d_->visible) return;
    QRect globalRect(origin, size());
    if (!globalRect.intersects(clip)) return;

    painter->save();
    painter->translate(origin.x(), origin.y());
    QRect localClip = clip.translated(-origin.x(), -origin.y()).intersected(rect());
    painter->setClipRect(localClip);
    QPoint paintOrigin = painter->paintOrigin();
    qtcPushPaintTarget(painter->graphics(), paintOrigin, localClip);
    {
        static int paintDebug = -1;
        if (paintDebug < 0) {
            wchar_t v[8] = {0};
            paintDebug = GetEnvironmentVariableW(L"QTC_PAINT_DEBUG", v, 8) > 0 ? 1 : 0;
        }
        if (paintDebug) {
            FILE* f = fopen("D:\\AI\\QtClone\\out\\paint.log", "a");
            if (f) {
                fprintf(f, "paint %s '%s' origin=%d,%d rect=%dx%d clip=%d,%d %dx%d\n",
                        className(), objectName().toUtf8().c_str(), origin.x(), origin.y(),
                        width(), height(), clip.x(), clip.y(), clip.width(), clip.height());
                fclose(f);
            }
        }
    }
    paintSelf(painter);
    // Children must never paint outside their parent widget.
    QRect childClip = clip.translated(-origin.x(), -origin.y()).intersected(rect());
    for (int i = 0; i < d_->children.count(); ++i) {
        QWidget* child = d_->children[i];
        child->renderInternal(painter, child->pos(), childClip);
    }
    qtcPopPaintTarget();
    painter->restore();
}

void QWidget::render(QPainter* painter, const QPoint& targetOffset) {
    if (!painter || !painter->isActive()) return;
    renderInternal(painter, targetOffset, QRect(targetOffset, size()));
}

QPixmap QWidget::grab() {
    QPixmap pm(qMax(1, width()), qMax(1, height()));
    pm.fill(QColor(0, 0, 0, 0));
    QPainter p(&pm);
    render(&p, QPoint(0, 0));
    return pm;
}

void QWidget::paintEvent(QPaintEvent*) {}

// ---------------------------------------------------------------------------
// default event implementations
// ---------------------------------------------------------------------------
void QWidget::resizeEvent(QResizeEvent*) {
    if (d_->layout) d_->layout->setGeometry(contentsRect());
}
void QWidget::moveEvent(QMoveEvent*) {}
void QWidget::showEvent(QShowEvent*) {}
void QWidget::hideEvent(QHideEvent*) {}
void QWidget::closeEvent(QCloseEvent* e) { e->accept(); }
void QWidget::mousePressEvent(QMouseEvent* e) { e->ignore(); }
void QWidget::mouseReleaseEvent(QMouseEvent* e) { e->ignore(); }
void QWidget::mouseDoubleClickEvent(QMouseEvent* e) { e->ignore(); }
void QWidget::mouseMoveEvent(QMouseEvent* e) { e->ignore(); }
void QWidget::wheelEvent(QWheelEvent* e) { e->ignore(); }
void QWidget::keyPressEvent(QKeyEvent* e) { e->ignore(); }
void QWidget::keyReleaseEvent(QKeyEvent* e) { e->ignore(); }
void QWidget::focusInEvent(QFocusEvent*) {}
void QWidget::focusOutEvent(QFocusEvent*) {}
void QWidget::enterEvent(QEvent*) {}
void QWidget::leaveEvent(QEvent*) {}
void QWidget::contextMenuEvent(QContextMenuEvent* e) { e->ignore(); }
void QWidget::inputMethodEvent(QInputMethodEvent* e) { e->ignore(); }
void QWidget::changeEvent(QEvent*) {}

bool QWidget::event(QEvent* e) {
    switch (e->type()) {
    case QEvent::Paint:
        paintEvent((QPaintEvent*)e);
        return true;
    case QEvent::Resize:
        resizeEvent((QResizeEvent*)e);
        if (d_->layout) d_->layout->setGeometry(contentsRect());
        return true;
    case QEvent::Move:
        moveEvent((QMoveEvent*)e);
        return true;
    case QEvent::Show:
        showEvent((QShowEvent*)e);
        return true;
    case QEvent::Hide:
        hideEvent((QHideEvent*)e);
        return true;
    case QEvent::Close:
        closeEvent((QCloseEvent*)e);
        return true;
    case QEvent::MouseButtonPress:
        mousePressEvent((QMouseEvent*)e);
        return true;
    case QEvent::MouseButtonRelease:
        mouseReleaseEvent((QMouseEvent*)e);
        return true;
    case QEvent::MouseButtonDblClick:
        mouseDoubleClickEvent((QMouseEvent*)e);
        return true;
    case QEvent::MouseMove:
        mouseMoveEvent((QMouseEvent*)e);
        return true;
    case QEvent::Wheel:
        wheelEvent((QWheelEvent*)e);
        return true;
    case QEvent::KeyPress:
        keyPressEvent((QKeyEvent*)e);
        return true;
    case QEvent::KeyRelease:
        keyReleaseEvent((QKeyEvent*)e);
        return true;
    case QEvent::FocusIn:
        d_->hasFocus = true;
        focusInEvent((QFocusEvent*)e);
        update();
        return true;
    case QEvent::FocusOut:
        d_->hasFocus = false;
        focusOutEvent((QFocusEvent*)e);
        update();
        return true;
    case QEvent::Enter:
        d_->underMouse = true;
        enterEvent(e);
        update();
        return true;
    case QEvent::Leave:
        d_->underMouse = false;
        leaveEvent(e);
        update();
        return true;
    case QEvent::ContextMenu:
        contextMenuEvent((QContextMenuEvent*)e);
        return true;
    case QEvent::InputMethod:
        inputMethodEvent((QInputMethodEvent*)e);
        return true;
    case QEvent::ThemeChange:
        themeChanged();
        QStyleSheet::invalidate(this);
        update();
        changeEvent(e);
        return true;
    case QEvent::StyleChange:
        update();
        changeEvent(e);
        return true;
    case QEvent::LayoutRequest:
        if (d_->layout) d_->layout->activate();
        return true;
    default:
        break;
    }
    return QObject::event(e);
}

void QWidget::ensurePolished() {
    if (!d_->qssDirty) return;
    QStyleSheet::invalidate(this);
    d_->qssDirty = false;
}

QWidget* QWidget::childAt(const QPoint& p) const {
    for (int i = d_->children.count() - 1; i >= 0; --i) {
        QWidget* child = d_->children[i];
        if (!child->isVisible()) continue;
        QRect g = child->geometry();
        if (g.contains(p)) {
            QWidget* deeper = child->childAt(p - g.topLeft());
            return deeper ? deeper : child;
        }
    }
    return 0;
}

QPoint QWidget::mapToGlobal(const QPoint& p) const {
    QPoint result = p;
    const QWidget* w = this;
    while (w->parentWidget()) {
        result += w->pos();
        w = w->parentWidget();
    }
    QWidgetWindow* win = w->windowHandle();
    if (win && win->hwnd) {
        POINT pt;
        pt.x = result.x();
        pt.y = result.y();
        ClientToScreen(win->hwnd, &pt);
        return QPoint(pt.x, pt.y);
    }
    return result + w->pos();
}

QPoint QWidget::mapFromGlobal(const QPoint& p) const {
    return p - mapToGlobal(QPoint(0, 0));
}

QPoint QWidget::mapTo(const QWidget* parent, const QPoint& p) const {
    // p is in this widget's coordinates
    return p + mapToGlobal(QPoint(0, 0)) - parent->mapToGlobal(QPoint(0, 0));
}

QPoint QWidget::mapFrom(const QWidget* parent, const QPoint& p) const {
    // p is in parent's coordinates
    return p + parent->mapToGlobal(QPoint(0, 0)) - mapToGlobal(QPoint(0, 0));
}

void QWidget::raise() {
    if (d_->parent) {
        QWidgetPrivate* pd = d_->parent->d_func();
        pd->children.removeAll(this);
        pd->children.append(this);
        QWidgetWindow* win = windowHandle();
        if (win) win->updateArea(qtcMapToWindowRect(this, rect()));
    } else if (d_->window && d_->window->hwnd) {
        SetWindowPos(d_->window->hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void QWidget::lower() {
    if (d_->parent) {
        QWidgetPrivate* pd = d_->parent->d_func();
        pd->children.removeAll(this);
        pd->children.prepend(this);
        QWidgetWindow* win = windowHandle();
        if (win) win->updateArea(qtcMapToWindowRect(this, rect()));
    } else if (d_->window && d_->window->hwnd) {
        SetWindowPos(d_->window->hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void QWidget::notifyThemeChanged() {
    QStyleSheet::invalidate(this);
    QEvent e(QEvent::ThemeChange);
    event(&e);
    for (int i = 0; i < d_->children.count(); ++i) d_->children[i]->notifyThemeChanged();
}

// ---------------------------------------------------------------------------
// QWidget properties
// ---------------------------------------------------------------------------
static const QMetaObject::PropertyData qtc_props_QWidget[] = {
    { "windowTitle", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "geometry", "QRect", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "minimumSize", "QSize", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "maximumSize", "QSize", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "font", "QFont", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "styleSheet", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "toolTip", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "statusTip", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "enabled", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "visible", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "focusPolicy", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::FocusPolicy" }
};

static const QMetaObject::SignalInfo qtc_signals_QWidget[] = {
    { "windowTitleChanged", "windowTitleChanged(QString)" },
    { "themeChanged", "themeChanged()" }
};

const QMetaObject QWidget::staticMetaObject = {
    &QObject::staticMetaObject, "QWidget",
    qtc_props_QWidget, 11, qtc_signals_QWidget, 2
};

QVariant QWidget::qtcReadProperty(int id) const {
    const int base = QObject::staticMetaObject.propertyCount();
    if (id < base) return QObject::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(windowTitle());
    case 1: return QVariant(geometry());
    case 2: return QVariant(minimumSize());
    case 3: return QVariant(maximumSize());
    case 4: return QVariant(font());
    case 5: return QVariant(styleSheet());
    case 6: return QVariant(toolTip());
    case 7: return QVariant(statusTip());
    case 8: return QVariant(isEnabled());
    case 9: return QVariant(isVisible());
    case 10: return QVariant((int)focusPolicy());
    default: break;
    }
    return QVariant();
}

bool QWidget::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QObject::staticMetaObject.propertyCount();
    if (id < base) return QObject::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setWindowTitle(value.toString()); return true;
    case 1: setGeometry(value.toRect()); return true;
    case 2: setMinimumSize(value.toSize()); return true;
    case 3: setMaximumSize(value.toSize()); return true;
    case 4: setFont(value.toFont()); return true;
    case 5: setStyleSheet(value.toString()); return true;
    case 6: setToolTip(value.toString()); return true;
    case 7: setStatusTip(value.toString()); return true;
    case 8: setEnabled(value.toBool()); return true;
    case 9: setVisible(value.toBool()); return true;
    case 10: setFocusPolicy((Qt::FocusPolicy)value.toInt()); return true;
    default: break;
    }
    return false;
}

// ---------------------------------------------------------------------------
// QWidgetFactory
// ---------------------------------------------------------------------------
namespace {
struct Registry {
    QList<QWidgetClassInfo> classes;
    bool sorted;
    Registry() : sorted(false) {}
};
Registry& registry() { static Registry r; return r; }
}

void QWidgetFactory::registerClass(const QWidgetClassInfo& info) {
    Registry& r = registry();
    for (int i = 0; i < r.classes.count(); ++i) {
        if (strcmp(r.classes[i].className, info.className) == 0) {
            r.classes[i] = info;
            return;
        }
    }
    r.classes.append(info);
    r.sorted = false;
}

extern void qtcWidgetAnchor_basic();
extern void qtcWidgetAnchor_value();
extern void qtcWidgetAnchor_containers();
extern void qtcWidgetAnchor_views();
extern void qtcWidgetAnchor_menus();
extern void qtcWidgetAnchor_dialogs();
extern void qtcWidgetAnchor_textedit();

static void qtcLinkWidgetObjects() {
    static bool linked = false;
    if (linked) return;
    linked = true;
    // Referencing one symbol per translation unit keeps the object file (and
    // therefore its static widget registrars) in the final binary even when
    // the framework is consumed as a static library.
    qtcWidgetAnchor_basic();
    qtcWidgetAnchor_value();
    qtcWidgetAnchor_containers();
    qtcWidgetAnchor_views();
    qtcWidgetAnchor_menus();
    qtcWidgetAnchor_dialogs();
    qtcWidgetAnchor_textedit();
}

QList<QWidgetClassInfo> QWidgetFactory::classes() {
    qtcLinkWidgetObjects();
    return registry().classes;
}

const QWidgetClassInfo* QWidgetFactory::classInfo(const char* className) {
    Registry& r = registry();
    for (int i = 0; i < r.classes.count(); ++i)
        if (strcmp(r.classes[i].className, className) == 0) return &r.classes[i];
    return 0;
}

bool QWidgetFactory::isRegistered(const char* className) {
    return classInfo(className) != 0;
}

QWidget* QWidgetFactory::create(const char* className) {
    qtcLinkWidgetObjects();
    const QWidgetClassInfo* info = classInfo(className);
    if (!info || !info->create) return 0;
    return info->create();
}
