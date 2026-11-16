// QtClone - qstylesheet.cpp : a practical subset of Qt style sheets.
#include "qwidget_p.h"
#include <qtc/QStyle.h>
#include <qtc/QtWidgets.h>
#include <map>
#include <vector>
#include <algorithm>

namespace {

struct Decl {
    QString name;
    QString value;
};

struct SelectorPart {
    QString type;
    QString id;
    QStringList pseudos;
};

struct Selector {
    QList<SelectorPart> parts;
    int specificity;
    Selector() : specificity(0) {}
};

struct Rule {
    QList<Selector> selectors;
    QList<Decl> decls;
    int order;
};

struct Sheet {
    QList<Rule> rules;
    bool parsed;
    Sheet() : parsed(false) {}
};

struct Cache {
    std::map<std::wstring, Sheet> sheets;
    std::map<const QWidget*, QStyleSheet::Box> boxes;
    int version;
    QString appSheet;
    Cache() : version(1) {}
};

Cache& cache() { static Cache c; return c; }

// ---------------------------------------------------------------------------
// parsing
// ---------------------------------------------------------------------------
QString stripComments(const QString& text) {
    QString out;
    int i = 0;
    while (i < text.size()) {
        if (i + 1 < text.size() && text[i] == L'/' && text[i + 1] == L'*') {
            i += 2;
            while (i + 1 < text.size() && !(text[i] == L'*' && text[i + 1] == L'/')) ++i;
            i += 2;
            continue;
        }
        out += text[i];
        ++i;
    }
    return out;
}

QString trimAll(const QString& s) { return s.trimmed(); }

Selector parseSelector(const QString& text) {
    Selector sel;
    QString normalized;
    for (int i = 0; i < text.size(); ++i) {
        wchar_t c = text[i];
        if (c == L'>' || c == L'+' || c == L'~') continue;   // treat combinators as descendant
        normalized += c;
    }
    QStringList tokens = normalized.split(L' ');
    for (int t = 0; t < tokens.count(); ++t) {
        QString tok = tokens[t].trimmed();
        if (tok.isEmpty()) continue;
        SelectorPart part;
        QString base = tok;
        int colon = base.indexOf(L':');
        while (colon >= 0) {
            int next = base.indexOf(L':', colon + 1);
            QString pseudo = next < 0 ? base.mid(colon + 1) : base.mid(colon + 1, next - colon - 1);
            if (!pseudo.isEmpty()) part.pseudos.append(pseudo);
            base = next < 0 ? base.left(colon) : base.left(colon) + base.mid(next);
            colon = base.indexOf(L':');
        }
        int hash = base.indexOf(L'#');
        if (hash >= 0) {
            part.id = base.mid(hash + 1).trimmed();
            base = base.left(hash);
        }
        part.type = base.trimmed();
        // specificity: id = 100, type = 10, pseudo = 10
        sel.specificity += part.id.isEmpty() ? 0 : 100;
        sel.specificity += part.type.isEmpty() ? 0 : 10;
        sel.specificity += part.pseudos.count() * 10;
        sel.parts.append(part);
    }
    return sel;
}

Sheet parseSheet(const QString& text) {
    Sheet sheet;
    QString cleaned = stripComments(text);
    int pos = 0;
    int order = 0;
    while (pos < cleaned.size()) {
        int open = cleaned.indexOf(L'{', pos);
        if (open < 0) break;
        int close = cleaned.indexOf(L'}', open);
        if (close < 0) break;
        QString selectorsText = cleaned.mid(pos, open - pos).trimmed();
        QString body = cleaned.mid(open + 1, close - open - 1);
        pos = close + 1;
        if (selectorsText.isEmpty()) continue;

        Rule rule;
        rule.order = order++;
        QStringList selectorTexts = selectorsText.split(L',');
        for (int i = 0; i < selectorTexts.count(); ++i) {
            QString s = selectorTexts[i].trimmed();
            if (!s.isEmpty()) rule.selectors.append(parseSelector(s));
        }
        QStringList declTexts = body.split(L';');
        for (int i = 0; i < declTexts.count(); ++i) {
            QString d = declTexts[i].trimmed();
            if (d.isEmpty()) continue;
            int colon = d.indexOf(L':');
            if (colon <= 0) continue;
            Decl decl;
            decl.name = d.left(colon).trimmed().toLower();
            decl.value = d.mid(colon + 1).trimmed();
            rule.decls.append(decl);
        }
        if (!rule.selectors.isEmpty() && !rule.decls.isEmpty()) sheet.rules.append(rule);
    }
    sheet.parsed = true;
    return sheet;
}

// ---------------------------------------------------------------------------
// matching
// ---------------------------------------------------------------------------
bool pseudoMatches(const QString& pseudo, const QWidget* widget) {
    if (pseudo == L"hover") return widget->underMouse();
    if (pseudo == L"disabled") return !widget->isEnabled();
    if (pseudo == L"enabled") return widget->isEnabled();
    if (pseudo == L"focus") return widget->hasFocus();
    if (pseudo == L"pressed") {
        const QAbstractButton* b = dynamic_cast<const QAbstractButton*>(widget);
        if (b) return b->isDown();
        return false;
    }
    if (pseudo == L"checked") {
        const QAbstractButton* b = dynamic_cast<const QAbstractButton*>(widget);
        return b ? b->isChecked() : false;
    }
    if (pseudo == L"on") {
        const QAbstractButton* b = dynamic_cast<const QAbstractButton*>(widget);
        return b ? b->isChecked() : false;
    }
    if (pseudo == L"off") {
        const QAbstractButton* b = dynamic_cast<const QAbstractButton*>(widget);
        return b ? !b->isChecked() : true;
    }
    if (pseudo == L"editable") {
        const QComboBox* c = dynamic_cast<const QComboBox*>(widget);
        return c ? c->isEditable() : false;
    }
    // unknown pseudo class: do not match
    return false;
}

bool partMatches(const SelectorPart& part, const QWidget* widget) {
    if (!widget) return false;
    if (!part.id.isEmpty() && widget->objectName() != part.id) return false;
    if (!part.type.isEmpty()) {
        std::string type = part.type.toUtf8();
        if (!widget->inherits(type.c_str())) return false;
    }
    for (int i = 0; i < part.pseudos.count(); ++i)
        if (!pseudoMatches(part.pseudos[i], widget)) return false;
    return true;
}

bool selectorMatches(const Selector& sel, const QWidget* widget) {
    if (sel.parts.isEmpty() || !widget) return false;
    int index = sel.parts.count() - 1;
    if (!partMatches(sel.parts[index], widget)) return false;
    --index;
    for (const QWidget* ancestor = widget->parentWidget(); ancestor && index >= 0;
         ancestor = ancestor->parentWidget()) {
        if (partMatches(sel.parts[index], ancestor)) --index;
    }
    return index < 0;
}

// ---------------------------------------------------------------------------
// declaration application
// ---------------------------------------------------------------------------
int lengthValue(const QString& value, bool* ok = 0) {
    QString v = value.trimmed();
    if (v.endsWith(L"px")) v = v.left(v.size() - 2);
    bool good = false;
    int n = v.toInt(&good);
    if (ok) *ok = good;
    return good ? n : 0;
}

QColor colorValue(const QString& value) {
    return QColor(value.trimmed());
}

void applyDecl(const Decl& decl, QStyleSheet::Box* box, const QWidget* widget) {
    const QString& n = decl.name;
    const QString& v = decl.value;
    if (n == L"background-color" || n == L"background") {
        box->hasBackground = true;
        box->background = colorValue(v);
    } else if (n == L"color") {
        box->hasColor = true;
        box->color = colorValue(v);
    } else if (n == L"border") {
        QStringList parts = v.split(L' ');
        int width = 1;
        QString style = L"solid";
        QColor color = QTheme::colors().border;
        for (int i = 0; i < parts.count(); ++i) {
            QString p = parts[i].trimmed();
            if (p.isEmpty()) continue;
            if (p.endsWith(L"px") || ((p[0] >= L'0' && p[0] <= L'9') && !p.startsWith(L"#"))) {
                bool ok = false;
                int w = lengthValue(p, &ok);
                if (ok) width = w;
            } else if (p == L"solid" || p == L"dashed" || p == L"dotted" || p == L"none") {
                style = p;
            } else {
                QColor c = colorValue(p);
                if (c.isValid()) color = c;
            }
        }
        box->hasBorder = true;
        box->borderWidth = style == L"none" ? 0 : width;
        box->borderColor = color;
    } else if (n == L"border-color") {
        box->hasBorder = true;
        box->borderColor = colorValue(v);
    } else if (n == L"border-width") {
        box->hasBorder = true;
        box->borderWidth = lengthValue(v);
    } else if (n == L"border-radius") {
        box->hasBorder = box->hasBorder;
        box->radius = lengthValue(v);
        if (box->radius > 0) box->hasBorder = true;
    } else if (n == L"padding") {
        QStringList parts = v.split(L' ');
        QList<int> values;
        for (int i = 0; i < parts.count(); ++i) if (!parts[i].trimmed().isEmpty()) values.append(lengthValue(parts[i]));
        box->hasPadding = true;
        if (values.count() == 1) { box->padLeft = box->padTop = box->padRight = box->padBottom = values[0]; }
        else if (values.count() == 2) { box->padTop = box->padBottom = values[0]; box->padLeft = box->padRight = values[1]; }
        else if (values.count() == 3) { box->padTop = values[0]; box->padLeft = box->padRight = values[1]; box->padBottom = values[2]; }
        else if (values.count() >= 4) { box->padTop = values[0]; box->padRight = values[1]; box->padBottom = values[2]; box->padLeft = values[3]; }
    } else if (n == L"padding-left") { box->hasPadding = true; box->padLeft = lengthValue(v); }
    else if (n == L"padding-right") { box->hasPadding = true; box->padRight = lengthValue(v); }
    else if (n == L"padding-top") { box->hasPadding = true; box->padTop = lengthValue(v); }
    else if (n == L"padding-bottom") { box->hasPadding = true; box->padBottom = lengthValue(v); }
    else if (n == L"font-family") {
        box->hasFont = true;
        box->font.setFamily(v);
    } else if (n == L"font-size") {
        bool ok = false;
        int size = lengthValue(v, &ok);
        if (ok) {
            box->hasFont = true;
            box->font = box->hasFont ? box->font : (widget ? widget->font() : QApplication::font());
            box->font.setPixelSize(size);
        }
    } else if (n == L"font-weight") {
        box->hasFont = true;
        if (!box->font.family().isEmpty()) {} else if (widget) box->font = widget->font();
        box->font.setBold(v == L"bold" || (v.toInt() >= 600));
    } else if (n == L"font-style") {
        box->hasFont = true;
        if (widget && box->font.family().isEmpty()) box->font = widget->font();
        box->font.setItalic(v == L"italic");
    } else if (n == L"min-width") {
        box->hasMinWidth = true;
        box->minWidth = lengthValue(v);
    } else if (n == L"min-height") {
        box->hasMinHeight = true;
        box->minHeight = lengthValue(v);
    } else if (n == L"max-width") {
        box->hasMaxWidth = true;
        box->maxWidth = lengthValue(v);
    } else if (n == L"max-height") {
        box->hasMaxHeight = true;
        box->maxHeight = lengthValue(v);
    } else if (n == L"selection-background-color") {
        box->hasSelection = true;
        box->selectionBackground = colorValue(v);
    } else if (n == L"selection-color") {
        box->hasSelection = true;
        box->selectionColor = colorValue(v);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// QStyleSheet
// ---------------------------------------------------------------------------
void QStyleSheet::setApplicationSheet(const QString& sheet) {
    cache().appSheet = sheet;
    invalidate();
}

const QString& QStyleSheet::applicationSheet() { return cache().appSheet; }

void QStyleSheet::invalidate() {
    cache().boxes.clear();
    ++cache().version;
}

void QStyleSheet::invalidate(const QWidget* widget) { cache().boxes.erase(widget); }

const QStyleSheet::Box& QStyleSheet::boxFor(const QWidget* widget) {
    static Box empty;
    if (!widget) return empty;
    std::map<const QWidget*, Box>::iterator it = cache().boxes.find(widget);
    if (it != cache().boxes.end()) return it->second;

    Box box;
    // collect sheets from the root down to the widget
    QList<const QWidget*> chain;
    for (const QWidget* w = widget; w; w = w->parentWidget()) chain.prepend(w);
    chain.prepend(0);   // slot for the application sheet

    struct Match {
        int specificity;
        int order;
        const Rule* rule;
    };
    std::vector<Match> matches;
    int order = 0;
    for (int c = 0; c < chain.count(); ++c) {
        const QWidget* w = chain[c];
        QString sheetText = (c == 0) ? cache().appSheet : (w ? w->styleSheet() : QString());
        if (sheetText.isEmpty()) continue;
        std::wstring key = sheetText.c_str();
        Sheet& sheet = cache().sheets[key];
        if (!sheet.parsed) sheet = parseSheet(sheetText);
        for (int r = 0; r < sheet.rules.count(); ++r) {
            const Rule& rule = sheet.rules[r];
            int best = -1;
            for (int s = 0; s < rule.selectors.count(); ++s) {
                if (selectorMatches(rule.selectors[s], widget)) {
                    best = qMax(best, rule.selectors[s].specificity);
                }
            }
            if (best >= 0) {
                Match m;
                m.specificity = best;
                m.order = order++;
                m.rule = &rule;
                matches.push_back(m);
            }
        }
    }
    for (size_t i = 1; i < matches.size(); ++i) {
        Match key = matches[i];
        size_t j = i;
        while (j > 0 && (matches[j - 1].specificity > key.specificity ||
                         (matches[j - 1].specificity == key.specificity && matches[j - 1].order > key.order))) {
            matches[j] = matches[j - 1];
            --j;
        }
        matches[j] = key;
    }
    for (size_t i = 0; i < matches.size(); ++i) {
        const Rule* rule = matches[i].rule;
        for (int d = 0; d < rule->decls.count(); ++d) applyDecl(rule->decls[d], &box, widget);
    }

    // QSS min sizes participate in geometry
    if (box.hasMinWidth || box.hasMinHeight) {
        QWidget* w = const_cast<QWidget*>(widget);
        QSize minSize = w->minimumSize();
        if (box.hasMinWidth) minSize.setWidth(box.minWidth);
        if (box.hasMinHeight) minSize.setHeight(box.minHeight);
        w->setMinimumSize(minSize);
    }
    if (box.hasMaxWidth || box.hasMaxHeight) {
        QWidget* w = const_cast<QWidget*>(widget);
        QSize maxSize = w->maximumSize();
        if (box.hasMaxWidth) maxSize.setWidth(box.maxWidth);
        if (box.hasMaxHeight) maxSize.setHeight(box.maxHeight);
        w->setMaximumSize(maxSize);
    }

    cache().boxes[widget] = box;
    return cache().boxes[widget];
}

void QStyleSheet::paintBackground(QPainter* painter, const QWidget* widget, const Box& box) {
    if (!painter || !widget) return;
    QRect r = widget->rect();
    qreal radius = box.radius;
    if (box.hasBackground && box.background.alpha() > 0) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(box.background));
        if (radius > 0) painter->drawRoundedRect(r, radius, radius);
        else painter->drawRect(r);
    }
    if (box.hasBorder && box.borderWidth > 0 && box.borderColor.alpha() > 0) {
        painter->setPen(QPen(box.borderColor, box.borderWidth));
        painter->setBrush(Qt::NoBrush);
        QRect br = r.adjusted(0, 0, -1, -1);
        if (radius > 0) painter->drawRoundedRect(br, radius, radius);
        else painter->drawRect(br);
    }
}

QColor QStyleSheet::resolveColor(const QWidget* widget, QPalette::ColorRole role) {
    if (widget) {
        const Box& box = boxFor(widget);
        if (box.hasColor) return box.color;
        return widget->palette().color(role);
    }
    return QTheme::colors().windowText;
}
