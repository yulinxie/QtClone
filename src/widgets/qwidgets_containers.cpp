// QtClone - qwidgets_containers.cpp : tab widget, scroll area, splitter, stacked widget.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include "qwidget_p.h"

// ===========================================================================
// QTabWidget
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QTabWidget[] = {
    { "currentIndex", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "tabPosition", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "QTabWidget::TabPosition" },
    { "tabsClosable", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "documentMode", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QTabWidget[] = {
    { "currentChanged", "currentChanged(int)" },
    { "tabCloseRequested", "tabCloseRequested(int)" },
    { "tabBarClicked", "tabBarClicked(int)" }
};
const QMetaObject QTabWidget::staticMetaObject = {
    &QWidget::staticMetaObject, "QTabWidget",
    qtc_props_QTabWidget, 4, qtc_signals_QTabWidget, 3
};

QVariant QTabWidget::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(currentIndex_);
    case 1: return QVariant((int)position_);
    case 2: return QVariant(closable_);
    case 3: return QVariant(documentMode_);
    default: break;
    }
    return QVariant();
}

bool QTabWidget::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setCurrentIndex(value.toInt()); return true;
    case 1: setTabPosition((TabPosition)value.toInt()); return true;
    case 2: setTabsClosable(value.toBool()); return true;
    case 3: setDocumentMode(value.toBool()); return true;
    default: break;
    }
    return false;
}

QTabWidget::QTabWidget(QWidget* parent)
    : QWidget(parent), currentIndex_(-1), currentWidget_(0), position_(North),
      closable_(false), documentMode_(false), hoverIndex_(-1), closePressed_(-1) {}

QTabWidget::~QTabWidget() {}

int QTabWidget::addTab(QWidget* page, const QString& label) {
    return insertTab(pages_.count(), page, label);
}

int QTabWidget::insertTab(int index, QWidget* page, const QString& label) {
    if (!page) return -1;
    if (index < 0 || index > pages_.count()) index = pages_.count();
    page->setParentWidget(this);
    pages_.insert(index, page);
    labels_.insert(index, label);
    enabled_.insert(index, 1);
    page->hide();
    if (currentIndex_ < 0) {
        currentIndex_ = index;
        currentWidget_ = page;
        page->show();
    } else if (index <= currentIndex_) {
        ++currentIndex_;
    }
    updatePages();
    updateGeometry();
    update();
    return index;
}

void QTabWidget::removeTab(int index) {
    if (index < 0 || index >= pages_.count()) return;
    QWidget* page = pages_[index];
    pages_.removeAt(index);
    labels_.removeAt(index);
    enabled_.removeAt(index);
    if (page) {
        page->hide();
        page->setParentWidget(0);
    }
    if (currentIndex_ >= pages_.count()) currentIndex_ = pages_.count() - 1;
    if (currentIndex_ >= 0) currentWidget_ = pages_[currentIndex_];
    else currentWidget_ = 0;
    updatePages();
    update();
}

void QTabWidget::clear() {
    while (!pages_.isEmpty()) removeTab(pages_.count() - 1);
    currentIndex_ = -1;
    currentWidget_ = 0;
}

QWidget* QTabWidget::widget(int index) const {
    if (index < 0 || index >= pages_.count()) return 0;
    return pages_[index];
}

QString QTabWidget::tabText(int index) const {
    if (index < 0 || index >= labels_.count()) return QString();
    return labels_[index];
}

void QTabWidget::setTabText(int index, const QString& text) {
    if (index < 0 || index >= labels_.count()) return;
    labels_[index] = text;
    update();
}

bool QTabWidget::isTabEnabled(int index) const {
    if (index < 0 || index >= enabled_.count()) return false;
    return enabled_[index];
}

void QTabWidget::setTabEnabled(int index, bool enabled) {
    if (index < 0 || index >= enabled_.count()) return;
    enabled_[index] = enabled;
    update();
}

int QTabWidget::tabBarHeight() const {
    QFontMetrics fm(font());
    return qMax(30, fm.height() + 14);
}

