// QtClone - qwidgets_value.cpp : QLineEdit, sliders, spin boxes, combos, group box.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include "qwidget_p.h"
#include <stdio.h>
#include <string.h>

// ===========================================================================
// QLineEdit
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QLineEdit[] = {
    { "text", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "placeholderText", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "readOnly", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "echoMode", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "QLineEdit::EchoMode" },
    { "maxLength", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "alignment", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::Alignment" },
    { "clearButtonEnabled", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QLineEdit[] = {
    { "textChanged", "textChanged(QString)" },
    { "textEdited", "textEdited(QString)" },
    { "returnPressed", "returnPressed()" },
    { "editingFinished", "editingFinished()" },
    { "selectionChanged", "selectionChanged()" }
};
const QMetaObject QLineEdit::staticMetaObject = {
    &QWidget::staticMetaObject, "QLineEdit",
    qtc_props_QLineEdit, 7, qtc_signals_QLineEdit, 5
};

QVariant QLineEdit::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(text_);
    case 1: return QVariant(placeholder_);
    case 2: return QVariant(readOnly_);
    case 3: return QVariant((int)echoMode_);
    case 4: return QVariant(maxLength_);
    case 5: return QVariant((int)alignment_);
    case 6: return QVariant(clearButton_);
    default: break;
    }
    return QVariant();
}

bool QLineEdit::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setText(value.toString()); return true;
    case 1: setPlaceholderText(value.toString()); return true;
    case 2: setReadOnly(value.toBool()); return true;
    case 3: setEchoMode((EchoMode)value.toInt()); return true;
    case 4: setMaxLength(value.toInt()); return true;
    case 5: setAlignment((Qt::Alignment)value.toInt()); return true;
    case 6: setClearButtonEnabled(value.toBool()); return true;
    default: break;
    }
    return false;
}

QLineEdit::QLineEdit(QWidget* parent)
    : QWidget(parent), cursor_(0), selStart_(0), selEnd_(0), scrollOffset_(0),
      readOnly_(false), echoMode_(Normal), maxLength_(32767),
      alignment_(Qt::AlignLeft | Qt::AlignVCenter), modified_(false), clearButton_(false),
      cursorVisible_(true), cursorBlink_(false), blinkTimer_(0), selecting_(false),
      selectAnchor_(0) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumSize(60, 26);
}

QLineEdit::QLineEdit(const QString& text, QWidget* parent)
    : QWidget(parent), text_(text), cursor_(text.size()), selStart_(0), selEnd_(0),
      scrollOffset_(0), readOnly_(false), echoMode_(Normal), maxLength_(32767),
      alignment_(Qt::AlignLeft | Qt::AlignVCenter), modified_(false), clearButton_(false),
      cursorVisible_(true), cursorBlink_(false), blinkTimer_(0), selecting_(false),
      selectAnchor_(0) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumSize(60, 26);
}

QLineEdit::~QLineEdit() { if (blinkTimer_) killTimer(blinkTimer_); }

QString QLineEdit::displayText() const {
    if (echoMode_ == Password || echoMode_ == PasswordEchoOnEdit) {
        QString hidden;
        for (int i = 0; i < text_.size(); ++i) hidden += (wchar_t)0x25CF;
        return hidden;
    }
    return text_;
}

void QLineEdit::setText(const QString& text) {
    if (text_ == text) return;
    text_ = text;
    if (cursor_ > text_.size()) cursor_ = text_.size();
    selStart_ = selEnd_ = cursor_;
    update();
    textChanged(text_);
}

void QLineEdit::setPlaceholderText(const QString& text) { placeholder_ = text; update(); }
void QLineEdit::setReadOnly(bool on) { readOnly_ = on; update(); }
void QLineEdit::setEchoMode(EchoMode mode) { echoMode_ = mode; update(); }
void QLineEdit::setMaxLength(int length) { maxLength_ = length; }
void QLineEdit::setAlignment(Qt::Alignment alignment) { alignment_ = alignment; update(); }

void QLineEdit::setCursorPosition(int pos) {
    cursor_ = qBound(0, pos, text_.size());
    update();
}

QString QLineEdit::selectedText() const {
    if (selStart_ == selEnd_) return QString();
    int a = qMin(selStart_, selEnd_);
    int b = qMax(selStart_, selEnd_);
    return text_.mid(a, b - a);
}

void QLineEdit::selectAll() { selStart_ = 0; selEnd_ = text_.size(); update(); }
void QLineEdit::deselect() { selStart_ = selEnd_ = cursor_; update(); }

void QLineEdit::clear() {
    setText(QString());
    cursor_ = 0;
    selStart_ = selEnd_ = 0;
    update();
}

QSize QLineEdit::sizeHint() const {
    QFontMetrics fm(font());
    return QSize(qMax(120, fm.width(text_) + 24), qMax(26, fm.height() + 10));
}

QSize QLineEdit::minimumSizeHint() const { return QSize(60, qMax(24, QFontMetrics(font()).height() + 8)); }

int QLineEdit::xForPosition(int pos) const {
    QString shown = displayText();
    if (pos <= 0) return 8;
    int x = 8 + QFontMetrics(font()).width(shown.left(pos));
    return x - scrollOffset_;
}

int QLineEdit::positionForX(int x) const {
    QString shown = displayText();
    int target = x - 8 + scrollOffset_;
    int best = 0;
    for (int i = 1; i <= shown.size(); ++i) {
        int w = QFontMetrics(font()).width(shown.left(i));
        if (w <= target) best = i;
        else break;
    }
    return best;
}

void QLineEdit::ensureCursorVisible() {
    int x = xForPosition(cursor_);
    int left = 8;
    int right = width() - 8;
    if (x > right) scrollOffset_ += x - right;
    else if (x < left) scrollOffset_ = qMax(0, scrollOffset_ - (left - x));
    if (xForPosition(0) > 8) scrollOffset_ = 0;
}

void QLineEdit::insertText(const QString& text) {
    if (readOnly_) return;
    deleteSelection();
    if (text_.size() + text.size() > maxLength_) {
        int room = maxLength_ - text_.size();
        if (room <= 0) return;
        QString cut = text.left(room);
        text_ = text_.left(cursor_) + cut + text_.mid(cursor_);
        cursor_ += cut.size();
    } else {
        text_ = text_.left(cursor_) + text + text_.mid(cursor_);
        cursor_ += text.size();
    }
    modified_ = true;
    selStart_ = selEnd_ = cursor_;
    ensureCursorVisible();
    update();
    textEdited(text_);
    textChanged(text_);
    startBlink();
}

void QLineEdit::deleteSelection() {
    if (selStart_ == selEnd_) return;
    int a = qMin(selStart_, selEnd_);
    int b = qMax(selStart_, selEnd_);
    text_ = text_.left(a) + text_.mid(b);
    cursor_ = a;
    selStart_ = selEnd_ = cursor_;
    modified_ = true;
    textChanged(text_);
}

