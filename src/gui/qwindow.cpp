// QtClone - qwindow.cpp : the native window backing one top-level QWidget.
#include "qwidget_p.h"
#include "qtc_gdiplus.h"
#include <qtc/QStyle.h>
#include <imm.h>
#include <windowsx.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>

using namespace Gdiplus;

namespace {

std::map<HWND, QWidgetWindow*>& windowMap() { static std::map<HWND, QWidgetWindow*> m; return m; }
bool g_classRegistered = false;
int g_windowCount = 0;

const wchar_t* kWindowClass = L"QtCloneWindow";

QWidget* deepestWidgetAt(QWidget* widget, const QPoint& pos, bool includeDisabled) {
    if (!widget) return 0;
    QWidgetPrivate* d = qtcWidgetPrivate(widget);
    if (!d) return 0;
    if (!includeDisabled && !widget->isEnabled()) return widget;
    for (int i = d->children.count() - 1; i >= 0; --i) {
        QWidget* child = d->children[i];
        QWidgetPrivate* cd = qtcWidgetPrivate(child);
        if (!cd->visible) continue;
        if (cd->attrs.testFlag(Qt::WA_TransparentForMouseEvents)) continue;
        QRect g = cd->geometry;
        if (g.contains(pos)) {
            QWidget* deeper = deepestWidgetAt(child, pos - g.topLeft(), includeDisabled);
            if (deeper) return deeper;
            return child;
        }
    }
    return widget;
}

} // namespace

int QWidgetWindow::windowCount() { return g_windowCount; }
QWidgetWindow* QWidgetWindow::fromHwnd(HWND hwnd) {
    std::map<HWND, QWidgetWindow*>::iterator it = windowMap().find(hwnd);
    return it == windowMap().end() ? 0 : it->second;
}

QWidgetWindow::QWidgetWindow(QWidget* w)
    : hwnd(0), widget(w), hover(0), grabber(0), focus(0), defaultWidget(0),
      visible(false), inPaint(false), popup(false), trackingLeave(false),
      lastButtons(Qt::NoButton), style(0), exStyle(0) {}

QWidgetWindow::~QWidgetWindow() {
    if (hwnd) destroy();
}

// ---------------------------------------------------------------------------
// window creation
// ---------------------------------------------------------------------------
bool QWidgetWindow::create() {
    if (!widget) return false;
    if (!g_classRegistered) {
        WNDCLASSEXW wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = &QWidgetWindow::wndProc;
        wc.hInstance = GetModuleHandleW(0);
        wc.hCursor = LoadCursorW(0, IDC_ARROW);
        wc.hIcon = LoadIconW(0, IDI_APPLICATION);
        wc.lpszClassName = kWindowClass;
        if (!RegisterClassExW(&wc)) return false;
        g_classRegistered = true;
    }

    Qt::WindowFlags flags = widget->windowFlags();
    bool popupWindow = (flags & Qt::Popup) == Qt::Popup || (flags & Qt::ToolTip) == Qt::ToolTip;
    popup = popupWindow;

    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    DWORD dwExStyle = WS_EX_APPWINDOW;
    if (popupWindow) {
        dwStyle = WS_POPUP;
        if ((flags & Qt::ToolTip) != Qt::ToolTip) dwStyle |= WS_BORDER;
        dwExStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
    } else if ((flags & Qt::Dialog) == Qt::Dialog) {
        dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    }
    if (flags & Qt::FramelessWindowHint) {
        dwStyle = WS_POPUP;
        dwExStyle |= WS_EX_TOOLWINDOW;
    }
    style = (ulong)dwStyle;
    exStyle = (ulong)dwExStyle;

    QRect g = widget->geometry();
    if (g.width() <= 0) g.setWidth(800);
    if (g.height() <= 0) g.setHeight(600);

    hwnd = CreateWindowExW(dwExStyle, kWindowClass, widget->windowTitle().c_str(),
                           dwStyle, g.x(), g.y(), g.width(), g.height(),
                           0, 0, GetModuleHandleW(0), this);
    if (!hwnd) return false;
    windowMap()[hwnd] = this;
    ++g_windowCount;

    RECT rc;
    GetClientRect(hwnd, &rc);
    resizeBackingStore(rc.right - rc.left, rc.bottom - rc.top);
    return true;
}

