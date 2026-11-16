// QtClone - qstyle.cpp : the built-in "Fusion" style.
#include "qwidget_p.h"
#include <qtc/QStyle.h>
#include <qtc/QtWidgets.h>
#include <math.h>

namespace {

QColor withAlpha(const QColor& c, int alpha) {
    QColor r(c);
    r.setAlpha(alpha);
    return r;
}

bool isDisabled(const QStyleOption& opt) {
    return (opt.state & QStyle::State_Enabled) == 0;
}

QColor blend(const QColor& a, const QColor& b, qreal t) {
    return QColor(qRound(a.red() * (1 - t) + b.red() * t),
                  qRound(a.green() * (1 - t) + b.green() * t),
                  qRound(a.blue() * (1 - t) + b.blue() * t),
                  qRound(a.alpha() * (1 - t) + b.alpha() * t));
}

void roundedFill(QPainter* p, const QRect& r, qreal radius, const QColor& fill,
                 const QColor& border, int borderWidth) {
    if (r.isEmpty()) return;
    p->setBrush(QBrush(fill));
    if (borderWidth > 0 && border.alpha() > 0) p->setPen(QPen(border, borderWidth));
    else p->setPen(Qt::NoPen);
    if (radius > 0) p->drawRoundedRect(r.adjusted(0, 0, -1, -1), radius, radius);
    else if (borderWidth > 0) p->drawRect(r.adjusted(0, 0, -1, -1));
    else { p->setPen(Qt::NoPen); p->drawRect(r); }
}

} // namespace

QStyle::QStyle() {}
QStyle::~QStyle() {}

QStyle* QStyle::createDefault() { return new QStyle(); }

void QStyle::polish(QWidget* widget) { Q_UNUSED(widget); }
void QStyle::unpolish(QWidget* widget) { Q_UNUSED(widget); }

QSize QStyle::sizeFromContents(const QStyleOptionButton& opt, const QSize& textSize) const {
    QSize s = textSize;
    s.setWidth(s.width() + textHMargin() * 2 + 4);
    s.setHeight(qMax(24, s.height() + textVMargin() * 2 + 6));
    return s;
}

void QStyle::drawButton(QPainter* p, const QStyleOptionButton& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    const QStyleSheet::Box& box = QStyleSheet::boxFor(w);
    bool enabled = !isDisabled(opt);
    bool hover = (opt.state & State_Hover) != 0;
    bool down = opt.down || (opt.state & State_Sunken) != 0;
    bool checked = opt.checked || (opt.state & State_On) != 0;
    bool focus = (opt.state & State_HasFocus) != 0;

    QColor fill = box.hasBackground ? box.background : c.button;
    QColor border = box.hasBorder ? box.borderColor : c.border;
    QColor text = box.hasColor ? box.color : c.buttonText;
    qreal radius = box.hasBorder ? box.radius : 6;

    if (!enabled) {
        fill = blend(c.button, c.window, 0.5);
        text = c.disabledText;
        border = c.border;
    } else if (checked) {
        fill = c.accent;
        border = c.accent;
        text = c.accentText;
        if (!box.hasBackground) {
            fill = hover ? c.accentHover : c.accent;
        }
    } else if (down) {
        fill = c.buttonPressed;
        border = hover ? c.borderStrong : c.border;
    } else if (hover) {
        fill = c.buttonHover;
        border = c.borderStrong;
    }
    if (opt.defaultButton && enabled && !checked) border = c.accent;

    bool flatLook = opt.flat && !box.hasBackground && !box.hasBorder;
    if (flatLook) {
        if (down || checked) {
            roundedFill(p, opt.rect, 5, withAlpha(c.accent, 32), withAlpha(c.accent, 32), 1);
        } else if (hover) {
            roundedFill(p, opt.rect, 5, blend(c.window, c.buttonHover, 0.6),
                        blend(c.window, c.buttonHover, 0.6), 1);
        }
    } else {
        if (opt.state & State_Default) {
            p->setPen(QPen(withAlpha(c.accent, 80), 2));
            p->setBrush(Qt::NoBrush);
            p->drawRoundedRect(opt.rect.adjusted(1, 1, -2, -2), radius + 1, radius + 1);
        }

        roundedFill(p, opt.rect, radius, fill, border, 1);
    }

    if (opt.hasIcon) {
        QSize is(16, 16);
        QPixmap pm = opt.icon.pixmap(is);
        int tw = qtcTextWidth(w ? w->font() : QApplication::font(), opt.text);
        int totalW = pm.width() + (opt.text.isEmpty() ? 0 : 6) + tw;
        int x = opt.rect.left() + (opt.rect.width() - totalW) / 2;
        int y = opt.rect.top() + (opt.rect.height() - pm.height()) / 2;
        p->drawPixmap(QPoint(x, y), pm);
        x += pm.width() + 6;
        QRect tr(x, opt.rect.top(), tw + 4, opt.rect.height());
        p->setPen(QPen(text));
        p->setFont(w ? w->font() : QApplication::font());
        p->drawText(tr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, opt.text);
    } else {
        p->setPen(QPen(text));
        p->setFont(w ? w->font() : QApplication::font());
        p->drawText(opt.rect, Qt::AlignCenter | Qt::TextShowMnemonic, opt.text);
    }

    if (focus && enabled) drawFocusRect(p, opt.rect.adjusted(3, 3, -3, -3), w);
}

