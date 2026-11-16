// QtClone - QtWidgets.h : concrete widgets (the <QtWidgets> module header).
#ifndef QTC_QTWIDGETS_H
#define QTC_QTWIDGETS_H

#include <qtc/QWidget.h>
#include <qtc/QLayout.h>
#include <qtc/QStyle.h>

class QAbstractButton;
class QMenu;
class QMenuBar;
class QToolBar;
class QStatusBar;
class QAction;
class QListWidgetItem;
class QTreeWidgetItem;
class QTableWidgetItem;
class QScrollBar;

// ---------------------------------------------------------------------------
// QFrame
// ---------------------------------------------------------------------------
class QFrame : public QWidget {
    QTC_OBJECT(QFrame, QWidget)
public:
    enum Shape { NoFrame, Box, Panel, StyledPanel, HLine, VLine, WinPanel };
    enum Shadow { Plain, Raised, Sunken };

    explicit QFrame(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QFrame();

    Shape frameShape() const { return shape_; }
    void setFrameShape(Shape shape);
    Shadow frameShadow() const { return shadow_; }
    void setFrameShadow(Shadow shadow);
    int lineWidth() const { return lineWidth_; }
    void setLineWidth(int width);
    int midLineWidth() const { return midLineWidth_; }
    void setMidLineWidth(int width);
    QRect frameRect() const;
    void setFrameRect(const QRect& r) { frameRect_ = r; update(); }

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent* event);
    QRect frameRect_;

private:
    Shape shape_;
    Shadow shadow_;
    int lineWidth_;
    int midLineWidth_;
};

// ---------------------------------------------------------------------------
// QLabel
// ---------------------------------------------------------------------------
class QLabel : public QFrame {
    QTC_OBJECT(QLabel, QFrame)
public:
    explicit QLabel(QWidget* parent = 0, Qt::WindowFlags f = 0);
    explicit QLabel(const QString& text, QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QLabel();

    QString text() const { return text_; }
    void setText(const QString& text);
    const QPixmap& pixmap() const { return pixmap_; }
    void setPixmap(const QPixmap& pixmap);
    Qt::Alignment alignment() const { return alignment_; }
    void setAlignment(Qt::Alignment alignment);
    bool wordWrap() const { return wordWrap_; }
    void setWordWrap(bool on);
    int margin() const { return margin_; }
    void setMargin(int margin);
    int indent() const { return indent_; }
    void setIndent(int indent);
    bool isTextSelectable() const { return selectable_; }
    void setTextInteractionFlags(bool selectable);
    QString selectedText() const;
    void clear();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

signals:
    Signal<void()> linkActivated;
    Signal<void()> linkHovered;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    QString text_;
    QPixmap pixmap_;
    Qt::Alignment alignment_;
    bool wordWrap_;
    int margin_;
    int indent_;
    bool selectable_;
    int selStart_;
    int selEnd_;
    bool selecting_;
    QPoint selectAnchor_;
    int elideWidth_;
};

// ---------------------------------------------------------------------------
// QAbstractButton
// ---------------------------------------------------------------------------
class QAbstractButton : public QWidget {
    QTC_OBJECT(QAbstractButton, QWidget)
public:
    explicit QAbstractButton(QWidget* parent = 0);
    ~QAbstractButton();

    QString text() const { return text_; }
    void setText(const QString& text);
    QIcon icon() const { return icon_; }
    void setIcon(const QIcon& icon);
    QSize iconSize() const { return iconSize_; }
    void setIconSize(const QSize& size);
    bool isCheckable() const { return checkable_; }
    void setCheckable(bool on);
    bool isChecked() const { return checked_; }
    void setChecked(bool on);
    bool autoExclusive() const { return autoExclusive_; }
    void setAutoExclusive(bool on) { autoExclusive_ = on; }
    bool isDown() const { return down_; }
    void setDown(bool on) { down_ = on; update(); }
    bool isDefault() const { return defaultButton_; }
    void setDefault(bool on) { defaultButton_ = on; update(); }
    bool isFlat() const { return flat_; }
    void setFlat(bool on) { flat_ = on; update(); }
    void click();
    void toggle();
    void animateClick(int msec = 100);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;


signals:
    Signal<void(bool)> clicked;
    Signal<void()> pressed;
    Signal<void()> released;
    Signal<void(bool)> toggled;

protected:
    virtual void drawButton(QPainter* painter);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void changeEvent(QEvent* event);
    void checkStateSet();

    QString text_;
    QIcon icon_;
    QSize iconSize_;
    bool checkable_;
    bool checked_;
    bool autoExclusive_;
    bool down_;
    bool defaultButton_;
    bool flat_;
    bool hover_;
    bool pressInProgress_;
    bool keyboardPress_;
};

class QPushButton : public QAbstractButton {
    QTC_OBJECT(QPushButton, QAbstractButton)
public:
    explicit QPushButton(QWidget* parent = 0);
    explicit QPushButton(const QString& text, QWidget* parent = 0);
    ~QPushButton();
    QMenu* menu() const { return menu_; }
    void setMenu(QMenu* menu);
    bool autoDefault() const { return autoDefault_; }
    void setAutoDefault(bool on) { autoDefault_ = on; }

protected:
    void drawButton(QPainter* painter);
    void paintEvent(QPaintEvent* event);

private:
    QMenu* menu_;
    bool autoDefault_;
};

class QToolButton : public QAbstractButton {
    QTC_OBJECT(QToolButton, QAbstractButton)
public:
    explicit QToolButton(QWidget* parent = 0);
    ~QToolButton();
    Qt::ToolButtonStyle toolButtonStyle() const { return toolStyle_; }
    void setToolButtonStyle(Qt::ToolButtonStyle style) { toolStyle_ = style; update(); }
    bool autoRaise() const { return autoRaise_; }
    void setAutoRaise(bool on) { autoRaise_ = on; update(); }

protected:
    void drawButton(QPainter* painter);
    void paintEvent(QPaintEvent* event);

private:
    Qt::ToolButtonStyle toolStyle_;
    bool autoRaise_;
};

class QCheckBox : public QAbstractButton {
    QTC_OBJECT(QCheckBox, QAbstractButton)
public:
    explicit QCheckBox(QWidget* parent = 0);
    explicit QCheckBox(const QString& text, QWidget* parent = 0);
    ~QCheckBox();
    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);
    bool isTristate() const { return tristate_; }
    void setTristate(bool on) { tristate_ = on; }

signals:
    Signal<void(int)> stateChanged;

protected:
    void drawButton(QPainter* painter);
    void paintEvent(QPaintEvent* event);
    void checkStateSet();
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    Qt::CheckState state_;
    bool tristate_;
};

