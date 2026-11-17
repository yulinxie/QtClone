// QtClone demo: an IDA Pro 9 style interface built entirely with the QtClone
// widgets and QPainter.  It is a UI mock-up (no disassembler behind it), meant
// to exercise the framework: ribbon, splitters, dock-like panels, custom
// painted listing/graph views, tabs, status bar and light/dark themes.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include <stdio.h>
#include <string.h>
#include <functional>

// ===========================================================================
// palette (IDA-like colours, theme aware)
// ===========================================================================
struct IdaColors {
    QColor window;
    QColor panel;
    QColor gutter;
    QColor text;
    QColor muted;
    QColor keyword;
    QColor control;
    QColor reg;
    QColor number;
    QColor string;
    QColor comment;
    QColor label;
    QColor selection;
    QColor border;
    QColor accent;
    QColor currentLine;
};

static IdaColors idaColors() {
    IdaColors c;
    if (QTheme::isDark()) {
        c.window = QColor(0x1E, 0x1E, 0x1E);
        c.panel = QColor(0x25, 0x25, 0x26);
        c.gutter = QColor(0x2D, 0x2D, 0x30);
        c.text = QColor(0xD4, 0xD4, 0xD4);
        c.muted = QColor(0x85, 0x85, 0x85);
        c.keyword = QColor(0x56, 0x9C, 0xD6);
        c.control = QColor(0xC5, 0x86, 0xC0);
        c.reg = QColor(0x9C, 0xDC, 0xFE);
        c.number = QColor(0xB5, 0xCE, 0xA8);
        c.string = QColor(0xCE, 0x91, 0x78);
        c.comment = QColor(0x6A, 0x99, 0x55);
        c.label = QColor(0xDC, 0xDC, 0xAA);
        c.selection = QColor(0x26, 0x4F, 0x78);
        c.border = QColor(0x3F, 0x3F, 0x46);
        c.accent = QColor(0x00, 0x7A, 0xCC);
        c.currentLine = QColor(0x2A, 0x2D, 0x2E);
    } else {
        c.window = QColor(0xFF, 0xFF, 0xFF);
        c.panel = QColor(0xF3, 0xF3, 0xF3);
        c.gutter = QColor(0xEF, 0xEF, 0xEF);
        c.text = QColor(0x1E, 0x1E, 0x1E);
        c.muted = QColor(0x6E, 0x6E, 0x6E);
        c.keyword = QColor(0x00, 0x00, 0xFF);
        c.control = QColor(0xAF, 0x00, 0xDB);
        c.reg = QColor(0x00, 0x70, 0xC1);
        c.number = QColor(0x09, 0x86, 0x58);
        c.string = QColor(0xA3, 0x15, 0x15);
        c.comment = QColor(0x00, 0x80, 0x00);
        c.label = QColor(0x79, 0x5E, 0x26);
        c.selection = QColor(0xAD, 0xD6, 0xFF);
        c.border = QColor(0xD4, 0xD4, 0xD4);
        c.accent = QColor(0x00, 0x66, 0xCC);
        c.currentLine = QColor(0xEE, 0xF4, 0xFB);
    }
    return c;
}

static QFont monoFont(int pixels = 13) {
    QFont f(L"Consolas", 10);
    f.setPixelSize(pixels);
    return f;
}

static QString hex8(int value) {
    char buf[16];
    _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%08X", value);
    return QString::fromUtf8(buf);
}

static QString hex4(int value) {
    char buf[16];
    _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%04X", value);
    return QString::fromUtf8(buf);
}

// ===========================================================================
// demo data
// ===========================================================================
struct DisasmLine {
    QString address;
    QString bytes;
    QString text;
    int kind; // 0 code, 1 label, 2 call, 3 jump, 4 ret
};

struct FunctionInfo {
    QString name;
    QString address;
    QString segment;
    int length;
    QString type;
    QString prototype;
    QList<DisasmLine> lines;
    QStringList pseudo;
};

static DisasmLine mk(const char* address, const char* bytes, const char* text, int kind = 0) {
    DisasmLine line;
    line.address = QString::fromUtf8(address);
    line.bytes = QString::fromUtf8(bytes);
    line.text = QString::fromUtf8(text);
    line.kind = kind;
    return line;
}