QRect QTabWidget::tabRect(int index) const {
    if (index < 0 || index >= labels_.count()) return QRect();
    int h = tabBarHeight();
    int x = 4;
    QFontMetrics fm(font());
    for (int i = 0; i < index; ++i) {
        x += fm.width(labels_[i]) + 34;
        if (closable_) x += 16;
    }
    int w = fm.width(labels_[index]) + 34 + (closable_ ? 16 : 0);
    return QRect(x, 4, w, h - 4);
}

int QTabWidget::tabAt(const QPoint& pos) const {
    for (int i = 0; i < labels_.count(); ++i)
        if (tabRect(i).contains(pos)) return i;
    return -1;
}

QRect QTabWidget::pageRect() const {
    int h = tabBarHeight();
    return rect().adjusted(0, h, 0, 0);
}

void QTabWidget::setCurrentIndex(int index) {
    if (index < -1 || index >= pages_.count()) return;
    if (index == currentIndex_) return;
    if (currentIndex_ >= 0 && currentIndex_ < pages_.count() && pages_[currentIndex_])
        pages_[currentIndex_]->hide();
    currentIndex_ = index;
    currentWidget_ = (index >= 0) ? pages_[index] : 0;
    updatePages();
    update();
    currentChanged(index);
}

void QTabWidget::setCurrentWidget(QWidget* page) {
    int index = pages_.indexOf(page);
    if (index >= 0) setCurrentIndex(index);
}

void QTabWidget::updatePages() {
    QRect area = pageRect();
    for (int i = 0; i < pages_.count(); ++i) {
        QWidget* page = pages_[i];
        if (!page) continue;
        if (i == currentIndex_) {
            page->setGeometry(area);
            page->show();
        } else {
            page->hide();
        }
    }
}

void QTabWidget::showEvent(QShowEvent* event) {
    updatePages();
    QWidget::showEvent(event);
}

void QTabWidget::resizeEvent(QResizeEvent* event) {
    updatePages();
    QWidget::resizeEvent(event);
}

void QTabWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.window);
    int h = tabBarHeight();
    p.fillRect(QRect(0, 0, width(), h), c.surface);
    p.setPen(QPen(c.border, 1));
    p.drawLine(0, h - 1, width(), h - 1);

    QRect area = pageRect();
    p.setPen(QPen(c.border, 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(area.adjusted(0, 0, -1, -1));

    for (int i = 0; i < labels_.count(); ++i) {
        QStyleOptionTab opt;
        opt.rect = tabRect(i);
        opt.text = labels_[i];
        opt.selected = (i == currentIndex_);
        opt.hovered = (i == hoverIndex_);
        if (i == 0 && labels_.count() == 1) opt.position = QStyleOptionTab::OnlyOne;
        else if (i == 0) opt.position = QStyleOptionTab::Beginning;
        else if (i == labels_.count() - 1) opt.position = QStyleOptionTab::End;
        else opt.position = QStyleOptionTab::Middle;
        QRect textRect = opt.rect;
        if (closable_) textRect.setWidth(textRect.width() - 16);
        QStyleOptionTab textOpt = opt;
        textOpt.rect = textRect;
        style()->drawTab(&p, textOpt, this);
        if (closable_) {
            QRect closeRect(opt.rect.right() - 18, opt.rect.center().y() - 5, 10, 10);
            p.setPen(QPen(i == closePressed_ ? c.accent : c.disabledText, 1));
            p.drawLine(closeRect.left(), closeRect.top(), closeRect.right(), closeRect.bottom());
            p.drawLine(closeRect.right(), closeRect.top(), closeRect.left(), closeRect.bottom());
        }
    }
}

void QTabWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    int index = tabAt(event->pos());
    if (index >= 0) {
        if (closable_) {
            QRect closeRect(tabRect(index).right() - 18, tabRect(index).center().y() - 5, 10, 10);
            if (closeRect.contains(event->pos())) {
                closePressed_ = index;
                update();
                return;
            }
        }
        tabBarClicked(index);
        if (enabled_[index]) setCurrentIndex(index);
    }
    event->accept();
}

void QTabWidget::mouseMoveEvent(QMouseEvent* event) {
    int index = tabAt(event->pos());
    if (index != hoverIndex_) {
        hoverIndex_ = index;
        update();
    }
}

void QTabWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (closePressed_ >= 0) {
        int index = tabAt(event->pos());
        if (index == closePressed_) tabCloseRequested(index);
        closePressed_ = -1;
        update();
    }
}

void QTabWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Left) {
        if (currentIndex_ > 0) setCurrentIndex(currentIndex_ - 1);
        return;
    }
    if (event->key() == Qt::Key_Right) {
        if (currentIndex_ + 1 < pages_.count()) setCurrentIndex(currentIndex_ + 1);
        return;
    }
    QWidget::keyPressEvent(event);
}

QSize QTabWidget::sizeHint() const {
    int w = 240, h = 180;
    for (int i = 0; i < pages_.count(); ++i) {
        QSize s = pages_[i]->sizeHint();
        w = qMax(w, s.width());
        h = qMax(h, s.height() + tabBarHeight());
    }
    return QSize(w, h);
}

// ===========================================================================
// QScrollArea
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QScrollArea[] = {
    { "widgetResizable", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "horizontalScrollBarPolicy", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::ScrollBarPolicy" },
    { "verticalScrollBarPolicy", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::ScrollBarPolicy" }
};
const QMetaObject QScrollArea::staticMetaObject = {
    &QWidget::staticMetaObject, "QScrollArea", qtc_props_QScrollArea, 3, 0, 0
};

QVariant QScrollArea::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(resizable_);
    case 1: return QVariant((int)hPolicy_);
    case 2: return QVariant((int)vPolicy_);
    default: break;
    }
    return QVariant();
}

bool QScrollArea::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setWidgetResizable(value.toBool()); return true;
    case 1: setHorizontalScrollBarPolicy((Qt::ScrollBarPolicy)value.toInt()); return true;
    case 2: setVerticalScrollBarPolicy((Qt::ScrollBarPolicy)value.toInt()); return true;
    default: break;
    }
    return false;
}

QScrollArea::QScrollArea(QWidget* parent)
    : QWidget(parent), content_(0), hbar_(0), vbar_(0), resizable_(false),
      hPolicy_(Qt::ScrollBarAsNeeded), vPolicy_(Qt::ScrollBarAsNeeded) {
    hbar_ = new QScrollBar(Qt::Horizontal, this);
    vbar_ = new QScrollBar(Qt::Vertical, this);
    hbar_->hide();
    vbar_->hide();
    hbar_->valueChanged.connect(this, this, &QScrollArea::updateBars);
    vbar_->valueChanged.connect(this, this, &QScrollArea::updateBars);
}

QScrollArea::~QScrollArea() {}

void QScrollArea::setWidget(QWidget* widget) {
    if (content_ == widget) return;
    if (content_) {
        content_->hide();
        content_->setParentWidget(0);
    }
    content_ = widget;
    if (content_) {
        content_->setParentWidget(this);
        content_->show();
    }
    updateBars();
    update();
}

QWidget* QScrollArea::takeWidget() {
    QWidget* w = content_;
    content_ = 0;
    if (w) w->setParentWidget(0);
    updateBars();
    return w;
}

void QScrollArea::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy) { hPolicy_ = policy; updateBars(); }
void QScrollArea::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy) { vPolicy_ = policy; updateBars(); }

QRect QScrollArea::viewportRect() const {
    QRect r = rect();
    bool needV = vbar_ && vbar_->isVisible();
    bool needH = hbar_ && hbar_->isVisible();
    if (needV) r.setRight(r.right() - vbar_->width());
    if (needH) r.setBottom(r.bottom() - hbar_->height());
    return r;
}

