// QtClone - qwidgets_views.cpp : list, tree and table widgets.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include "qwidget_p.h"

// ===========================================================================
// QListWidgetItem
// ===========================================================================
QListWidgetItem::QListWidgetItem(const QString& text)
    : text_(text), checkState_(Qt::Unchecked), flags_(Qt::ItemIsSelectable | Qt::ItemIsEnabled),
      selected_(false), hidden_(false), row_(-1) {}

QListWidgetItem::~QListWidgetItem() {}

QVariant QListWidgetItem::data(int role) const {
    if (role == Qt::DisplayRole) return QVariant(text_);
    return userData_;
}

void QListWidgetItem::setData(int role, const QVariant& value) {
    if (role == Qt::DisplayRole) text_ = value.toString();
    else userData_ = value;
}

// ===========================================================================
// QListWidget
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QListWidget[] = {
    { "alternatingRowColors", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "itemHeight", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "spacing", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "sortingEnabled", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QListWidget[] = {
    { "currentRowChanged", "currentRowChanged(int)" },
    { "itemClicked", "itemClicked(QListWidgetItem*)" },
    { "itemDoubleClicked", "itemDoubleClicked(QListWidgetItem*)" },
    { "itemSelectionChanged", "itemSelectionChanged()" }
};
const QMetaObject QListWidget::staticMetaObject = {
    &QWidget::staticMetaObject, "QListWidget",
    qtc_props_QListWidget, 4, qtc_signals_QListWidget, 4
};

QVariant QListWidget::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(alternate_);
    case 1: return QVariant(itemHeight_);
    case 2: return QVariant(itemSpacing_);
    case 3: return QVariant(sorting_);
    default: break;
    }
    return QVariant();
}

bool QListWidget::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setAlternatingRowColors(value.toBool()); return true;
    case 1: setItemHeight(value.toInt()); return true;
    case 2: setSpacing(value.toInt()); return true;
    case 3: setSortingEnabled(value.toBool()); return true;
    default: break;
    }
    return false;
}

QListWidget::QListWidget(QWidget* parent)
    : QWidget(parent), currentRow_(-1), selectionMode_(0), alternate_(false), itemSpacing_(2),
      itemHeight_(26), sorting_(false), vbar_(0), hoverRow_(-1), dragStartRow_(-1), dragging_(false) {
    setFocusPolicy(Qt::StrongFocus);
    vbar_ = new QScrollBar(Qt::Vertical, this);
    vbar_->hide();
    vbar_->valueChanged.connect(this, this, &QListWidget::updateScrollBars);
}

QListWidget::~QListWidget() {}

void QListWidget::addItem(const QString& text) { addItem(new QListWidgetItem(text)); }

void QListWidget::addItem(QListWidgetItem* item) {
    if (!item) return;
    item->setRow(items_.count());
    items_.append(item);
    if (currentRow_ < 0 && (item->flags() & Qt::ItemIsEnabled)) setCurrentRow(0);
    updateScrollBars();
    update();
}

void QListWidget::insertItem(int row, const QString& text) {
    if (row < 0 || row > items_.count()) row = items_.count();
    QListWidgetItem* item = new QListWidgetItem(text);
    items_.insert(row, item);
    for (int i = 0; i < items_.count(); ++i) items_[i]->setRow(i);
    updateScrollBars();
    update();
}

void QListWidget::takeItem(int row) {
    if (row < 0 || row >= items_.count()) return;
    items_.removeAt(row);
    for (int i = 0; i < items_.count(); ++i) items_[i]->setRow(i);
    if (currentRow_ >= items_.count()) currentRow_ = items_.count() - 1;
    updateScrollBars();
    update();
}

QListWidgetItem* QListWidget::item(int row) const {
    if (row < 0 || row >= items_.count()) return 0;
    return items_[row];
}

void QListWidget::setCurrentRow(int row) {
    if (row < -1 || row >= items_.count()) return;
    if (currentRow_ == row) return;
    if (currentRow_ >= 0 && currentRow_ < items_.count()) items_[currentRow_]->setSelected(false);
    currentRow_ = row;
    if (currentRow_ >= 0) {
        items_[currentRow_]->setSelected(true);
        ensureVisible(currentRow_);
    }
    update();
    currentRowChanged(currentRow_);
    itemSelectionChanged();
}

QListWidgetItem* QListWidget::currentItem() const { return item(currentRow_); }

void QListWidget::setCurrentItem(QListWidgetItem* item) {
    int index = items_.indexOf(item);
    if (index >= 0) setCurrentRow(index);
}

void QListWidget::clear() {
    for (int i = 0; i < items_.count(); ++i) delete items_[i];
    items_.clearAll();
    currentRow_ = -1;
    updateScrollBars();
    update();
}

