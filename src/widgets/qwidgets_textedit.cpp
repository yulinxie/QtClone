// QtClone - qwidgets_textedit.cpp : a plain multi-line text editor.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include "qwidget_p.h"
#include <string.h>

// ===========================================================================
// QTextEdit
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QTextEdit[] = {
    { "text", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "readOnly", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "wordWrap", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "placeholderText", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QTextEdit[] = {
    { "textChanged", "textChanged()" },
    { "cursorPositionChanged", "cursorPositionChanged()" }
};
const QMetaObject QTextEdit::staticMetaObject = {
    &QWidget::staticMetaObject, "QTextEdit",
    qtc_props_QTextEdit, 4, qtc_signals_QTextEdit, 2
};

QVariant QTextEdit::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(text_);
    case 1: return QVariant(readOnly_);
    case 2: return QVariant(wordWrap_);
    case 3: return QVariant(placeholder_);
    default: break;
    }
    return QVariant();
}

bool QTextEdit::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setPlainText(value.toString()); return true;
    case 1: setReadOnly(value.toBool()); return true;
    case 2: setWordWrap(value.toBool()); return true;
    case 3: setPlaceholderText(value.toString()); return true;
    default: break;
    }
    return false;
}

QTextEdit::QTextEdit(QWidget* parent)
    : QWidget(parent), cursor_(0), anchor_(0), readOnly_(false), wordWrap_(true),
      scrollY_(0), blinkTimer_(0), cursorVisible_(true), selecting_(false),
      lineHeight_(18), vbar_(0) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(120, 80);
    lineHeight_ = QFontMetrics(font()).height() + 2;
    vbar_ = new QScrollBar(Qt::Vertical, this);
    vbar_->hide();
    vbar_->valueChanged.connect(vbar_, this, &QTextEdit::onScrollBarMoved);
    rebuildLayout();
}

QTextEdit::QTextEdit(const QString& text, QWidget* parent)
    : QWidget(parent), text_(text), cursor_(0), anchor_(0), readOnly_(false), wordWrap_(true),
      scrollY_(0), blinkTimer_(0), cursorVisible_(true), selecting_(false),
      lineHeight_(18), vbar_(0) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(120, 80);
    lineHeight_ = QFontMetrics(font()).height() + 2;
    vbar_ = new QScrollBar(Qt::Vertical, this);
    vbar_->hide();
    vbar_->valueChanged.connect(vbar_, this, &QTextEdit::onScrollBarMoved);
    cursor_ = text_.size();
    rebuildLayout();
}

QTextEdit::~QTextEdit() {
    if (blinkTimer_) killTimer(blinkTimer_);
}

void QTextEdit::setPlainText(const QString& text) {
    if (text_ == text) return;
    text_ = text;
    if (cursor_ > text_.size()) cursor_ = text_.size();
    anchor_ = cursor_;
    scrollY_ = 0;
    rebuildLayout();
    updateScrollBar();
    update();
    textChanged();
}

void QTextEdit::insertPlainText(const QString& text) {
    if (readOnly_) return;
    insertText(text);
}

void QTextEdit::append(const QString& text) {
    if (readOnly_) return;
    pushUndo();
    if (!text_.isEmpty() && !text_.endsWith(L"\n")) text_ += L'\n';
    text_ += text;
    cursor_ = text_.size();
    anchor_ = cursor_;
    rebuildLayout();
    updateScrollBar();
    ensureCursorVisible();
    update();
    textChanged();
}

void QTextEdit::clear() {
    if (text_.isEmpty()) return;
    pushUndo();
    text_.clear();
    cursor_ = 0;
    anchor_ = 0;
    scrollY_ = 0;
    rebuildLayout();
    updateScrollBar();
    update();
    textChanged();
}

void QTextEdit::setCursorPosition(int pos) {
    cursor_ = qBound(0, pos, text_.size());
    anchor_ = cursor_;
    ensureCursorVisible();
    update();
    cursorPositionChanged();
}

QString QTextEdit::selectedText() const {
    if (anchor_ == cursor_) return QString();
    int a = qMin(anchor_, cursor_);
    int b = qMax(anchor_, cursor_);
    return text_.mid(a, b - a);
}

void QTextEdit::selectAll() {
    anchor_ = 0;
    cursor_ = text_.size();
    update();
}