void QLineEdit::copy() {
    if (hasSelectedText()) QApplication::clipboard()->setText(selectedText());
}

void QLineEdit::cut() {
    if (!hasSelectedText() || readOnly_) return;
    QApplication::clipboard()->setText(selectedText());
    deleteSelection();
    update();
}

void QLineEdit::paste() {
    if (readOnly_) return;
    insertText(QApplication::clipboard()->text());
}

void QLineEdit::undo() {}
void QLineEdit::redo() {}

void QLineEdit::startBlink() {
    if (!hasFocus()) return;
    stopBlink();
    cursorVisible_ = true;
    cursorBlink_ = true;
    blinkTimer_ = startTimer(500);
    update();
}

void QLineEdit::stopBlink() {
    if (blinkTimer_) {
        killTimer(blinkTimer_);
        blinkTimer_ = 0;
    }
    cursorBlink_ = false;
}

void QLineEdit::timerEvent(QTimerEvent* event) {
    if (event->timerId() == blinkTimer_) {
        cursorVisible_ = !cursorVisible_;
        update();
        return;
    }
    QWidget::timerEvent(event);
}

void QLineEdit::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOptionFrame opt;
    opt.rect = rect();
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    if (hasFocus()) opt.state |= QStyle::State_HasFocus;
    style()->drawLineEditFrame(&p, opt, this);

    QRect inner = rect().adjusted(8, 3, -8, -3);
    p.setPen(QPen(QStyleSheet::resolveColor(this, QPalette::Text)));
    p.setFont(font());
    p.setClipRect(inner);
    QString shown = displayText();
    int textW = QFontMetrics(font()).width(shown);
    int x = 8 - scrollOffset_;
    if (alignment_ & Qt::AlignRight) x = inner.right() - textW;
    else if (alignment_ & Qt::AlignHCenter) x = inner.left() + (inner.width() - textW) / 2;

    if (selectionStart() != -1 && selStart_ != selEnd_) {
        int a = qMin(selStart_, selEnd_);
        int b = qMax(selStart_, selEnd_);
        int sx = x + QFontMetrics(font()).width(shown.left(a));
        int sw = QFontMetrics(font()).width(shown.mid(a, b - a));
        p.fillRect(QRect(sx, inner.top(), sw, inner.height()), QTheme::colors().selection);
    }
    if (text_.isEmpty() && !placeholder_.isEmpty()) {
        p.setPen(QPen(QTheme::colors().disabledText));
        p.drawText(inner, Qt::AlignLeft | Qt::AlignVCenter, placeholder_);
    } else {
        p.setPen(QPen(QStyleSheet::resolveColor(this, QPalette::Text)));
        p.drawText(QRect(x, inner.top(), qMax(textW + 2, 4), inner.height()),
                   Qt::AlignLeft | Qt::AlignVCenter, shown);
    }
    if (!imePreedit_.isEmpty()) {
        int px = x + QFontMetrics(font()).width(shown.left(cursor_));
        p.setPen(QPen(QTheme::colors().accent));
        p.drawText(QRect(px, inner.top(), 200, inner.height()), Qt::AlignLeft | Qt::AlignVCenter, imePreedit_);
        p.drawLine(px, inner.bottom() - 1, px + QFontMetrics(font()).width(imePreedit_), inner.bottom() - 1);
    }
    if (hasFocus() && cursorVisible_ && selStart_ == selEnd_) {
        int cx = xForPosition(cursor_);
        p.setPen(QPen(QTheme::colors().windowText, 1));
        p.drawLine(cx, inner.top() + 1, cx, inner.bottom() - 1);
    }
    p.setClipping(false);
    if (clearButton_ && !text_.isEmpty() && underMouse()) {
        QRect cb(rect().right() - 20, rect().center().y() - 6, 12, 12);
        p.setPen(QPen(QTheme::colors().disabledText, 1));
        p.drawLine(cb.left(), cb.top(), cb.right(), cb.bottom());
        p.drawLine(cb.right(), cb.top(), cb.left(), cb.bottom());
    }
}

void QLineEdit::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    if (clearButton_ && !text_.isEmpty() && event->pos().x() > width() - 24) {
        clear();
        return;
    }
    setFocus(Qt::MouseFocusReason);
    selecting_ = true;
    selectAnchor_ = positionForX(event->pos().x());
    cursor_ = selectAnchor_;
    selStart_ = selEnd_ = cursor_;
    update();
}

void QLineEdit::mouseMoveEvent(QMouseEvent* event) {
    if (!selecting_) return;
    int pos = positionForX(event->pos().x());
    selStart_ = selectAnchor_;
    selEnd_ = pos;
    cursor_ = pos;
    update();
}

void QLineEdit::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    selecting_ = false;
}

void QLineEdit::mouseDoubleClickEvent(QMouseEvent*) { selectAll(); }

void QLineEdit::keyPressEvent(QKeyEvent* event) {
    if (event->text().isEmpty() || event->key() == Qt::Key_Tab) {
        bool shift = event->modifiers() & Qt::ShiftModifier;
        bool ctrl = event->modifiers() & Qt::ControlModifier;
        switch (event->key()) {
        case Qt::Key_Left:
            if (shift) { selEnd_ = qMax(0, selEnd_ - 1); cursor_ = selEnd_; }
            else { cursor_ = qMax(0, cursor_ - 1); selStart_ = selEnd_ = cursor_; }
            ensureCursorVisible();
            update();
            event->accept();
            return;
        case Qt::Key_Right:
            if (shift) { selEnd_ = qMin(text_.size(), selEnd_ + 1); cursor_ = selEnd_; }
            else { cursor_ = qMin(text_.size(), cursor_ + 1); selStart_ = selEnd_ = cursor_; }
            ensureCursorVisible();
            update();
            event->accept();
            return;
        case Qt::Key_Home:
            cursor_ = 0; if (!shift) selStart_ = selEnd_ = cursor_;
            ensureCursorVisible(); update(); event->accept(); return;
        case Qt::Key_End:
            cursor_ = text_.size(); if (!shift) selStart_ = selEnd_ = cursor_;
            ensureCursorVisible(); update(); event->accept(); return;
        case Qt::Key_Backspace:
            if (readOnly_) break;
            if (selStart_ != selEnd_) deleteSelection();
            else if (cursor_ > 0) {
                text_ = text_.left(cursor_ - 1) + text_.mid(cursor_);
                --cursor_;
                selStart_ = selEnd_ = cursor_;
                textChanged(text_);
            }
            ensureCursorVisible();
            update();
            event->accept();
            return;
        case Qt::Key_Delete:
            if (readOnly_) break;
            if (selStart_ != selEnd_) deleteSelection();
            else if (cursor_ < text_.size()) {
                text_ = text_.left(cursor_) + text_.mid(cursor_ + 1);
                textChanged(text_);
            }
            update();
            event->accept();
            return;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            returnPressed();
            editingFinished();
            event->accept();
            return;
        case Qt::Key_A:
            if (ctrl) { selectAll(); event->accept(); return; }
            break;
        case Qt::Key_C:
            if (ctrl) { copy(); event->accept(); return; }
            break;
        case Qt::Key_X:
            if (ctrl) { cut(); event->accept(); return; }
            break;
        case Qt::Key_V:
            if (ctrl) { paste(); event->accept(); return; }
            break;
        default:
            break;
        }
        if (event->key() == Qt::Key_Space) {
            insertText(L" ");
            event->accept();
            return;
        }
        event->ignore();
        return;
    }
    if (!readOnly_) insertText(event->text());
    event->accept();
}

