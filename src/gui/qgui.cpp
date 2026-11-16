// QtClone - qgui.cpp : QColor/QFont/QPen/QBrush/QRegion/QPalette/QPixmap/QImage.
#include <qtc/QtGui.h>

#include "qtc_gdiplus.h"
#include <stdio.h>
#include <math.h>
#include <string>
#include <map>
#include <vector>

using namespace Gdiplus;

namespace {

ULONG_PTR& gdiplusToken() {
    static ULONG_PTR token = 0;
    static bool started = false;
    if (!started) {
        GdiplusStartupInput input;
        GdiplusStartup(&token, &input, 0);
        started = true;
    }
    return token;
}

void ensureGdiplus() { (void)gdiplusToken(); }

struct ColorName {
    const wchar_t* name;
    quint32 rgb;
};

const ColorName kColorNames[] = {
    { L"black", 0x000000 }, { L"white", 0xFFFFFF }, { L"red", 0xFF0000 },
    { L"green", 0x008000 }, { L"blue", 0x0000FF }, { L"gray", 0x808080 },
    { L"grey", 0x808080 }, { L"darkgray", 0xA9A9A9 }, { L"lightgray", 0xD3D3D3 },
    { L"yellow", 0xFFFF00 }, { L"cyan", 0x00FFFF }, { L"magenta", 0xFF00FF },
    { L"transparent", 0xFFFFFF }, { L"orange", 0xFFA500 }, { L"purple", 0x800080 },
    { L"navy", 0x000080 }, { L"teal", 0x008080 }, { L"silver", 0xC0C0C0 },
    { L"maroon", 0x800000 }, { L"olive", 0x808000 }, { L"lime", 0x00FF00 },
    { L"gold", 0xFFD700 }, { L"pink", 0xFFC0CB }, { L"brown", 0xA52A2A },
    { L"darkblue", 0x00008B }, { L"darkgreen", 0x006400 }, { L"darkred", 0x8B0000 },
    { L"lightblue", 0xADD8E6 }, { L"lightgreen", 0x90EE90 }, { L"steelblue", 0x4682B4 },
    { L"royalblue", 0x4169E1 }, { L"dodgerblue", 0x1E90FF }, { L"tomato", 0xFF6347 },
    { L"crimson", 0xDC143C }, { L"indigo", 0x4B0082 }, { L"violet", 0xEE82EE },
    { L"beige", 0xF5F5DC }, { L"ivory", 0xFFFFF0 }, { L"khaki", 0xF0E68C },
    { L"salmon", 0xFA8072 }, { L"tan", 0xD2B48C }, { L"turquoise", 0x40E0D0 },
    { 0, 0 }
};

int hexDigit(wchar_t c) {
    if (c >= L'0' && c <= L'9') return c - L'0';
    if (c >= L'a' && c <= L'f') return c - L'a' + 10;
    if (c >= L'A' && c <= L'F') return c - L'A' + 10;
    return -1;
}

} // namespace

// ---------------------------------------------------------------------------
// QColor
// ---------------------------------------------------------------------------
QColor::QColor() : rgba_(0xFF000000u) {}

QColor::QColor(Qt::GlobalColor color) : rgba_(0xFF000000u) {
    switch (color) {
    case Qt::color0: rgba_ = 0x00000000; break;
    case Qt::color1: rgba_ = 0xFF000000; break;
    case Qt::white: rgba_ = 0xFFFFFFFF; break;
    case Qt::black: rgba_ = 0xFF000000; break;
    case Qt::red: rgba_ = 0xFFFF0000; break;
    case Qt::darkRed: rgba_ = 0xFF8B0000; break;
    case Qt::green: rgba_ = 0xFF00FF00; break;
    case Qt::darkGreen: rgba_ = 0xFF008000; break;
    case Qt::blue: rgba_ = 0xFF0000FF; break;
    case Qt::darkBlue: rgba_ = 0xFF00008B; break;
    case Qt::cyan: rgba_ = 0xFF00FFFF; break;
    case Qt::darkCyan: rgba_ = 0xFF008B8B; break;
    case Qt::magenta: rgba_ = 0xFFFF00FF; break;
    case Qt::darkMagenta: rgba_ = 0xFF8B008B; break;
    case Qt::yellow: rgba_ = 0xFFFFFF00; break;
    case Qt::darkYellow: rgba_ = 0xFF808000; break;
    case Qt::gray: rgba_ = 0xFF808080; break;
    case Qt::darkGray: rgba_ = 0xFFA9A9A9; break;
    case Qt::lightGray: rgba_ = 0xFFD3D3D3; break;
    case Qt::transparent: rgba_ = 0x00000000; break;
    default: rgba_ = 0xFF000000; break;
    }
}