void QTextEdit::copy() {
    if (hasSelectedText()) QApplication::clipboard()->setText(selectedText());
}

void QTextEdit::cut() {
    if (readOnly_ || !hasSelectedText()) return;
    QApplication::clipboard()->setText(selectedText());
    pushUndo();
    deleteSelection();
    textChanged();
}

void QTextEdit::paste() {
    if (readOnly_) return;
    insertText(QApplication::clipboard()->text());
}

void QTextEdit::pushUndo() {
    undoStack_.append(text_);
    undoCursors_.append(cursor_);
    if (undoStack_.count() > 100) {
        undoStack_.removeAt(0);
        undoCursors_.removeAt(0);
    }
    redoStack_.clearAll();
    redoCursors_.clearAll();
}

void QTextEdit::undo() {
    if (undoStack_.isEmpty()) return;
    redoStack_.append(text_);
    redoCursors_.append(cursor_);
    text_ = undoStack_.takeLast();
    cursor_ = undoCursors_.takeLast();
    if (cursor_ > text_.size()) cursor_ = text_.size();
    anchor_ = cursor_;
    rebuildLayout();
    updateScrollBar();
    ensureCursorVisible();
    update();
    textChanged();
}

void QTextEdit::redo() {
    if (redoStack_.isEmpty()) return;
    undoStack_.append(text_);
    undoCursors_.append(cursor_);
    text_ = redoStack_.takeLast();
    cursor_ = redoCursors_.takeLast();
    if (cursor_ > text_.size()) cursor_ = text_.size();
    anchor_ = cursor_;
    rebuildLayout();
    updateScrollBar();
    ensureCursorVisible();
    update();
    textChanged();
}

int QTextEdit::contentWidth() const {
    int w = width() - textLeft() * 2;
    if (vbar_ && vbar_->isVisible()) w -= vbar_->width();
    return qMax(20, w);
}

void QTextEdit::rebuildLayout() {
    lines_.clear();
    QFontMetrics fm(font());
    lineHeight_ = fm.height() + 2;
    int maxWidth = contentWidth();

    if (text_.isEmpty()) {
        VisualLine empty;
        empty.start = 0;
        empty.end = 0;
        lines_.append(empty);
        return;
    }

    int paraStart = 0;
    for (int i = 0; i <= text_.size(); ++i) {
        bool end = (i == text_.size());
        if (!end && text_[i] != L'\n') continue;

        if (!wordWrap_ || maxWidth <= 0) {
            VisualLine vl;
            vl.start = paraStart;
            vl.end = i;
            lines_.append(vl);
        } else if (paraStart == i) {
            VisualLine vl;
            vl.start = paraStart;
            vl.end = i;
            lines_.append(vl);
        } else {
            int start = paraStart;
            while (start < i) {
                int w = 0;
                int k = start;
                int lastSpace = -1;
                while (k < i) {
                    wchar_t ch = text_[k];
                    int cw = fm.width(QString(ch));
                    if (w + cw > maxWidth && k > start) break;
                    w += cw;
                    if (ch == L' ') lastSpace = k;
                    ++k;
                }
                int endPos = k;
                if (endPos < i && lastSpace > start && endPos - start > 2) endPos = lastSpace + 1;
                VisualLine vl;
                vl.start = start;
                vl.end = endPos;
                lines_.append(vl);
                start = endPos;
            }
        }
        paraStart = i + 1;
        if (end) break;
    }
    if (lines_.isEmpty()) {
        VisualLine vl;
        vl.start = 0;
        vl.end = 0;
        lines_.append(vl);
    }
}

int QTextEdit::lineIndexAt(int pos) const {
    int last = 0;
    for (int i = 0; i < lines_.count(); ++i) {
        if (pos >= lines_[i].start && pos <= lines_[i].end) {
            last = i;
            if (pos < lines_[i].end) return i;
        }
    }
    return last;
}

int QTextEdit::xForPos(int pos) const {
    int line = lineIndexAt(pos);
    const VisualLine& vl = lines_[line];
    QFontMetrics fm(font());
    return textLeft() + fm.width(text_.mid(vl.start, qMax(0, pos - vl.start)));
}

