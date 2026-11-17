// QtClone - QtGui.h : painting, colors, fonts, bitmaps.
#ifndef QTC_QTGUI_H
#define QTC_QTGUI_H

#include <qtc/QtGlobal.h>
#include <qtc/QRect.h>
#include <qtc/QString.h>
#include <qtc/QList.h>
#include <qtc/QVariant.h>

class QPainter;
class QPixmap;
class QImage;
struct QBitmapData;
struct QPainterPrivate;
class QWidget;

// ---------------------------------------------------------------------------
// QColor
// ---------------------------------------------------------------------------
class QColor {
public:
    enum NameFormat { HexRgb, HexArgb };

    QColor();
    QColor(Qt::GlobalColor color);
    QColor(int r, int g, int b, int a = 255);
    QColor(const QString& name);
    QColor(const char* name);
    QColor(const QColor& other) : rgba_(other.rgba_) {}

    QColor& operator=(const QColor& other) { rgba_ = other.rgba_; return *this; }

    bool isValid() const { return rgba_ != 0xFFFFFFFFu; }
    int red() const { return (int)((rgba_ >> 16) & 0xff); }
    int green() const { return (int)((rgba_ >> 8) & 0xff); }
    int blue() const { return (int)(rgba_ & 0xff); }
    int alpha() const { return (int)((rgba_ >> 24) & 0xff); }
    void setRed(int r);
    void setGreen(int g);
    void setBlue(int b);
    void setAlpha(int a);
    void setRgb(int r, int g, int b, int a = 255);
    void setRgba(int r, int g, int b, int a) { setRgb(r, g, b, a); }

    QString name() const { return name(HexRgb); }
    QString name(NameFormat format) const;

    QColor lighter(int factor = 150) const;
    QColor darker(int factor = 150) const;

    bool operator==(const QColor& c) const { return rgba_ == c.rgba_; }
    bool operator!=(const QColor& c) const { return rgba_ != c.rgba_; }

    static QColor fromRgb(int r, int g, int b) { return QColor(r, g, b); }
    static QColor fromRgba(int r, int g, int b, int a) { return QColor(r, g, b, a); }
    static QColor fromName(const QString& name) { return QColor(name); }
    static bool isValidColor(const QString& name);

    quint32 rgba() const { return rgba_; }
    void setRgbaValue(quint32 v) { rgba_ = v; }

private:
    quint32 rgba_;
};

// ---------------------------------------------------------------------------
// QFont
// ---------------------------------------------------------------------------
class QFont {
public:
    QFont();
    QFont(const QString& family, int pointSize = -1, int weight = -1, bool italic = false);
    QFont(const QFont& other);

    QString family() const { return family_; }
    void setFamily(const QString& f) { family_ = f; }
    int pointSize() const { return pointSize_; }
    void setPointSize(int p) { pointSize_ = p; pixelSize_ = -1; }
    int pixelSize() const { return pixelSize_; }
    void setPixelSize(int p) { pixelSize_ = p; }
    int weight() const { return weight_; }
    void setWeight(int w) { weight_ = w; }
    bool bold() const { return weight_ >= 75; }
    void setBold(bool b) { weight_ = b ? 75 : 50; }
    bool italic() const { return italic_; }
    void setItalic(bool i) { italic_ = i; }
    bool underline() const { return underline_; }
    void setUnderline(bool u) { underline_ = u; }
    bool strikeOut() const { return strikeOut_; }
    void setStrikeOut(bool s) { strikeOut_ = s; }

    // effective size in device pixels (point sizes assume 96 dpi)
    int pixelHeight() const;
    bool operator==(const QFont& f) const;
    bool operator!=(const QFont& f) const { return !(*this == f); }

private:
    QString family_;
    int pointSize_;
    int pixelSize_;
    int weight_;
    bool italic_;
    bool underline_;
    bool strikeOut_;
};

// ---------------------------------------------------------------------------
// QPen / QBrush
// ---------------------------------------------------------------------------
class QPen {
public:
    QPen();
    QPen(Qt::PenStyle style);
    explicit QPen(const QColor& color);
    QPen(const QColor& color, qreal width, Qt::PenStyle style = Qt::SolidLine,
         Qt::PenCapStyle cap = Qt::SquareCap, Qt::PenJoinStyle join = Qt::BevelJoin);