static void fillFunctions(QList<FunctionInfo>* out) {
    FunctionInfo f;

    f.name = L"sub_401000";
    f.address = L"00401000";
    f.segment = L".text";
    f.length = 0x52;
    f.type = L"int()";
    f.prototype = L"int __cdecl sub_401000(int argc, const char **argv)";
    f.lines
        << mk("00401000", "55", "push    ebp")
        << mk("00401001", "8B EC", "mov     ebp, esp")
        << mk("00401003", "81 EC C0 00 00 00", "sub     esp, 0C0h")
        << mk("00401009", "53", "push    ebx")
        << mk("0040100A", "56", "push    esi")
        << mk("0040100B", "57", "push    edi")
        << mk("0040100C", "8D 85 40 FF FF FF", "lea     eax, [ebp-0C0h]")
        << mk("00401012", "50", "push    eax")
        << mk("00401013", "FF 15 04 20 40 00", "call    ds:GetCommandLineA", 2)
        << mk("00401019", "83 C4 04", "add     esp, 4")
        << mk("0040101C", "8D 85 40 FF FF FF", "lea     eax, [ebp-0C0h]")
        << mk("00401022", "50", "push    eax")
        << mk("00401023", "E8 D8 00 00 00", "call    sub_401100", 2)
        << mk("00401028", "83 C4 04", "add     esp, 4")
        << mk("0040102B", "85 C0", "test    eax, eax")
        << mk("0040102D", "74 1E", "jz      short loc_40104D", 3)
        << mk("0040102F", "68 30 40 40 00", "push    offset aCorrect ; \"Correct!\"")
        << mk("00401034", "E8 C7 0F 00 00", "call    sub_402000", 2)
        << mk("00401039", "83 C4 04", "add     esp, 4")
        << mk("0040103C", "B8 01 00 00 00", "mov     eax, 1")
        << mk("00401041", "5F", "pop     edi")
        << mk("00401042", "5E", "pop     esi")
        << mk("00401043", "5B", "pop     ebx")
        << mk("00401044", "8B E5", "mov     esp, ebp")
        << mk("00401046", "5D", "pop     ebp")
        << mk("00401047", "C3", "retn", 4)
        << mk("00401048", "CC", "align   4")
        << mk("0040104D", "68 44 40 40 00", "push    offset aWrong ; \"Wrong!\"")
        << mk("00401052", "E8 A9 0F 00 00", "call    sub_402000", 2)
        << mk("00401057", "83 C4 04", "add     esp, 4")
        << mk("0040105A", "33 C0", "xor     eax, eax")
        << mk("0040105C", "5F", "pop     edi")
        << mk("0040105D", "5E", "pop     esi")
        << mk("0040105E", "5B", "pop     ebx")
        << mk("0040105F", "8B E5", "mov     esp, ebp")
        << mk("00401061", "5D", "pop     ebp")
        << mk("00401062", "C3", "retn", 4);
    f.pseudo
        << L"int __cdecl sub_401000(int argc, const char **argv)"
        << L"{"
        << L"  char Buffer[192]; // [esp+0h] [ebp-0C0h]"
        << L""
        << L"  GetCommandLineA(Buffer);"
        << L"  if ( sub_401100(Buffer) )"
        << L"  {"
        << L"    printf(\"Correct!\");"
        << L"    return 1;"
        << L"  }"
        << L"  printf(\"Wrong!\");"
        << L"  return 0;"
        << L"}";
    out->append(f);

    f = FunctionInfo();
    f.name = L"sub_401100";
    f.address = L"00401100";
    f.segment = L".text";
    f.length = 0x27;
    f.type = L"int()";
    f.prototype = L"int __cdecl sub_401100(char *Str)";
    f.lines
        << mk("00401100", "55", "push    ebp")
        << mk("00401101", "8B EC", "mov     ebp, esp")
        << mk("00401103", "8B 4D 08", "mov     ecx, [ebp+arg_0]")
        << mk("00401106", "33 C0", "xor     eax, eax")
        << mk("00401108", "8A 01", "mov     al, [ecx]")
        << mk("0040110A", "84 C0", "test    al, al")
        << mk("0040110C", "74 19", "jz      short loc_401127", 3)
        << mk("0040110E", "0F BE 01", "movsx   eax, byte ptr [ecx]", 1)
        << mk("00401111", "35 5A 00 00 00", "xor     eax, 5Ah")
        << mk("00401116", "88 01", "mov     [ecx], al")
        << mk("00401118", "41", "inc     ecx")
        << mk("00401119", "8A 01", "mov     al, [ecx]")
        << mk("0040111B", "84 C0", "test    al, al")
        << mk("0040111D", "75 EF", "jnz     short loc_40110E", 3)
        << mk("0040111F", "B8 01 00 00 00", "mov     eax, 1")
        << mk("00401124", "5D", "pop     ebp")
        << mk("00401125", "C3", "retn", 4)
        << mk("00401127", "33 C0", "xor     eax, eax")
        << mk("00401129", "5D", "pop     ebp")
        << mk("0040112A", "C3", "retn", 4);
    f.pseudo
        << L"int __cdecl sub_401100(char *Str)"
        << L"{"
        << L"  int result; // eax"
        << L"  char v2; // al"
        << L""
        << L"  result = 0;"
        << L"  for ( v2 = *Str; v2; v2 = *++Str )"
        << L"  {"
        << L"    *Str ^= 0x5A;"
        << L"    result = 1;"
        << L"  }"
        << L"  return result;"
        << L"}";
    out->append(f);

    f = FunctionInfo();
    f.name = L"main";
    f.address = L"00401130";
    f.segment = L".text";
    f.length = 0x0B;
    f.type = L"int()";
    f.prototype = L"int __cdecl main(int argc, const char **argv, const char **envp)";
    f.lines
        << mk("00401130", "6A 00", "push    0")
        << mk("00401132", "E8 C9 FE FF FF", "call    sub_401000", 2)
        << mk("00401137", "50", "push    eax")
        << mk("00401138", "FF 15 08 20 40 00", "call    ds:ExitProcess", 2)
        << mk("0040113E", "CC", "align   10h");
    f.pseudo
        << L"int __cdecl main(int argc, const char **argv, const char **envp)"
        << L"{"
        << L"  ExitProcess(sub_401000(argc, argv, envp));"
        << L"}";
    out->append(f);

    f = FunctionInfo();
    f.name = L"WinMain";
    f.address = L"00401150";
    f.segment = L".text";
    f.length = 0x36;
    f.type = L"int()";
    f.prototype = L"int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nShow)";
    f.lines
        << mk("00401150", "55", "push    ebp")
        << mk("00401151", "8B EC", "mov     ebp, esp")
        << mk("00401153", "6A 00", "push    0")
        << mk("00401155", "68 44 40 40 00", "push    offset aWrong ; \"Wrong!\"")
        << mk("0040115A", "68 30 40 40 00", "push    offset aCorrect ; \"Correct!\"")
        << mk("0040115F", "6A 00", "push    0")
        << mk("00401161", "FF 15 0C 20 40 00", "call    ds:MessageBoxA", 2)
        << mk("00401167", "33 C0", "xor     eax, eax")
        << mk("00401169", "5D", "pop     ebp")
        << mk("0040116A", "C3", "retn    10h", 4);
    f.pseudo
        << L"int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nShow)"
        << L"{"
        << L"  MessageBoxA(0, \"Correct!\", \"Wrong!\", 0);"
        << L"  return 0;"
        << L"}";
    out->append(f);

    f = FunctionInfo();
    f.name = L"start";
    f.address = L"004011F0";
    f.segment = L".text";
    f.length = 0x1B;
    f.type = L"void()";
    f.prototype = L"void __noreturn start()";
    f.lines
        << mk("004011F0", "6A 00", "push    0")
        << mk("004011F2", "6A 00", "push    0")
        << mk("004011F4", "6A 00", "push    0")
        << mk("004011F6", "E8 55 FF FF FF", "call    WinMain", 2)
        << mk("004011FB", "50", "push    eax")
        << mk("004011FC", "FF 15 08 20 40 00", "call    ds:ExitProcess", 2)
        << mk("00401202", "CC", "align   10h");
    f.pseudo
        << L"void __noreturn start()"
        << L"{"
        << L"  ExitProcess(WinMain(0, 0, 0));"
        << L"}";
    out->append(f);

    f = FunctionInfo();
    f.name = L"_printf";
    f.address = L"00402000";
    f.segment = L".text";
    f.length = 0x2A;
    f.type = L"int()";
    f.prototype = L"int __cdecl printf(const char *Format, ...)";
    f.lines
        << mk("00402000", "55", "push    ebp")
        << mk("00402001", "8B EC", "mov     ebp, esp")
        << mk("00402003", "8B 45 08", "mov     eax, [ebp+Format]")
        << mk("00402006", "50", "push    eax")
        << mk("00402007", "E8 F4 F0 FF FF", "call    __vfprintf", 2)
        << mk("0040200C", "83 C4 04", "add     esp, 4")
        << mk("0040200F", "5D", "pop     ebp")
        << mk("00402010", "C3", "retn", 4);
    f.pseudo
        << L"int __cdecl printf(const char *Format, ...)"
        << L"{"
        << L"  return __vfprintf(Format);"
        << L"}";
    out->append(f);
}

// ===========================================================================
// NavigationBand : the coloured segment strip above the listing
// ===========================================================================
class NavigationBand : public QWidget {
public:
    NavigationBand(QWidget* parent = 0) : QWidget(parent), position_(0.42) {
        setFixedHeight(14);
        setMinimumSize(80, 14);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setPosition(double p) {
        position_ = p;
        update();
    }
    double position() const { return position_; }

    Signal<void(double)> clickedAt;

protected:
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        IdaColors c = idaColors();
        p.fillRect(rect(), c.gutter);
        struct Seg { double from, to; QColor color; };
        Seg segs[] = {
            { 0.00, 0.55, QColor(0x2E, 0x55, 0x97) },  // .text
            { 0.55, 0.72, QColor(0x2E, 0x7D, 0x32) },  // .rdata
            { 0.72, 0.86, QColor(0x8A, 0x6D, 0x1F) },  // .data
            { 0.86, 1.00, QColor(0x6A, 0x30, 0x30) }   // .idata
        };
        for (int i = 0; i < 4; ++i) {
            int x1 = (int)(segs[i].from * width());
            int x2 = (int)(segs[i].to * width());
            p.fillRect(QRect(x1, 0, qMax(1, x2 - x1), height()), segs[i].color);
        }
        int x = (int)(position_ * width());
        p.setPen(QPen(QColor(0xFF, 0xFF, 0xFF), 1));
        p.drawLine(x, 0, x, height());
        p.setPen(QPen(c.border, 1));
        p.drawRect(QRect(0, 0, width() - 1, height() - 1));
    }

    void mousePressEvent(QMouseEvent* event) {
        double p = width() > 0 ? (double)event->pos().x() / (double)width() : 0.0;
        if (p < 0.0) p = 0.0;
        if (p > 1.0) p = 1.0;
        position_ = p;
        update();
        clickedAt(p);
    }

private:
    double position_;
};

