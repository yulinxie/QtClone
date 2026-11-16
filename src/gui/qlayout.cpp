// QtClone - qlayout.cpp : box, grid, form and stacked layouts.
#include <qtc/QLayout.h>
#include <qtc/QtWidgets.h>
#include <math.h>
#include <vector>

namespace {

int clampInt(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

Qt::Orientations expandingFor(const QSizePolicy& policy) { return policy.expandingDirections(); }

bool policyExpands(QSizePolicy::Policy p) {
    return p == QSizePolicy::Expanding || p == QSizePolicy::MinimumExpanding;
}

// Distributes `total` pixels over `count` cells.
// mins/hints/maxs may be null (0 means "no constraint"/"unbounded").
void distribute(int total, int count, const int* mins, const int* hints, const int* maxs,
                const int* stretch, const unsigned char* expanding, int spacing, int* pos, int* size) {
    if (count <= 0) return;
    std::vector<int> s((size_t)count);
    int sum = 0;
    for (int i = 0; i < count; ++i) {
        int hint = hints ? hints[i] : 0;
        int mn = mins ? mins[i] : 0;
        int mx = maxs ? maxs[i] : 0x7fffffff;
        if (mx <= 0) mx = 0x7fffffff;
        int v = clampInt(hint, mn, mx);
        s[(size_t)i] = v;
        sum += v;
    }
    int spacingTotal = spacing * (count - 1);
    int available = total - spacingTotal;

    if (sum < available) {
        int extra = available - sum;
        int weightTotal = 0;
        for (int i = 0; i < count; ++i) {
            int w = stretch ? stretch[i] : 0;
            if (w <= 0 && expanding && expanding[i]) w = 1;
            weightTotal += w > 0 ? w : 0;
        }
        if (weightTotal > 0) {
            int given = 0;
            for (int i = 0; i < count; ++i) {
                int w = stretch ? stretch[i] : 0;
                if (w <= 0 && expanding && expanding[i]) w = 1;
                if (w <= 0) continue;
                int add = extra * w / weightTotal;
                int mx = maxs ? maxs[i] : 0x7fffffff;
                if (mx <= 0) mx = 0x7fffffff;
                int nv = s[(size_t)i] + add;
                if (nv > mx) nv = mx;
                given += nv - s[(size_t)i];
                s[(size_t)i] = nv;
            }
            // hand out the rounding remainder to the first expanding cell
            int rest = extra - given;
            for (int i = 0; i < count && rest > 0; ++i) {
                int w = stretch ? stretch[i] : 0;
                if (w <= 0 && !(expanding && expanding[i])) continue;
                int mx = maxs ? maxs[i] : 0x7fffffff;
                if (mx <= 0) mx = 0x7fffffff;
                int room = mx - s[(size_t)i];
                int add = rest < room ? rest : room;
                s[(size_t)i] += add;
                rest -= add;
            }
        }
    } else if (sum > available) {
        int deficit = sum - available;
        for (int pass = 0; pass < 4 && deficit > 0; ++pass) {
            int shrinkable = 0;
            for (int i = 0; i < count; ++i) {
                int mn = mins ? mins[i] : 0;
                if (s[(size_t)i] > mn) shrinkable += s[(size_t)i] - mn;
            }
            if (shrinkable <= 0) break;
            int removed = 0;
            for (int i = 0; i < count; ++i) {
                int mn = mins ? mins[i] : 0;
                int room = s[(size_t)i] - mn;
                if (room <= 0) continue;
                int take = deficit * room / shrinkable;
                if (take > room) take = room;
                s[(size_t)i] -= take;
                removed += take;
            }
            deficit -= removed;
            if (removed == 0) break;
        }
    }

    int cursor = 0;
    for (int i = 0; i < count; ++i) {
        pos[i] = cursor;
        size[i] = s[(size_t)i];
        cursor += s[(size_t)i] + spacing;
    }
}

QSize itemHint(const QLayoutItem* item, bool minimum) {
    if (!item) return QSize(0, 0);
    QSize s = minimum ? item->minimumSize() : item->sizeHint();
    if (s.width() < 0) s.setWidth(0);
    if (s.height() < 0) s.setHeight(0);
    return s;
}

} // namespace

// ---------------------------------------------------------------------------
// QSpacerItem
// ---------------------------------------------------------------------------
QSpacerItem::QSpacerItem(int w, int h, QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy)
    : hPolicy_(hPolicy), vPolicy_(vPolicy), expanding_(0) {
    changeSize(w, h, hPolicy, vPolicy);
}

void QSpacerItem::changeSize(int w, int h, QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy) {
    hPolicy_ = hPolicy;
    vPolicy_ = vPolicy;
    Qt::Orientations ex = 0;
    if (policyExpands(hPolicy_)) ex = ex | Qt::Horizontal;
    if (policyExpands(vPolicy_)) ex = ex | Qt::Vertical;
    expanding_ = ex;
    hint_ = QSize(w, h);
    min_ = QSize(policyExpands(hPolicy_) ? w : 0, policyExpands(vPolicy_) ? h : 0);
    max_ = QSize(hPolicy == QSizePolicy::Fixed ? w : 0x7fffffff,
                 vPolicy == QSizePolicy::Fixed ? h : 0x7fffffff);
}

// ---------------------------------------------------------------------------
// QWidgetItem
// ---------------------------------------------------------------------------
QWidgetItem::QWidgetItem(QWidget* widget) : widget_(widget), alignment_(0) {}

QSize QWidgetItem::sizeHint() const {
    if (!widget_) return QSize(0, 0);
    QSize s = widget_->sizeHint();
    if (s.width() < 0) s.setWidth(0);
    if (s.height() < 0) s.setHeight(0);
    return s;
}

QSize QWidgetItem::minimumSize() const {
    if (!widget_) return QSize(0, 0);
    QSize s = widget_->minimumSizeHint();
    QSize fixed = widget_->minimumSize();
    if (fixed.width() > 0) s.setWidth(qMax(s.width(), fixed.width()));
    if (fixed.height() > 0) s.setHeight(qMax(s.height(), fixed.height()));
    QSizePolicy policy = widget_->sizePolicy();
    QSize hint = sizeHint();
    if (policy.verticalPolicy() == QSizePolicy::Fixed) s.setHeight(hint.height());
    if (policy.horizontalPolicy() == QSizePolicy::Fixed) s.setWidth(hint.width());
    return s;
}

QSize QWidgetItem::maximumSize() const {
    if (!widget_) return QSize(0x7fffffff, 0x7fffffff);
    QSize s = widget_->maximumSize();
    QSize hint = sizeHint();
    QSizePolicy policy = widget_->sizePolicy();
    if (policy.verticalPolicy() == QSizePolicy::Fixed || policy.verticalPolicy() == QSizePolicy::Maximum)
        s.setHeight(qMin(s.height(), hint.height()));
    if (policy.horizontalPolicy() == QSizePolicy::Fixed || policy.horizontalPolicy() == QSizePolicy::Maximum)
        s.setWidth(qMin(s.width(), hint.width()));
    return s;
}

Qt::Orientations QWidgetItem::expandingDirections() const {
    if (!widget_) return 0;
    return widget_->sizePolicy().expandingDirections();
}

void QWidgetItem::setGeometry(const QRect& rect) { setGeometry(rect, alignment_); }

void QWidgetItem::setGeometry(const QRect& rect, Qt::Alignment alignment) {
    if (!widget_) return;
    QRect r = rect;
    QSize hint = sizeHint();
    if (alignment & Qt::AlignHorizontal_Mask) {
        int w = qMin(rect.width(), hint.width());
        if (alignment & Qt::AlignRight) r.setLeft(rect.right() - w + 1);
        else if (alignment & Qt::AlignHCenter) r.setLeft(rect.left() + (rect.width() - w) / 2);
        r.setRight(r.left() + w - 1);
    }
    if (alignment & Qt::AlignVertical_Mask) {
        int h = qMin(rect.height(), hint.height());
        if (alignment & Qt::AlignBottom) r.setTop(rect.bottom() - h + 1);
        else if (alignment & Qt::AlignVCenter) r.setTop(rect.top() + (rect.height() - h) / 2);
        r.setBottom(r.top() + h - 1);
    }
    widget_->setGeometry(r);
}

// ---------------------------------------------------------------------------
// QLayout
// ---------------------------------------------------------------------------
QLayout::QLayout(QWidget* parent)
    : QObject(parent), parentWidget_(parent), spacing_(6), enabled_(true), dirty_(true) {
    if (parent) parent->setLayout(this);
}

QLayout::~QLayout() {
    for (int i = 0; i < items_.count(); ++i) delete items_[i];
    items_.clearAll();
    if (parentWidget_ && parentWidget_->layout() == this) parentWidget_->setLayout(0);
}

void QLayout::addWidget(QWidget* widget) {
    if (!widget) return;
    if (parentWidget_ && widget->parentWidget() != parentWidget_) widget->setParentWidget(parentWidget_);
    if (parentWidget_ && parentWidget_->isVisible() && !widget->isVisible() && !widget->isWindow())
        widget->show();
    items_.append(new QWidgetItem(widget));
    invalidate();
}

void QLayout::addItem(QLayoutItem* item) {
    if (!item) return;
    items_.append(item);
    invalidate();
}

void QLayout::addLayout(QLayout* layout, int) {
    if (!layout) return;
    layout->setParentWidget(parentWidget_);
    items_.append(layout);
    invalidate();
}

void QLayout::removeWidget(QWidget* widget) {
    for (int i = items_.count() - 1; i >= 0; --i) {
        if (items_[i]->widget() == widget) {
            delete items_[i];
            items_.removeAt(i);
        }
    }
    invalidate();
}

void QLayout::removeItem(QLayoutItem* item) {
    int idx = items_.indexOf(item);
    if (idx >= 0) items_.removeAt(idx);
    invalidate();
}

QLayoutItem* QLayout::takeAt(int index) {
    if (index < 0 || index >= items_.count()) return 0;
    QLayoutItem* item = items_[index];
    items_.removeAt(index);
    invalidate();
    return item;
}

void QLayout::setContentsMargins(int left, int top, int right, int bottom) {
    margins_ = QMargins(left, top, right, bottom);
    invalidate();
}

void QLayout::getContentsMargins(int* left, int* top, int* right, int* bottom) const {
    if (left) *left = margins_.left();
    if (top) *top = margins_.top();
    if (right) *right = margins_.right();
    if (bottom) *bottom = margins_.bottom();
}

void QLayout::setParentWidget(QWidget* widget) {
    if (parentWidget_ == widget) return;
    parentWidget_ = widget;
    for (int i = 0; i < items_.count(); ++i) {
        QLayoutItem* item = items_[i];
        if (item->widget() && widget) {
            if (item->widget()->parentWidget() != widget) item->widget()->setParentWidget(widget);
            if (widget->isVisible() && !item->widget()->isVisible() && !item->widget()->isWindow())
                item->widget()->show();
        } else if (item->layout()) {
            item->layout()->setParentWidget(widget);
        }
    }
}

void QLayout::setGeometry(const QRect& rect) {
    geometry_ = rect;
    dirty_ = false;
    QRect inner = rect.adjusted(margins_.left(), margins_.top(), -margins_.right(), -margins_.bottom());
    for (int i = 0; i < items_.count(); ++i) {
        QLayoutItem* item = items_[i];
        if (item->isEmpty()) continue;
        item->setGeometry(inner);
    }
}

bool QLayout::isEmpty() const {
    for (int i = 0; i < items_.count(); ++i) if (!items_[i]->isEmpty()) return false;
    return true;
}

void QLayout::activate() {
    if (!parentWidget_) return;
    if (!dirty_ && geometry_.isValid() && geometry_ == parentWidget_->contentsRect()) return;
    QRect r = parentWidget_->contentsRect();
    setGeometry(r);
}

bool QLayout::event(QEvent* e) {
    if (e->type() == QEvent::LayoutRequest || e->type() == QEvent::Resize) activate();
    return QObject::event(e);
}

// ---------------------------------------------------------------------------
// QBoxLayout
// ---------------------------------------------------------------------------
QBoxLayout::QBoxLayout(QWidget* parent, Direction direction)
    : QLayout(0), direction_(direction) {
    if (parent) {
        parent->setLayout(this);
        setParentWidget(parent);
    }
}

QBoxLayout::QBoxLayout(Direction direction) : QLayout(0), direction_(direction) {}

QBoxLayout::~QBoxLayout() {}

void QBoxLayout::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment) {
    if (!widget) return;
    if (parentWidget_ && widget->parentWidget() != parentWidget_) widget->setParentWidget(parentWidget_);
    QWidgetItem* item = new QWidgetItem(widget);
    item->setAlignment(alignment);
    items_.append(item);
    stretches_.append(stretch);
    invalidate();
}

