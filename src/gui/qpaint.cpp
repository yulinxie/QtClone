// QtClone - qpaint.cpp : QPainter on top of GDI+, and QVariant glue for GUI types.
#include <qtc/QtGui.h>
#include "qwidget_p.h"

#include "qtc_gdiplus.h"
#include <vector>
#include <stdio.h>

using namespace Gdiplus;

struct QPainterState {
    QPen pen;
    QBrush brush;
    QFont font;
    QBrush background;
    QRect clip;
    bool clipping;
    qreal tx, ty;
    GraphicsState gs;
};

struct QPainterPrivate {
    Graphics* g;
    bool ownsGraphics;
    GraphicsState savedState;
    QBitmapData* target;
    QPen pen;
    QBrush brush;
    QFont font;
    QBrush background;
    QRect clip;
    bool clipping;
    qreal tx, ty;
    bool antialias, textAA, smoothPixmap;
    bool active;
    bool needSync;
    std::vector<QPainterState> stack;

    QPainterPrivate()
        : g(0), ownsGraphics(true), savedState(0), target(0), clipping(false), tx(0), ty(0),
          antialias(true), textAA(true), smoothPixmap(true), active(false), needSync(true) {}
};

namespace {

void applyState(QPainterPrivate* d) {
    if (!d->g) return;
    if (!d->needSync) return;
    d->g->ResetTransform();
    if (d->tx != 0 || d->ty != 0) d->g->TranslateTransform((REAL)d->tx, (REAL)d->ty);
    d->g->ResetClip();
    if (d->clipping) {
        // SetClip interprets the rectangle in the current (already translated)
        // transform space, so no manual offset is applied here.
        RectF r((REAL)d->clip.x(), (REAL)d->clip.y(),
                (REAL)d->clip.width(), (REAL)d->clip.height());
        d->g->SetClip(r, CombineModeIntersect);
    }
    d->g->SetSmoothingMode(d->antialias ? SmoothingModeAntiAlias : SmoothingModeNone);
    d->g->SetTextRenderingHint(d->textAA ? TextRenderingHintClearTypeGridFit : TextRenderingHintSystemDefault);
    d->g->SetInterpolationMode(d->smoothPixmap ? InterpolationModeHighQualityBicubic
                                              : InterpolationModeNearestNeighbor);
    d->g->SetPixelOffsetMode(PixelOffsetModeHalf);
    d->g->SetCompositingMode(CompositingModeSourceOver);
    d->g->SetCompositingQuality(CompositingQualityHighQuality);
    d->needSync = false;
}

void setupPen(Pen& p, const QPen& pen) {
    QColor c = pen.color();
    p.SetColor(Color(c.alpha(), c.red(), c.green(), c.blue()));
    p.SetWidth(pen.widthF() > 0 ? (REAL)pen.widthF() : 1.0f);
    switch (pen.style()) {
    case Qt::DashLine: p.SetDashStyle(DashStyleDash); break;
    case Qt::DotLine: p.SetDashStyle(DashStyleDot); break;
    case Qt::DashDotLine: p.SetDashStyle(DashStyleDashDot); break;
    case Qt::DashDotDotLine: p.SetDashStyle(DashStyleDashDotDot); break;
    case Qt::NoPen: p.SetColor(Color(0, 0, 0, 0)); break;
    default: p.SetDashStyle(DashStyleSolid); break;
    }
    LineCap cap = LineCapSquare;
    if (pen.capStyle() == Qt::FlatCap) cap = LineCapFlat;
    else if (pen.capStyle() == Qt::RoundCap) cap = LineCapRound;
    p.SetLineCap(cap, cap, DashCapFlat);
    switch (pen.joinStyle()) {
    case Qt::MiterJoin: p.SetLineJoin(LineJoinMiter); break;
    case Qt::RoundJoin: p.SetLineJoin(LineJoinRound); break;
    default: p.SetLineJoin(LineJoinBevel); break;
    }
}

bool brushVisible(const QBrush& b) { return b.style() != Qt::NoBrush && b.color().alpha() > 0; }
bool penVisible(const QPen& p) { return p.style() != Qt::NoPen && p.color().alpha() > 0; }

void setupFormat(StringFormat& fmt, int flags, bool hasMnemonic) {
    INT f = 0;
    if (!(flags & Qt::TextWordWrap)) f |= StringFormatFlagsNoWrap;
    if (flags & Qt::TextDontClip) f |= StringFormatFlagsNoClip;
    fmt.SetFormatFlags(f);
    fmt.SetHotkeyPrefix(hasMnemonic ? HotkeyPrefixShow : HotkeyPrefixNone);
    if (flags & Qt::AlignRight) fmt.SetAlignment(StringAlignmentFar);
    else if (flags & Qt::AlignHCenter) fmt.SetAlignment(StringAlignmentCenter);
    else fmt.SetAlignment(StringAlignmentNear);
    if (flags & Qt::AlignBottom) fmt.SetLineAlignment(StringAlignmentFar);
    else if (flags & Qt::AlignVCenter) fmt.SetLineAlignment(StringAlignmentCenter);
    else fmt.SetLineAlignment(StringAlignmentNear);
    if (flags & Qt::TextSingleLine) fmt.SetTrimming(StringTrimmingEllipsisCharacter);
    if (flags & Qt::TextWordWrap) fmt.SetTrimming(StringTrimmingNone);
}

GraphicsPath* roundedPath(const QRect& rect, qreal rx, qreal ry) {
    GraphicsPath* path = new GraphicsPath();
    qreal w = rect.width(), h = rect.height();
    qreal x = rect.x(), y = rect.y();
    qreal maxR = qMin(w, h) / 2.0;
    rx = qMin(rx, maxR);
    ry = qMin(ry, maxR);
    if (rx <= 0.5 || ry <= 0.5) {
        path->AddRectangle(RectF((REAL)x, (REAL)y, (REAL)w, (REAL)h));
        return path;
    }
    path->AddArc((REAL)x, (REAL)y, (REAL)(rx * 2), (REAL)(ry * 2), 180.0f, 90.0f);
    path->AddArc((REAL)(x + w - rx * 2), (REAL)y, (REAL)(rx * 2), (REAL)(ry * 2), 270.0f, 90.0f);
    path->AddArc((REAL)(x + w - rx * 2), (REAL)(y + h - ry * 2), (REAL)(rx * 2), (REAL)(ry * 2), 0.0f, 90.0f);
    path->AddArc((REAL)x, (REAL)(y + h - ry * 2), (REAL)(rx * 2), (REAL)(ry * 2), 90.0f, 90.0f);
    path->CloseFigure();
    return path;
}

} // namespace

