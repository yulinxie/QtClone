// QtClone - qapplication.cpp : application object, event loop, theme, global state.
#include "qwidget_p.h"
#include <qtc/QStyle.h>
#include <qtc/QLayout.h>
#include <stdio.h>
#include <vector>

QApplication* QApplication::self_ = 0;

namespace {

struct AppState {
    QFont font;
    QPalette palette;
    QStyle* style;
    QString styleSheet;
    QWidget* activeWindow;
    QWidget* mouseGrabber;
    bool inEventLoop;
    bool quit;
    int exitCode;
    bool initialized;
    QList<QWidgetWindow*> windows;
    std::vector<QWidget*> widgets;
    AppState()
        : style(0), activeWindow(0), mouseGrabber(0), inEventLoop(false), quit(false),
          exitCode(0), initialized(false) {}
};

AppState& app() { static AppState state; return state; }

QPalette paletteForTheme() {
    QPalette p;
    const QTheme::Colors& c = QTheme::colors();
    p.setColor(QPalette::Window, c.window);
    p.setColor(QPalette::WindowText, c.windowText);
    p.setColor(QPalette::Base, c.base);
    p.setColor(QPalette::AlternateBase, c.alternateBase);
    p.setColor(QPalette::Text, c.windowText);
    p.setColor(QPalette::Button, c.button);
    p.setColor(QPalette::ButtonText, c.buttonText);
    p.setColor(QPalette::Light, c.surface);
    p.setColor(QPalette::Midlight, c.surfaceAlt);
    p.setColor(QPalette::Mid, c.border);
    p.setColor(QPalette::Dark, c.borderStrong);
    p.setColor(QPalette::Shadow, c.shadow);
    p.setColor(QPalette::Highlight, c.selection);
    p.setColor(QPalette::HighlightedText, c.selectionText);
    p.setColor(QPalette::ToolTipBase, c.toolTipBase);
    p.setColor(QPalette::ToolTipText, c.toolTipText);
    p.setColor(QPalette::PlaceholderText, c.disabledText);
    p.setColor(QPalette::Disabled, QPalette::WindowText, c.disabledText);
    p.setColor(QPalette::Disabled, QPalette::Text, c.disabledText);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, c.disabledText);
    p.setColor(QPalette::Disabled, QPalette::Highlight, c.border);
    p.setColor(QPalette::Disabled, QPalette::HighlightedText, c.disabledText);
    return p;
}

void enableDpiAwareness() {
    typedef BOOL (WINAPI *SetProcessDPIAwareFn)(void);
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (!user32) return;
    SetProcessDPIAwareFn fn = (SetProcessDPIAwareFn)GetProcAddress(user32, "SetProcessDPIAware");
    if (fn) fn();
}

QString applicationArgsJoin(int argc, char** argv) {
    QString out;
    for (int i = 0; i < argc; ++i) {
        if (i) out += L" ";
        out += qtcFromUtf8(argv[i] ? argv[i] : "");
    }
    return out;
}

} // namespace

// widget registration (called from QWidget ctor/dtor)
void qtcRegisterWidget(QWidget* widget) {
    app().widgets.push_back(widget);
}

void qtcUnregisterWidget(QWidget* widget) {
    std::vector<QWidget*>& v = app().widgets;
    for (size_t i = v.size(); i > 0; --i) {
        if (v[i - 1] == widget) {
            v.erase(v.begin() + (i - 1));
            return;
        }
    }
}

QApplication::QApplication(int& argc, char** argv) {
    self_ = this;
    AppState& a = app();
    qtcEnsureGraphics();
    a.font = QFont(L"Microsoft YaHei UI", 9);
    enableDpiAwareness();
    CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    a.style = QStyle::createDefault();
    a.palette = paletteForTheme();
    a.initialized = true;
    (void)argc;
    (void)argv;
    applicationArgsJoin(argc, argv);
}

QApplication::~QApplication() {
    for (size_t i = app().windows.size(); i > 0; --i) {
        QWidgetWindow* w = app().windows[i - 1];
        if (w) delete w;
    }
    app().windows.clearAll();
    delete app().style;
    app().style = 0;
    self_ = 0;
}

QApplication* QApplication::instance() { return self_; }

int QApplication::exec() {
    AppState& a = app();
    a.inEventLoop = true;
    a.quit = false;
    MSG msg;
    while (!a.quit) {
        if (GetMessageW(&msg, 0, 0, 0) <= 0) break;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        qtcDeliverPostedEvents();
    }
    a.inEventLoop = false;
    return a.exitCode;
}

void QApplication::quit() { app().quit = true; PostQuitMessage(0); }

void QApplication::exit(int code) {
    app().exitCode = code;
    quit();
}

void QApplication::processEvents() {
    MSG msg;
    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    qtcDeliverPostedEvents();
}

void QApplication::sendPostedEvents() { qtcDeliverPostedEvents(); }
bool QApplication::closingDown() { return app().quit; }

QWidget* QApplication::activeWindow() {
    QWidget* w = app().activeWindow;
    if (w && !w->isVisible()) return 0;
    return w;
}

void QApplication::setActiveWindow(QWidget* widget) {
    app().activeWindow = widget;
    if (widget) widget->d_func()->activeWindow = true;
}

