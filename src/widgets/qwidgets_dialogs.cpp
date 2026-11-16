// QtClone - qwidgets_dialogs.cpp : dialogs, message boxes, native pickers, tool tips.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include "qwidget_p.h"
#include <commdlg.h>
#include <shlobj.h>
#include <stdio.h>
#include <vector>

// ===========================================================================
// QDialog
// ===========================================================================
const QMetaObject QDialog::staticMetaObject = {
    &QWidget::staticMetaObject, "QDialog", 0, 0, 0, 0
};

QVariant QDialog::qtcReadProperty(int id) const { return QWidget::qtcReadProperty(id); }
bool QDialog::qtcWriteProperty(int id, const QVariant& value) { return QWidget::qtcWriteProperty(id, value); }

QDialog::QDialog(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), result_(0), modal_(true), sizeGrip_(false) {
    setWindowModality(true);
}

QDialog::~QDialog() {}

int QDialog::exec() {
    result_ = 0;
    QWidget* owner = parentWidget();
    HWND ownerHwnd = 0;
    bool ownerEnabled = true;
    if (owner) {
        ownerHwnd = (HWND)owner->winId();
        if (ownerHwnd) {
            ownerEnabled = IsWindowEnabled(ownerHwnd) != FALSE;
            EnableWindow(ownerHwnd, FALSE);
        }
    }
    setResult(0);
    show();
    activateWindow();
    while (isVisible()) {
        MSG msg;
        if (GetMessageW(&msg, 0, 0, 0) <= 0) break;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        qtcDeliverPostedEvents();
        if (!isVisible()) break;
    }
    if (ownerHwnd) EnableWindow(ownerHwnd, ownerEnabled ? TRUE : FALSE);
    if (owner && ownerHwnd) SetForegroundWindow(ownerHwnd);
    return result_;
}

void QDialog::done(int result) {
    setResult(result);
    hide();
    finished(result);
    if (result == Accepted) accepted();
    else rejected();
}

void QDialog::accept() { done(Accepted); }
void QDialog::reject() { done(Rejected); }

void QDialog::closeEvent(QCloseEvent* event) {
    event->accept();
    done(Rejected);
}

void QDialog::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        reject();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void QDialog::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (sizeGrip_) setMinimumSize(200, 120);
}

// ===========================================================================
// QMessageBox
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QMessageBox[] = {
    { "text", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "informativeText", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "detailedText", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "icon", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "QMessageBox::Icon" },
    { "standardButtons", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
const QMetaObject QMessageBox::staticMetaObject = {
    &QDialog::staticMetaObject, "QMessageBox", qtc_props_QMessageBox, 5, 0, 0
};

QVariant QMessageBox::qtcReadProperty(int id) const {
    const int base = QDialog::staticMetaObject.propertyCount();
    if (id < base) return QDialog::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(text_);
    case 1: return QVariant(informative_);
    case 2: return QVariant(detailed_);
    case 3: return QVariant((int)icon_);
    case 4: return QVariant((int)buttons_);
    default: break;
    }
    return QVariant();
}

bool QMessageBox::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QDialog::staticMetaObject.propertyCount();
    if (id < base) return QDialog::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setText(value.toString()); return true;
    case 1: setInformativeText(value.toString()); return true;
    case 2: setDetailedText(value.toString()); return true;
    case 3: setIcon((Icon)value.toInt()); return true;
    case 4: setStandardButtons((StandardButtons)value.toInt()); return true;
    default: break;
    }
    return false;
}

QMessageBox::QMessageBox(QWidget* parent)
    : QDialog(parent, Qt::Dialog), icon_(NoIcon), buttons_(Ok), defaultButton_(NoButton),
      clicked_(NoButton), textFormat_(0) {
    setWindowTitle(L"Message");
    setMinimumSize(360, 150);
}

QMessageBox::QMessageBox(Icon icon, const QString& title, const QString& text,
                         StandardButtons buttons, QWidget* parent)
    : QDialog(parent, Qt::Dialog), icon_(icon), text_(text), buttons_(buttons),
      defaultButton_(NoButton), clicked_(NoButton), textFormat_(0) {
    setWindowTitle(title);
    setMinimumSize(380, 160);
    layoutContent();
}