// ===========================================================================
// ListingView : the disassembly listing
// ===========================================================================
class ListingView : public QWidget {
public:
    ListingView(QWidget* parent = 0)
        : QWidget(parent), scroll_(0), current_(0), hover_(-1), lines_(0), vbar_(0) {
        setFocusPolicy(Qt::StrongFocus);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        vbar_ = new QScrollBar(Qt::Vertical, this);
        vbar_->hide();
        vbar_->valueChanged.connect(vbar_, this, &ListingView::onScroll);
    }

    void setLines(const QList<DisasmLine>* lines, const QString& title) {
        lines_ = lines;
        title_ = title;
        scroll_ = 0;
        current_ = 0;
        layoutScroll();
        update();
    }

    void setCurrent(int index) {
        if (!lines_) return;
        if (index < 0) index = 0;
        if (index >= lines_->count()) index = lines_->count() - 1;
        current_ = index;
        ensureVisible();
        update();
    }

    int current() const { return current_; }
    const QList<DisasmLine>* lines() const { return lines_; }

    Signal<void(int)> lineSelected;

protected:
    int rowHeight() const { return QFontMetrics(monoFont()).height() + 4; }
    int addrWidth() const { return QFontMetrics(monoFont()).width(QString(L"00401000")) + 16; }
    int bytesWidth() const { return QFontMetrics(monoFont()).width(QString(L"FF 15 0C 20 40 00")) + 12; }

    void layoutScroll() {
        int rows = lines_ ? lines_->count() : 0;
        int content = rows * rowHeight();
        int view = height();
        if (content > view) {
            vbar_->setGeometry(width() - 13, 0, 13, height());
            vbar_->setRange(0, qMax(0, content - view));
            vbar_->setPageStep(view);
            vbar_->show();
        } else {
            vbar_->hide();
            scroll_ = 0;
            vbar_->setValue(0);
        }
    }

    void ensureVisible() {
        int view = height();
        int top = current_ * rowHeight();
        if (top < scroll_) {
            scroll_ = top;
            vbar_->setValue(scroll_);
        } else if (top + rowHeight() > scroll_ + view) {
            scroll_ = top + rowHeight() - view;
            vbar_->setValue(scroll_);
        }
    }

    void onScroll(int value) {
        scroll_ = value;
        update();
    }

    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        IdaColors c = idaColors();
        p.fillRect(rect(), c.window);
        p.setFont(monoFont());
        QFontMetrics fm = p.fontMetrics();
        int rh = rowHeight();
        int aw = addrWidth();
        int bw = bytesWidth();
        int rows = lines_ ? lines_->count() : 0;
        int first = qMax(0, scroll_ / rh);
        int last = qMin(rows, first + height() / rh + 2);
        p.fillRect(QRect(0, 0, aw + bw - 8, height()), c.gutter);
        p.setPen(QPen(c.border, 1));
        p.drawLine(aw + bw - 8, 0, aw + bw - 8, height());

        for (int i = first; i < last; ++i) {
            int y = i * rh - scroll_;
            const DisasmLine& line = (*lines_)[i];
            if (i == current_) {
                p.fillRect(QRect(0, y, width(), rh), c.currentLine);
                p.fillRect(QRect(0, y, 3, rh), c.accent);
            } else if (i == hover_) {
                p.fillRect(QRect(0, y, width(), rh), c.selection);
            }
            p.setPen(QPen(c.muted));
            p.drawText(QRect(6, y, aw - 8, rh), Qt::AlignLeft | Qt::AlignVCenter, line.address);
            p.drawText(QRect(aw, y, bw - 8, rh), Qt::AlignLeft | Qt::AlignVCenter, line.bytes);
            drawInstruction(p, aw + bw, y, rh, line, c, fm);
        }
        p.setPen(QPen(c.border, 1));
        p.drawLine(0, height() - 1, width(), height() - 1);
    }

    void drawInstruction(QPainter& p, int x, int y, int rh, const DisasmLine& line,
                         const IdaColors& c, const QFontMetrics& fm) {
        QString text = line.text;
        int comment = text.indexOf(L';');
        QString code = comment >= 0 ? text.left(comment) : text;
        QString tail = comment >= 0 ? text.mid(comment) : QString();

        QList<QString> tokens = code.split(L' ');
        int cx = x + 4;
        for (int t = 0; t < tokens.count(); ++t) {
            QString token = tokens[t];
            if (token.isEmpty()) {
                cx += fm.width(QString(L" "));
                continue;
            }
            QColor color = c.text;
            if (t == 0) {
                if (token == L"call" || token == L"jmp" || token == L"jz" || token == L"jnz")
                    color = c.control;
                else if (token == L"retn" || token == L"ret")
                    color = c.control;
                else if (token == L"push" || token == L"pop")
                    color = c.keyword;
                else
                    color = c.keyword;
            } else if (token.startsWith(L"loc_") || token.startsWith(L"sub_") ||
                       token.startsWith(L"aCorrect") || token.startsWith(L"aWrong")) {
                color = c.label;
            } else if (token.startsWith(L"\"") || token.startsWith(L"offset")) {
                color = c.string;
            } else if (token.startsWith(L"0x") || (token.count() > 0 && token[0] >= L'0' && token[0] <= L'9')) {
                color = c.number;
            } else if (token == L"eax" || token == L"ebx" || token == L"ecx" || token == L"edx" ||
                       token == L"esi" || token == L"edi" || token == L"esp" || token == L"ebp" ||
                       token == L"al" || token == L"ah" || token == L"ax") {
                color = c.reg;
            } else if (token.startsWith(L"ds:")) {
                color = c.text;
            }
            p.setPen(QPen(color));
            p.drawText(QPoint(cx, y + rh - fm.descent() - 3), token);
            cx += fm.width(token);
            if (t + 1 < tokens.count()) cx += fm.width(QString(L" "));
        }
        if (!tail.isEmpty()) {
            p.setPen(QPen(c.comment));
            p.drawText(QPoint(cx + fm.width(QString(L" ")), y + rh - fm.descent() - 3), tail);
        }
    }

    void mousePressEvent(QMouseEvent* event) {
        if (!lines_) return;
        int index = (event->pos().y() + scroll_) / rowHeight();
        if (index >= 0 && index < lines_->count()) {
            current_ = index;
            update();
            lineSelected(index);
        }
        setFocus(Qt::MouseFocusReason);
    }

    void mouseMoveEvent(QMouseEvent* event) {
        if (!lines_) return;
        int index = (event->pos().y() + scroll_) / rowHeight();
        if (index != hover_) {
            hover_ = index;
            update();
        }
    }

    void leaveEvent(QEvent*) {
        hover_ = -1;
        update();
    }

    void wheelEvent(QWheelEvent* event) {
        int delta = event->delta() > 0 ? -3 * rowHeight() : 3 * rowHeight();
        int value = qBound(0, vbar_->value() + delta, vbar_->maximum());
        vbar_->setValue(value);
    }

    void keyPressEvent(QKeyEvent* event) {
        if (!lines_) return;
        int next = current_;
        if (event->key() == Qt::Key_Up) --next;
        else if (event->key() == Qt::Key_Down) ++next;
        else if (event->key() == Qt::Key_PageUp) next -= height() / rowHeight();
        else if (event->key() == Qt::Key_PageDown) next += height() / rowHeight();
        else if (event->key() == Qt::Key_Home) next = 0;
        else if (event->key() == Qt::Key_End) next = lines_->count() - 1;
        else {
            QWidget::keyPressEvent(event);
            return;
        }
        setCurrent(next);
        lineSelected(current_);
    }

    void resizeEvent(QResizeEvent* event) {
        layoutScroll();
        QWidget::resizeEvent(event);
    }