    Qt::PenStyle style() const { return style_; }
    void setStyle(Qt::PenStyle s) { style_ = s; }
    qreal widthF() const { return width_; }
    int width() const { return qRound(width_); }
    void setWidth(int w) { width_ = w; }
    void setWidthF(qreal w) { width_ = w; }
    QColor color() const { return color_; }
    void setColor(const QColor& c) { color_ = c; }
    Qt::PenCapStyle capStyle() const { return cap_; }
    void setCapStyle(Qt::PenCapStyle c) { cap_ = c; }
    Qt::PenJoinStyle joinStyle() const { return join_; }
    void setJoinStyle(Qt::PenJoinStyle j) { join_ = j; }
    bool operator==(const QPen& p) const;
    bool operator!=(const QPen& p) const { return !(*this == p); }

private:
    Qt::PenStyle style_;
    qreal width_;
    QColor color_;
    Qt::PenCapStyle cap_;
    Qt::PenJoinStyle join_;
};

class QBrush {
public:
    QBrush();
    QBrush(Qt::BrushStyle style);
    QBrush(const QColor& color, Qt::BrushStyle style = Qt::SolidPattern);
    QColor color() const { return color_; }
    void setColor(const QColor& c) { color_ = c; }
    Qt::BrushStyle style() const { return style_; }
    void setStyle(Qt::BrushStyle s) { style_ = s; }
    bool isOpaque() const { return color_.alpha() == 255 && style_ != Qt::NoBrush; }
    bool operator==(const QBrush& b) const { return style_ == b.style_ && color_ == b.color_; }

private:
    QColor color_;
    Qt::BrushStyle style_;
};

// ---------------------------------------------------------------------------
// QRegion
// ---------------------------------------------------------------------------
class QRegion {
public:
    QRegion();
    QRegion(int x, int y, int w, int h);
    explicit QRegion(const QRect& r);

    bool isEmpty() const;
    bool contains(const QPoint& p) const;
    QRect boundingRect() const;
    QList<QRect> rects() const { return rects_; }
    int rectCount() const { return rects_.count(); }

    void setRects(const QRect& r) { rects_.clearAll(); if (!r.isEmpty()) rects_.append(r); }
    void unite(const QRect& r);
    void unite(const QRegion& r);
    void intersect(const QRect& r);
    void intersect(const QRegion& r);
    void translate(int dx, int dy);
    QRegion translated(int dx, int dy) const;
    bool operator==(const QRegion& r) const;
    bool operator!=(const QRegion& r) const { return !(*this == r); }

private:
    QList<QRect> rects_;
};

// ---------------------------------------------------------------------------
// QPixmap / QImage / QIcon  (shared, refcounted GDI+ bitmap)
// ---------------------------------------------------------------------------
struct QBitmapData {
    void* bmp;          // Gdiplus::Bitmap*
    int refs;
    QSize sz;
};

class QPixmap {
public:
    QPixmap();
    QPixmap(int w, int h);
    QPixmap(const QSize& size);
    QPixmap(const QPixmap& other);
    explicit QPixmap(const QImage& image);
    ~QPixmap();

    QPixmap& operator=(const QPixmap& other);
    bool isNull() const { return data_ == 0; }
    int width() const { return data_ ? data_->sz.width() : 0; }
    int height() const { return data_ ? data_->sz.height() : 0; }
    QSize size() const { return data_ ? data_->sz : QSize(); }
    QRect rect() const { return QRect(0, 0, width(), height()); }

    bool load(const QString& fileName, const char* format = 0);
    bool loadFromData(const std::string& data);
    bool save(const QString& fileName, const char* format = 0) const;
    void fill(const QColor& color);
    QPixmap scaled(const QSize& s, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio,
                   Qt::TransformationMode transform = Qt::FastTransformation) const;
    QImage toImage() const;
    static QPixmap fromImage(const QImage& image);

    QBitmapData* data() const { return data_; }
    friend QPixmap qtcPixmapFromData(QBitmapData* data);
    void* bitmap() const { return data_ ? data_->bmp : 0; }
    static QPixmap fromBitmap(void* gdiplusBitmap);   // takes ownership

private:
    QBitmapData* data_;
};

