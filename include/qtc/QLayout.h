// QtClone - QLayout.h : layout items and box/grid/form/stacked layouts.
#ifndef QTC_QLAYOUT_H
#define QTC_QLAYOUT_H

#include <qtc/QWidget.h>

class QLayout;
class QWidgetItem;

// ---------------------------------------------------------------------------
// QLayoutItem
// ---------------------------------------------------------------------------
class QLayoutItem {
public:
    virtual ~QLayoutItem() {}
    virtual QSize sizeHint() const = 0;
    virtual QSize minimumSize() const = 0;
    virtual QSize maximumSize() const = 0;
    virtual Qt::Orientations expandingDirections() const = 0;
    virtual void setGeometry(const QRect& rect) = 0;
    virtual QRect geometry() const = 0;
    virtual bool isEmpty() const = 0;
    virtual QWidget* widget() const { return 0; }
    virtual QLayout* layout() { return 0; }
    virtual Qt::Alignment alignment() const { return 0; }
    virtual void setAlignment(Qt::Alignment alignment) { Q_UNUSED(alignment); }
};

class QSpacerItem : public QLayoutItem {
public:
    QSpacerItem(int w, int h, QSizePolicy::Policy hPolicy = QSizePolicy::Minimum,
                QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);
    QSize sizeHint() const { return hint_; }
    QSize minimumSize() const { return min_; }
    QSize maximumSize() const { return max_; }
    Qt::Orientations expandingDirections() const { return expanding_; }
    void setGeometry(const QRect& rect) { rect_ = rect; }
    QRect geometry() const { return rect_; }
    bool isEmpty() const { return true; }
    void changeSize(int w, int h, QSizePolicy::Policy hPolicy = QSizePolicy::Minimum,
                    QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);
private:
    QSize hint_;
    QSize min_;
    QSize max_;
    QSizePolicy::Policy hPolicy_;
    QSizePolicy::Policy vPolicy_;
    Qt::Orientations expanding_;
    QRect rect_;
};

class QWidgetItem : public QLayoutItem {
public:
    explicit QWidgetItem(QWidget* widget);
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    Qt::Orientations expandingDirections() const;
    void setGeometry(const QRect& rect);
    QRect geometry() const { return rect_; }
    bool isEmpty() const { return widget_ == 0 || widget_->isHidden(); }
    QWidget* widget() const { return widget_; }
    Qt::Alignment alignment() const { return alignment_; }
    void setAlignment(Qt::Alignment alignment) { alignment_ = alignment; }
    void setGeometry(const QRect& rect, Qt::Alignment alignment);
private:
    QWidget* widget_;
    QRect rect_;
    Qt::Alignment alignment_;
};

// ---------------------------------------------------------------------------
// QLayout
// ---------------------------------------------------------------------------
class QLayout : public QObject, public QLayoutItem {
public:
    explicit QLayout(QWidget* parent = 0);
    ~QLayout();

    void addWidget(QWidget* widget);
    void addItem(QLayoutItem* item);
    void addLayout(QLayout* layout, int stretch = 0);
    void removeWidget(QWidget* widget);
    void removeItem(QLayoutItem* item);

    int count() const { return items_.count(); }
    QLayoutItem* itemAt(int index) const { return (index >= 0 && index < items_.count()) ? items_[index] : 0; }
    QLayoutItem* takeAt(int index);

    QMargins contentsMargins() const { return margins_; }
    void setContentsMargins(int left, int top, int right, int bottom);
    void setContentsMargins(const QMargins& m) { setContentsMargins(m.left(), m.top(), m.right(), m.bottom()); }
    void getContentsMargins(int* left, int* top, int* right, int* bottom) const;

    int spacing() const { return spacing_; }
    void setSpacing(int spacing) { spacing_ = spacing; invalidate(); }

    virtual QSize sizeHint() const = 0;
    virtual QSize minimumSize() const = 0;
    virtual QSize maximumSize() const { return QSize(0x7fffffff, 0x7fffffff); }
    virtual Qt::Orientations expandingDirections() const { return 0; }
    virtual void setGeometry(const QRect& rect);
    virtual QRect geometry() const { return geometry_; }
    virtual bool isEmpty() const;
    virtual bool hasHeightForWidth() const { return false; }
    virtual int heightForWidth(int width) const { Q_UNUSED(width); return -1; }

    void activate();
    void invalidate() { dirty_ = true; }
    void update() { activate(); }
    QWidget* parentWidget() const { return parentWidget_; }
    void setParentWidget(QWidget* widget);
    bool isEnabled() const { return enabled_; }
    void setEnabled(bool enabled) { enabled_ = enabled; }