private:
    const QList<DisasmLine>* lines_;
    QString title_;
    int scroll_;
    int current_;
    int hover_;
    QScrollBar* vbar_;
};

// ===========================================================================
// PseudoView : Hex-Rays style pseudocode
// ===========================================================================
class PseudoView : public QWidget {
public:
    PseudoView(QWidget* parent = 0)
        : QWidget(parent), scroll_(0), current_(0), vbar_(0) {
        setFocusPolicy(Qt::StrongFocus);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        vbar_ = new QScrollBar(Qt::Vertical, this);
        vbar_->hide();
        vbar_->valueChanged.connect(vbar_, this, &PseudoView::onScroll);
    }

    void setLines(const QStringList& lines) {
        lines_ = lines;
        scroll_ = 0;
        current_ = 0;
        layoutScroll();
        update();
    }

    void setCurrent(int index) {
        if (lines_.isEmpty()) return;
        current_ = qBound(0, index, lines_.count() - 1);
        ensureVisible();
        update();
    }

    Signal<void(int)> lineSelected;

protected:
    int rowHeight() const { return QFontMetrics(monoFont()).height() + 3; }

    void layoutScroll() {
        int content = lines_.count() * rowHeight();
        if (content > height()) {
            vbar_->setGeometry(width() - 13, 0, 13, height());
            vbar_->setRange(0, qMax(0, content - height()));
            vbar_->setPageStep(height());
            vbar_->show();
        } else {
            vbar_->hide();
            vbar_->setValue(0);
            scroll_ = 0;
        }
    }

    void ensureVisible() {
        int top = current_ * rowHeight();
        if (top < scroll_) {
            scroll_ = top;
            vbar_->setValue(scroll_);
        } else if (top + rowHeight() > scroll_ + height()) {
            scroll_ = top + rowHeight() - height();
            vbar_->setValue(scroll_);
        }
    }

    void onScroll(int value) {
        scroll_ = value;
        update();
    }

    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        IdaColors c = idaColors();
        p.fillRect(rect(), c.window);
        p.setFont(monoFont());
        QFontMetrics fm = p.fontMetrics();
        int rh = rowHeight();
        int gutter = fm.width(QString(L"999")) + 12;
        int first = qMax(0, scroll_ / rh);
        int last = qMin(lines_.count(), first + height() / rh + 2);
        p.fillRect(QRect(0, 0, gutter, height()), c.gutter);
        p.setPen(QPen(c.border, 1));
        p.drawLine(gutter, 0, gutter, height());
        for (int i = first; i < last; ++i) {
            int y = i * rh - scroll_;
            if (i == current_) p.fillRect(QRect(0, y, width(), rh), c.currentLine);
            p.setPen(QPen(c.muted));
            p.drawText(QRect(4, y, gutter - 6, rh), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(i + 1));
            drawLine(p, gutter + 6, y, rh, lines_[i], c, fm);
        }
    }

    void drawLine(QPainter& p, int x, int y, int rh, const QString& line,
                  const IdaColors& c, const QFontMetrics& fm) {
        QString text = line;
        int comment = text.indexOf(L"//");
        QString code = comment >= 0 ? text.left(comment) : text;
        QString tail = comment >= 0 ? text.mid(comment) : QString();
        QList<QString> tokens = code.split(L' ');
        int cx = x;
        const wchar_t* keywords[] = { L"int", L"char", L"void", L"return", L"if", L"else", L"for",
                                     L"while", L"const", L"unsigned", L"signed", L"sizeof" };
        for (int t = 0; t < tokens.count(); ++t) {
            QString token = tokens[t];
            if (token.isEmpty()) {
                cx += fm.width(QString(L" "));
                continue;
            }
            QColor color = c.text;
            for (int k = 0; k < 11; ++k) {
                if (token == keywords[k]) { color = c.keyword; break; }
            }
            if (token.startsWith(L"\"") || token.startsWith(L"'")) color = c.string;
            else if (token.startsWith(L"0x") || (token[0] >= L'0' && token[0] <= L'9')) color = c.number;
            else if (token.startsWith(L"sub_") || token.startsWith(L"loc_")) color = c.label;
            p.setPen(QPen(color));
            p.drawText(QPoint(cx, y + rh - fm.descent() - 3), token);
            cx += fm.width(token);
            if (t + 1 < tokens.count()) cx += fm.width(QString(L" "));
        }
        if (!tail.isEmpty()) {
            p.setPen(QPen(c.comment));
            p.drawText(QPoint(cx + 6, y + rh - fm.descent() - 3), tail);
        }
    }

    void mousePressEvent(QMouseEvent* event) {
        int index = (event->pos().y() + scroll_) / rowHeight();
        if (index >= 0 && index < lines_.count()) {
            current_ = index;
            update();
            lineSelected(index);
        }
        setFocus(Qt::MouseFocusReason);
    }

    void wheelEvent(QWheelEvent* event) {
        int delta = event->delta() > 0 ? -3 * rowHeight() : 3 * rowHeight();
        vbar_->setValue(qBound(0, vbar_->value() + delta, vbar_->maximum()));
    }

    void resizeEvent(QResizeEvent* event) {
        layoutScroll();
        QWidget::resizeEvent(event);
    }

private:
    QStringList lines_;
    int scroll_;
    int current_;
    QScrollBar* vbar_;
};

// ===========================================================================
// GraphView : a small control-flow graph
// ===========================================================================
class GraphView : public QWidget {
public:
    GraphView(QWidget* parent = 0) : QWidget(parent), currentBlock_(0) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMinimumSize(200, 160);
    }

    void setFunction(const QString& name) {
        function_ = name;
        currentBlock_ = 0;
        update();
    }

    void setCurrentBlock(int block) {
        currentBlock_ = block;
        update();
    }

    Signal<void(int)> blockSelected;

