// QtClone - qxml.cpp : minimal but strict XML reader/writer.
#include <qtc/QXml.h>
#include <stdio.h>

QXmlNode::QXmlNode() : parent_(0) {}
QXmlNode::QXmlNode(const QString& tag) : tag_(tag), parent_(0) {}

QXmlNode::~QXmlNode() {
    for (int i = 0; i < children_.count(); ++i) delete children_[i];
    children_.clearAll();
}

bool QXmlNode::hasAttribute(const QString& name) const { return attrNames_.indexOf(name) >= 0; }

QString QXmlNode::attribute(const QString& name, const QString& defaultValue) const {
    int index = attrNames_.indexOf(name);
    return index >= 0 ? attrValues_[index] : defaultValue;
}

void QXmlNode::setAttribute(const QString& name, const QString& value) {
    int index = attrNames_.indexOf(name);
    if (index >= 0) {
        attrValues_[index] = value;
        return;
    }
    attrNames_.append(name);
    attrValues_.append(value);
}

QXmlNode* QXmlNode::childAt(int index) const {
    if (index < 0 || index >= children_.count()) return 0;
    return children_[index];
}

QXmlNode* QXmlNode::firstChild(const QString& tag) const {
    for (int i = 0; i < children_.count(); ++i)
        if (children_[i]->tag_ == tag) return children_[i];
    return 0;
}

QList<QXmlNode*> QXmlNode::childrenNamed(const QString& tag) const {
    QList<QXmlNode*> out;
    for (int i = 0; i < children_.count(); ++i)
        if (children_[i]->tag_ == tag) out.append(children_[i]);
    return out;
}

QXmlNode* QXmlNode::appendChild(QXmlNode* child) {
    if (!child) return 0;
    child->parent_ = this;
    children_.append(child);
    return child;
}

QXmlNode* QXmlNode::appendChild(const QString& tag) { return appendChild(new QXmlNode(tag)); }

void QXmlNode::removeChild(QXmlNode* child) {
    int index = children_.indexOf(child);
    if (index < 0) return;
    children_.removeAt(index);
    child->parent_ = 0;
    delete child;
}

void QXmlNode::clearChildren() {
    for (int i = 0; i < children_.count(); ++i) delete children_[i];
    children_.clearAll();
}

QString QXmlNode::escape(const QString& text) {
    QString out;
    for (int i = 0; i < text.size(); ++i) {
        wchar_t c = text[i];
        switch (c) {
        case L'&': out += L"&amp;"; break;
        case L'<': out += L"&lt;"; break;
        case L'>': out += L"&gt;"; break;
        case L'"': out += L"&quot;"; break;
        default: out += c; break;
        }
    }
    return out;
}

QString QXmlNode::unescape(const QString& text) {
    QString out;
    for (int i = 0; i < text.size(); ++i) {
        wchar_t c = text[i];
        if (c != L'&') { out += c; continue; }
        int semi = text.indexOf(L';', i);
        if (semi < 0) { out += c; continue; }
        QString entity = text.mid(i + 1, semi - i - 1);
        if (entity == L"amp") out += L'&';
        else if (entity == L"lt") out += L'<';
        else if (entity == L"gt") out += L'>';
        else if (entity == L"quot") out += L'"';
        else if (entity == L"apos") out += L'\'';
        else if (entity.size() > 1 && entity[0] == L'#') {
            int code = entity[1] == L'x' || entity[1] == L'X'
                           ? (int)wcstol(entity.c_str() + 2, 0, 16)
                           : (int)wcstol(entity.c_str() + 1, 0, 10);
            if (code > 0) out += (wchar_t)code;
        } else {
            out += text.mid(i, semi - i + 1);
        }
        i = semi;
    }
    return out;
}

QString QXmlNode::toXml(int indent) const {
    QString pad;
    for (int i = 0; i < indent; ++i) pad += L" ";
    QString out = pad + L"<" + tag_;
    for (int i = 0; i < attrNames_.count(); ++i)
        out += L" " + attrNames_[i] + L"=\"" + escape(attrValues_[i]) + L"\"";
    if (children_.isEmpty() && text_.isEmpty()) return out + L"/>";
    out += L">";
    if (children_.isEmpty()) return out + escape(text_) + L"</" + tag_ + L">";
    out += L"\n";
    if (!text_.isEmpty()) out += pad + L"  " + escape(text_) + L"\n";
    for (int i = 0; i < children_.count(); ++i)
        out += children_[i]->toXml(indent + 2) + L"\n";
    return out + pad + L"</" + tag_ + L">";
}

namespace {

struct Parser {
    const QString& s;
    int i;
    QString error;