void QWidgetWindow::destroy() {
    if (!hwnd) return;
    windowMap().erase(hwnd);
    DestroyWindow(hwnd);
    hwnd = 0;
    visible = false;
    --g_windowCount;
}

void QWidgetWindow::show() {
    if (!hwnd) return;
    int cmd = popup ? SW_SHOWNOACTIVATE : SW_SHOWNORMAL;
    ShowWindow(hwnd, cmd);
    if (!popup) SetForegroundWindow(hwnd);
    visible = true;
    dirty.unite(QRect(0, 0, widget->width(), widget->height()));
    if (popup) SetCapture(hwnd);
    UpdateWindow(hwnd);
}

void QWidgetWindow::hide() {
    if (!hwnd) return;
    if (popup && GetCapture() == hwnd) ReleaseCapture();
    ShowWindow(hwnd, SW_HIDE);
    visible = false;
}

void QWidgetWindow::setTitle(const QString& title) {
    if (hwnd) SetWindowTextW(hwnd, title.c_str());
}

void QWidgetWindow::setWindowIcon(const QIcon& icon) {
    if (!hwnd || icon.isNull()) return;
    QPixmap pm = icon.pixmap(QSize(32, 32));
    if (pm.isNull()) return;
    HBITMAP hbmp = 0;
    if (pm.bitmap()) ((Bitmap*)pm.bitmap())->GetHBITMAP(Color(0, 0, 0, 0), &hbmp);
    if (hbmp) {
        HICON hicon = 0;
        ICONINFO ii;
        ZeroMemory(&ii, sizeof(ii));
        ii.fIcon = TRUE;
        ii.hbmMask = hbmp;
        ii.hbmColor = hbmp;
        hicon = CreateIconIndirect(&ii);
        if (hicon) {
            SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicon);
            SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
        }
        DeleteObject(hbmp);
    }
}