// ---------------------------------------------------------------------------
// QPainter
// ---------------------------------------------------------------------------
QPainter::QPainter() : d_(new QPainterPrivate()) {}
QPainter::QPainter(QPixmap* pixmap) : d_(new QPainterPrivate()) { begin(pixmap); }

namespace {
struct PaintTarget {
    void* graphics;
    QPoint origin;
};
std::vector<PaintTarget>& paintTargets() { static std::vector<PaintTarget> v; return v; }
} // namespace

void qtcPushPaintTarget(void* graphics, const QPoint& origin) {
    PaintTarget t;
    t.graphics = graphics;
    t.origin = origin;
    paintTargets().push_back(t);
}

void qtcPopPaintTarget() {
    if (!paintTargets().empty()) paintTargets().pop_back();
}

bool qtcCurrentPaintTarget(void** graphics, QPoint* origin) {
    if (paintTargets().empty()) return false;
    const PaintTarget& t = paintTargets().back();
    if (graphics) *graphics = t.graphics;
    if (origin) *origin = t.origin;
    return t.graphics != 0;
}

QPainter::QPainter(QWidget* widget) : d_(new QPainterPrivate()) {
    if (!widget) return;
    void* graphics = 0;
    QPoint origin(0, 0);
    if (qtcCurrentPaintTarget(&graphics, &origin)) {
        d_->g = (Graphics*)graphics;
        d_->ownsGraphics = false;
        d_->savedState = d_->g->Save();
        d_->tx = origin.x();
        d_->ty = origin.y();
        d_->clip = widget->rect();
        d_->clipping = true;
        d_->needSync = true;
        d_->active = true;
        return;
    }
    QWidgetWindow* win = widget->windowHandle();
    if (!win || win->back.isNull()) return;
    QPoint windowOrigin(0, 0);
    for (const QWidget* w = widget; w && !w->isWindow(); w = w->parentWidget())
        windowOrigin += w->pos();
    if (!begin(&win->back)) return;
    d_->tx = windowOrigin.x();
    d_->ty = windowOrigin.y();
    d_->clip = widget->rect();
    d_->clipping = true;
    d_->needSync = true;
}
QPainter::QPainter(QImage* image) : d_(new QPainterPrivate()) { begin(image); }

