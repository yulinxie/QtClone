// QtClone - qstring.cpp
#include <qtc/QString.h>
#include <windows.h>
#include <stdio.h>
#include <wctype.h>
#include <vector>

std::wstring qtcFromUtf8(const char* s, int len) {
    if (!s) return std::wstring();
    int n = (len < 0) ? (int)strlen(s) : len;
    if (n <= 0) return std::wstring();
    int wlen = MultiByteToWideChar(CP_UTF8, 0, s, n, 0, 0);
    if (wlen <= 0) {
        // not valid UTF-8: fall back to Latin-1 style widening
        std::wstring w;
        w.reserve(n);
        for (int i = 0; i < n; ++i) w += (wchar_t)(unsigned char)s[i];
        return w;
    }
    std::wstring w;
    w.resize((size_t)wlen);
    MultiByteToWideChar(CP_UTF8, 0, s, n, &w[0], wlen);
    return w;
}

std::string qtcToUtf8(const std::wstring& s) {
    if (s.empty()) return std::string();
    int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), 0, 0, 0, 0);
    if (n <= 0) return std::string();
    std::string out;
    out.resize((size_t)n);
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], n, 0, 0);
    return out;
}

static bool qtcIsSpace(wchar_t c) {
    return c == L' ' || c == L'\t' || c == L'\n' || c == L'\r' || c == L'\f' || c == L'\v';
}

QString QString::trimmed() const {
    int b = 0, e = size();
    while (b < e && qtcIsSpace((*this)[b])) ++b;
    while (e > b && qtcIsSpace((*this)[e - 1])) --e;
    return QString(c_str() + b, e - b);
}

QString QString::simplified() const {
    QString out;
    bool lastSpace = false;
    for (int i = 0; i < size(); ++i) {
        wchar_t c = (*this)[i];
        if (qtcIsSpace(c)) {
            if (!out.empty()) lastSpace = true;
        } else {
            if (lastSpace) { out += L' '; lastSpace = false; }
            out += c;
        }
    }
    return out;
}

QString QString::toLower() const {
    QString out(*this);
    for (int i = 0; i < out.size(); ++i) out[i] = (wchar_t)towlower(out[i]);
    return out;
}

QString QString::toUpper() const {
    QString out(*this);
    for (int i = 0; i < out.size(); ++i) out[i] = (wchar_t)towupper(out[i]);
    return out;
}

QString QString::left(int n) const {
    if (n < 0) return *this;
    if (n >= size()) return *this;
    return QString(c_str(), n);
}

QString QString::right(int n) const {
    if (n < 0) return *this;
    if (n >= size()) return *this;
    return QString(c_str() + (size() - n), n);
}

QString QString::mid(int pos, int len) const {
    if (pos < 0) pos = 0;
    if (pos >= size()) return QString();
    if (len < 0 || pos + len > size()) len = size() - pos;
    return QString(c_str() + pos, len);
}

int QString::indexOf(const QString& needle, int from) const {
    if (from < 0) from = 0;
    if (needle.empty()) return from <= size() ? from : -1;
    size_t p = find(needle, (size_t)from);
    return p == npos ? -1 : (int)p;
}

int QString::indexOf(wchar_t ch, int from) const {
    if (from < 0) from = 0;
    size_t p = find(ch, (size_t)from);
    return p == npos ? -1 : (int)p;
}

bool QString::startsWith(const QString& s) const {
    return size() >= s.size() && compare(0, s.size(), s) == 0;
}

bool QString::endsWith(const QString& s) const {
    return size() >= s.size() && compare(size() - s.size(), s.size(), s) == 0;
}

QString QString::replace(const QString& before, const QString& after) const {
    if (before.empty()) return *this;
    QString out;
    int pos = 0;
    for (;;) {
        int hit = indexOf(before, pos);
        if (hit < 0) break;
        out.append(mid(pos, hit - pos));
        out.append(after);
        pos = hit + before.size();
    }
    out.append(mid(pos));
    return out;
}

QString QString::replace(wchar_t before, wchar_t after) const {
    QString out(*this);
    for (int i = 0; i < out.size(); ++i) if (out[i] == before) out[i] = after;
    return out;
}

QString QString::repeated(int times) const {
    QString out;
    for (int i = 0; i < times; ++i) out.append(*this);
    return out;
}

static int qtcLowestPlaceholder(const QString& s) {
    for (int n = 1; n <= 9; ++n) {
        QString tag(L'%');
        tag += QString::number(n);
        if (s.indexOf(tag) >= 0) return n;
    }
    return -1;
}

static QString qtcArgReplace(const QString& self, const QString& value) {
    int n = qtcLowestPlaceholder(self);
    if (n < 0) return self;
    QString tag(L'%');
    tag += QString::number(n);
    return self.replace(tag, value);
}

QString QString::arg(const QString& a1) const { return qtcArgReplace(*this, a1); }

QString QString::arg(const QString& a1, const QString& a2) const {
    return arg(a1).arg(a2);
}

QString QString::arg(const QString& a1, const QString& a2, const QString& a3) const {
    return arg(a1).arg(a2).arg(a3);
}