class QRadioButton : public QAbstractButton {
    QTC_OBJECT(QRadioButton, QAbstractButton)
public:
    explicit QRadioButton(QWidget* parent = 0);
    explicit QRadioButton(const QString& text, QWidget* parent = 0);
    ~QRadioButton();

protected:
    void drawButton(QPainter* painter);
    void paintEvent(QPaintEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
};

// ---------------------------------------------------------------------------
// QLineEdit
// ---------------------------------------------------------------------------
class QLineEdit : public QWidget {
    QTC_OBJECT(QLineEdit, QWidget)
public:
    enum EchoMode { Normal, NoEcho, Password, PasswordEchoOnEdit };

    explicit QLineEdit(QWidget* parent = 0);
    explicit QLineEdit(const QString& text, QWidget* parent = 0);
    ~QLineEdit();

    QString text() const { return text_; }
    void setText(const QString& text);
    QString displayText() const;
    QString placeholderText() const { return placeholder_; }
    void setPlaceholderText(const QString& text);
    bool isReadOnly() const { return readOnly_; }
    void setReadOnly(bool on);
    EchoMode echoMode() const { return echoMode_; }
    void setEchoMode(EchoMode mode);
    int maxLength() const { return maxLength_; }
    void setMaxLength(int length);
    Qt::Alignment alignment() const { return alignment_; }
    void setAlignment(Qt::Alignment alignment);
    int cursorPosition() const { return cursor_; }
    void setCursorPosition(int pos);
    bool hasSelectedText() const { return selStart_ != selEnd_; }
    QString selectedText() const;
    int selectionStart() const { return qMin(selStart_, selEnd_); }
    void selectAll();
    void deselect();
    void clear();
    bool isModified() const { return modified_; }
    void setModified(bool on) { modified_ = on; }
    bool isClearButtonEnabled() const { return clearButton_; }
    void setClearButtonEnabled(bool on) { clearButton_ = on; update(); }

    void copy();
    void cut();
    void paste();
    void undo();
    void redo();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;


signals:
    Signal<void(const QString&)> textChanged;
    Signal<void(const QString&)> textEdited;
    Signal<void()> returnPressed;
    Signal<void()> editingFinished;
    Signal<void()> selectionChanged;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void inputMethodEvent(QInputMethodEvent* event);
    void changeEvent(QEvent* event);
    void timerEvent(QTimerEvent* event);

private:
    void ensureCursorVisible();
    int xForPosition(int pos) const;
    int positionForX(int x) const;
    void insertText(const QString& text);
    void deleteSelection();
    void updateGeometryForText();
    void startBlink();
    void stopBlink();
    void drawTextArea(QPainter* painter);

    QString text_;
    QString placeholder_;
    int cursor_;
    int selStart_;
    int selEnd_;
    int scrollOffset_;
    bool readOnly_;
    EchoMode echoMode_;
    int maxLength_;
    Qt::Alignment alignment_;
    bool modified_;
    bool clearButton_;
    bool cursorVisible_;
    bool cursorBlink_;
    int blinkTimer_;
    bool selecting_;
    int selectAnchor_;
    QString imePreedit_;
};

// ---------------------------------------------------------------------------
// QTextEdit
// ---------------------------------------------------------------------------
class QTextEdit : public QWidget {
    QTC_OBJECT(QTextEdit, QWidget)
public:
    explicit QTextEdit(QWidget* parent = 0);
    explicit QTextEdit(const QString& text, QWidget* parent = 0);
    ~QTextEdit();

    QString toPlainText() const { return text_; }
    void setPlainText(const QString& text);
    void insertPlainText(const QString& text);
    void append(const QString& text);
    void clear();

    bool isReadOnly() const { return readOnly_; }
    void setReadOnly(bool on) { readOnly_ = on; update(); }
    bool wordWrap() const { return wordWrap_; }
    void setWordWrap(bool on) { wordWrap_ = on; rebuildLayout(); update(); }
    QString placeholderText() const { return placeholder_; }
    void setPlaceholderText(const QString& text) { placeholder_ = text; update(); }

    int cursorPosition() const { return cursor_; }
    void setCursorPosition(int pos);
    bool hasSelectedText() const { return anchor_ != cursor_; }
    QString selectedText() const;
    void selectAll();
    void copy();
    void cut();
    void paste();
    void undo();
    void redo();

    QScrollBar* verticalScrollBar() const { return vbar_; }
    QSize sizeHint() const;

signals:
    Signal<void()> textChanged;
    Signal<void()> cursorPositionChanged;

protected:
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void inputMethodEvent(QInputMethodEvent* event);
    void resizeEvent(QResizeEvent* event);
    void timerEvent(QTimerEvent* event);

private:
    struct VisualLine {
        int start;
        int end;
    };

    void rebuildLayout();
    int lineIndexAt(int pos) const;
    int posAtPoint(const QPoint& point) const;
    int xForPos(int pos) const;
    QRect caretRect() const;
    void ensureCursorVisible();
    void setCursor(int pos, bool keepAnchor);
    void insertText(const QString& text);
    void deleteSelection();
    void pushUndo();
    void updateScrollBar();
    int contentWidth() const;
    void startBlinkTimer();
    void onScrollBarMoved(int value);
    int textLeft() const { return 6; }
    int textTop() const { return 6; }

    QString text_;
    QString placeholder_;
    QString imePreedit_;
    int cursor_;
    int anchor_;
    bool readOnly_;
    bool wordWrap_;
    int scrollY_;
    int blinkTimer_;
    bool cursorVisible_;
    bool selecting_;
    int lineHeight_;
    QList<VisualLine> lines_;
    QScrollBar* vbar_;
    QList<QString> undoStack_;
    QList<int> undoCursors_;
    QList<QString> redoStack_;
    QList<int> redoCursors_;
};

// ---------------------------------------------------------------------------
// QProgressBar
// ---------------------------------------------------------------------------
class QProgressBar : public QWidget {
    QTC_OBJECT(QProgressBar, QWidget)
public:
    explicit QProgressBar(QWidget* parent = 0);
    ~QProgressBar();
    int minimum() const { return minimum_; }
    void setMinimum(int value) { minimum_ = value; update(); }
    int maximum() const { return maximum_; }
    void setMaximum(int value) { maximum_ = value; update(); }
    int value() const { return value_; }
    void setValue(int value);
    void setRange(int minimum, int maximum);
    bool isTextVisible() const { return textVisible_; }
    void setTextVisible(bool on) { textVisible_ = on; update(); }
    Qt::Orientation orientation() const { return orientation_; }
    void setOrientation(Qt::Orientation o) { orientation_ = o; update(); }
    QString format() const { return format_; }
    void setFormat(const QString& format) { format_ = format; update(); }
    QString text() const;
    bool isInvertedAppearance() const { return inverted_; }
    void setInvertedAppearance(bool on) { inverted_ = on; update(); }

