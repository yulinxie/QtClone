// QtClone - QtList.h : QList / QVector / QMap style containers built on the STL.
#ifndef QTC_QLIST_H
#define QTC_QLIST_H

#include <qtc/QtGlobal.h>
#include <vector>
#include <map>

// QList<T> is a thin value-semantics wrapper over std::vector<T> with Qt's
// method names (append/at/count/...).  It is used everywhere in the public API
// so that client code reads like Qt code.
template <class T>
class QList : public std::vector<T> {
public:
    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;

    QList() {}
    QList(const std::vector<T>& other) : std::vector<T>(other) {}

    void append(const T& t) { this->push_back(t); }
    void append(const QList<T>& other) { for (int i = 0; i < other.count(); ++i) this->push_back(other[i]); }
    void prepend(const T& t) { std::vector<T>::insert(this->begin(), t); }
    void push_back(const T& t) { std::vector<T>::push_back(t); }

    int count() const { return (int)std::vector<T>::size(); }
    int size() const { return (int)std::vector<T>::size(); }
    bool isEmpty() const { return std::vector<T>::empty(); }

    T& at(int i) { return (*this)[i]; }
    const T& at(int i) const { return (*this)[i]; }
    T& operator[](int i) { return std::vector<T>::operator[]((size_t)i); }
    const T& operator[](int i) const { return std::vector<T>::operator[]((size_t)i); }

    T& first() { return this->front(); }
    const T& first() const { return this->front(); }
    T& last() { return this->back(); }
    const T& last() const { return this->back(); }

    bool contains(const T& t) const { return indexOf(t) >= 0; }
    int indexOf(const T& t, int from = 0) const {
        for (int i = qMax(from, 0); i < count(); ++i)
            if ((*this)[i] == t) return i;
        return -1;
    }
    void removeAt(int i) { this->erase(this->begin() + i); }
    void removeAll(const T& t) {
        for (int i = count() - 1; i >= 0; --i)
            if ((*this)[i] == t) removeAt(i);
    }
    T takeAt(int i) { T t = at(i); removeAt(i); return t; }
    T takeFirst() { return takeAt(0); }
    T takeLast() { return takeAt(count() - 1); }
    void insert(int i, const T& t) { std::vector<T>::insert(this->begin() + i, t); }
    void replace(int i, const T& t) { (*this)[i] = t; }
    void clearAll() { std::vector<T>::clear(); }
    QList<T>& operator<<(const T& t) { append(t); return *this; }
    QList<T>& operator+=(const T& t) { append(t); return *this; }
};

// QVector is an alias of QList in this implementation.
template <class T> class QVector : public QList<T> {};

#endif // QTC_QLIST_H