void QStyle::drawCheckBox(QPainter* p, const QStyleOptionButton& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    const QStyleSheet::Box& box = QStyleSheet::boxFor(w);
    bool enabled = !isDisabled(opt);
    bool hover = (opt.state & State_Hover) != 0;
    bool checked = opt.checked || (opt.state & State_On) != 0;
    bool partial = (opt.state & State_NoChange) != 0;
    bool focus = (opt.state & State_HasFocus) != 0;

    int size = 16;
    QRect boxRect(opt.rect.left() + 1, opt.rect.top() + (opt.rect.height() - size) / 2, size, size);
    QColor bg = checked ? c.accent : c.base;
    QColor border = checked ? c.accent : (hover ? c.borderStrong : c.border);
    QColor mark = c.accentText;
    if (!enabled) {
        bg = blend(checked ? c.accent : c.base, c.window, 0.6);
        border = c.border;
        mark = c.disabledText;
    }
    roundedFill(p, boxRect, 4, bg, border, 1);
    if (checked) drawCheckMark(p, boxRect.adjusted(2, 2, -2, -2), mark);
    else if (partial) {
        p->setPen(QPen(mark, 2));
        p->drawLine(boxRect.left() + 4, boxRect.center().y(),
                    boxRect.right() - 3, boxRect.center().y());
    }
    if (focus) drawFocusRect(p, boxRect.adjusted(-2, -2, 2, 2), w);

    QRect textRect = opt.rect.adjusted(size + 8, 0, 0, 0);
    QColor textColor = box.hasColor ? box.color : (enabled ? c.windowText : c.disabledText);
    p->setPen(QPen(textColor));
    p->setFont(w ? w->font() : QApplication::font());
    p->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, opt.text);
}

void QStyle::drawRadioButton(QPainter* p, const QStyleOptionButton& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    const QStyleSheet::Box& box = QStyleSheet::boxFor(w);
    bool enabled = !isDisabled(opt);
    bool hover = (opt.state & State_Hover) != 0;
    bool checked = opt.checked || (opt.state & State_On) != 0;
    bool focus = (opt.state & State_HasFocus) != 0;

    int size = 16;
    QRect outer(opt.rect.left() + 1, opt.rect.top() + (opt.rect.height() - size) / 2, size, size);
    QColor border = checked ? c.accent : (hover ? c.borderStrong : c.border);
    if (!enabled) border = c.border;
    p->setPen(QPen(border, 1));
    p->setBrush(QBrush(checked ? c.accent : c.base));
    p->drawEllipse(outer);
    if (checked) {
        QRect inner = outer.adjusted(4, 4, -4, -4);
        p->setPen(Qt::NoPen);
        p->setBrush(QBrush(enabled ? c.accentText : c.disabledText));
        p->drawEllipse(inner);
    }
    if (focus) drawFocusRect(p, outer.adjusted(-2, -2, 2, 2), w);

    QRect textRect = opt.rect.adjusted(size + 8, 0, 0, 0);
    QColor textColor = box.hasColor ? box.color : (enabled ? c.windowText : c.disabledText);
    p->setPen(QPen(textColor));
    p->setFont(w ? w->font() : QApplication::font());
    p->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, opt.text);
}