QList<QListWidgetItem*> QListWidget::selectedItems() const {
    QList<QListWidgetItem*> out;
    for (int i = 0; i < items_.count(); ++i)
        if (items_[i]->isSelected()) out.append(items_[i]);
    return out;
}

void QListWidget::sortItems() {
    for (int i = 1; i < items_.count(); ++i) {
        QListWidgetItem* key = items_[i];
        int j = i;
        while (j > 0 && items_[j - 1]->text() > key->text()) {
            items_[j] = items_[j - 1];
            --j;
        }
        items_[j] = key;
    }
    for (int i = 0; i < items_.count(); ++i) items_[i]->setRow(i);
    update();
}

QRect QListWidget::itemRect(int row) const {
    int y = itemSpacing_ + row * (itemHeight_ + itemSpacing_);
    return QRect(itemSpacing_, y, width() - itemSpacing_ * 2 - (vbar_ && vbar_->isVisible() ? vbar_->width() : 0),
                 itemHeight_);
}

int QListWidget::rowAt(const QPoint& pos) const {
    int y = pos.y() - itemSpacing_;
    int row = y / qMax(1, itemHeight_ + itemSpacing_);
    if (row < 0 || row >= items_.count()) return -1;
    return row;
}

void QListWidget::ensureVisible(int row) {
    if (!vbar_) return;
    QRect r = itemRect(row);
    int offset = -vbar_->value();
    if (r.top() + offset < 0) vbar_->setValue(vbar_->value() - (0 - (r.top() + offset)));
    else if (r.bottom() + offset > height()) vbar_->setValue(vbar_->value() + (r.bottom() + offset - height()));
}

void QListWidget::updateScrollBars() {
    if (!vbar_) return;
    int contentH = items_.count() * (itemHeight_ + itemSpacing_) + itemSpacing_;
    if (contentH > height()) {
        vbar_->setGeometry(width() - vbar_->sizeHint().width(), 0, vbar_->sizeHint().width(), height());
        vbar_->setRange(0, contentH - height());
        vbar_->setPageStep(height());
        vbar_->show();
    } else {
        vbar_->hide();
        vbar_->setValue(0);
    }
    update();
}

void QListWidget::resizeEvent(QResizeEvent* event) {
    updateScrollBars();
    QWidget::resizeEvent(event);
}

QSize QListWidget::sizeHint() const { return QSize(200, 160); }

void QListWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.base);
    p.setPen(QPen(c.border, 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    int offset = vbar_ ? -vbar_->value() : 0;
    p.setClipRect(rect().adjusted(1, 1, -1, -1));
    for (int i = 0; i < items_.count(); ++i) {
        QRect r = itemRect(i);
        r.translate(0, offset);
        if (!r.intersects(rect())) continue;
        if (items_[i]->isHidden()) continue;
        QStyleOptionViewItem opt;
        opt.rect = r;
        opt.text = items_[i]->text();
        opt.selected = items_[i]->isSelected();
        opt.hovered = (i == hoverRow_);
        opt.alternate = alternate_ && (i % 2 == 1);
        opt.hasIcon = !items_[i]->icon().isNull();
        opt.icon = items_[i]->icon();
        style()->drawViewItem(&p, opt, this);
    }
    p.setClipping(false);
}

void QListWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    int row = rowAt(event->pos());
    if (row >= 0) {
        setCurrentRow(row);
        itemClicked(items_[row]);
        dragStartRow_ = row;
    }
    event->accept();
}

void QListWidget::mouseMoveEvent(QMouseEvent* event) {
    int row = rowAt(event->pos());
    if (row != hoverRow_) {
        hoverRow_ = row;
        update();
    }
}

void QListWidget::mouseReleaseEvent(QMouseEvent*) { dragStartRow_ = -1; }

void QListWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    int row = rowAt(event->pos());
    if (row >= 0) itemDoubleClicked(items_[row]);
}

void QListWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Down) { setCurrentRow(qMin(items_.count() - 1, currentRow_ + 1)); return; }
    if (event->key() == Qt::Key_Up) { setCurrentRow(qMax(0, currentRow_ - 1)); return; }
    if (event->key() == Qt::Key_Home) { setCurrentRow(0); return; }
    if (event->key() == Qt::Key_End) { setCurrentRow(items_.count() - 1); return; }
    if (event->key() == Qt::Key_Return && currentRow_ >= 0) { itemDoubleClicked(items_[currentRow_]); return; }
    QWidget::keyPressEvent(event);
}

void QListWidget::wheelEvent(QWheelEvent* event) {
    if (vbar_ && vbar_->isVisible())
        vbar_->setValue(vbar_->value() - (event->delta() > 0 ? 60 : -60));
}