void QLineEdit::focusInEvent(QFocusEvent* event) {
    QWidget::focusInEvent(event);
    startBlink();
    update();
}

void QLineEdit::focusOutEvent(QFocusEvent* event) {
    stopBlink();
    cursorVisible_ = false;
    selStart_ = selEnd_ = cursor_;
    editingFinished();
    QWidget::focusOutEvent(event);
    update();
}

void QLineEdit::inputMethodEvent(QInputMethodEvent* event) {
    if (!event->commitString().isEmpty()) {
        insertText(event->commitString());
        imePreedit_.clear();
    }
    if (!event->preeditString().isEmpty()) imePreedit_ = event->preeditString();
    else imePreedit_.clear();
    update();
}

void QLineEdit::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange) update();
    QWidget::changeEvent(event);
}

// ===========================================================================
// QProgressBar
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QProgressBar[] = {
    { "minimum", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "maximum", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "value", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "textVisible", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "format", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "orientation", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::Orientation" }
};
static const QMetaObject::SignalInfo qtc_signals_QProgressBar[] = {
    { "valueChanged", "valueChanged(int)" }
};
const QMetaObject QProgressBar::staticMetaObject = {
    &QWidget::staticMetaObject, "QProgressBar",
    qtc_props_QProgressBar, 6, qtc_signals_QProgressBar, 1
};

QVariant QProgressBar::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(minimum_);
    case 1: return QVariant(maximum_);
    case 2: return QVariant(value_);
    case 3: return QVariant(textVisible_);
    case 4: return QVariant(format_);
    case 5: return QVariant((int)orientation_);
    default: break;
    }
    return QVariant();
}

bool QProgressBar::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setMinimum(value.toInt()); return true;
    case 1: setMaximum(value.toInt()); return true;
    case 2: setValue(value.toInt()); return true;
    case 3: setTextVisible(value.toBool()); return true;
    case 4: setFormat(value.toString()); return true;
    case 5: setOrientation((Qt::Orientation)value.toInt()); return true;
    default: break;
    }
    return false;
}

QProgressBar::QProgressBar(QWidget* parent)
    : QWidget(parent), minimum_(0), maximum_(100), value_(0), textVisible_(true),
      orientation_(Qt::Horizontal), format_(L"%p%"), inverted_(false) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumSize(80, 22);
}

QProgressBar::~QProgressBar() {}

void QProgressBar::setValue(int value) {
    int clamped = qBound(minimum_, value, maximum_);
    if (clamped == value_ && value_ != 0) return;
    value_ = clamped;
    update();
    valueChanged(value_);
}

void QProgressBar::setRange(int minimum, int maximum) {
    minimum_ = minimum;
    maximum_ = maximum;
    if (maximum_ < minimum_) maximum_ = minimum_;
    setValue(qBound(minimum_, value_, maximum_));
}

QString QProgressBar::text() const {
    if (!textVisible_) return QString();
    QString fmt = format_;
    int percent = 0;
    int range = maximum_ - minimum_;
    if (range > 0) percent = (value_ - minimum_) * 100 / range;
    fmt = fmt.replace(L"%p", QString::number(percent));
    fmt = fmt.replace(L"%v", QString::number(value_));
    fmt = fmt.replace(L"%m", QString::number(maximum_ - minimum_));
    return fmt;
}

QSize QProgressBar::sizeHint() const { return QSize(160, 22); }

void QProgressBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOptionProgressBar opt;
    opt.rect = rect();
    opt.minimum = minimum_;
    opt.maximum = maximum_;
    opt.value = value_;
    opt.textVisible = textVisible_;
    opt.text = text();
    opt.vertical = orientation_ == Qt::Vertical;
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    style()->drawProgressBar(&p, opt, this);
}

// ===========================================================================
// QSlider
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QSlider[] = {
    { "minimum", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "maximum", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "value", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "singleStep", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "pageStep", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "orientation", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::Orientation" },
    { "tickPosition", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "tracking", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QSlider[] = {
    { "valueChanged", "valueChanged(int)" },
    { "sliderMoved", "sliderMoved(int)" },
    { "sliderPressed", "sliderPressed()" },
    { "sliderReleased", "sliderReleased()" }
};
const QMetaObject QSlider::staticMetaObject = {
    &QWidget::staticMetaObject, "QSlider",
    qtc_props_QSlider, 8, qtc_signals_QSlider, 4
};

QVariant QSlider::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(minimum_);
    case 1: return QVariant(maximum_);
    case 2: return QVariant(value_);
    case 3: return QVariant(singleStep_);
    case 4: return QVariant(pageStep_);
    case 5: return QVariant((int)orientation_);
    case 6: return QVariant(ticks_);
    case 7: return QVariant(tracking_);
    default: break;
    }
    return QVariant();
}

bool QSlider::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setMinimum(value.toInt()); return true;
    case 1: setMaximum(value.toInt()); return true;
    case 2: setValue(value.toInt()); return true;
    case 3: setSingleStep(value.toInt()); return true;
    case 4: setPageStep(value.toInt()); return true;
    case 5: setOrientation((Qt::Orientation)value.toInt()); return true;
    case 6: setTickPosition(value.toBool()); return true;
    case 7: setTracking(value.toBool()); return true;
    default: break;
    }
    return false;
}

QSlider::QSlider(QWidget* parent)
    : QWidget(parent), minimum_(0), maximum_(100), value_(0), singleStep_(1), pageStep_(10),
      orientation_(Qt::Horizontal), ticks_(false), tracking_(true), dragging_(false), dragOffset_(0) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumSize(80, 24);
}

QSlider::QSlider(Qt::Orientation orientation, QWidget* parent)
    : QWidget(parent), minimum_(0), maximum_(100), value_(0), singleStep_(1), pageStep_(10),
      orientation_(orientation), ticks_(false), tracking_(true), dragging_(false), dragOffset_(0) {
    setFocusPolicy(Qt::StrongFocus);
    if (orientation == Qt::Horizontal) setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    else setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMinimumSize(24, 24);
}

