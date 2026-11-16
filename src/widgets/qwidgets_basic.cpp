// QtClone - qwidgets_basic.cpp : frames, labels, buttons and line edits.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include "qwidget_p.h"
#include <string.h>

// ===========================================================================
// QFrame
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QFrame[] = {
    { "frameShape", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "QFrame::Shape" },
    { "frameShadow", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "QFrame::Shadow" },
    { "lineWidth", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "midLineWidth", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
const QMetaObject QFrame::staticMetaObject = {
    &QWidget::staticMetaObject, "QFrame", qtc_props_QFrame, 4, 0, 0
};

QVariant QFrame::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant((int)shape_);
    case 1: return QVariant((int)shadow_);
    case 2: return QVariant(lineWidth_);
    case 3: return QVariant(midLineWidth_);
    default: break;
    }
    return QVariant();
}

bool QFrame::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setFrameShape((Shape)value.toInt()); return true;
    case 1: setFrameShadow((Shadow)value.toInt()); return true;
    case 2: setLineWidth(value.toInt()); return true;
    case 3: setMidLineWidth(value.toInt()); return true;
    default: break;
    }
    return false;
}

QFrame::QFrame(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), shape_(NoFrame), shadow_(Plain), lineWidth_(1), midLineWidth_(0) {
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QFrame::~QFrame() {}

void QFrame::setFrameShape(Shape shape) { shape_ = shape; update(); }
void QFrame::setFrameShadow(Shadow shadow) { shadow_ = shadow; update(); }
void QFrame::setLineWidth(int width) { lineWidth_ = width; update(); }
void QFrame::setMidLineWidth(int width) { midLineWidth_ = width; update(); }
QRect QFrame::frameRect() const { return rect(); }
QSize QFrame::sizeHint() const { return QSize(100, 30); }

void QFrame::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    QRect r = rect();
    switch (shape_) {
    case HLine:
        p.setPen(QPen(c.border, qMax(1, lineWidth_)));
        p.drawLine(r.left(), r.center().y(), r.right(), r.center().y());
        break;
    case VLine:
        p.setPen(QPen(c.border, qMax(1, lineWidth_)));
        p.drawLine(r.center().x(), r.top(), r.center().x(), r.bottom());
        break;
    case Box:
    case Panel:
    case StyledPanel:
    case WinPanel:
        p.setPen(QPen(c.border, qMax(1, lineWidth_)));
        p.setBrush(QBrush(c.surface));
        p.drawRoundedRect(r.adjusted(0, 0, -1, -1), shape_ == Panel ? 5 : 0, shape_ == Panel ? 5 : 0);
        break;
    default:
        break;
    }
}

// ===========================================================================
// QLabel
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QLabel[] = {
    { "text", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "alignment", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::Alignment" },
    { "wordWrap", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "margin", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "indent", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "pixmap", "QPixmap", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QLabel[] = {
    { "linkActivated", "linkActivated()" },
    { "linkHovered", "linkHovered()" }
};
const QMetaObject QLabel::staticMetaObject = {
    &QFrame::staticMetaObject, "QLabel", qtc_props_QLabel, 6, qtc_signals_QLabel, 2
};

QVariant QLabel::qtcReadProperty(int id) const {
    const int base = QFrame::staticMetaObject.propertyCount();
    if (id < base) return QFrame::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(text_);
    case 1: return QVariant((int)alignment_);
    case 2: return QVariant(wordWrap_);
    case 3: return QVariant(margin_);
    case 4: return QVariant(indent_);
    case 5: return QVariant(pixmap_);
    default: break;
    }
    return QVariant();
}

bool QLabel::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QFrame::staticMetaObject.propertyCount();
    if (id < base) return QFrame::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setText(value.toString()); return true;
    case 1: setAlignment((Qt::Alignment)value.toInt()); return true;
    case 2: setWordWrap(value.toBool()); return true;
    case 3: setMargin(value.toInt()); return true;
    case 4: setIndent(value.toInt()); return true;
    case 5: setPixmap(value.toPixmap()); return true;
    default: break;
    }
    return false;
}

QLabel::QLabel(QWidget* parent, Qt::WindowFlags f)
    : QFrame(parent, f), alignment_(Qt::AlignLeft | Qt::AlignVCenter), wordWrap_(false),
      margin_(0), indent_(-1), selectable_(false), selStart_(0), selEnd_(0),
      selecting_(false), elideWidth_(0) {}

QLabel::QLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
    : QFrame(parent, f), text_(text), alignment_(Qt::AlignLeft | Qt::AlignVCenter),
      wordWrap_(false), margin_(0), indent_(-1), selectable_(false), selStart_(0),
      selEnd_(0), selecting_(false), elideWidth_(0) {}

