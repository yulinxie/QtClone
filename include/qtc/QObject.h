// QtClone - QObject.h : meta object system without any code generator.
//
// Design notes (see docs/REFLECTION.md):
//   * QMetaObject is a POD aggregate initialised statically, so there is no
//     static initialisation order problem across translation units.
//   * Properties are read/written through virtual qtcReadProperty()/
//     qtcWriteProperty() switches implemented by hand in each class.  The
//     metadata table only describes names/types for the designer.
//   * Signals are typed member objects (Signal<void(int)> etc.).  Qt's
//     `emit sig(args)` syntax keeps working because emit expands to nothing.
//   * Connecting uses member function pointers, so it is type safe and needs
//     no string parsing or generated dispatch code.
#ifndef QTC_QOBJECT_H
#define QTC_QOBJECT_H

#include <qtc/QtGlobal.h>
#include <qtc/QString.h>
#include <qtc/QList.h>
#include <qtc/QVariant.h>

class QObject;
class QEvent;
class QWidget;
class QTimerEvent;
class QChildEvent;
class QMetaProperty;
class QSlotCallback;

// ---------------------------------------------------------------------------
// signal argument traits: strip const/reference for storage
// ---------------------------------------------------------------------------
template <class T> struct QSignalType { typedef T Storage; typedef T Param; };
template <class T> struct QSignalType<const T&> { typedef T Storage; typedef const T& Param; };
template <class T> struct QSignalType<T&> { typedef T Storage; typedef T& Param; };

// ---------------------------------------------------------------------------
// QSlotCallback: type erased callable bound to one receiver
// ---------------------------------------------------------------------------
class QSlotCallback {
public:
    virtual ~QSlotCallback() {}
    virtual void call(void** args) = 0;
};

template <class R>
class QSlot0 : public QSlotCallback {
public:
    typedef void (R::*Fun)();
    QSlot0(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void**) { (obj_->*f_)(); }
private:
    R* obj_;
    Fun f_;
};

template <class R, class A1>
class QSlot1 : public QSlotCallback {
public:
    typedef void (R::*Fun)(A1);
    typedef typename QSignalType<A1>::Storage S1;
    QSlot1(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void** a) { (obj_->*f_)(*static_cast<S1*>(a[0])); }
private:
    R* obj_;
    Fun f_;
};

template <class R, class A1>
class QSlot1Ignoring : public QSlotCallback {
public:
    typedef void (R::*Fun)();
    QSlot1Ignoring(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void**) { (obj_->*f_)(); }
private:
    R* obj_;
    Fun f_;
};

template <class R, class A1, class A2>
class QSlot2 : public QSlotCallback {
public:
    typedef void (R::*Fun)(A1, A2);
    typedef typename QSignalType<A1>::Storage S1;
    typedef typename QSignalType<A2>::Storage S2;
    QSlot2(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void** a) { (obj_->*f_)(*static_cast<S1*>(a[0]), *static_cast<S2*>(a[1])); }
private:
    R* obj_;
    Fun f_;
};

template <class R, class A1, class A2>
class QSlot2a : public QSlotCallback {
public:
    typedef void (R::*Fun)(A1);
    typedef typename QSignalType<A1>::Storage S1;
    QSlot2a(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void** a) { (obj_->*f_)(*static_cast<S1*>(a[0])); }
private:
    R* obj_;
    Fun f_;
};

template <class R, class A1, class A2>
class QSlot2Ignoring : public QSlotCallback {
public:
    typedef void (R::*Fun)();
    QSlot2Ignoring(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void**) { (obj_->*f_)(); }
private:
    R* obj_;
    Fun f_;
};

template <class R, class A1, class A2, class A3>
class QSlot3 : public QSlotCallback {
public:
    typedef void (R::*Fun)(A1, A2, A3);
    typedef typename QSignalType<A1>::Storage S1;
    typedef typename QSignalType<A2>::Storage S2;
    typedef typename QSignalType<A3>::Storage S3;
    QSlot3(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void** a) {
        (obj_->*f_)(*static_cast<S1*>(a[0]), *static_cast<S2*>(a[1]), *static_cast<S3*>(a[2]));
    }
private:
    R* obj_;
    Fun f_;
};

