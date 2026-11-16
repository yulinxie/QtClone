// QtClone - qobject.cpp : connections, meta object queries, timers, posted events.
#include <qtc/QObject.h>
#include <qtc/QEvent.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <set>

namespace {

struct QConnection {
    const QSignalBase* signal;
    QObject* sender;
    QObject* receiver;
    QSlotCallback* callback;
    bool active;
};

struct PostedEvent {
    QObject* receiver;
    QEvent* event;
};

struct TimerEntry {
    UINT_PTR id;
    QObject* obj;
};

std::set<QObject*>& aliveSet() { static std::set<QObject*> s; return s; }
std::vector<QConnection*>& connections() { static std::vector<QConnection*> v; return v; }
QList<PostedEvent*>& postedQueue() { static QList<PostedEvent*> q; return q; }
std::vector<TimerEntry>& timers() { static std::vector<TimerEntry> t; return t; }
UINT_PTR& nextTimerId() { static UINT_PTR id = 1; return id; }
unsigned int& nextSignalId() { static unsigned int id = 1; return id; }
QObject*& currentSender() { static QObject* s = 0; return s; }

LRESULT CALLBACK qtcTimerWndProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
    if (msg == WM_TIMER) {
        std::vector<TimerEntry>& t = timers();
        for (size_t i = 0; i < t.size(); ++i) {
            if (t[i].id == (UINT_PTR)w) {
                QObject* obj = t[i].obj;
                if (qtcIsAlive(obj)) {
                    QTimerEvent ev((int)w);
                    qtcSendEvent(obj, &ev);
                }
                break;
            }
        }
        return 0;
    }
    return DefWindowProcW(h, msg, w, l);
}

HWND timerWindow() {
    static HWND wnd = 0;
    if (wnd) return wnd;
    static const wchar_t* kClass = L"QtCloneTimerWindow";
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = &qtcTimerWndProc;
        wc.hInstance = GetModuleHandleW(0);
        wc.lpszClassName = kClass;
        RegisterClassExW(&wc);
        registered = true;
    }
    wnd = CreateWindowExW(0, kClass, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, GetModuleHandleW(0), 0);
    return wnd;
}

void removeTimersFor(QObject* obj) {
    std::vector<TimerEntry>& t = timers();
    for (size_t i = t.size(); i > 0; --i) {
        if (t[i - 1].obj == obj) {
            if (timerWindow()) KillTimer(timerWindow(), t[i - 1].id);
            t.erase(t.begin() + (i - 1));
        }
    }
}

void removeConnectionsForSignal(const QSignalBase* signal) {
    std::vector<QConnection*>& list = connections();
    for (size_t i = list.size(); i > 0; --i) {
        QConnection* c = list[i - 1];
        if (c->signal == signal) {
            delete c->callback;
            delete c;
            list.erase(list.begin() + (i - 1));
        }
    }
}

static const QMetaObject::PropertyData* declaringProperty(const QMetaObject* mo, int index, int* local) {
    for (const QMetaObject* m = mo; m; m = m->superClass()) {
        int off = m->propertyOffset();
        if (index >= off && index < off + m->d_propertyCount) {
            if (local) *local = index - off;
            return &m->d_properties[index - off];
        }
    }
    return 0;
}

static const QMetaObject::SignalInfo* declaringSignal(const QMetaObject* mo, int index) {
    for (const QMetaObject* m = mo; m; m = m->superClass()) {
        int off = m->signalOffset();
        if (index >= off && index < off + m->d_signalCount) return &m->d_signals[index - off];
    }
    return 0;
}

} // namespace

bool qtcIsAlive(QObject* obj) { return obj && aliveSet().count(obj) != 0; }
QObject* qtcCurrentSender() { return currentSender(); }
void qtcSetCurrentSender(QObject* sender) { currentSender() = sender; }

// ---------------------------------------------------------------------------
// QSignalBase
// ---------------------------------------------------------------------------
QSignalBase::QSignalBase() : id_(nextSignalId()++) {}
QSignalBase::~QSignalBase() { removeConnectionsForSignal(this); }

int QSignalBase::connectionCount() const {
    int n = 0;
    const std::vector<QConnection*>& list = connections();
    for (size_t i = 0; i < list.size(); ++i)
        if (list[i]->signal == this && list[i]->active) ++n;
    return n;
}

bool QSignalBase::addConnection(QObject* sender, QObject* receiver, QSlotCallback* callback) {
    if (!sender || !receiver || !callback) {
        delete callback;
        return false;
    }
    QConnection* c = new QConnection();
    c->signal = this;
    c->sender = sender;
    c->receiver = receiver;
    c->callback = callback;
    c->active = true;
    connections().push_back(c);
    return true;
}