// ===========================================================================
// QTreeWidgetItem
// ===========================================================================
QTreeWidgetItem::QTreeWidgetItem(const QStringList& texts)
    : texts_(texts), parent_(0), expanded_(false), selected_(false), hidden_(false) {}

QTreeWidgetItem::~QTreeWidgetItem() {
    for (int i = 0; i < children_.count(); ++i) delete children_[i];
    children_.clearAll();
}

QString QTreeWidgetItem::text(int column) const {
    if (column < 0 || column >= texts_.count()) return QString();
    return texts_[column];
}

void QTreeWidgetItem::setText(int column, const QString& text) {
    while (texts_.count() <= column) texts_.append(QString());
    texts_[column] = text;
}

QTreeWidgetItem* QTreeWidgetItem::child(int index) const {
    if (index < 0 || index >= children_.count()) return 0;
    return children_[index];
}

int QTreeWidgetItem::indexOfChild(QTreeWidgetItem* child) const { return children_.indexOf(child); }

void QTreeWidgetItem::addChild(QTreeWidgetItem* child) { insertChild(children_.count(), child); }

void QTreeWidgetItem::insertChild(int index, QTreeWidgetItem* child) {
    if (!child) return;
    if (index < 0 || index > children_.count()) index = children_.count();
    child->parent_ = this;
    children_.insert(index, child);
}

void QTreeWidgetItem::removeChild(QTreeWidgetItem* child) { children_.removeAll(child); }

void QTreeWidgetItem::takeChildren() {
    for (int i = 0; i < children_.count(); ++i) children_[i]->parent_ = 0;
    children_.clearAll();
}

QIcon QTreeWidgetItem::icon(int column) const {
    if (column < 0 || column >= icons_.count()) return QIcon();
    return icons_[column];
}

void QTreeWidgetItem::setIcon(int column, const QIcon& icon) {
    while (icons_.count() <= column) icons_.append(QIcon());
    icons_[column] = icon;
}

QVariant QTreeWidgetItem::data(int, int role) const {
    if (role == Qt::DisplayRole) return QVariant(text(0));
    return userData_;
}

void QTreeWidgetItem::setData(int, int role, const QVariant& value) {
    if (role == Qt::DisplayRole) setText(0, value.toString());
    else userData_ = value;
}

// ===========================================================================
// QTreeWidget
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QTreeWidget[] = {
    { "alternatingRowColors", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "indentation", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "itemHeight", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QTreeWidget[] = {
    { "itemClicked", "itemClicked(QTreeWidgetItem*,int)" },
    { "itemDoubleClicked", "itemDoubleClicked(QTreeWidgetItem*,int)" },
    { "itemExpanded", "itemExpanded(QTreeWidgetItem*,int)" },
    { "itemCollapsed", "itemCollapsed(QTreeWidgetItem*,int)" },
    { "currentItemChanged", "currentItemChanged()" }
};
const QMetaObject QTreeWidget::staticMetaObject = {
    &QWidget::staticMetaObject, "QTreeWidget",
    qtc_props_QTreeWidget, 3, qtc_signals_QTreeWidget, 5
};

QVariant QTreeWidget::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(alternate_);
    case 1: return QVariant(indentation_);
    case 2: return QVariant(itemHeight_);
    default: break;
    }
    return QVariant();
}

bool QTreeWidget::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setAlternatingRowColors(value.toBool()); return true;
    case 1: setIndentation(value.toInt()); return true;
    case 2: setItemHeight(value.toInt()); return true;
    default: break;
    }
    return false;
}

QTreeWidget::QTreeWidget(QWidget* parent)
    : QWidget(parent), current_(0), alternate_(false), indentation_(18), itemHeight_(24),
      vbar_(0), columnWidth_(160), hoverItemY_(-1) {
    setFocusPolicy(Qt::StrongFocus);
    vbar_ = new QScrollBar(Qt::Vertical, this);
    vbar_->hide();
    vbar_->valueChanged.connect(this, this, &QTreeWidget::updateScrollBars);
}

QTreeWidget::~QTreeWidget() { clear(); }

void QTreeWidget::setHeaderLabels(const QStringList& labels) {
    headers_ = labels;
    if (headers_.isEmpty()) headers_.append(L"Column 1");
    update();
}

void QTreeWidget::setColumnCount(int count) {
    while (headers_.count() < count) headers_.append(QString(L"Column ") + QString::number(headers_.count() + 1));
    while (headers_.count() > count) headers_.removeAt(headers_.count() - 1);
    update();
}

void QTreeWidget::addTopLevelItem(QTreeWidgetItem* item) { insertTopLevelItem(roots_.count(), item); }

void QTreeWidget::insertTopLevelItem(int index, QTreeWidgetItem* item) {
    if (!item) return;
    if (index < 0 || index > roots_.count()) index = roots_.count();
    roots_.insert(index, item);
    updateScrollBars();
    update();
}