int QTextEdit::posAtPoint(const QPoint& point) const {
    int y = point.y() - textTop() + scrollY_;
    int line = y / qMax(1, lineHeight_);
    if (line < 0) line = 0;
    if (line >= lines_.count()) line = lines_.count() - 1;
    const VisualLine& vl = lines_[line];
    QFontMetrics fm(font());
    int target = point.x() - textLeft();
    int best = vl.start;
    for (int i = vl.start; i < vl.end; ++i) {
        int w = fm.width(text_.mid(vl.start, i - vl.start + 1));
        if (w <= target) best = i + 1;
        else break;
    }
    return best;
}

QRect QTextEdit::caretRect() const {
    int line = lineIndexAt(cursor_);
    int y = textTop() + line * lineHeight_ - scrollY_;
    return QRect(xForPos(cursor_), y, 1, lineHeight_);
}

void QTextEdit::ensureCursorVisible() {
    if (!vbar_) return;
    int line = lineIndexAt(cursor_);
    int top = line * lineHeight_ - scrollY_;
    int viewH = height() - textTop() * 2;
    if (top < 0) {
        scrollY_ += top;
        vbar_->setValue(scrollY_);
    } else if (top + lineHeight_ > viewH) {
        scrollY_ += top + lineHeight_ - viewH;
        vbar_->setValue(scrollY_);
    }
    if (scrollY_ < 0) scrollY_ = 0;
}

void QTextEdit::updateScrollBar() {
    if (!vbar_) return;
    int contentH = lines_.count() * lineHeight_ + textTop() * 2;
    int viewH = height();
    if (contentH > viewH) {
        vbar_->setGeometry(width() - vbar_->sizeHint().width(), 0,
                           vbar_->sizeHint().width(), height());
        vbar_->setRange(0, contentH - viewH);
        vbar_->setPageStep(viewH);
        vbar_->show();
    } else {
        vbar_->hide();
        scrollY_ = 0;
        vbar_->setValue(0);
    }
    if (scrollY_ > vbar_->maximum()) scrollY_ = vbar_->maximum();
    update();
}

void QTextEdit::setCursor(int pos, bool keepAnchor) {
    cursor_ = qBound(0, pos, text_.size());
    if (!keepAnchor) anchor_ = cursor_;
    ensureCursorVisible();
    update();
    cursorPositionChanged();
}

void QTextEdit::deleteSelection() {
    if (anchor_ == cursor_) return;
    int a = qMin(anchor_, cursor_);
    int b = qMax(anchor_, cursor_);
    text_ = text_.left(a) + text_.mid(b);
    cursor_ = a;
    anchor_ = a;
    rebuildLayout();
    updateScrollBar();
    update();
}

void QTextEdit::insertText(const QString& text) {
    if (readOnly_ || text.isEmpty()) return;
    pushUndo();
    if (anchor_ != cursor_) {
        int a = qMin(anchor_, cursor_);
        int b = qMax(anchor_, cursor_);
        text_ = text_.left(a) + text_.mid(b);
        cursor_ = a;
        anchor_ = a;
    }
    text_ = text_.left(cursor_) + text + text_.mid(cursor_);
    cursor_ += text.size();
    anchor_ = cursor_;
    imePreedit_.clear();
    rebuildLayout();
    updateScrollBar();
    ensureCursorVisible();
    update();
    textChanged();
    cursorPositionChanged();
}