QColor::QColor(int r, int g, int b, int a) : rgba_(0xFF000000u) { setRgb(r, g, b, a); }

QColor::QColor(const QString& name) : rgba_(0xFF000000u) {
    QString s = name.trimmed().toLower();
    if (s.isEmpty()) { rgba_ = 0xFF000000u; return; }
    if (s[0] == L'#') {
        QString hex = s.mid(1);
        quint32 value = 0;
        for (int i = 0; i < hex.size(); ++i) {
            int d = hexDigit(hex[i]);
            if (d < 0) { rgba_ = 0xFF000000u; return; }
            value = (value << 4) | (quint32)d;
        }
        if (hex.size() == 3) {
            int r = (int)((value >> 8) & 0xf), g = (int)((value >> 4) & 0xf), b = (int)(value & 0xf);
            setRgb(r * 17, g * 17, b * 17);
        } else if (hex.size() == 6) {
            setRgb((int)((value >> 16) & 0xff), (int)((value >> 8) & 0xff), (int)(value & 0xff));
        } else if (hex.size() == 8) {
            setRgb((int)((value >> 16) & 0xff), (int)((value >> 8) & 0xff), (int)(value & 0xff),
                   (int)((value >> 24) & 0xff));
        } else {
            rgba_ = 0xFF000000u;
        }
        return;
    }
    // rgb(r,g,b) / rgba(...)
    if (s.startsWith(L"rgb")) {
        int open = s.indexOf(L'(');
        int close = s.indexOf(L')');
        if (open >= 0 && close > open) {
            QList<QString> parts = s.mid(open + 1, close - open - 1).split(L',');
            if (parts.count() >= 3) {
                int a = parts.count() >= 4 ? parts[3].trimmed().toInt() : 255;
                setRgb(parts[0].trimmed().toInt(), parts[1].trimmed().toInt(), parts[2].trimmed().toInt(), a);
                return;
            }
        }
    }
    for (int i = 0; kColorNames[i].name; ++i) {
        if (s == QString(kColorNames[i].name)) {
            quint32 rgb = kColorNames[i].rgb;
            setRgb((int)((rgb >> 16) & 0xff), (int)((rgb >> 8) & 0xff), (int)(rgb & 0xff),
                   s == QString(L"transparent") ? 0 : 255);
            return;
        }
    }
    rgba_ = 0xFF000000u;
}

QColor::QColor(const char* name) : rgba_(0xFF000000u) {
    *this = QColor(QString(name));
}

void QColor::setRed(int r) { setRgb(r, green(), blue(), alpha()); }
void QColor::setGreen(int g) { setRgb(red(), g, blue(), alpha()); }
void QColor::setBlue(int b) { setRgb(red(), green(), b, alpha()); }
void QColor::setAlpha(int a) { setRgb(red(), green(), blue(), a); }

void QColor::setRgb(int r, int g, int b, int a) {
    r = qBound(0, r, 255);
    g = qBound(0, g, 255);
    b = qBound(0, b, 255);
    a = qBound(0, a, 255);
    rgba_ = ((quint32)a << 24) | ((quint32)r << 16) | ((quint32)g << 8) | (quint32)b;
}

QString QColor::name(NameFormat format) const {
    if (format == HexArgb) {
        wchar_t buf[16];
        _snwprintf_s(buf, 16, _TRUNCATE, L"#%02x%02x%02x%02x", alpha(), red(), green(), blue());
        return QString(buf);
    }
    wchar_t buf[16];
    _snwprintf_s(buf, 16, _TRUNCATE, L"#%02x%02x%02x", red(), green(), blue());
    return QString(buf);
}

QColor QColor::lighter(int factor) const {
    if (factor <= 0) return *this;
    int r = qMin(255, int(red() * factor / 100.0));
    int g = qMin(255, int(green() * factor / 100.0));
    int b = qMin(255, int(blue() * factor / 100.0));
    return QColor(r, g, b, alpha());
}