void QStyle::drawToolButton(QPainter* p, const QStyleOptionToolButton& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    bool enabled = !isDisabled(opt);
    bool hover = (opt.state & State_Hover) != 0;
    bool down = opt.down || (opt.state & State_Sunken) != 0;
    if (hover || down) {
        QColor bg = down ? c.buttonPressed : c.buttonHover;
        roundedFill(p, opt.rect, 5, bg, c.border, 1);
    }
    QColor text = enabled ? (opt.state & State_On ? c.accent : c.windowText) : c.disabledText;
    p->setPen(QPen(text));
    p->setFont(w ? w->font() : QApplication::font());
    if (opt.toolStyle == Qt::ToolButtonTextOnly) {
        p->drawText(opt.rect, Qt::AlignCenter | Qt::TextShowMnemonic, opt.text);
    } else if (opt.toolStyle == Qt::ToolButtonIconOnly) {
        if (opt.hasIcon) {
            QPixmap pm = opt.icon.pixmap(QSize(20, 20));
            p->drawPixmap(QPoint(opt.rect.left() + (opt.rect.width() - pm.width()) / 2,
                                 opt.rect.top() + (opt.rect.height() - pm.height()) / 2), pm);
        } else {
            p->drawText(opt.rect, Qt::AlignCenter, opt.text);
        }
    } else if (opt.toolStyle == Qt::ToolButtonTextUnderIcon) {
        p->drawText(opt.rect.adjusted(0, 2, 0, -2), Qt::AlignHCenter | Qt::AlignTop | Qt::TextShowMnemonic, opt.text);
    } else {
        p->drawText(opt.rect.adjusted(4, 0, -4, 0), Qt::AlignCenter | Qt::TextShowMnemonic, opt.text);
    }
}

void QStyle::drawLineEditFrame(QPainter* p, const QStyleOptionFrame& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    const QStyleSheet::Box& box = QStyleSheet::boxFor(w);
    bool enabled = !isDisabled(opt);
    bool focus = (opt.state & State_HasFocus) != 0;
    qreal radius = box.hasBorder ? box.radius : 5;
    QColor bg = box.hasBackground ? box.background : (enabled ? c.base : blend(c.base, c.window, 0.5));
    QColor border = box.hasBorder ? box.borderColor : (focus ? c.accent : c.border);
    int bw = focus ? 2 : (box.hasBorder ? box.borderWidth : 1);
    roundedFill(p, opt.rect, radius, bg, border, bw);
}

void QStyle::drawFrame(QPainter* p, const QStyleOptionFrame& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    QColor border = c.border;
    p->setPen(QPen(border, opt.lineWidth > 0 ? opt.lineWidth : 1));
    p->setBrush(Qt::NoBrush);
    if (opt.rounded) p->drawRoundedRect(opt.rect.adjusted(0, 0, -1, -1), 5, 5);
    else p->drawRect(opt.rect.adjusted(0, 0, -1, -1));
    Q_UNUSED(w);
}

void QStyle::drawPanel(QPainter* p, const QStyleOption& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    const QStyleSheet::Box& box = QStyleSheet::boxFor(w);
    QColor fill = box.hasBackground ? box.background : c.surface;
    QColor border = box.hasBorder ? box.borderColor : c.border;
    roundedFill(p, opt.rect, box.hasBorder ? box.radius : 6, fill, border, box.hasBorder ? box.borderWidth : 1);
}