QMessageBox::~QMessageBox() {}

void QMessageBox::setText(const QString& text) { text_ = text; layoutContent(); update(); }
void QMessageBox::setInformativeText(const QString& text) { informative_ = text; layoutContent(); update(); }
void QMessageBox::setWindowTitle(const QString& title) { QWidget::setWindowTitle(title); }
QString QMessageBox::windowTitle() const { return QWidget::windowTitle(); }

void QMessageBox::setStandardButtons(StandardButtons buttons) {
    buttons_ = buttons;
    layoutContent();
}

void QMessageBox::addButton(const QString& text, int role) {
    QPushButton* b = new QPushButton(text, this);
    b->show();
    buttonWidgets_.append(b);
    buttonValues_.append(role);
    b->clicked.connect(this, this, &QMessageBox::onButtonClicked);
}

void QMessageBox::layoutContent() {
    for (int i = 0; i < buttonWidgets_.count(); ++i) {
        delete buttonWidgets_[i];
        buttonWidgets_[i] = 0;
    }
    buttonWidgets_.clearAll();
    buttonValues_.clearAll();

    struct Entry { int flag; const wchar_t* text; bool isDefault; };
    Entry entries[] = {
        { Ok, L"OK", true }, { Save, L"Save", false }, { Cancel, L"Cancel", false },
        { Yes, L"Yes", true }, { No, L"No", false }, { Abort, L"Abort", false },
        { Retry, L"Retry", false }, { Ignore, L"Ignore", false }, { Close, L"Close", false },
        { Discard, L"Discard", false }, { Apply, L"Apply", false }, { Reset, L"Reset", false },
        { YesToAll, L"Yes to All", false }, { NoToAll, L"No to All", false }, { 0, 0, false }
    };
    for (int i = 0; entries[i].flag; ++i) {
        if (!(buttons_ & entries[i].flag)) continue;
        QPushButton* b = new QPushButton(entries[i].text, this);
        if (entries[i].flag == (int)defaultButton_ || (defaultButton_ == NoButton && entries[i].isDefault))
            b->setDefault(true);
        b->show();
        int value = entries[i].flag;
        b->clicked.connect(this, this, &QMessageBox::onButtonClicked);
        buttonWidgets_.append(b);
        buttonValues_.append(value);
    }
    QFontMetrics fm(font());
    int textW = qMax(fm.width(text_), fm.width(informative_));
    int wantW = qMax(300, textW + 140);
    int wantH = 130 + (informative_.isEmpty() ? 0 : fm.height() * 2);
    if (width() < wantW || height() < wantH) resize(wantW, wantH);
    update();
}

void QMessageBox::onButtonClicked() {
    QObject* source = qtcCurrentSender();
    for (int i = 0; i < buttonWidgets_.count(); ++i) {
        if ((QObject*)buttonWidgets_[i] == source) {
            clicked_ = (StandardButton)buttonValues_[i];
            done(buttonValues_[i]);
            return;
        }
    }
}

void QMessageBox::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    int bh = 30;
    int bw = 84;
    int gap = 8;
    int x = width() - 16;
    for (int i = buttonWidgets_.count() - 1; i >= 0; --i) {
        x -= bw;
        buttonWidgets_[i]->setGeometry(x, height() - bh - 14, bw, bh);
        x -= gap;
    }
}

void QMessageBox::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    if (buttonWidgets_.isEmpty()) layoutContent();
}