    QSize sizeHint() const;

signals:
    Signal<void(int)> valueChanged;

protected:
    void paintEvent(QPaintEvent* event);

private:
    int minimum_;
    int maximum_;
    int value_;
    bool textVisible_;
    Qt::Orientation orientation_;
    QString format_;
    bool inverted_;
};

// ---------------------------------------------------------------------------
// QSlider / QScrollBar
// ---------------------------------------------------------------------------
class QSlider : public QWidget {
    QTC_OBJECT(QSlider, QWidget)
public:
    explicit QSlider(QWidget* parent = 0);
    explicit QSlider(Qt::Orientation orientation, QWidget* parent = 0);
    ~QSlider();
    int minimum() const { return minimum_; }
    void setMinimum(int value) { minimum_ = value; update(); }
    int maximum() const { return maximum_; }
    void setMaximum(int value) { maximum_ = value; update(); }
    void setRange(int minimum, int maximum);
    int value() const { return value_; }
    void setValue(int value);
    int singleStep() const { return singleStep_; }
    void setSingleStep(int step) { singleStep_ = step; }
    int pageStep() const { return pageStep_; }
    void setPageStep(int step) { pageStep_ = step; }
    Qt::Orientation orientation() const { return orientation_; }
    void setOrientation(Qt::Orientation o) { orientation_ = o; update(); }
    bool tickPosition() const { return ticks_; }
    void setTickPosition(bool on) { ticks_ = on; update(); }
    bool tracking() const { return tracking_; }
    void setTracking(bool on) { tracking_ = on; }

    QSize sizeHint() const;

signals:
    Signal<void(int)> valueChanged;
    Signal<void(int)> sliderMoved;
    Signal<void()> sliderPressed;
    Signal<void()> sliderReleased;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    void setValueInternal(int value, bool move);

    int minimum_;
    int maximum_;
    int value_;
    int singleStep_;
    int pageStep_;
    Qt::Orientation orientation_;
    bool ticks_;
    bool tracking_;
    bool dragging_;
    int dragOffset_;
};

class QScrollBar : public QWidget {
    QTC_OBJECT(QScrollBar, QWidget)
public:
    explicit QScrollBar(QWidget* parent = 0);
    explicit QScrollBar(Qt::Orientation orientation, QWidget* parent = 0);
    ~QScrollBar();
    int minimum() const { return minimum_; }
    void setMinimum(int value) { minimum_ = value; update(); }
    int maximum() const { return maximum_; }
    void setMaximum(int value) { maximum_ = value; update(); }
    void setRange(int minimum, int maximum);
    int value() const { return value_; }
    void setValue(int value);
    int pageStep() const { return pageStep_; }
    void setPageStep(int step) { pageStep_ = step; update(); }
    int singleStep() const { return singleStep_; }
    void setSingleStep(int step) { singleStep_ = step; }
    Qt::Orientation orientation() const { return orientation_; }
    void setOrientation(Qt::Orientation o) { orientation_ = o; update(); }
    bool isSliderDown() const { return dragging_; }

    QSize sizeHint() const;

signals:
    Signal<void(int)> valueChanged;
    Signal<void()> sliderPressed;
    Signal<void()> sliderReleased;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);

private:
    QRect grooveRect() const;
    QRect sliderRect() const;
    int partAt(const QPoint& pos) const;
    int pixelToValue(int pixel) const;

    int minimum_;
    int maximum_;
    int value_;
    int pageStep_;
    int singleStep_;
    Qt::Orientation orientation_;
    bool dragging_;
    int dragOffset_;
    int pressedPart_;
    int hoverPart_;
};

// ---------------------------------------------------------------------------
// QSpinBox / QDoubleSpinBox
// ---------------------------------------------------------------------------
class QSpinBox : public QWidget {
    QTC_OBJECT(QSpinBox, QWidget)
public:
    explicit QSpinBox(QWidget* parent = 0);
    ~QSpinBox();
    int minimum() const { return minimum_; }
    void setMinimum(int value) { minimum_ = value; clamp(); }
    int maximum() const { return maximum_; }
    void setMaximum(int value) { maximum_ = value; clamp(); }
    void setRange(int minimum, int maximum);
    int value() const { return value_; }
    void setValue(int value);
    int singleStep() const { return singleStep_; }
    void setSingleStep(int step) { singleStep_ = step; }
    QString prefix() const { return prefix_; }
    void setPrefix(const QString& prefix) { prefix_ = prefix; update(); }
    QString suffix() const { return suffix_; }
    void setSuffix(const QString& suffix) { suffix_ = suffix; update(); }
    bool isReadOnly() const { return readOnly_; }
    void setReadOnly(bool on) { readOnly_ = on; update(); }
    bool wrapping() const { return wrapping_; }
    void setWrapping(bool on) { wrapping_ = on; }
    QString text() const;

    QSize sizeHint() const;

signals:
    Signal<void(int)> valueChanged;
    Signal<void(const QString&)> textChanged;

protected:
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    virtual void stepBy(int steps);
    virtual void valueFromText();

private:
    void clamp();
    QRect editorRect() const;
    QRect upButtonRect() const;
    QRect downButtonRect() const;

    int minimum_;
    int maximum_;
    int value_;
    int singleStep_;
    QString prefix_;
    QString suffix_;
    bool readOnly_;
    bool wrapping_;
    QString draft_;
    bool editing_;
    bool cursorVisible_;
    int cursor_;
    int blinkTimer_;
};

class QDoubleSpinBox : public QWidget {
    QTC_OBJECT(QDoubleSpinBox, QWidget)
public:
    explicit QDoubleSpinBox(QWidget* parent = 0);
    ~QDoubleSpinBox();
    double minimum() const { return minimum_; }
    void setMinimum(double value) { minimum_ = value; clamp(); }
    double maximum() const { return maximum_; }
    void setMaximum(double value) { maximum_ = value; clamp(); }
    void setRange(double minimum, double maximum);
    double value() const { return value_; }
    void setValue(double value);
    double singleStep() const { return singleStep_; }
    void setSingleStep(double step) { singleStep_ = step; }
    int decimals() const { return decimals_; }
    void setDecimals(int decimals);
    QString prefix() const { return prefix_; }
    void setPrefix(const QString& prefix) { prefix_ = prefix; update(); }
    QString suffix() const { return suffix_; }
    void setSuffix(const QString& suffix) { suffix_ = suffix; update(); }
    QString text() const;

    QSize sizeHint() const;

signals:
    Signal<void(double)> valueChanged;

protected:
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);

private:
    void clamp();
    QRect editorRect() const;
    QRect upButtonRect() const;
    QRect downButtonRect() const;
    QString formatted() const;

    double minimum_;
    double maximum_;
    double value_;
    double singleStep_;
    int decimals_;
    QString prefix_;
    QString suffix_;
    QString draft_;
    bool editing_;
    bool cursorVisible_;
    int cursor_;
    int blinkTimer_;
};