QSlider::~QSlider() {}

void QSlider::setRange(int minimum, int maximum) {
    minimum_ = minimum;
    maximum_ = maximum;
    if (maximum_ < minimum_) maximum_ = minimum_;
    setValue(qBound(minimum_, value_, maximum_));
}

void QSlider::setValue(int value) { setValueInternal(value, false); }

void QSlider::setValueInternal(int value, bool move) {
    int clamped = qBound(minimum_, value, maximum_);
    if (clamped == value_ && !move) return;
    value_ = clamped;
    update();
    if (move) sliderMoved(value_);
    valueChanged(value_);
}

QSize QSlider::sizeHint() const {
    return orientation_ == Qt::Horizontal ? QSize(160, 24) : QSize(24, 160);
}

void QSlider::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOptionSlider opt;
    opt.rect = rect();
    opt.minimum = minimum_;
    opt.maximum = maximum_;
    opt.value = value_;
    opt.orientation = orientation_;
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    if (underMouse()) opt.state |= QStyle::State_Hover;
    if (hasFocus()) opt.state |= QStyle::State_HasFocus;
    style()->drawSlider(&p, opt, this);
}

void QSlider::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    dragging_ = true;
    sliderPressed();
    int range = maximum_ - minimum_;
    if (range <= 0) return;
    int pos = orientation_ == Qt::Horizontal ? event->pos().x() : event->pos().y();
    int extent = (orientation_ == Qt::Horizontal ? width() : height()) - 12;
    int v = minimum_ + (pos - 6) * range / qMax(1, extent);
    setValueInternal(v, true);
    event->accept();
}

void QSlider::mouseMoveEvent(QMouseEvent* event) {
    if (!dragging_) return;
    int range = maximum_ - minimum_;
    if (range <= 0) return;
    int pos = orientation_ == Qt::Horizontal ? event->pos().x() : event->pos().y();
    int extent = (orientation_ == Qt::Horizontal ? width() : height()) - 12;
    int v = minimum_ + (pos - 6) * range / qMax(1, extent);
    setValueInternal(v, true);
}

void QSlider::mouseReleaseEvent(QMouseEvent*) {
    if (dragging_) {
        dragging_ = false;
        sliderReleased();
    }
}

void QSlider::wheelEvent(QWheelEvent* event) {
    setValueInternal(value_ + (event->delta() > 0 ? singleStep_ : -singleStep_), true);
}

void QSlider::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Down:
        setValueInternal(value_ - singleStep_, false);
        event->accept();
        return;
    case Qt::Key_Right:
    case Qt::Key_Up:
        setValueInternal(value_ + singleStep_, false);
        event->accept();
        return;
    case Qt::Key_PageUp:
        setValueInternal(value_ + pageStep_, false);
        event->accept();
        return;
    case Qt::Key_PageDown:
        setValueInternal(value_ - pageStep_, false);
        event->accept();
        return;
    case Qt::Key_Home:
        setValueInternal(minimum_, false);
        event->accept();
        return;
    case Qt::Key_End:
        setValueInternal(maximum_, false);
        event->accept();
        return;
    default:
        break;
    }
    QWidget::keyPressEvent(event);
}

// ===========================================================================
// QScrollBar
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QScrollBar[] = {
    { "minimum", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "maximum", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "value", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "pageStep", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "singleStep", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "orientation", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::Orientation" }
};
static const QMetaObject::SignalInfo qtc_signals_QScrollBar[] = {
    { "valueChanged", "valueChanged(int)" },
    { "sliderPressed", "sliderPressed()" },
    { "sliderReleased", "sliderReleased()" }
};
const QMetaObject QScrollBar::staticMetaObject = {
    &QWidget::staticMetaObject, "QScrollBar",
    qtc_props_QScrollBar, 6, qtc_signals_QScrollBar, 3
};

QVariant QScrollBar::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(minimum_);
    case 1: return QVariant(maximum_);
    case 2: return QVariant(value_);
    case 3: return QVariant(pageStep_);
    case 4: return QVariant(singleStep_);
    case 5: return QVariant((int)orientation_);
    default: break;
    }
    return QVariant();
}

bool QScrollBar::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setMinimum(value.toInt()); return true;
    case 1: setMaximum(value.toInt()); return true;
    case 2: setValue(value.toInt()); return true;
    case 3: setPageStep(value.toInt()); return true;
    case 4: setSingleStep(value.toInt()); return true;
    case 5: setOrientation((Qt::Orientation)value.toInt()); return true;
    default: break;
    }
    return false;
}

QScrollBar::QScrollBar(QWidget* parent)
    : QWidget(parent), minimum_(0), maximum_(100), value_(0), pageStep_(10), singleStep_(1),
      orientation_(Qt::Vertical), dragging_(false), dragOffset_(0), pressedPart_(0), hoverPart_(0) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

QScrollBar::QScrollBar(Qt::Orientation orientation, QWidget* parent)
    : QWidget(parent), minimum_(0), maximum_(100), value_(0), pageStep_(10), singleStep_(1),
      orientation_(orientation), dragging_(false), dragOffset_(0), pressedPart_(0), hoverPart_(0) {
    setSizePolicy(orientation == Qt::Vertical ? QSizePolicy::Fixed : QSizePolicy::Expanding,
                  orientation == Qt::Vertical ? QSizePolicy::Expanding : QSizePolicy::Fixed);
}

QScrollBar::~QScrollBar() {}

void QScrollBar::setRange(int minimum, int maximum) {
    minimum_ = minimum;
    maximum_ = maximum;
    if (maximum_ < minimum_) maximum_ = minimum_;
    setValue(qBound(minimum_, value_, maximum_));
}

void QScrollBar::setValue(int value) {
    int clamped = qBound(minimum_, value, maximum_);
    if (clamped == value_) return;
    value_ = clamped;
    update();
    valueChanged(value_);
}

QSize QScrollBar::sizeHint() const {
    return orientation_ == Qt::Vertical ? QSize(12, 100) : QSize(100, 12);
}

QRect QScrollBar::grooveRect() const { return rect().adjusted(0, 0, -1, -1); }

QRect QScrollBar::sliderRect() const {
    QRect g = grooveRect();
    bool horizontal = orientation_ == Qt::Horizontal;
    int extent = horizontal ? g.width() : g.height();
    int range = maximum_ - minimum_ + pageStep_;
    if (range <= 0) range = 1;
    int thumb = qMax(24, extent * pageStep_ / range);
    int travel = extent - thumb;
    int pos = (value_ - minimum_) * travel / qMax(1, maximum_ - minimum_);
    if (horizontal) return QRect(g.left() + pos, g.top(), thumb, g.height());
    return QRect(g.left(), g.top() + pos, g.width(), thumb);
}