void QWidgetWindow::updateWindowFlags() {
    if (!hwnd) return;
    long s = (long)style;
    long e = (long)exStyle;
    SetWindowLongW(hwnd, GWL_STYLE, s);
    SetWindowLongW(hwnd, GWL_EXSTYLE, e);
    SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

// ---------------------------------------------------------------------------
// painting
// ---------------------------------------------------------------------------
void QWidgetWindow::resizeBackingStore(int w, int h) {
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
    if (!back.isNull() && back.width() == w && back.height() == h) return;
    back = QPixmap(w, h);
    back.fill(widget ? widget->palette().color(QPalette::Window) : QColor(0xF0, 0xF0, 0xF0));
    dirty.setRects(QRect(0, 0, w, h));
}

void QWidgetWindow::updateArea(const QRect& r) {
    if (!widget) return;
    QRect client = r.intersected(QRect(0, 0, widget->width(), widget->height()));
    if (client.isEmpty()) return;
    dirty.unite(client);
    if (hwnd) InvalidateRect(hwnd, 0, FALSE);
}

void QWidgetWindow::updateAll() {
    if (!widget) return;
    dirty.unite(QRect(0, 0, widget->width(), widget->height()));
    if (hwnd) InvalidateRect(hwnd, 0, FALSE);
}

void QWidgetWindow::paint(HDC targetDc) {
    if (!widget || !hwnd) return;
    if (inPaint) return;
    QRect client(0, 0, widget->width(), widget->height());
    if (dirty.isEmpty()) dirty.unite(client);
    QRect bbox = dirty.boundingRect().intersected(client);
    if (bbox.isEmpty()) return;

    if (back.isNull() || back.width() != widget->width() || back.height() != widget->height())
        resizeBackingStore(widget->width(), widget->height());

    inPaint = true;
    {
        QPainter p(&back);
        p.setClipRect(bbox);
        widget->renderInternal(&p, QPoint(0, 0), bbox);
    }

    HDC dc = targetDc;
    bool ownDc = false;
    if (!dc) { dc = GetDC(hwnd); ownDc = true; }
    if (dc) {
        Graphics g(dc);
        g.SetCompositingMode(CompositingModeSourceCopy);
        g.SetInterpolationMode(InterpolationModeNearestNeighbor);
        g.SetPixelOffsetMode(PixelOffsetModeHalf);
        if (back.bitmap()) {
            g.DrawImage((Bitmap*)back.bitmap(),
                        Rect(bbox.x(), bbox.y(), bbox.width(), bbox.height()),
                        bbox.x(), bbox.y(), bbox.width(), bbox.height(), UnitPixel);
        }
    }
    if (ownDc && dc) ReleaseDC(hwnd, dc);
    dirty = QRegion();
    inPaint = false;
}

void QWidgetWindow::flush(const QRect& clientRect) {
    if (!hwnd) return;
    QRect r = clientRect.intersected(QRect(0, 0, widget->width(), widget->height()));
    if (r.isEmpty()) return;
    dirty.unite(r);
    HDC dc = GetDC(hwnd);
    if (dc) {
        paint(dc);
        ReleaseDC(hwnd, dc);
    }
}

QPoint QWidgetWindow::mapToGlobal(const QPoint& client) const {
    if (!hwnd) return client;
    POINT pt;
    pt.x = client.x();
    pt.y = client.y();
    ClientToScreen(hwnd, &pt);
    return QPoint(pt.x, pt.y);
}

QPoint QWidgetWindow::mapFromGlobal(const QPoint& global) const {
    if (!hwnd) return global;
    POINT pt;
    pt.x = global.x();
    pt.y = global.y();
    ScreenToClient(hwnd, &pt);
    return QPoint(pt.x, pt.y);
}

// ---------------------------------------------------------------------------
// focus / hover
// ---------------------------------------------------------------------------
void QWidgetWindow::setFocusWidget(QWidget* w, Qt::FocusReason reason) {
    if (w && w->focusProxy()) {
        setFocusWidget(w->focusProxy(), reason);
        return;
    }
    if (focus == w) return;
    QWidget* oldFocus = focus;
    focus = w;
    if (oldFocus) {
        oldFocus->d_func()->hasFocus = false;
        QFocusEvent ev(QEvent::FocusOut, reason);
        qtcSendEvent(oldFocus, &ev);
        oldFocus->update();
    }
    if (w) {
        w->d_func()->hasFocus = true;
        QFocusEvent ev(QEvent::FocusIn, reason);
        qtcSendEvent(w, &ev);
        w->update();
    }
}

void QWidgetWindow::setHoverWidget(QWidget* w) {
    if (hover == w) return;
    // collect ancestor chains
    QList<QWidget*> oldChain, newChain;
    for (QWidget* p = hover; p && p != widget; p = p->parentWidget()) oldChain.append(p);
    for (QWidget* p = w; p && p != widget; p = p->parentWidget()) newChain.append(p);

    for (int i = 0; i < oldChain.count(); ++i) {
        if (newChain.indexOf(oldChain[i]) < 0) {
            QEvent ev(QEvent::Leave);
            qtcSendEvent(oldChain[i], &ev);
        }
    }
    for (int i = newChain.count() - 1; i >= 0; --i) {
        if (oldChain.indexOf(newChain[i]) < 0) {
            QEvent ev(QEvent::Enter);
            qtcSendEvent(newChain[i], &ev);
        }
    }
    hover = w;
}

void QWidgetWindow::applyCursor() {
    if (!hwnd) return;
    QWidget* w = deepestWidgetAt(widget, mapFromGlobal(QCursor::pos()), false);
    HCURSOR h = LoadCursorW(0, IDC_ARROW);
    while (w) {
        QCursor c = w->cursor();
        if (w->d_func()->hasCursor) { h = (HCURSOR)c.handle(); break; }
        w = w->parentWidget();
    }
    if (!h) h = LoadCursorW(0, IDC_ARROW);
    SetCursor(h);
}

QWidget* QWidgetWindow::widgetAt(const QPoint& clientPos, bool includeDisabled) {
    if (!widget) return 0;
    QWidget* w = deepestWidgetAt(widget, clientPos, includeDisabled);
    return w == widget ? widget : w;
}

// ---------------------------------------------------------------------------
// mouse / wheel
// ---------------------------------------------------------------------------
void QWidgetWindow::handleMouse(QEvent::Type type, const QPoint& clientPos,
                                Qt::MouseButton button, Qt::MouseButtons buttons,
                                Qt::KeyboardModifiers mods) {
    if (!widget) return;

    QWidget* target = 0;
    QPoint localPos = clientPos;
    if (grabber) {
        target = grabber;
        localPos = grabber->mapFrom(widget, clientPos);
    } else {
        target = deepestWidgetAt(widget, clientPos, false);
        if (target) localPos = target->mapFrom(widget, clientPos);
    }

    if (type == QEvent::MouseButtonPress) {
        if (popup && !QRect(0, 0, widget->width(), widget->height()).contains(clientPos)) {
            widget->hide();
            return;
        }
        setHoverWidget(target);
        if (target) {
            Qt::FocusPolicy fp = target->focusPolicy();
            if ((fp & Qt::ClickFocus) || (fp & Qt::StrongFocus)) target->setFocus(Qt::MouseFocusReason);
            grabber = target;
            SetCapture(hwnd);
        }
    } else if (type == QEvent::MouseButtonRelease) {
        if (!grabber && target) {
            // release delivered to the widget that accepts it
        }
    }

    if (target) {
QMouseEvent ev(type, localPos, mapToGlobal(clientPos), button, buttons, mods);
        qtcSendEvent(target, &ev);
    }

    if (type == QEvent::MouseButtonRelease) {
        if (grabber) {
            grabber = 0;
            if (GetCapture() == hwnd) ReleaseCapture();
        }
        if (popup) widget->hide();
    }
}

void QWidgetWindow::handleWheel(const QPoint& clientPos, int delta, Qt::MouseButtons buttons,
                               Qt::KeyboardModifiers mods) {
    QWidget* target = deepestWidgetAt(widget, clientPos, false);
    if (!target) return;
    QPoint local = target->mapFrom(widget, clientPos);
    QWheelEvent ev(local, mapToGlobal(clientPos), delta, buttons, mods, Qt::Vertical);
    qtcSendEvent(target, &ev);
}

// ---------------------------------------------------------------------------
// keyboard / IME
// ---------------------------------------------------------------------------
void QWidgetWindow::handleKey(QEvent::Type type, int key, Qt::KeyboardModifiers mods,
                              const QString& text) {
    if (!focus) return;
    QKeyEvent ev(type, key, mods, text);
    qtcSendEvent(focus, &ev);
    if (!ev.isAccepted() && type == QEvent::KeyPress && key == Qt::Key_Tab) {
        focus->focusNextPrevChild(true);
    } else if (!ev.isAccepted() && type == QEvent::KeyPress && key == Qt::Key_Backtab) {
        focus->focusNextPrevChild(false);
    }
}

void QWidgetWindow::handleIme(const QString& commit, const QString& preedit) {
    if (!focus) return;
    QInputMethodEvent ev;
    if (!preedit.isNull()) ev.setPreeditString(preedit);
    if (!commit.isEmpty()) ev.setCommitString(commit);
    qtcSendEvent(focus, &ev);
}

// ---------------------------------------------------------------------------
// window procedure
// ---------------------------------------------------------------------------
LRESULT CALLBACK QWidgetWindow::wndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = (CREATESTRUCTW*)l;
        QWidgetWindow* self = (QWidgetWindow*)cs->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)self);
        if (self) self->hwnd = hwnd;
    }
    QWidgetWindow* self = (QWidgetWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (!self) return DefWindowProcW(hwnd, msg, w, l);
    return self->handleMessage(hwnd, msg, w, l);
}