QWidget* QApplication::focusWidget() {
    QWidget* w = activeWindow();
    if (!w) {
        // fall back to the first visible window
        QWidgetList list = topLevelWidgets();
        for (int i = 0; i < list.count(); ++i) if (list[i]->isVisible()) { w = list[i]; break; }
    }
    if (!w) return 0;
    QWidgetWindow* win = w->windowHandle();
    return win ? win->focusWidget() : 0;
}

QWidget* QApplication::widgetAt(const QPoint& globalPos) {
    QWidget* active = activeWindow();
    if (active && active->windowHandle()) {
        QWidget* hit = active->windowHandle()->widgetAt(active->windowHandle()->mapFromGlobal(globalPos));
        if (hit) return hit;
    }
    QWidgetList list = topLevelWidgets();
    for (int i = 0; i < list.count(); ++i) {
        QWidget* w = list[i];
        if (!w->isVisible() || !w->windowHandle()) continue;
        QWidget* hit = w->windowHandle()->widgetAt(w->windowHandle()->mapFromGlobal(globalPos));
        if (hit) return hit;
    }
    return 0;
}

QWidgetList QApplication::topLevelWidgets() {
    QWidgetList out;
    std::vector<QWidget*>& v = app().widgets;
    for (size_t i = 0; i < v.size(); ++i) if (v[i]->isWindow()) out.append(v[i]);
    return out;
}

QWidgetList QApplication::allWidgets() {
    QWidgetList out;
    std::vector<QWidget*>& v = app().widgets;
    for (size_t i = 0; i < v.size(); ++i) out.append(v[i]);
    return out;
}

QFont QApplication::font() { return app().font; }
void QApplication::setFont(const QFont& font) {
    app().font = font;
    QWidgetList all = allWidgets();
    for (int i = 0; i < all.count(); ++i) all[i]->updateGeometry();
    qtcRepaintAll();
}

QPalette QApplication::palette() { return app().palette; }
void QApplication::setPalette(const QPalette& palette) {
    app().palette = palette;
    qtcRepaintAll();
}

QStyle* QApplication::style() {
    if (!app().style) app().style = QStyle::createDefault();
    return app().style;
}

void QApplication::setStyle(QStyle* style) {
    if (app().style == style) return;
    delete app().style;
    app().style = style;
    qtcRepaintAll();
}

QString QApplication::styleSheet() { return app().styleSheet; }

void QApplication::setStyleSheet(const QString& sheet) {
    app().styleSheet = sheet;
    QStyleSheet::setApplicationSheet(sheet);
    qtcRepaintAll();
}

QTheme::Mode QApplication::themeMode() { return QTheme::mode(); }

void QApplication::setThemeMode(QTheme::Mode mode) { QTheme::setMode(mode); }

bool QApplication::isDarkTheme() { return QTheme::isDark(); }

QClipboard* QApplication::clipboard() {
    static QClipboard clipboard;
    return &clipboard;
}

int QApplication::doubleClickInterval() { return GetDoubleClickTime(); }

int QApplication::keyboardRepeatRate() {
    int speed = 31;
    SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0, &speed, 0);
    return speed;
}

Qt::KeyboardModifiers QApplication::keyboardModifiers() { return qtcQueryModifiers(); }
Qt::MouseButtons QApplication::mouseButtons() { return qtcQueryMouseButtons(); }

QPoint QApplication::cursorPos() {
    POINT p;
    GetCursorPos(&p);
    return QPoint(p.x, p.y);
}

void QApplication::setOverrideCursor(const QCursor&) {}
void QApplication::restoreOverrideCursor() {}

void QApplication::beep() { MessageBeep(MB_OK); }

void QApplication::qtcRegisterWindow(QWidgetWindow* window) { app().windows.append(window); }

void QApplication::qtcUnregisterWindow(QWidgetWindow* window) {
    app().windows.removeAll(window);
}

void QApplication::qtcRepaintAll() {
    for (int i = 0; i < app().windows.count(); ++i) {
        QWidgetWindow* w = app().windows[i];
        if (w && w->widget) {
            w->widget->update();
            w->updateAll();
        }
    }
}

void QApplication::qtcNotifyThemeChange() {
    app().palette = paletteForTheme();
    QStyleSheet::invalidate();
    QWidgetList all = allWidgets();
    for (int i = 0; i < all.count(); ++i) all[i]->notifyThemeChanged();
    qtcRepaintAll();
}

void QApplication::qtcRebuildStyleSheet() { QStyleSheet::invalidate(); }

void QApplication::qtcPolishedChanged() { qtcRepaintAll(); }

void QApplication::qtcSetMouseGrabber(QWidget* widget) { app().mouseGrabber = widget; }
QWidget* QApplication::qtcMouseGrabber() { return app().mouseGrabber; }

void QApplication::qtcPostQuit() { PostQuitMessage(0); }

bool QApplication::qtcInEventLoop() { return app().inEventLoop; }

void QApplication::qtcAddWidgetToPolish(QWidget*) {}

// ---------------------------------------------------------------------------
// application wide defaults used during startup
// ---------------------------------------------------------------------------
namespace qtc {
void applyDefaultPalette() { app().palette = paletteForTheme(); }
}