QColor QColor::darker(int factor) const {
    if (factor <= 0) return *this;
    int r = qMax(0, int(red() * 100.0 / factor));
    int g = qMax(0, int(green() * 100.0 / factor));
    int b = qMax(0, int(blue() * 100.0 / factor));
    return QColor(r, g, b, alpha());
}

bool QColor::isValidColor(const QString& name) {
    QString s = name.trimmed().toLower();
    if (s.startsWith(L"#")) return s.size() == 4 || s.size() == 7 || s.size() == 9;
    for (int i = 0; kColorNames[i].name; ++i) if (s == QString(kColorNames[i].name)) return true;
    return false;
}

// ---------------------------------------------------------------------------
// QFont
// ---------------------------------------------------------------------------
QFont::QFont()
    : family_(L"Microsoft YaHei UI"), pointSize_(9), pixelSize_(-1), weight_(50),
      italic_(false), underline_(false), strikeOut_(false) {}

QFont::QFont(const QString& family, int pointSize, int weight, bool italic)
    : family_(family), pointSize_(pointSize), pixelSize_(-1),
      weight_(weight < 0 ? 50 : weight), italic_(italic), underline_(false), strikeOut_(false) {}

QFont::QFont(const QFont& other)
    : family_(other.family_), pointSize_(other.pointSize_), pixelSize_(other.pixelSize_),
      weight_(other.weight_), italic_(other.italic_), underline_(other.underline_),
      strikeOut_(other.strikeOut_) {}

int QFont::pixelHeight() const {
    if (pixelSize_ > 0) return pixelSize_;
    if (pointSize_ > 0) return qMax(1, qRound(pointSize_ * 96.0 / 72.0));
    return 12;
}

bool QFont::operator==(const QFont& f) const {
    return family_ == f.family_ && pointSize_ == f.pointSize_ && pixelSize_ == f.pixelSize_ &&
           weight_ == f.weight_ && italic_ == f.italic_ && underline_ == f.underline_ &&
           strikeOut_ == f.strikeOut_;
}

// ---------------------------------------------------------------------------
// QPen / QBrush
// ---------------------------------------------------------------------------
QPen::QPen()
    : style_(Qt::SolidLine), width_(1), color_(Qt::black),
      cap_(Qt::SquareCap), join_(Qt::BevelJoin) {}

QPen::QPen(Qt::PenStyle style)
    : style_(style), width_(1), color_(Qt::black), cap_(Qt::SquareCap), join_(Qt::BevelJoin) {}

QPen::QPen(const QColor& color)
    : style_(Qt::SolidLine), width_(1), color_(color), cap_(Qt::SquareCap), join_(Qt::BevelJoin) {}

QPen::QPen(const QColor& color, qreal width, Qt::PenStyle style, Qt::PenCapStyle cap, Qt::PenJoinStyle join)
    : style_(style), width_(width), color_(color), cap_(cap), join_(join) {}

bool QPen::operator==(const QPen& p) const {
    return style_ == p.style_ && width_ == p.width_ && color_ == p.color_ &&
           cap_ == p.cap_ && join_ == p.join_;
}

QBrush::QBrush() : color_(Qt::black) { style_ = Qt::NoBrush; }
QBrush::QBrush(Qt::BrushStyle style) : color_(Qt::black), style_(style) {}
QBrush::QBrush(const QColor& color, Qt::BrushStyle style) : color_(color), style_(style) {}

// ---------------------------------------------------------------------------
// QRegion
// ---------------------------------------------------------------------------
QRegion::QRegion() {}
QRegion::QRegion(int x, int y, int w, int h) { rects_.append(QRect(x, y, w, h)); }
QRegion::QRegion(const QRect& r) { if (!r.isEmpty()) rects_.append(r); }

bool QRegion::isEmpty() const { return rects_.isEmpty(); }

bool QRegion::contains(const QPoint& p) const {
    for (int i = 0; i < rects_.count(); ++i) if (rects_[i].contains(p)) return true;
    return false;
}

QRect QRegion::boundingRect() const {
    QRect r;
    for (int i = 0; i < rects_.count(); ++i) r = r.united(rects_[i]);
    return r;
}

void QRegion::unite(const QRect& r) {
    if (r.isEmpty()) return;
    rects_.append(r);
    if (rects_.count() > 24) {
        // collapse into the bounding rect to keep painting cheap
        QRect b = boundingRect();
        rects_.clearAll();
        rects_.append(b);
    }
}