void QTreeWidget::takeTopLevelItem(int index) {
    if (index < 0 || index >= roots_.count()) return;
    roots_.removeAt(index);
    updateScrollBars();
    update();
}

QTreeWidgetItem* QTreeWidget::topLevelItem(int index) const {
    if (index < 0 || index >= roots_.count()) return 0;
    return roots_[index];
}

void QTreeWidget::clear() {
    for (int i = 0; i < roots_.count(); ++i) delete roots_[i];
    roots_.clearAll();
    current_ = 0;
    updateScrollBars();
    update();
}

void QTreeWidget::setCurrentItem(QTreeWidgetItem* item) {
    if (current_ == item) return;
    if (current_) current_->setSelected(false);
    current_ = item;
    if (current_) {
        current_->setSelected(true);
        ensureVisible(current_);
    }
    update();
    currentItemChanged();
}

QList<QTreeWidgetItem*> QTreeWidget::selectedItems() const {
    QList<QTreeWidgetItem*> out;
    if (current_ && current_->isSelected()) out.append(current_);
    return out;
}

void QTreeWidget::expandAll() {
    QList<QTreeWidgetItem*> stack;
    for (int i = 0; i < roots_.count(); ++i) stack.append(roots_[i]);
    while (!stack.isEmpty()) {
        QTreeWidgetItem* item = stack.last();
        stack.removeAt(stack.count() - 1);
        item->setExpanded(true);
        for (int c = 0; c < item->childCount(); ++c) stack.append(item->child(c));
    }
    updateScrollBars();
    update();
}

void QTreeWidget::collapseAll() {
    QList<QTreeWidgetItem*> stack;
    for (int i = 0; i < roots_.count(); ++i) stack.append(roots_[i]);
    while (!stack.isEmpty()) {
        QTreeWidgetItem* item = stack.last();
        stack.removeAt(stack.count() - 1);
        item->setExpanded(false);
        for (int c = 0; c < item->childCount(); ++c) stack.append(item->child(c));
    }
    updateScrollBars();
    update();
}

void QTreeWidget::expandItem(QTreeWidgetItem* item) {
    if (!item) return;
    item->setExpanded(true);
    updateScrollBars();
    update();
    itemExpanded(item, 0);
}

void QTreeWidget::collapseItem(QTreeWidgetItem* item) {
    if (!item) return;
    item->setExpanded(false);
    updateScrollBars();
    update();
    itemCollapsed(item, 0);
}

void QTreeWidget::collectVisible(QList<QTreeWidgetItem*>& out, QList<int>& depths) {
    QList<QTreeWidgetItem*> stack;
    QList<int> stackDepth;
    for (int i = roots_.count() - 1; i >= 0; --i) {
        stack.append(roots_[i]);
        stackDepth.append(0);
    }
    while (!stack.isEmpty()) {
        QTreeWidgetItem* item = stack.last();
        int depth = stackDepth.last();
        stack.removeAt(stack.count() - 1);
        stackDepth.removeAt(stackDepth.count() - 1);
        if (item->isHidden()) continue;
        out.append(item);
        depths.append(depth);
        if (item->isExpanded()) {
            for (int c = item->childCount() - 1; c >= 0; --c) {
                stack.append(item->child(c));
                stackDepth.append(depth + 1);
            }
        }
    }
}

int QTreeWidget::contentHeight() const {
    QList<QTreeWidgetItem*> items;
    QList<int> depths;
    const_cast<QTreeWidget*>(this)->collectVisible(items, depths);
    int headerH = headers_.isEmpty() ? 0 : itemHeight_;
    return items.count() * itemHeight_ + headerH;
}

void QTreeWidget::updateScrollBars() {
    if (!vbar_) return;
    int content = contentHeight();
    if (content > height()) {
        vbar_->setGeometry(width() - vbar_->sizeHint().width(), 0, vbar_->sizeHint().width(), height());
        vbar_->setRange(0, content - height());
        vbar_->setPageStep(height());
        vbar_->show();
    } else {
        vbar_->hide();
        vbar_->setValue(0);
    }
    update();
}

void QTreeWidget::ensureVisible(QTreeWidgetItem* item) {
    if (!item || !vbar_) return;
    QList<QTreeWidgetItem*> items;
    QList<int> depths;
    collectVisible(items, depths);
    int index = items.indexOf(item);
    if (index < 0) return;
    int y = index * itemHeight_ - vbar_->value();
    if (y < 0) vbar_->setValue(vbar_->value() + y);
    else if (y + itemHeight_ > height()) vbar_->setValue(vbar_->value() + y + itemHeight_ - height());
}

QSize QTreeWidget::sizeHint() const { return QSize(240, 200); }