    explicit Parser(const QString& text) : s(text), i(0) {}

    bool atEnd() const { return i >= s.size(); }

    void skipSpace() {
        while (i < s.size() && (s[i] == L' ' || s[i] == L'\t' || s[i] == L'\n' || s[i] == L'\r')) ++i;
    }

    bool skipMisc() {
        for (;;) {
            skipSpace();
            if (i + 3 < s.size() && s[i] == L'<' && s[i + 1] == L'!' && s[i + 2] == L'-' && s[i + 3] == L'-') {
                int end = s.indexOf(L"-->", i);
                if (end < 0) { error = L"unterminated comment"; return false; }
                i = end + 3;
                continue;
            }
            if (i + 1 < s.size() && s[i] == L'<' && s[i + 1] == L'?') {
                int end = s.indexOf(L"?>", i);
                if (end < 0) { error = L"unterminated declaration"; return false; }
                i = end + 2;
                continue;
            }
            break;
        }
        return true;
    }

    bool parseName(QString* out) {
        if (i >= s.size()) return false;
        int start = i;
        while (i < s.size()) {
            wchar_t c = s[i];
            if ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9') ||
                c == L'_' || c == L':' || c == L'-' || c == L'.') {
                ++i;
            } else {
                break;
            }
        }
        if (i == start) return false;
        *out = s.mid(start, i - start);
        return true;
    }

    QXmlNode* parseElement() {
        if (i >= s.size() || s[i] != L'<') { error = L"expected element"; return 0; }
        ++i;
        QString tag;
        if (!parseName(&tag)) { error = L"expected element name"; return 0; }
        QXmlNode* node = new QXmlNode(tag);
        for (;;) {
            skipSpace();
            if (i >= s.size()) { error = L"unterminated tag"; delete node; return 0; }
            if (s[i] == L'/') {
                ++i;
                if (i >= s.size() || s[i] != L'>') { error = L"expected '>'"; delete node; return 0; }
                ++i;
                return node;
            }
            if (s[i] == L'>') { ++i; break; }
            QString name;
            if (!parseName(&name)) { error = L"expected attribute name"; delete node; return 0; }
            skipSpace();
            if (i >= s.size() || s[i] != L'=') { error = L"expected '='"; delete node; return 0; }
            ++i;
            skipSpace();
            if (i >= s.size() || s[i] != L'"') { error = L"expected quoted value"; delete node; return 0; }
            ++i;
            int start = i;
            while (i < s.size() && s[i] != L'"') ++i;
            if (i >= s.size()) { error = L"unterminated attribute"; delete node; return 0; }
            QString value = s.mid(start, i - start);
            ++i;
            node->setAttribute(name, QXmlNode::unescape(value));
        }
        // content
        for (;;) {
            if (i >= s.size()) { error = L"unterminated element"; delete node; return 0; }
            if (s[i] == L'<') {
                if (i + 1 < s.size() && s[i + 1] == L'/') {
                    i += 2;
                    QString closing;
                    if (!parseName(&closing)) { error = L"expected closing name"; delete node; return 0; }
                    skipSpace();
                    if (i >= s.size() || s[i] != L'>') { error = L"expected '>'"; delete node; return 0; }
                    ++i;
                    if (closing != tag) { error = L"mismatched closing tag"; delete node; return 0; }
                    return node;
                }
                if (!skipMisc()) { delete node; return 0; }
                QXmlNode* child = parseElement();
                if (!child) { delete node; return 0; }
                node->appendChild(child);
                continue;
            }
            int start = i;
            while (i < s.size() && s[i] != L'<') ++i;
            QString text = s.mid(start, i - start);
            if (!text.isEmpty()) node->addText(QXmlNode::unescape(text));
        }
    }
};

} // namespace

QXmlNode* QXmlNode::parse(const QString& text, QString* error) {
    // strip BOM
    QString source = text;
    if (!source.isEmpty() && source[0] == (wchar_t)0xFEFF) source = source.mid(1);
    Parser p(source);
    if (!p.skipMisc()) {
        if (error) *error = p.error;
        return 0;
    }
    if (p.i >= source.size() || source[p.i] != L'<') {
        if (error) *error = L"no root element";
        return 0;
    }
    QXmlNode* root = p.parseElement();
    if (!root) {
        if (error) *error = p.error;
        return 0;
    }
    while (!p.atEnd()) {
        p.skipSpace();
        if (p.atEnd()) break;
        if (source[p.i] == L'<') {
            if (!p.skipMisc()) break;
            if (p.atEnd()) break;
        }
        ++p.i;
    }
    return root;
}