void QStyle::drawGroupBox(QPainter* p, const QStyleOptionGroupBox& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    QFont f = w ? w->font() : QApplication::font();
    f.setBold(true);
    int th = QFontMetrics(f).height() + 4;
    QRect frame = opt.rect.adjusted(0, th / 2, -1, -1);
    QColor border = c.border;
    p->setPen(QPen(border, 1));
    p->setBrush(Qt::NoBrush);
    p->drawRoundedRect(frame, 6, 6);
    if (!opt.text.isEmpty()) {
        QFontMetrics fm(f);
        int tw = fm.width(opt.text) + 12;
        QRect titleRect(opt.rect.left() + 10, opt.rect.top(), tw, th);
        p->fillRect(QRect(titleRect.left() - 2, titleRect.top(), titleRect.width(), th), c.window);
        p->setPen(QPen(c.windowText));
        p->setFont(f);
        p->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, opt.text);
    }
}

void QStyle::drawProgressBar(QPainter* p, const QStyleOptionProgressBar& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    const QStyleSheet::Box& box = QStyleSheet::boxFor(w);
    QColor groove = blend(c.window, c.border, 0.5);
    qreal radius = qMin(8, qMin(opt.rect.height(), opt.rect.width()) / 2);
    roundedFill(p, opt.rect, radius, groove, QColor(0, 0, 0, 0), 0);

    int range = opt.maximum - opt.minimum;
    int value = opt.value - opt.minimum;
    if (range <= 0) range = 1;
    if (value < 0) value = 0;
    if (value > range) value = range;
    int extent = opt.vertical ? opt.rect.height() : opt.rect.width();
    int filled = extent * value / range;
    if (filled > 0) {
        QRect chunk = opt.vertical
            ? QRect(opt.rect.left(), opt.rect.bottom() - filled + 1, opt.rect.width(), filled)
            : QRect(opt.rect.left(), opt.rect.top(), filled, opt.rect.height());
        roundedFill(p, chunk, radius, box.hasBackground ? box.background : c.accent, QColor(0, 0, 0, 0), 0);
    }
    if (opt.textVisible) {
        p->setPen(QPen(box.hasColor ? box.color : c.windowText));
        p->setFont(w ? w->font() : QApplication::font());
        p->drawText(opt.rect, Qt::AlignCenter, opt.text);
    }
}

void QStyle::drawSlider(QPainter* p, const QStyleOptionSlider& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    bool enabled = !isDisabled(opt);
    bool horizontal = opt.orientation == Qt::Horizontal;
    int range = opt.maximum - opt.minimum;
    if (range <= 0) range = 1;
    int value = qBound(0, opt.value - opt.minimum, range);

    int grooveThickness = 4;
    QRect groove = horizontal
        ? QRect(opt.rect.left() + 6, opt.rect.center().y() - grooveThickness / 2,
                opt.rect.width() - 12, grooveThickness)
        : QRect(opt.rect.center().x() - grooveThickness / 2, opt.rect.top() + 6,
                grooveThickness, opt.rect.height() - 12);
    QColor grooveColor = blend(c.window, c.border, 0.7);
    roundedFill(p, groove, 2, grooveColor, QColor(0, 0, 0, 0), 0);

    int handleW = 14;
    int handleH = 14;
    int pos;
    QRect filled;
    if (horizontal) {
        pos = groove.left() + (groove.width() - handleW) * value / range;
        filled = QRect(groove.left(), groove.top(), pos - groove.left() + handleW / 2, groove.height());
    } else {
        pos = groove.bottom() - (groove.height() - handleH) * value / range - handleH + 1;
        filled = QRect(groove.left(), pos + handleH / 2, groove.width(), groove.bottom() - pos);
    }
    if (enabled) roundedFill(p, filled, 2, c.accent, QColor(0, 0, 0, 0), 0);

    QRect handle = horizontal
        ? QRect(pos, opt.rect.center().y() - handleH / 2, handleW, handleH)
        : QRect(opt.rect.center().x() - handleW / 2, pos, handleW, handleH);
    QColor fill = enabled ? c.surface : blend(c.surface, c.window, 0.5);
    if ((opt.state & State_Hover) && enabled) fill = c.surfaceAlt;
    QColor border = enabled ? c.borderStrong : c.border;
    p->setPen(QPen(border, 1));
    p->setBrush(QBrush(fill));
    p->drawRoundedRect(handle.adjusted(0, 0, -1, -1), 4, 4);
    if (opt.state & State_HasFocus) {
        p->setPen(QPen(c.accent, 1));
        p->setBrush(Qt::NoBrush);
        p->drawRoundedRect(handle.adjusted(1, 1, -2, -2), 3, 3);
    }
}