LRESULT QWidgetWindow::handleMessage(HWND h, UINT msg, WPARAM w, LPARAM l) {
    switch (msg) {
    case WM_SIZE: {
        if (!widget) break;
        int nw = LOWORD(l), nh = HIWORD(l);
        QWidgetPrivate* d = widget->d_func();
        QSize oldSize = d->geometry.size();
        RECT rc;
        GetWindowRect(h, &rc);
        d->geometry = QRect(rc.left, rc.top, nw, nh);
        resizeBackingStore(nw, nh);
        dirty.unite(QRect(0, 0, nw, nh));
        if (oldSize != QSize(nw, nh)) {
            QResizeEvent ev(QSize(nw, nh), oldSize);
            qtcSendEvent(widget, &ev);
        }
        InvalidateRect(h, 0, FALSE);
        return 0;
    }
    case WM_MOVE: {
        if (!widget) break;
        RECT rc;
        GetWindowRect(h, &rc);
        widget->d_func()->geometry = QRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        if (widget) {
            if (dirty.isEmpty()) {
                RECT rc;
                GetClientRect(h, &rc);
                dirty.unite(QRect(0, 0, rc.right, rc.bottom));
            }
            paint(dc);
        }
        EndPaint(h, &ps);
        return 0;
    }
    case WM_MOUSEMOVE: {
        QPoint pos(GET_X_LPARAM(l), GET_Y_LPARAM(l));
        if (pos != lastMousePos) {
            lastMousePos = pos;
            if (widget && widget->hasMouseTracking()) {
                // fall through to dispatch below
            }
        }
        if (!grabber) setHoverWidget(deepestWidgetAt(widget, pos, false));
        handleMouse(QEvent::MouseMove, pos, Qt::NoButton,
                    lastButtons | qtcQueryMouseButtons(), qtcQueryModifiers());
        return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN: {
        int mid = GetDoubleClickTime();
        SetFocus(h);
        QPoint pos(GET_X_LPARAM(l), GET_Y_LPARAM(l));
        Qt::MouseButton b = msg == WM_LBUTTONDOWN ? Qt::LeftButton
                           : (msg == WM_RBUTTONDOWN ? Qt::RightButton : Qt::MidButton);
        lastButtons = lastButtons | b;
        handleMouse(QEvent::MouseButtonPress, pos, b, lastButtons, qtcQueryModifiers());
        return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP: {
        QPoint pos(GET_X_LPARAM(l), GET_Y_LPARAM(l));
        Qt::MouseButton b = msg == WM_LBUTTONUP ? Qt::LeftButton
                           : (msg == WM_RBUTTONUP ? Qt::RightButton : Qt::MidButton);
        lastButtons = lastButtons & ~b;
        if (msg == WM_RBUTTONUP) {
            QWidget* target = grabber ? grabber : deepestWidgetAt(widget, pos, false);
            if (target) {
                QContextMenuEvent ev(target->mapFrom(widget, pos), mapToGlobal(pos));
                qtcSendEvent(target, &ev);
            }
        }
        handleMouse(QEvent::MouseButtonRelease, pos, b, lastButtons, qtcQueryModifiers());
        return 0;
    }
    case WM_LBUTTONDBLCLK: {
        QPoint pos(GET_X_LPARAM(l), GET_Y_LPARAM(l));
        lastButtons = lastButtons | Qt::LeftButton;
        handleMouse(QEvent::MouseButtonDblClick, pos, Qt::LeftButton, lastButtons, qtcQueryModifiers());
        return 0;
    }
    case WM_MOUSEWHEEL: {
        POINT pt;
        pt.x = GET_X_LPARAM(l);
        pt.y = GET_Y_LPARAM(l);
        ScreenToClient(h, &pt);
        int delta = GET_WHEEL_DELTA_WPARAM(w);
        handleWheel(QPoint(pt.x, pt.y), delta, qtcQueryMouseButtons(), qtcQueryModifiers());
        return 0;
    }
    case WM_SETCURSOR:
        applyCursor();
        return TRUE;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        bool printable = false;
        int key = qtcKeyFromVirtualKey(w, l, &printable);
        Qt::KeyboardModifiers mods = qtcQueryModifiers();
        handleKey(QEvent::KeyPress, key, mods, QString());
        if (msg == WM_SYSKEYDOWN && key == Qt::Key_Alt) return 0;
        return 0;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        int key = qtcKeyFromVirtualKey(w, l, 0);
        handleKey(QEvent::KeyRelease, key, qtcQueryModifiers(), QString());
        return 0;
    }
    case WM_CHAR: {
        wchar_t ch = (wchar_t)w;
        if (ch == L'\r') return 0;   // handled through WM_KEYDOWN (Key_Return)
        if (ch == L'\t' || ch == L'\b') return 0;
        if (ch < 0x20 && ch != L'\n') return 0;
        handleKey(QEvent::KeyPress, 0, qtcQueryModifiers(), QString(ch));
        return 0;
    }
    case WM_IME_STARTCOMPOSITION: {
        if (focus && focus->d_func()) {
            COMPOSITIONFORM cf;
            ZeroMemory(&cf, sizeof(cf));
            cf.dwStyle = CFS_CANDIDATEPOS;
            QPoint caret = focus->mapToGlobal(QPoint(4, focus->height() - 4));
            cf.ptCurrentPos.x = caret.x();
            cf.ptCurrentPos.y = caret.y();
            HIMC imc = ImmGetContext(h);
            if (imc) { ImmSetCompositionWindow(imc, &cf); ImmReleaseContext(h, imc); }
        }
        return 0;
    }
    case WM_IME_COMPOSITION: {
        HIMC imc = ImmGetContext(h);
        if (!imc) return 0;
        if (l & GCS_RESULTSTR) {
            LONG bytes = ImmGetCompositionStringW(imc, GCS_RESULTSTR, 0, 0);
            if (bytes > 0) {
                int len = bytes / (int)sizeof(wchar_t);
                std::vector<wchar_t> buf((size_t)len + 1, 0);
                ImmGetCompositionStringW(imc, GCS_RESULTSTR, &buf[0], bytes);
                handleIme(QString(&buf[0]), QString());
            }
        } else if (l & GCS_COMPSTR) {
            LONG bytes = ImmGetCompositionStringW(imc, GCS_COMPSTR, 0, 0);
            if (bytes > 0) {
                int len = bytes / (int)sizeof(wchar_t);
                std::vector<wchar_t> buf((size_t)len + 1, 0);
                ImmGetCompositionStringW(imc, GCS_COMPSTR, &buf[0], bytes);
                handleIme(QString(), QString(&buf[0]));
            } else {
                handleIme(QString(), QString());
            }
        }
        ImmReleaseContext(h, imc);
        return 0;
    }
    case WM_IME_ENDCOMPOSITION:
        handleIme(QString(), QString());
        return 0;
    case WM_SETFOCUS:
        if (widget && !focus) {
            // give focus to the first focusable child
            QWidget* target = 0;
            QList<QWidget*> stack;
            stack.append(widget);
            while (!stack.isEmpty()) {
                QWidget* wd = stack.last();
                stack.removeAt(stack.count() - 1);
                if (wd != widget && wd->d_func()->visible && wd->d_func()->enabled &&
                    (wd->focusPolicy() & Qt::TabFocus)) { target = wd; break; }
                for (int i = wd->d_func()->children.count() - 1; i >= 0; --i)
                    stack.append(wd->d_func()->children[i]);
            }
            if (target) setFocusWidget(target, Qt::ActiveWindowFocusReason);
        }
        return 0;
    case WM_KILLFOCUS:
        return 0;
    case WM_ACTIVATE:
        if (widget) {
            widget->d_func()->activeWindow = (LOWORD(w) != WA_INACTIVE);
            QEvent ev(LOWORD(w) != WA_INACTIVE ? QEvent::WindowActivate : QEvent::WindowDeactivate);
            qtcSendEvent(widget, &ev);
        }
        return 0;
    case WM_SETTINGCHANGE:
        if (l && wcscmp((const wchar_t*)l, L"ImmersiveColorSet") == 0) {
            QTheme::setSystemDark(qtcIsSystemDark());
        }
        return 0;
    case WM_CLOSE:
        if (widget) widget->close();
        return 0;
    case WM_DESTROY:
        return 0;
    default:
        break;
    }
    return DefWindowProcW(h, msg, w, l);
}

// ---------------------------------------------------------------------------
// input helpers
// ---------------------------------------------------------------------------
Qt::KeyboardModifiers qtcQueryModifiers() {
    int mods = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000) mods |= Qt::ShiftModifier;
    if (GetKeyState(VK_CONTROL) & 0x8000) mods |= Qt::ControlModifier;
    if (GetKeyState(VK_MENU) & 0x8000) mods |= Qt::AltModifier;
    return Qt::KeyboardModifiers(mods);
}