QLabel::~QLabel() {}

void QLabel::setText(const QString& text) { text_ = text; updateGeometry(); update(); }
void QLabel::setPixmap(const QPixmap& pixmap) { pixmap_ = pixmap; updateGeometry(); update(); }
void QLabel::setAlignment(Qt::Alignment alignment) { alignment_ = alignment; update(); }
void QLabel::setWordWrap(bool on) { wordWrap_ = on; updateGeometry(); update(); }
void QLabel::setMargin(int margin) { margin_ = margin; updateGeometry(); update(); }
void QLabel::setIndent(int indent) { indent_ = indent; update(); }
void QLabel::setTextInteractionFlags(bool selectable) { selectable_ = selectable; }
void QLabel::clear() { text_.clear(); pixmap_ = QPixmap(); updateGeometry(); update(); }
QString QLabel::selectedText() const {
    if (selStart_ == selEnd_) return QString();
    int a = qMin(selStart_, selEnd_);
    int b = qMax(selStart_, selEnd_);
    return text_.mid(a, b - a);
}

QSize QLabel::sizeHint() const {
    QFontMetrics fm(font());
    int w = 0, h = 0;
    if (!pixmap_.isNull()) { w = pixmap_.width(); h = pixmap_.height(); }
    if (!text_.isEmpty()) {
        if (wordWrap_) {
            w = qMax(w, 120);
            h = qMax(h, fm.height() * 2);
        } else {
            w = qMax(w, fm.width(text_));
            h = qMax(h, fm.height());
        }
    }
    return QSize(w + margin_ * 2 + 4 + (indent_ > 0 ? indent_ : 0), h + margin_ * 2 + 2);
}

QSize QLabel::minimumSizeHint() const {
    if (wordWrap_) return QSize(40, QFontMetrics(font()).height() + margin_ * 2);
    return sizeHint();
}

void QLabel::paintEvent(QPaintEvent*) {
    QPainter p(this);
    if (!pixmap_.isNull()) {
        QRect target = rect().adjusted(margin_, margin_, -margin_, -margin_);
        QSize size = pixmap_.size();
        if (size.width() > target.width() || size.height() > target.height())
            size = size.boundedTo(target.size());
        QRect r(target.left() + (target.width() - size.width()) / 2,
                target.top() + (target.height() - size.height()) / 2, size.width(), size.height());
        p.drawPixmap(r, pixmap_);
    }
    if (text_.isEmpty()) return;
    p.setPen(QPen(QStyleSheet::resolveColor(this, QPalette::WindowText)));
    p.setFont(font());
    int flags = (int)alignment_;
    if (!wordWrap_) flags |= Qt::TextSingleLine;
    else flags |= Qt::TextWordWrap;
    QRect textRect = rect().adjusted(margin_ + (indent_ > 0 ? indent_ : 0), margin_, -margin_, -margin_);
    p.drawText(textRect, flags, text_);
}

void QLabel::mousePressEvent(QMouseEvent* event) { QFrame::mousePressEvent(event); }
void QLabel::mouseMoveEvent(QMouseEvent* event) { QFrame::mouseMoveEvent(event); }
void QLabel::mouseReleaseEvent(QMouseEvent* event) { QFrame::mouseReleaseEvent(event); }
void QLabel::mouseDoubleClickEvent(QMouseEvent* event) { QFrame::mouseDoubleClickEvent(event); }
void QLabel::keyPressEvent(QKeyEvent* event) { QFrame::keyPressEvent(event); }

// ===========================================================================
// QAbstractButton
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QAbstractButton[] = {
    { "text", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "icon", "QIcon", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "iconSize", "QSize", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "checkable", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "checked", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "autoExclusive", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "flat", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "default", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QAbstractButton[] = {
    { "clicked", "clicked(bool)" },
    { "pressed", "pressed()" },
    { "released", "released()" },
    { "toggled", "toggled(bool)" }
};
const QMetaObject QAbstractButton::staticMetaObject = {
    &QWidget::staticMetaObject, "QAbstractButton",
    qtc_props_QAbstractButton, 8, qtc_signals_QAbstractButton, 4
};

QVariant QAbstractButton::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(text_);
    case 1: return QVariant(icon_);
    case 2: return QVariant(iconSize_);
    case 3: return QVariant(checkable_);
    case 4: return QVariant(checked_);
    case 5: return QVariant(autoExclusive_);
    case 6: return QVariant(flat_);
    case 7: return QVariant(defaultButton_);
    default: break;
    }
    return QVariant();
}