int QScrollBar::pixelToValue(int pixel) const {
    QRect g = grooveRect();
    bool horizontal = orientation_ == Qt::Horizontal;
    int extent = horizontal ? g.width() : g.height();
    int range = maximum_ - minimum_;
    if (extent <= 0) return minimum_;
    return minimum_ + qBound(0, pixel, extent) * range / extent;
}

void QScrollBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOptionScrollBar opt;
    opt.rect = rect();
    opt.minimum = minimum_;
    opt.maximum = maximum_;
    opt.value = value_;
    opt.pageStep = pageStep_;
    opt.orientation = orientation_;
    opt.pressedPart = pressedPart_;
    opt.hoverPart = hoverPart_;
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    style()->drawScrollBar(&p, opt, this);
}

void QScrollBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    QRect slider = sliderRect();
    if (slider.contains(event->pos())) {
        dragging_ = true;
        dragOffset_ = (orientation_ == Qt::Horizontal ? event->pos().x() - slider.left()
                                                     : event->pos().y() - slider.top());
        sliderPressed();
    } else {
        int target = orientation_ == Qt::Horizontal ? event->pos().x() : event->pos().y();
        int thumbPos = orientation_ == Qt::Horizontal ? slider.left() : slider.top();
        setValue(target < thumbPos ? value_ - pageStep_ : value_ + pageStep_);
    }
    event->accept();
}

void QScrollBar::mouseMoveEvent(QMouseEvent* event) {
    if (!dragging_) return;
    bool horizontal = orientation_ == Qt::Horizontal;
    int pos = (horizontal ? event->pos().x() : event->pos().y()) - dragOffset_;
    QRect g = grooveRect();
    int extent = horizontal ? g.width() : g.height();
    int thumb = horizontal ? sliderRect().width() : sliderRect().height();
    int travel = extent - thumb;
    if (travel <= 0) travel = 1;
    int value = minimum_ + (pos - (horizontal ? g.left() : g.top())) * (maximum_ - minimum_) / travel;
    setValue(value);
}

void QScrollBar::mouseReleaseEvent(QMouseEvent*) {
    if (dragging_) {
        dragging_ = false;
        sliderReleased();
    }
}

void QScrollBar::wheelEvent(QWheelEvent* event) {
    setValue(value_ - (event->delta() > 0 ? singleStep_ * 3 : -singleStep_ * 3));
}

// ===========================================================================
// QSpinBox
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QSpinBox[] = {
    { "minimum", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "maximum", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "value", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "singleStep", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "prefix", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "suffix", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "readOnly", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "wrapping", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QSpinBox[] = {
    { "valueChanged", "valueChanged(int)" },
    { "textChanged", "textChanged(QString)" }
};
const QMetaObject QSpinBox::staticMetaObject = {
    &QWidget::staticMetaObject, "QSpinBox",
    qtc_props_QSpinBox, 8, qtc_signals_QSpinBox, 2
};

QVariant QSpinBox::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(minimum_);
    case 1: return QVariant(maximum_);
    case 2: return QVariant(value_);
    case 3: return QVariant(singleStep_);
    case 4: return QVariant(prefix_);
    case 5: return QVariant(suffix_);
    case 6: return QVariant(readOnly_);
    case 7: return QVariant(wrapping_);
    default: break;
    }
    return QVariant();
}

bool QSpinBox::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setMinimum(value.toInt()); return true;
    case 1: setMaximum(value.toInt()); return true;
    case 2: setValue(value.toInt()); return true;
    case 3: setSingleStep(value.toInt()); return true;
    case 4: setPrefix(value.toString()); return true;
    case 5: setSuffix(value.toString()); return true;
    case 6: setReadOnly(value.toBool()); return true;
    case 7: setWrapping(value.toBool()); return true;
    default: break;
    }
    return false;
}

QSpinBox::QSpinBox(QWidget* parent)
    : QWidget(parent), minimum_(0), maximum_(99), value_(0), singleStep_(1),
      readOnly_(false), wrapping_(false), editing_(false), cursorVisible_(false),
      cursor_(0), blinkTimer_(0) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMinimumSize(70, 26);
}

QSpinBox::~QSpinBox() { if (blinkTimer_) killTimer(blinkTimer_); }

void QSpinBox::setRange(int minimum, int maximum) {
    minimum_ = minimum;
    maximum_ = maximum;
    if (maximum_ < minimum_) maximum_ = minimum_;
    clamp();
}

void QSpinBox::setValue(int value) {
    int clamped = qBound(minimum_, value, maximum_);
    if (clamped == value_) return;
    value_ = clamped;
    if (editing_) editing_ = false;
    update();
    valueChanged(value_);
    textChanged(text());
}

void QSpinBox::clamp() { setValue(qBound(minimum_, value_, maximum_)); }

QString QSpinBox::text() const {
    if (editing_) return prefix_ + draft_ + suffix_;
    return prefix_ + QString::number(value_) + suffix_;
}

QRect QSpinBox::editorRect() const { return rect().adjusted(0, 0, -18, 0); }
QRect QSpinBox::upButtonRect() const { return QRect(width() - 18, 0, 18, height() / 2); }
QRect QSpinBox::downButtonRect() const { return QRect(width() - 18, height() / 2, 18, height() - height() / 2); }

QSize QSpinBox::sizeHint() const { return QSize(90, 26); }

void QSpinBox::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOptionFrame opt;
    opt.rect = rect();
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    if (hasFocus()) opt.state |= QStyle::State_HasFocus;
    style()->drawLineEditFrame(&p, opt, this);

    p.setPen(QPen(QStyleSheet::resolveColor(this, QPalette::Text)));
    p.setFont(font());
    p.drawText(editorRect().adjusted(8, 0, -4, 0), Qt::AlignLeft | Qt::AlignVCenter, text());

    QRect up = upButtonRect();
    QRect down = downButtonRect();
    p.setPen(QPen(QTheme::colors().border, 1));
    p.drawLine(up.left(), up.top() + 3, up.left(), down.bottom() - 3);
    p.drawLine(up.left(), up.bottom(), width() - 1, up.bottom());
    style()->drawArrow(&p, up.adjusted(4, 3, -4, -3), Qt::UpArrow, QTheme::colors().windowText);
    style()->drawArrow(&p, down.adjusted(4, 3, -4, -3), Qt::DownArrow, QTheme::colors().windowText);
}

void QSpinBox::stepBy(int steps) {
    int v = value_ + steps * singleStep_;
    if (wrapping_) {
        int range = maximum_ - minimum_ + 1;
        if (range > 0) {
            v = ((v - minimum_) % range + range) % range + minimum_;
        }
    }
    setValue(v);
}