void QMessageBox::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.window);
    QFont f = font();
    QFont bold = f;
    bold.setBold(true);
    bold.setPixelSize(qMax(12, f.pixelHeight() + 1));

    int iconSize = 42;
    int x = 20;
    if (icon_ != NoIcon) {
        QRect iconRect(20, 22, iconSize, iconSize);
        QColor accent = c.accent;
        if (icon_ == Warning) accent = QColor(0xE8, 0xA2, 0x3C);
        if (icon_ == Critical) accent = QColor(0xD9, 0x3A, 0x3A);
        if (icon_ == Question) accent = QColor(0x4C, 0x8D, 0xFF);
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(accent));
        p.drawEllipse(iconRect);
        p.setPen(QPen(c.accentText, 3));
        QFont sym = bold;
        sym.setPixelSize(24);
        p.setFont(sym);
        const wchar_t* glyph = L"i";
        if (icon_ == Warning) glyph = L"!";
        else if (icon_ == Critical) glyph = L"x";
        else if (icon_ == Question) glyph = L"?";
        p.drawText(iconRect, Qt::AlignCenter, QString(glyph));
        x = 20 + iconSize + 16;
    }

    QRect textRect(x, 24, width() - x - 20, height() - 100);
    p.setFont(bold);
    p.setPen(QPen(c.windowText));
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignTop | (textFormat_ == 0 ? Qt::TextWordWrap : 0), text_);

    if (!informative_.isEmpty()) {
        QFont smallFont = f;
        smallFont.setPixelSize(qMax(11, f.pixelHeight() - 1));
        p.setFont(smallFont);
        p.setPen(QPen(c.disabledText));
        QRect infoRect(x, textRect.bottom() - 40, width() - x - 20, 44);
        p.drawText(infoRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, informative_);
    }
    p.setPen(QPen(c.border, 1));
    p.drawLine(0, height() - 56, width(), height() - 56);
}

void QMessageBox::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        if (buttons_ & Cancel) clicked_ = Cancel;
        else if (buttons_ & Close) clicked_ = Close;
        else clicked_ = NoButton;
        done((int)clicked_);
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        for (int i = 0; i < buttonWidgets_.count(); ++i) {
            if (buttonWidgets_[i]->isDefault()) {
                clicked_ = (StandardButton)buttonValues_[i];
                done(buttonValues_[i]);
                return;
            }
        }
        if (!buttonWidgets_.isEmpty()) {
            clicked_ = (StandardButton)buttonValues_[0];
            done(buttonValues_[0]);
            return;
        }
    }
    QDialog::keyPressEvent(event);
}

QMessageBox::StandardButton QMessageBox::information(QWidget* parent, const QString& title,
                                                     const QString& text, StandardButtons buttons,
                                                     StandardButton defaultButton) {
    QMessageBox box(Information, title, text, buttons, parent);
    box.setDefaultButton(defaultButton);
    int r = box.exec();
    return (StandardButton)r;
}

QMessageBox::StandardButton QMessageBox::warning(QWidget* parent, const QString& title,
                                                 const QString& text, StandardButtons buttons,
                                                 StandardButton defaultButton) {
    QMessageBox box(Warning, title, text, buttons, parent);
    box.setDefaultButton(defaultButton);
    int r = box.exec();
    return (StandardButton)r;
}

QMessageBox::StandardButton QMessageBox::critical(QWidget* parent, const QString& title,
                                                  const QString& text, StandardButtons buttons,
                                                  StandardButton defaultButton) {
    QMessageBox box(Critical, title, text, buttons, parent);
    box.setDefaultButton(defaultButton);
    int r = box.exec();
    return (StandardButton)r;
}

QMessageBox::StandardButton QMessageBox::question(QWidget* parent, const QString& title,
                                                  const QString& text, StandardButtons buttons,
                                                  StandardButton defaultButton) {
    QMessageBox box(Question, title, text, buttons, parent);
    box.setDefaultButton(defaultButton);
    int r = box.exec();
    return (StandardButton)r;
}

void QMessageBox::about(QWidget* parent, const QString& title, const QString& text) {
    QMessageBox box(Information, title, text, Ok, parent);
    box.exec();
}

void QMessageBox::aboutQt(QWidget* parent, const QString& title) {
    QMessageBox box(Information, title,
                    L"QtClone - a Qt-style C++ UI framework for Visual Studio 2010.\n"
                    L"No code generator, no external dependencies.", Ok, parent);
    box.exec();
}