bool QAbstractButton::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setText(value.toString()); return true;
    case 1: setIcon(value.toIcon()); return true;
    case 2: setIconSize(value.toSize()); return true;
    case 3: setCheckable(value.toBool()); return true;
    case 4: setChecked(value.toBool()); return true;
    case 5: setAutoExclusive(value.toBool()); return true;
    case 6: setFlat(value.toBool()); return true;
    case 7: setDefault(value.toBool()); return true;
    default: break;
    }
    return false;
}

QAbstractButton::QAbstractButton(QWidget* parent)
    : QWidget(parent), iconSize_(16, 16), checkable_(false), checked_(false),
      autoExclusive_(false), down_(false), defaultButton_(false), flat_(false),
      hover_(false), pressInProgress_(false), keyboardPress_(false) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QAbstractButton::~QAbstractButton() {}

void QAbstractButton::setText(const QString& text) { text_ = text; updateGeometry(); update(); }
void QAbstractButton::setIcon(const QIcon& icon) { icon_ = icon; update(); }
void QAbstractButton::setIconSize(const QSize& size) { iconSize_ = size; update(); }

void QAbstractButton::setCheckable(bool on) {
    if (checkable_ == on) return;
    checkable_ = on;
    if (!checkable_ && checked_) setChecked(false);
}

void QAbstractButton::setChecked(bool on) {
    if (checked_ == on) return;
    checked_ = on;
    checkStateSet();
    update();
    toggled(checked_);
    if (checked_ && autoExclusive_ && parentWidget()) {
        QList<QAbstractButton*> siblings = parentWidget()->findChildren<QAbstractButton>();
        for (int i = 0; i < siblings.count(); ++i) {
            QAbstractButton* other = siblings[i];
            if (other != this && other->autoExclusive() && other->isChecked() &&
                strcmp(other->className(), className()) == 0) {
                other->setChecked(false);
            }
        }
    }
}

void QAbstractButton::click() {
    if (!isEnabled()) return;
    if (checkable_) setChecked(!checked_);
    clicked(checked_);
}

void QAbstractButton::toggle() {
    if (!checkable_) {
        checkable_ = true;
    }
    setChecked(!checked_);
}

void QAbstractButton::animateClick(int msec) {
    Q_UNUSED(msec);
    click();
}

QSize QAbstractButton::sizeHint() const {
    QFontMetrics fm(font());
    int w = fm.width(text_) + 28;
    int h = qMax(26, fm.height() + 12);
    if (!icon_.isNull()) w += iconSize_.width() + 6;
    return QSize(w, h);
}

QSize QAbstractButton::minimumSizeHint() const { return sizeHint(); }

void QAbstractButton::drawButton(QPainter* painter) {
    Q_UNUSED(painter);
}

void QAbstractButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    drawButton(&p);
}

void QAbstractButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    if (!rect().contains(event->pos())) { event->ignore(); return; }
    down_ = true;
    pressInProgress_ = true;
    update();
    pressed();
    event->accept();
}

void QAbstractButton::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    bool wasDown = down_;
    down_ = false;
    update();
    if (pressInProgress_) {
        pressInProgress_ = false;
        released();
        if (wasDown && rect().contains(event->pos())) click();
    }
    event->accept();
}

void QAbstractButton::mouseMoveEvent(QMouseEvent* event) {
    bool inside = rect().contains(event->pos());
    if (down_ != inside && pressInProgress_) {
        down_ = inside;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void QAbstractButton::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (!down_) {
            down_ = true;
            keyboardPress_ = true;
            update();
            pressed();
        }
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void QAbstractButton::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (down_ && keyboardPress_) {
            down_ = false;
            keyboardPress_ = false;
            update();
            released();
            click();
        }
        event->accept();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void QAbstractButton::focusInEvent(QFocusEvent* event) { QWidget::focusInEvent(event); update(); }
void QAbstractButton::focusOutEvent(QFocusEvent* event) { QWidget::focusOutEvent(event); update(); }

void QAbstractButton::enterEvent(QEvent* event) {
    hover_ = true;
    update();
    QWidget::enterEvent(event);
}

void QAbstractButton::leaveEvent(QEvent* event) {
    hover_ = false;
    update();
    QWidget::leaveEvent(event);
}

void QAbstractButton::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange) update();
    QWidget::changeEvent(event);
}

void QAbstractButton::checkStateSet() {}