// ---------------------------------------------------------------------------
// QComboBox
// ---------------------------------------------------------------------------
class QComboBox : public QWidget {
    QTC_OBJECT(QComboBox, QWidget)
public:
    explicit QComboBox(QWidget* parent = 0);
    ~QComboBox();
    void addItem(const QString& text);
    void addItems(const QStringList& texts);
    void insertItem(int index, const QString& text);
    void removeItem(int index);
    void clear();
    int count() const { return items_.count(); }
    QString itemText(int index) const;
    void setItemText(int index, const QString& text);
    int currentIndex() const { return currentIndex_; }
    void setCurrentIndex(int index);
    QString currentText() const;
    bool isEditable() const { return editable_; }
    void setEditable(bool on);
    void setPlaceholderText(const QString& text) { placeholder_ = text; update(); }
    QString placeholderText() const { return placeholder_; }
    int maxVisibleItems() const { return maxVisibleItems_; }
    void setMaxVisibleItems(int count) { maxVisibleItems_ = qMax(1, count); }
    void showPopup();
    void hidePopup();
    bool isPopupVisible() const { return popupVisible_; }

    QSize sizeHint() const;

signals:
    Signal<void(int)> currentIndexChanged;
    Signal<void(const QString&)> currentTextChanged;
    Signal<void(int)> activated;
    Signal<void(int)> highlighted;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void wheelEvent(QWheelEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

private:
    void updatePopup();

    QStringList items_;
    int currentIndex_;
    bool editable_;
    QString placeholder_;
    bool popupVisible_;
    int popupIndex_;
    int hoveredIndex_;
    int maxVisibleItems_;
    QWidget* popup_;
    int popupScroll_;
};

// ---------------------------------------------------------------------------
// QGroupBox
// ---------------------------------------------------------------------------
class QGroupBox : public QWidget {
    QTC_OBJECT(QGroupBox, QWidget)
public:
    explicit QGroupBox(QWidget* parent = 0);
    explicit QGroupBox(const QString& title, QWidget* parent = 0);
    ~QGroupBox();
    QString title() const { return title_; }
    void setTitle(const QString& title) { title_ = title; update(); }
    bool isCheckable() const { return checkable_; }
    void setCheckable(bool on) { checkable_ = on; update(); }
    bool isChecked() const { return checked_; }
    void setChecked(bool on) { checked_ = on; update(); }
    Qt::Alignment alignment() const { return alignment_; }
    void setAlignment(Qt::Alignment a) { alignment_ = a; update(); }
    int titleHeight() const;

    QSize sizeHint() const;
    QRect contentsRect() const;

signals:
    Signal<void(bool)> toggled;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    QString title_;
    bool checkable_;
    bool checked_;
    Qt::Alignment alignment_;
};

// ---------------------------------------------------------------------------
// QTabWidget
// ---------------------------------------------------------------------------
class QTabWidget : public QWidget {
    QTC_OBJECT(QTabWidget, QWidget)
public:
    enum TabPosition { North, South, West, East };
    explicit QTabWidget(QWidget* parent = 0);
    ~QTabWidget();
    int addTab(QWidget* page, const QString& label);
    int insertTab(int index, QWidget* page, const QString& label);
    void removeTab(int index);
    void clear();
    int count() const { return pages_.count(); }
    int currentIndex() const { return currentIndex_; }
    void setCurrentIndex(int index);
    QWidget* currentWidget() const { return currentWidget_; }
    void setCurrentWidget(QWidget* page);
    QWidget* widget(int index) const;
    QString tabText(int index) const;
    void setTabText(int index, const QString& text);
    TabPosition tabPosition() const { return position_; }
    void setTabPosition(TabPosition p) { position_ = p; update(); }
    bool isTabEnabled(int index) const;
    void setTabEnabled(int index, bool enabled);
    bool tabsClosable() const { return closable_; }
    void setTabsClosable(bool on) { closable_ = on; update(); }
    bool documentMode() const { return documentMode_; }
    void setDocumentMode(bool on) { documentMode_ = on; update(); }
    int tabBarHeight() const;

    QSize sizeHint() const;

signals:
    Signal<void(int)> currentChanged;
    Signal<void(int)> tabCloseRequested;
    Signal<void(int)> tabBarClicked;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);

private:
    QRect tabRect(int index) const;
    int tabAt(const QPoint& pos) const;
    void updatePages();
    QRect pageRect() const;

    QList<QWidget*> pages_;
    QStringList labels_;
    QList<int> enabled_;
    int currentIndex_;
    QWidget* currentWidget_;
    TabPosition position_;
    bool closable_;
    bool documentMode_;
    int hoverIndex_;
    int closePressed_;
};

// ---------------------------------------------------------------------------
// QScrollArea
// ---------------------------------------------------------------------------
class QScrollArea : public QWidget {
    QTC_OBJECT(QScrollArea, QWidget)
public:
    explicit QScrollArea(QWidget* parent = 0);
    ~QScrollArea();
    QWidget* widget() const { return content_; }
    void setWidget(QWidget* widget);
    QWidget* takeWidget();
    void setWidgetResizable(bool resizable) { resizable_ = resizable; updateBars(); }
    bool widgetResizable() const { return resizable_; }
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    Qt::ScrollBarPolicy horizontalScrollBarPolicy() const { return hPolicy_; }
    Qt::ScrollBarPolicy verticalScrollBarPolicy() const { return vPolicy_; }
    QScrollBar* horizontalScrollBar() const { return hbar_; }
    QScrollBar* verticalScrollBar() const { return vbar_; }
    QWidget* viewport() const { return const_cast<QScrollArea*>(this); }
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);

private:
    void updateBars();
    QRect viewportRect() const;

    QWidget* content_;
    QScrollBar* hbar_;
    QScrollBar* vbar_;
    bool resizable_;
    Qt::ScrollBarPolicy hPolicy_;
    Qt::ScrollBarPolicy vPolicy_;
};

// ---------------------------------------------------------------------------
// QSplitter
// ---------------------------------------------------------------------------
class QSplitter : public QWidget {
    QTC_OBJECT(QSplitter, QWidget)
public:
    explicit QSplitter(QWidget* parent = 0);
    explicit QSplitter(Qt::Orientation orientation, QWidget* parent = 0);
    ~QSplitter();
    void addWidget(QWidget* widget);
    void insertWidget(int index, QWidget* widget);
    int count() const { return panels_.count(); }
    QWidget* widget(int index) const;
    Qt::Orientation orientation() const { return orientation_; }
    void setOrientation(Qt::Orientation o) { orientation_ = o; updateGeometry(); }
    void setSizes(const QList<int>& sizes);
    QList<int> sizes() const { return sizes_; }
    void setStretchFactor(int index, int stretch);
    bool isCollapsible(int index) const;
    void setCollapsible(int index, bool collapsible);
    int handleWidth() const { return handleWidth_; }
    void setHandleWidth(int width) { handleWidth_ = width; update(); }
    QSize sizeHint() const;

signals:
    Signal<void(int, int)> splitterMoved;

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    void applySizes();
    void normalizeSizes();
    QRect handleRect(int index) const;
    int handleAt(const QPoint& pos) const;