void QRegion::unite(const QRegion& r) {
    for (int i = 0; i < r.rectCount(); ++i) unite(r.rects()[i]);
}

void QRegion::intersect(const QRect& r) {
    QList<QRect> out;
    for (int i = 0; i < rects_.count(); ++i) {
        QRect x = rects_[i].intersected(r);
        if (!x.isEmpty()) out.append(x);
    }
    rects_ = out;
}

void QRegion::intersect(const QRegion& r) {
    QList<QRect> out;
    for (int i = 0; i < rects_.count(); ++i) {
        for (int j = 0; j < r.rectCount(); ++j) {
            QRect x = rects_[i].intersected(r.rects()[j]);
            if (!x.isEmpty()) out.append(x);
        }
    }
    rects_ = out;
}

void QRegion::translate(int dx, int dy) {
    for (int i = 0; i < rects_.count(); ++i) rects_[i].translate(dx, dy);
}

QRegion QRegion::translated(int dx, int dy) const {
    QRegion r(*this);
    r.translate(dx, dy);
    return r;
}

bool QRegion::operator==(const QRegion& r) const {
    if (rects_.count() != r.rects_.count()) return false;
    for (int i = 0; i < rects_.count(); ++i) if (!(rects_[i] == r.rects_[i])) return false;
    return true;
}

// ---------------------------------------------------------------------------
// QBitmapData helpers
// ---------------------------------------------------------------------------
namespace {

QBitmapData* createData(int w, int h) {
    ensureGdiplus();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
    Bitmap* bmp = new Bitmap(w, h, PixelFormat32bppPARGB);
    Graphics g(bmp);
    g.Clear(Color(0, 0, 0, 0));
    QBitmapData* d = new QBitmapData();
    d->bmp = bmp;
    d->refs = 1;
    d->sz = QSize(w, h);
    return d;
}

void addRef(QBitmapData* d) { if (d) ++d->refs; }
void release(QBitmapData* d) {
    if (!d) return;
    if (--d->refs == 0) {
        delete (Bitmap*)d->bmp;
        delete d;
    }
}

int encoderClsidForSuffix(const wchar_t* suffix, CLSID* clsid) {
    UINT num = 0, size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;
    ImageCodecInfo* info = (ImageCodecInfo*)malloc(size);
    if (!info) return -1;
    GetImageEncoders(num, size, info);
    int found = -1;
    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(info[i].MimeType, suffix) == 0) { *clsid = info[i].Clsid; found = (int)i; break; }
    }
    free(info);
    return found;
}

const wchar_t* mimeForFile(const QString& file) {
    QString f = file.toLower();
    if (f.endsWith(L".jpg") || f.endsWith(L".jpeg")) return L"image/jpeg";
    if (f.endsWith(L".bmp")) return L"image/bmp";
    if (f.endsWith(L".gif")) return L"image/gif";
    if (f.endsWith(L".tif") || f.endsWith(L".tiff")) return L"image/tiff";
    return L"image/png";
}

Bitmap* loadBitmap(const QString& file) {
    ensureGdiplus();
    Bitmap* src = Bitmap::FromFile(file.c_str(), FALSE);
    if (!src) return 0;
    if (src->GetLastStatus() != Ok) { delete src; return 0; }
    // copy into a 32bpp PARGB bitmap so drawing handles alpha consistently
    Bitmap* dst = new Bitmap(src->GetWidth(), src->GetHeight(), PixelFormat32bppPARGB);
    Graphics g(dst);
    g.SetCompositingMode(CompositingModeSourceCopy);
    g.DrawImage(src, Rect(0, 0, (int)src->GetWidth(), (int)src->GetHeight()),
                0, 0, (int)src->GetWidth(), (int)src->GetHeight(), UnitPixel);
    delete src;
    return dst;
}

bool saveBitmap(Bitmap* bmp, const QString& file) {
    if (!bmp) return false;
    CLSID clsid;
    if (encoderClsidForSuffix(mimeForFile(file), &clsid) < 0) return false;
    return bmp->Save(file.c_str(), &clsid, 0) == Ok;
}

void fillBitmap(Bitmap* bmp, const QColor& color) {
    if (!bmp) return;
    Graphics g(bmp);
    g.SetCompositingMode(CompositingModeSourceCopy);
    g.Clear(Color(color.alpha(), color.red(), color.green(), color.blue()));
}

