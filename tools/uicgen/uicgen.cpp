// QtClone - uicgen.cpp : generates C++ setup code from a .ui document.
#include "uicgen.h"
#include <qtc/QXml.h>
#include <qtc/QRect.h>
#include <stdio.h>

namespace {

struct WidgetInfo {
    QString className;
    QString name;
    QString parentName;
    int x, y, w, h;
    bool inLayout;
    QList<QXmlNode*> properties;
};

struct LayoutInfo {
    QString name;
    QString className;
    QString container;
    QList<QString> children;
};

QString cppString(const QString& text) {
    // UTF-8 escaped C string literal: safe for MSVC 2010 with any code page.
    std::string utf8 = text.toUtf8();
    QString out = L"QString::fromUtf8(\"";
    char buf[8];
    for (size_t i = 0; i < utf8.size(); ++i) {
        unsigned char c = (unsigned char)utf8[i];
        if (c >= 0x20 && c < 0x7f && c != '"' && c != '\\') {
            out += (wchar_t)c;
        } else {
            _snprintf_s(buf, sizeof(buf), _TRUNCATE, "\\x%02x", c);
            out += QString::fromUtf8(buf);
        }
    }
    out += L"\")";
    return out;
}

QString intText(int v) { return QString::number(v); }

QString propertyValueExpr(const QString& typeName, QXmlNode* valueNode, const QString& enumName);

QString rectExpr(QXmlNode* node) {
    QXmlNode* r = node->firstChild(L"rect");
    if (!r) return L"QRect()";
    int x = 0, y = 0, w = 0, h = 0;
    if (r->firstChild(L"x")) x = r->firstChild(L"x")->text().toInt();
    if (r->firstChild(L"y")) y = r->firstChild(L"y")->text().toInt();
    if (r->firstChild(L"width")) w = r->firstChild(L"width")->text().toInt();
    if (r->firstChild(L"height")) h = r->firstChild(L"height")->text().toInt();
    return L"QRect(" + intText(x) + L", " + intText(y) + L", " + intText(w) + L", " + intText(h) + L")";
}

QString sizeExpr(QXmlNode* node) {
    QXmlNode* s = node->firstChild(L"size");
    if (!s) return L"QSize()";
    int w = 0, h = 0;
    if (s->firstChild(L"width")) w = s->firstChild(L"width")->text().toInt();
    if (s->firstChild(L"height")) h = s->firstChild(L"height")->text().toInt();
    return L"QSize(" + intText(w) + L", " + intText(h) + L")";
}

QString pointExpr(QXmlNode* node) {
    QXmlNode* p = node->firstChild(L"point");
    if (!p) return L"QPoint()";
    int x = 0, y = 0;
    if (p->firstChild(L"x")) x = p->firstChild(L"x")->text().toInt();
    if (p->firstChild(L"y")) y = p->firstChild(L"y")->text().toInt();
    return L"QPoint(" + intText(x) + L", " + intText(y) + L")";
}

QString colorExpr(QXmlNode* node) {
    QXmlNode* c = node->firstChild(L"color");
    if (!c) return L"QColor()";
    int r = 0, g = 0, b = 0, a = 255;
    if (c->firstChild(L"red")) r = c->firstChild(L"red")->text().toInt();
    if (c->firstChild(L"green")) g = c->firstChild(L"green")->text().toInt();
    if (c->firstChild(L"blue")) b = c->firstChild(L"blue")->text().toInt();
    if (c->firstChild(L"alpha")) a = c->firstChild(L"alpha")->text().toInt();
    if (a == 255) return L"QColor(" + intText(r) + L", " + intText(g) + L", " + intText(b) + L")";
    return L"QColor(" + intText(r) + L", " + intText(g) + L", " + intText(b) + L", " + intText(a) + L")";
}

QString fontExpr(QXmlNode* node) {
    QXmlNode* f = node->firstChild(L"font");
    if (!f) return L"QFont()";
    QString family = f->firstChild(L"family") ? f->firstChild(L"family")->text() : QString(L"Microsoft YaHei UI");
    int pointSize = f->firstChild(L"pointsize") ? f->firstChild(L"pointsize")->text().toInt() : 9;
    int pixelSize = f->firstChild(L"pixelsize") ? f->firstChild(L"pixelsize")->text().toInt() : -1;
    bool bold = f->firstChild(L"weight") && f->firstChild(L"weight")->text().toInt() >= 75;
    bool italic = f->firstChild(L"italic") && f->firstChild(L"italic")->text() == L"true";
    QString expr = L"QFont(" + cppString(family) + L", " + intText(pointSize);
    if (bold) expr += L", 75";
    else if (italic) expr += L", 50, true";
    else if (italic) expr += L", 50, true";
    expr += L")";
    return expr;
}

QString stringListExpr(QXmlNode* node) {
    QList<QXmlNode*> items = node->childrenNamed(L"string");
    QString out = L"QStringList()";
    for (int i = 0; i < items.count(); ++i) out += L" << " + cppString(items[i]->text());
    return out;
}

QString propertyValueExpr(const QString& typeName, QXmlNode* valueNode, const QString&) {
    if (!valueNode) return QString();
    if (typeName == L"QString") {
        QXmlNode* s = valueNode->firstChild(L"string");
        return s ? cppString(s->text()) : cppString(QString());
    }
    if (typeName == L"bool") {
        QXmlNode* b = valueNode->firstChild(L"bool");
        return (b && b->text() == L"true") ? QString(L"true") : QString(L"false");
    }
    if (typeName == L"int" || typeName == L"uint") {
        QXmlNode* n = valueNode->firstChild(L"number");
        return n ? n->text() : QString(L"0");
    }
    if (typeName == L"double") {
        QXmlNode* n = valueNode->firstChild(L"double") ? valueNode->firstChild(L"double")
                                                       : valueNode->firstChild(L"number");
        return n ? n->text() : QString(L"0.0");
    }
    if (typeName == L"QRect") return rectExpr(valueNode);
    if (typeName == L"QSize") return sizeExpr(valueNode);
    if (typeName == L"QPoint") return pointExpr(valueNode);
    if (typeName == L"QColor") return colorExpr(valueNode);
    if (typeName == L"QFont") return fontExpr(valueNode);
    if (typeName == L"QStringList") return stringListExpr(valueNode);
    if (typeName == L"QPixmap" || typeName == L"QIcon") {
        QXmlNode* s = valueNode->firstChild(L"string");
        return s ? QString(L"QIcon(") + cppString(s->text()) + L")" : QString(L"QIcon()");
    }
    return QString();
}

void collectWidgets(QXmlNode* widgetNode, const QString& parentName, bool inLayout,
                    QList<WidgetInfo>* out, QList<LayoutInfo>* layouts) {
    if (!widgetNode) return;
    WidgetInfo info;
    info.className = widgetNode->attribute(L"class");
    info.name = widgetNode->attribute(L"name");
    info.parentName = parentName;
    info.inLayout = inLayout;
    info.x = info.y = 0;
    info.w = 100;
    info.h = 30;
    QList<QXmlNode*> props = widgetNode->childrenNamed(L"property");
    for (int i = 0; i < props.count(); ++i) {
        QString propName = props[i]->attribute(L"name");
        if (propName == L"geometry") {
            QXmlNode* r = props[i]->firstChild(L"rect");
            if (r) {
                if (r->firstChild(L"x")) info.x = r->firstChild(L"x")->text().toInt();
                if (r->firstChild(L"y")) info.y = r->firstChild(L"y")->text().toInt();
                if (r->firstChild(L"width")) info.w = r->firstChild(L"width")->text().toInt();
                if (r->firstChild(L"height")) info.h = r->firstChild(L"height")->text().toInt();
            }
        }
        info.properties.append(props[i]);
    }
    out->append(info);

    QList<QXmlNode*> layoutNodes = widgetNode->childrenNamed(L"layout");
    if (layoutNodes.count() > 0 && layouts) {
        LayoutInfo li;
        li.className = layoutNodes[0]->attribute(L"class");
        li.container = info.name;
        li.name = layoutNodes[0]->attribute(L"name");
        if (li.name.isEmpty()) li.name = info.name + L"Layout";
        QList<QXmlNode*> items = layoutNodes[0]->childrenNamed(L"item");
        for (int i = 0; i < items.count(); ++i) {
            QList<QXmlNode*> ws = items[i]->childrenNamed(L"widget");
            for (int k = 0; k < ws.count(); ++k) {
                li.children.append(ws[k]->attribute(L"name"));
                collectWidgets(ws[k], info.name, true, out, layouts);
            }
        }
        layouts->append(li);
    }

    QList<QXmlNode*> children = widgetNode->childrenNamed(L"widget");
    for (int i = 0; i < children.count(); ++i)
        collectWidgets(children[i], info.name, false, out, layouts);
}

} // namespace