    QList<QWidget*> panels_;
    QList<int> sizes_;
    QList<int> stretches_;
    QList<int> collapsible_;
    Qt::Orientation orientation_;
    int handleWidth_;
    int draggingHandle_;
    int dragStartPos_;
    QList<int> dragStartSizes_;
};

// ---------------------------------------------------------------------------
// QStackedWidget
// ---------------------------------------------------------------------------
class QStackedWidget : public QWidget {
    QTC_OBJECT(QStackedWidget, QWidget)
public:
    explicit QStackedWidget(QWidget* parent = 0);
    ~QStackedWidget();
    int addWidget(QWidget* widget);
    int insertWidget(int index, QWidget* widget);
    void removeWidget(QWidget* widget);
    int count() const { return pages_.count(); }
    QWidget* widget(int index) const;
    int currentIndex() const { return currentIndex_; }
    QWidget* currentWidget() const;
    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget* widget);
    QSize sizeHint() const;

signals:
    Signal<void(int)> currentChanged;

protected:
    void resizeEvent(QResizeEvent* event);

private:
    QList<QWidget*> pages_;
    int currentIndex_;
};

// ---------------------------------------------------------------------------
// item view classes (list / tree / table)
// ---------------------------------------------------------------------------
class QListWidgetItem {
public:
    explicit QListWidgetItem(const QString& text = QString());
    virtual ~QListWidgetItem();
    QString text() const { return text_; }
    void setText(const QString& text) { text_ = text; }
    QIcon icon() const { return icon_; }
    void setIcon(const QIcon& icon) { icon_ = icon; }
    Qt::CheckState checkState() const { return checkState_; }
    void setCheckState(Qt::CheckState state) { checkState_ = state; }
    QVariant data(int role) const;
    void setData(int role, const QVariant& value);
    Qt::ItemFlags flags() const { return flags_; }
    void setFlags(Qt::ItemFlags flags) { flags_ = flags; }
    bool isSelected() const { return selected_; }
    void setSelected(bool on) { selected_ = on; }
    bool isHidden() const { return hidden_; }
    void setHidden(bool on) { hidden_ = on; }
    int row() const { return row_; }
    void setRow(int row) { row_ = row; }

private:
    friend class QListWidget;
    QString text_;
    QIcon icon_;
    Qt::CheckState checkState_;
    Qt::ItemFlags flags_;
    bool selected_;
    bool hidden_;
    int row_;
    QVariant userData_;
};

class QListWidget : public QWidget {
    QTC_OBJECT(QListWidget, QWidget)
public:
    explicit QListWidget(QWidget* parent = 0);
    ~QListWidget();
    void addItem(const QString& text);
    void addItem(QListWidgetItem* item);
    void insertItem(int row, const QString& text);
    void takeItem(int row);
    int count() const { return items_.count(); }
    QListWidgetItem* item(int row) const;
    int currentRow() const { return currentRow_; }
    void setCurrentRow(int row);
    QListWidgetItem* currentItem() const;
    void setCurrentItem(QListWidgetItem* item);
    void clear();
    QList<QListWidgetItem*> selectedItems() const;
    void setSelectionMode(int mode) { selectionMode_ = mode; }
    int selectionMode() const { return selectionMode_; }
    bool isAlternatingRowColors() const { return alternate_; }
    void setAlternatingRowColors(bool on) { alternate_ = on; update(); }
    int spacing() const { return itemSpacing_; }
    void setSpacing(int spacing) { itemSpacing_ = spacing; updateGeometry(); }
    int itemHeight() const { return itemHeight_; }
    void setItemHeight(int h) { itemHeight_ = h; updateGeometry(); }
    bool isSortingEnabled() const { return sorting_; }
    void setSortingEnabled(bool on) { sorting_ = on; }
    void sortItems();
    QScrollBar* verticalScrollBar() const { return vbar_; }

    QSize sizeHint() const;

signals:
    Signal<void(int)> currentRowChanged;
    Signal<void(QListWidgetItem*)> itemClicked;
    Signal<void(QListWidgetItem*)> itemDoubleClicked;
    Signal<void()> itemSelectionChanged;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void wheelEvent(QWheelEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    void updateScrollBars();
    QRect itemRect(int row) const;
    int rowAt(const QPoint& pos) const;
    void ensureVisible(int row);

    QList<QListWidgetItem*> items_;
    int currentRow_;
    int selectionMode_;
    bool alternate_;
    int itemSpacing_;
    int itemHeight_;
    bool sorting_;
    QScrollBar* vbar_;
    int hoverRow_;
    int dragStartRow_;
    bool dragging_;
};

class QTreeWidgetItem {
public:
    explicit QTreeWidgetItem(const QStringList& texts = QStringList());
    virtual ~QTreeWidgetItem();
    QString text(int column = 0) const;
    void setText(int column, const QString& text);
    int columnCount() const { return texts_.count(); }
    QTreeWidgetItem* parent() const { return parent_; }
    QTreeWidgetItem* child(int index) const;
    int childCount() const { return children_.count(); }
    int indexOfChild(QTreeWidgetItem* child) const;
    void addChild(QTreeWidgetItem* child);
    void insertChild(int index, QTreeWidgetItem* child);
    void removeChild(QTreeWidgetItem* child);
    void takeChildren();
    bool isExpanded() const { return expanded_; }
    void setExpanded(bool on) { expanded_ = on; }
    bool isSelected() const { return selected_; }
    void setSelected(bool on) { selected_ = on; }
    bool isHidden() const { return hidden_; }
    void setHidden(bool on) { hidden_ = on; }
    QIcon icon(int column = 0) const;
    void setIcon(int column, const QIcon& icon);
    QVariant data(int column, int role) const;
    void setData(int column, int role, const QVariant& value);
    QTreeWidgetItem* parentItem() const { return parent_; }

private:
    friend class QTreeWidget;
    QStringList texts_;
    QList<QIcon> icons_;
    QList<QTreeWidgetItem*> children_;
    QTreeWidgetItem* parent_;
    bool expanded_;
    bool selected_;
    bool hidden_;
    QVariant userData_;
};

class QTreeWidget : public QWidget {
    QTC_OBJECT(QTreeWidget, QWidget)
public:
    explicit QTreeWidget(QWidget* parent = 0);
    ~QTreeWidget();
    void setHeaderLabels(const QStringList& labels);
    QStringList headerLabels() const { return headers_; }
    void setColumnCount(int count);
    int columnCount() const { return headers_.count(); }
    void addTopLevelItem(QTreeWidgetItem* item);
    void insertTopLevelItem(int index, QTreeWidgetItem* item);
    void takeTopLevelItem(int index);
    QTreeWidgetItem* topLevelItem(int index) const;
    int topLevelItemCount() const { return roots_.count(); }
    void clear();
    QTreeWidgetItem* currentItem() const { return current_; }
    QTreeWidgetItem* itemAt(const QPoint& pos) const;
    void setCurrentItem(QTreeWidgetItem* item);
    void expandAll();
    void collapseAll();
    void expandItem(QTreeWidgetItem* item);
    void collapseItem(QTreeWidgetItem* item);
    QList<QTreeWidgetItem*> selectedItems() const;
    bool isAlternatingRowColors() const { return alternate_; }
    void setAlternatingRowColors(bool on) { alternate_ = on; update(); }
    int indentation() const { return indentation_; }
    void setIndentation(int indentation) { indentation_ = indentation; update(); }
    int itemHeight() const { return itemHeight_; }
    void setItemHeight(int h) { itemHeight_ = h; updateGeometry(); }
    QScrollBar* verticalScrollBar() const { return vbar_; }