template <class R, class A1, class A2, class A3>
class QSlot3b : public QSlotCallback {
public:
    typedef void (R::*Fun)(A1, A2);
    typedef typename QSignalType<A1>::Storage S1;
    typedef typename QSignalType<A2>::Storage S2;
    QSlot3b(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void** a) { (obj_->*f_)(*static_cast<S1*>(a[0]), *static_cast<S2*>(a[1])); }
private:
    R* obj_;
    Fun f_;
};

template <class R, class A1, class A2, class A3>
class QSlot3c : public QSlotCallback {
public:
    typedef void (R::*Fun)(A1);
    typedef typename QSignalType<A1>::Storage S1;
    QSlot3c(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void** a) { (obj_->*f_)(*static_cast<S1*>(a[0])); }
private:
    R* obj_;
    Fun f_;
};

template <class R, class A1, class A2, class A3>
class QSlot3Ignoring : public QSlotCallback {
public:
    typedef void (R::*Fun)();
    QSlot3Ignoring(R* obj, Fun f) : obj_(obj), f_(f) {}
    void call(void**) { (obj_->*f_)(); }
private:
    R* obj_;
    Fun f_;
};

// ---------------------------------------------------------------------------
// QSignalBase
// ---------------------------------------------------------------------------
class QSignalBase {
public:
    QSignalBase();
    virtual ~QSignalBase();

    int connectionCount() const;
    void disconnectAll();
    void disconnect(QObject* receiver);

    // internal connection plumbing
    bool addConnection(QObject* sender, QObject* receiver, QSlotCallback* callback);
    void emitArgs(void** args, int argc);

private:
    QSignalBase(const QSignalBase&);
    QSignalBase& operator=(const QSignalBase&);
    unsigned int id_;
};

// ---------------------------------------------------------------------------
// typed signals
// ---------------------------------------------------------------------------
template <class Signature> class Signal;

template <>
class Signal<void()> : public QSignalBase {
public:
    void operator()() {
        void* none = 0;
        emitArgs(&none, 0);
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)()) {
        return addConnection(sender, receiver, new QSlot0<R>(receiver, slot));
    }
};

template <class A1>
class Signal<void(A1)> : public QSignalBase {
public:
    typedef typename QSignalType<A1>::Storage S1;
    typedef typename QSignalType<A1>::Param P1;

    void operator()(P1 a1) {
        void* args[1];
        args[0] = const_cast<S1*>(reinterpret_cast<const S1*>(&a1));
        emitArgs(args, 1);
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)(A1)) {
        return addConnection(sender, receiver, new QSlot1<R, A1>(receiver, slot));
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)()) {
        return addConnection(sender, receiver, new QSlot1Ignoring<R, A1>(receiver, slot));
    }
};

template <class A1, class A2>
class Signal<void(A1, A2)> : public QSignalBase {
public:
    typedef typename QSignalType<A1>::Storage S1;
    typedef typename QSignalType<A2>::Storage S2;
    typedef typename QSignalType<A1>::Param P1;
    typedef typename QSignalType<A2>::Param P2;

    void operator()(P1 a1, P2 a2) {
        void* args[2];
        args[0] = const_cast<S1*>(reinterpret_cast<const S1*>(&a1));
        args[1] = const_cast<S2*>(reinterpret_cast<const S2*>(&a2));
        emitArgs(args, 2);
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)(A1, A2)) {
        return addConnection(sender, receiver, new QSlot2<R, A1, A2>(receiver, slot));
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)(A1)) {
        return addConnection(sender, receiver, new QSlot2a<R, A1, A2>(receiver, slot));
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)()) {
        return addConnection(sender, receiver, new QSlot2Ignoring<R, A1, A2>(receiver, slot));
    }
};

template <class A1, class A2, class A3>
class Signal<void(A1, A2, A3)> : public QSignalBase {
public:
    typedef typename QSignalType<A1>::Storage S1;
    typedef typename QSignalType<A2>::Storage S2;
    typedef typename QSignalType<A3>::Storage S3;
    typedef typename QSignalType<A1>::Param P1;
    typedef typename QSignalType<A2>::Param P2;
    typedef typename QSignalType<A3>::Param P3;