QPainter::~QPainter() {
    end();
    delete d_;
}

bool QPainter::begin(QPixmap* pixmap) {
    if (!pixmap || !pixmap->bitmap()) return false;
    end();
    d_->target = pixmap->data();
    if (d_->target) ++d_->target->refs;
    d_->g = new Graphics((Bitmap*)pixmap->bitmap());
    d_->ownsGraphics = true;
    d_->active = true;
    d_->needSync = true;
    return true;
}

bool QPainter::begin(QImage* image) {
    if (!image || !image->data()) return false;
    end();
    d_->target = image->data();
    if (d_->target) ++d_->target->refs;
    d_->g = new Graphics((Bitmap*)image->data()->bmp);
    d_->active = true;
    d_->needSync = true;
    return true;
}

bool QPainter::end() {
    if (d_->g) {
        if (d_->ownsGraphics) {
            delete d_->g;
        } else {
            d_->g->Restore(d_->savedState);
        }
        d_->g = 0;
    }
    if (d_->target) {
        // refcount was increased in begin(); QBitmapData is shared with QPixmap
        if (--d_->target->refs == 0) {
            delete (Bitmap*)d_->target->bmp;
            delete d_->target;
        }
        d_->target = 0;
    }
    d_->active = false;
    return true;
}

bool QPainter::isActive() const { return d_->active && d_->g != 0; }

void QPainter::save() {
    QPainterState st;
    st.pen = d_->pen;
    st.brush = d_->brush;
    st.font = d_->font;
    st.background = d_->background;
    st.clip = d_->clip;
    st.clipping = d_->clipping;
    st.tx = d_->tx;
    st.ty = d_->ty;
    st.gs = d_->g ? d_->g->Save() : 0;
    d_->stack.push_back(st);
}

void QPainter::restore() {
    if (d_->stack.empty()) return;
    QPainterState st = d_->stack.back();
    d_->stack.pop_back();
    d_->pen = st.pen;
    d_->brush = st.brush;
    d_->font = st.font;
    d_->background = st.background;
    d_->clip = st.clip;
    d_->clipping = st.clipping;
    d_->tx = st.tx;
    d_->ty = st.ty;
    d_->needSync = true;
}

void QPainter::setPen(const QPen& pen) { d_->pen = pen; }
void QPainter::setPen(const QColor& color) { d_->pen = QPen(color); }
void QPainter::setPen(Qt::PenStyle style) { d_->pen.setStyle(style); }
const QPen& QPainter::pen() const { return d_->pen; }

void QPainter::setBrush(const QBrush& brush) { d_->brush = brush; }
void QPainter::setBrush(const QColor& color) { d_->brush = QBrush(color); }
const QBrush& QPainter::brush() const { return d_->brush; }

void QPainter::setFont(const QFont& font) { d_->font = font; }
const QFont& QPainter::font() const { return d_->font; }

void QPainter::setRenderHint(RenderHint hint, bool on) {
    if (hint == Antialiasing) d_->antialias = on;
    else if (hint == TextAntialiasing) d_->textAA = on;
    else if (hint == SmoothPixmapTransform) d_->smoothPixmap = on;
    d_->needSync = true;
}