void QScrollArea::updateBars() {
    if (!content_) {
        if (hbar_) hbar_->hide();
        if (vbar_) vbar_->hide();
        return;
    }
    QSize contentSize = content_->size();
    if (resizable_) {
        contentSize = QSize(qMax(content_->sizeHint().width(), viewportRect().width()),
                            qMax(content_->sizeHint().height(), viewportRect().height()));
        content_->resize(contentSize);
    }

    bool needV = (contentSize.height() > height()) && vPolicy_ != Qt::ScrollBarAlwaysOff;
    bool needH = (contentSize.width() > width()) && hPolicy_ != Qt::ScrollBarAlwaysOff;

    if (vPolicy_ == Qt::ScrollBarAlwaysOn) needV = true;
    if (hPolicy_ == Qt::ScrollBarAlwaysOn) needH = true;

    if (hbar_) {
        if (needH) {
            hbar_->setGeometry(0, height() - hbar_->sizeHint().height(), width() - (needV ? vbar_->sizeHint().width() : 0), hbar_->sizeHint().height());
            hbar_->setRange(0, qMax(0, contentSize.width() - viewportRect().width()));
            hbar_->setPageStep(qMax(1, viewportRect().width()));
            hbar_->show();
        } else {
            hbar_->hide();
            hbar_->setValue(0);
        }
    }
    if (vbar_) {
        if (needV) {
            vbar_->setGeometry(width() - vbar_->sizeHint().width(), 0, vbar_->sizeHint().width(), height() - (needH ? hbar_->sizeHint().height() : 0));
            vbar_->setRange(0, qMax(0, contentSize.height() - viewportRect().height()));
            vbar_->setPageStep(qMax(1, viewportRect().height()));
            vbar_->show();
        } else {
            vbar_->hide();
            vbar_->setValue(0);
        }
    }

    int dx = hbar_ && hbar_->isVisible() ? -hbar_->value() : 0;
    int dy = vbar_ && vbar_->isVisible() ? -vbar_->value() : 0;
    content_->move(dx, dy);
    update();
}

void QScrollArea::resizeEvent(QResizeEvent* event) {
    updateBars();
    QWidget::resizeEvent(event);
}

void QScrollArea::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QStyleSheet::resolveColor(this, QPalette::Base));
    if (!content_ && QStyleSheet::boxFor(this).hasBorder) {
        p.setPen(QPen(QTheme::colors().border, 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }
}

void QScrollArea::wheelEvent(QWheelEvent* event) {
    if (vbar_ && vbar_->isVisible()) {
        vbar_->setValue(vbar_->value() - (event->delta() > 0 ? 40 : -40));
    } else if (hbar_ && hbar_->isVisible()) {
        hbar_->setValue(hbar_->value() - (event->delta() > 0 ? 40 : -40));
    }
}

QSize QScrollArea::sizeHint() const {
    return content_ ? QSize(qMin(400, content_->width()), qMin(300, content_->height())) : QSize(200, 150);
}

// ===========================================================================
// QSplitter
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QSplitter[] = {
    { "orientation", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, "Qt::Orientation" },
    { "handleWidth", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QSplitter[] = {
    { "splitterMoved", "splitterMoved(int,int)" }
};
const QMetaObject QSplitter::staticMetaObject = {
    &QWidget::staticMetaObject, "QSplitter",
    qtc_props_QSplitter, 2, qtc_signals_QSplitter, 1
};

QVariant QSplitter::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant((int)orientation_);
    case 1: return QVariant(handleWidth_);
    default: break;
    }
    return QVariant();
}

bool QSplitter::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setOrientation((Qt::Orientation)value.toInt()); return true;
    case 1: setHandleWidth(value.toInt()); return true;
    default: break;
    }
    return false;
}

QSplitter::QSplitter(QWidget* parent)
    : QWidget(parent), orientation_(Qt::Horizontal), handleWidth_(5),
      draggingHandle_(-1), dragStartPos_(0) {}

QSplitter::QSplitter(Qt::Orientation orientation, QWidget* parent)
    : QWidget(parent), orientation_(orientation), handleWidth_(5),
      draggingHandle_(-1), dragStartPos_(0) {}

QSplitter::~QSplitter() {}

void QSplitter::addWidget(QWidget* widget) { insertWidget(panels_.count(), widget); }

void QSplitter::insertWidget(int index, QWidget* widget) {
    if (!widget) return;
    if (index < 0 || index > panels_.count()) index = panels_.count();
    widget->setParentWidget(this);
    panels_.insert(index, widget);
    stretches_.insert(index, 1);
    collapsible_.insert(index, 1);
    widget->show();
    normalizeSizes();
    applySizes();
    update();
}

QWidget* QSplitter::widget(int index) const {
    if (index < 0 || index >= panels_.count()) return 0;
    return panels_[index];
}

void QSplitter::setSizes(const QList<int>& sizes) {
    sizes_ = sizes;
    normalizeSizes();
    applySizes();
}