void QTextEdit::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOptionFrame opt;
    opt.rect = rect();
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    if (hasFocus()) opt.state |= QStyle::State_HasFocus;
    style()->drawLineEditFrame(&p, opt, this);

    QRect content(textLeft(), textTop(), contentWidth(), height() - textTop() * 2);
    p.setClipRect(content);
    p.setFont(font());
    QFontMetrics fm(font());

    if (text_.isEmpty() && !placeholder_.isEmpty()) {
        p.setPen(QPen(QTheme::colors().disabledText));
        p.drawText(QRect(content.left(), textTop(), content.width(), lineHeight_),
                   Qt::AlignLeft | Qt::AlignVCenter, placeholder_);
    }

    if (anchor_ != cursor_) {
        int a = qMin(anchor_, cursor_);
        int b = qMax(anchor_, cursor_);
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QTheme::colors().selection));
        for (int i = 0; i < lines_.count(); ++i) {
            int s = qMax(a, lines_[i].start);
            int e = qMin(b, lines_[i].end);
            if (s >= e) continue;
            int y = textTop() + i * lineHeight_ - scrollY_;
            if (y + lineHeight_ < 0 || y > height()) continue;
            int x1 = xForPos(s);
            int x2 = xForPos(e);
            p.drawRect(QRect(x1, y, x2 - x1, lineHeight_));
        }
    }

    p.setPen(QPen(QStyleSheet::resolveColor(this, QPalette::Text)));
    for (int i = 0; i < lines_.count(); ++i) {
        int y = textTop() + i * lineHeight_ - scrollY_;
        if (y + lineHeight_ < 0) continue;
        if (y > height()) break;
        const VisualLine& vl = lines_[i];
        if (vl.end <= vl.start) continue;
        p.drawText(QRect(textLeft(), y, content.width(), lineHeight_),
                   Qt::AlignLeft | Qt::AlignVCenter, text_.mid(vl.start, vl.end - vl.start));
    }

    if (!imePreedit_.isEmpty()) {
        QRect caret = caretRect();
        p.setPen(QPen(QTheme::colors().accent));
        p.drawText(QRect(caret.left(), caret.top(), content.width(), lineHeight_),
                   Qt::AlignLeft | Qt::AlignVCenter, imePreedit_);
        int w = fm.width(imePreedit_);
        p.drawLine(caret.left(), caret.bottom(), caret.left() + w, caret.bottom());
    }

    if (hasFocus() && cursorVisible_ && anchor_ == cursor_) {
        QRect caret = caretRect();
        p.setPen(QPen(QTheme::colors().windowText, 1));
        p.drawLine(caret.left(), caret.top() + 2, caret.left(), caret.bottom() - 2);
    }
    p.setClipping(false);
}

void QTextEdit::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    selecting_ = true;
    int pos = posAtPoint(event->pos());
    setCursor(pos, event->modifiers() & Qt::ShiftModifier);
    startBlinkTimer();
    update();
}

void QTextEdit::mouseMoveEvent(QMouseEvent* event) {
    if (!selecting_) return;
    setCursor(posAtPoint(event->pos()), true);
}

void QTextEdit::mouseReleaseEvent(QMouseEvent*) { selecting_ = false; }