void QStyle::drawScrollBar(QPainter* p, const QStyleOptionScrollBar& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    bool horizontal = opt.orientation == Qt::Horizontal;
    roundedFill(p, opt.rect, 0, c.scrollbar, QColor(0, 0, 0, 0), 0);

    int range = opt.maximum - opt.minimum;
    if (range <= 0) range = 1;
    int extent = horizontal ? opt.rect.width() : opt.rect.height();
    int thumb = qMax(20, extent * opt.pageStep / qMax(1, range + opt.pageStep));
    int travel = extent - thumb;
    int pos = opt.rect.left();
    if (horizontal) {
        pos = opt.rect.left() + travel * (opt.value - opt.minimum) / range;
    } else {
        pos = opt.rect.top() + travel * (opt.value - opt.minimum) / range;
    }
    QRect thumbRect = horizontal
        ? QRect(pos, opt.rect.top() + 2, thumb, opt.rect.height() - 4)
        : QRect(opt.rect.left() + 2, pos, opt.rect.width() - 4, thumb);
    QColor thumbColor = c.scrollbarHandle;
    if (opt.pressedPart == 3) thumbColor = c.borderStrong;
    else if (opt.hoverPart == 3) thumbColor = blend(c.scrollbarHandle, c.borderStrong, 0.5);
    roundedFill(p, thumbRect, (horizontal ? thumbRect.height() : thumbRect.width()) / 2,
                thumbColor, QColor(0, 0, 0, 0), 0);
    Q_UNUSED(w);
}

void QStyle::drawComboBox(QPainter* p, const QStyleOptionComboBox& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    const QStyleSheet::Box& box = QStyleSheet::boxFor(w);
    bool enabled = !isDisabled(opt);
    bool focus = (opt.state & State_HasFocus) != 0;
    qreal radius = box.hasBorder ? box.radius : 5;
    QColor bg = box.hasBackground ? box.background : (enabled ? c.base : blend(c.base, c.window, 0.5));
    QColor border = box.hasBorder ? box.borderColor : (focus || opt.open ? c.accent : c.border);
    roundedFill(p, opt.rect, radius, bg, border, 1);

    int arrowW = 22;
    QRect textRect = opt.rect.adjusted(10, 0, -arrowW, 0);
    p->setPen(QPen(enabled ? (box.hasColor ? box.color : c.windowText) : c.disabledText));
    p->setFont(w ? w->font() : QApplication::font());
    p->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, opt.text);

    QRect arrowRect(opt.rect.right() - arrowW, opt.rect.top(), arrowW - 1, opt.rect.height());
    p->fillRect(arrowRect, bg);
    p->setPen(QPen(c.border, 1));
    p->drawLine(arrowRect.left(), arrowRect.top() + 4, arrowRect.left(), arrowRect.bottom() - 4);
    drawArrow(p, arrowRect.adjusted(4, 4, -4, -4), Qt::DownArrow, enabled ? c.windowText : c.disabledText);
}

void QStyle::drawTab(QPainter* p, const QStyleOptionTab& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    const QStyleSheet::Box& box = QStyleSheet::boxFor(w);
    QColor bg = opt.selected ? (box.hasBackground ? box.background : c.surface)
                             : (opt.hovered ? c.surfaceAlt : blend(c.window, c.surface, 0.5));
    QColor border = opt.selected ? c.border : blend(c.window, c.border, 0.5);
    QRect r = opt.rect;
    if (opt.selected) r.setBottom(r.bottom() + 1);
    p->setPen(QPen(border, 1));
    p->setBrush(QBrush(bg));
    QList<QPoint> pts;
    int radius = 4;
    pts.append(QPoint(r.left(), r.bottom()));
    pts.append(QPoint(r.left(), r.top() + radius));
    pts.append(QPoint(r.left() + radius, r.top()));
    pts.append(QPoint(r.right() - radius, r.top()));
    pts.append(QPoint(r.right(), r.top() + radius));
    pts.append(QPoint(r.right(), r.bottom()));
    p->drawPolygon(pts);
    p->setPen(QPen(opt.selected ? c.windowText : c.disabledText));
    p->setFont(w ? w->font() : QApplication::font());
    p->drawText(opt.rect.adjusted(10, 0, -10, 0), Qt::AlignCenter, opt.text);
}

