// QtClone - QEvent.h : event classes.
#ifndef QTC_QEVENT_H
#define QTC_QEVENT_H

#include <qtc/QtGlobal.h>
#include <qtc/QRect.h>
#include <qtc/QString.h>

class QObject;
class QWidget;

class QEvent {
public:
    enum Type {
        None = 0,
        Timer = 1,
        MouseButtonPress = 2,
        MouseButtonRelease = 3,
        MouseButtonDblClick = 4,
        MouseMove = 5,
        KeyPress = 6,
        KeyRelease = 7,
        FocusIn = 8,
        FocusOut = 9,
        Enter = 10,
        Leave = 11,
        Paint = 12,
        Move = 13,
        Resize = 14,
        Create = 15,
        Destroy = 16,
        Show = 17,
        Hide = 18,
        Close = 19,
        Quit = 20,
        ParentChange = 21,
        WindowActivate = 24,
        WindowDeactivate = 25,
        ShowToParent = 26,
        HideToParent = 27,
        Wheel = 31,
        WindowTitleChange = 33,
        WindowIconChange = 34,
        ApplicationWindowIconChange = 35,
        ApplicationFontChange = 36,
        ApplicationLayoutDirectionChange = 37,
        ApplicationPaletteChange = 38,
        PaletteChange = 39,
        LanguageChange = 40,
        Clipboard = 41,
        MetaCall = 43,
        ChildAdded = 44,
        ChildPolished = 45,
        ChildRemoved = 46,
        ShowWindowRequest = 47,
        PolishRequest = 48,
        Polish = 49,
        LayoutRequest = 50,
        UpdateRequest = 51,
        UpdateLater = 52,
        ContextMenu = 82,
        LocaleChange = 88,
        DeferredDelete = 200,
        ZeroTimer = 201,
        ShortcutOverride = 202,
        InputMethod = 204,
        ThemeChange = 205,
        StyleChange = 206,
        WindowStateChange = 207,
        EnabledChange = 208,
        FontChange = 209,
        User = 1000,
        MaxUser = 65535
    };

    explicit QEvent(Type type);
    virtual ~QEvent();

    Type type() const { return t_; }
    bool spontaneous() const { return spontaneous_; }
    void accept() { accepted_ = true; }
    void ignore() { accepted_ = false; }
    bool isAccepted() const { return accepted_; }
    void setAccepted(bool accepted) { accepted_ = accepted; }

    static int registerEventType(int hint = -1);

protected:
    Type t_;
    bool accepted_;
    bool spontaneous_;
    friend class QWidget;
    friend class QApplication;
};

class QTimerEvent : public QEvent {
public:
    explicit QTimerEvent(int timerId);
    int timerId() const { return id_; }
private:
    int id_;
};

class QChildEvent : public QEvent {
public:
    QChildEvent(Type type, QObject* child);
    QObject* child() const { return child_; }
    bool added() const { return type() == ChildAdded; }
    bool polished() const { return type() == ChildPolished; }
    bool removed() const { return type() == ChildRemoved; }
private:
    QObject* child_;
};

class QPaintEvent : public QEvent {
public:
    explicit QPaintEvent(const QRect& paintRect);
    QPaintEvent(const QRect& paintRect, const QRect& paintRegion);
    const QRect& rect() const { return rect_; }
    const QRect& region() const { return region_; }
private:
    QRect rect_;
    QRect region_;
};

class QMoveEvent : public QEvent {
public:
    QMoveEvent(const QPoint& pos, const QPoint& oldPos);
    const QPoint& pos() const { return pos_; }
    const QPoint& oldPos() const { return oldPos_; }
private:
    QPoint pos_, oldPos_;
};

class QResizeEvent : public QEvent {
public:
    QResizeEvent(const QSize& size, const QSize& oldSize);
    const QSize& size() const { return size_; }
    const QSize& oldSize() const { return oldSize_; }
private:
    QSize size_, oldSize_;
};