Bitmap* scaleBitmap(Bitmap* src, int w, int h, Qt::TransformationMode mode) {
    if (!src) return 0;
    Bitmap* dst = new Bitmap(w, h, PixelFormat32bppPARGB);
    Graphics g(dst);
    g.SetCompositingMode(CompositingModeSourceCopy);
    g.SetInterpolationMode(mode == Qt::SmoothTransformation ? InterpolationModeHighQualityBicubic
                                                            : InterpolationModeNearestNeighbor);
    g.DrawImage(src, Rect(0, 0, w, h), 0, 0, (int)src->GetWidth(), (int)src->GetHeight(), UnitPixel);
    return dst;
}

} // namespace

// ---------------------------------------------------------------------------
// QPixmap
// ---------------------------------------------------------------------------
QPixmap::QPixmap() : data_(0) {}

QPixmap::QPixmap(int w, int h) : data_(createData(w, h)) {}

QPixmap::QPixmap(const QSize& size) : data_(createData(size.width(), size.height())) {}

QPixmap::QPixmap(const QPixmap& other) : data_(other.data_) { addRef(data_); }

QPixmap::QPixmap(const QImage& image) : data_(0) {
    if (image.data()) { data_ = image.data(); addRef(data_); }
}

QPixmap::~QPixmap() { release(data_); }

QPixmap& QPixmap::operator=(const QPixmap& other) {
    if (this == &other) return *this;
    QBitmapData* old = data_;
    data_ = other.data_;
    addRef(data_);
    release(old);
    return *this;
}

bool QPixmap::load(const QString& fileName, const char*) {
    Bitmap* bmp = loadBitmap(fileName);
    if (!bmp) return false;
    release(data_);
    data_ = new QBitmapData();
    data_->bmp = bmp;
    data_->refs = 1;
    data_->sz = QSize((int)bmp->GetWidth(), (int)bmp->GetHeight());
    return true;
}

bool QPixmap::loadFromData(const std::string& bytes) {
    ensureGdiplus();
    if (bytes.empty()) return false;
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, bytes.size());
    if (!mem) return false;
    void* p = GlobalLock(mem);
    memcpy(p, bytes.data(), bytes.size());
    GlobalUnlock(mem);
    IStream* stream = 0;
    if (CreateStreamOnHGlobal(mem, TRUE, &stream) != S_OK) { GlobalFree(mem); return false; }
    Bitmap* bmp = Bitmap::FromStream(stream, FALSE);
    stream->Release();
    if (!bmp || bmp->GetLastStatus() != Ok) { delete bmp; return false; }
    release(data_);
    data_ = new QBitmapData();
    data_->bmp = bmp;
    data_->refs = 1;
    data_->sz = QSize((int)bmp->GetWidth(), (int)bmp->GetHeight());
    return true;
}

bool QPixmap::save(const QString& fileName, const char*) const {
    return saveBitmap((Bitmap*)(data_ ? data_->bmp : 0), fileName);
}

void QPixmap::fill(const QColor& color) {
    if (!data_) return;
    fillBitmap((Bitmap*)data_->bmp, color);
}

QPixmap QPixmap::scaled(const QSize& s, Qt::AspectRatioMode mode, Qt::TransformationMode transform) const {
    if (!data_) return QPixmap();
    int srcW = width(), srcH = height();
    if (srcW <= 0 || srcH <= 0) return QPixmap();
    int w = s.width(), h = s.height();
    if (mode == Qt::KeepAspectRatio || mode == Qt::KeepAspectRatioByExpanding) {
        qreal sx = qreal(w) / srcW, sy = qreal(h) / srcH;
        qreal k = (mode == Qt::KeepAspectRatio) ? qMin(sx, sy) : qMax(sx, sy);
        w = qMax(1, qRound(srcW * k));
        h = qMax(1, qRound(srcH * k));
    }
    Bitmap* bmp = scaleBitmap((Bitmap*)data_->bmp, w, h, transform);
    return QPixmap::fromBitmap(bmp);
}

QImage QPixmap::toImage() const { return QImage(*this); }

QPixmap QPixmap::fromImage(const QImage& image) { return QPixmap(image); }