void QPainter::setBackground(const QBrush& brush) { d_->background = brush; }
const QBrush& QPainter::background() const { return d_->background; }

void QPainter::setClipRect(const QRect& rect) {
    d_->clip = rect;
    d_->clipping = true;
    d_->needSync = true;
}

void QPainter::setClipRegion(const QRegion& region) {
    d_->clip = region.boundingRect();
    d_->clipping = true;
    d_->needSync = true;
}

void QPainter::setClipping(bool enable) { d_->clipping = enable; d_->needSync = true; }
bool QPainter::hasClipping() const { return d_->clipping; }
QRect QPainter::clipBoundingRect() const { return d_->clipping ? d_->clip : QRect(0, 0, 100000, 100000); }

void QPainter::translate(qreal dx, qreal dy) {
    d_->tx += dx;
    d_->ty += dy;
    d_->needSync = true;
}

void QPainter::scale(qreal, qreal) {}

void QPainter::resetTransform() {
    d_->tx = 0;
    d_->ty = 0;
    d_->needSync = true;
}

void QPainter::fillRect(const QRect& rect, const QColor& color) { fillRect(rect, QBrush(color)); }

void QPainter::fillRect(int x, int y, int w, int h, const QColor& color) {
    fillRect(QRect(x, y, w, h), QBrush(color));
}

void QPainter::fillRect(const QRect& rect, const QBrush& brush) {
    if (!isActive() || rect.isEmpty() || !brushVisible(brush)) return;
    applyState(d_);
    QColor c = brush.color();
    SolidBrush br(Color(c.alpha(), c.red(), c.green(), c.blue()));
    d_->g->FillRectangle(&br, (REAL)rect.x(), (REAL)rect.y(), (REAL)rect.width(), (REAL)rect.height());
}

void QPainter::eraseRect(const QRect& rect) {
    if (d_->background.style() != Qt::NoBrush) fillRect(rect, d_->background);
}

void QPainter::drawRect(const QRect& rect) {
    if (!isActive() || rect.isEmpty() || !penVisible(d_->pen)) return;
    applyState(d_);
    Pen p(Color(0, 0, 0));
    setupPen(p, d_->pen);
    qreal inset = d_->pen.widthF() / 2.0;
    d_->g->DrawRectangle(&p, (REAL)(rect.x() + inset), (REAL)(rect.y() + inset),
                         (REAL)qMax(0, rect.width() - (int)(inset * 2)),
                         (REAL)qMax(0, rect.height() - (int)(inset * 2)));
}

void QPainter::drawRoundedRect(const QRect& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode) {
    if (!isActive() || rect.isEmpty()) return;
    applyState(d_);
    qreal rx = xRadius, ry = yRadius;
    if (mode == Qt::RelativeSize) { rx = rect.width() * xRadius / 100.0; ry = rect.height() * yRadius / 100.0; }
    GraphicsPath* path = roundedPath(rect, rx, ry);
    if (brushVisible(d_->brush)) {
        QColor c = d_->brush.color();
        SolidBrush br(Color(c.alpha(), c.red(), c.green(), c.blue()));
        d_->g->FillPath(&br, path);
    }
    if (penVisible(d_->pen)) {
        Pen p(Color(0, 0, 0));
        setupPen(p, d_->pen);
        d_->g->DrawPath(&p, path);
    }
    delete path;
}

void QPainter::drawPoint(const QPoint& p) { drawPoint(p.x(), p.y()); }

void QPainter::drawPoint(int x, int y) {
    if (!isActive() || !penVisible(d_->pen)) return;
    applyState(d_);
    Pen pen(Color(0, 0, 0));
    setupPen(pen, d_->pen);
    QColor c = d_->pen.color();
    SolidBrush brush(Color(c.alpha(), c.red(), c.green(), c.blue()));
    d_->g->FillRectangle(&brush, (REAL)x, (REAL)y, (REAL)1, (REAL)1);
    (void)pen;
}