void QSpinBox::valueFromText() {
    bool ok = false;
    int v = draft_.toInt(&ok);
    if (ok) setValue(v);
    editing_ = false;
    update();
}

void QSpinBox::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Up) { stepBy(1); event->accept(); return; }
    if (event->key() == Qt::Key_Down) { stepBy(-1); event->accept(); return; }
    if (!event->text().isEmpty() && (event->text()[0] >= L'0' && event->text()[0] <= L'9')) {
        if (!editing_) { draft_.clear(); editing_ = true; }
        draft_ += event->text();
        update();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Backspace && editing_) {
        draft_ = draft_.left(draft_.size() - 1);
        update();
        event->accept();
        return;
    }
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && editing_) {
        valueFromText();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void QSpinBox::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    if (upButtonRect().contains(event->pos())) stepBy(1);
    else if (downButtonRect().contains(event->pos())) stepBy(-1);
    event->accept();
}

void QSpinBox::wheelEvent(QWheelEvent* event) { stepBy(event->delta() > 0 ? 1 : -1); }

void QSpinBox::focusInEvent(QFocusEvent* event) {
    editing_ = false;
    QWidget::focusInEvent(event);
    update();
}

void QSpinBox::focusOutEvent(QFocusEvent* event) {
    if (editing_) valueFromText();
    QWidget::focusOutEvent(event);
}

// ===========================================================================
// QDoubleSpinBox
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QDoubleSpinBox[] = {
    { "minimum", "double", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "maximum", "double", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "value", "double", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "singleStep", "double", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "decimals", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "prefix", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "suffix", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QDoubleSpinBox[] = {
    { "valueChanged", "valueChanged(double)" }
};
const QMetaObject QDoubleSpinBox::staticMetaObject = {
    &QWidget::staticMetaObject, "QDoubleSpinBox",
    qtc_props_QDoubleSpinBox, 7, qtc_signals_QDoubleSpinBox, 1
};

QVariant QDoubleSpinBox::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(minimum_);
    case 1: return QVariant(maximum_);
    case 2: return QVariant(value_);
    case 3: return QVariant(singleStep_);
    case 4: return QVariant(decimals_);
    case 5: return QVariant(prefix_);
    case 6: return QVariant(suffix_);
    default: break;
    }
    return QVariant();
}

bool QDoubleSpinBox::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setMinimum(value.toDouble()); return true;
    case 1: setMaximum(value.toDouble()); return true;
    case 2: setValue(value.toDouble()); return true;
    case 3: setSingleStep(value.toDouble()); return true;
    case 4: setDecimals(value.toInt()); return true;
    case 5: setPrefix(value.toString()); return true;
    case 6: setSuffix(value.toString()); return true;
    default: break;
    }
    return false;
}

QDoubleSpinBox::QDoubleSpinBox(QWidget* parent)
    : QWidget(parent), minimum_(0), maximum_(99), value_(0), singleStep_(1), decimals_(2),
      editing_(false), cursorVisible_(false), cursor_(0), blinkTimer_(0) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMinimumSize(80, 26);
}

QDoubleSpinBox::~QDoubleSpinBox() { if (blinkTimer_) killTimer(blinkTimer_); }

void QDoubleSpinBox::setRange(double minimum, double maximum) {
    minimum_ = minimum;
    maximum_ = maximum;
    if (maximum_ < minimum_) maximum_ = minimum_;
    clamp();
}

void QDoubleSpinBox::setValue(double value) {
    double clamped = value;
    if (clamped < minimum_) clamped = minimum_;
    if (clamped > maximum_) clamped = maximum_;
    if (clamped == value_ && !editing_) return;
    value_ = clamped;
    editing_ = false;
    update();
    valueChanged(value_);
}

void QDoubleSpinBox::setDecimals(int decimals) { decimals_ = qBound(0, decimals, 8); update(); }
void QDoubleSpinBox::clamp() { setValue(value_); }
QRect QDoubleSpinBox::editorRect() const { return rect().adjusted(0, 0, -18, 0); }
QRect QDoubleSpinBox::upButtonRect() const { return QRect(width() - 18, 0, 18, height() / 2); }
QRect QDoubleSpinBox::downButtonRect() const { return QRect(width() - 18, height() / 2, 18, height() - height() / 2); }

QString QDoubleSpinBox::formatted() const {
    return prefix_ + QString::number(value_, 'f', decimals_) + suffix_;
}

QString QDoubleSpinBox::text() const {
    if (editing_) return prefix_ + draft_ + suffix_;
    return formatted();
}

QSize QDoubleSpinBox::sizeHint() const { return QSize(100, 26); }

void QDoubleSpinBox::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOptionFrame opt;
    opt.rect = rect();
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    if (hasFocus()) opt.state |= QStyle::State_HasFocus;
    style()->drawLineEditFrame(&p, opt, this);
    p.setPen(QPen(QStyleSheet::resolveColor(this, QPalette::Text)));
    p.setFont(font());
    p.drawText(editorRect().adjusted(8, 0, -4, 0), Qt::AlignLeft | Qt::AlignVCenter, text());
    QRect up = upButtonRect();
    QRect down = downButtonRect();
    p.setPen(QPen(QTheme::colors().border, 1));
    p.drawLine(up.left(), up.top() + 3, up.left(), down.bottom() - 3);
    p.drawLine(up.left(), up.bottom(), width() - 1, up.bottom());
    style()->drawArrow(&p, up.adjusted(4, 3, -4, -3), Qt::UpArrow, QTheme::colors().windowText);
    style()->drawArrow(&p, down.adjusted(4, 3, -4, -3), Qt::DownArrow, QTheme::colors().windowText);
}

void QDoubleSpinBox::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Up) { setValue(value_ + singleStep_); event->accept(); return; }
    if (event->key() == Qt::Key_Down) { setValue(value_ - singleStep_); event->accept(); return; }
    if (!event->text().isEmpty()) {
        wchar_t c = event->text()[0];
        if ((c >= L'0' && c <= L'9') || c == L'.' || c == L'-') {
            if (!editing_) { draft_.clear(); editing_ = true; }
            draft_ += event->text();
            update();
            event->accept();
            return;
        }
    }
    if (event->key() == Qt::Key_Backspace && editing_) {
        draft_ = draft_.left(draft_.size() - 1);
        update();
        event->accept();
        return;
    }
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && editing_) {
        bool ok = false;
        double v = draft_.toDouble(&ok);
        if (ok) setValue(v);
        editing_ = false;
        update();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void QDoubleSpinBox::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    if (upButtonRect().contains(event->pos())) setValue(value_ + singleStep_);
    else if (downButtonRect().contains(event->pos())) setValue(value_ - singleStep_);
    event->accept();
}

