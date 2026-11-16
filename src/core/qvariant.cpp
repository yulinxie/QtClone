// QtClone - qvariant.cpp : QMetaType registry and QVariant core conversions.
// GUI value types (QColor/QFont/...) are registered by qvariant_gui.cpp.
#include <qtc/QVariant.h>
#include <stdio.h>
#include <vector>
#include <map>

namespace {

struct Registry {
    std::vector<QMetaType::TypeInfo> types;   // index == meta type id
    std::map<std::string, int> names;
    int nextCustom;
    bool builtinsDone;

    Registry() : nextCustom(QMetaType::User), builtinsDone(false) {
        types.resize(64);
        for (size_t i = 0; i < types.size(); ++i) {
            types[i].name = 0;
            types[i].size = 0;
            types[i].construct = 0;
            types[i].destruct = 0;
            types[i].copy = 0;
        }
    }
};

Registry& reg() {
    static Registry r;
    return r;
}

void ensureSize(int id) {
    Registry& r = reg();
    if ((int)r.types.size() <= id) r.types.resize((size_t)id + 1);
}

template <class T>
QMetaType::TypeInfo makeInfo(const char* name) {
    QMetaType::TypeInfo info;
    info.name = name;
    info.size = (int)sizeof(T);
    return info;
}

} // namespace

void qtcRegisterCoreMetaTypes(); // defined below

int QMetaType::registerTypeInfo(const TypeInfo& info, int id) {
    Registry& r = reg();
    if (id == 0) {
        std::map<std::string, int>::iterator it = r.names.find(std::string(info.name));
        if (it != r.names.end()) return it->second;
        id = r.nextCustom++;
        ensureSize(id);
        r.types[(size_t)id] = info;
        r.names[std::string(info.name)] = id;
        return id;
    }
    ensureSize(id);
    r.types[(size_t)id] = info;
    if (info.name) r.names[std::string(info.name)] = id;
    return id;
}

void qtcRegisterCoreMetaTypes() {
    Registry& r = reg();
    if (r.builtinsDone) return;
    r.builtinsDone = true;

    QMetaType::registerBuiltinType<bool>("bool", QMetaType::Bool);
    QMetaType::registerBuiltinType<int>("int", QMetaType::Int);
    QMetaType::registerBuiltinType<unsigned int>("uint", QMetaType::UInt);
    QMetaType::registerBuiltinType<qint64>("qint64", QMetaType::LongLong);
    QMetaType::registerBuiltinType<quint64>("quint64", QMetaType::ULongLong);
    QMetaType::registerBuiltinType<double>("double", QMetaType::Double);
    QMetaType::registerBuiltinType<QString>("QString", QMetaType::QString);
    QMetaType::registerBuiltinType<QStringList>("QStringList", QMetaType::StringList);
    QMetaType::registerBuiltinType<std::string>("QByteArray", QMetaType::ByteArray);

    qtcRegisterGuiMetaTypes();
}

void qtcRegisterCoreMetaTypes();

static void ensureRegistry() {
    if (!reg().builtinsDone) qtcRegisterCoreMetaTypes();
}

int QMetaType::type(const char* typeName) {
    ensureRegistry();
    if (!typeName) return 0;
    std::map<std::string, int>::iterator it = reg().names.find(std::string(typeName));
    return it == reg().names.end() ? 0 : it->second;
}

const char* QMetaType::typeName(int type) {
    ensureRegistry();
    if (type <= 0 || type >= (int)reg().types.size()) return 0;
    return reg().types[(size_t)type].name;
}

int QMetaType::sizeOf(int type) {
    ensureRegistry();
    if (type <= 0 || type >= (int)reg().types.size()) return 0;
    return reg().types[(size_t)type].size;
}

bool QMetaType::isRegistered(int type) {
    ensureRegistry();
    return type > 0 && type < (int)reg().types.size() && reg().types[(size_t)type].name != 0;
}

void* QMetaType::create(int type) {
    if (!isRegistered(type)) return 0;
    const TypeInfo& info = reg().types[(size_t)type];
    void* p = ::operator new((size_t)info.size);
    info.construct(p);
    return p;
}

void QMetaType::destroy(int type, void* data) {
    ensureRegistry();
    if (!data) return;
    if (isRegistered(type)) reg().types[(size_t)type].destruct(data);
    ::operator delete(data);
}

void QMetaType::construct(int type, void* data) {
    if (data && isRegistered(type)) reg().types[(size_t)type].construct(data);
}

void QMetaType::destruct(int type, void* data) {
    if (data && isRegistered(type)) reg().types[(size_t)type].destruct(data);
}

void QMetaType::copyValue(int type, void* dst, const void* src) {
    ensureRegistry();
    if (dst && src && isRegistered(type)) reg().types[(size_t)type].copy(dst, src);
}

// ---------------------------------------------------------------------------
// QVariant
// ---------------------------------------------------------------------------
QVariant::QVariant() : type_(QMetaType::UnknownType), data_(0) {}