void QPainter::drawLine(const QPoint& p1, const QPoint& p2) {
    drawLine(p1.x(), p1.y(), p2.x(), p2.y());
}

void QPainter::drawLine(int x1, int y1, int x2, int y2) {
    if (!isActive() || !penVisible(d_->pen)) return;
    applyState(d_);
    Pen p(Color(0, 0, 0));
    setupPen(p, d_->pen);
    d_->g->DrawLine(&p, (REAL)x1, (REAL)y1, (REAL)x2, (REAL)y2);
}

void QPainter::drawEllipse(const QRect& rect) {
    if (!isActive() || rect.isEmpty()) return;
    applyState(d_);
    if (brushVisible(d_->brush)) {
        QColor c = d_->brush.color();
        SolidBrush br(Color(c.alpha(), c.red(), c.green(), c.blue()));
        d_->g->FillEllipse(&br, (REAL)rect.x(), (REAL)rect.y(), (REAL)rect.width(), (REAL)rect.height());
    }
    if (penVisible(d_->pen)) {
        Pen p(Color(0, 0, 0));
        setupPen(p, d_->pen);
        qreal inset = d_->pen.widthF() / 2.0;
        d_->g->DrawEllipse(&p, (REAL)(rect.x() + inset), (REAL)(rect.y() + inset),
                           (REAL)qMax(0.0, rect.width() - inset * 2), (REAL)qMax(0.0, rect.height() - inset * 2));
    }
}

void QPainter::drawEllipse(const QPoint& center, int rx, int ry) {
    drawEllipse(QRect(center.x() - rx, center.y() - ry, rx * 2, ry * 2));
}

void QPainter::drawPolyline(const QList<QPoint>& points) {
    if (!isActive() || points.count() < 2 || !penVisible(d_->pen)) return;
    applyState(d_);
    Pen p(Color(0, 0, 0));
    setupPen(p, d_->pen);
    std::vector<Point> pts;
    pts.reserve(points.count());
    for (int i = 0; i < points.count(); ++i) pts.push_back(Point(points[i].x(), points[i].y()));
    d_->g->DrawLines(&p, &pts[0], (INT)pts.size());
}

void QPainter::drawPolygon(const QList<QPoint>& points, Qt::FillRule) {
    if (!isActive() || points.count() < 3) return;
    applyState(d_);
    std::vector<Point> pts;
    for (int i = 0; i < points.count(); ++i) pts.push_back(Point(points[i].x(), points[i].y()));
    if (brushVisible(d_->brush)) {
        QColor c = d_->brush.color();
        SolidBrush br(Color(c.alpha(), c.red(), c.green(), c.blue()));
        d_->g->FillPolygon(&br, &pts[0], (INT)pts.size());
    }
    if (penVisible(d_->pen)) {
        Pen p(Color(0, 0, 0));
        setupPen(p, d_->pen);
        d_->g->DrawPolygon(&p, &pts[0], (INT)pts.size());
    }
}

void QPainter::drawArc(const QRect& rect, int startAngle, int spanAngle) {
    if (!isActive() || !penVisible(d_->pen)) return;
    applyState(d_);
    Pen p(Color(0, 0, 0));
    setupPen(p, d_->pen);
    d_->g->DrawArc(&p, (REAL)rect.x(), (REAL)rect.y(), (REAL)rect.width(), (REAL)rect.height(),
                   (REAL)startAngle, (REAL)spanAngle);
}

void QPainter::drawText(const QPoint& pos, const QString& text) {
    if (!isActive() || text.isEmpty()) return;
    applyState(d_);
    QColor c = d_->pen.color();
    SolidBrush br(Color(c.alpha(), c.red(), c.green(), c.blue()));
    StringFormat fmt;
    setupFormat(fmt, Qt::TextSingleLine | Qt::TextDontClip, false);
    d_->g->DrawString(qtcStripMnemonics(text).c_str(), -1, (Font*)qtcGdiplusFontFor(d_->font),
                      PointF((REAL)pos.x(), (REAL)pos.y()), &fmt, &br);
}

