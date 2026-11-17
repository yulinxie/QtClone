// QtClone click regression test.
//
// Reproduces a real window -> WM_LBUTTONDOWN/UP -> hit testing -> event
// dispatch -> repaint round trip, and asserts that the click both reaches the
// widget (state change) and produces different pixels. This is the class of
// regression that made "clicks did nothing" look like a dead UI.

#include <qtc/QtWidgets.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(bool ok, const char* what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++failures;
}

// FNV-1a over a client-rect region, read from the screen: exactly what the
// user sees. PrintWindow would force a fresh paint and hide stale repaints.
static unsigned long long hashWindowRegion(HWND hwnd, int x, int y, int cw, int ch) {
    if (!hwnd) return 0;
    POINT origin = {x, y};
    ClientToScreen(hwnd, &origin);
    RECT r;
    r.left = origin.x;
    r.top = origin.y;
    r.right = origin.x + cw;
    r.bottom = origin.y + ch;
    int w = r.right - r.left;
    int h = r.bottom - r.top;
    if (w <= 0 || h <= 0) return 0;

    HDC screen = GetDC(NULL);
    HDC mem = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateCompatibleBitmap(screen, w, h);
    HGDIOBJ old = SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, w, h, screen, r.left, r.top, SRCCOPY);

    BITMAPINFO bi;
    memset(&bi, 0, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    unsigned char* buf = (unsigned char*)malloc((size_t)w * h * 4);
    unsigned long long hash = 1469598103934665603ULL;
    if (buf && GetDIBits(mem, bmp, 0, h, buf, &bi, DIB_RGB_COLORS) == (int)h) {
        size_t total = (size_t)w * h * 4;
        for (size_t i = 0; i < total; ++i) {
            hash ^= buf[i];
            hash *= 1099511628211ULL;
        }
    }
    free(buf);
    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
    ReleaseDC(NULL, screen);
    return hash;
}

// A widget that changes its own state on click but never asks for a repaint.
// Without the framework's "click always repaints" guarantee this stays stale,
// which is exactly what users described as "the click did nothing".
class SilentBox : public QWidget {
public:
    explicit SilentBox(QWidget* parent) : QWidget(parent), on_(false) {}
    bool on() const { return on_; }

protected:
    void mousePressEvent(QMouseEvent*) { on_ = !on_; }
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        p.fillRect(rect(), on_ ? QColor(200, 60, 60) : QColor(60, 140, 200));
    }

private:
    bool on_;
};

static unsigned long long hashWindow(HWND hwnd) {
    RECT r;
    GetWindowRect(hwnd, &r);
    return hashWindowRegion(hwnd, 0, 0, r.right - r.left, r.bottom - r.top);
}

class ClickWindow : public QWidget {
public:
    ClickWindow()
        : QWidget(0, Qt::Window), windowPresses_(0), buttonClicks_(0) {
        resize(340, 220);
        setWindowTitle(L"QtClone click regression");

        label_ = new QLabel(L"clicks: 0", this);
        label_->setGeometry(20, 100, 200, 26);

        button_ = new QPushButton(L"OK", this);
        button_->setGeometry(20, 20, 120, 32);

        silent_ = new SilentBox(this);
        silent_->setGeometry(200, 20, 120, 32);
        QObject::connect(button_, button_->clicked, this, &ClickWindow::onButton);
    }

    int windowPresses() const { return windowPresses_; }
    int buttonClicks() const { return buttonClicks_; }
    SilentBox* silentBox() const { return silent_; }

protected:
    void mousePressEvent(QMouseEvent*) {
        ++windowPresses_;
        update();
    }

    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        p.fillRect(rect(), Qt::white);
        QFont f = font();
        f.setPixelSize(13);
        p.setFont(f);
        p.setPen(Qt::black);
        p.drawText(QRect(20, 60, 300, 30), 0,
                   QString::fromUtf8("window presses: ") + QString::number(windowPresses_));
    }