void QTextEdit::mouseDoubleClickEvent(QMouseEvent* event) {
    int pos = posAtPoint(event->pos());
    int start = pos;
    int end = pos;
    while (start > 0) {
        wchar_t c = text_[start - 1];
        if (!((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9') || c == L'_'))
            break;
        --start;
    }
    while (end < text_.size()) {
        wchar_t c = text_[end];
        if (!((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9') || c == L'_'))
            break;
        ++end;
    }
    anchor_ = start;
    cursor_ = end;
    update();
}

void QTextEdit::startBlinkTimer() {
    if (blinkTimer_) killTimer(blinkTimer_);
    cursorVisible_ = true;
    blinkTimer_ = startTimer(500);
}

void QTextEdit::onScrollBarMoved(int value) {
    scrollY_ = value;
    update();
}

void QTextEdit::timerEvent(QTimerEvent* event) {
    if (event->timerId() == blinkTimer_) {
        cursorVisible_ = !cursorVisible_;
        update();
        return;
    }
    QWidget::timerEvent(event);
}

void QTextEdit::keyPressEvent(QKeyEvent* event) {
    bool shift = event->modifiers() & Qt::ShiftModifier;
    bool ctrl = event->modifiers() & Qt::ControlModifier;
    switch (event->key()) {
    case Qt::Key_Left:
        setCursor(cursor_ - 1, shift);
        return;
    case Qt::Key_Right:
        setCursor(cursor_ + 1, shift);
        return;
    case Qt::Key_Up: {
        int line = lineIndexAt(cursor_);
        if (line > 0) {
            int x = xForPos(cursor_);
            int newLine = line - 1;
            int best = lines_[newLine].start;
            QFontMetrics fm(font());
            for (int i = lines_[newLine].start; i <= lines_[newLine].end; ++i) {
                if (textLeft() + fm.width(text_.mid(lines_[newLine].start, i - lines_[newLine].start)) <= x) best = i;
                else break;
            }
            setCursor(best, shift);
        }
        return;
    }
    case Qt::Key_Down: {
        int line = lineIndexAt(cursor_);
        if (line + 1 < lines_.count()) {
            int x = xForPos(cursor_);
            int newLine = line + 1;
            int best = lines_[newLine].start;
            QFontMetrics fm(font());
            for (int i = lines_[newLine].start; i <= lines_[newLine].end; ++i) {
                if (textLeft() + fm.width(text_.mid(lines_[newLine].start, i - lines_[newLine].start)) <= x) best = i;
                else break;
            }
            setCursor(best, shift);
        }
        return;
    }
    case Qt::Key_Home: {
        int line = lineIndexAt(cursor_);
        setCursor(lines_[line].start, shift);
        return;
    }
    case Qt::Key_End: {
        int line = lineIndexAt(cursor_);
        setCursor(lines_[line].end, shift);
        return;
    }
    case Qt::Key_PageUp:
        setCursor(cursor_ - 10, shift);
        return;
    case Qt::Key_PageDown:
        setCursor(cursor_ + 10, shift);
        return;
    case Qt::Key_Backspace: {
        if (readOnly_) return;
        if (anchor_ != cursor_) {
            pushUndo();
            deleteSelection();
            textChanged();
        } else if (cursor_ > 0) {
            pushUndo();
            text_ = text_.left(cursor_ - 1) + text_.mid(cursor_);
            --cursor_;
            anchor_ = cursor_;
            rebuildLayout();
            updateScrollBar();
            ensureCursorVisible();
            update();
            textChanged();
        }
        return;
    }
    case Qt::Key_Delete: {
        if (readOnly_) return;
        if (anchor_ != cursor_) {
            pushUndo();
            deleteSelection();
            textChanged();
        } else if (cursor_ < text_.size()) {
            pushUndo();
            text_ = text_.left(cursor_) + text_.mid(cursor_ + 1);
            rebuildLayout();
            updateScrollBar();
            update();
            textChanged();
        }
        return;
    }
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (!readOnly_) insertText(L"\n");
        return;
    case Qt::Key_Tab:
        event->ignore();
        return;
    case Qt::Key_A:
        if (ctrl) { selectAll(); return; }
        break;
    case Qt::Key_C:
        if (ctrl) { copy(); return; }
        break;
    case Qt::Key_X:
        if (ctrl) { cut(); return; }
        break;
    case Qt::Key_V:
        if (ctrl) { paste(); return; }
        break;
    case Qt::Key_Z:
        if (ctrl) {
            if (shift) redo();
            else undo();
            return;
        }
        break;
    case Qt::Key_Y:
        if (ctrl) { redo(); return; }
        break;
    default:
        break;
    }
    if (!event->text().isEmpty() && !ctrl) {
        insertText(event->text());
        return;
    }
    QWidget::keyPressEvent(event);
}

void QTextEdit::wheelEvent(QWheelEvent* event) {
    if (!vbar_ || !vbar_->isVisible()) return;
    vbar_->setValue(vbar_->value() - (event->delta() > 0 ? lineHeight_ * 3 : -lineHeight_ * 3));
}

void QTextEdit::focusInEvent(QFocusEvent* event) {
    QWidget::focusInEvent(event);
    startBlinkTimer();
    update();
}

void QTextEdit::focusOutEvent(QFocusEvent* event) {
    if (blinkTimer_) {
        killTimer(blinkTimer_);
        blinkTimer_ = 0;
    }
    cursorVisible_ = false;
    imePreedit_.clear();
    QWidget::focusOutEvent(event);
    update();
}

void QTextEdit::inputMethodEvent(QInputMethodEvent* event) {
    if (!event->commitString().isEmpty()) {
        insertText(event->commitString());
        imePreedit_.clear();
    }
    if (!event->preeditString().isEmpty()) imePreedit_ = event->preeditString();
    else imePreedit_.clear();
    update();
}

void QTextEdit::resizeEvent(QResizeEvent* event) {
    rebuildLayout();
    updateScrollBar();
    QWidget::resizeEvent(event);
}

QSize QTextEdit::sizeHint() const { return QSize(320, 180); }

QTC_REGISTER_WIDGET(QTextEdit, "Input", "Text Edit", "<qtc/QtWidgets.h>")

// Keeps this translation unit linked when the framework is a static library.
void qtcWidgetAnchor_textedit() {}