// ===========================================================================
// QPushButton
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QPushButton[] = {
    { "autoDefault", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
const QMetaObject QPushButton::staticMetaObject = {
    &QAbstractButton::staticMetaObject, "QPushButton", qtc_props_QPushButton, 1, 0, 0
};

QVariant QPushButton::qtcReadProperty(int id) const {
    const int base = QAbstractButton::staticMetaObject.propertyCount();
    if (id < base) return QAbstractButton::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(autoDefault_);
    default: break;
    }
    return QVariant();
}

bool QPushButton::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QAbstractButton::staticMetaObject.propertyCount();
    if (id < base) return QAbstractButton::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setAutoDefault(value.toBool()); return true;
    default: break;
    }
    return false;
}

QPushButton::QPushButton(QWidget* parent)
    : QAbstractButton(parent), menu_(0), autoDefault_(false) {
    setFocusPolicy(Qt::StrongFocus);
}

QPushButton::QPushButton(const QString& text, QWidget* parent)
    : QAbstractButton(parent), menu_(0), autoDefault_(false) {
    setText(text);
    setFocusPolicy(Qt::StrongFocus);
}

QPushButton::~QPushButton() {}

void QPushButton::setMenu(QMenu* menu) { menu_ = menu; update(); }

void QPushButton::drawButton(QPainter* painter) {
    QStyleOptionButton opt;
    opt.rect = rect();
    opt.text = text_;
    opt.icon = icon_;
    opt.hasIcon = !icon_.isNull();
    opt.checked = checked_;
    opt.down = down_ || pressInProgress_;
    opt.defaultButton = defaultButton_;
    opt.state = QStyle::State_Enabled | QStyle::State_Raised;
    if (!isEnabled()) opt.state = QStyle::State_None;
    if (down_) opt.state |= QStyle::State_Sunken;
    if (checked_) opt.state |= QStyle::State_On;
    if (hover_) opt.state |= QStyle::State_Hover;
    if (hasFocus()) opt.state |= QStyle::State_HasFocus;
    if (defaultButton_) opt.state |= QStyle::State_Default;
    style()->drawButton(painter, opt, this);
}

void QPushButton::paintEvent(QPaintEvent* event) { QAbstractButton::paintEvent(event); }

// ===========================================================================
// QToolButton
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QToolButton[] = {
    { "toolButtonStyle", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::ToolButtonStyle" },
    { "autoRaise", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
const QMetaObject QToolButton::staticMetaObject = {
    &QAbstractButton::staticMetaObject, "QToolButton", qtc_props_QToolButton, 2, 0, 0
};

QVariant QToolButton::qtcReadProperty(int id) const {
    const int base = QAbstractButton::staticMetaObject.propertyCount();
    if (id < base) return QAbstractButton::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant((int)toolStyle_);
    case 1: return QVariant(autoRaise_);
    default: break;
    }
    return QVariant();
}

bool QToolButton::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QAbstractButton::staticMetaObject.propertyCount();
    if (id < base) return QAbstractButton::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setToolButtonStyle((Qt::ToolButtonStyle)value.toInt()); return true;
    case 1: setAutoRaise(value.toBool()); return true;
    default: break;
    }
    return false;
}

QToolButton::QToolButton(QWidget* parent)
    : QAbstractButton(parent), toolStyle_(Qt::ToolButtonTextBesideIcon), autoRaise_(false) {
    setFocusPolicy(Qt::TabFocus);
}

QToolButton::~QToolButton() {}

void QToolButton::drawButton(QPainter* painter) {
    QStyleOptionToolButton opt;
    opt.rect = rect();
    opt.text = text_;
    opt.icon = icon_;
    opt.hasIcon = !icon_.isNull();
    opt.checked = checked_;
    opt.down = down_;
    opt.toolStyle = toolStyle_;
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    if (hover_) opt.state |= QStyle::State_Hover;
    if (checked_) opt.state |= QStyle::State_On;
    style()->drawToolButton(painter, opt, this);
}

void QToolButton::paintEvent(QPaintEvent* event) { QAbstractButton::paintEvent(event); }

// ===========================================================================
// QCheckBox
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QCheckBox[] = {
    { "checkState", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::CheckState" },
    { "tristate", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QCheckBox[] = {
    { "stateChanged", "stateChanged(int)" }
};
const QMetaObject QCheckBox::staticMetaObject = {
    &QAbstractButton::staticMetaObject, "QCheckBox",
    qtc_props_QCheckBox, 2, qtc_signals_QCheckBox, 1
};

QVariant QCheckBox::qtcReadProperty(int id) const {
    const int base = QAbstractButton::staticMetaObject.propertyCount();
    if (id < base) return QAbstractButton::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant((int)checkState());
    case 1: return QVariant(tristate_);
    default: break;
    }
    return QVariant();
}

bool QCheckBox::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QAbstractButton::staticMetaObject.propertyCount();
    if (id < base) return QAbstractButton::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setCheckState((Qt::CheckState)value.toInt()); return true;
    case 1: setTristate(value.toBool()); return true;
    default: break;
    }
    return false;
}