    void operator()(P1 a1, P2 a2, P3 a3) {
        void* args[3];
        args[0] = const_cast<S1*>(reinterpret_cast<const S1*>(&a1));
        args[1] = const_cast<S2*>(reinterpret_cast<const S2*>(&a2));
        args[2] = const_cast<S3*>(reinterpret_cast<const S3*>(&a3));
        emitArgs(args, 3);
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)(A1, A2, A3)) {
        return addConnection(sender, receiver, new QSlot3<R, A1, A2, A3>(receiver, slot));
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)(A1, A2)) {
        return addConnection(sender, receiver, new QSlot3b<R, A1, A2, A3>(receiver, slot));
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)(A1)) {
        return addConnection(sender, receiver, new QSlot3c<R, A1, A2, A3>(receiver, slot));
    }
    template <class R>
    bool connect(QObject* sender, R* receiver, void (R::*slot)()) {
        return addConnection(sender, receiver, new QSlot3Ignoring<R, A1, A2, A3>(receiver, slot));
    }
};

// ---------------------------------------------------------------------------
// QMetaObject : statically initialised metadata (aggregate)
// ---------------------------------------------------------------------------
struct QMetaObject {
    struct PropertyData {
        const char* name;
        const char* typeName;
        int flags;
        const char* enumName;   // e.g. "Qt::Alignment" when the property is an enum
    };
    struct SignalInfo {
        const char* name;
        const char* signature;  // "clicked(bool)"
    };

    enum PropertyFlags {
        Readable = 0x1,
        Writable = 0x2,
        Designable = 0x8,
        Constant = 0x400
    };

    const QMetaObject* superdata;
    const char* d_className;
    const PropertyData* d_properties;
    int d_propertyCount;
    const SignalInfo* d_signals;
    int d_signalCount;

    const char* className() const { return d_className; }
    const QMetaObject* superClass() const { return superdata; }

    int propertyCount() const { return (superdata ? superdata->propertyCount() : 0) + d_propertyCount; }
    int propertyOffset() const { return superdata ? superdata->propertyCount() : 0; }
    int signalCount() const { return (superdata ? superdata->signalCount() : 0) + d_signalCount; }
    int signalOffset() const { return superdata ? superdata->signalCount() : 0; }

    int indexOfProperty(const char* name) const;
    int indexOfSignal(const char* name) const;
    QMetaProperty property(int index) const;
    const char* signalSignature(int index) const;
    const char* signalName(int index) const;

    bool inherits(const QMetaObject* other) const;
};

// ---------------------------------------------------------------------------
// QMetaProperty
// ---------------------------------------------------------------------------
class QMetaProperty {
public:
    QMetaProperty() : d_mo(0), d_index(-1) {}
    QMetaProperty(const QMetaObject* mo, int index) : d_mo(mo), d_index(index) {}

    bool isValid() const { return d_mo != 0 && d_index >= 0; }
    const char* name() const;
    const char* typeName() const;
    int userType() const;
    int flags() const;
    bool isReadable() const;
    bool isWritable() const;
    bool isDesignable() const;
    bool isEnumType() const;
    const char* enumName() const;

    QVariant read(const QObject* object) const;
    bool write(QObject* object, const QVariant& value) const;

private:
    const QMetaObject* d_mo;
    int d_index;
};

// ---------------------------------------------------------------------------
// Q_OBJECT replacement
// ---------------------------------------------------------------------------
#define QTC_OBJECT(CLASS, BASE) \
public: \
    static const QMetaObject staticMetaObject; \
    virtual const QMetaObject* metaObject() const { return &CLASS::staticMetaObject; } \
    virtual QVariant qtcReadProperty(int id) const; \
    virtual bool qtcWriteProperty(int id, const QVariant& value); \
private:
    typedef void qtc_metaobject_tag;

// ---------------------------------------------------------------------------
// QObject
// ---------------------------------------------------------------------------
class QObject {
    QTC_OBJECT(QObject, QObject);
public:
    explicit QObject(QObject* parent = 0);
    virtual ~QObject();

    QString objectName() const { return objectName_; }
    void setObjectName(const QString& name);

    bool isWidgetType() const { return isWidget_; }
    bool inherits(const char* className) const;
    const char* className() const;

    QObject* parent() const { return parent_; }
    void setParent(QObject* parent);
    QList<QObject*> children() const { return children_; }
    QList<QObject*> findChildren(const char* className) const;
    QObject* findChild(const char* className, const QString& name = QString()) const;

    template <class T> QList<T*> findChildren() const {
        QList<T*> out;
        for (int i = 0; i < children_.count(); ++i) {
            T* t = dynamic_cast<T*>(children_.at(i));
            if (t) out.append(t);
            out.append(children_.at(i)->findChildren<T>());
        }
        return out;
    }