void QPainter::drawText(int x, int y, const QString& text) { drawText(QPoint(x, y), text); }

void QPainter::drawText(const QRect& rect, int flags, const QString& text) {
    if (!isActive() || text.isEmpty()) return;
    applyState(d_);
    QColor c = d_->pen.color();
    SolidBrush br(Color(c.alpha(), c.red(), c.green(), c.blue()));
    StringFormat fmt;
    bool mnemonic = (flags & Qt::TextShowMnemonic) != 0;
    setupFormat(fmt, flags, mnemonic);
    QString draw = mnemonic ? text : qtcStripMnemonics(text);
    d_->g->DrawString(draw.c_str(), -1, (Font*)qtcGdiplusFontFor(d_->font),
                      RectF((REAL)rect.x(), (REAL)rect.y(), (REAL)rect.width(), (REAL)rect.height()),
                      &fmt, &br);
}

QRect QPainter::boundingRect(const QRect& rect, int flags, const QString& text) const {
    if (!d_->g || text.isEmpty()) return QRect();
    StringFormat fmt;
    setupFormat(fmt, flags, false);
    RectF out;
    d_->g->MeasureString(qtcStripMnemonics(text).c_str(), -1, (Font*)qtcGdiplusFontFor(d_->font),
                         RectF((REAL)rect.x(), (REAL)rect.y(), (REAL)rect.width(), (REAL)rect.height()),
                         &fmt, &out);
    return QRect(qRound(out.X), qRound(out.Y), qCeil(out.Width), qCeil(out.Height));
}

void QPainter::drawPixmap(const QPoint& pos, const QPixmap& pixmap) {
    if (pixmap.isNull()) return;
    drawPixmap(QRect(pos, pixmap.size()), pixmap);
}

void QPainter::drawPixmap(const QRect& target, const QPixmap& pixmap) {
    if (pixmap.isNull()) return;
    drawPixmap(target, pixmap, QRect(0, 0, pixmap.width(), pixmap.height()));
}

void QPainter::drawPixmap(const QRect& target, const QPixmap& pixmap, const QRect& source) {
    if (!isActive() || pixmap.isNull() || target.isEmpty()) return;
    applyState(d_);
    Bitmap* bmp = (Bitmap*)pixmap.bitmap();
    if (!bmp) return;
    QRect src = source.isEmpty() ? QRect(0, 0, pixmap.width(), pixmap.height()) : source;
    d_->g->DrawImage(bmp, Rect((INT)target.x(), (INT)target.y(), (INT)target.width(), (INT)target.height()),
                     (INT)src.x(), (INT)src.y(), (INT)src.width(), (INT)src.height(), UnitPixel);
}

void QPainter::drawImage(const QPoint& pos, const QImage& image) {
    if (image.isNull()) return;
    drawImage(QRect(pos, image.size()), image);
}

void QPainter::drawImage(const QRect& target, const QImage& image) {
    if (image.isNull()) return;
    drawImage(target, image, QRect(0, 0, image.width(), image.height()));
}

void QPainter::drawImage(const QRect& target, const QImage& image, const QRect& source) {
    if (!isActive() || image.isNull() || target.isEmpty()) return;
    applyState(d_);
    Bitmap* bmp = (Bitmap*)image.data()->bmp;
    if (!bmp) return;
    QRect src = source.isEmpty() ? QRect(0, 0, image.width(), image.height()) : source;
    d_->g->DrawImage(bmp, Rect((INT)target.x(), (INT)target.y(), (INT)target.width(), (INT)target.height()),
                     (INT)src.x(), (INT)src.y(), (INT)src.width(), (INT)src.height(), UnitPixel);
}

void* QPainter::graphics() const { return d_->g; }

QBitmapData* QPainter::targetData() const { return d_->target; }

QPoint QPainter::paintOrigin() const { return QPoint(qRound(d_->tx), qRound(d_->ty)); }