// ===========================================================================
// QFileDialog (native dialogs)
// ===========================================================================
namespace {

struct FilterBuffer {
    std::wstring data;
    FilterBuffer(const QString& filter) {
        QStringList parts = filter.split(L"|");
        if (parts.count() < 2) {
            data = L"All files (*.*)\0*.*\0\0";
            return;
        }
        for (int i = 0; i + 1 < parts.count(); i += 2) {
            data += parts[i].c_str();
            data += (wchar_t)0;
            data += parts[i + 1].c_str();
            data += (wchar_t)0;
        }
        data += (wchar_t)0;
    }
};

QString runOpenDialog(QWidget* parent, const QString& caption, const QString& dir,
                      const QString& filter, bool save, bool multi) {
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    std::vector<wchar_t> file(2048, 0);
    std::wstring initial = dir.c_str();
    if (!initial.empty() && initial[initial.size() - 1] != L'\\') initial += L'\\';
    wcsncpy_s(&file[0], file.size(), initial.c_str(), _TRUNCATE);
    FilterBuffer fb(filter);
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = parent ? (HWND)parent->winId() : 0;
    ofn.lpstrFilter = fb.data.c_str();
    ofn.lpstrFile = &file[0];
    ofn.nMaxFile = (DWORD)file.size();
    ofn.lpstrTitle = caption.isEmpty() ? 0 : caption.c_str();
    ofn.Flags = OFN_EXPLORER | (multi ? OFN_ALLOWMULTISELECT : 0) |
                (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
    BOOL ok = save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
    if (!ok) return QString();
    return QString(&file[0]);
}

} // namespace

QString QFileDialog::getOpenFileName(QWidget* parent, const QString& caption, const QString& dir,
                                     const QString& filter, QString*) {
    return runOpenDialog(parent, caption, dir, filter, false, false);
}

QString QFileDialog::getSaveFileName(QWidget* parent, const QString& caption, const QString& dir,
                                     const QString& filter, QString*) {
    return runOpenDialog(parent, caption, dir, filter, true, false);
}

QStringList QFileDialog::getOpenFileNames(QWidget* parent, const QString& caption, const QString& dir,
                                          const QString& filter) {
    QStringList out;
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    std::vector<wchar_t> file(8192, 0);
    FilterBuffer fb(filter);
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = parent ? (HWND)parent->winId() : 0;
    ofn.lpstrFilter = fb.data.c_str();
    ofn.lpstrFile = &file[0];
    ofn.nMaxFile = (DWORD)file.size();
    ofn.lpstrTitle = caption.isEmpty() ? 0 : caption.c_str();
    ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) return out;
    const wchar_t* p = &file[0];
    std::wstring first = p;
    p += first.size() + 1;
    if (*p == 0) {
        out.append(QString(first.c_str()));
        return out;
    }
    while (*p) {
        QString name(p);
        out.append(QString(first.c_str()) + QString(L"\\") + name);
        p += name.size() + 1;
    }
    return out;
}

QString QFileDialog::getExistingDirectory(QWidget* parent, const QString& caption, const QString& dir) {
    BROWSEINFOW bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = parent ? (HWND)parent->winId() : 0;
    bi.lpszTitle = caption.isEmpty() ? L"Select folder" : caption.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST idl = SHBrowseForFolderW(&bi);
    if (!idl) return QString();
    wchar_t path[MAX_PATH];
    if (!SHGetPathFromIDListW(idl, path)) return QString();
    CoTaskMemFree(idl);
    return QString(path);
}

// ===========================================================================
// QColorDialog / QFontDialog
// ===========================================================================
QColor QColorDialog::getColor(const QColor& initial, QWidget* parent, const QString& title) {
    CHOOSECOLORW cc;
    ZeroMemory(&cc, sizeof(cc));
    COLORREF custom[16];
    for (int i = 0; i < 16; ++i) custom[i] = RGB(255, 255, 255);
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = parent ? (HWND)parent->winId() : 0;
    cc.lpCustColors = custom;
    cc.rgbResult = RGB(initial.red(), initial.green(), initial.blue());
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    cc.lpTemplateName = 0;
    if (!ChooseColorW(&cc)) return QColor();
    return QColor(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));
}

bool QColorDialog::getColor(QColor* color, QWidget* parent, const QString& title) {
    if (!color) return false;
    QColor result = getColor(*color, parent, title);
    if (!result.isValid()) return false;
    *color = result;
    return true;
}