Qt::MouseButtons qtcQueryMouseButtons() {
    int buttons = 0;
    if (GetKeyState(VK_LBUTTON) & 0x8000) buttons |= Qt::LeftButton;
    if (GetKeyState(VK_RBUTTON) & 0x8000) buttons |= Qt::RightButton;
    if (GetKeyState(VK_MBUTTON) & 0x8000) buttons |= Qt::MidButton;
    return Qt::MouseButtons(buttons);
}

int qtcKeyFromVirtualKey(WPARAM vk, LPARAM lp, bool* isPrintable) {
    if (isPrintable) *isPrintable = false;
    if (vk >= 'A' && vk <= 'Z') return (int)vk;
    if (vk >= '0' && vk <= '9') return (int)vk;
    switch (vk) {
    case VK_ESCAPE: return Qt::Key_Escape;
    case VK_TAB: return (lp & (1 << 24)) ? Qt::Key_Backtab : Qt::Key_Tab;
    case VK_BACK: return Qt::Key_Backspace;
    case VK_RETURN: return Qt::Key_Return;
    case VK_INSERT: return Qt::Key_Insert;
    case VK_DELETE: return Qt::Key_Delete;
    case VK_HOME: return Qt::Key_Home;
    case VK_END: return Qt::Key_End;
    case VK_LEFT: return Qt::Key_Left;
    case VK_RIGHT: return Qt::Key_Right;
    case VK_UP: return Qt::Key_Up;
    case VK_DOWN: return Qt::Key_Down;
    case VK_PRIOR: return Qt::Key_PageUp;
    case VK_NEXT: return Qt::Key_PageDown;
    case VK_SPACE: return Qt::Key_Space;
    case VK_SHIFT: return Qt::Key_Shift;
    case VK_CONTROL: return Qt::Key_Control;
    case VK_MENU: return Qt::Key_Alt;
    case VK_F1: return Qt::Key_F1;
    case VK_F2: return Qt::Key_F2;
    case VK_F3: return Qt::Key_F3;
    case VK_F4: return Qt::Key_F4;
    case VK_F5: return Qt::Key_F5;
    case VK_F6: return Qt::Key_F6;
    case VK_F7: return Qt::Key_F7;
    case VK_F8: return Qt::Key_F8;
    case VK_F9: return Qt::Key_F9;
    case VK_F10: return Qt::Key_F10;
    case VK_F11: return Qt::Key_F11;
    case VK_F12: return Qt::Key_F12;
    case VK_OEM_COMMA: return Qt::Key_Comma;
    case VK_OEM_PERIOD: return Qt::Key_Period;
    case VK_OEM_MINUS: return Qt::Key_Minus;
    case VK_OEM_PLUS: return Qt::Key_Plus;
    case VK_OEM_1: return Qt::Key_Semicolon;
    case VK_OEM_2: return Qt::Key_Slash;
    case VK_OEM_3: return Qt::Key_QuoteLeft;
    case VK_OEM_4: return Qt::Key_BracketLeft;
    case VK_OEM_5: return Qt::Key_Backslash;
    case VK_OEM_6: return Qt::Key_BracketRight;
    case VK_OEM_7: return Qt::Key_Apostrophe;
    default: break;
    }
    return Qt::Key_unknown;
}

// ---------------------------------------------------------------------------
// system theme
// ---------------------------------------------------------------------------
bool qtcIsSystemDark() {
    HKEY key = 0;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &key) != ERROR_SUCCESS) {
        return false;
    }
    DWORD value = 1;
    DWORD size = sizeof(value);
    DWORD type = 0;
    bool dark = false;
    if (RegQueryValueExW(key, L"AppsUseLightTheme", 0, &type, (BYTE*)&value, &size) == ERROR_SUCCESS) {
        dark = (value == 0);
    }
    RegCloseKey(key);
    return dark;
}