void QDoubleSpinBox::wheelEvent(QWheelEvent* event) {
    setValue(value_ + (event->delta() > 0 ? singleStep_ : -singleStep_));
}

void QDoubleSpinBox::focusInEvent(QFocusEvent* event) { QWidget::focusInEvent(event); }
void QDoubleSpinBox::focusOutEvent(QFocusEvent* event) {
    if (editing_) {
        bool ok = false;
        double v = draft_.toDouble(&ok);
        if (ok) setValue(v);
        editing_ = false;
        update();
    }
    QWidget::focusOutEvent(event);
}

// ===========================================================================
// QComboBox
// ===========================================================================
class QComboPopup : public QWidget {
public:
    QComboPopup(QComboBox* combo);
    void setItems(const QStringList& items, int current);
    void updatePosition();
protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void hideEvent(QHideEvent* event);
private:
    int indexAt(const QPoint& pos) const;
    QComboBox* combo_;
    QStringList items_;
    int current_;
    int hovered_;
    int itemHeight_;
};

QComboPopup::QComboPopup(QComboBox* combo)
    : QWidget(0, Qt::Popup), combo_(combo), current_(-1), hovered_(-1), itemHeight_(24) {
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
}

void QComboPopup::setItems(const QStringList& items, int current) {
    items_ = items;
    current_ = current;
    hovered_ = current;
}

void QComboPopup::updatePosition() {
    if (!combo_) return;
    QRect r = combo_->rect();
    QPoint global = combo_->mapToGlobal(QPoint(0, r.height()));
    int visibleItems = qMin(items_.count(), combo_->maxVisibleItems());
    int h = qMax(itemHeight_, visibleItems * itemHeight_ + 2);
    setGeometry(QRect(global, QSize(r.width(), h)));
}

int QComboPopup::indexAt(const QPoint& pos) const {
    int index = (pos.y() - 1) / itemHeight_;
    if (index < 0) return -1;
    if (index >= items_.count()) return -1;
    return index;
}

void QComboPopup::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.menuPopup);
    p.setPen(QPen(c.border, 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
    QFont f = combo_ ? combo_->font() : QApplication::font();
    p.setFont(f);
    for (int i = 0; i < items_.count(); ++i) {
        QRect r(1, 1 + i * itemHeight_, width() - 2, itemHeight_);
        if (i == hovered_) p.fillRect(r, c.selection);
        else if (i == current_) p.fillRect(r, c.surfaceAlt);
        p.setPen(QPen(i == hovered_ ? c.selectionText : c.windowText));
        p.drawText(r.adjusted(10, 0, -6, 0), Qt::AlignLeft | Qt::AlignVCenter, items_[i]);
    }
}

void QComboPopup::mousePressEvent(QMouseEvent* event) {
    int index = indexAt(event->pos());
    if (index >= 0 && combo_) {
        combo_->setCurrentIndex(index);
        combo_->activated(index);
        hide();
    }
    event->accept();
}

void QComboPopup::mouseMoveEvent(QMouseEvent* event) {
    int index = indexAt(event->pos());
    if (index != hovered_) {
        hovered_ = index;
        if (index >= 0 && combo_) combo_->highlighted(index);
        update();
    }
}

void QComboPopup::leaveEvent(QEvent* event) {
    hovered_ = -1;
    update();
    QWidget::leaveEvent(event);
}

void QComboPopup::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        hide();
        return;
    }
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) {
        int next = hovered_ + (event->key() == Qt::Key_Down ? 1 : -1);
        if (next >= 0 && next < items_.count()) {
            hovered_ = next;
            update();
        }
        return;
    }
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && hovered_ >= 0 && combo_) {
        combo_->setCurrentIndex(hovered_);
        combo_->activated(hovered_);
        hide();
        return;
    }
}

void QComboPopup::hideEvent(QHideEvent* event) {
    if (combo_) combo_->hidePopup();
    QWidget::hideEvent(event);
}

static const QMetaObject::PropertyData qtc_props_QComboBox[] = {
    { "currentIndex", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "currentText", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "editable", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "placeholderText", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "maxVisibleItems", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QComboBox[] = {
    { "currentIndexChanged", "currentIndexChanged(int)" },
    { "currentTextChanged", "currentTextChanged(QString)" },
    { "activated", "activated(int)" },
    { "highlighted", "highlighted(int)" }
};
const QMetaObject QComboBox::staticMetaObject = {
    &QWidget::staticMetaObject, "QComboBox",
    qtc_props_QComboBox, 5, qtc_signals_QComboBox, 4
};

QVariant QComboBox::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(currentIndex_);
    case 1: return QVariant(currentText());
    case 2: return QVariant(editable_);
    case 3: return QVariant(placeholder_);
    case 4: return QVariant(maxVisibleItems_);
    default: break;
    }
    return QVariant();
}

bool QComboBox::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setCurrentIndex(value.toInt()); return true;
    case 1:
        if (items_.isEmpty()) addItem(value.toString());
        else setItemText(qMax(0, currentIndex_), value.toString());
        return true;
    case 2: setEditable(value.toBool()); return true;
    case 3: setPlaceholderText(value.toString()); return true;
    case 4: setMaxVisibleItems(value.toInt()); return true;
    default: break;
    }
    return false;
}

QComboBox::QComboBox(QWidget* parent)
    : QWidget(parent), currentIndex_(-1), editable_(false), popupVisible_(false),
      popupIndex_(-1), hoveredIndex_(-1), maxVisibleItems_(10), popup_(0), popupScroll_(0) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setMinimumSize(80, 26);
}

QComboBox::~QComboBox() {
    if (popup_) {
        popup_->hide();
        delete popup_;
        popup_ = 0;
    }
}

void QComboBox::addItem(const QString& text) {
    items_.append(text);
    if (currentIndex_ < 0) {
        currentIndex_ = 0;
        update();
        currentIndexChanged(0);
        currentTextChanged(text);
    }
    update();
}

void QComboBox::addItems(const QStringList& texts) {
    for (int i = 0; i < texts.count(); ++i) addItem(texts[i]);
}

void QComboBox::insertItem(int index, const QString& text) {
    if (index < 0 || index > items_.count()) index = items_.count();
    items_.insert(index, text);
    if (currentIndex_ >= index) ++currentIndex_;
    if (currentIndex_ < 0) currentIndex_ = 0;
    update();
}

void QComboBox::removeItem(int index) {
    if (index < 0 || index >= items_.count()) return;
    items_.removeAt(index);
    if (currentIndex_ >= items_.count()) currentIndex_ = items_.count() - 1;
    update();
    currentIndexChanged(currentIndex_);
}

void QComboBox::clear() {
    items_.clearAll();
    currentIndex_ = -1;
    update();
}

QString QComboBox::itemText(int index) const {
    if (index < 0 || index >= items_.count()) return QString();
    return items_[index];
}