void QBoxLayout::addLayout(QLayout* layout, int stretch) {
    if (!layout) return;
    layout->setParentWidget(parentWidget_);
    items_.append(layout);
    stretches_.append(stretch);
    invalidate();
}

void QBoxLayout::addSpacing(int size) {
    items_.append(new QSpacerItem(size, size, QSizePolicy::Fixed, QSizePolicy::Fixed));
    stretches_.append(0);
    invalidate();
}

void QBoxLayout::addStretch(int stretch) {
    items_.append(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    stretches_.append(stretch);
    invalidate();
}

void QBoxLayout::addStrut(int size) {
    items_.append(new QSpacerItem(0, size, QSizePolicy::Fixed, QSizePolicy::Fixed));
    stretches_.append(0);
    invalidate();
}

void QBoxLayout::setDirection(Direction direction) { direction_ = direction; invalidate(); }

void QBoxLayout::setStretch(int index, int stretch) {
    if (index >= 0 && index < stretches_.count()) stretches_[index] = stretch;
    invalidate();
}

int QBoxLayout::stretch(int index) const {
    return (index >= 0 && index < stretches_.count()) ? stretches_[index] : 0;
}

void QBoxLayout::insertWidget(int index, QWidget* widget, int stretch, Qt::Alignment alignment) {
    if (!widget) return;
    QWidgetItem* item = new QWidgetItem(widget);
    item->setAlignment(alignment);
    items_.insert(index, item);
    stretches_.insert(index, stretch);
    invalidate();
}

QSize QBoxLayout::sizeHint() const {
    QSize total(0, 0);
    int visible = 0;
    for (int i = 0; i < items_.count(); ++i) {
        if (items_[i]->isEmpty()) continue;
        QSize s = items_[i]->sizeHint();
        if (direction_ == LeftToRight || direction_ == RightToLeft) {
            total.setWidth(total.width() + s.width());
            total.setHeight(qMax(total.height(), s.height()));
        } else {
            total.setHeight(total.height() + s.height());
            total.setWidth(qMax(total.width(), s.width()));
        }
        ++visible;
    }
    if (visible > 1) {
        int extra = spacing_ * (visible - 1);
        if (direction_ == LeftToRight || direction_ == RightToLeft) total.setWidth(total.width() + extra);
        else total.setHeight(total.height() + extra);
    }
    total += QSize(margins_.left() + margins_.right(), margins_.top() + margins_.bottom());
    return total;
}

QSize QBoxLayout::minimumSize() const {
    QSize total(0, 0);
    int visible = 0;
    for (int i = 0; i < items_.count(); ++i) {
        if (items_[i]->isEmpty()) continue;
        QSize s = items_[i]->minimumSize();
        if (direction_ == LeftToRight || direction_ == RightToLeft) {
            total.setWidth(total.width() + s.width());
            total.setHeight(qMax(total.height(), s.height()));
        } else {
            total.setHeight(total.height() + s.height());
            total.setWidth(qMax(total.width(), s.width()));
        }
        ++visible;
    }
    if (visible > 1) {
        int extra = spacing_ * (visible - 1);
        if (direction_ == LeftToRight || direction_ == RightToLeft) total.setWidth(total.width() + extra);
        else total.setHeight(total.height() + extra);
    }
    total += QSize(margins_.left() + margins_.right(), margins_.top() + margins_.bottom());
    return total;
}

QSize QBoxLayout::maximumSize() const {
    QSize total(0, 0);
    for (int i = 0; i < items_.count(); ++i) {
        QSize s = items_[i]->maximumSize();
        if (direction_ == LeftToRight || direction_ == RightToLeft) {
            total.setWidth(total.width() + s.width());
            total.setHeight(qMax(total.height(), s.height()));
        } else {
            total.setHeight(total.height() + s.height());
            total.setWidth(qMax(total.width(), s.width()));
        }
    }
    return total;
}

Qt::Orientations QBoxLayout::expandingDirections() const {
    bool horizontal = (direction_ == LeftToRight || direction_ == RightToLeft);
    Qt::Orientations axis = horizontal ? Qt::Horizontal : Qt::Vertical;
    Qt::Orientations ex = 0;
    for (int i = 0; i < items_.count(); ++i) {
        if (items_[i]->expandingDirections() & axis) ex = ex | axis;
    }
    return ex;
}

void QBoxLayout::setGeometry(const QRect& rect) {
    geometry_ = rect;
    dirty_ = false;
    QRect inner = rect.adjusted(margins_.left(), margins_.top(), -margins_.right(), -margins_.bottom());
    bool horizontal = (direction_ == LeftToRight || direction_ == RightToLeft);

    QList<QLayoutItem*> visible;
    QList<int> stretches;
    for (int i = 0; i < items_.count(); ++i) {
        if (items_[i]->isEmpty()) continue;
        visible.append(items_[i]);
        stretches.append(i < stretches_.count() ? stretches_[i] : 0);
    }
    int count = visible.count();
    if (count == 0) return;

    std::vector<int> mins((size_t)count), hints((size_t)count), maxs((size_t)count);
    std::vector<int> stretch((size_t)count);
    std::vector<unsigned char> expanding((size_t)count);
    for (int i = 0; i < count; ++i) {
        QSize mn = visible[i]->minimumSize();
        QSize h = visible[i]->sizeHint();
        QSize mx = visible[i]->maximumSize();
        hints[(size_t)i] = horizontal ? h.width() : h.height();
        mins[(size_t)i] = horizontal ? mn.width() : mn.height();
        maxs[(size_t)i] = horizontal ? mx.width() : mx.height();
        stretch[(size_t)i] = stretches[i];
        Qt::Orientations ex = visible[i]->expandingDirections();
        expanding[(size_t)i] = (ex & (horizontal ? Qt::Horizontal : Qt::Vertical)) != 0 ? 1 : 0;
    }
    std::vector<int> pos((size_t)count), size((size_t)count);
    distribute(horizontal ? inner.width() : inner.height(), count,
               &mins[0], &hints[0], &maxs[0], &stretch[0], &expanding[0], spacing_,
               &pos[0], &size[0]);

    for (int i = 0; i < count; ++i) {
        QRect cell;
        if (horizontal) {
            int left = inner.left() + pos[(size_t)i];
            if (direction_ == RightToLeft) left = inner.right() - pos[(size_t)i] - size[(size_t)i] + 1;
            cell = QRect(left, inner.top(), size[(size_t)i], inner.height());
        } else {
            int top = inner.top() + pos[(size_t)i];
            if (direction_ == BottomToTop) top = inner.bottom() - pos[(size_t)i] - size[(size_t)i] + 1;
            cell = QRect(inner.left(), top, inner.width(), size[(size_t)i]);
        }
        QWidgetItem* widgetItem = dynamic_cast<QWidgetItem*>(visible[i]);
        if (widgetItem) widgetItem->setGeometry(cell, widgetItem->alignment());
        else visible[i]->setGeometry(cell);
    }
}

// ---------------------------------------------------------------------------
// QGridLayout
// ---------------------------------------------------------------------------
QGridLayout::QGridLayout(QWidget* parent) : QLayout(0), hSpacing_(6), vSpacing_(6) {
    if (parent) {
        parent->setLayout(this);
        setParentWidget(parent);
    }
}

QGridLayout::~QGridLayout() {}

void QGridLayout::addWidget(QWidget* widget, int row, int column, int rowSpan, int columnSpan,
                            Qt::Alignment alignment) {
    addItem(new QWidgetItem(widget), row, column, rowSpan, columnSpan);
}

void QGridLayout::addLayout(QLayout* layout, int row, int column, int rowSpan, int columnSpan) {
    addItem(layout, row, column, rowSpan, columnSpan);
}

void QGridLayout::addItem(QLayoutItem* item, int row, int column, int rowSpan, int columnSpan) {
    if (!item) return;
    if (parentWidget_ && item->widget() && item->widget()->parentWidget() != parentWidget_)
        item->widget()->setParentWidget(parentWidget_);
    Cell cell;
    cell.item = item;
    cell.row = row;
    cell.column = column;
    cell.rowSpan = qMax(1, rowSpan);
    cell.columnSpan = qMax(1, columnSpan);
    cell.alignment = 0;
    cells_.append(cell);
    invalidate();
}

void QGridLayout::setColumnStretch(int column, int stretch) {
    while (columnStretch_.count() <= column) columnStretch_.append(0);
    columnStretch_[column] = stretch;
    invalidate();
}

void QGridLayout::setRowStretch(int row, int stretch) {
    while (rowStretch_.count() <= row) rowStretch_.append(0);
    rowStretch_[row] = stretch;
    invalidate();
}

void QGridLayout::setColumnMinimumWidth(int column, int minSize) {
    while (columnMin_.count() <= column) columnMin_.append(0);
    columnMin_[column] = minSize;
    invalidate();
}

void QGridLayout::setRowMinimumHeight(int row, int minSize) {
    while (rowMin_.count() <= row) rowMin_.append(0);
    rowMin_[row] = minSize;
    invalidate();
}

void QGridLayout::setHorizontalSpacing(int spacing) { hSpacing_ = spacing; invalidate(); }
void QGridLayout::setVerticalSpacing(int spacing) { vSpacing_ = spacing; invalidate(); }

int QGridLayout::rowCount() const {
    int n = 0;
    for (int i = 0; i < cells_.count(); ++i) n = qMax(n, cells_[i].row + cells_[i].rowSpan);
    return n;
}

int QGridLayout::columnCount() const {
    int n = 0;
    for (int i = 0; i < cells_.count(); ++i) n = qMax(n, cells_[i].column + cells_[i].columnSpan);
    return n;
}

QSize QGridLayout::sizeHint() const {
    int rows = rowCount(), cols = columnCount();
    std::vector<int> colW((size_t)qMax(cols, 1), 0), rowH((size_t)qMax(rows, 1), 0);
    for (int i = 0; i < cells_.count(); ++i) {
        QSize s = cells_[i].item->sizeHint();
        int perCol = s.width() / cells_[i].columnSpan;
        int perRow = s.height() / cells_[i].rowSpan;
        for (int c = 0; c < cells_[i].columnSpan; ++c)
            colW[(size_t)(cells_[i].column + c)] = qMax(colW[(size_t)(cells_[i].column + c)], perCol);
        for (int r = 0; r < cells_[i].rowSpan; ++r)
            rowH[(size_t)(cells_[i].row + r)] = qMax(rowH[(size_t)(cells_[i].row + r)], perRow);
    }
    int w = 0, h = 0;
    for (int c = 0; c < cols; ++c) w += colW[(size_t)c];
    for (int r = 0; r < rows; ++r) h += rowH[(size_t)r];
    if (cols > 1) w += hSpacing_ * (cols - 1);
    if (rows > 1) h += vSpacing_ * (rows - 1);
    return QSize(w + margins_.left() + margins_.right(), h + margins_.top() + margins_.bottom());
}

QSize QGridLayout::minimumSize() const {
    int rows = rowCount(), cols = columnCount();
    std::vector<int> colW((size_t)qMax(cols, 1), 0), rowH((size_t)qMax(rows, 1), 0);
    for (int i = 0; i < cells_.count(); ++i) {
        QSize s = cells_[i].item->minimumSize();
        for (int c = 0; c < cells_[i].columnSpan; ++c)
            colW[(size_t)(cells_[i].column + c)] = qMax(colW[(size_t)(cells_[i].column + c)], s.width());
        for (int r = 0; r < cells_[i].rowSpan; ++r)
            rowH[(size_t)(cells_[i].row + r)] = qMax(rowH[(size_t)(cells_[i].row + r)], s.height());
    }
    for (int c = 0; c < cols && c < columnMin_.count(); ++c)
        colW[(size_t)c] = qMax(colW[(size_t)c], columnMin_[c]);
    for (int r = 0; r < rows && r < rowMin_.count(); ++r)
        rowH[(size_t)r] = qMax(rowH[(size_t)r], rowMin_[r]);
    int w = 0, h = 0;
    for (int c = 0; c < cols; ++c) w += colW[(size_t)c];
    for (int r = 0; r < rows; ++r) h += rowH[(size_t)r];
    if (cols > 1) w += hSpacing_ * (cols - 1);
    if (rows > 1) h += vSpacing_ * (rows - 1);
    return QSize(w + margins_.left() + margins_.right(), h + margins_.top() + margins_.bottom());
}

Qt::Orientations QGridLayout::expandingDirections() const {
    Qt::Orientations ex = 0;
    for (int i = 0; i < cells_.count(); ++i) ex = ex | cells_[i].item->expandingDirections();
    return ex;
}

void QGridLayout::setGeometry(const QRect& rect) {
    geometry_ = rect;
    dirty_ = false;
    QRect inner = rect.adjusted(margins_.left(), margins_.top(), -margins_.right(), -margins_.bottom());
    int rows = rowCount(), cols = columnCount();
    if (rows == 0 || cols == 0) return;

    std::vector<int> colMin((size_t)cols, 0), colHint((size_t)cols, 0), colMax((size_t)cols, 0x7fffffff);
    std::vector<int> rowMin((size_t)rows, 0), rowHint((size_t)rows, 0), rowMax((size_t)rows, 0x7fffffff);
    std::vector<int> colStretch((size_t)cols, 0), rowStretch((size_t)rows, 0);
    std::vector<unsigned char> colExp((size_t)cols, 0), rowExp((size_t)rows, 0);

    for (int c = 0; c < cols; ++c) {
        if (c < columnStretch_.count()) colStretch[(size_t)c] = columnStretch_[c];
        if (c < columnMin_.count()) colMin[(size_t)c] = columnMin_[c];
    }
    for (int r = 0; r < rows; ++r) {
        if (r < rowStretch_.count()) rowStretch[(size_t)r] = rowStretch_[r];
        if (r < rowMin_.count()) rowMin[(size_t)r] = rowMin_[r];
    }

    for (int i = 0; i < cells_.count(); ++i) {
        const Cell& cell = cells_[i];
        QSize hint = cell.item->sizeHint();
        QSize mn = cell.item->minimumSize();
        Qt::Orientations ex = cell.item->expandingDirections();
        int perW = hint.width() / cell.columnSpan;
        int perH = hint.height() / cell.rowSpan;
        int minPerW = mn.width() / cell.columnSpan;
        int minPerH = mn.height() / cell.rowSpan;
        for (int c = 0; c < cell.columnSpan; ++c) {
            int idx = cell.column + c;
            colHint[(size_t)idx] = qMax(colHint[(size_t)idx], perW);
            colMin[(size_t)idx] = qMax(colMin[(size_t)idx], minPerW);
            if (ex & Qt::Horizontal) colExp[(size_t)idx] = 1;
        }
        for (int r = 0; r < cell.rowSpan; ++r) {
            int idx = cell.row + r;
            rowHint[(size_t)idx] = qMax(rowHint[(size_t)idx], perH);
            rowMin[(size_t)idx] = qMax(rowMin[(size_t)idx], minPerH);
            if (ex & Qt::Vertical) rowExp[(size_t)idx] = 1;
        }
    }

    std::vector<int> colPos((size_t)cols), colSize((size_t)cols);
    std::vector<int> rowPos((size_t)rows), rowSize((size_t)rows);
    distribute(inner.width(), cols, &colMin[0], &colHint[0], &colMax[0], &colStretch[0], &colExp[0],
               hSpacing_, &colPos[0], &colSize[0]);
    distribute(inner.height(), rows, &rowMin[0], &rowHint[0], &rowMax[0], &rowStretch[0], &rowExp[0],
               vSpacing_, &rowPos[0], &rowSize[0]);

    for (int i = 0; i < cells_.count(); ++i) {
        const Cell& cell = cells_[i];
        int left = inner.left() + colPos[(size_t)cell.column];
        int top = inner.top() + rowPos[(size_t)cell.row];
        int w = 0, h = 0;
        for (int c = 0; c < cell.columnSpan; ++c) w += colSize[(size_t)(cell.column + c)];
        for (int r = 0; r < cell.rowSpan; ++r) h += rowSize[(size_t)(cell.row + r)];
        w += hSpacing_ * (cell.columnSpan - 1);
        h += vSpacing_ * (cell.rowSpan - 1);
        QRect cellRect(left, top, w, h);
        QWidgetItem* wi = dynamic_cast<QWidgetItem*>(cell.item);
        if (wi) wi->setGeometry(cellRect, wi->alignment());
        else cell.item->setGeometry(cellRect);
    }
}

// ---------------------------------------------------------------------------
// QFormLayout
// ---------------------------------------------------------------------------
QFormLayout::QFormLayout(QWidget* parent) : QLayout(0), labelWidth_(0) {
    if (parent) {
        parent->setLayout(this);
        setParentWidget(parent);
    }
}

QFormLayout::~QFormLayout() {}

void QFormLayout::addRow(const QString& labelText, QWidget* field) {
    QWidget* label = 0;
    if (!labelText.isEmpty()) {
        label = new QLabel(labelText, parentWidget_);
    }
    addRow(label, field);
}

void QFormLayout::addRow(QWidget* label, QWidget* field) {
    if (field && parentWidget_ && field->parentWidget() != parentWidget_)
        field->setParentWidget(parentWidget_);
    Row row;
    row.label = label;
    row.field = new QWidgetItem(field);
    rows_.append(row);
    invalidate();
}

void QFormLayout::addRow(const QString& labelText, QLayout* field) {
    Row row;
    row.label = labelText.isEmpty() ? 0 : new QLabel(labelText, parentWidget_);
    row.field = field;
    rows_.append(row);
    invalidate();
}

void QFormLayout::addRow(QWidget* widget) {
    Row row;
    row.label = 0;
    row.field = new QWidgetItem(widget);
    rows_.append(row);
    invalidate();
}

int QFormLayout::rowCount() const { return rows_.count(); }

QSize QFormLayout::sizeHint() const {
    int labelW = 0, fieldW = 0, totalH = 0;
    for (int i = 0; i < rows_.count(); ++i) {
        if (rows_[i].label) labelW = qMax(labelW, rows_[i].label->sizeHint().width());
        if (rows_[i].field) {
            QSize s = rows_[i].field->sizeHint();
            fieldW = qMax(fieldW, s.width());
            totalH += qMax(s.height(), rows_[i].label ? rows_[i].label->sizeHint().height() : 0);
            totalH += spacing_;
        }
    }
    return QSize(labelW + fieldW + spacing_ + margins_.left() + margins_.right(),
                 totalH + margins_.top() + margins_.bottom());
}

QSize QFormLayout::minimumSize() const {
    int labelW = 0, fieldW = 0, totalH = 0;
    for (int i = 0; i < rows_.count(); ++i) {
        if (rows_[i].label) labelW = qMax(labelW, rows_[i].label->minimumSizeHint().width());
        if (rows_[i].field) {
            QSize s = rows_[i].field->minimumSize();
            fieldW = qMax(fieldW, s.width());
            totalH += s.height() + spacing_;
        }
    }
    return QSize(labelW + fieldW + spacing_ + margins_.left() + margins_.right(),
                 totalH + margins_.top() + margins_.bottom());
}

void QFormLayout::setGeometry(const QRect& rect) {
    geometry_ = rect;
    dirty_ = false;
    QRect inner = rect.adjusted(margins_.left(), margins_.top(), -margins_.right(), -margins_.bottom());
    int labelW = 0;
    for (int i = 0; i < rows_.count(); ++i) {
        if (rows_[i].label) labelW = qMax(labelW, rows_[i].label->sizeHint().width());
    }
    labelWidth_ = labelW;
    int y = inner.top();
    for (int i = 0; i < rows_.count(); ++i) {
        int rowH = 0;
        if (rows_[i].field) rowH = qMax(rowH, rows_[i].field->sizeHint().height());
        if (rows_[i].label) rowH = qMax(rowH, rows_[i].label->sizeHint().height());
        rowH = qMax(rowH, 22);
        if (rows_[i].label) {
            QRect lr(inner.left(), y, labelW, rowH);
            rows_[i].label->setGeometry(lr);
        }
        if (rows_[i].field) {
            QRect fr(inner.left() + labelW + spacing_, y, inner.width() - labelW - spacing_, rowH);
            rows_[i].field->setGeometry(fr);
        }
        y += rowH + spacing_;
    }
}

// ---------------------------------------------------------------------------
// QStackedLayout
// ---------------------------------------------------------------------------
QStackedLayout::QStackedLayout(QWidget* parent) : QLayout(0), currentIndex_(-1) {
    if (parent) {
        parent->setLayout(this);
        setParentWidget(parent);
    }
}

QStackedLayout::~QStackedLayout() {}

void QStackedLayout::addWidget(QWidget* widget) {
    if (!widget) return;
    if (parentWidget_ && widget->parentWidget() != parentWidget_) widget->setParentWidget(parentWidget_);
    items_.append(new QWidgetItem(widget));
    widget->hide();
    if (currentIndex_ < 0) setCurrentIndex(0);
    invalidate();
}

void QStackedLayout::setCurrentIndex(int index) {
    if (index < -1 || index >= items_.count()) return;
    if (currentIndex_ == index) return;
    if (currentIndex_ >= 0 && currentIndex_ < items_.count()) {
        QWidget* old = items_[currentIndex_]->widget();
        if (old) old->hide();
    }
    currentIndex_ = index;
    if (index >= 0) {
        QWidget* w = items_[index]->widget();
        if (w) {
            w->show();
            w->raise();
            w->setGeometry(geometry_);
        }
    }
    invalidate();
}

QWidget* QStackedLayout::currentWidget() const {
    if (currentIndex_ < 0 || currentIndex_ >= items_.count()) return 0;
    return items_[currentIndex_]->widget();
}

int QStackedLayout::countVisible() const {
    int n = 0;
    for (int i = 0; i < items_.count(); ++i) if (!items_[i]->isEmpty()) ++n;
    return n;
}

QSize QStackedLayout::sizeHint() const {
    QSize best(0, 0);
    for (int i = 0; i < items_.count(); ++i) {
        QSize s = items_[i]->sizeHint();
        best.setWidth(qMax(best.width(), s.width()));
        best.setHeight(qMax(best.height(), s.height()));
    }
    return best + QSize(margins_.left() + margins_.right(), margins_.top() + margins_.bottom());
}

QSize QStackedLayout::minimumSize() const {
    QSize best(0, 0);
    for (int i = 0; i < items_.count(); ++i) {
        QSize s = items_[i]->minimumSize();
        best.setWidth(qMax(best.width(), s.width()));
        best.setHeight(qMax(best.height(), s.height()));
    }
    return best + QSize(margins_.left() + margins_.right(), margins_.top() + margins_.bottom());
}

void QStackedLayout::setGeometry(const QRect& rect) {
    geometry_ = rect;
    dirty_ = false;
    QRect inner = rect.adjusted(margins_.left(), margins_.top(), -margins_.right(), -margins_.bottom());
    for (int i = 0; i < items_.count(); ++i) {
        if (i == currentIndex_) items_[i]->setGeometry(inner);
        else items_[i]->setGeometry(QRect(inner.topLeft(), QSize(0, 0)));
    }
}