QString uicGenerateHeader(const QString& uiXml, const QString& className, QString* error) {
    QXmlNode* root = QXmlNode::parse(uiXml, error);
    if (!root) return QString();
    if (root->tagName() != L"ui") {
        if (error) *error = L"root element is not <ui>";
        delete root;
        return QString();
    }
    QString formClass = className;
    if (formClass.isEmpty()) {
        QXmlNode* classNode = root->firstChild(L"class");
        if (classNode) formClass = classNode->text().trimmed();
    }
    if (formClass.isEmpty()) formClass = L"Form1";

    QXmlNode* topWidget = root->firstChild(L"widget");
    if (!topWidget) {
        if (error) *error = L"no top level <widget>";
        delete root;
        return QString();
    }
    QList<WidgetInfo> widgets;
    QList<LayoutInfo> layouts;
    collectWidgets(topWidget, QString(), false, &widgets, &layouts);

    QString includeGuard = L"UI_" + formClass.toUpper() + L"_H";
    QString out;
    out += L"// Generated by qtcuic (QtClone). Do not edit - re-run the generator instead.\n";
    out += L"#ifndef " + includeGuard + L"\n#define " + includeGuard + L"\n\n";
    out += L"#include <qtc/QtWidgets.h>\n#include <qtc/QStyle.h>\n\n";
    out += L"class Ui_" + formClass + L" {\npublic:\n";
    for (int i = 1; i < widgets.count(); ++i)
        out += L"    " + widgets[i].className + L"* " + widgets[i].name + L";\n";
    out += L"\n    void setupUi(QWidget* " + formClass + L") {\n";
    out += L"        if (" + formClass + L"->objectName().isEmpty())\n";
    out += L"            " + formClass + L"->setObjectName(QString::fromUtf8(\"" + formClass + L"\"));\n";
    out += L"        " + formClass + L"->resize(" + intText(widgets[0].w) + L", " + intText(widgets[0].h) + L");\n";

    // create widgets in document order (parents before children)
    for (int i = 1; i < widgets.count(); ++i) {
        const WidgetInfo& w = widgets[i];
        QString parentExpr = w.parentName.isEmpty() ? formClass : w.parentName;
        out += L"\n        " + w.name + L" = new " + w.className + L"(" + parentExpr + L");\n";
        out += L"        " + w.name + L"->setObjectName(QString::fromUtf8(\"" + w.name + L"\"));\n";
        if (!w.inLayout) {
            out += L"        " + w.name + L"->setGeometry(QRect(" + intText(w.x) + L", " + intText(w.y) + L", " +
                   intText(w.w) + L", " + intText(w.h) + L"));\n";
        }
    }

    // layouts are created after their children exist
    for (int i = 0; i < layouts.count(); ++i) {
        const LayoutInfo& l = layouts[i];
        QString container = l.container.isEmpty() ? formClass : l.container;
        out += L"\n        " + l.className + L"* " + l.name + L" = new " + l.className + L"(" + container + L");\n";
        for (int c = 0; c < l.children.count(); ++c)
            out += L"        " + l.name + L"->addWidget(" + l.children[c] + L");\n";
    }

    // properties (strings live in retranslateUi)
    for (int i = 1; i < widgets.count(); ++i) {
        const WidgetInfo& w = widgets[i];
        for (int p = 0; p < w.properties.count(); ++p) {
            QString propName = w.properties[p]->attribute(L"name");
            if (propName == L"geometry") continue;
            QXmlNode* valueNode = 0;
            QList<QXmlNode*> kids = w.properties[p]->children();
            for (int k = 0; k < kids.count(); ++k) {
                if (kids[k]->tagName() != L"property") { valueNode = kids[k]; break; }
            }
            QString typeName = w.properties[p]->attribute(L"stdset");
            Q_UNUSED(typeName);
            QString valueTag = valueNode ? valueNode->tagName() : QString();
            QString cppExpr;
            if (valueTag == L"string") cppExpr = cppString(valueNode->text());
            else if (valueTag == L"bool") cppExpr = (valueNode->text() == L"true") ? QString(L"true") : QString(L"false");
            else if (valueTag == L"number") cppExpr = valueNode->text();
            else if (valueTag == L"double") cppExpr = valueNode->text();
            else if (valueTag == L"rect") cppExpr = rectExpr(w.properties[p]);
            else if (valueTag == L"size") cppExpr = sizeExpr(w.properties[p]);
            else if (valueTag == L"point") cppExpr = pointExpr(w.properties[p]);
            else if (valueTag == L"color") cppExpr = colorExpr(w.properties[p]);
            else if (valueTag == L"font") cppExpr = fontExpr(w.properties[p]);
            else if (valueTag == L"stringlist") cppExpr = stringListExpr(w.properties[p]);
            if (cppExpr.isEmpty()) continue;
            if (!valueTag.isEmpty() && (valueTag == L"string" || valueTag == L"stringlist")) continue;
            out += L"        " + w.name + L"->setProperty(\"" + propName + L"\", QVariant(" + cppExpr + L"));\n";
        }
    }

    out += L"\n        retranslateUi(" + formClass + L");\n";
    out += L"    }\n\n";

    out += L"    void retranslateUi(QWidget* " + formClass + L") {\n";
    out += L"        Q_UNUSED(" + formClass + L");\n";
    for (int i = 0; i < widgets.count(); ++i) {
        const WidgetInfo& w = widgets[i];
        for (int p = 0; p < w.properties.count(); ++p) {
            QXmlNode* s = w.properties[p]->firstChild(L"string");
            if (!s) continue;
            QString propName = w.properties[p]->attribute(L"name");
            out += L"        " + w.name + L"->setProperty(\"" + propName + L"\", QVariant(" +
                   cppString(s->text()) + L"));\n";
        }
    }
    out += L"    }\n};\n\n";
    out += L"namespace Ui { class " + formClass + L" : public Ui_" + formClass + L" {}; }\n\n";
    out += L"#endif // " + includeGuard + L"\n";
    delete root;
    return out;
}

QString uicReadFile(const QString& path, bool* ok) {
    FILE* f = _wfopen(path.c_str(), L"rb");
    if (!f) {
        if (ok) *ok = false;
        return QString();
    }
    std::string bytes;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) bytes.append(buf, n);
    fclose(f);
    if (ok) *ok = true;
    if (bytes.size() >= 3 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB &&
        (unsigned char)bytes[2] == 0xBF) {
        bytes = bytes.substr(3);
    }
    return QString::fromUtf8(bytes.c_str());
}

bool uicWriteFile(const QString& path, const QString& text, bool withBom) {
    FILE* f = _wfopen(path.c_str(), L"wb");
    if (!f) return false;
    if (withBom) {
        const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
        fwrite(bom, 1, 3, f);
    }
    std::string bytes = text.toUtf8();
    if (!bytes.empty()) fwrite(bytes.data(), 1, bytes.size(), f);
    fclose(f);
    return true;
}