    template <class T> T* findChild(const QString& name = QString()) const {
        for (int i = 0; i < children_.count(); ++i) {
            T* t = dynamic_cast<T*>(children_.at(i));
            if (t && (name.isEmpty() || t->objectName() == name)) return t;
        }
        for (int i = 0; i < children_.count(); ++i) {
            T* t = children_.at(i)->findChild<T>(name);
            if (t) return t;
        }
        return 0;
    }

    bool blockSignals(bool block);
    bool signalsBlocked() const { return blockSignals_; }

    void installEventFilter(QObject* filter);
    void removeEventFilter(QObject* filter);
    QList<QObject*> eventFilters() const { return eventFilters_; }
    virtual bool eventFilter(QObject* watched, QEvent* event);

    virtual bool event(QEvent* event);
    void deleteLater();

    int startTimer(int interval);
    void killTimer(int timerId);

    QVariant property(const char* name) const;
    bool setProperty(const char* name, const QVariant& value);

    // Typed connection: qobject->connect(&emitter, emitter.sig, receiver, &R::slot)
    template <class S, class R>
    static bool connect(QObject* sender, S& signal, R* receiver, void (R::*slot)()) {
        return signal.connect(sender, receiver, slot);
    }
    template <class S, class R, class A1>
    static bool connect(QObject* sender, S& signal, R* receiver, void (R::*slot)(A1)) {
        return signal.connect(sender, receiver, slot);
    }
    template <class S, class R, class A1, class A2>
    static bool connect(QObject* sender, S& signal, R* receiver, void (R::*slot)(A1, A2)) {
        return signal.connect(sender, receiver, slot);
    }
    template <class S, class R, class A1, class A2, class A3>
    static bool connect(QObject* sender, S& signal, R* receiver, void (R::*slot)(A1, A2, A3)) {
        return signal.connect(sender, receiver, slot);
    }

    static bool disconnect(QObject* sender, QSignalBase& signal, QObject* receiver) {
        (void)sender;
        signal.disconnect(receiver);
        return true;
    }
    static bool disconnect(QObject* receiver);

signals:
    Signal<void(QObject*)> destroyed;
    Signal<void(const QString&)> objectNameChanged;

protected:
    virtual void timerEvent(QTimerEvent* event);
    virtual void childEvent(QChildEvent* event);
    virtual void customEvent(QEvent* event);

    void setWidgetType(bool on) { isWidget_ = on; }

private:
    friend class QWidget;
    friend class QTimer;
    friend void qtcDeliverPostedEvents();
    friend void qtcRemoveConnectionsFor(QObject*);

    void addChild(QObject* child);
    void removeChild(QObject* child);
    void deleteChildren();

    QString objectName_;
    QObject* parent_;
    QList<QObject*> children_;
    QList<QObject*> eventFilters_;
    bool isWidget_;
    bool blockSignals_;
};

void qtcPostEvent(QObject* receiver, QEvent* event);
void qtcDeliverPostedEvents();
bool qtcSendEvent(QObject* receiver, QEvent* event);
bool qtcIsAlive(QObject* obj);
QObject* qtcCurrentSender();
void qtcSetCurrentSender(QObject* sender);
void qtcRemoveConnectionsFor(QObject* obj);

// ---------------------------------------------------------------------------
// QTimer
// ---------------------------------------------------------------------------
class QTimer : public QObject {
    QTC_OBJECT(QTimer, QObject);
public:
    explicit QTimer(QObject* parent = 0);
    ~QTimer();

    bool isActive() const { return timerId_ != 0; }
    int timerId() const { return timerId_; }
    int interval() const { return interval_; }
    void setInterval(int msec) { interval_ = msec; }
    bool isSingleShot() const { return singleShot_; }
    void setSingleShot(bool singleShot) { singleShot_ = singleShot; }

    template <class R>
    static void singleShot(int msec, R* receiver, void (R::*slot)());

public slots:
    void start(int msec);
    void start();
    void stop();

signals:
    Signal<void()> timeout;

protected:
    void timerEvent(QTimerEvent* e);

private:
    int interval_;
    int timerId_;
    bool singleShot_;
};

template <class R>
void QTimer::singleShot(int msec, R* receiver, void (R::*slot)()) {
    QTimer* t = new QTimer(receiver);
    t->setSingleShot(true);
    t->setInterval(msec);
    t->timeout.connect(t, receiver, slot);
    t->start();
}

#endif // QTC_QOBJECT_H
