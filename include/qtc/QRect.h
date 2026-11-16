// QtClone - QRect.h : QPoint / QPointF / QSize / QSizeF / QRect / QRectF / QMargins
#ifndef QTC_QRECT_H
#define QTC_QRECT_H

#include <qtc/QtGlobal.h>
#include <math.h>

class QPoint;
class QSize;
class QRect;
class QPointF;

// ---------------------------------------------------------------------------
// QPoint
// ---------------------------------------------------------------------------
class QPoint {
public:
    QPoint() : xp(0), yp(0) {}
    QPoint(int xpos, int ypos) : xp(xpos), yp(ypos) {}
    bool isNull() const { return xp == 0 && yp == 0; }
    int x() const { return xp; }
    int y() const { return yp; }
    void setX(int x) { xp = x; }
    void setY(int y) { yp = y; }
    int manhattanLength() const { return qAbs(xp) + qAbs(yp); }
    QPoint& operator+=(const QPoint& p) { xp += p.xp; yp += p.yp; return *this; }
    QPoint& operator-=(const QPoint& p) { xp -= p.xp; yp -= p.yp; return *this; }
    QPoint& operator*=(qreal f) { xp = qRound(xp * f); yp = qRound(yp * f); return *this; }
    int xp;
    int yp;
};

inline bool operator==(const QPoint& a, const QPoint& b) { return a.xp == b.xp && a.yp == b.yp; }
inline bool operator!=(const QPoint& a, const QPoint& b) { return !(a == b); }
inline const QPoint operator+(const QPoint& a, const QPoint& b) { return QPoint(a.xp + b.xp, a.yp + b.yp); }
inline const QPoint operator-(const QPoint& a, const QPoint& b) { return QPoint(a.xp - b.xp, a.yp - b.yp); }
inline const QPoint operator-(const QPoint& a) { return QPoint(-a.xp, -a.yp); }
inline const QPoint operator*(const QPoint& a, qreal f) { return QPoint(qRound(a.xp * f), qRound(a.yp * f)); }

// ---------------------------------------------------------------------------
// QSize
// ---------------------------------------------------------------------------
class QSize {
public:
    QSize() : wd(-1), ht(-1) {}
    QSize(int w, int h) : wd(w), ht(h) {}
    int width() const { return wd; }
    int height() const { return ht; }
    void setWidth(int w) { wd = w; }
    void setHeight(int h) { ht = h; }
    int& rwidth() { return wd; }
    int& rheight() { return ht; }
    bool isNull() const { return wd == 0 && ht == 0; }
    bool isEmpty() const { return wd <= 0 || ht <= 0; }
    bool isValid() const { return wd >= 0 && ht >= 0; }
    QSize transposed() const { return QSize(ht, wd); }
    QSize expandedTo(const QSize& other) const {
        return QSize(qMax(wd, other.wd), qMax(ht, other.ht));
    }
    QSize boundedTo(const QSize& other) const {
        return QSize(qMin(wd, other.wd), qMin(ht, other.ht));
    }
    QSize& operator+=(const QSize& s) { wd += s.wd; ht += s.ht; return *this; }
    QSize& operator-=(const QSize& s) { wd -= s.wd; ht -= s.ht; return *this; }
    int wd;
    int ht;
};

inline bool operator==(const QSize& a, const QSize& b) { return a.wd == b.wd && a.ht == b.ht; }
inline bool operator!=(const QSize& a, const QSize& b) { return !(a == b); }
inline const QSize operator+(const QSize& a, const QSize& b) { return QSize(a.wd + b.wd, a.ht + b.ht); }
inline const QSize operator-(const QSize& a, const QSize& b) { return QSize(a.wd - b.wd, a.ht - b.ht); }
inline const QSize operator*(const QSize& a, qreal f) { return QSize(qRound(a.wd * f), qRound(a.ht * f)); }

// ---------------------------------------------------------------------------
// QRect  (right()/bottom() are inclusive, as in Qt)
// ---------------------------------------------------------------------------
class QRect {
public:
    QRect() : x1(0), y1(0), x2(-1), y2(-1) {}
    QRect(const QPoint& topleft, const QPoint& bottomright)
        : x1(topleft.xp), y1(topleft.yp), x2(bottomright.xp), y2(bottomright.yp) {}
    QRect(const QPoint& topleft, const QSize& size)
        : x1(topleft.xp), y1(topleft.yp), x2(topleft.xp + size.wd - 1), y2(topleft.yp + size.ht - 1) {}
    QRect(int left, int top, int width, int height)
        : x1(left), y1(top), x2(left + width - 1), y2(top + height - 1) {}