void QSplitter::setStretchFactor(int index, int stretch) {
    if (index >= 0 && index < stretches_.count()) stretches_[index] = stretch;
}

bool QSplitter::isCollapsible(int index) const {
    return index >= 0 && index < collapsible_.count() && collapsible_[index];
}

void QSplitter::setCollapsible(int index, bool collapsible) {
    if (index >= 0 && index < collapsible_.count()) collapsible_[index] = collapsible;
}

void QSplitter::normalizeSizes() {
    if (sizes_.count() != panels_.count()) {
        sizes_.clearAll();
        for (int i = 0; i < panels_.count(); ++i) {
            if (orientation_ == Qt::Horizontal) sizes_.append(panels_[i]->width() > 0 ? panels_[i]->width() : 100);
            else sizes_.append(panels_[i]->height() > 0 ? panels_[i]->height() : 100);
        }
    }
}

int QSplitter::handleAt(const QPoint& pos) const {
    for (int i = 0; i < panels_.count() - 1; ++i) {
        if (handleRect(i).contains(pos)) return i;
    }
    return -1;
}

QRect QSplitter::handleRect(int index) const {
    if (index < 0 || index >= panels_.count() - 1) return QRect();
    int pos = 0;
    for (int i = 0; i <= index; ++i) pos += sizes_[i] + handleWidth_;
    pos -= handleWidth_;
    if (orientation_ == Qt::Horizontal) return QRect(pos, 0, handleWidth_, height());
    return QRect(0, pos, width(), handleWidth_);
}

void QSplitter::applySizes() {
    if (panels_.isEmpty()) return;
    int total = orientation_ == Qt::Horizontal ? width() : height();
    int handles = handleWidth_ * (panels_.count() - 1);
    int available = total - handles;
    int sum = 0;
    for (int i = 0; i < sizes_.count(); ++i) sum += sizes_[i];
    if (sum <= 0) return;
    int pos = 0, used = 0;
    for (int i = 0; i < panels_.count(); ++i) {
        int size = (i == panels_.count() - 1) ? available - used : available * sizes_[i] / sum;
        used += size;
        if (orientation_ == Qt::Horizontal) panels_[i]->setGeometry(QRect(pos, 0, size, height()));
        else panels_[i]->setGeometry(QRect(0, pos, width(), size));
        pos += size + handleWidth_;
    }
}

void QSplitter::resizeEvent(QResizeEvent* event) {
    applySizes();
    QWidget::resizeEvent(event);
}

void QSplitter::paintEvent(QPaintEvent*) {
    const QTheme::Colors& c = QTheme::colors();
    QPainter p(this);
    for (int i = 0; i < panels_.count() - 1; ++i) {
        QRect r = handleRect(i);
        p.fillRect(r, c.window);
        QRect grip(r.center().x() - 1, r.center().y() - 8, 2, 16);
        if (orientation_ == Qt::Horizontal) grip = QRect(r.center().x() - 1, r.center().y() - 8, 2, 16);
        else grip = QRect(r.center().x() - 8, r.center().y() - 1, 16, 2);
        p.fillRect(grip, c.borderStrong);
    }
}

void QSplitter::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    int handle = handleAt(event->pos());
    if (handle >= 0) {
        draggingHandle_ = handle;
        dragStartPos_ = (orientation_ == Qt::Horizontal ? event->pos().x() : event->pos().y());
        dragStartSizes_ = sizes_;
    }
}

void QSplitter::mouseMoveEvent(QMouseEvent* event) {
    if (draggingHandle_ < 0) return;
    int pos = (orientation_ == Qt::Horizontal ? event->pos().x() : event->pos().y());
    int delta = pos - dragStartPos_;
    sizes_ = dragStartSizes_;
    int minSize = 24;
    int newLeft = qMax(minSize, sizes_[draggingHandle_] + delta);
    int shift = newLeft - sizes_[draggingHandle_];
    sizes_[draggingHandle_] += shift;
    sizes_[draggingHandle_ + 1] = qMax(minSize, sizes_[draggingHandle_ + 1] - shift);
    applySizes();
}