void QSignalBase::emitArgs(void** args, int argc) {
    std::vector<QConnection*>& list = connections();
    if (list.empty()) return;

    std::vector<QConnection*> targets;
    for (size_t i = 0; i < list.size(); ++i) {
        QConnection* c = list[i];
        if (c->signal != this || !c->active) continue;
        if (!qtcIsAlive(c->sender) || !qtcIsAlive(c->receiver)) continue;
        if (c->sender->signalsBlocked()) continue;
        targets.push_back(c);
    }
    (void)argc;
    for (size_t i = 0; i < targets.size(); ++i) {
        QConnection* c = targets[i];
        if (!c->active || !qtcIsAlive(c->receiver)) continue;
        QObject* previous = currentSender();
        currentSender() = c->sender;
        c->callback->call(args);
        currentSender() = previous;
    }
}

void QSignalBase::disconnect(QObject* receiver) {
    std::vector<QConnection*>& list = connections();
    for (size_t i = list.size(); i > 0; --i) {
        QConnection* c = list[i - 1];
        if (c->signal == this && c->receiver == receiver) {
            delete c->callback;
            delete c;
            list.erase(list.begin() + (i - 1));
        }
    }
}

void QSignalBase::disconnectAll() { removeConnectionsForSignal(this); }

void qtcRemoveConnectionsFor(QObject* obj) {
    std::vector<QConnection*>& list = connections();
    for (size_t i = list.size(); i > 0; --i) {
        QConnection* c = list[i - 1];
        if (c->sender == obj || c->receiver == obj) {
            delete c->callback;
            delete c;
            list.erase(list.begin() + (i - 1));
        }
    }
}

// ---------------------------------------------------------------------------
// QMetaObject
// ---------------------------------------------------------------------------
int QMetaObject::indexOfProperty(const char* name) const {
    if (!name) return -1;
    for (const QMetaObject* m = this; m; m = m->superClass()) {
        for (int i = 0; i < m->d_propertyCount; ++i) {
            if (strcmp(m->d_properties[i].name, name) == 0) return m->propertyOffset() + i;
        }
    }
    return -1;
}

int QMetaObject::indexOfSignal(const char* name) const {
    if (!name) return -1;
    for (const QMetaObject* m = this; m; m = m->superClass()) {
        for (int i = 0; i < m->d_signalCount; ++i) {
            if (strcmp(m->d_signals[i].name, name) == 0) return m->signalOffset() + i;
        }
    }
    return -1;
}

QMetaProperty QMetaObject::property(int index) const {
    if (index < 0 || index >= propertyCount()) return QMetaProperty();
    return QMetaProperty(this, index);
}

const char* QMetaObject::signalSignature(int index) const {
    const SignalInfo* s = declaringSignal(this, index);
    return s ? s->signature : "";
}

const char* QMetaObject::signalName(int index) const {
    const SignalInfo* s = declaringSignal(this, index);
    return s ? s->name : "";
}

bool QMetaObject::inherits(const QMetaObject* other) const {
    if (!other) return false;
    for (const QMetaObject* m = this; m; m = m->superClass())
        if (m == other) return true;
    return false;
}

// ---------------------------------------------------------------------------
// QMetaProperty
// ---------------------------------------------------------------------------
const char* QMetaProperty::name() const {
    int local = 0;
    const QMetaObject::PropertyData* d = declaringProperty(d_mo, d_index, &local);
    return d ? d->name : "";
}

const char* QMetaProperty::typeName() const {
    const QMetaObject::PropertyData* d = declaringProperty(d_mo, d_index, 0);
    return d ? d->typeName : "";
}

int QMetaProperty::userType() const {
    const QMetaObject::PropertyData* d = declaringProperty(d_mo, d_index, 0);
    return d ? QMetaType::type(d->typeName) : 0;
}

int QMetaProperty::flags() const {
    const QMetaObject::PropertyData* d = declaringProperty(d_mo, d_index, 0);
    return d ? d->flags : 0;
}

bool QMetaProperty::isReadable() const { return (flags() & QMetaObject::Readable) != 0; }
bool QMetaProperty::isWritable() const { return (flags() & QMetaObject::Writable) != 0; }
bool QMetaProperty::isDesignable() const { return (flags() & QMetaObject::Designable) != 0; }
bool QMetaProperty::isEnumType() const { return enumName() != 0 && enumName()[0]; }

const char* QMetaProperty::enumName() const {
    const QMetaObject::PropertyData* d = declaringProperty(d_mo, d_index, 0);
    return (d && d->enumName) ? d->enumName : "";
}

QVariant QMetaProperty::read(const QObject* object) const {
    if (!object || !isReadable()) return QVariant();
    return object->qtcReadProperty(d_index);
}