QFont QFontDialog::getFont(bool* ok, const QFont& initial, QWidget* parent, const QString& title) {
    LOGFONTW lf;
    ZeroMemory(&lf, sizeof(lf));
    lf.lfHeight = -initial.pixelHeight();
    lf.lfWeight = initial.bold() ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = initial.italic() ? TRUE : FALSE;
    lf.lfUnderline = initial.underline() ? TRUE : FALSE;
    wcsncpy_s(lf.lfFaceName, LF_FACESIZE, initial.family().c_str(), _TRUNCATE);
    CHOOSEFONTW cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = parent ? (HWND)parent->winId() : 0;
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
    if (!ChooseFontW(&cf)) {
        if (ok) *ok = false;
        return initial;
    }
    if (ok) *ok = true;
    QFont result(lf.lfFaceName);
    int px = -lf.lfHeight;
    result.setPixelSize(px > 0 ? px : 12);
    result.setBold(lf.lfWeight >= FW_BOLD);
    result.setItalic(lf.lfItalic != 0);
    result.setUnderline(lf.lfUnderline != 0);
    return result;
}

// ===========================================================================
// QInputDialog
// ===========================================================================
namespace {

class QSimpleInputDialog : public QDialog {
public:
    QSimpleInputDialog(QWidget* parent, const QString& title, const QString& label)
        : QDialog(parent, Qt::Dialog), edit_(0), spin_(0), dspin_(0), combo_(0) {
        setWindowTitle(title);
        label_ = label;
        setMinimumSize(380, 160);
        ok_ = new QPushButton(L"OK", this);
        cancel_ = new QPushButton(L"Cancel", this);
        ok_->setDefault(true);
        ok_->clicked.connect(this, this, &QSimpleInputDialog::onOk);
        cancel_->clicked.connect(this, this, &QSimpleInputDialog::onCancel);
    }
    void addEdit(const QString& text) {
        edit_ = new QLineEdit(text, this);
        edit_->show();
        edit_->setFocus();
    }
    void addSpin(int value, int minimum, int maximum, int step) {
        spin_ = new QSpinBox(this);
        spin_->setRange(minimum, maximum);
        spin_->setSingleStep(step);
        spin_->setValue(value);
        spin_->show();
        spin_->setFocus();
    }
    void addDoubleSpin(double value, double minimum, double maximum, int decimals) {
        dspin_ = new QDoubleSpinBox(this);
        dspin_->setRange(minimum, maximum);
        dspin_->setDecimals(decimals);
        dspin_->setValue(value);
        dspin_->show();
        dspin_->setFocus();
    }
    void addCombo(const QStringList& items, int current) {
        combo_ = new QComboBox(this);
        combo_->addItems(items);
        combo_->setCurrentIndex(current);
        combo_->show();
        combo_->setFocus();
    }
    QLineEdit* edit() const { return edit_; }
    QSpinBox* spin() const { return spin_; }
    QDoubleSpinBox* dspin() const { return dspin_; }
    QComboBox* combo() const { return combo_; }

protected:
    void resizeEvent(QResizeEvent* event) {
        QDialog::resizeEvent(event);
        QRect r = rect();
        int y = 56;
        QWidget* field = 0;
        if (edit_) field = edit_;
        else if (spin_) field = spin_;
        else if (dspin_) field = dspin_;
        else if (combo_) field = combo_;
        if (field) field->setGeometry(20, y, r.width() - 40, 28);
        int bw = 84, bh = 30;
        cancel_->setGeometry(r.width() - 20 - bw, r.height() - bh - 14, bw, bh);
        ok_->setGeometry(r.width() - 20 - bw * 2 - 8, r.height() - bh - 14, bw, bh);
    }
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        p.fillRect(rect(), QTheme::colors().window);
        p.setPen(QPen(QTheme::colors().windowText));
        p.setFont(font());
        p.drawText(QRect(20, 18, width() - 40, 28), Qt::AlignLeft | Qt::AlignVCenter, label_);
    }
    void keyPressEvent(QKeyEvent* event) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) { onOk(); return; }
        QDialog::keyPressEvent(event);
    }