class QImage {
public:
    QImage();
    QImage(int w, int h);
    QImage(const QSize& size);
    QImage(const QImage& other);
    explicit QImage(const QPixmap& pixmap);
    ~QImage();

    QImage& operator=(const QImage& other);
    bool isNull() const { return data_ == 0; }
    int width() const { return data_ ? data_->sz.width() : 0; }
    int height() const { return data_ ? data_->sz.height() : 0; }
    QSize size() const { return data_ ? data_->sz : QSize(); }
    QRect rect() const { return QRect(0, 0, width(), height()); }
    void fill(const QColor& color);
    bool load(const QString& fileName);
    bool save(const QString& fileName, const char* format = 0) const;
    QPixmap toPixmap() const { return QPixmap(*this); }
    QBitmapData* data() const { return data_; }
    friend QPixmap qtcPixmapFromData(QBitmapData* data);

private:
    QBitmapData* data_;
};

class QIcon {
public:
    QIcon();
    explicit QIcon(const QString& fileName);
    QIcon(const QPixmap& pixmap);
    bool isNull() const { return pix_.isNull(); }
    QPixmap pixmap(const QSize& size = QSize(), int mode = 0, int state = 0) const;
    QSize actualSize(const QSize& size) const;
    void addPixmap(const QPixmap& pixmap) { pix_ = pixmap; }

private:
    QPixmap pix_;
};

class QCursor {
public:
    QCursor();
    explicit QCursor(Qt::CursorShape shape);
    Qt::CursorShape shape() const { return shape_; }
    void setShape(Qt::CursorShape s) { shape_ = s; }
    static QPoint pos();
    static void setPos(int x, int y);
    static void setPos(const QPoint& p) { setPos(p.x(), p.y()); }
    void* handle() const;
private:
    Qt::CursorShape shape_;
};

// ---------------------------------------------------------------------------
// QPalette
// ---------------------------------------------------------------------------
class QPalette {
public:
    enum ColorRole {
        WindowText, Button, Light, Midlight, Dark, Mid, Text, BrightText,
        ButtonText, Base, Window, Shadow, Highlight, HighlightedText,
        Link, LinkVisited, AlternateBase, NoRole, ToolTipBase, ToolTipText,
        PlaceholderText, NColorRoles
    };
    enum ColorGroup { Active, Disabled, Inactive, NColorGroups };

    QPalette();
    QColor color(ColorRole role) const { return color(Active, role); }
    QColor color(ColorGroup group, ColorRole role) const;
    void setColor(ColorRole role, const QColor& c);
    void setColor(ColorGroup group, ColorRole role, const QColor& c);
    void setBrush(ColorRole role, const QBrush& b);

private:
    QColor colors_[NColorGroups][NColorRoles];
};

// ---------------------------------------------------------------------------
// QFontMetrics
// ---------------------------------------------------------------------------
class QFontMetrics {
public:
    explicit QFontMetrics(const QFont& font);
    int width(const QString& text) const;
    int width(wchar_t ch) const;
    int height() const;
    int ascent() const;
    int descent() const;
    int lineSpacing() const;
    QRect boundingRect(const QString& text) const;
    QRect boundingRect(wchar_t ch) const;
    int horizontalAdvance(const QString& text) const { return width(text); }
    const QFont& font() const { return font_; }
private:
    QFont font_;
};

// ---------------------------------------------------------------------------
// QPainter
// ---------------------------------------------------------------------------
class QPainter {
public:
    enum RenderHint {
        Antialiasing = 0x01,
        TextAntialiasing = 0x02,
        SmoothPixmapTransform = 0x04
    };

    QPainter();
    explicit QPainter(QPixmap* pixmap);
    explicit QPainter(QWidget* widget);
    explicit QPainter(QImage* image);
    ~QPainter();

    bool begin(QPixmap* pixmap);
    bool begin(QImage* image);
    bool end();
    bool isActive() const;

    void save();
    void restore();

    void setPen(const QPen& pen);
    void setPen(const QColor& color);
    void setPen(Qt::PenStyle style);
    const QPen& pen() const;

    void setBrush(const QBrush& brush);
    void setBrush(const QColor& color);
    const QBrush& brush() const;