protected:
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        IdaColors c = idaColors();
        p.fillRect(rect(), c.window);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setFont(monoFont(12));
        QFontMetrics fm = p.fontMetrics();

        struct Block { int x, y, w, h; QStringList text; };
        QList<Block> blocks;
        int bw = 250, bh = 64, cx = width() / 2 - bw / 2;
        Block b0; b0.x = cx; b0.y = 24; b0.w = bw; b0.h = bh;
        b0.text << L"loc_401000" << L"push ebp / mov ebp, esp";
        Block b1; b1.x = 40; b1.y = 150; b1.w = bw; b1.h = bh;
        b1.text << L"loc_40100C" << L"call GetCommandLineA";
        Block b2; b2.x = width() - bw - 40; b2.y = 150; b2.w = bw; b2.h = bh;
        b2.text << L"loc_40102D" << L"jz short loc_40104D";
        Block b3; b3.x = 40; b3.y = 276; b3.w = bw; b3.h = bh;
        b3.text << L"loc_40102F" << L"printf(\"Correct!\")";
        Block b4; b4.x = width() - bw - 40; b4.y = 276; b4.w = bw; b4.h = bh;
        b4.text << L"loc_40104D" << L"printf(\"Wrong!\")";
        Block b5; b5.x = cx; b5.y = 400; b5.w = bw; b5.h = bh;
        b5.text << L"loc_401047" << L"retn";
        blocks << b0 << b1 << b2 << b3 << b4 << b5;

        for (int i = 0; i < blocks.count(); ++i) {
            QRect r(blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h);
            p.setPen(QPen(i == currentBlock_ ? c.accent : c.border, i == currentBlock_ ? 2 : 1));
            p.setBrush(QBrush(c.panel));
            p.drawRoundedRect(r, 6, 6);
            p.setPen(QPen(c.muted));
            p.drawText(QRect(r.left() + 8, r.top() + 4, r.width() - 16, fm.height()),
                       Qt::AlignLeft | Qt::AlignVCenter, blocks[i].text[0]);
            p.setPen(QPen(c.text));
            p.drawText(QRect(r.left() + 8, r.top() + 4 + fm.height(), r.width() - 16,
                             r.height() - 8 - fm.height()),
                       Qt::AlignLeft | Qt::AlignTop, blocks[i].text[1]);
        }

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(c.muted, 1));
        drawArrow(p, QPoint(cx + bw / 2, 24 + bh), QPoint(40 + bw / 2, 150), c);
        drawArrow(p, QPoint(cx + bw / 2, 24 + bh), QPoint(width() - bw - 40 + bw / 2, 150), c);
        drawArrow(p, QPoint(40 + bw / 2, 150 + bh), QPoint(40 + bw / 2, 276), c);
        drawArrow(p, QPoint(width() - bw - 40 + bw / 2, 150 + bh), QPoint(width() - bw - 40 + bw / 2, 276), c);
        drawArrow(p, QPoint(40 + bw / 2, 276 + bh), QPoint(cx + bw / 2, 400), c);
        drawArrow(p, QPoint(width() - bw - 40 + bw / 2, 276 + bh), QPoint(cx + bw / 2, 400), c);

        p.setPen(QPen(c.border, 1));
        p.drawRect(QRect(0, 0, width() - 1, height() - 1));
    }

    void drawArrow(QPainter& p, const QPoint& from, const QPoint& to, const IdaColors& c) {
        p.drawLine(from, to);
        QList<QPoint> head;
        head << QPoint(to.x(), to.y()) << QPoint(to.x() - 4, to.y() - 8) << QPoint(to.x() + 4, to.y() - 8);
        p.setBrush(QBrush(c.muted));
        p.drawPolygon(head);
        p.setBrush(Qt::NoBrush);
    }

    void mousePressEvent(QMouseEvent* event) {
        int bh = 64, bw = 250;
        int cx = width() / 2 - bw / 2;
        struct RectDef { int x, y; };
        RectDef defs[6] = {
            { cx, 24 }, { 40, 150 }, { width() - bw - 40, 150 },
            { 40, 276 }, { width() - bw - 40, 276 }, { cx, 400 }
        };
        for (int i = 0; i < 6; ++i) {
            if (QRect(defs[i].x, defs[i].y, bw, bh).contains(event->pos())) {
                currentBlock_ = i;
                update();
                blockSelected(i);
                return;
            }
        }
    }

private:
    QString function_;
    int currentBlock_;
};

// ===========================================================================
// helper widgets
// ===========================================================================
class Relay : public QObject {
public:
    Relay() {}
    std::function<void()> fn;
    void fire() {
        if (fn) fn();
    }
};

static QPushButton* ribbonButton(const QString& text, const QString& command, QWidget* parent,
                                 void (*dispatch)(void*, const QString&), void* window) {
    QPushButton* button = new QPushButton(text, parent);
    button->setFlat(true);
    button->setMinimumSize(button->minimumSize().width(), 24);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    Relay* relay = new Relay();
    relay->setParent(parent);
    relay->fn = [dispatch, window, command]() { dispatch(window, command); };
    QObject::connect(button, button->clicked, relay, &Relay::fire);
    return button;
}

struct RibbonGroup {
    QString caption;
    QStringList commands;
};

static QWidget* makeRibbonGroup(const RibbonGroup& group, QWidget* parent,
                                void (*dispatch)(void*, const QString&), void* window) {
    QWidget* box = new QWidget(parent);
    QVBoxLayout* v = new QVBoxLayout(box);
    v->setContentsMargins(6, 4, 6, 2);
    v->setSpacing(2);
    QHBoxLayout* row = new QHBoxLayout();
    row->setSpacing(2);
    for (int i = 0; i < group.commands.count(); ++i) {
        QString label = group.commands[i];
        QString command = label;
        int sep = label.indexOf(L'|');
        if (sep >= 0) {
            command = label.mid(sep + 1);
            label = label.left(sep);
        }
        row->addWidget(ribbonButton(label, command, box, dispatch, window));
    }
    row->addStretch(0);
    v->addLayout(row);
    QLabel* caption = new QLabel(group.caption, box);
    QFont f = caption->font();
    f.setPixelSize(10);
    caption->setFont(f);
    caption->setAlignment(Qt::AlignHCenter);
    v->addWidget(caption);
    box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    return box;
}

// ===========================================================================
// IdaWindow
// ===========================================================================
class IdaWindow : public QMainWindow {
public:
    IdaWindow();
    ~IdaWindow();

    void runCommand(const QString& command);

    void selectFunction(int index);
    void selectIndexByName(const QString& name);

    int selfTest();
    bool screenshot(const QString& path);

private:
    void buildUi();
    void buildRibbon();
    void buildPanels();
    void buildStatus();
    void buildData();
    void rebuildFunctionList(const QString& filter);
    void updateViews();
    void setStatusLine(const QString& message);
    void onTreeClicked(QTreeWidgetItem* item, int column);
    void onFilterChanged(const QString& text);
    void onListingLine(int index);
    void onPseudoLine(int index);
    void onNavClicked(double position);
    void onGraphBlock(int block);

    QList<FunctionInfo> functions_;
    QList<DisasmLine> emptyLines_;

    QTabWidget* ribbon_;
    QTreeWidget* functionTree_;
    QLineEdit* filterEdit_;
    QTabWidget* leftTabs_;
    QTabWidget* centerTabs_;
    QTabWidget* rightTabs_;
    QTabWidget* bottomTabs_;
    ListingView* listing_;
    NavigationBand* navBand_;
    PseudoView* pseudo_;
    GraphView* graph_;
    QTextEdit* hexView_;
    QTreeWidget* xrefs_;
    QTreeWidget* calls_;
    QTextEdit* output_;
    QLabel* statusLeft_;
    QLabel* statusRight_;
    QStatusBar* statusBar_;
    QLabel* pathLabel_;
    int currentFunction_;
};

IdaWindow::IdaWindow() : QMainWindow(0, Qt::Window), ribbon_(0), functionTree_(0), filterEdit_(0),
                         leftTabs_(0), centerTabs_(0), rightTabs_(0), bottomTabs_(0), listing_(0),
                         navBand_(0), pseudo_(0), graph_(0), hexView_(0), xrefs_(0), calls_(0),
                         output_(0), statusLeft_(0), statusRight_(0), statusBar_(0),
                         pathLabel_(0), currentFunction_(0) {
    setWindowTitle(L"IDA Pro 9 - crackme.exe  [QtClone demo  " + QApplication::buildInfo() + L"]");
    resize(1440, 900);
    buildData();
    buildUi();
    rebuildFunctionList(QString());
    selectFunction(0);
}

IdaWindow::~IdaWindow() {}

static void idaDispatch(void* window, const QString& command) {
    static_cast<IdaWindow*>(window)->runCommand(command);
}

void IdaWindow::buildData() {
    fillFunctions(&functions_);
}

