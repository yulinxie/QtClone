// QtClone - QString.h : UTF-16 (wchar_t) string value class.
#ifndef QTC_QSTRING_H
#define QTC_QSTRING_H

#include <qtc/QtGlobal.h>
#include <qtc/QList.h>
#include <string>

class QString;

typedef std::string QByteArray;

std::wstring qtcFromUtf8(const char* s, int len = -1);
std::string  qtcToUtf8(const std::wstring& s);

class QString : public std::wstring {
public:
    QString() {}
    QString(const wchar_t* s) : std::wstring(s ? s : L"") {}
    QString(const wchar_t* s, int len) : std::wstring(s ? s : L"", (size_t)(len < 0 ? 0 : len)) {}
    QString(const std::wstring& s) : std::wstring(s) {}
    QString(const QString& s) : std::wstring(s) {}
    QString(const char* utf8) : std::wstring(qtcFromUtf8(utf8)) {}
    QString(wchar_t c) : std::wstring(1, c) {}

    QString& operator=(const wchar_t* s) { assign(s ? s : L""); return *this; }
    QString& operator=(const QString& s) { if (this != &s) assign(s.c_str()); return *this; }
    QString& operator=(const std::wstring& s) { if (this != &s) assign(s); return *this; }
    QString& operator=(const char* utf8) { assign(qtcFromUtf8(utf8)); return *this; }

    QString& operator+=(const wchar_t* s) { append(s ? s : L""); return *this; }
    QString& operator+=(wchar_t c) { append(1, c); return *this; }
    QString& operator+=(const char* utf8) { append(qtcFromUtf8(utf8)); return *this; }
    QString& operator+=(const QString& s) { append(s); return *this; }
    QString& operator+=(const std::wstring& s) { append(s); return *this; }

    bool isEmpty() const { return empty(); }
    bool isNull() const { return empty(); }
    int size() const { return (int)std::wstring::size(); }
    int length() const { return (int)std::wstring::size(); }
    int count() const { return (int)std::wstring::size(); }
    const wchar_t* constData() const { return c_str(); }

    QString trimmed() const;
    QString simplified() const;
    QString toLower() const;
    QString toUpper() const;
    QString left(int n) const;
    QString right(int n) const;
    QString mid(int pos, int len = -1) const;
    int indexOf(const QString& needle, int from = 0) const;
    int indexOf(wchar_t ch, int from = 0) const;
    bool contains(const QString& needle) const { return indexOf(needle) >= 0; }
    bool contains(wchar_t ch) const { return indexOf(ch) >= 0; }
    bool startsWith(const QString& s) const;
    bool endsWith(const QString& s) const;
    QString replace(const QString& before, const QString& after) const;
    QString replace(wchar_t before, wchar_t after) const;
    QString repeated(int times) const;

    // "%1" .. "%9" substitution (lowest placeholder replaced per call)
    QString arg(const QString& a1) const;
    QString arg(const QString& a1, const QString& a2) const;
    QString arg(const QString& a1, const QString& a2, const QString& a3) const;
    QString arg(int a, int fieldWidth = 0, int base = 10, wchar_t fillChar = L' ') const;
    QString arg(qint64 a) const;
    QString arg(double a, int fieldWidth = 0, char format = 'g', int precision = -1) const;

    QList<QString> split(const QString& separator) const;
    QList<QString> split(wchar_t separator) const;

    int toInt(bool* ok = 0, int base = 10) const;
    qint64 toLongLong(bool* ok = 0, int base = 10) const;
    double toDouble(bool* ok = 0) const;
    bool toBool() const;

    std::string toUtf8() const { return qtcToUtf8(*this); }
    std::string toLocal8Bit() const { return qtcToUtf8(*this); }
    static QString fromUtf8(const char* s) { return QString(qtcFromUtf8(s)); }
    static QString fromLocal8Bit(const char* s) { return QString(qtcFromUtf8(s)); }
    static QString fromLatin1(const char* s);
    static QString fromWCharArray(const wchar_t* s) { return QString(s); }

    static QString number(int n, int base = 10);
    static QString number(qint64 n, int base = 10);
    static QString number(double n, char format = 'g', int precision = 6);

    QString& sprintf_like(const wchar_t* fmt);
};

class QStringList : public QList<QString> {
public:
    QStringList() {}
    QStringList(const QString& one) { append(one); }
    QStringList(const QList<QString>& other) : QList<QString>(other) {}
    QStringList& operator<<(const QString& s) { append(s); return *this; }
    QString join(const QString& separator) const;
    QString join(wchar_t separator) const { return join(QString(separator)); }
};

inline QString operator+(const QString& a, const QString& b) { QString r(a); r.append(b); return r; }
inline QString operator+(const QString& a, const wchar_t* b) { QString r(a); r.append(b ? b : L""); return r; }
inline QString operator+(const wchar_t* a, const QString& b) { QString r(a ? a : L""); r.append(b); return r; }
inline QString operator+(const QString& a, const char* b) { QString r(a); r.append(qtcFromUtf8(b)); return r; }
inline QString operator+(const char* a, const QString& b) { QString r(qtcFromUtf8(a)); r.append(b); return r; }

#endif // QTC_QSTRING_H