    void setFont(const QFont& font);
    const QFont& font() const;

    void setRenderHint(RenderHint hint, bool on = true);
    void setBackground(const QBrush& brush);
    const QBrush& background() const;

    void setClipRect(const QRect& rect);
    void setClipRegion(const QRegion& region);
    void setClipping(bool enable);
    bool hasClipping() const;
    QRect clipBoundingRect() const;

    void translate(qreal dx, qreal dy);
    void scale(qreal sx, qreal sy);
    void resetTransform();

    void fillRect(const QRect& rect, const QColor& color);
    void fillRect(const QRect& rect, const QBrush& brush);
    void fillRect(int x, int y, int w, int h, const QColor& color);
    void eraseRect(const QRect& rect);

    void drawRect(const QRect& rect);
    void drawRoundedRect(const QRect& rect, qreal xRadius, qreal yRadius,
                         Qt::SizeMode mode = Qt::AbsoluteSize);
    void drawPoint(const QPoint& p);
    void drawPoint(int x, int y);
    void drawLine(const QPoint& p1, const QPoint& p2);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawEllipse(const QRect& rect);
    void drawEllipse(const QPoint& center, int rx, int ry);
    void drawPolyline(const QList<QPoint>& points);
    void drawPolygon(const QList<QPoint>& points, Qt::FillRule rule = Qt::OddEvenFill);
    void drawArc(const QRect& rect, int startAngle, int spanAngle);

    void drawText(const QPoint& pos, const QString& text);
    void drawText(int x, int y, const QString& text);
    void drawText(const QRect& rect, int flags, const QString& text);
    QRect boundingRect(const QRect& rect, int flags, const QString& text) const;

    void drawPixmap(const QPoint& pos, const QPixmap& pixmap);
    void drawPixmap(const QRect& target, const QPixmap& pixmap);
    void drawPixmap(const QRect& target, const QPixmap& pixmap, const QRect& source);
    void drawImage(const QPoint& pos, const QImage& image);
    void drawImage(const QRect& target, const QImage& image);
    void drawImage(const QRect& target, const QImage& image, const QRect& source);

    QFontMetrics fontMetrics() const { return QFontMetrics(font()); }

    // internal: direct access for the widget/compositor layer
    QPainterPrivate* d_func() const { return d_; }
    void* graphics() const;
    QBitmapData* targetData() const;
    QPoint paintOrigin() const;

private:
    QPainterPrivate* d_;
    QPainter(const QPainter&);
    QPainter& operator=(const QPainter&);
};

// helpers used across the framework
void qtcEnsureGraphics();
void* qtcGdiplusFontFor(const QFont& font);          // returns Gdiplus::Font*
QString qtcStripMnemonics(const QString& text);
int qtcTextWidth(const QFont& font, const QString& text);
QPixmap qtcCreateTransparentPixmap(int w, int h);
QPixmap qtcPixmapFromData(QBitmapData* data);

// Per-thread paint target used by QPainter(QWidget*) so that nested widget
// painting shares the Graphics of the painter currently rendering the tree.
void qtcPushPaintTarget(void* graphics, const QPoint& origin, const QRect& clip);
void qtcPopPaintTarget();
bool qtcCurrentPaintTarget(void** graphics, QPoint* origin);

// ---------------------------------------------------------------------------
// QVariant support for GUI value types
// ---------------------------------------------------------------------------
Q_DECLARE_METATYPE(QColor)
Q_DECLARE_METATYPE(QFont)
Q_DECLARE_METATYPE(QPoint)
Q_DECLARE_METATYPE(QSize)
Q_DECLARE_METATYPE(QRect)

template <> struct QVariantValue<QColor> {
    static QColor get(const QVariant& v) { return v.toColor(); }
};
template <> struct QVariantValue<QFont> {
    static QFont get(const QVariant& v) { return v.toFont(); }
};
template <> struct QVariantValue<QPoint> {
    static QPoint get(const QVariant& v) { return v.toPoint(); }
};
template <> struct QVariantValue<QSize> {
    static QSize get(const QVariant& v) { return v.toSize(); }
};
template <> struct QVariantValue<QRect> {
    static QRect get(const QVariant& v) { return v.toRect(); }
};

#endif // QTC_QTGUI_H