QPixmap QPixmap::fromBitmap(void* gdiplusBitmap) {
    QPixmap pm;
    if (!gdiplusBitmap) return pm;
    Bitmap* bmp = (Bitmap*)gdiplusBitmap;
    pm.data_ = new QBitmapData();
    pm.data_->bmp = bmp;
    pm.data_->refs = 1;
    pm.data_->sz = QSize((int)bmp->GetWidth(), (int)bmp->GetHeight());
    return pm;
}

// ---------------------------------------------------------------------------
// QImage
// ---------------------------------------------------------------------------
QImage::QImage() : data_(0) {}
QImage::QImage(int w, int h) : data_(createData(w, h)) {}
QImage::QImage(const QSize& size) : data_(createData(size.width(), size.height())) {}
QImage::QImage(const QImage& other) : data_(other.data_) { addRef(data_); }
QImage::QImage(const QPixmap& pixmap) : data_(pixmap.data()) { addRef(data_); }
QImage::~QImage() { release(data_); }

QImage& QImage::operator=(const QImage& other) {
    if (this == &other) return *this;
    QBitmapData* old = data_;
    data_ = other.data_;
    addRef(data_);
    release(old);
    return *this;
}

void QImage::fill(const QColor& color) { fillBitmap((Bitmap*)(data_ ? data_->bmp : 0), color); }

bool QImage::load(const QString& fileName) {
    QPixmap pm;
    if (!pm.load(fileName)) return false;
    *this = QImage(pm);
    return true;
}

bool QImage::save(const QString& fileName, const char*) const {
    return saveBitmap((Bitmap*)(data_ ? data_->bmp : 0), fileName);
}

// ---------------------------------------------------------------------------
// QIcon
// ---------------------------------------------------------------------------
QIcon::QIcon() {}
QIcon::QIcon(const QString& fileName) { pix_.load(fileName); }
QIcon::QIcon(const QPixmap& pixmap) : pix_(pixmap) {}

QPixmap QIcon::pixmap(const QSize& size, int, int) const {
    if (pix_.isNull()) return QPixmap();
    if (size.width() > 0 && size.height() > 0 && size != pix_.size())
        return pix_.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return pix_;
}

QSize QIcon::actualSize(const QSize& size) const {
    if (pix_.isNull()) return QSize(0, 0);
    if (size.width() <= 0 || size.height() <= 0) return pix_.size();
    return pix_.size().boundedTo(size);
}

// ---------------------------------------------------------------------------
// QCursor
// ---------------------------------------------------------------------------
QCursor::QCursor() : shape_(Qt::ArrowCursor) {}
QCursor::QCursor(Qt::CursorShape shape) : shape_(shape) {}

QPoint QCursor::pos() {
    POINT p;
    GetCursorPos(&p);
    return QPoint(p.x, p.y);
}

void QCursor::setPos(int x, int y) { SetCursorPos(x, y); }

void* QCursor::handle() const {
    static void* cache[Qt::BusyCursor + 2];
    int idx = (int)shape_;
    if (idx < 0 || idx >= (int)(sizeof(cache) / sizeof(cache[0]))) idx = 0;
    if (cache[idx]) return cache[idx];
    const wchar_t* id = IDC_ARROW;
    switch (shape_) {
    case Qt::UpArrowCursor: id = IDC_UPARROW; break;
    case Qt::CrossCursor: id = IDC_CROSS; break;
    case Qt::WaitCursor: id = IDC_WAIT; break;
    case Qt::IBeamCursor: id = IDC_IBEAM; break;
    case Qt::SizeVerCursor: id = IDC_SIZENS; break;
    case Qt::SizeHorCursor: id = IDC_SIZEWE; break;
    case Qt::SizeBDiagCursor: id = IDC_SIZENESW; break;
    case Qt::SizeFDiagCursor: id = IDC_SIZENWSE; break;
    case Qt::SizeAllCursor: id = IDC_SIZEALL; break;
    case Qt::BlankCursor: id = 0; break;
    case Qt::SplitVCursor: id = IDC_SIZENS; break;
    case Qt::SplitHCursor: id = IDC_SIZEWE; break;
    case Qt::PointingHandCursor: id = IDC_HAND; break;
    case Qt::OpenHandCursor: id = IDC_HAND; break;
    case Qt::ClosedHandCursor: id = IDC_HAND; break;
    case Qt::WhatsThisCursor: id = IDC_HELP; break;
    case Qt::BusyCursor: id = IDC_APPSTARTING; break;
    default: id = IDC_ARROW; break;
    }
    HCURSOR h = id ? LoadCursorW(0, id) : 0;
    cache[idx] = (void*)h;
    return (void*)h;
}