void IdaWindow::buildUi() {
    QWidget* root = new QWidget();
    QVBoxLayout* rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    buildRibbon();
    rootLayout->addWidget(ribbon_);

    // path / breadcrumb bar
    QWidget* pathBar = new QWidget();
    pathBar->setFixedHeight(28);
    QHBoxLayout* pathLayout = new QHBoxLayout(pathBar);
    pathLayout->setContentsMargins(8, 2, 8, 2);
    pathLayout->setSpacing(6);
    QLabel* icon = new QLabel(L"IDA");
    QFont iconFont = icon->font();
    iconFont.setBold(true);
    icon->setFont(iconFont);
    pathLayout->addWidget(icon);
    pathLabel_ = new QLabel(L"C:\\work\\crackme.exe   >   .text:00401000   >   sub_401000");
    pathLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    pathLayout->addWidget(pathLabel_);
    pathLayout->addWidget(new QLabel(L"Text"));
    pathLayout->addWidget(new QLabel(L"Graph"));
    rootLayout->addWidget(pathBar);
    buildPanels();
    // main area: left | center | right, with the output window below
    QSplitter* horizontal = new QSplitter(Qt::Horizontal);
    horizontal->addWidget(leftTabs_);
    horizontal->addWidget(centerTabs_);
    horizontal->addWidget(rightTabs_);
    horizontal->setSizes(QList<int>() << 360 << 750 << 330);

    QSplitter* vertical = new QSplitter(Qt::Vertical);
    vertical->addWidget(horizontal);
    vertical->addWidget(bottomTabs_);
    vertical->setSizes(QList<int>() << 600 << 170);
    rootLayout->addWidget(vertical, 1);
    setCentralWidget(root);
    buildStatus();
}

void IdaWindow::buildRibbon() {
    ribbon_ = new QTabWidget();
    ribbon_->setDocumentMode(true);
    ribbon_->setFixedHeight(104);

    struct PageDef {
        const wchar_t* name;
        RibbonGroup groups[4];
        int count;
    };

    PageDef pages[7];
    pages[0].name = L"FILE"; pages[0].count = 2;
    pages[0].groups[0].caption = L"Database";
    pages[0].groups[0].commands = QStringList() << L"New" << L"Open..." << L"Close" << L"Save";
    pages[0].groups[1].caption = L"Session";
    pages[0].groups[1].commands = QStringList() << L"Snapshot" << L"Exit";

    pages[1].name = L"EDIT"; pages[1].count = 2;
    pages[1].groups[0].caption = L"Clipboard";
    pages[1].groups[0].commands = QStringList() << L"Undo" << L"Redo" << L"Cut" << L"Copy" << L"Paste";
    pages[1].groups[1].caption = L"Modify";
    pages[1].groups[1].commands = QStringList() << L"Patch program" << L"Assemble" << L"Segments";

    pages[2].name = L"VIEW"; pages[2].count = 3;
    pages[2].groups[0].caption = L"Open subviews";
    pages[2].groups[0].commands = QStringList() << L"Functions" << L"Imports" << L"Exports" << L"Names" << L"Strings";
    pages[2].groups[1].caption = L"Display";
    pages[2].groups[1].commands = QStringList() << L"Text" << L"Graph" << L"Hex" << L"Decompiler";
    pages[2].groups[2].caption = L"Appearance";
    pages[2].groups[2].commands = QStringList() << L"Dark|theme_dark" << L"Light|theme_light" << L"System|theme_system";

    pages[3].name = L"SEARCH"; pages[3].count = 2;
    pages[3].groups[0].caption = L"Search";
    pages[3].groups[0].commands = QStringList() << L"Next code" << L"Text..." << L"Bytes..." << L"Immediate...";
    pages[3].groups[1].caption = L"Navigation";
    pages[3].groups[1].commands = QStringList() << L"Jump to address" << L"Previous" << L"Next";

    pages[4].name = L"OPTIONS"; pages[4].count = 1;
    pages[4].groups[0].caption = L"Configuration";
    pages[4].groups[0].commands = QStringList() << L"Compiler..." << L"General..." << L"Colors..." << L"Font...";

    pages[5].name = L"WINDOWS"; pages[5].count = 2;
    pages[5].groups[0].caption = L"Windows";
    pages[5].groups[0].commands = QStringList() << L"Output" << L"Functions" << L"Pseudocode" << L"Xrefs";
    pages[5].groups[1].caption = L"Layout";
    pages[5].groups[1].commands = QStringList() << L"Reset desktop" << L"Tile" << L"Cascade";

    pages[6].name = L"HELP"; pages[6].count = 1;
    pages[6].groups[0].caption = L"Support";
    pages[6].groups[0].commands = QStringList() << L"Help" << L"About";

    for (int p = 0; p < 7; ++p) {
        QWidget* page = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(page);
        layout->setContentsMargins(4, 2, 4, 2);
        layout->setSpacing(0);
        for (int g = 0; g < pages[p].count; ++g)
            layout->addWidget(makeRibbonGroup(pages[p].groups[g], page, &idaDispatch, this));
        layout->addStretch(1);
        ribbon_->addTab(page, QString(pages[p].name));
    }
    ribbon_->setCurrentIndex(2);
}