QVariant::QVariant(QMetaType::Type type) : type_(QMetaType::UnknownType), data_(0) {
    init((int)type, 0);
}

QVariant::QVariant(int typeId, const void* copy) : type_(QMetaType::UnknownType), data_(0) {
    init(typeId, copy);
}

QVariant::QVariant(const QVariant& other) : type_(QMetaType::UnknownType), data_(0) {
    init(other.type_, other.data_);
}

QVariant::~QVariant() { clear(); }

void QVariant::init(int type, const void* copy) {
    type_ = QMetaType::UnknownType;
    data_ = 0;
    if (!QMetaType::isRegistered(type)) {
        if (type != QMetaType::UnknownType) {
            // keep the declared type even when it cannot be constructed
            type_ = type;
        }
        return;
    }
    data_ = QMetaType::create(type);
    if (copy && data_) QMetaType::copyValue(type, data_, copy);
    type_ = type;
}

void QVariant::clear() {
    if (data_) {
        QMetaType::destroy(type_, data_);
        data_ = 0;
    }
    type_ = QMetaType::UnknownType;
}

QVariant& QVariant::operator=(const QVariant& other) {
    if (this == &other) return *this;
    clear();
    init(other.type_, other.data_);
    return *this;
}

QVariant::QVariant(bool v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Bool, &v); }
QVariant::QVariant(int v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Int, &v); }
QVariant::QVariant(uint v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::UInt, &v); }
QVariant::QVariant(qint64 v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::LongLong, &v); }
QVariant::QVariant(quint64 v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::ULongLong, &v); }
QVariant::QVariant(double v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Double, &v); }

QVariant::QVariant(const QString& v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::QString, &v); }
QVariant::QVariant(const wchar_t* v) : type_(QMetaType::UnknownType), data_(0) {
    QString s(v ? v : L"");
    init(QMetaType::QString, &s);
}
QVariant::QVariant(const char* v) : type_(QMetaType::UnknownType), data_(0) {
    QString s(qtcFromUtf8(v));
    init(QMetaType::QString, &s);
}
QVariant::QVariant(const QStringList& v) : type_(QMetaType::UnknownType), data_(0) {
    init(QMetaType::StringList, &v);
}

const char* QVariant::typeName() const { return QMetaType::typeName(type_); }

bool QVariant::toBool() const {
    switch (type_) {
    case QMetaType::Bool: return data_ ? *(bool*)data_ : false;
    case QMetaType::Int: return data_ ? (*(int*)data_ != 0) : false;
    case QMetaType::UInt: return data_ ? (*(uint*)data_ != 0) : false;
    case QMetaType::LongLong: return data_ ? (*(qint64*)data_ != 0) : false;
    case QMetaType::Double: return data_ ? (*(double*)data_ != 0.0) : false;
    case QMetaType::QString: return data_ ? ((QString*)data_)->toBool() : false;
    default: return false;
    }
}

int QVariant::toInt(bool* ok) const {
    bool good = true;
    int r = 0;
    switch (type_) {
    case QMetaType::Bool: r = (data_ && *(bool*)data_) ? 1 : 0; break;
    case QMetaType::Int: r = data_ ? *(int*)data_ : 0; break;
    case QMetaType::UInt: r = data_ ? (int)*(uint*)data_ : 0; break;
    case QMetaType::LongLong: r = data_ ? (int)*(qint64*)data_ : 0; break;
    case QMetaType::ULongLong: r = data_ ? (int)*(quint64*)data_ : 0; break;
    case QMetaType::Double: r = data_ ? qRound(*(double*)data_) : 0; break;
    case QMetaType::QString: r = data_ ? ((QString*)data_)->toInt(&good) : 0; break;
    default: good = false; break;
    }
    if (ok) *ok = good;
    return good ? r : 0;
}

uint QVariant::toUInt(bool* ok) const {
    bool good = true;
    uint r = 0;
    switch (type_) {
    case QMetaType::UInt: r = data_ ? *(uint*)data_ : 0; break;
    case QMetaType::Int: r = data_ ? (uint)*(int*)data_ : 0; break;
    case QMetaType::Bool: r = (data_ && *(bool*)data_) ? 1u : 0u; break;
    case QMetaType::Double: r = data_ ? (uint)qRound(*(double*)data_) : 0; break;
    case QMetaType::QString: r = data_ ? (uint)((QString*)data_)->toInt(&good) : 0; break;
    default: good = false; break;
    }
    if (ok) *ok = good;
    return good ? r : 0;
}

qint64 QVariant::toLongLong(bool* ok) const {
    bool good = true;
    qint64 r = 0;
    switch (type_) {
    case QMetaType::LongLong: r = data_ ? *(qint64*)data_ : 0; break;
    case QMetaType::Int: r = data_ ? *(int*)data_ : 0; break;
    case QMetaType::UInt: r = data_ ? *(uint*)data_ : 0; break;
    case QMetaType::Bool: r = (data_ && *(bool*)data_) ? 1 : 0; break;
    case QMetaType::Double: r = data_ ? (qint64)qRound(*(double*)data_) : 0; break;
    case QMetaType::QString: r = data_ ? ((QString*)data_)->toLongLong(&good) : 0; break;
    default: good = false; break;
    }
    if (ok) *ok = good;
    return good ? r : 0;
}