private:
    void onButton() {
        ++buttonClicks_;
        label_->setText(QString::fromUtf8("clicks: ") + QString::number(buttonClicks_));
    }

    QLabel* label_;
    QPushButton* button_;
    SilentBox* silent_;
    int windowPresses_;
    int buttonClicks_;
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QTheme::setMode(QTheme::Light);

    ClickWindow window;
    window.move(80, 100);
    window.show();
    for (int i = 0; i < 40; ++i) QApplication::processEvents();

    HWND hwnd = (HWND)window.winId();
    if (hwnd) {
        // topmost + foreground so the screen region belongs to us and the
        // console cursor cannot pollute the captured pixels
        SetWindowPos(hwnd, HWND_TOPMOST, 80, 100, 0, 0,
                     SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
        SetForegroundWindow(hwnd);
    }
    for (int i = 0; i < 15; ++i) {
        QApplication::processEvents();
        Sleep(30);
    }
    check(hwnd != 0, "window has a native handle");
    if (!hwnd) {
        printf("clicktest: FAILED, no HWND\n");
        return 1;
    }

    // the screen content must be stable before we trust the deltas
    unsigned long long stable1 = hashWindowRegion(hwnd, 200, 20, 120, 32);
    QApplication::processEvents();
    Sleep(120);
    QApplication::processEvents();
    unsigned long long stable2 = hashWindowRegion(hwnd, 200, 20, 120, 32);
    check(stable1 == stable2, "screen content is stable before clicking");
    if (stable1 != stable2) {
        printf("note: unstable capture, later pixel checks are advisory\n");
    }

    // 1. click the QPushButton child (hit testing + child event dispatch)
    //    the label under it shows the click count, so the button's rect changes
    unsigned long long before = hashWindowRegion(hwnd, 20, 100, 140, 26);
    LPARAM buttonPoint = MAKELPARAM(80, 36);
    SendMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, buttonPoint);
    SendMessageW(hwnd, WM_LBUTTONUP, 0, buttonPoint);
    for (int i = 0; i < 20; ++i) QApplication::processEvents();
    check(window.buttonClicks() == 1, "QPushButton received the click");
    unsigned long long afterButton = hashWindowRegion(hwnd, 20, 100, 140, 26);
    check(before != afterButton, "pixels changed after clicking the button");

    // 2. click the window background (custom mousePressEvent)
    unsigned long long beforeWindow = hashWindowRegion(hwnd, 20, 60, 300, 30);
    LPARAM emptyPoint = MAKELPARAM(280, 190);
    SendMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, emptyPoint);
    SendMessageW(hwnd, WM_LBUTTONUP, 0, emptyPoint);
    for (int i = 0; i < 20; ++i) QApplication::processEvents();
    check(window.windowPresses() >= 1, "window received the click");
    unsigned long long afterWindow = hashWindowRegion(hwnd, 20, 60, 300, 30);
    check(beforeWindow != afterWindow, "pixels changed after clicking the window");

    // 3. a widget that forgets to call update() must still show the change
    unsigned long long beforeSilent = hashWindowRegion(hwnd, 200, 20, 120, 32);
    LPARAM silentPoint = MAKELPARAM(260, 36);
    SendMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, silentPoint);
    SendMessageW(hwnd, WM_LBUTTONUP, 0, silentPoint);
    for (int i = 0; i < 20; ++i) QApplication::processEvents();
    check(window.silentBox()->on(), "silent widget received the click");
    unsigned long long afterSilent = hashWindowRegion(hwnd, 200, 20, 120, 32);
    check(beforeSilent != afterSilent, "pixels changed even though widget skipped update()");

    // 4. second button click must still work
    SendMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, buttonPoint);
    SendMessageW(hwnd, WM_LBUTTONUP, 0, buttonPoint);
    for (int i = 0; i < 20; ++i) QApplication::processEvents();
    check(window.buttonClicks() == 2, "second click still works");

    // 5. QMessageBox: pressing a standard button must close the box. This is
    //    the regression behind "the exit confirmation dialog ignores clicks".
    QMessageBox box(QMessageBox::Question, L"Q", L"close me",
                    QMessageBox::Yes | QMessageBox::No, &window);
    box.show();
    for (int i = 0; i < 20; ++i) QApplication::processEvents();
    HWND boxHwnd = (HWND)box.winId();
    QWidgetList kids = box.childrenWidgets();
    QWidget* standard = 0;
    for (int i = 0; i < kids.count(); ++i) {
        if (kids[i]->width() == 84 && kids[i]->height() == 30) { standard = kids[i]; break; }
    }
    check(boxHwnd != 0 && standard != 0, "message box has a standard button");
    if (boxHwnd && standard) {
        QPoint hit = standard->mapTo(&box, QPoint(standard->width() / 2, standard->height() / 2));
        LPARAM pt = MAKELPARAM(hit.x(), hit.y());
        SendMessageW(boxHwnd, WM_LBUTTONDOWN, MK_LBUTTON, pt);
        SendMessageW(boxHwnd, WM_LBUTTONUP, 0, pt);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();
        check(!box.isVisible(), "message box closed after pressing its button");
    }

    printf("clicktest: %s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