void QTreeWidget::resizeEvent(QResizeEvent* event) {
    updateScrollBars();
    QWidget::resizeEvent(event);
}

QTreeWidgetItem* QTreeWidget::itemAtPoint(const QPoint& pos, int* depth, QRect* rowRect, int* column) const {
    QList<QTreeWidgetItem*> items;
    QList<int> depths;
    const_cast<QTreeWidget*>(this)->collectVisible(items, depths);
    int headerH = headers_.isEmpty() ? 0 : itemHeight_;
    int offset = vbar_ ? -vbar_->value() : 0;
    int index = (pos.y() - headerH - offset) / qMax(1, itemHeight_);
    if (index < 0 || index >= items.count()) return 0;
    if (rowRect) *rowRect = QRect(0, headerH + offset + index * itemHeight_, width(), itemHeight_);
    if (depth) *depth = depths[index];
    if (column) *column = qBound(0, pos.x() / qMax(1, columnWidth_), qMax(0, headers_.count() - 1));
    return items[index];
}

void QTreeWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.base);
    p.setPen(QPen(c.border, 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    int headerH = 0;
    if (!headers_.isEmpty()) {
        headerH = itemHeight_;
        for (int i = 0; i < headers_.count(); ++i) {
            QStyleOptionHeader opt;
            opt.rect = QRect(i * columnWidth_, 0, columnWidth_, headerH);
            opt.text = headers_[i];
            style()->drawHeader(&p, opt, this);
        }
    }

    QList<QTreeWidgetItem*> items;
    QList<int> depths;
    collectVisible(items, depths);
    int offset = vbar_ ? -vbar_->value() : 0;
    p.setClipRect(QRect(0, headerH, width(), height() - headerH));

    QFont f = font();
    QFont bold = f;
    bold.setBold(true);
    for (int i = 0; i < items.count(); ++i) {
        int y = headerH + offset + i * itemHeight_;
        QRect row(1, y, width() - 2 - (vbar_ && vbar_->isVisible() ? vbar_->width() : 0), itemHeight_);
        if (!row.intersects(rect())) continue;
        bool selected = items[i]->isSelected();
        bool hovered = (y == hoverItemY_);
        if (selected) p.fillRect(row, c.selection);
        else if (hovered) p.fillRect(row, c.surfaceAlt);
        else if (alternate_ && (i % 2 == 1)) p.fillRect(row, c.alternateBase);

        int x = 6 + depths[i] * indentation_;
        bool hasChildren = items[i]->childCount() > 0;
        if (hasChildren) {
            QRect arrow(x, y + itemHeight_ / 2 - 4, 10, 8);
            style()->drawArrow(&p, arrow, items[i]->isExpanded() ? Qt::DownArrow : Qt::RightArrow,
                               selected ? c.selectionText : c.windowText);
        }
        x += 16;
        if (!items[i]->icon(0).isNull()) {
            QPixmap pm = items[i]->icon(0).pixmap(QSize(16, 16));
            p.drawPixmap(QPoint(x, y + (itemHeight_ - 16) / 2), pm);
            x += 20;
        }
        p.setPen(QPen(selected ? c.selectionText : c.windowText));
        p.setFont(hasChildren ? bold : f);
        p.drawText(QRect(x, y, width() - x - 8, itemHeight_), Qt::AlignLeft | Qt::AlignVCenter, items[i]->text(0));
    }
    p.setClipping(false);
}

void QTreeWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    int depth = 0;
    QRect rowRect;
    int column = 0;
    QTreeWidgetItem* item = itemAtPoint(event->pos(), &depth, &rowRect, &column);
    if (!item) return;
    int arrowX = 6 + depth * indentation_;
    QRect arrow(arrowX, rowRect.top() + itemHeight_ / 2 - 6, 14, 12);
    if (item->childCount() > 0 && arrow.contains(event->pos())) {
        if (item->isExpanded()) collapseItem(item);
        else expandItem(item);
        return;
    }
    setCurrentItem(item);
    itemClicked(item, column);
    event->accept();
}

void QTreeWidget::mouseMoveEvent(QMouseEvent* event) {
    int depth = 0;
    QRect rowRect;
    QTreeWidgetItem* item = itemAtPoint(event->pos(), &depth, &rowRect, 0);
    int y = item ? rowRect.top() : -1;
    if (y != hoverItemY_) {
        hoverItemY_ = y;
        update();
    }
}

void QTreeWidget::mouseReleaseEvent(QMouseEvent*) {}

void QTreeWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    int depth = 0;
    QRect rowRect;
    int column = 0;
    QTreeWidgetItem* item = itemAtPoint(event->pos(), &depth, &rowRect, &column);
    if (!item) return;
    if (item->childCount() > 0) {
        if (item->isExpanded()) collapseItem(item);
        else expandItem(item);
    }
    itemDoubleClicked(item, column);
}

void QTreeWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Right && current_ && current_->childCount() > 0) {
        if (!current_->isExpanded()) expandItem(current_);
        return;
    }
    if (event->key() == Qt::Key_Left && current_ && current_->isExpanded()) {
        collapseItem(current_);
        return;
    }
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) {
        QList<QTreeWidgetItem*> items;
        QList<int> depths;
        collectVisible(items, depths);
        int index = items.indexOf(current_);
        int next = index + (event->key() == Qt::Key_Down ? 1 : -1);
        if (next >= 0 && next < items.count()) setCurrentItem(items[next]);
        return;
    }
    QWidget::keyPressEvent(event);
}

void QTreeWidget::wheelEvent(QWheelEvent* event) {
    if (vbar_ && vbar_->isVisible())
        vbar_->setValue(vbar_->value() - (event->delta() > 0 ? 60 : -60));
}

// ===========================================================================
// QTableWidgetItem
// ===========================================================================
QTableWidgetItem::QTableWidgetItem(const QString& text)
    : text_(text), checkState_(Qt::Unchecked), flags_(Qt::ItemIsSelectable | Qt::ItemIsEnabled) {}

QTableWidgetItem::~QTableWidgetItem() {}

QVariant QTableWidgetItem::data(int role) const {
    if (role == Qt::DisplayRole) return QVariant(text_);
    return userData_;
}

void QTableWidgetItem::setData(int role, const QVariant& value) {
    if (role == Qt::DisplayRole) text_ = value.toString();
    else userData_ = value;
}

// ===========================================================================
// QTableWidget
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QTableWidget[] = {
    { "rowCount", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "columnCount", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "alternatingRowColors", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "rowHeight", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "gridEnabled", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QTableWidget[] = {
    { "cellClicked", "cellClicked(int,int)" },
    { "cellDoubleClicked", "cellDoubleClicked(int,int)" },
    { "currentCellChanged", "currentCellChanged(int,int)" }
};
const QMetaObject QTableWidget::staticMetaObject = {
    &QWidget::staticMetaObject, "QTableWidget",
    qtc_props_QTableWidget, 5, qtc_signals_QTableWidget, 3
};

QVariant QTableWidget::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(rows_);
    case 1: return QVariant(columns_);
    case 2: return QVariant(alternate_);
    case 3: return QVariant(rowHeight_);
    case 4: return QVariant(grid_);
    default: break;
    }
    return QVariant();
}

bool QTableWidget::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setRowCount(value.toInt()); return true;
    case 1: setColumnCount(value.toInt()); return true;
    case 2: setAlternatingRowColors(value.toBool()); return true;
    case 3: setRowHeight(value.toInt()); return true;
    case 4: setGridEnabled(value.toBool()); return true;
    default: break;
    }
    return false;
}

QTableWidget::QTableWidget(QWidget* parent)
    : QWidget(parent), rows_(0), columns_(0), currentRow_(-1), currentColumn_(-1),
      selectionMode_(0), alternate_(false), grid_(true), hHeader_(true), vHeader_(false),
      rowHeight_(26), sorting_(false), vbar_(0), hbar_(0), columnWidth_(110),
      scrollY_(0), scrollX_(0) {
    setFocusPolicy(Qt::StrongFocus);
    vbar_ = new QScrollBar(Qt::Vertical, this);
    vbar_->hide();
    hbar_ = new QScrollBar(Qt::Horizontal, this);
    hbar_->hide();
    vbar_->valueChanged.connect(this, this, &QTableWidget::updateScrollBars);
    hbar_->valueChanged.connect(this, this, &QTableWidget::updateScrollBars);
}

QTableWidget::QTableWidget(int rows, int columns, QWidget* parent)
    : QWidget(parent), rows_(0), columns_(0), currentRow_(-1), currentColumn_(-1),
      selectionMode_(0), alternate_(false), grid_(true), hHeader_(true), vHeader_(false),
      rowHeight_(26), sorting_(false), vbar_(0), hbar_(0), columnWidth_(110),
      scrollY_(0), scrollX_(0) {
    setFocusPolicy(Qt::StrongFocus);
    vbar_ = new QScrollBar(Qt::Vertical, this);
    vbar_->hide();
    hbar_ = new QScrollBar(Qt::Horizontal, this);
    hbar_->hide();
    vbar_->valueChanged.connect(this, this, &QTableWidget::updateScrollBars);
    hbar_->valueChanged.connect(this, this, &QTableWidget::updateScrollBars);
    setRowCount(rows);
    setColumnCount(columns);
}

QTableWidget::~QTableWidget() {
    for (int i = 0; i < cells_.count(); ++i) delete cells_[i];
}