// ---------------------------------------------------------------------------
// QPalette
// ---------------------------------------------------------------------------
QPalette::QPalette() {
    for (int g = 0; g < NColorGroups; ++g)
        for (int r = 0; r < NColorRoles; ++r)
            colors_[g][r] = QColor(0, 0, 0);

    QColor window(0xF0, 0xF0, 0xF0);
    QColor base(0xFF, 0xFF, 0xFF);
    QColor text(0x1A, 0x1A, 0x1A);
    QColor disabledText(0x9A, 0x9A, 0x9A);
    QColor highlight(0x00, 0x78, 0xD4);

    for (int g = 0; g < NColorGroups; ++g) {
        QColor fg = (g == Disabled) ? disabledText : text;
        colors_[g][WindowText] = fg;
        colors_[g][Text] = fg;
        colors_[g][ButtonText] = fg;
        colors_[g][BrightText] = QColor(0xFF, 0xFF, 0xFF);
        colors_[g][Button] = window;
        colors_[g][Light] = QColor(0xFF, 0xFF, 0xFF);
        colors_[g][Midlight] = QColor(0xE3, 0xE3, 0xE3);
        colors_[g][Mid] = QColor(0xC8, 0xC8, 0xC8);
        colors_[g][Dark] = QColor(0x9F, 0x9F, 0x9F);
        colors_[g][Shadow] = QColor(0x69, 0x69, 0x69);
        colors_[g][Base] = base;
        colors_[g][AlternateBase] = QColor(0xF7, 0xF7, 0xF7);
        colors_[g][Window] = window;
        colors_[g][Highlight] = (g == Disabled) ? QColor(0xBD, 0xBD, 0xBD) : highlight;
        colors_[g][HighlightedText] = QColor(0xFF, 0xFF, 0xFF);
        colors_[g][Link] = QColor(0x00, 0x00, 0xEE);
        colors_[g][LinkVisited] = QColor(0x80, 0x00, 0x80);
        colors_[g][ToolTipBase] = QColor(0xFF, 0xFF, 0xDC);
        colors_[g][ToolTipText] = QColor(0x1A, 0x1A, 0x1A);
        colors_[g][PlaceholderText] = QColor(0x8A, 0x8A, 0x8A);
        colors_[g][NoRole] = QColor(0, 0, 0);
    }
}

QColor QPalette::color(ColorGroup group, ColorRole role) const {
    if (role < 0 || role >= NColorRoles) return QColor();
    return colors_[group][role];
}

void QPalette::setColor(ColorRole role, const QColor& c) {
    if (role < 0 || role >= NColorRoles) return;
    for (int g = 0; g < NColorGroups; ++g) colors_[g][role] = c;
}

void QPalette::setColor(ColorGroup group, ColorRole role, const QColor& c) {
    if (role < 0 || role >= NColorRoles || group < 0 || group >= NColorGroups) return;
    colors_[group][role] = c;
}

void QPalette::setBrush(ColorRole role, const QBrush& b) { setColor(role, b.color()); }

// ---------------------------------------------------------------------------
// QFontMetrics + text helpers
// ---------------------------------------------------------------------------
namespace {

Graphics* measureGraphics() {
    static Bitmap* bmp = 0;
    static Graphics* g = 0;
    if (!g) {
        ensureGdiplus();
        bmp = new Bitmap(8, 8, PixelFormat32bppPARGB);
        g = new Graphics(bmp);
        g->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    }
    return g;
}

std::wstring fontKey(const QFont& f) {
    wchar_t buf[128];
    _snwprintf_s(buf, 128, _TRUNCATE, L"%s|%d|%d|%d|%d|%d",
                 f.family().c_str(), f.pixelHeight(), f.weight(),
                 f.italic() ? 1 : 0, f.underline() ? 1 : 0, f.strikeOut() ? 1 : 0);
    return std::wstring(buf);
}

struct FontCache {
    std::map<std::wstring, Font*> map;
    ~FontCache() {
        for (std::map<std::wstring, Font*>::iterator it = map.begin(); it != map.end(); ++it)
            delete it->second;
        map.clear();
    }
};

FontCache& fontCache() { static FontCache c; return c; }

FontFamily* pickFontFamily(const QString& family) {
    static std::map<std::wstring, FontFamily*> cache;
    std::wstring key = family.c_str();
    std::map<std::wstring, FontFamily*>::iterator it = cache.find(key);
    if (it != cache.end()) return it->second;
    FontFamily* fam = new FontFamily(family.c_str());
    if (fam->GetLastStatus() != Ok) {
        delete fam;
        const wchar_t* fallbacks[] = { L"Microsoft YaHei UI", L"Microsoft YaHei", L"Segoe UI",
                                       L"Tahoma", L"Arial", L"SimSun", 0 };
        fam = 0;
        for (int i = 0; fallbacks[i] && !fam; ++i) {
            FontFamily* f = new FontFamily(fallbacks[i]);
            if (f->GetLastStatus() == Ok) fam = f;
            else delete f;
        }
        if (!fam) fam = FontFamily::GenericSansSerif()->Clone();
    }
    cache[key] = fam;
    return fam;
}

} // namespace