    bool isNull() const { return x2 == x1 - 1 && y2 == y1 - 1; }
    bool isEmpty() const { return x1 > x2 || y1 > y2; }
    bool isValid() const { return x1 <= x2 && y1 <= y2; }

    int left() const { return x1; }
    int top() const { return y1; }
    int right() const { return x2; }
    int bottom() const { return y2; }
    int x() const { return x1; }
    int y() const { return y1; }
    void setLeft(int pos) { x1 = pos; }
    void setTop(int pos) { y1 = pos; }
    void setRight(int pos) { x2 = pos; }
    void setBottom(int pos) { y2 = pos; }
    void setX(int x) { x1 = x; }
    void setY(int y) { y1 = y; }
    void setTopLeft(const QPoint& p) { x1 = p.xp; y1 = p.yp; }
    void setBottomRight(const QPoint& p) { x2 = p.xp; y2 = p.yp; }
    void setTopRight(const QPoint& p) { x2 = p.xp; y1 = p.yp; }
    void setBottomLeft(const QPoint& p) { x1 = p.xp; y2 = p.yp; }
    QPoint topLeft() const { return QPoint(x1, y1); }
    QPoint topRight() const { return QPoint(x2, y1); }
    QPoint bottomLeft() const { return QPoint(x1, y2); }
    QPoint bottomRight() const { return QPoint(x2, y2); }
    QPoint center() const { return QPoint((x1 + x2) / 2, (y1 + y2) / 2); }

    void moveLeft(int pos) { x2 += (pos - x1); x1 = pos; }
    void moveTop(int pos) { y2 += (pos - y1); y1 = pos; }
    void moveRight(int pos) { x1 += (pos - x2); x2 = pos; }
    void moveBottom(int pos) { y1 += (pos - y2); y2 = pos; }
    void moveTopLeft(const QPoint& p) { moveLeft(p.xp); moveTop(p.yp); }
    void moveBottomRight(const QPoint& p) { moveRight(p.xp); moveBottom(p.yp); }
    void moveCenter(const QPoint& p) { int w = width(); int h = height(); x1 = p.xp - w / 2; y1 = p.yp - h / 2; x2 = x1 + w - 1; y2 = y1 + h - 1; }
    void moveTo(int x, int y) { moveTopLeft(QPoint(x, y)); }
    void moveTo(const QPoint& p) { moveTopLeft(p); }

    void translate(int dx, int dy) { x1 += dx; y1 += dy; x2 += dx; y2 += dy; }
    void translate(const QPoint& p) { translate(p.xp, p.yp); }
    QRect translated(int dx, int dy) const { return QRect(QPoint(x1 + dx, y1 + dy), QPoint(x2 + dx, y2 + dy)); }
    QRect translated(const QPoint& p) const { return translated(p.xp, p.yp); }

    void setRect(int x, int y, int w, int h) { x1 = x; y1 = y; x2 = x + w - 1; y2 = y + h - 1; }
    void setCoords(int x11, int y11, int x22, int y22) { x1 = x11; y1 = y11; x2 = x22; y2 = y22; }
    void setSize(const QSize& s) { x2 = x1 + s.wd - 1; y2 = y1 + s.ht - 1; }

    int width() const { return x2 - x1 + 1; }
    int height() const { return y2 - y1 + 1; }
    QSize size() const { return QSize(width(), height()); }
    void setWidth(int w) { x2 = x1 + w - 1; }
    void setHeight(int h) { y2 = y1 + h - 1; }

    void adjust(int dx1, int dy1, int dx2, int dy2) { x1 += dx1; y1 += dy1; x2 += dx2; y2 += dy2; }
    QRect adjusted(int dx1, int dy1, int dx2, int dy2) const {
        return QRect(QPoint(x1 + dx1, y1 + dy1), QPoint(x2 + dx2, y2 + dy2));
    }
    QRect inflated(int dx, int dy) const { return adjusted(-dx, -dy, dx, dy); }

