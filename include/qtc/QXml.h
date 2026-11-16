// QtClone - QXml.h : a small XML DOM used by the designer and the .ui compiler.
#ifndef QTC_QXML_H
#define QTC_QXML_H

#include <qtc/QtGlobal.h>
#include <qtc/QString.h>
#include <qtc/QList.h>

class QXmlNode {
public:
    QXmlNode();
    explicit QXmlNode(const QString& tag);
    ~QXmlNode();

    QString tagName() const { return tag_; }
    void setTagName(const QString& tag) { tag_ = tag; }

    bool hasAttribute(const QString& name) const;
    QString attribute(const QString& name, const QString& defaultValue = QString()) const;
    void setAttribute(const QString& name, const QString& value);
    QList<QString> attributeNames() const { return attrNames_; }
    QList<QString> attributeValues() const { return attrValues_; }

    QString text() const { return text_; }
    void setText(const QString& text) { text_ = text; }
    void addText(const QString& text) { text_ += text; }

    QList<QXmlNode*> children() const { return children_; }
    int childCount() const { return children_.count(); }
    QXmlNode* childAt(int index) const;
    QXmlNode* firstChild(const QString& tag) const;
    QList<QXmlNode*> childrenNamed(const QString& tag) const;

    QXmlNode* appendChild(QXmlNode* child);
    QXmlNode* appendChild(const QString& tag);
    void removeChild(QXmlNode* child);
    void clearChildren();

    QXmlNode* parent() const { return parent_; }

    static QXmlNode* parse(const QString& text, QString* error = 0);
    QString toXml(int indent = 0) const;

    static QString escape(const QString& text);
    static QString unescape(const QString& text);

private:
    QString tag_;
    QList<QString> attrNames_;
    QList<QString> attrValues_;
    QString text_;
    QList<QXmlNode*> children_;
    QXmlNode* parent_;
};

#endif // QTC_QXML_H