void IdaWindow::buildPanels() {
    // ---- left: functions / imports / exports / names / strings ----
    leftTabs_ = new QTabWidget();
    QWidget* functionsPage = new QWidget();
    QVBoxLayout* funcLayout = new QVBoxLayout(functionsPage);
    funcLayout->setContentsMargins(0, 0, 0, 0);
    funcLayout->setSpacing(2);
    filterEdit_ = new QLineEdit();
    filterEdit_->setPlaceholderText(L"Filter functions...");
    filterEdit_->setClearButtonEnabled(true);
    QObject::connect(filterEdit_, filterEdit_->textChanged, this, &IdaWindow::onFilterChanged);
    funcLayout->addWidget(filterEdit_);
    functionTree_ = new QTreeWidget();
    functionTree_->setHeaderLabels(QStringList() << L"Name" << L"Address" << L"Segment" << L"Type");
    functionTree_->setAlternatingRowColors(true);
    QObject::connect(functionTree_, functionTree_->itemClicked, this, &IdaWindow::onTreeClicked);
    funcLayout->addWidget(functionTree_, 1);
    leftTabs_->addTab(functionsPage, L"Functions");

    QTreeWidget* imports = new QTreeWidget();
    imports->setHeaderLabels(QStringList() << L"Name" << L"Address" << L"Library");
    QStringList libs;
    libs << L"KERNEL32.dll" << L"USER32.dll" << L"MSVCRT.dll";
    QStringList names;
    names << L"GetCommandLineA" << L"ExitProcess" << L"CreateFileW" << L"ReadFile" << L"LoadLibraryA"
          << L"GetProcAddress" << L"MessageBoxA" << L"printf" << L"__vfprintf";
    for (int i = 0; i < names.count(); ++i) {
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << names[i]
            << hex8(0x402010 + i * 4)
            << (i < 6 ? libs[0] : libs[2]));
        imports->addTopLevelItem(item);
    }
    leftTabs_->addTab(imports, L"Imports");

    QTreeWidget* namesTree = new QTreeWidget();
    namesTree->setHeaderLabels(QStringList() << L"Name" << L"Address" << L"Segment");
    const wchar_t* symbols[] = { L"start", L"WinMain", L"sub_401000", L"sub_401100", L"main",
                                 L"_printf", L"aCorrect", L"aWrong", L"aName", L"aFormat" };
    for (int i = 0; i < 10; ++i) {
        namesTree->addTopLevelItem(new QTreeWidgetItem(QStringList() << QString(symbols[i])
            << hex8(0x401000 + i * 0x20) << L".text"));
    }
    leftTabs_->addTab(namesTree, L"Names");

    QTreeWidget* stringsTree = new QTreeWidget();
    stringsTree->setHeaderLabels(QStringList() << L"Address" << L"Length" << L"String");
    const wchar_t* texts[] = { L"Correct!", L"Wrong!", L"Enter your name:", L"license.dat",
                               L"crackme.exe", L"%s", L"SOFTWARE\\Demo" };
    for (int i = 0; i < 7; ++i) {
        stringsTree->addTopLevelItem(new QTreeWidgetItem(QStringList() << hex8(0x403000 + i * 16)
            << QString::number(QString(texts[i]).size()) << QString(texts[i])));
    }
    leftTabs_->addTab(stringsTree, L"Strings");

    // ---- center: listing / graph / pseudocode / hex ----
    centerTabs_ = new QTabWidget();
    centerTabs_->setDocumentMode(true);
    QWidget* listingPage = new QWidget();
    QVBoxLayout* listingLayout = new QVBoxLayout(listingPage);
    listingLayout->setContentsMargins(0, 0, 0, 0);
    listingLayout->setSpacing(0);
    navBand_ = new NavigationBand();
    navBand_->clickedAt.connect(navBand_, this, &IdaWindow::onNavClicked);
    listingLayout->addWidget(navBand_);
    listing_ = new ListingView();
    listing_->lineSelected.connect(listing_, this, &IdaWindow::onListingLine);
    listingLayout->addWidget(listing_, 1);
    centerTabs_->addTab(listingPage, L"IDA View-A");

    graph_ = new GraphView();
    graph_->blockSelected.connect(graph_, this, &IdaWindow::onGraphBlock);
    centerTabs_->addTab(graph_, L"Graph view");

    pseudo_ = new PseudoView();
    pseudo_->lineSelected.connect(pseudo_, this, &IdaWindow::onPseudoLine);
    centerTabs_->addTab(pseudo_, L"Pseudocode-A");

    hexView_ = new QTextEdit();
    hexView_->setReadOnly(true);
    hexView_->setWordWrap(false);
    QFont hexFont = monoFont(13);
    hexView_->setFont(hexFont);
    centerTabs_->addTab(hexView_, L"Hex View-1");

    // ---- right: xrefs / calls ----
    rightTabs_ = new QTabWidget();
    xrefs_ = new QTreeWidget();
    xrefs_->setHeaderLabels(QStringList() << L"From" << L"Type");
    rightTabs_->addTab(xrefs_, L"Xrefs");
    calls_ = new QTreeWidget();
    calls_->setHeaderLabels(QStringList() << L"Function" << L"Address" << L"Type");
    rightTabs_->addTab(calls_, L"Function calls");

    // ---- bottom: output / errors / warnings / search / log ----
    bottomTabs_ = new QTabWidget();
    bottomTabs_->setDocumentMode(true);
    output_ = new QTextEdit();
    output_->setReadOnly(true);
    output_->setWordWrap(false);
    output_->setFont(monoFont(12));
    output_->setPlainText(L"-----------------------------------------\n"
                          L"The database is being loaded...\n"
                          L"Loading file 'C:\\work\\crackme.exe'...\n"
                          L"Loading segments...\n"
                          L"Hex-Rays Decompiler plugin has been loaded (9.0.240925)\n"
                          L"AU: idle\n");
    bottomTabs_->addTab(output_, L"Output");
    QListWidget* errors = new QListWidget();
    bottomTabs_->addTab(errors, L"Errors");
    QListWidget* warnings = new QListWidget();
    warnings->addItem(L"Warning: The function has no frame pointer.");
    bottomTabs_->addTab(warnings, L"Warnings");
    QTreeWidget* search = new QTreeWidget();
    search->setHeaderLabels(QStringList() << L"Address" << L"Text");
    search->addTopLevelItem(new QTreeWidgetItem(QStringList() << L"00403000" << L"\"Correct!\""));
    search->addTopLevelItem(new QTreeWidgetItem(QStringList() << L"00403010" << L"\"Wrong!\""));
    bottomTabs_->addTab(search, L"Search");
    QTextEdit* log = new QTextEdit();
    log->setReadOnly(true);
    log->setPlainText(L"9.0.240925 - AU: idle\n"
                      L"Loading segments...\n"
                      L"  .text  00401000 - 00402000\n"
                      L"  .rdata 00403000 - 00403200\n"
                      L"  .data  00404000 - 00404100\n"
                      L"  .idata 00405000 - 00405100\n");
    bottomTabs_->addTab(log, L"Log");
}

void IdaWindow::buildStatus() {
    statusBar_ = new QStatusBar();
    setStatusBar(statusBar_);
    statusBar_->showMessage(L"AU: idle");
}

void IdaWindow::rebuildFunctionList(const QString& filter) {
    functionTree_->clear();
    QString needle = filter.toLower();
    QTreeWidgetItem* textFolder = new QTreeWidgetItem(QStringList() << L".text" << QString() << L".text" << QString());
    QTreeWidgetItem* rdataFolder = new QTreeWidgetItem(QStringList() << L".rdata" << QString() << L".rdata" << QString());
    for (int i = 0; i < functions_.count(); ++i) {
        const FunctionInfo& f = functions_[i];
        if (!needle.isEmpty() && f.name.toLower().indexOf(needle) < 0) continue;
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << f.name << f.address << f.segment << f.type);
        item->setData(0, Qt::UserRole, QVariant(i));
        textFolder->addChild(item);
    }
    textFolder->setExpanded(true);
    functionTree_->addTopLevelItem(textFolder);
    functionTree_->addTopLevelItem(rdataFolder);
    if (functionTree_->topLevelItem(0) && functionTree_->topLevelItem(0)->childCount() > 0)
        functionTree_->setCurrentItem(functionTree_->topLevelItem(0)->child(0));
}

void IdaWindow::onTreeClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (!item) return;
    QVariant data = item->data(0, Qt::UserRole);
    if (!data.isValid()) return;
    selectFunction(data.toInt());
}

void IdaWindow::onFilterChanged(const QString& text) {
    rebuildFunctionList(text);
}

void IdaWindow::selectFunction(int index) {
    if (index < 0 || index >= functions_.count()) return;
    currentFunction_ = index;
    updateViews();
}

void IdaWindow::selectIndexByName(const QString& name) {
    for (int i = 0; i < functions_.count(); ++i) {
        if (functions_[i].name == name) {
            selectFunction(i);
            return;
        }
    }
}

void IdaWindow::updateViews() {
    const FunctionInfo& f = functions_[currentFunction_];
    listing_->setLines(&f.lines, QString(L"IDA View-A  -  ") + f.name);
    listing_->setCurrent(0);
    pseudo_->setLines(f.pseudo);
    pseudo_->setCurrent(0);
    graph_->setFunction(f.name);
    navBand_->setPosition(0.42);
    QString dump = L"Address        Hex dump                                         ASCII\n";
    dump += L"--------------------------------------------------------------------\n";
    for (int row = 0; row < 24; ++row) {
        int base = 0x401000 + row * 16;
        QString line = hex8(base) + L"  ";
        for (int i = 0; i < 16; ++i) {
            int v = (base + i) * 7 + i * 13 + currentFunction_ * 31;
            line += hex4(v & 0xFF).right(2) + L" ";
        }
        line += L" ";
        for (int i = 0; i < 16; ++i) {
            int v = ((base + i) * 7 + i * 13 + currentFunction_ * 31) & 0xFF;
            wchar_t ch = (v >= 32 && v < 127) ? (wchar_t)v : L'.';
            line += ch;
        }
        dump += line + L"\n";
    }
    hexView_->setPlainText(dump);
    hexView_->setCursorPosition(0);
    xrefs_->clear();
    xrefs_->setHeaderLabels(QStringList() << L"From" << L"Type");
    if (f.name == L"sub_401000") {
        xrefs_->addTopLevelItem(new QTreeWidgetItem(QStringList() << L"main+2" << L"Code"));
        xrefs_->addTopLevelItem(new QTreeWidgetItem(QStringList() << L"sub_401100" << L"Code"));
    } else if (f.name == L"sub_401100") {
        xrefs_->addTopLevelItem(new QTreeWidgetItem(QStringList() << L"sub_401000+23" << L"Code"));
    } else if (f.name == L"WinMain") {
        xrefs_->addTopLevelItem(new QTreeWidgetItem(QStringList() << L"start+6" << L"Code"));
    } else if (f.name == L"start") {
        xrefs_->addTopLevelItem(new QTreeWidgetItem(QStringList() << L"entry point" << L"Code"));
    } else if (f.name == L"main") {
        xrefs_->addTopLevelItem(new QTreeWidgetItem(QStringList() << L"start+2" << L"Code"));
    } else {
        xrefs_->addTopLevelItem(new QTreeWidgetItem(QStringList() << L"start" << L"Code"));
    }

    calls_->clear();
    calls_->setHeaderLabels(QStringList() << L"Function" << L"Address" << L"Type");
    for (int i = 0; i < f.lines.count(); ++i) {
        if (f.lines[i].kind != 2) continue;
        QString text = f.lines[i].text;
        int at = text.indexOf(L"call");
        QString callee = at >= 0 ? text.mid(at + 4).trimmed() : text;
        calls_->addTopLevelItem(new QTreeWidgetItem(QStringList() << callee << f.lines[i].address << L"Code"));
    }
    pathLabel_->setText(QString(L"C:\\work\\crackme.exe   >   ") + f.segment + L":" + f.address +
                        L"   >   " + f.name);
    setStatusLine(QString(L"AU: idle   |   ") + f.name + L"   |   " + f.address + L"  " + f.segment +
                  L"   |   " + f.prototype);
}