    bool contains(const QPoint& p, bool proper = false) const {
        if (proper) return p.xp > x1 && p.xp < x2 && p.yp > y1 && p.yp < y2;
        return p.xp >= x1 && p.xp <= x2 && p.yp >= y1 && p.yp <= y2;
    }
    bool contains(int x, int y, bool proper = false) const { return contains(QPoint(x, y), proper); }
    bool contains(const QRect& r, bool proper = false) const {
        if (proper)
            return r.x1 > x1 && r.x2 < x2 && r.y1 > y1 && r.y2 < y2;
        return r.x1 >= x1 && r.x2 <= x2 && r.y1 >= y1 && r.y2 <= y2;
    }
    bool intersects(const QRect& r) const {
        return !(r.x1 > x2 || r.x2 < x1 || r.y1 > y2 || r.y2 < y1);
    }
    QRect intersected(const QRect& r) const {
        QRect out;
        if (!intersects(r)) return out;
        out.x1 = qMax(x1, r.x1);
        out.y1 = qMax(y1, r.y1);
        out.x2 = qMin(x2, r.x2);
        out.y2 = qMin(y2, r.y2);
        return out;
    }
    QRect united(const QRect& r) const {
        if (isNull()) return r;
        if (r.isNull()) return *this;
        QRect out;
        out.x1 = qMin(x1, r.x1);
        out.y1 = qMin(y1, r.y1);
        out.x2 = qMax(x2, r.x2);
        out.y2 = qMax(y2, r.y2);
        return out;
    }
    QRect normalized() const {
        QRect r(*this);
        if (r.x1 > r.x2) qSwap(r.x1, r.x2);
        if (r.y1 > r.y2) qSwap(r.y1, r.y2);
        return r;
    }

    int x1, y1, x2, y2;
};

inline bool operator==(const QRect& a, const QRect& b) {
    return a.x1 == b.x1 && a.y1 == b.y1 && a.x2 == b.x2 && a.y2 == b.y2;
}
inline bool operator!=(const QRect& a, const QRect& b) { return !(a == b); }

// ---------------------------------------------------------------------------
// QPointF / QSizeF / QRectF
// ---------------------------------------------------------------------------
class QPointF {
public:
    QPointF() : xp(0), yp(0) {}
    QPointF(qreal xpos, qreal ypos) : xp(xpos), yp(ypos) {}
    QPointF(const QPoint& p) : xp(p.xp), yp(p.yp) {}
    qreal x() const { return xp; }
    qreal y() const { return yp; }
    void setX(qreal v) { xp = v; }
    void setY(qreal v) { yp = v; }
    qreal xp, yp;
};

class QSizeF {
public:
    QSizeF() : wd(-1), ht(-1) {}
    QSizeF(qreal w, qreal h) : wd(w), ht(h) {}
    QSizeF(const QSize& s) : wd(s.wd), ht(s.ht) {}
    qreal width() const { return wd; }
    qreal height() const { return ht; }
    QSize toSize() const { return QSize(qRound(wd), qRound(ht)); }
    qreal wd, ht;
};

class QRectF {
public:
    QRectF() : x1(0), y1(0), x2(-1), y2(-1) {}
    QRectF(qreal left, qreal top, qreal width, qreal height)
        : x1(left), y1(top), x2(left + width), y2(top + height) {}
    QRectF(const QRect& r) : x1(r.x1), y1(r.y1), x2(r.x2 + 1.0), y2(r.y2 + 1.0) {}
    qreal left() const { return x1; }
    qreal top() const { return y1; }
    qreal right() const { return x2; }
    qreal bottom() const { return y2; }
    qreal width() const { return x2 - x1; }
    qreal height() const { return y2 - y1; }
    QRect toRect() const { return QRect(qRound(x1), qRound(y1), qRound(x2 - x1), qRound(y2 - y1)); }
    QRectF adjusted(qreal dx1, qreal dy1, qreal dx2, qreal dy2) const {
        return QRectF(x1 + dx1, y1 + dy1, (x2 - x1) + (dx2 - dx1), (y2 - y1) + (dy2 - dy1));
    }
    QRectF intersected(const QRectF& r) const {
        qreal l = qMax(x1, r.x1), t = qMax(y1, r.y1);
        qreal rr = qMin(x2, r.x2), b = qMin(y2, r.y2);
        if (l > rr || t > b) return QRectF();
        return QRectF(l, t, rr - l, b - t);
    }
    bool contains(qreal px, qreal py) const { return px >= x1 && px <= x2 && py >= y1 && py <= y2; }
    qreal x1, y1, x2, y2;
};

// ---------------------------------------------------------------------------
// QMargins
// ---------------------------------------------------------------------------
class QMargins {
public:
    QMargins() : l(0), t(0), r(0), b(0) {}
    QMargins(int left, int top, int right, int bottom) : l(left), t(top), r(right), b(bottom) {}
    int left() const { return l; }
    int top() const { return t; }
    int right() const { return r; }
    int bottom() const { return b; }
    void setLeft(int v) { l = v; }
    void setTop(int v) { t = v; }
    void setRight(int v) { r = v; }
    void setBottom(int v) { b = v; }
    bool isNull() const { return l == 0 && t == 0 && r == 0 && b == 0; }
    int l, t, r, b;
};

#endif // QTC_QRECT_H