void QSplitter::mouseReleaseEvent(QMouseEvent* event) {
    if (draggingHandle_ >= 0) {
        splitterMoved(draggingHandle_, (orientation_ == Qt::Horizontal ? event->pos().x() : event->pos().y()));
        draggingHandle_ = -1;
    }
}

QSize QSplitter::sizeHint() const {
    QSize total(0, 0);
    for (int i = 0; i < panels_.count(); ++i) {
        QSize s = panels_[i]->sizeHint();
        if (orientation_ == Qt::Horizontal) {
            total.setWidth(total.width() + s.width() + handleWidth_);
            total.setHeight(qMax(total.height(), s.height()));
        } else {
            total.setHeight(total.height() + s.height() + handleWidth_);
            total.setWidth(qMax(total.width(), s.width()));
        }
    }
    return total;
}

// ===========================================================================
// QStackedWidget
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QStackedWidget[] = {
    { "currentIndex", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QStackedWidget[] = {
    { "currentChanged", "currentChanged(int)" }
};
const QMetaObject QStackedWidget::staticMetaObject = {
    &QWidget::staticMetaObject, "QStackedWidget",
    qtc_props_QStackedWidget, 1, qtc_signals_QStackedWidget, 1
};

QVariant QStackedWidget::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(currentIndex_);
    default: break;
    }
    return QVariant();
}

bool QStackedWidget::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setCurrentIndex(value.toInt()); return true;
    default: break;
    }
    return false;
}

QStackedWidget::QStackedWidget(QWidget* parent) : QWidget(parent), currentIndex_(-1) {}
QStackedWidget::~QStackedWidget() {}

int QStackedWidget::addWidget(QWidget* widget) { return insertWidget(pages_.count(), widget); }

int QStackedWidget::insertWidget(int index, QWidget* widget) {
    if (!widget) return -1;
    if (index < 0 || index > pages_.count()) index = pages_.count();
    widget->setParentWidget(this);
    pages_.insert(index, widget);
    widget->hide();
    if (currentIndex_ < 0) setCurrentIndex(index);
    else if (index <= currentIndex_) ++currentIndex_;
    widget->setGeometry(rect());
    return index;
}

void QStackedWidget::removeWidget(QWidget* widget) {
    int index = pages_.indexOf(widget);
    if (index < 0) return;
    pages_.removeAt(index);
    widget->hide();
    widget->setParentWidget(0);
    if (currentIndex_ >= pages_.count()) currentIndex_ = pages_.count() - 1;
    if (currentIndex_ >= 0) pages_[currentIndex_]->show();
}

QWidget* QStackedWidget::widget(int index) const {
    if (index < 0 || index >= pages_.count()) return 0;
    return pages_[index];
}

QWidget* QStackedWidget::currentWidget() const { return widget(currentIndex_); }

void QStackedWidget::setCurrentIndex(int index) {
    if (index < -1 || index >= pages_.count()) return;
    if (index == currentIndex_) return;
    if (currentIndex_ >= 0 && currentIndex_ < pages_.count()) pages_[currentIndex_]->hide();
    currentIndex_ = index;
    if (index >= 0) {
        pages_[index]->setGeometry(rect());
        pages_[index]->show();
        pages_[index]->raise();
    }
    currentChanged(index);
}

void QStackedWidget::setCurrentWidget(QWidget* widget) {
    int index = pages_.indexOf(widget);
    if (index >= 0) setCurrentIndex(index);
}

void QStackedWidget::resizeEvent(QResizeEvent* event) {
    for (int i = 0; i < pages_.count(); ++i) pages_[i]->setGeometry(rect());
    QWidget::resizeEvent(event);
}

QSize QStackedWidget::sizeHint() const {
    QSize best(0, 0);
    for (int i = 0; i < pages_.count(); ++i) {
        QSize s = pages_[i]->sizeHint();
        best.setWidth(qMax(best.width(), s.width()));
        best.setHeight(qMax(best.height(), s.height()));
    }
    return best;
}

QTC_REGISTER_WIDGET(QTabWidget, "Containers", "Tab Widget", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QScrollArea, "Containers", "Scroll Area", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QSplitter, "Containers", "Splitter", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QStackedWidget, "Containers", "Stacked Widget", "<qtc/QtWidgets.h>")