    QSize sizeHint() const;

signals:
    Signal<void(QTreeWidgetItem*, int)> itemClicked;
    Signal<void(QTreeWidgetItem*, int)> itemDoubleClicked;
    Signal<void(QTreeWidgetItem*, int)> itemExpanded;
    Signal<void(QTreeWidgetItem*, int)> itemCollapsed;
    Signal<void()> currentItemChanged;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void wheelEvent(QWheelEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    void flatten(QTreeWidgetItem* item, int depth, int& y);
    QTreeWidgetItem* itemAtPoint(const QPoint& pos, int* depth, QRect* rowRect, int* column) const;
    void collectVisible(QList<QTreeWidgetItem*>& out, QList<int>& depths);
    void updateScrollBars();
    void ensureVisible(QTreeWidgetItem* item);
    int contentHeight() const;

    QList<QTreeWidgetItem*> roots_;
    QStringList headers_;
    QTreeWidgetItem* current_;
    bool alternate_;
    int indentation_;
    int itemHeight_;
    QScrollBar* vbar_;
    int columnWidth_;
    int hoverItemY_;
};

class QTableWidgetItem {
public:
    explicit QTableWidgetItem(const QString& text = QString());
    virtual ~QTableWidgetItem();
    QString text() const { return text_; }
    void setText(const QString& text) { text_ = text; }
    Qt::CheckState checkState() const { return checkState_; }
    void setCheckState(Qt::CheckState state) { checkState_ = state; }
    QVariant data(int role) const;
    void setData(int role, const QVariant& value);
    Qt::ItemFlags flags() const { return flags_; }
    void setFlags(Qt::ItemFlags flags) { flags_ = flags; }
    QIcon icon() const { return icon_; }
    void setIcon(const QIcon& icon) { icon_ = icon; }

private:
    friend class QTableWidget;
    QString text_;
    QIcon icon_;
    Qt::CheckState checkState_;
    Qt::ItemFlags flags_;
    QVariant userData_;
};

class QTableWidget : public QWidget {
    QTC_OBJECT(QTableWidget, QWidget)
public:
    explicit QTableWidget(QWidget* parent = 0);
    QTableWidget(int rows, int columns, QWidget* parent = 0);
    ~QTableWidget();
    void setRowCount(int rows);
    int rowCount() const { return rows_; }
    void setColumnCount(int columns);
    int columnCount() const { return columns_; }
    void setHorizontalHeaderLabels(const QStringList& labels);
    void setVerticalHeaderLabels(const QStringList& labels);
    QStringList horizontalHeaderLabels() const { return columnHeaders_; }
    QTableWidgetItem* item(int row, int column) const;
    void setItem(int row, int column, QTableWidgetItem* item);
    QTableWidgetItem* takeItemAt(int row, int column);
    void clearContents();
    void clear();
    int currentRow() const { return currentRow_; }
    int currentColumn() const { return currentColumn_; }
    void setCurrentCell(int row, int column);
    QTableWidgetItem* currentItem() const;
    void selectRow(int row);
    void selectColumn(int column);
    void setSelectionMode(int mode) { selectionMode_ = mode; }
    bool isAlternatingRowColors() const { return alternate_; }
    void setAlternatingRowColors(bool on) { alternate_ = on; update(); }
    int rowHeight() const { return rowHeight_; }
    void setRowHeight(int h) { rowHeight_ = h; updateGeometry(); }
    bool isSortingEnabled() const { return sorting_; }
    void setSortingEnabled(bool on) { sorting_ = on; }
    void sortItems(int column, Qt::SortOrder order = Qt::AscendingOrder);
    QScrollBar* verticalScrollBar() const { return vbar_; }
    QScrollBar* horizontalScrollBar() const { return hbar_; }
    bool horizontalHeaderVisible() const { return hHeader_; }
    void setHorizontalHeaderVisible(bool on) { hHeader_ = on; update(); }
    bool verticalHeaderVisible() const { return vHeader_; }
    void setVerticalHeaderVisible(bool on) { vHeader_ = on; update(); }
    bool gridEnabled() const { return grid_; }
    void setGridEnabled(bool on) { grid_ = on; update(); }
    QRect cellRect(int row, int column) const;
    int headerHeight() const;
    int headerWidth() const;