// ---------------------------------------------------------------------------
// QVariant glue for GUI value types
// ---------------------------------------------------------------------------
void qtcRegisterGuiMetaTypes() {
    static bool done = false;
    if (done) return;
    done = true;
    QMetaType::registerBuiltinType<QColor>("QColor", QMetaType::Color);
    QMetaType::registerBuiltinType<QFont>("QFont", QMetaType::Font);
    QMetaType::registerBuiltinType<QPixmap>("QPixmap", QMetaType::Pixmap);
    QMetaType::registerBuiltinType<QImage>("QImage", QMetaType::Image);
    QMetaType::registerBuiltinType<QBrush>("QBrush", QMetaType::Brush);
    QMetaType::registerBuiltinType<QPen>("QPen", QMetaType::Pen);
    QMetaType::registerBuiltinType<QIcon>("QIcon", QMetaType::Icon);
    QMetaType::registerBuiltinType<QCursor>("QCursor", QMetaType::Cursor);
    QMetaType::registerBuiltinType<QRect>("QRect", QMetaType::Rect);
    QMetaType::registerBuiltinType<QSize>("QSize", QMetaType::Size);
    QMetaType::registerBuiltinType<QPoint>("QPoint", QMetaType::Point);
}

QVariant::QVariant(const QColor& v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Color, &v); }
QVariant::QVariant(const QFont& v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Font, &v); }
QVariant::QVariant(const QPixmap& v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Pixmap, &v); }
QVariant::QVariant(const QImage& v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Image, &v); }
QVariant::QVariant(const QIcon& v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Icon, &v); }
QVariant::QVariant(const QPoint& v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Point, &v); }
QVariant::QVariant(const QSize& v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Size, &v); }
QVariant::QVariant(const QRect& v) : type_(QMetaType::UnknownType), data_(0) { init(QMetaType::Rect, &v); }

QColor QVariant::toColor() const {
    if (type_ == QMetaType::Color && data_) return *(QColor*)data_;
    if (type_ == QMetaType::QString && data_) return QColor(*(QString*)data_);
    return QColor();
}

QFont QVariant::toFont() const {
    if (type_ == QMetaType::Font && data_) return *(QFont*)data_;
    return QFont();
}

QPixmap QVariant::toPixmap() const {
    if (type_ == QMetaType::Pixmap && data_) return *(QPixmap*)data_;
    return QPixmap();
}

QImage QVariant::toImage() const {
    if (type_ == QMetaType::Image && data_) return *(QImage*)data_;
    return QImage();
}

QIcon QVariant::toIcon() const {
    if (type_ == QMetaType::Icon && data_) return *(QIcon*)data_;
    return QIcon();
}

QPoint QVariant::toPoint() const {
    if (type_ == QMetaType::Point && data_) return *(QPoint*)data_;
    return QPoint();
}

QSize QVariant::toSize() const {
    if (type_ == QMetaType::Size && data_) return *(QSize*)data_;
    return QSize();
}

QRect QVariant::toRect() const {
    if (type_ == QMetaType::Rect && data_) return *(QRect*)data_;
    return QRect();
}

bool qtcGuiVariantToString(int type, const void* data, QString* out) {
    if (!data || !out) return false;
    switch (type) {
    case QMetaType::Color: *out = ((const QColor*)data)->name(); return true;
    case QMetaType::Point: {
        const QPoint* p = (const QPoint*)data;
        *out = QString::number(p->x()) + L"," + QString::number(p->y());
        return true;
    }
    case QMetaType::Size: {
        const QSize* s = (const QSize*)data;
        *out = QString::number(s->width()) + L"x" + QString::number(s->height());
        return true;
    }
    case QMetaType::Rect: {
        const QRect* r = (const QRect*)data;
        *out = QString::number(r->x()) + L"," + QString::number(r->y()) + L"," +
               QString::number(r->width()) + L"," + QString::number(r->height());
        return true;
    }
    case QMetaType::Font: {
        const QFont* f = (const QFont*)data;
        *out = f->family() + L" " + QString::number(f->pointSize()) + L"pt";
        return true;
    }
    default: return false;
    }
}