QCheckBox::QCheckBox(QWidget* parent)
    : QAbstractButton(parent), state_(Qt::Unchecked), tristate_(false) {
    setCheckable(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QCheckBox::QCheckBox(const QString& text, QWidget* parent)
    : QAbstractButton(parent), state_(Qt::Unchecked), tristate_(false) {
    setText(text);
    setCheckable(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QCheckBox::~QCheckBox() {}

Qt::CheckState QCheckBox::checkState() const { return state_; }

void QCheckBox::setCheckState(Qt::CheckState state) {
    if (state_ == state) return;
    state_ = state;
    bool checked = state_ == Qt::Checked;
    if (isChecked() != checked) setChecked(checked);
    update();
    stateChanged((int)state_);
}

void QCheckBox::checkStateSet() {
    state_ = isChecked() ? Qt::Checked : Qt::Unchecked;
}

void QCheckBox::mouseReleaseEvent(QMouseEvent* event) {
    QAbstractButton::mouseReleaseEvent(event);
    if (tristate_) {
        if (state_ == Qt::Unchecked) setCheckState(Qt::PartiallyChecked);
        else if (state_ == Qt::PartiallyChecked) setCheckState(Qt::Checked);
        else setCheckState(Qt::Unchecked);
    } else {
        setCheckState(isChecked() ? Qt::Checked : Qt::Unchecked);
    }
}

void QCheckBox::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        setChecked(!isChecked());
        setCheckState(isChecked() ? Qt::Checked : Qt::Unchecked);
        event->accept();
        return;
    }
    QAbstractButton::keyPressEvent(event);
}

void QCheckBox::drawButton(QPainter* painter) {
    QStyleOptionButton opt;
    opt.rect = rect();
    opt.text = text_;
    opt.checked = isChecked();
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    if (state_ == Qt::PartiallyChecked) opt.state |= QStyle::State_NoChange;
    if (hover_) opt.state |= QStyle::State_Hover;
    if (hasFocus()) opt.state |= QStyle::State_HasFocus;
    style()->drawCheckBox(painter, opt, this);
}

void QCheckBox::paintEvent(QPaintEvent* event) { QAbstractButton::paintEvent(event); }

// ===========================================================================
// QRadioButton
// ===========================================================================
const QMetaObject QRadioButton::staticMetaObject = {
    &QAbstractButton::staticMetaObject, "QRadioButton", 0, 0, 0, 0
};

QVariant QRadioButton::qtcReadProperty(int id) const {
    return QAbstractButton::qtcReadProperty(id);
}

bool QRadioButton::qtcWriteProperty(int id, const QVariant& value) {
    return QAbstractButton::qtcWriteProperty(id, value);
}

QRadioButton::QRadioButton(QWidget* parent) : QAbstractButton(parent) {
    setCheckable(true);
    setAutoExclusive(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QRadioButton::QRadioButton(const QString& text, QWidget* parent) : QAbstractButton(parent) {
    setText(text);
    setCheckable(true);
    setAutoExclusive(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QRadioButton::~QRadioButton() {}

void QRadioButton::mouseReleaseEvent(QMouseEvent* event) {
    bool wasDown = isDown();
    QAbstractButton::mouseReleaseEvent(event);
    if (wasDown && isChecked()) {
        // auto exclusive groups cannot be unchecked by clicking again
    }
}

void QRadioButton::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        setChecked(true);
        event->accept();
        return;
    }
    QAbstractButton::keyPressEvent(event);
}

void QRadioButton::drawButton(QPainter* painter) {
    QStyleOptionButton opt;
    opt.rect = rect();
    opt.text = text_;
    opt.checked = isChecked();
    opt.state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
    if (checked_) opt.state |= QStyle::State_On;
    if (hover_) opt.state |= QStyle::State_Hover;
    if (hasFocus()) opt.state |= QStyle::State_HasFocus;
    style()->drawRadioButton(painter, opt, this);
}

void QRadioButton::paintEvent(QPaintEvent* event) { QAbstractButton::paintEvent(event); }

QTC_REGISTER_WIDGET(QFrame, "Containers", "Frame", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QLabel, "Display", "Label", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QPushButton, "Buttons", "Push Button", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QToolButton, "Buttons", "Tool Button", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QCheckBox, "Buttons", "Check Box", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QRadioButton, "Buttons", "Radio Button", "<qtc/QtWidgets.h>")