void qtcEnsureGraphics() { ensureGdiplus(); }

void* qtcGdiplusFontFor(const QFont& font) {
    ensureGdiplus();
    std::wstring key = fontKey(font);
    FontCache& cache = fontCache();
    std::map<std::wstring, Font*>::iterator it = cache.map.find(key);
    if (it != cache.map.end()) return it->second;

    int style = FontStyleRegular;
    if (font.bold()) style |= FontStyleBold;
    if (font.italic()) style |= FontStyleItalic;
    if (font.underline()) style |= FontStyleUnderline;
    if (font.strikeOut()) style |= FontStyleStrikeout;

    FontFamily* fam = pickFontFamily(font.family());
    Font* gdiFont = new Font(fam, (REAL)font.pixelHeight(), style, UnitPixel);
    if (gdiFont->GetLastStatus() != Ok) {
        delete gdiFont;
        gdiFont = new Font(FontFamily::GenericSansSerif(), (REAL)font.pixelHeight(), style, UnitPixel);
    }
    cache.map[key] = gdiFont;
    return gdiFont;
}

QString qtcStripMnemonics(const QString& text) {
    QString out;
    for (int i = 0; i < text.size(); ++i) {
        wchar_t c = text[i];
        if (c == L'&') {
            if (i + 1 < text.size() && text[i + 1] == L'&') { out += L'&'; ++i; }
            continue;
        }
        out += c;
    }
    return out;
}

int qtcTextWidth(const QFont& font, const QString& text) {
    if (text.isEmpty()) return 0;
    ensureGdiplus();
    QString plain = qtcStripMnemonics(text);
    if (plain.isEmpty()) return 0;
    RectF box;
    measureGraphics()->MeasureString(plain.c_str(), -1, (Font*)qtcGdiplusFontFor(font),
                                     PointF(0, 0), &box);
    return qCeil(box.Width);
}

QFontMetrics::QFontMetrics(const QFont& font) : font_(font) {}

int QFontMetrics::width(const QString& text) const { return qtcTextWidth(font_, text); }
int QFontMetrics::width(wchar_t ch) const { return qtcTextWidth(font_, QString(ch)); }

int QFontMetrics::height() const {
    Font* f = (Font*)qtcGdiplusFontFor(font_);
    return qCeil(f->GetHeight(measureGraphics()));
}

int QFontMetrics::ascent() const {
    Font* f = (Font*)qtcGdiplusFontFor(font_);
    return qCeil(f->GetSize() * 0.8);
}

int QFontMetrics::descent() const { return qMax(1, height() - ascent()); }
int QFontMetrics::lineSpacing() const { return height() + 2; }

QRect QFontMetrics::boundingRect(const QString& text) const {
    if (text.isEmpty()) return QRect();
    RectF box;
    measureGraphics()->MeasureString(text.c_str(), -1, (Font*)qtcGdiplusFontFor(font_),
                                     PointF(0, 0), &box);
    return QRect(0, 0, qCeil(box.Width) + 1, qCeil(box.Height) + 1);
}

QRect QFontMetrics::boundingRect(wchar_t ch) const { return boundingRect(QString(ch)); }

QPixmap qtcCreateTransparentPixmap(int w, int h) { return QPixmap(w, h); }

QPixmap qtcPixmapFromData(QBitmapData* data) {
    QPixmap pm;
    if (!data) return pm;
    ++data->refs;
    pm.data_ = data;
    return pm;
}