class QShowEvent : public QEvent { public: QShowEvent(); };
class QHideEvent : public QEvent { public: QHideEvent(); };
class QCloseEvent : public QEvent { public: QCloseEvent(); };
class QDeferredDeleteEvent : public QEvent { public: QDeferredDeleteEvent(); };

class QFocusEvent : public QEvent {
public:
    explicit QFocusEvent(Type type, Qt::FocusReason reason = Qt::OtherFocusReason);
    bool gotFocus() const { return type() == FocusIn; }
    bool lostFocus() const { return type() == FocusOut; }
    Qt::FocusReason reason() const { return reason_; }
private:
    Qt::FocusReason reason_;
};

class QKeyEvent : public QEvent {
public:
    QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers,
              const QString& text = QString(), bool autorep = false, int count = 1);
    int key() const { return key_; }
    Qt::KeyboardModifiers modifiers() const { return mods_; }
    QString text() const { return text_; }
    bool isAutoRepeat() const { return autorep_; }
    int count() const { return count_; }
private:
    int key_;
    Qt::KeyboardModifiers mods_;
    QString text_;
    bool autorep_;
    int count_;
};

class QMouseEvent : public QEvent {
public:
    QMouseEvent(Type type, const QPoint& pos, const QPoint& globalPos,
                Qt::MouseButton button, Qt::MouseButtons buttons,
                Qt::KeyboardModifiers modifiers);
    const QPoint& pos() const { return pos_; }
    const QPoint& globalPos() const { return globalPos_; }
    int x() const { return pos_.x(); }
    int y() const { return pos_.y(); }
    int globalX() const { return globalPos_.x(); }
    int globalY() const { return globalPos_.y(); }
    Qt::MouseButton button() const { return button_; }
    Qt::MouseButtons buttons() const { return buttons_; }
    Qt::KeyboardModifiers modifiers() const { return mods_; }
private:
    QPoint pos_, globalPos_;
    Qt::MouseButton button_;
    Qt::MouseButtons buttons_;
    Qt::KeyboardModifiers mods_;
};

class QWheelEvent : public QEvent {
public:
    QWheelEvent(const QPoint& pos, const QPoint& globalPos, int delta,
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                Qt::Orientation orientation = Qt::Vertical);
    const QPoint& pos() const { return pos_; }
    const QPoint& globalPos() const { return globalPos_; }
    int delta() const { return delta_; }
    int angleDelta() const { return delta_; }
    Qt::Orientation orientation() const { return orientation_; }
    Qt::MouseButtons buttons() const { return buttons_; }
    Qt::KeyboardModifiers modifiers() const { return mods_; }
private:
    QPoint pos_, globalPos_;
    int delta_;
    Qt::MouseButtons buttons_;
    Qt::KeyboardModifiers mods_;
    Qt::Orientation orientation_;
};

class QContextMenuEvent : public QEvent {
public:
    QContextMenuEvent(const QPoint& pos, const QPoint& globalPos);
    const QPoint& pos() const { return pos_; }
    const QPoint& globalPos() const { return globalPos_; }
private:
    QPoint pos_, globalPos_;
};

class QApplicationEvent : public QEvent {
public:
    QApplicationEvent(Type type) : QEvent(type) {}
};

class QInputMethodEvent : public QEvent {
public:
    QInputMethodEvent() : QEvent(QEvent::InputMethod), replaceFrom_(0), replaceLength_(0) {}
    void setCommitString(const QString& commitString, int replaceFrom = 0, int replaceLength = 0) {
        commit_ = commitString;
        replaceFrom_ = replaceFrom;
        replaceLength_ = replaceLength;
    }
    const QString& preeditString() const { return preedit_; }
    const QString& commitString() const { return commit_; }
    int replacementStart() const { return replaceFrom_; }
    int replacementLength() const { return replaceLength_; }
    void setPreeditString(const QString& text) { preedit_ = text; }
    bool hasCommit() const { return !commit_.isEmpty(); }

private:
    QString preedit_;
    QString commit_;
    int replaceFrom_;
    int replaceLength_;
};

#endif // QTC_QEVENT_H