    QSize sizeHint() const;

signals:
    Signal<void(int, int)> cellClicked;
    Signal<void(int, int)> cellDoubleClicked;
    Signal<void(int, int)> currentCellChanged;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void wheelEvent(QWheelEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    QPoint cellAt(const QPoint& pos) const;
    void updateScrollBars();
    void ensureVisible(int row, int column);

    int rows_;
    int columns_;
    QList<QTableWidgetItem*> cells_;
    QStringList columnHeaders_;
    QStringList rowHeaders_;
    int currentRow_;
    int currentColumn_;
    int selectionMode_;
    bool alternate_;
    bool grid_;
    bool hHeader_;
    bool vHeader_;
    int rowHeight_;
    bool sorting_;
    QScrollBar* vbar_;
    QScrollBar* hbar_;
    int columnWidth_;
    int scrollY_;
    int scrollX_;
};

// ---------------------------------------------------------------------------
// dialogs
// ---------------------------------------------------------------------------
class QDialog : public QWidget {
    QTC_OBJECT(QDialog, QWidget)
public:
    explicit QDialog(QWidget* parent = 0, Qt::WindowFlags f = Qt::Dialog);
    ~QDialog();
    enum DialogCode { Rejected = 0, Accepted = 1 };
    int exec();
    void done(int result);
    void accept();
    void reject();
    int result() const { return result_; }
    void setResult(int result) { result_ = result; }
    void setModal(bool modal) { modal_ = modal; }
    bool isModal() const { return modal_; }
    void setSizeGripEnabled(bool on) { sizeGrip_ = on; }
    bool isSizeGripEnabled() const { return sizeGrip_; }

signals:
    Signal<void(int)> finished;
    Signal<void()> accepted;
    Signal<void()> rejected;

protected:
    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void showEvent(QShowEvent* event);

private:
    int result_;
    bool modal_;
    bool sizeGrip_;
};

class QMessageBox : public QDialog {
    QTC_OBJECT(QMessageBox, QDialog)
public:
    enum Icon { NoIcon, Information, Warning, Critical, Question };
    enum StandardButton {
        NoButton = 0, Ok = 0x1, Save = 0x2, Cancel = 0x4, Close = 0x8, Discard = 0x10,
        Apply = 0x20, Reset = 0x40, Yes = 0x80, No = 0x100, Abort = 0x200, Retry = 0x400,
        Ignore = 0x800, YesToAll = 0x1000, NoToAll = 0x2000
    };
    typedef QFlags<StandardButton> StandardButtons;

    explicit QMessageBox(QWidget* parent = 0);
    QMessageBox(Icon icon, const QString& title, const QString& text,
                StandardButtons buttons = Ok, QWidget* parent = 0);
    ~QMessageBox();

    void setText(const QString& text);
    QString text() const { return text_; }
    void setInformativeText(const QString& text);
    QString informativeText() const { return informative_; }
    void setDetailedText(const QString& text) { detailed_ = text; }
    QString detailedText() const { return detailed_; }
    void setWindowTitle(const QString& title);
    QString windowTitle() const;
    void setIcon(Icon icon) { icon_ = icon; update(); }
    Icon icon() const { return icon_; }
    void setStandardButtons(StandardButtons buttons);
    StandardButtons standardButtons() const { return buttons_; }
    void setDefaultButton(StandardButton button) { defaultButton_ = button; update(); }
    StandardButton defaultButton() const { return defaultButton_; }
    StandardButton clickedButton() const { return clicked_; }
    void addButton(const QString& text, int role);
    void setTextFormat(int format) { textFormat_ = format; }

    static StandardButton information(QWidget* parent, const QString& title, const QString& text,
                                      StandardButtons buttons = Ok, StandardButton defaultButton = NoButton);
    static StandardButton warning(QWidget* parent, const QString& title, const QString& text,
                                  StandardButtons buttons = Ok, StandardButton defaultButton = NoButton);
    static StandardButton critical(QWidget* parent, const QString& title, const QString& text,
                                   StandardButtons buttons = Ok, StandardButton defaultButton = NoButton);
    static StandardButton question(QWidget* parent, const QString& title, const QString& text,
                                   StandardButtons buttons = Yes | No, StandardButton defaultButton = NoButton);
    static void about(QWidget* parent, const QString& title, const QString& text);
    static void aboutQt(QWidget* parent, const QString& title = QString());


protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void showEvent(QShowEvent* event);

private:
    void layoutContent();
    void onButtonClicked();

    Icon icon_;
    QString text_;
    QString informative_;
    QString detailed_;
    StandardButtons buttons_;
    StandardButton defaultButton_;
    StandardButton clicked_;
    int textFormat_;
    QList<QPushButton*> buttonWidgets_;
    QList<int> buttonValues_;
};

class QFileDialog {
public:
    static QString getOpenFileName(QWidget* parent, const QString& caption,
                                   const QString& dir, const QString& filter,
                                   QString* selectedFilter = 0);
    static QString getSaveFileName(QWidget* parent, const QString& caption,
                                   const QString& dir, const QString& filter,
                                   QString* selectedFilter = 0);
    static QString getExistingDirectory(QWidget* parent, const QString& caption, const QString& dir);
    static QStringList getOpenFileNames(QWidget* parent, const QString& caption,
                                        const QString& dir, const QString& filter);
};

class QColorDialog {
public:
    static QColor getColor(const QColor& initial, QWidget* parent = 0, const QString& title = QString());
    static bool getColor(QColor* color, QWidget* parent = 0, const QString& title = QString());
};

class QFontDialog {
public:
    static QFont getFont(bool* ok, const QFont& initial, QWidget* parent = 0, const QString& title = QString());
};

class QInputDialog {
public:
    static QString getText(QWidget* parent, const QString& title, const QString& label,
                           int echoMode = 0, const QString& text = QString(), bool* ok = 0);
    static int getInt(QWidget* parent, const QString& title, const QString& label,
                      int value = 0, int minimum = -2147483647, int maximum = 2147483647,
                      int step = 1, bool* ok = 0);
    static double getDouble(QWidget* parent, const QString& title, const QString& label,
                            double value = 0, double minimum = -2147483647.0,
                            double maximum = 2147483647.0, int decimals = 1, bool* ok = 0);
    static QString getItem(QWidget* parent, const QString& title, const QString& label,
                           const QStringList& items, int current = 0, bool editable = true, bool* ok = 0);
};

class QToolTip {
public:
    static void showText(const QPoint& pos, const QString& text, QWidget* widget = 0, int msec = 10000);
    static void hideText();
    static bool isVisible();
    static QString text();
};

// ---------------------------------------------------------------------------
// actions / menus / docks / main window
// ---------------------------------------------------------------------------
class QAction : public QObject {
    QTC_OBJECT(QAction, QObject)
public:
    explicit QAction(QObject* parent = 0);
    QAction(const QString& text, QObject* parent = 0);
    QAction(const QIcon& icon, const QString& text, QObject* parent = 0);
    ~QAction();

    QString text() const { return text_; }
    void setText(const QString& text);
    QIcon icon() const { return icon_; }
    void setIcon(const QIcon& icon) { icon_ = icon; }
    QString shortcut() const { return shortcut_; }
    void setShortcut(const QString& shortcut) { shortcut_ = shortcut; }
    bool isEnabled() const { return enabled_; }
    void setEnabled(bool on) { enabled_ = on; }
    bool isCheckable() const { return checkable_; }
    void setCheckable(bool on) { checkable_ = on; }
    bool isChecked() const { return checked_; }
    void setChecked(bool on);
    bool isSeparator() const { return separator_; }
    void setSeparator(bool on) { separator_ = on; }
    QString toolTip() const { return toolTip_; }
    void setToolTip(const QString& tip) { toolTip_ = tip; }
    QString statusTip() const { return statusTip_; }
    void setStatusTip(const QString& tip) { statusTip_ = tip; }
    QString objectName() const { return QObject::objectName(); }
    void setObjectName(const QString& name) { QObject::setObjectName(name); }
    int data() const { return data_; }
    void setData(int data) { data_ = data; }
    QList<QWidget*> associatedWidgets() const;

signals:
    Signal<void(bool)> triggered;
    Signal<void(bool)> toggled;
    Signal<void()> hovered;
    Signal<void()> changed;

private:
    friend class QMenu;
    friend class QMenuBar;
    friend class QToolBar;
    friend class QAbstractButton;