QString QString::arg(int a, int fieldWidth, int base, wchar_t fillChar) const {
    QString num;
    if (base == 10) {
        num = QString::number(a);
    } else {
        bool neg = a < 0;
        unsigned int v = neg ? (unsigned int)(-(a + 1)) + 1u : (unsigned int)a;
        const wchar_t* digits = L"0123456789abcdef";
        QString d;
        if (v == 0) d = L"0";
        while (v) { d = QString(digits[v % (unsigned)base]) + d; v /= (unsigned)base; }
        num = neg ? (QString(L"-") + d) : d;
    }
    while (num.size() < fieldWidth) num = QString(fillChar) + num;
    return qtcArgReplace(*this, num);
}

QString QString::arg(qint64 a) const {
    wchar_t b[40];
    _snwprintf_s(b, 40, _TRUNCATE, L"%I64d", a);
    return qtcArgReplace(*this, QString(b));
}

QString QString::arg(double a, int fieldWidth, char format, int precision) const {
    wchar_t fmt[16];
    wchar_t b[128];
    if (precision < 0) {
        _snwprintf_s(fmt, 16, _TRUNCATE, L"%%%d%c", fieldWidth, (wchar_t)format);
        _snwprintf_s(b, 128, _TRUNCATE, fmt, a);
    } else {
        _snwprintf_s(fmt, 16, _TRUNCATE, L"%%%d.%d%c", fieldWidth, precision, (wchar_t)format);
        _snwprintf_s(b, 128, _TRUNCATE, fmt, a);
    }
    return qtcArgReplace(*this, QString(b));
}

QList<QString> QString::split(const QString& separator) const {
    QList<QString> parts;
    if (separator.empty()) { parts.append(*this); return parts; }
    int pos = 0;
    for (;;) {
        int hit = indexOf(separator, pos);
        if (hit < 0) { parts.append(mid(pos)); break; }
        parts.append(mid(pos, hit - pos));
        pos = hit + separator.size();
    }
    return parts;
}

QList<QString> QString::split(wchar_t separator) const {
    return split(QString(separator));
}

QString QStringList::join(const QString& separator) const {
    QString out;
    for (int i = 0; i < count(); ++i) {
        if (i) out.append(separator);
        out.append(at(i));
    }
    return out;
}

int QString::toInt(bool* ok, int base) const {
    if (empty()) { if (ok) *ok = false; return 0; }
    wchar_t* end = 0;
    long v = wcstol(c_str(), &end, base);
    bool good = end && *end == 0;
    while (end && *end == L' ') ++end;
    if (ok) *ok = good;
    return good ? (int)v : 0;
}

qint64 QString::toLongLong(bool* ok, int base) const {
    if (empty()) { if (ok) *ok = false; return 0; }
    wchar_t* end = 0;
    qint64 v = _wcstoi64(c_str(), &end, base);
    bool good = end && (*end == 0 || *end == L' ');
    if (ok) *ok = good;
    return good ? v : 0;
}

double QString::toDouble(bool* ok) const {
    if (empty()) { if (ok) *ok = false; return 0.0; }
    wchar_t* end = 0;
    double v = wcstod(c_str(), &end);
    bool good = end && (*end == 0 || *end == L' ');
    if (ok) *ok = good;
    return good ? v : 0.0;
}

bool QString::toBool() const {
    QString t = trimmed().toLower();
    return t == L"true" || t == L"1" || t == L"yes" || t == L"on";
}

QString QString::fromLatin1(const char* s) {
    if (!s) return QString();
    std::wstring w;
    size_t n = strlen(s);
    w.reserve(n);
    for (size_t i = 0; i < n; ++i) w += (wchar_t)(unsigned char)s[i];
    return QString(w);
}

QString QString::number(int n, int base) {
    if (base == 10) {
        wchar_t b[32];
        _snwprintf_s(b, 32, _TRUNCATE, L"%d", n);
        return QString(b);
    }
    bool neg = n < 0;
    unsigned int v = neg ? (unsigned int)(-(n + 1)) + 1u : (unsigned int)n;
    const wchar_t* digits = L"0123456789abcdef";
    QString d;
    if (v == 0) d = L"0";
    while (v) { d = QString(digits[v % (unsigned)base]) + d; v /= (unsigned)base; }
    return neg ? QString(L"-") + d : d;
}

QString QString::number(qint64 n, int base) {
    if (base == 10) {
        wchar_t b[40];
        _snwprintf_s(b, 40, _TRUNCATE, L"%I64d", n);
        return QString(b);
    }
    bool neg = n < 0;
    quint64 v = neg ? (quint64)(-(n + 1)) + 1 : (quint64)n;
    const wchar_t* digits = L"0123456789abcdef";
    QString d;
    if (v == 0) d = L"0";
    while (v) { d = QString(digits[v % (unsigned)base]) + d; v /= (unsigned)base; }
    return neg ? QString(L"-") + d : d;
}

QString QString::number(double n, char format, int precision) {
    wchar_t b[128];
    wchar_t f[16];
    _snwprintf_s(f, 16, _TRUNCATE, L"%%.%d%c", precision, (wchar_t)format);
    _snwprintf_s(b, 128, _TRUNCATE, f, n);
    return QString(b);
}

QString& QString::sprintf_like(const wchar_t* fmt) {
    if (!fmt) return *this;
    wchar_t b[1024];
    _snwprintf_s(b, 1024, _TRUNCATE, fmt);
    assign(b);
    return *this;
}