void QTableWidget::setRowCount(int rows) {
    if (rows < 0) rows = 0;
    int old = rows_;
    rows_ = rows;
    cells_.resize((size_t)(rows_ * columns_), (QTableWidgetItem*)0);
    for (int i = old; i < rows_; ++i) cells_.resize((size_t)(rows_ * columns_), (QTableWidgetItem*)0);
    updateScrollBars();
    update();
}

void QTableWidget::setColumnCount(int columns) {
    if (columns < 0) columns = 0;
    int old = columns_;
    QList<QTableWidgetItem*> newCells;
    newCells.resize((size_t)(rows_ * columns), (QTableWidgetItem*)0);
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < qMin(old, columns); ++c) newCells[(size_t)(r * columns + c)] = cells_[(size_t)(r * old + c)];
    }
    for (int c = old; c < columns; ++c) columnHeaders_.append(QString(L"Column ") + QString::number(c + 1));
    for (int c = columns; c < columnHeaders_.count(); ++c) columnHeaders_.removeAt(columnHeaders_.count() - 1);
    cells_ = newCells;
    columns_ = columns;
    updateScrollBars();
    update();
}

void QTableWidget::setHorizontalHeaderLabels(const QStringList& labels) {
    columnHeaders_ = labels;
    while (columnHeaders_.count() < columns_) columnHeaders_.append(QString());
    update();
}

void QTableWidget::setVerticalHeaderLabels(const QStringList& labels) {
    rowHeaders_ = labels;
    update();
}

QTableWidgetItem* QTableWidget::item(int row, int column) const {
    if (row < 0 || row >= rows_ || column < 0 || column >= columns_) return 0;
    return cells_[(size_t)(row * columns_ + column)];
}

void QTableWidget::setItem(int row, int column, QTableWidgetItem* item) {
    if (row < 0 || row >= rows_ || column < 0 || column >= columns_) {
        delete item;
        return;
    }
    cells_[(size_t)(row * columns_ + column)] = item;
    update();
}

QTableWidgetItem* QTableWidget::takeItemAt(int row, int column) {
    if (row < 0 || row >= rows_ || column < 0 || column >= columns_) return 0;
    QTableWidgetItem* item = cells_[(size_t)(row * columns_ + column)];
    cells_[(size_t)(row * columns_ + column)] = 0;
    update();
    return item;
}

void QTableWidget::clearContents() {
    for (int i = 0; i < cells_.count(); ++i) {
        delete cells_[i];
        cells_[i] = 0;
    }
    update();
}

void QTableWidget::clear() {
    clearContents();
    columnHeaders_.clearAll();
    rowHeaders_.clearAll();
    update();
}

void QTableWidget::setCurrentCell(int row, int column) {
    if (row < 0 || row >= rows_ || column < 0 || column >= columns_) return;
    currentRow_ = row;
    currentColumn_ = column;
    ensureVisible(row, column);
    update();
    currentCellChanged(row, column);
}

QTableWidgetItem* QTableWidget::currentItem() const { return item(currentRow_, currentColumn_); }
void QTableWidget::selectRow(int row) { setCurrentCell(row, 0); }
void QTableWidget::selectColumn(int column) { setCurrentCell(0, column); }

int QTableWidget::headerHeight() const { return hHeader_ ? 28 : 0; }
int QTableWidget::headerWidth() const { return vHeader_ ? 60 : 0; }

QRect QTableWidget::cellRect(int row, int column) const {
    int x = headerWidth() + column * columnWidth_ - scrollX_;
    int y = headerHeight() + row * rowHeight_ - scrollY_;
    return QRect(x, y, columnWidth_, rowHeight_);
}

QPoint QTableWidget::cellAt(const QPoint& pos) const {
    int column = (pos.x() - headerWidth() + scrollX_) / qMax(1, columnWidth_);
    int row = (pos.y() - headerHeight() + scrollY_) / qMax(1, rowHeight_);
    return QPoint(column, row);
}

void QTableWidget::updateScrollBars() {
    if (!vbar_ || !hbar_) return;
    int contentH = rows_ * rowHeight_ + headerHeight();
    int contentW = columns_ * columnWidth_ + headerWidth();
    if (contentH > height()) {
        vbar_->setGeometry(width() - vbar_->sizeHint().width(), 0, vbar_->sizeHint().width(), height());
        vbar_->setRange(0, contentH - height());
        vbar_->setPageStep(height());
        vbar_->show();
    } else {
        vbar_->hide();
        vbar_->setValue(0);
    }
    if (contentW > width()) {
        hbar_->setGeometry(0, height() - hbar_->sizeHint().height(),
                           width() - (vbar_->isVisible() ? vbar_->width() : 0), hbar_->sizeHint().height());
        hbar_->setRange(0, contentW - width());
        hbar_->setPageStep(width());
        hbar_->show();
    } else {
        hbar_->hide();
        hbar_->setValue(0);
    }
    scrollY_ = vbar_->value();
    scrollX_ = hbar_->value();
    update();
}