    QString text_;
    QIcon icon_;
    QString shortcut_;
    QString toolTip_;
    QString statusTip_;
    bool enabled_;
    bool checkable_;
    bool checked_;
    bool separator_;
    int data_;
    QList<QWidget*> widgets_;
};


class QMenu : public QWidget {
    QTC_OBJECT(QMenu, QWidget)
public:
    explicit QMenu(QWidget* parent = 0);
    explicit QMenu(const QString& title, QWidget* parent = 0);
    ~QMenu();
    QAction* addAction(const QString& text);
    QAction* addAction(const QIcon& icon, const QString& text);
    void addAction(QAction* action);
    QAction* addMenu(QMenu* menu);
    QMenu* addMenu(const QString& title);
    void addSeparator();
    void insertAction(QAction* before, QAction* action);
    void insertSeparator(QAction* before);
    void removeAction(QAction* action);
    void clear();
    QList<QAction*> actions() const { return actions_; }
    QString title() const { return title_; }
    void setTitle(const QString& title) { title_ = title; }
    QAction* actionAt(const QPoint& pos) const;
    void popup(const QPoint& globalPos);
    void exec(const QPoint& globalPos);
    bool isVisible() const;
    void setActiveAction(QAction* action);
    QAction* activeAction() const { return active_; }
    int actionGeometry(QAction* action) const;

    QSize sizeHint() const;

signals:
    Signal<void()> aboutToShow;
    Signal<void()> aboutToHide;
    Signal<void(QAction*)> triggeredAction;
    Signal<void(QAction*)> hoveredAction;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

private:
    void updateGeometryForActions();
    int indexAt(const QPoint& pos) const;
    QRect actionRect(int index) const;
    QMenu* submenuFor(int index) const;

    QString title_;
    QList<QAction*> actions_;
    QList<QMenu*> submenus_;
    QAction* active_;
    int activeIndex_;
    int itemHeight_;
    int separatorHeight_;
    int leftMargin_;
    int rightMargin_;
    QMenu* parentMenu_;
    QMenu* openSubmenu_;
    bool suppressRelease_;
};

class QMenuBar : public QWidget {
    QTC_OBJECT(QMenuBar, QWidget)
public:
    explicit QMenuBar(QWidget* parent = 0);
    ~QMenuBar();
    QAction* addMenu(QMenu* menu);
    QMenu* addMenu(const QString& title);
    void addAction(QAction* action);
    void removeAction(QAction* action);
    QList<QAction*> actions() const { return actions_; }
    int heightForWidth(int width) const;
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

private:
    int itemAt(const QPoint& pos) const;
    QRect itemRect(int index) const;

    QList<QAction*> actions_;
    QList<QMenu*> menus_;
    int activeIndex_;
    int hoverIndex_;
};

class QToolBar : public QWidget {
    QTC_OBJECT(QToolBar, QWidget)
public:
    explicit QToolBar(QWidget* parent = 0);
    explicit QToolBar(const QString& title, QWidget* parent = 0);
    ~QToolBar();
    QAction* addAction(const QString& text);
    QAction* addAction(const QIcon& icon, const QString& text);
    void addAction(QAction* action);
    void addSeparator();
    void addWidget(QWidget* widget);
    void clear();
    QList<QAction*> actions() const { return actions_; }
    QString windowTitle() const { return title_; }
    void setWindowTitle(const QString& title) { title_ = title; }
    Qt::ToolButtonStyle toolButtonStyle() const { return toolStyle_; }
    void setToolButtonStyle(Qt::ToolButtonStyle style) { toolStyle_ = style; updateGeometry(); }
    bool isMovable() const { return movable_; }
    void setMovable(bool on) { movable_ = on; }
    bool isFloatable() const { return movable_; }
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    QAction* actionAt(const QPoint& pos) const;
    QRect actionRect(QAction* action) const;
    void relayout();

    QString title_;
    QList<QAction*> actions_;
    QList<QWidget*> widgets_;
    QList<QRect> actionRects_;
    Qt::ToolButtonStyle toolStyle_;
    bool movable_;
    int iconSize_;
    int hoverIndex_;
    int pressedIndex_;
};

class QStatusBar : public QWidget {
    QTC_OBJECT(QStatusBar, QWidget)
public:
    explicit QStatusBar(QWidget* parent = 0);
    ~QStatusBar();
    void showMessage(const QString& text, int timeout = 0);
    void clearMessage();
    QString currentMessage() const { return message_; }
    void addWidget(QWidget* widget);
    void addPermanentWidget(QWidget* widget);
    void removeWidget(QWidget* widget);
    QSize sizeHint() const;

signals:
    Signal<void(const QString&)> messageChanged;

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);
    void timerEvent(QTimerEvent* event);

private:
    QString message_;
    QList<QWidget*> permanent_;
    QList<QWidget*> normal_;
    int messageTimer_;
};

class QMainWindow : public QWidget {
    QTC_OBJECT(QMainWindow, QWidget)
public:
    explicit QMainWindow(QWidget* parent = 0, Qt::WindowFlags f = Qt::Window);
    ~QMainWindow();
    void setCentralWidget(QWidget* widget);
    QWidget* centralWidget() const { return central_; }
    void setMenuBar(QMenuBar* menuBar);
    QMenuBar* menuBar() const { return menuBar_; }
    QMenuBar* qtcCreateMenuBar();
    void setStatusBar(QStatusBar* statusBar);
    QStatusBar* statusBar() const { return statusBar_; }
    QStatusBar* qtcCreateStatusBar();
    void addToolBar(QToolBar* toolbar);
    void addToolBarBreak();
    void insertToolBar(QToolBar* before, QToolBar* toolbar);
    void removeToolBar(QToolBar* toolbar);
    QList<QToolBar*> toolBars() const { return toolBars_; }
    void addDockWidgetArea(QWidget* widget, int area);
    QSize sizeHint() const;

signals:

protected:
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent* event);
    void showEvent(QShowEvent* event);

private:
    void layoutMain();

    QWidget* central_;
    QMenuBar* menuBar_;
    QStatusBar* statusBar_;
    QList<QToolBar*> toolBars_;
    bool toolbarBreak_;
};

#endif // QTC_QTWIDGETS_H