void QStyle::drawHeader(QPainter* p, const QStyleOptionHeader& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    QColor bg = opt.pressed ? c.buttonPressed : (opt.hovered ? c.surfaceAlt : c.header);
    p->fillRect(opt.rect, bg);
    p->setPen(QPen(c.gridLine, 1));
    p->drawLine(opt.rect.right(), opt.rect.top() + 2, opt.rect.right(), opt.rect.bottom() - 2);
    p->drawLine(opt.rect.left(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
    QRect textRect = opt.rect.adjusted(6, 0, -6, 0);
    p->setPen(QPen(c.headerText));
    p->setFont(w ? w->font() : QApplication::font());
    p->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, opt.text);
    if (opt.sortIndicator) {
        drawArrow(p, QRect(opt.rect.right() - 16, opt.rect.center().y() - 4, 8, 8),
                  opt.sortIndicator == 1 ? Qt::DownArrow : Qt::UpArrow, c.headerText);
    }
}

void QStyle::drawMenuItem(QPainter* p, const QStyleOptionMenuItem& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    if (opt.separator) {
        int y = opt.rect.center().y();
        p->setPen(QPen(c.gridLine, 1));
        p->drawLine(opt.rect.left() + 8, y, opt.rect.right() - 8, y);
        return;
    }
    if (opt.active && opt.enabled) {
        roundedFill(p, opt.rect.adjusted(4, 1, -4, -1), 4, c.accent, QColor(0, 0, 0, 0), 0);
    }
    QColor text = opt.enabled ? (opt.active ? c.accentText : c.windowText) : c.disabledText;
    int left = opt.rect.left() + 10;
    if (opt.checkable) {
        QRect checkRect(opt.rect.left() + 8, opt.rect.center().y() - 6, 12, 12);
        if (opt.checked) drawCheckMark(p, checkRect, opt.active ? c.accentText : c.accent);
        left = opt.rect.left() + 28;
    }
    if (opt.iconVisible) left += 20;
    QRect textRect(left, opt.rect.top(), opt.rect.width() - left - 10, opt.rect.height());
    p->setPen(QPen(text));
    p->setFont(w ? w->font() : QApplication::font());
    p->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, opt.text);
    if (!opt.shortcut.isEmpty()) {
        QRect scRect(opt.rect.left(), opt.rect.top(), opt.rect.width() - 12, opt.rect.height());
        QFont f = w ? w->font() : QApplication::font();
        f.setPixelSize(qMax(10, f.pixelHeight() - 1));
        p->setFont(f);
        p->setPen(QPen(opt.active ? withAlpha(c.accentText, 200) : c.disabledText));
        p->drawText(scRect, Qt::AlignRight | Qt::AlignVCenter, opt.shortcut);
    }
    if (opt.subMenu) {
        drawArrow(p, QRect(opt.rect.right() - 18, opt.rect.center().y() - 4, 8, 8),
                  Qt::RightArrow, text);
    }
}