void IdaWindow::setStatusLine(const QString& message) {
    // IDA keeps "AU: idle" in the status bar and writes hints to the output.
    if (statusBar_) statusBar_->showMessage(message);
    if (output_) output_->append(message);
}

void IdaWindow::onListingLine(int index) {
    Q_UNUSED(index);
    if (!listing_->lines()) return;
    const DisasmLine& line = (*listing_->lines())[index];
    setStatusLine(QString(L"AU: idle   |   ") + line.address + L"  .text   |   " + line.text);
    if (line.kind == 2) {
        QString text = line.text;
        int at = text.indexOf(L"call");
        if (at >= 0) {
            QString callee = text.mid(at + 4).trimmed();
            if (callee.indexOf(L"sub_") == 0 || callee == L"WinMain" || callee == L"main") {
                selectIndexByName(callee);
            }
        }
    }
}

void IdaWindow::onPseudoLine(int index) {
    Q_UNUSED(index);
    setStatusLine(L"AU: idle   |   pseudocode line " + QString::number(index + 1));
}

void IdaWindow::onNavClicked(double position) {
    setStatusLine(L"AU: idle   |   navigate to " + QString::number((int)(position * 100)) + L"%");
}

void IdaWindow::onGraphBlock(int block) {
    listing_->setCurrent(qMin(block * 3, listing_->lines() ? listing_->lines()->count() - 1 : 0));
    setStatusLine(L"AU: idle   |   graph block " + QString::number(block));
}

void IdaWindow::runCommand(const QString& command) {
    if (command == L"theme_dark") { QTheme::setMode(QTheme::Dark); setStatusLine(L"AU: idle   |   theme: dark"); return; }
    if (command == L"theme_light") { QTheme::setMode(QTheme::Light); setStatusLine(L"AU: idle   |   theme: light"); return; }
    if (command == L"theme_system") { QTheme::setMode(QTheme::System); setStatusLine(L"AU: idle   |   theme: system"); return; }
    if (command == L"Text") { centerTabs_->setCurrentIndex(0); return; }
    if (command == L"Graph") { centerTabs_->setCurrentIndex(1); return; }
    if (command == L"Decompiler") { centerTabs_->setCurrentIndex(2); return; }
    if (command == L"Hex") { centerTabs_->setCurrentIndex(3); return; }
    if (command == L"Pseudocode") { centerTabs_->setCurrentIndex(2); return; }
    if (command == L"Xrefs") { rightTabs_->setCurrentIndex(0); return; }
    if (command == L"Output") { bottomTabs_->setCurrentIndex(0); return; }
    if (command == L"Functions") { leftTabs_->setCurrentIndex(0); return; }
    if (command == L"Imports") { leftTabs_->setCurrentIndex(1); return; }
    if (command == L"Exports") { leftTabs_->setCurrentIndex(1); return; }
    if (command == L"Names") { leftTabs_->setCurrentIndex(2); return; }
    if (command == L"Strings") { leftTabs_->setCurrentIndex(3); return; }
    if (command == L"About") {
        QMessageBox::about(this, L"About IDA Pro",
                           L"IDA Pro 9.0 (demo UI)\n\n"
                           L"Built with QtClone: ribbon, splitters, custom painted\n"
                           L"listing / graph / pseudocode views, light and dark themes.");
        return;
    }
    if (command == L"Exit") { close(); return; }
    setStatusLine(QString(L"AU: idle   |   command: ") + command);
}

int IdaWindow::selfTest() {
    int failures = 0;
    if (functions_.count() < 5) { printf("FAIL: functions\n"); ++failures; }
    if (!listing_->lines() || listing_->lines()->isEmpty()) { printf("FAIL: listing lines\n"); ++failures; }
    if (centerTabs_->count() != 4) { printf("FAIL: center tabs\n"); ++failures; }
    if (leftTabs_->count() != 4) { printf("FAIL: left tabs\n"); ++failures; }
    if (bottomTabs_->count() != 5) { printf("FAIL: bottom tabs\n"); ++failures; }
    selectFunction(1);
    if (functions_[1].name != L"sub_401100") { printf("FAIL: selection\n"); ++failures; }
    runCommand(L"Graph");
    if (centerTabs_->currentIndex() != 1) { printf("FAIL: graph command\n"); ++failures; }
    runCommand(L"Text");
    if (centerTabs_->currentIndex() != 0) { printf("FAIL: text command\n"); ++failures; }
    runCommand(L"theme_light");
    if (QTheme::isDark()) { printf("FAIL: light theme\n"); ++failures; }
    runCommand(L"theme_dark");
    if (!QTheme::isDark()) { printf("FAIL: dark theme\n"); ++failures; }
    printf("ida demo selftest: %s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures;
}

bool IdaWindow::screenshot(const QString& path) {
    QPixmap pm = grab();
    return pm.save(path);
}

// ===========================================================================
// entry point
// ===========================================================================
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QTheme::setMode(QTheme::Dark);
    IdaWindow window;
    QString shot;
    QString theme;
    bool selfTest = false;
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromUtf8(argv[i]);
        if (arg == L"--screenshot" && i + 1 < argc) shot = QString::fromUtf8(argv[++i]);
        else if (arg == L"--theme" && i + 1 < argc) theme = QString::fromUtf8(argv[++i]);
        else if (arg == L"--selftest") selfTest = true;
    }
    QString tab;
    for (int i = 1; i < argc; ++i) {
        if (QString::fromUtf8(argv[i]) == L"--tab" && i + 1 < argc) tab = QString::fromUtf8(argv[++i]);
    }
    if (theme == L"light") QTheme::setMode(QTheme::Light);
    else if (theme == L"dark") QTheme::setMode(QTheme::Dark);
    else if (theme == L"system") QTheme::setMode(QTheme::System);
    window.show();
    if (tab == L"graph") window.runCommand(L"Graph");
    else if (tab == L"pseudo") window.runCommand(L"Decompiler");
    else if (tab == L"hex") window.runCommand(L"Hex");
    for (int i = 0; i < 20; ++i) QApplication::processEvents();
    if (selfTest) return window.selfTest() ? 1 : 0;
    if (!shot.isEmpty()) {
        bool ok = window.screenshot(shot);
        printf("ida demo screenshot %s -> %s\n", ok ? "saved" : "FAILED", shot.toUtf8().c_str());
        return ok ? 0 : 1;
    }
    return QApplication::exec();
}