    virtual bool event(QEvent* event);

protected:
    QList<QLayoutItem*> items_;
    QWidget* parentWidget_;
    QMargins margins_;
    int spacing_;
    bool enabled_;
    bool dirty_;
    QRect geometry_;
};

// ---------------------------------------------------------------------------
// QBoxLayout
// ---------------------------------------------------------------------------
class QBoxLayout : public QLayout {
public:
    enum Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop };

    QBoxLayout(QWidget* parent, Direction direction);
    explicit QBoxLayout(Direction direction);
    ~QBoxLayout();

    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = 0);
    void addLayout(QLayout* layout, int stretch = 0);
    void addSpacing(int size);
    void addStretch(int stretch = 0);
    void addStrut(int size);

    void setDirection(Direction direction);
    Direction direction() const { return direction_; }
    void setStretch(int index, int stretch);
    int stretch(int index) const;
    void insertWidget(int index, QWidget* widget, int stretch = 0, Qt::Alignment alignment = 0);

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    Qt::Orientations expandingDirections() const;
    void setGeometry(const QRect& rect);
    bool hasHeightForWidth() const { return false; }

private:
    Direction direction_;
    QList<int> stretches_;
    QList<QLayoutItem*> pendingItems_;
};

class QHBoxLayout : public QBoxLayout {
public:
    explicit QHBoxLayout(QWidget* parent = 0) : QBoxLayout(parent, LeftToRight) {}
};

class QVBoxLayout : public QBoxLayout {
public:
    explicit QVBoxLayout(QWidget* parent = 0) : QBoxLayout(parent, TopToBottom) {}
};

// ---------------------------------------------------------------------------
// QGridLayout
// ---------------------------------------------------------------------------
class QGridLayout : public QLayout {
public:
    explicit QGridLayout(QWidget* parent = 0);
    ~QGridLayout();

    void addWidget(QWidget* widget, int row, int column, int rowSpan = 1, int columnSpan = 1,
                   Qt::Alignment alignment = 0);
    void addLayout(QLayout* layout, int row, int column, int rowSpan = 1, int columnSpan = 1);
    void addItem(QLayoutItem* item, int row, int column, int rowSpan = 1, int columnSpan = 1);

    void setColumnStretch(int column, int stretch);
    void setRowStretch(int row, int stretch);
    void setColumnMinimumWidth(int column, int minSize);
    void setRowMinimumHeight(int row, int minSize);
    void setHorizontalSpacing(int spacing);
    void setVerticalSpacing(int spacing);

    QSize sizeHint() const;
    QSize minimumSize() const;
    Qt::Orientations expandingDirections() const;
    void setGeometry(const QRect& rect);
    int rowCount() const;
    int columnCount() const;

private:
    struct Cell {
        QLayoutItem* item;
        int row, column, rowSpan, columnSpan;
        Qt::Alignment alignment;
    };
    QList<Cell> cells_;
    QList<int> rowStretch_;
    QList<int> columnStretch_;
    QList<int> rowMin_;
    QList<int> columnMin_;
    int hSpacing_;
    int vSpacing_;
};

// ---------------------------------------------------------------------------
// QFormLayout
// ---------------------------------------------------------------------------
class QFormLayout : public QLayout {
public:
    explicit QFormLayout(QWidget* parent = 0);
    ~QFormLayout();

    void addRow(const QString& labelText, QWidget* field);
    void addRow(const QString& labelText, QLayout* field);
    void addRow(QWidget* label, QWidget* field);
    void addRow(QWidget* widget);

    QSize sizeHint() const;
    QSize minimumSize() const;
    void setGeometry(const QRect& rect);
    int rowCount() const;

private:
    struct Row {
        QWidget* label;
        QLayoutItem* field;
    };
    QList<Row> rows_;
    int labelWidth_;
};

// ---------------------------------------------------------------------------
// QStackedLayout
// ---------------------------------------------------------------------------
class QStackedLayout : public QLayout {
public:
    explicit QStackedLayout(QWidget* parent = 0);
    ~QStackedLayout();

    void addWidget(QWidget* widget);
    void setCurrentIndex(int index);
    int currentIndex() const { return currentIndex_; }
    QWidget* currentWidget() const;
    int countVisible() const;

    QSize sizeHint() const;
    QSize minimumSize() const;
    void setGeometry(const QRect& rect);

private:
    int currentIndex_;
};

#endif // QTC_QLAYOUT_H