void QStyle::drawViewItem(QPainter* p, const QStyleOptionViewItem& opt, const QWidget* w) const {
    const QTheme::Colors& c = QTheme::colors();
    if (opt.selected) {
        p->fillRect(opt.rect, c.selection);
    } else if (opt.hovered) {
        p->fillRect(opt.rect, c.surfaceAlt);
    } else if (opt.alternate) {
        p->fillRect(opt.rect, c.alternateBase);
    }
    QRect textRect = opt.rect.adjusted(6, 0, -6, 0);
    if (opt.hasIcon) {
        QPixmap pm = opt.icon.pixmap(QSize(16, 16));
        p->drawPixmap(QPoint(textRect.left(), textRect.center().y() - 8), pm);
        textRect.setLeft(textRect.left() + 22);
    }
    if (opt.checkState) {
        QRect checkRect(textRect.left(), textRect.center().y() - 7, 14, 14);
        roundedFill(p, checkRect, 3, opt.checkState == 1 ? c.accent : c.base, c.border, 1);
        if (opt.checkState == 1) drawCheckMark(p, checkRect.adjusted(2, 2, -2, -2), c.accentText);
        textRect.setLeft(textRect.left() + 20);
    }
    p->setPen(QPen(opt.selected ? c.selectionText : c.windowText));
    p->setFont(w ? w->font() : QApplication::font());
    int flags = Qt::AlignVCenter | Qt::AlignLeft;
    if (opt.alignment & Qt::AlignHCenter) flags = Qt::AlignVCenter | Qt::AlignHCenter;
    else if (opt.alignment & Qt::AlignRight) flags = Qt::AlignVCenter | Qt::AlignRight;
    p->drawText(textRect, flags, opt.text);
}

void QStyle::drawFocusRect(QPainter* p, const QRect& rect, const QWidget*) const {
    const QTheme::Colors& c = QTheme::colors();
    p->setPen(QPen(withAlpha(c.accent, 170), 1, Qt::DotLine));
    p->setBrush(Qt::NoBrush);
    p->drawRect(rect.adjusted(0, 0, -1, -1));
}

void QStyle::drawToolTip(QPainter* p, const QStyleOption& opt, const QWidget*) const {
    const QTheme::Colors& c = QTheme::colors();
    roundedFill(p, opt.rect, 4, c.toolTipBase, c.borderStrong, 1);
}

void QStyle::drawArrow(QPainter* p, const QRect& rect, Qt::ArrowType type, const QColor& color) const {
    int cx = rect.center().x();
    int cy = rect.center().y();
    int s = qMin(rect.width(), rect.height()) / 2;
    QList<QPoint> pts;
    switch (type) {
    case Qt::UpArrow:
        pts.append(QPoint(cx, cy - s));
        pts.append(QPoint(cx + s, cy + s));
        pts.append(QPoint(cx - s, cy + s));
        break;
    case Qt::DownArrow:
        pts.append(QPoint(cx - s, cy - s));
        pts.append(QPoint(cx + s, cy - s));
        pts.append(QPoint(cx, cy + s));
        break;
    case Qt::LeftArrow:
        pts.append(QPoint(cx - s, cy));
        pts.append(QPoint(cx + s, cy - s));
        pts.append(QPoint(cx + s, cy + s));
        break;
    default:
        pts.append(QPoint(cx + s, cy));
        pts.append(QPoint(cx - s, cy - s));
        pts.append(QPoint(cx - s, cy + s));
        break;
    }
    p->setPen(Qt::NoPen);
    p->setBrush(QBrush(color));
    p->drawPolygon(pts);
}

void QStyle::drawCheckMark(QPainter* p, const QRect& rect, const QColor& color) const {
    qreal x = rect.left();
    qreal y = rect.top();
    qreal w = rect.width();
    qreal h = rect.height();
    QList<QPoint> pts;
    pts.append(QPoint(qRound(x + w * 0.18), qRound(y + h * 0.52)));
    pts.append(QPoint(qRound(x + w * 0.42), qRound(y + h * 0.76)));
    pts.append(QPoint(qRound(x + w * 0.84), qRound(y + h * 0.24)));
    p->setPen(QPen(color, qMax(2.0, h / 8.0)));
    p->drawPolyline(pts);
}

void QStyle::drawCopyIcon(QPainter* p, const QRect& rect, const QColor& color) const {
    p->setPen(QPen(color, 1));
    p->setBrush(Qt::NoBrush);
    p->drawRect(rect.adjusted(1, 1, -3, -3));
    p->drawRect(rect.adjusted(3, 3, -1, -1));
}