bool QMetaProperty::write(QObject* object, const QVariant& value) const {
    if (!object || !isWritable()) return false;
    return object->qtcWriteProperty(d_index, value);
}

// ---------------------------------------------------------------------------
// QObject
// ---------------------------------------------------------------------------
static const QMetaObject::PropertyData qtc_props_QObject[] = {
    { "objectName", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QObject[] = {
    { "destroyed", "destroyed(QObject*)" },
    { "objectNameChanged", "objectNameChanged(QString)" }
};
const QMetaObject QObject::staticMetaObject = {
    0, "QObject", qtc_props_QObject, 1, qtc_signals_QObject, 2
};

QObject::QObject(QObject* parent)
    : parent_(0), isWidget_(false), blockSignals_(false) {
    aliveSet().insert(this);
    if (parent) setParent(parent);
}

QObject::~QObject() {
    emit destroyed(this);
    blockSignals_ = true;
    qtcRemoveConnectionsFor(this);
    removeTimersFor(this);
    deleteChildren();
    if (parent_) {
        QObject* p = parent_;
        parent_ = 0;
        p->removeChild(this);
    }
    aliveSet().erase(this);
}

QVariant QObject::qtcReadProperty(int id) const {
    switch (id) {
    case 0: return QVariant(objectName_);
    default: break;
    }
    return QVariant();
}

bool QObject::qtcWriteProperty(int id, const QVariant& value) {
    switch (id) {
    case 0: setObjectName(value.toString()); return true;
    default: break;
    }
    return false;
}

void QObject::setObjectName(const QString& name) {
    if (objectName_ == name) return;
    objectName_ = name;
    objectNameChanged(name);
}

bool QObject::inherits(const char* className) const {
    if (!className) return false;
    for (const QMetaObject* m = metaObject(); m; m = m->superClass())
        if (strcmp(m->className(), className) == 0) return true;
    return false;
}

const char* QObject::className() const {
    const QMetaObject* m = metaObject();
    return m ? m->className() : "";
}

void QObject::setParent(QObject* parent) {
    if (parent_ == parent) return;
    if (parent_) parent_->removeChild(this);
    parent_ = parent;
    if (parent_) parent_->addChild(this);
}

void QObject::addChild(QObject* child) {
    if (!child) return;
    children_.append(child);
    if (!blockSignals_) {
        QChildEvent e(QEvent::ChildAdded, child);
        event(&e);
    }
}

void QObject::removeChild(QObject* child) {
    int idx = children_.indexOf(child);
    if (idx < 0) return;
    children_.removeAt(idx);
    if (!blockSignals_) {
        QChildEvent e(QEvent::ChildRemoved, child);
        event(&e);
    }
}

void QObject::deleteChildren() {
    while (!children_.isEmpty()) {
        QObject* child = children_.at(0);
        delete child;
        int idx = children_.indexOf(child);
        if (idx >= 0) children_.removeAt(idx);
    }
}

QObject* QObject::findChild(const char* className, const QString& name) const {
    if (!className) return 0;
    for (int i = 0; i < children_.count(); ++i) {
        QObject* c = children_.at(i);
        if (c->inherits(className) && (name.isEmpty() || c->objectName() == name)) return c;
    }
    for (int i = 0; i < children_.count(); ++i) {
        QObject* found = children_.at(i)->findChild(className, name);
        if (found) return found;
    }
    return 0;
}

QList<QObject*> QObject::findChildren(const char* className) const {
    QList<QObject*> out;
    for (int i = 0; i < children_.count(); ++i) {
        QObject* c = children_.at(i);
        if (c->inherits(className)) out.append(c);
        out.append(c->findChildren(className));
    }
    return out;
}

bool QObject::blockSignals(bool block) {
    bool old = blockSignals_;
    blockSignals_ = block;
    return old;
}

void QObject::installEventFilter(QObject* filter) {
    if (!filter || eventFilters_.contains(filter)) return;
    eventFilters_.append(filter);
}

void QObject::removeEventFilter(QObject* filter) { eventFilters_.removeAll(filter); }

bool QObject::eventFilter(QObject* /*watched*/, QEvent* /*event*/) { return false; }

bool QObject::event(QEvent* e) {
    switch (e->type()) {
    case QEvent::Timer:
        timerEvent((QTimerEvent*)e);
        return true;
    case QEvent::ChildAdded:
    case QEvent::ChildPolished:
    case QEvent::ChildRemoved:
        childEvent((QChildEvent*)e);
        return true;
    case QEvent::DeferredDelete:
        return true;
    default:
        if (e->type() >= QEvent::User) {
            customEvent(e);
            return true;
        }
        break;
    }
    return false;
}

void QObject::timerEvent(QTimerEvent*) {}
void QObject::childEvent(QChildEvent*) {}
void QObject::customEvent(QEvent*) {}

void QObject::deleteLater() { qtcPostEvent(this, new QDeferredDeleteEvent()); }

int QObject::startTimer(int interval) {
    HWND wnd = timerWindow();
    if (!wnd) return 0;
    if (interval < 0) interval = 0;
    UINT_PTR id = nextTimerId()++;
    if (id == 0) id = nextTimerId()++;
    if (!SetTimer(wnd, id, (UINT)interval, 0)) return 0;
    TimerEntry e;
    e.id = id;
    e.obj = this;
    timers().push_back(e);
    return (int)id;
}

void QObject::killTimer(int timerId) {
    if (timerId <= 0) return;
    std::vector<TimerEntry>& t = timers();
    for (size_t i = 0; i < t.size(); ++i) {
        if (t[i].id == (UINT_PTR)timerId) {
            if (timerWindow()) KillTimer(timerWindow(), t[i].id);
            t.erase(t.begin() + i);
            return;
        }
    }
}

QVariant QObject::property(const char* name) const {
    const QMetaObject* mo = metaObject();
    int idx = mo->indexOfProperty(name);
    if (idx < 0) return QVariant();
    return qtcReadProperty(idx);
}

bool QObject::setProperty(const char* name, const QVariant& value) {
    const QMetaObject* mo = metaObject();
    int idx = mo->indexOfProperty(name);
    if (idx < 0) return false;
    return qtcWriteProperty(idx, value);
}

bool QObject::disconnect(QObject* receiver) {
    std::vector<QConnection*>& list = connections();
    for (size_t i = list.size(); i > 0; --i) {
        QConnection* c = list[i - 1];
        if (c->receiver == receiver) {
            delete c->callback;
            delete c;
            list.erase(list.begin() + (i - 1));
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// posted events
// ---------------------------------------------------------------------------
void qtcPostEvent(QObject* receiver, QEvent* event) {
    if (!receiver || !event) { delete event; return; }
    PostedEvent* p = new PostedEvent();
    p->receiver = receiver;
    p->event = event;
    postedQueue().append(p);
}

void qtcDeliverPostedEvents() {
    if (postedQueue().isEmpty()) return;
    QList<PostedEvent*> queue = postedQueue();
    postedQueue().clearAll();
    for (int i = 0; i < queue.count(); ++i) {
        PostedEvent* p = queue.at(i);
        QObject* receiver = p->receiver;
        QEvent* ev = p->event;
        bool alive = qtcIsAlive(receiver);
        if (alive && ev->type() == QEvent::DeferredDelete) {
            delete receiver;
        } else if (alive) {
            qtcSendEvent(receiver, ev);
        }
        delete ev;
        delete p;
    }
}

bool qtcSendEvent(QObject* receiver, QEvent* event) {
    if (!receiver || !event) return false;
    QList<QObject*> filters = receiver->eventFilters();
    for (int i = 0; i < filters.count(); ++i) {
        QObject* f = filters.at(i);
        if (f && qtcIsAlive(f) && f->eventFilter(receiver, event)) return true;
    }
    return receiver->event(event);
}

// ---------------------------------------------------------------------------
// QTimer
// ---------------------------------------------------------------------------
static const QMetaObject::SignalInfo qtc_signals_QTimer[] = {
    { "timeout", "timeout()" }
};
const QMetaObject QTimer::staticMetaObject = {
    &QObject::staticMetaObject, "QTimer", 0, 0, qtc_signals_QTimer, 1
};

QVariant QTimer::qtcReadProperty(int id) const {
    if (id < QObject::staticMetaObject.propertyCount()) return QObject::qtcReadProperty(id);
    return QVariant();
}

bool QTimer::qtcWriteProperty(int id, const QVariant& value) {
    if (id < QObject::staticMetaObject.propertyCount()) return QObject::qtcWriteProperty(id, value);
    return false;
}

QTimer::QTimer(QObject* parent)
    : QObject(parent), interval_(0), timerId_(0), singleShot_(false) {}

QTimer::~QTimer() { stop(); }

void QTimer::start(int msec) {
    stop();
    interval_ = msec < 0 ? 0 : msec;
    timerId_ = startTimer(interval_);
}

void QTimer::start() { start(interval_); }

void QTimer::stop() {
    if (timerId_ != 0) {
        killTimer(timerId_);
        timerId_ = 0;
    }
}

void QTimer::timerEvent(QTimerEvent* e) {
    if (e->timerId() == timerId_) {
        if (singleShot_) stop();
        timeout();
    } else {
        QObject::timerEvent(e);
    }
}