double QVariant::toDouble(bool* ok) const {
    bool good = true;
    double r = 0.0;
    switch (type_) {
    case QMetaType::Double: r = data_ ? *(double*)data_ : 0.0; break;
    case QMetaType::Int: r = data_ ? *(int*)data_ : 0.0; break;
    case QMetaType::UInt: r = data_ ? *(uint*)data_ : 0.0; break;
    case QMetaType::LongLong: r = data_ ? (double)*(qint64*)data_ : 0.0; break;
    case QMetaType::Bool: r = (data_ && *(bool*)data_) ? 1.0 : 0.0; break;
    case QMetaType::QString: r = data_ ? ((QString*)data_)->toDouble(&good) : 0.0; break;
    default: good = false; break;
    }
    if (ok) *ok = good;
    return good ? r : 0.0;
}

QString QVariant::toString() const {
    switch (type_) {
    case QMetaType::QString: return data_ ? *(QString*)data_ : QString();
    case QMetaType::StringList:
        if (!data_) return QString();
        return QStringList(*((QStringList*)data_)).join(L", ");
    case QMetaType::Bool: return (data_ && *(bool*)data_) ? QString(L"true") : QString(L"false");
    case QMetaType::Int: return QString::number(data_ ? *(int*)data_ : 0);
    case QMetaType::UInt: return QString::number((int)(data_ ? *(uint*)data_ : 0));
    case QMetaType::LongLong: return QString::number(data_ ? *(qint64*)data_ : 0);
    case QMetaType::Double: return QString::number(data_ ? *(double*)data_ : 0.0, 'g', 6);
    case QMetaType::ByteArray: return data_ ? QString::fromUtf8(((std::string*)data_)->c_str()) : QString();
    case QMetaType::UnknownType: return QString();
    default: {
        QString text;
        if (data_ && qtcGuiVariantToString(type_, data_, &text)) return text;
        return QString();
    }
    }
}

QStringList QVariant::toStringList() const {
    if (type_ == QMetaType::StringList && data_) return *(QStringList*)data_;
    if (type_ == QMetaType::QString && data_) return ((QString*)data_)->split(L'\n');
    return QStringList();
}

QByteArray QVariant::toByteArray() const {
    if (type_ == QMetaType::ByteArray && data_) return *(std::string*)data_;
    return toString().toUtf8();
}

bool QVariant::canConvert(int targetType) const {
    if (type_ == QMetaType::UnknownType) return false;
    if (targetType == type_) return true;
    switch (targetType) {
    case QMetaType::QString: return true;
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::Double:
    case QMetaType::Bool:
        return type_ == QMetaType::StringList ? false : true;
    case QMetaType::StringList: return type_ == QMetaType::QString;
    default: return false;
    }
}

bool QVariant::convert(int targetType) {
    if (targetType == type_) return true;
    if (!canConvert(targetType)) return false;
    QString s;
    bool useString = (type_ == QMetaType::QString);
    if (useString && data_) s = *(QString*)data_;
    int oldType = type_;
    switch (targetType) {
    case QMetaType::Int: { int v = toInt(); clear(); init(QMetaType::Int, &v); return true; }
    case QMetaType::UInt: { uint v = toUInt(); clear(); init(QMetaType::UInt, &v); return true; }
    case QMetaType::LongLong: { qint64 v = toLongLong(); clear(); init(QMetaType::LongLong, &v); return true; }
    case QMetaType::Double: { double v = toDouble(); clear(); init(QMetaType::Double, &v); return true; }
    case QMetaType::Bool: { bool v = toBool(); clear(); init(QMetaType::Bool, &v); return true; }
    case QMetaType::QString: { QString v = toString(); clear(); init(QMetaType::QString, &v); return true; }
    case QMetaType::StringList: { QStringList v = s.split(L'\n'); clear(); init(QMetaType::StringList, &v); return true; }
    default: (void)oldType; return false;
    }
}

bool QVariant::operator==(const QVariant& other) const {
    if (type_ != other.type_) {
        if (canConvert(other.type_)) return toString() == other.toString();
        return false;
    }
    if (!data_ || !other.data_) return data_ == other.data_;
    switch (type_) {
    case QMetaType::Bool: return *(bool*)data_ == *(bool*)other.data_;
    case QMetaType::Int: return *(int*)data_ == *(int*)other.data_;
    case QMetaType::UInt: return *(uint*)data_ == *(uint*)other.data_;
    case QMetaType::LongLong: return *(qint64*)data_ == *(qint64*)other.data_;
    case QMetaType::Double: return *(double*)data_ == *(double*)other.data_;
    case QMetaType::QString: return *(QString*)data_ == *(QString*)other.data_;
    default: return toString() == other.toString();
    }
}