private:
    void onOk() { accept(); }
    void onCancel() { reject(); }
    QLineEdit* edit_;
    QSpinBox* spin_;
    QDoubleSpinBox* dspin_;
    QComboBox* combo_;
    QPushButton* ok_;
    QPushButton* cancel_;
    QString label_;
};

} // namespace

QString QInputDialog::getText(QWidget* parent, const QString& title, const QString& label,
                              int echoMode, const QString& text, bool* ok) {
    QSimpleInputDialog dlg(parent, title, label);
    dlg.addEdit(text);
    if (dlg.edit()) dlg.edit()->setEchoMode((QLineEdit::EchoMode)echoMode);
    int result = dlg.exec();
    if (ok) *ok = (result == QDialog::Accepted);
    return dlg.edit() ? dlg.edit()->text() : QString();
}

int QInputDialog::getInt(QWidget* parent, const QString& title, const QString& label,
                         int value, int minimum, int maximum, int step, bool* ok) {
    QSimpleInputDialog dlg(parent, title, label);
    dlg.addSpin(value, minimum, maximum, step);
    int result = dlg.exec();
    if (ok) *ok = (result == QDialog::Accepted);
    return dlg.spin() ? dlg.spin()->value() : value;
}

double QInputDialog::getDouble(QWidget* parent, const QString& title, const QString& label,
                               double value, double minimum, double maximum, int decimals, bool* ok) {
    QSimpleInputDialog dlg(parent, title, label);
    dlg.addDoubleSpin(value, minimum, maximum, decimals);
    int result = dlg.exec();
    if (ok) *ok = (result == QDialog::Accepted);
    return dlg.dspin() ? dlg.dspin()->value() : value;
}

QString QInputDialog::getItem(QWidget* parent, const QString& title, const QString& label,
                              const QStringList& items, int current, bool editable, bool* ok) {
    QSimpleInputDialog dlg(parent, title, label);
    dlg.addCombo(items, current);
    int result = dlg.exec();
    if (ok) *ok = (result == QDialog::Accepted);
    return dlg.combo() ? dlg.combo()->currentText() : QString();
}

// ===========================================================================
// QToolTip
// ===========================================================================
namespace {

class QToolTipWindow : public QWidget {
public:
    QToolTipWindow() : QWidget(0, Qt::ToolTip) {
        setFocusPolicy(Qt::NoFocus);
    }
    void showFor(const QPoint& pos, const QString& text) {
        text_ = text;
        QFontMetrics fm(font());
        int w = fm.width(text) + 20;
        int h = fm.height() + 12;
        setGeometry(QRect(QPoint(pos.x() + 12, pos.y() + 18), QSize(w, h)));
        show();
        raise();
        QTimer* timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->timeout.connect(this, this, &QToolTipWindow::hideTip);
        timer->start(6000);
    }
    void hideTip() { hide(); }
    QString text() const { return text_; }
protected:
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        QStyleOption opt;
        opt.rect = rect();
        style()->drawToolTip(&p, opt, this);
        p.setPen(QPen(QTheme::colors().toolTipText));
        p.setFont(font());
        p.drawText(rect().adjusted(8, 0, -8, 0), Qt::AlignLeft | Qt::AlignVCenter, text_);
    }
private:
    QString text_;
};

QToolTipWindow* toolTipWindow() {
    static QToolTipWindow* window = 0;
    if (!window) window = new QToolTipWindow();
    return window;
}

} // namespace

void QToolTip::showText(const QPoint& pos, const QString& text, QWidget*, int) {
    if (text.isEmpty()) { hideText(); return; }
    toolTipWindow()->showFor(pos, text);
}

void QToolTip::hideText() { if (toolTipWindow()->isVisible()) toolTipWindow()->hide(); }
bool QToolTip::isVisible() { return toolTipWindow()->isVisible(); }
QString QToolTip::text() { return toolTipWindow()->text(); }

// Keeps this translation unit linked when the framework is a static library.
void qtcWidgetAnchor_dialogs() {}
