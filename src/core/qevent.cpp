// QtClone - qevent.cpp
#include <qtc/QEvent.h>
#include <qtc/QObject.h>

int QEvent::registerEventType(int hint) {
    static int next = QEvent::User + 100;
    if (hint >= QEvent::User && hint < QEvent::MaxUser) {
        if (hint >= next) next = hint + 1;
        return hint;
    }
    return next++;
}

QEvent::QEvent(Type type) : t_(type), accepted_(true), spontaneous_(false) {}
QEvent::~QEvent() {}

QTimerEvent::QTimerEvent(int timerId) : QEvent(QEvent::Timer), id_(timerId) {}

QChildEvent::QChildEvent(Type type, QObject* child) : QEvent(type), child_(child) {}

QPaintEvent::QPaintEvent(const QRect& paintRect)
    : QEvent(QEvent::Paint), rect_(paintRect), region_(paintRect) {}

QPaintEvent::QPaintEvent(const QRect& paintRect, const QRect& paintRegion)
    : QEvent(QEvent::Paint), rect_(paintRect), region_(paintRegion) {}

QMoveEvent::QMoveEvent(const QPoint& pos, const QPoint& oldPos)
    : QEvent(QEvent::Move), pos_(pos), oldPos_(oldPos) {}

QResizeEvent::QResizeEvent(const QSize& size, const QSize& oldSize)
    : QEvent(QEvent::Resize), size_(size), oldSize_(oldSize) {}

QShowEvent::QShowEvent() : QEvent(QEvent::Show) {}
QHideEvent::QHideEvent() : QEvent(QEvent::Hide) {}
QCloseEvent::QCloseEvent() : QEvent(QEvent::Close) {}
QDeferredDeleteEvent::QDeferredDeleteEvent() : QEvent(QEvent::DeferredDelete) {}

QFocusEvent::QFocusEvent(Type type, Qt::FocusReason reason)
    : QEvent(type), reason_(reason) {}

QKeyEvent::QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers,
                     const QString& text, bool autorep, int count)
    : QEvent(type), key_(key), mods_(modifiers), text_(text), autorep_(autorep), count_(count) {}

QMouseEvent::QMouseEvent(Type type, const QPoint& pos, const QPoint& globalPos,
                         Qt::MouseButton button, Qt::MouseButtons buttons,
                         Qt::KeyboardModifiers modifiers)
    : QEvent(type), pos_(pos), globalPos_(globalPos), button_(button),
      buttons_(buttons), mods_(modifiers) {}

QWheelEvent::QWheelEvent(const QPoint& pos, const QPoint& globalPos, int delta,
                         Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                         Qt::Orientation orientation)
    : QEvent(QEvent::Wheel), pos_(pos), globalPos_(globalPos), delta_(delta),
      buttons_(buttons), mods_(modifiers), orientation_(orientation) {}

QContextMenuEvent::QContextMenuEvent(const QPoint& pos, const QPoint& globalPos)
    : QEvent(QEvent::ContextMenu), pos_(pos), globalPos_(globalPos) {}