void QTableWidget::ensureVisible(int row, int column) {
    QRect r = cellRect(row, column);
    if (r.top() < headerHeight() && vbar_) vbar_->setValue(vbar_->value() - (headerHeight() - r.top()));
    if (r.bottom() > height() && vbar_) vbar_->setValue(vbar_->value() + (r.bottom() - height()));
    if (r.left() < headerWidth() && hbar_) hbar_->setValue(hbar_->value() - (headerWidth() - r.left()));
    if (r.right() > width() && hbar_) hbar_->setValue(hbar_->value() + (r.right() - width()));
}

QSize QTableWidget::sizeHint() const { return QSize(320, 200); }

void QTableWidget::resizeEvent(QResizeEvent* event) {
    updateScrollBars();
    QWidget::resizeEvent(event);
}

void QTableWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.base);
    int hh = headerHeight();
    int hw = headerWidth();

    if (hHeader_) {
        for (int col = 0; col < columns_; ++col) {
            QStyleOptionHeader opt;
            opt.rect = QRect(hw + col * columnWidth_ - scrollX_, 0, columnWidth_, hh);
            opt.text = (col < columnHeaders_.count()) ? columnHeaders_[col] : QString();
            style()->drawHeader(&p, opt, this);
        }
    }
    if (vHeader_) {
        for (int row = 0; row < rows_; ++row) {
            QRect r(0, hh + row * rowHeight_ - scrollY_, hw, rowHeight_);
            p.fillRect(r, c.header);
            p.setPen(QPen(c.headerText));
            p.drawText(r, Qt::AlignCenter, QString::number(row + 1));
        }
    }

    p.setClipRect(QRect(hw, hh, width() - hw, height() - hh));
    for (int row = 0; row < rows_; ++row) {
        for (int col = 0; col < columns_; ++col) {
            QRect r = cellRect(row, col);
            if (!r.intersects(rect())) continue;
            bool selected = (row == currentRow_ && col == currentColumn_);
            if (selected) p.fillRect(r, c.selection);
            else if (alternate_ && (row % 2 == 1)) p.fillRect(r, c.alternateBase);
            if (grid_) {
                p.setPen(QPen(c.gridLine, 1));
                p.drawLine(r.right(), r.top(), r.right(), r.bottom());
                p.drawLine(r.left(), r.bottom(), r.right(), r.bottom());
            }
            QTableWidgetItem* it = item(row, col);
            if (it) {
                p.setPen(QPen(selected ? c.selectionText : c.windowText));
                p.setFont(font());
                p.drawText(r.adjusted(6, 0, -6, 0), Qt::AlignLeft | Qt::AlignVCenter, it->text());
            }
        }
    }
    p.setClipping(false);
    p.setPen(QPen(c.border, 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

void QTableWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    QPoint cell = cellAt(event->pos());
    if (cell.y() >= 0 && cell.y() < rows_ && cell.x() >= 0 && cell.x() < columns_) {
        setCurrentCell(cell.y(), cell.x());
        cellClicked(cell.y(), cell.x());
    }
    event->accept();
}

void QTableWidget::mouseMoveEvent(QMouseEvent*) {}
void QTableWidget::mouseReleaseEvent(QMouseEvent*) {}

void QTableWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    QPoint cell = cellAt(event->pos());
    if (cell.y() >= 0 && cell.y() < rows_ && cell.x() >= 0 && cell.x() < columns_)
        cellDoubleClicked(cell.y(), cell.x());
}

void QTableWidget::keyPressEvent(QKeyEvent* event) {
    int row = currentRow_ < 0 ? 0 : currentRow_;
    int col = currentColumn_ < 0 ? 0 : currentColumn_;
    if (event->key() == Qt::Key_Down) ++row;
    else if (event->key() == Qt::Key_Up) --row;
    else if (event->key() == Qt::Key_Left) --col;
    else if (event->key() == Qt::Key_Right) ++col;
    else { QWidget::keyPressEvent(event); return; }
    setCurrentCell(qBound(0, row, rows_ - 1), qBound(0, col, columns_ - 1));
}

void QTableWidget::wheelEvent(QWheelEvent* event) {
    if (vbar_ && vbar_->isVisible())
        vbar_->setValue(vbar_->value() - (event->delta() > 0 ? 60 : -60));
}

QTC_REGISTER_WIDGET(QListWidget, "Item Views", "List Widget", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QTreeWidget, "Item Views", "Tree Widget", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QTableWidget, "Item Views", "Table Widget", "<qtc/QtWidgets.h>")
