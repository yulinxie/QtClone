// QtClone - QVariant.h : type-erased value + QMetaType registry.
#ifndef QTC_QVARIANT_H
#define QTC_QVARIANT_H

#include <qtc/QtGlobal.h>
#include <qtc/QString.h>
#include <new>

class QColor;
class QFont;
class QPixmap;
class QImage;
class QIcon;
class QBrush;
class QPen;
class QCursor;
class QPoint;
class QSize;
class QRect;
class QPointF;
class QRectF;

class QMetaType {
private:
    template <class T>
    struct QMetaTypeOps {
        static void construct(void* p) { new (p) T(); }
        static void destruct(void* p) { static_cast<T*>(p)->~T(); }
        static void copy(void* d, const void* s) { *static_cast<T*>(d) = *static_cast<const T*>(s); }
    };

public:
    enum Type {
        UnknownType = 0,
        Bool = 1,
        Int = 2,
        UInt = 3,
        LongLong = 4,
        ULongLong = 5,
        Double = 6,
        QString = 10,
        StringList = 11,
        ByteArray = 12,
        Rect = 20,
        Size = 21,
        Point = 22,
        Color = 40,
        Font = 41,
        Pixmap = 42,
        Image = 43,
        Brush = 44,
        Pen = 45,
        Icon = 46,
        Cursor = 47,
        Variant = 48,
        Void = 49,
        User = 1000
    };

    struct TypeInfo {
        const char* name;
        int size;
        void (*construct)(void*);
        void (*destruct)(void*);
        void (*copy)(void* dst, const void* src);
    };

    static int type(const char* typeName);
    static const char* typeName(int type);
    static int sizeOf(int type);
    static bool isRegistered(int type);
    static void* create(int type);
    static void destroy(int type, void* data);
    static void construct(int type, void* data);
    static void destruct(int type, void* data);
    static void copyValue(int type, void* dst, const void* src);
    static int registerTypeInfo(const TypeInfo& info, int id = 0);

    template <class T>
    static int registerType(const char* name) {
        int existing = type(name);
        if (existing) return existing;
        TypeInfo info;
        info.name = name;
        info.size = (int)sizeof(T);
        info.construct = &QMetaTypeOps<T>::construct;
        info.destruct = &QMetaTypeOps<T>::destruct;
        info.copy = &QMetaTypeOps<T>::copy;
        return registerTypeInfo(info, 0);
    }

    template <class T>
    static int registerBuiltinType(const char* name, int id) {
        TypeInfo info;
        info.name = name;
        info.size = (int)sizeof(T);
        info.construct = &QMetaTypeOps<T>::construct;
        info.destruct = &QMetaTypeOps<T>::destruct;
        info.copy = &QMetaTypeOps<T>::copy;
        return registerTypeInfo(info, id);
    }
};

// Template hook used to map a C++ type to its meta type id.
// Specialisations are provided for the built-in types below; user types are
// enabled with Q_DECLARE_METATYPE(Type).
template <class T>
struct QMetaTypeName {
    static const char* name() { return "unknown"; }
};

template <class T>
inline int qMetaTypeId() {
    static int id = QMetaType::registerType<T>(QMetaTypeName<T>::name());
    return id;
}

#define Q_DECLARE_METATYPE(TYPE) \
    template <> struct QMetaTypeName<TYPE> { \
        static const char* name() { return #TYPE; } \
    };

Q_DECLARE_METATYPE(bool)
Q_DECLARE_METATYPE(int)
Q_DECLARE_METATYPE(unsigned int)
Q_DECLARE_METATYPE(qint64)
Q_DECLARE_METATYPE(quint64)
Q_DECLARE_METATYPE(double)
Q_DECLARE_METATYPE(QString)
Q_DECLARE_METATYPE(QStringList)

class QVariant {
public:
    QVariant();
    QVariant(const QVariant& other);
    ~QVariant();

    QVariant(bool v);
    QVariant(int v);
    QVariant(uint v);
    QVariant(qint64 v);
    QVariant(quint64 v);
    QVariant(double v);
    QVariant(const QString& v);
    QVariant(const wchar_t* v);
    QVariant(const char* v);
    QVariant(const QStringList& v);
    QVariant(const QColor& v);
    QVariant(const QFont& v);
    QVariant(const QPixmap& v);
    QVariant(const QImage& v);
    QVariant(const QIcon& v);
    QVariant(const QPoint& v);
    QVariant(const QSize& v);
    QVariant(const QRect& v);

    // creates an empty (default constructed) value of the given meta type
    QVariant(QMetaType::Type type);
    QVariant(int typeId, const void* copy);

    QVariant& operator=(const QVariant& other);

    bool isValid() const { return type_ != QMetaType::UnknownType && data_ != 0; }
    bool isNull() const { return !isValid(); }
    void clear();

    int type() const { return type_; }
    const char* typeName() const;
    void* data() { return data_; }
    const void* constData() const { return data_; }

    bool toBool() const;
    int toInt(bool* ok = 0) const;
    uint toUInt(bool* ok = 0) const;
    qint64 toLongLong(bool* ok = 0) const;
    double toDouble(bool* ok = 0) const;
    QString toString() const;
    QStringList toStringList() const;
    QByteArray toByteArray() const;
    QColor toColor() const;
    QFont toFont() const;
    QPixmap toPixmap() const;
    QImage toImage() const;
    QIcon toIcon() const;
    QPoint toPoint() const;
    QSize toSize() const;
    QRect toRect() const;

    bool canConvert(int targetType) const;
    bool convert(int targetType);

    template <class T> T value() const;
    template <class T> static QVariant fromValue(const T& t) {
        return QVariant(qMetaTypeId<T>(), &t);
    }

    bool operator==(const QVariant& other) const;
    bool operator!=(const QVariant& other) const { return !(*this == other); }

private:
    void init(int type, const void* copy);
    int type_;
    void* data_;
};

// ---------------------------------------------------------------------------
// value<T>() helper specialisations
// ---------------------------------------------------------------------------
template <class T>
struct QVariantValue {
    static T get(const QVariant&) { Q_ASSERT(0 && "QVariant::value<T> called for an unsupported type"); return T(); }
};

template <> struct QVariantValue<bool> {
    static bool get(const QVariant& v) { return v.toBool(); }
};
template <> struct QVariantValue<int> {
    static int get(const QVariant& v) { return v.toInt(); }
};
template <> struct QVariantValue<uint> {
    static uint get(const QVariant& v) { return v.toUInt(); }
};
template <> struct QVariantValue<qint64> {
    static qint64 get(const QVariant& v) { return v.toLongLong(); }
};
template <> struct QVariantValue<double> {
    static double get(const QVariant& v) { return v.toDouble(); }
};
template <> struct QVariantValue<QString> {
    static QString get(const QVariant& v) { return v.toString(); }
};
template <> struct QVariantValue<QStringList> {
    static QStringList get(const QVariant& v) { return v.toStringList(); }
};

template <class T>
inline T QVariant::value() const { return QVariantValue<T>::get(*this); }

// Implemented by the QtGui layer; called lazily the first time the meta type
// registry is used so that QColor/QFont/... ids are known to QVariant.
void qtcRegisterGuiMetaTypes();

// Implemented by the QtGui layer: textual form of GUI value types.
bool qtcGuiVariantToString(int type, const void* data, QString* out);

#endif // QTC_QVARIANT_H