void QComboBox::setItemText(int index, const QString& text) {
    if (index < 0 || index >= items_.count()) return;
    items_[index] = text;
    if (index == currentIndex_) update();
}

void QComboBox::setCurrentIndex(int index) {
    if (index < -1 || index >= items_.count()) return;
    if (currentIndex_ == index) return;
    currentIndex_ = index;
    update();
    currentIndexChanged(index);
    currentTextChanged(currentText());
}

QString QComboBox::currentText() const {
    if (currentIndex_ < 0 || currentIndex_ >= items_.count()) return QString();
    return items_[currentIndex_];
}

void QComboBox::setEditable(bool on) { editable_ = on; update(); }

void QComboBox::showPopup() {
    if (items_.isEmpty()) return;
    if (!popup_) popup_ = new QComboPopup(this);
    QComboPopup* p = (QComboPopup*)popup_;
    p->setItems(items_, currentIndex_);
    p->updatePosition();
    p->show();
    p->raise();
    popupVisible_ = true;
}

void QComboBox::hidePopup() {
    popupVisible_ = false;
    if (popup_ && popup_->isVisible()) popup_->hide();
}

void QComboBox::updatePopup() {
    if (popupVisible_) showPopup();
}

QSize QComboBox::sizeHint() const {
    int w = 80;
    QFontMetrics fm(font());
    for (int i = 0; i < items_.count(); ++i) w = qMax(w, fm.width(items_[i]) + 40);
    return QSize(w, 26);
}

void QComboBox::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOptionComboBox opt;
    opt.rect = rect();
    opt.text = currentText().isEmpty() ? placeholder_ : currentText();
    opt.open = popupVisible_;
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    if (hasFocus()) opt.state |= QStyle::State_HasFocus;
    if (hoveredIndex_ >= 0) opt.state |= QStyle::State_Hover;
    style()->drawComboBox(&p, opt, this);
}

void QComboBox::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    if (popupVisible_) hidePopup();
    else showPopup();
    event->accept();
}

void QComboBox::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Down) {
        setCurrentIndex(qMin(items_.count() - 1, currentIndex_ + 1));
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Up) {
        setCurrentIndex(qMax(0, currentIndex_ - 1));
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space) {
        showPopup();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape && popupVisible_) {
        hidePopup();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void QComboBox::wheelEvent(QWheelEvent* event) {
    if (!hasFocus()) return;
    setCurrentIndex(qBound(0, currentIndex_ + (event->delta() > 0 ? -1 : 1), items_.count() - 1));
}

void QComboBox::focusInEvent(QFocusEvent* event) { QWidget::focusInEvent(event); update(); }
void QComboBox::focusOutEvent(QFocusEvent* event) { QWidget::focusOutEvent(event); update(); }
void QComboBox::enterEvent(QEvent* event) { hoveredIndex_ = 0; update(); QWidget::enterEvent(event); }
void QComboBox::leaveEvent(QEvent* event) { hoveredIndex_ = -1; update(); QWidget::leaveEvent(event); }

// ===========================================================================
// QGroupBox
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QGroupBox[] = {
    { "title", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "checkable", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "checked", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "alignment", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::Alignment" }
};
static const QMetaObject::SignalInfo qtc_signals_QGroupBox[] = {
    { "toggled", "toggled(bool)" }
};
const QMetaObject QGroupBox::staticMetaObject = {
    &QWidget::staticMetaObject, "QGroupBox",
    qtc_props_QGroupBox, 4, qtc_signals_QGroupBox, 1
};

QVariant QGroupBox::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(title_);
    case 1: return QVariant(checkable_);
    case 2: return QVariant(checked_);
    case 3: return QVariant((int)alignment_);
    default: break;
    }
    return QVariant();
}

bool QGroupBox::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setTitle(value.toString()); return true;
    case 1: setCheckable(value.toBool()); return true;
    case 2: setChecked(value.toBool()); return true;
    case 3: setAlignment((Qt::Alignment)value.toInt()); return true;
    default: break;
    }
    return false;
}

QGroupBox::QGroupBox(QWidget* parent)
    : QWidget(parent), checkable_(false), checked_(true), alignment_(Qt::AlignLeft) {}

QGroupBox::QGroupBox(const QString& title, QWidget* parent)
    : QWidget(parent), title_(title), checkable_(false), checked_(true), alignment_(Qt::AlignLeft) {}

QGroupBox::~QGroupBox() {}

int QGroupBox::titleHeight() const {
    QFont f = font();
    f.setBold(true);
    return QFontMetrics(f).height() + 8;
}

QRect QGroupBox::contentsRect() const {
    return rect().adjusted(14, titleHeight(), -14, -12);
}

QSize QGroupBox::sizeHint() const {
    QSize inner(160, 80);
    if (layout()) inner = layout()->sizeHint();
    return QSize(inner.width() + 28, inner.height() + titleHeight() + 12);
}

void QGroupBox::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOptionGroupBox opt;
    opt.rect = rect();
    opt.text = title_;
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    style()->drawGroupBox(&p, opt, this);
    if (checkable_) {
        int cy = titleHeight() / 2;
        QRect box(8, cy - 7, 14, 14);
        QColor border = checked_ ? QTheme::colors().accent : QTheme::colors().border;
        p.setPen(QPen(border, 1));
        p.setBrush(QBrush(checked_ ? QTheme::colors().accent : QTheme::colors().base));
        p.drawRoundedRect(box, 3, 3);
        if (checked_) style()->drawCheckMark(&p, box.adjusted(2, 2, -2, -2), QTheme::colors().accentText);
    }
}

void QGroupBox::mousePressEvent(QMouseEvent* event) {
    if (checkable_) {
        int cy = titleHeight() / 2;
        if (QRect(8, cy - 7, 14, 14).contains(event->pos())) {
            checked_ = !checked_;
            update();
            toggled(checked_);
            QList<QWidget*> kids = childrenWidgets();
            for (int i = 0; i < kids.count(); ++i) {
                kids[i]->setEnabled(checked_);
                kids[i]->update();
            }
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void QGroupBox::resizeEvent(QResizeEvent* event) {
    if (layout()) layout()->setGeometry(contentsRect());
    QWidget::resizeEvent(event);
}

QTC_REGISTER_WIDGET(QLineEdit, "Input", "Line Edit", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QProgressBar, "Display", "Progress Bar", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QSlider, "Input", "Slider", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QScrollBar, "Input", "Scroll Bar", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QSpinBox, "Input", "Spin Box", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QDoubleSpinBox, "Input", "Double Spin Box", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QComboBox, "Input", "Combo Box", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QGroupBox, "Containers", "Group Box", "<qtc/QtWidgets.h>")

// Keeps this translation unit linked when the framework is a static library.
void qtcWidgetAnchor_value() {}
