// QtClone Designer - a visual form editor for the QtClone widget framework.
//
// The .ui XML document is the single source of truth. The canvas edits live
// widgets; every change is captured back into XML, which also drives undo,
// preview and C++ code generation.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include <qtc/QXml.h>
#include "../tools/uicgen/uicgen.h"
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// property metadata helpers
// ---------------------------------------------------------------------------
namespace {

struct EnumItem {
    const wchar_t* label;
    int value;
};

struct EnumDef {
    const wchar_t* name;
    const EnumItem* items;
    int count;
};

const EnumItem kShapeItems[] = {
    { L"QFrame::NoFrame", 0 }, { L"QFrame::Box", 1 }, { L"QFrame::Panel", 2 },
    { L"QFrame::StyledPanel", 3 }, { L"QFrame::HLine", 4 }, { L"QFrame::VLine", 5 }
};
const EnumItem kShadowItems[] = {
    { L"QFrame::Plain", 0 }, { L"QFrame::Raised", 1 }, { L"QFrame::Sunken", 2 }
};
const EnumItem kFocusItems[] = {
    { L"Qt::NoFocus", 0 }, { L"Qt::TabFocus", 1 }, { L"Qt::ClickFocus", 2 },
    { L"Qt::StrongFocus", 11 }, { L"Qt::WheelFocus", 15 }
};
const EnumItem kOrientationItems[] = { { L"Qt::Horizontal", 1 }, { L"Qt::Vertical", 2 } };
const EnumItem kEchoItems[] = {
    { L"QLineEdit::Normal", 0 }, { L"QLineEdit::NoEcho", 1 },
    { L"QLineEdit::Password", 2 }, { L"QLineEdit::PasswordEchoOnEdit", 3 }
};
const EnumItem kTabPositionItems[] = {
    { L"QTabWidget::North", 0 }, { L"QTabWidget::South", 1 },
    { L"QTabWidget::West", 2 }, { L"QTabWidget::East", 3 }
};
const EnumItem kScrollPolicyItems[] = {
    { L"Qt::ScrollBarAsNeeded", 0 }, { L"Qt::ScrollBarAlwaysOff", 1 },
    { L"Qt::ScrollBarAlwaysOn", 2 }
};
const EnumItem kToolStyleItems[] = {
    { L"Qt::ToolButtonIconOnly", 0 }, { L"Qt::ToolButtonTextOnly", 1 },
    { L"Qt::ToolButtonTextBesideIcon", 2 }, { L"Qt::ToolButtonTextUnderIcon", 3 }
};
const EnumItem kMessageIconItems[] = {
    { L"QMessageBox::NoIcon", 0 }, { L"QMessageBox::Information", 1 },
    { L"QMessageBox::Warning", 2 }, { L"QMessageBox::Critical", 3 },
    { L"QMessageBox::Question", 4 }
};

const EnumDef kEnums[] = {
    { L"QFrame::Shape", kShapeItems, 6 },
    { L"QFrame::Shadow", kShadowItems, 3 },
    { L"Qt::FocusPolicy", kFocusItems, 5 },
    { L"Qt::Orientation", kOrientationItems, 2 },
    { L"QLineEdit::EchoMode", kEchoItems, 4 },
    { L"QTabWidget::TabPosition", kTabPositionItems, 4 },
    { L"Qt::ScrollBarPolicy", kScrollPolicyItems, 3 },
    { L"Qt::ToolButtonStyle", kToolStyleItems, 4 },
    { L"QMessageBox::Icon", kMessageIconItems, 5 }
};

const EnumDef* findEnum(const QString& name) {
    for (int i = 0; i < (int)(sizeof(kEnums) / sizeof(kEnums[0])); ++i)
        if (name == kEnums[i].name) return &kEnums[i];
    return 0;
}

QString uniqueName(QWidget* form, const QString& base) {
    for (int index = 1;; ++index) {
        QString candidate = base + QString::number(index);
        if (!form->findChild<QWidget>(candidate)) return candidate;
    }
}

void writeRect(QXmlNode* parent, const char* tag, const QRect& r) {
    QXmlNode* node = parent->appendChild(QString::fromUtf8(tag));
    node->appendChild(L"x")->setText(QString::number(r.x()));
    node->appendChild(L"y")->setText(QString::number(r.y()));
    node->appendChild(L"width")->setText(QString::number(r.width()));
    node->appendChild(L"height")->setText(QString::number(r.height()));
}

void writePropertyNode(QXmlNode* parent, const QString& name, const QVariant& value) {
    QXmlNode* prop = parent->appendChild(L"property");
    prop->setAttribute(L"name", name);
    const char* typeRaw = value.typeName();
    QString type = typeRaw ? QString::fromUtf8(typeRaw) : QString();
    if (type == L"QString") {
        prop->appendChild(L"string")->setText(value.toString());
    } else if (type == L"bool") {
        prop->appendChild(L"bool")->setText(value.toBool() ? L"true" : L"false");
    } else if (type == L"int" || type == L"uint") {
        prop->appendChild(L"number")->setText(QString::number(value.toInt()));
    } else if (type == L"double") {
        prop->appendChild(L"double")->setText(QString::number(value.toDouble(), 'g', 6));
    } else if (type == L"QRect") {
        writeRect(prop, "rect", value.toRect());
    } else if (type == L"QSize") {
        QSize s = value.toSize();
        QXmlNode* node = prop->appendChild(L"size");
        node->appendChild(L"width")->setText(QString::number(s.width()));
        node->appendChild(L"height")->setText(QString::number(s.height()));
    } else if (type == L"QPoint") {
        QPoint p = value.toPoint();
        QXmlNode* node = prop->appendChild(L"point");
        node->appendChild(L"x")->setText(QString::number(p.x()));
        node->appendChild(L"y")->setText(QString::number(p.y()));
    } else if (type == L"QColor") {
        QColor c = value.toColor();
        QXmlNode* node = prop->appendChild(L"color");
        node->appendChild(L"red")->setText(QString::number(c.red()));
        node->appendChild(L"green")->setText(QString::number(c.green()));
        node->appendChild(L"blue")->setText(QString::number(c.blue()));
        node->appendChild(L"alpha")->setText(QString::number(c.alpha()));
    } else if (type == L"QFont") {
        QFont f = value.toFont();
        QXmlNode* node = prop->appendChild(L"font");
        node->appendChild(L"family")->setText(f.family());
        node->appendChild(L"pointsize")->setText(QString::number(f.pointSize()));
        node->appendChild(L"pixelsize")->setText(QString::number(f.pixelHeight()));
        node->appendChild(L"weight")->setText(QString::number(f.bold() ? 75 : 50));
        node->appendChild(L"italic")->setText(f.italic() ? L"true" : L"false");
    } else if (type == L"QStringList") {
        QXmlNode* node = prop->appendChild(L"stringlist");
        QStringList list = value.toStringList();
        for (int i = 0; i < list.count(); ++i) node->appendChild(L"string")->setText(list[i]);
    }
}

QVariant variantFromXml(const QString& typeName, QXmlNode* prop) {
    QList<QXmlNode*> kids = prop->children();
    QXmlNode* value = 0;
    for (int i = 0; i < kids.count(); ++i) {
        if (kids[i]->tagName() != L"property") { value = kids[i]; break; }
    }
    if (!value) return QVariant();
    if (typeName == L"QString") return QVariant(value->text());
    if (typeName == L"bool") return QVariant(value->text() == L"true");
    if (typeName == L"int" || typeName == L"uint") return QVariant(value->text().toInt());
    if (typeName == L"double") return QVariant(value->text().toDouble());
    if (typeName == L"QRect") {
        QXmlNode* r = value->tagName() == L"rect" ? value : value->firstChild(L"rect");
        if (!r) return QVariant();
        int x = r->firstChild(L"x") ? r->firstChild(L"x")->text().toInt() : 0;
        int y = r->firstChild(L"y") ? r->firstChild(L"y")->text().toInt() : 0;
        int w = r->firstChild(L"width") ? r->firstChild(L"width")->text().toInt() : 0;
        int h = r->firstChild(L"height") ? r->firstChild(L"height")->text().toInt() : 0;
        return QVariant(QRect(x, y, w, h));
    }
    if (typeName == L"QSize") {
        QXmlNode* s = value->tagName() == L"size" ? value : value->firstChild(L"size");
        if (!s) return QVariant();
        int w = s->firstChild(L"width") ? s->firstChild(L"width")->text().toInt() : 0;
        int h = s->firstChild(L"height") ? s->firstChild(L"height")->text().toInt() : 0;
        return QVariant(QSize(w, h));
    }
    if (typeName == L"QPoint") {
        QXmlNode* p = value->tagName() == L"point" ? value : value->firstChild(L"point");
        if (!p) return QVariant();
        int x = p->firstChild(L"x") ? p->firstChild(L"x")->text().toInt() : 0;
        int y = p->firstChild(L"y") ? p->firstChild(L"y")->text().toInt() : 0;
        return QVariant(QPoint(x, y));
    }
    if (typeName == L"QColor") {
        QXmlNode* c = value->tagName() == L"color" ? value : value->firstChild(L"color");
        if (!c) return QVariant();
        int r = c->firstChild(L"red") ? c->firstChild(L"red")->text().toInt() : 0;
        int g = c->firstChild(L"green") ? c->firstChild(L"green")->text().toInt() : 0;
        int b = c->firstChild(L"blue") ? c->firstChild(L"blue")->text().toInt() : 0;
        int a = c->firstChild(L"alpha") ? c->firstChild(L"alpha")->text().toInt() : 255;
        return QVariant(QColor(r, g, b, a));
    }
    if (typeName == L"QFont") {
        QXmlNode* f = value->tagName() == L"font" ? value : value->firstChild(L"font");
        if (!f) return QVariant();
        QString family = f->firstChild(L"family") ? f->firstChild(L"family")->text()
                                                 : QString(L"Microsoft YaHei UI");
        int px = f->firstChild(L"pixelsize") ? f->firstChild(L"pixelsize")->text().toInt() : 12;
        QFont font(family);
        font.setPixelSize(px);
        if (f->firstChild(L"weight")) font.setBold(f->firstChild(L"weight")->text().toInt() >= 75);
        if (f->firstChild(L"italic")) font.setItalic(f->firstChild(L"italic")->text() == L"true");
        return QVariant(font);
    }
    if (typeName == L"QStringList") {
        QStringList list;
        QList<QXmlNode*> items = value->childrenNamed(L"string");
        for (int i = 0; i < items.count(); ++i) list.append(items[i]->text());
        return QVariant(list);
    }
    return QVariant();
}

QString captureWidgetXml(QWidget* widget) {
    QXmlNode node(L"widget");
    node.setAttribute(L"class", QString(widget->className()));
    node.setAttribute(L"name", widget->objectName());
    QXmlNode* geometry = node.appendChild(L"property");
    geometry->setAttribute(L"name", L"geometry");
    writeRect(geometry, "rect", widget->geometry());
    const QMetaObject* mo = widget->metaObject();
    for (int i = 0; i < mo->propertyCount(); ++i) {
        QMetaProperty prop = mo->property(i);
        const char* name = prop.name();
        if (!prop.isDesignable()) continue;
        if (strcmp(name, "geometry") == 0) continue;
        if (strcmp(name, "visible") == 0) continue;
        QVariant value = prop.read(widget);
        if (!value.isValid()) continue;
        writePropertyNode(&node, QString::fromUtf8(name), value);
    }
    return node.toXml(0);
}

void rebuildWidgetFromXml(QWidget* widget, QXmlNode* node) {
    const QMetaObject* mo = widget->metaObject();
    QList<QXmlNode*> props = node->childrenNamed(L"property");
    for (int i = 0; i < props.count(); ++i) {
        QString name = props[i]->attribute(L"name");
        int index = mo->indexOfProperty(name.toUtf8().c_str());
        if (index < 0) continue;
        QMetaProperty prop = mo->property(index);
        QVariant value = variantFromXml(QString::fromUtf8(prop.typeName()), props[i]);
        if (value.isValid()) prop.write(widget, value);
    }
}

QWidget* buildWidgetTree(QXmlNode* node, QWidget* parent) {
    QString className = node->attribute(L"class");
    QWidget* widget = QWidgetFactory::create(className.toUtf8().c_str());
    if (!widget) return 0;
    widget->setParentWidget(parent);
    widget->setObjectName(node->attribute(L"name"));
    // geometry is applied explicitly: it drives the absolute layout on the canvas
    QList<QXmlNode*> props = node->childrenNamed(L"property");
    for (int i = 0; i < props.count(); ++i) {
        if (props[i]->attribute(L"name") != L"geometry") continue;
        QVariant g = variantFromXml(L"QRect", props[i]);
        if (g.isValid()) widget->setGeometry(g.toRect());
        break;
    }
    rebuildWidgetFromXml(widget, node);
    widget->show();
    QList<QXmlNode*> children = node->childrenNamed(L"widget");
    for (int i = 0; i < children.count(); ++i) buildWidgetTree(children[i], widget);
    return widget;
}

} // namespace

// ---------------------------------------------------------------------------
// DesignerDocument
// ---------------------------------------------------------------------------
class DesignerDocument {
public:
    DesignerDocument() : undoIndex_(-1), modified_(false) { formClass_ = L"Form1"; }

    const QString& xml() const { return xml_; }
    void setXml(const QString& xml) { xml_ = xml; }
    void setXmlAndCaptureUndo(const QString& xml) {
        xml_ = xml;
        while (undoStack_.count() > undoIndex_ + 1) undoStack_.removeAt(undoStack_.count() - 1);
        undoStack_.append(xml);
        undoIndex_ = undoStack_.count() - 1;
        if (undoStack_.count() > 200) {
            undoStack_.removeAt(0);
            --undoIndex_;
        }
        modified_ = true;
    }
    bool undo() {
        if (undoIndex_ <= 0) return false;
        --undoIndex_;
        xml_ = undoStack_[undoIndex_];
        modified_ = true;
        return true;
    }
    bool redo() {
        if (undoIndex_ + 1 >= undoStack_.count()) return false;
        ++undoIndex_;
        xml_ = undoStack_[undoIndex_];
        modified_ = true;
        return true;
    }
    bool canUndo() const { return undoIndex_ > 0; }
    bool canRedo() const { return undoIndex_ + 1 < undoStack_.count(); }

    void reset(const QString& formClass) {
        formClass_ = formClass;
        QXmlNode ui(L"ui");
        ui.setAttribute(L"version", L"4.0");
        ui.appendChild(L"class")->setText(formClass_);
        QXmlNode* top = ui.appendChild(L"widget");
        top->setAttribute(L"class", L"QWidget");
        top->setAttribute(L"name", formClass_);
        QXmlNode* geometry = top->appendChild(L"property");
        geometry->setAttribute(L"name", L"geometry");
        writeRect(geometry, "rect", QRect(0, 0, 640, 480));
        writePropertyNode(top, L"windowTitle", QVariant(formClass_));
        ui.appendChild(L"connections");
        xml_ = ui.toXml(0);
        undoStack_.clearAll();
        undoIndex_ = -1;
        modified_ = false;
    }

    QString formClass() const { return formClass_; }
    void setFormClass(const QString& c) { formClass_ = c; }
    QString filePath() const { return filePath_; }
    void setFilePath(const QString& path) { filePath_ = path; }
    bool modified() const { return modified_; }
    void setModified(bool on) { modified_ = on; }

private:
    QString xml_;
    QString formClass_;
    QString filePath_;
    QList<QString> undoStack_;
    int undoIndex_;
    bool modified_;
};

// ---------------------------------------------------------------------------
// DesignerFormWidget : the designed surface (always light, like a sheet of paper)
// ---------------------------------------------------------------------------
class DesignerFormWidget : public QWidget {
public:
    explicit DesignerFormWidget(QWidget* parent) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        p.fillRect(rect(), QColor(255, 255, 255));
        p.setPen(QPen(QColor(0xD0, 0xD5, 0xDD), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }
};

// ---------------------------------------------------------------------------
// DesignerCanvas
// ---------------------------------------------------------------------------
class DesignerCanvas : public QWidget {
public:
    DesignerCanvas(DesignerDocument* doc);

    QWidget* form() const { return form_; }
    QWidget* selected() const { return selected_; }
    bool gridVisible() const { return grid_; }
    void setGridVisible(bool visible) { grid_ = visible; update(); }

    void rebuild();
    void select(QWidget* widget, bool notify = true);
    void deleteSelected();
    void duplicateSelected();
    void raiseSelected();
    void lowerSelected();
    void captureToDocument();
    void applyProperty(QWidget* widget, const char* name, const QVariant& value);

    Signal<void(QWidget*)> selectionChanged;
    Signal<void()> documentModified;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    enum Mode { Idle, Moving, Resizing };
    QRect handlesRect(QList<QRect>* handles) const;
    int handleAt(const QPoint& pos) const;
    QWidget* widgetAt(const QPoint& pos, QWidget* root) const;
    QPoint snapPoint(const QPoint& p) const;

    DesignerDocument* doc_;
    QWidget* form_;
    QWidget* selected_;
    bool grid_;
    Mode mode_;
    int handle_;
    QPoint dragStart_;
    QRect startGeometry_;
};

DesignerCanvas::DesignerCanvas(DesignerDocument* doc)
    : QWidget(0), doc_(doc), form_(0), selected_(0), grid_(true), mode_(Idle), handle_(-1) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    form_ = new DesignerFormWidget(this);
    form_->setObjectName(doc_->formClass());
    form_->setGeometry(QRect(20, 20, 640, 480));
    form_->setObjectName(doc_->formClass());
    form_->show();
    rebuild();
}

void DesignerCanvas::rebuild() {
    QList<QWidget*> children = form_->childrenWidgets();
    for (int i = 0; i < children.count(); ++i) {
        children[i]->setParentWidget(0);
        delete children[i];
    }
    selected_ = 0;
    form_->setObjectName(doc_->formClass());

    QString error;
    QXmlNode* root = QXmlNode::parse(doc_->xml(), &error);
    if (!root) return;
    QXmlNode* top = root->firstChild(L"widget");
    if (!top) { delete root; return; }
    QList<QXmlNode*> props = top->childrenNamed(L"property");
    for (int i = 0; i < props.count(); ++i) {
        QString name = props[i]->attribute(L"name");
        if (name == L"geometry") {
            QVariant v = variantFromXml(L"QRect", props[i]);
            if (v.isValid()) form_->resize(v.toRect().size());
        } else if (name == L"windowTitle") {
            QVariant v = variantFromXml(L"QString", props[i]);
            if (v.isValid()) form_->setWindowTitle(v.toString());
        }
    }
    QList<QXmlNode*> childNodes = top->childrenNamed(L"widget");
    for (int i = 0; i < childNodes.count(); ++i) buildWidgetTree(childNodes[i], form_);
    delete root;
    update();
}

QPoint DesignerCanvas::snapPoint(const QPoint& p) const {
    if (!grid_ || (QApplication::keyboardModifiers() & Qt::AltModifier)) return p;
    return QPoint((p.x() / 8) * 8, (p.y() / 8) * 8);
}

QWidget* DesignerCanvas::widgetAt(const QPoint& pos, QWidget* root) const {
    QList<QWidget*> children = root->childrenWidgets();
    for (int i = children.count() - 1; i >= 0; --i) {
        QWidget* child = children[i];
        if (!child->isVisible()) continue;
        if (child->geometry().contains(pos)) {
            QWidget* deeper = widgetAt(pos - child->pos(), child);
            return deeper ? deeper : child;
        }
    }
    return 0;
}

QRect DesignerCanvas::handlesRect(QList<QRect>* handles) const {
    if (!selected_) return QRect();
    QRect r = selected_->geometry().translated(form_->pos());
    int s = 7;
    if (handles) {
        handles->clearAll();
        handles->append(QRect(r.left() - s / 2, r.top() - s / 2, s, s));
        handles->append(QRect(r.center().x() - s / 2, r.top() - s / 2, s, s));
        handles->append(QRect(r.right() - s / 2, r.top() - s / 2, s, s));
        handles->append(QRect(r.left() - s / 2, r.center().y() - s / 2, s, s));
        handles->append(QRect(r.right() - s / 2, r.center().y() - s / 2, s, s));
        handles->append(QRect(r.left() - s / 2, r.bottom() - s / 2, s, s));
        handles->append(QRect(r.center().x() - s / 2, r.bottom() - s / 2, s, s));
        handles->append(QRect(r.right() - s / 2, r.bottom() - s / 2, s, s));
    }
    return r;
}

int DesignerCanvas::handleAt(const QPoint& pos) const {
    QList<QRect> handles;
    handlesRect(&handles);
    for (int i = 0; i < handles.count(); ++i)
        if (handles[i].contains(pos)) return i;
    return -1;
}

void DesignerCanvas::select(QWidget* widget, bool notify) {
    if (selected_ == widget) return;
    selected_ = widget;
    update();
    if (notify) selectionChanged(selected_);
}

void DesignerCanvas::deleteSelected() {
    if (!selected_) return;
    QWidget* victim = selected_;
    select(0, true);
    victim->setParentWidget(0);
    delete victim;
    captureToDocument();
    update();
}

void DesignerCanvas::duplicateSelected() {
    if (!selected_) return;
    QString xml = captureWidgetXml(selected_);
    QXmlNode* node = QXmlNode::parse(xml, 0);
    if (!node) return;
    QWidget* copy = buildWidgetTree(node, form_);
    delete node;
    if (!copy) return;
    copy->setObjectName(uniqueName(form_, QString(copy->className()).toLower()));
    QRect g = selected_->geometry();
    copy->setGeometry(g.translated(16, 16));
    copy->show();
    select(copy, true);
    captureToDocument();
}

void DesignerCanvas::raiseSelected() {
    if (selected_) {
        selected_->raise();
        captureToDocument();
        update();
    }
}

void DesignerCanvas::lowerSelected() {
    if (selected_) {
        selected_->lower();
        captureToDocument();
        update();
    }
}

void DesignerCanvas::captureToDocument() {
    QXmlNode ui(L"ui");
    ui.setAttribute(L"version", L"4.0");
    ui.appendChild(L"class")->setText(doc_->formClass());
    QXmlNode* top = ui.appendChild(L"widget");
    top->setAttribute(L"class", L"QWidget");
    top->setAttribute(L"name", doc_->formClass());
    QXmlNode* geometry = top->appendChild(L"property");
    geometry->setAttribute(L"name", L"geometry");
    writeRect(geometry, "rect", QRect(0, 0, form_->width(), form_->height()));
    writePropertyNode(top, L"windowTitle", QVariant(form_->windowTitle()));

    QList<QWidget*> children = form_->childrenWidgets();
    for (int i = 0; i < children.count(); ++i) {
        QXmlNode* child = QXmlNode::parse(captureWidgetXml(children[i]), 0);
        if (!child) continue;
        // nested children are captured separately, keep the tree shallow for now
        QList<QWidget*> grandchildren = children[i]->childrenWidgets();
        for (int g = 0; g < grandchildren.count(); ++g) {
            QXmlNode* gnode = QXmlNode::parse(captureWidgetXml(grandchildren[g]), 0);
            if (gnode) child->appendChild(gnode);
        }
        top->appendChild(child);
    }
    ui.appendChild(L"connections");
    doc_->setXmlAndCaptureUndo(ui.toXml(0));
    documentModified();
}

void DesignerCanvas::applyProperty(QWidget* widget, const char* name, const QVariant& value) {
    if (!widget) return;
    const QMetaObject* mo = widget->metaObject();
    int index = mo->indexOfProperty(name);
    if (index < 0) return;
    mo->property(index).write(widget, value);
    widget->update();
    captureToDocument();
    update();
}

void DesignerCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QTheme::colors().canvas);

    if (grid_) {
        QPen dot(QTheme::colors().border, 1);
        p.setPen(dot);
        for (int x = 20; x < width(); x += 8) {
            for (int y = 20; y < height(); y += 8) {
                if (((x - 20) % 32 == 0) && ((y - 20) % 32 == 0)) p.drawPoint(x, y);
            }
        }
    }

    if (selected_) {
        QList<QRect> handles;
        QRect r = handlesRect(&handles);
        p.setPen(QPen(QTheme::colors().accent, 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(r);
        p.setBrush(QBrush(QColor(0xFF, 0xFF, 0xFF)));
        p.setPen(QPen(QTheme::colors().accent, 1));
        for (int i = 0; i < handles.count(); ++i) p.drawRect(handles[i]);
    }
}

void DesignerCanvas::mousePressEvent(QMouseEvent* event) {
    setFocus(Qt::MouseFocusReason);
    if (event->button() != Qt::LeftButton) return;
    int handle = selected_ ? handleAt(event->pos()) : -1;
    if (handle >= 0) {
        mode_ = Resizing;
        handle_ = handle;
        dragStart_ = event->pos();
        startGeometry_ = selected_->geometry();
        return;
    }
    QPoint local = event->pos() - form_->pos();
    QWidget* hit = widgetAt(local, form_);
    if (hit) {
        select(hit);
        mode_ = Moving;
        handle_ = -1;
        dragStart_ = event->pos();
        startGeometry_ = hit->geometry();
    } else {
        select(0);
        mode_ = Idle;
    }
}

void DesignerCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (mode_ == Idle || !selected_) return;
    QPoint delta = event->pos() - dragStart_;
    if (mode_ == Moving) {
        QPoint target = startGeometry_.topLeft() + delta;
        target = snapPoint(target);
        selected_->move(target);
    } else if (mode_ == Resizing) {
        QRect g = startGeometry_;
        QPoint delta = snapPoint(startGeometry_.topLeft() + (event->pos() - dragStart_)) - startGeometry_.topLeft();
        int minW = 16, minH = 16;
        switch (handle_) {
        case 0: g.setTopLeft(g.topLeft() + delta); break;
        case 1: g.setTop(g.top() + delta.y()); break;
        case 2: g.setTopRight(g.topRight() + delta); break;
        case 3: g.setLeft(g.left() + delta.x()); break;
        case 4: g.setRight(g.right() + delta.x()); break;
        case 5: g.setBottomLeft(g.bottomLeft() + delta); break;
        case 6: g.setBottom(g.bottom() + delta.y()); break;
        case 7: g.setBottomRight(g.bottomRight() + delta); break;
        default: break;
        }
        if (g.width() < minW) { g.setLeft(startGeometry_.left()); g.setRight(startGeometry_.right()); }
        if (g.height() < minH) { g.setTop(startGeometry_.top()); g.setBottom(startGeometry_.bottom()); }
        selected_->setGeometry(g);
    }
    update();
}

void DesignerCanvas::mouseReleaseEvent(QMouseEvent*) {
    if (mode_ != Idle) {
        mode_ = Idle;
        handle_ = -1;
        if (selected_) captureToDocument();
    }
}

void DesignerCanvas::keyPressEvent(QKeyEvent* event) {
    if (!selected_) {
        QWidget::keyPressEvent(event);
        return;
    }
    int step = (event->modifiers() & Qt::ShiftModifier) ? 8 : 1;
    QRect g = selected_->geometry();
    switch (event->key()) {
    case Qt::Key_Left: g.translate(-step, 0); break;
    case Qt::Key_Right: g.translate(step, 0); break;
    case Qt::Key_Up: g.translate(0, -step); break;
    case Qt::Key_Down: g.translate(0, step); break;
    case Qt::Key_Delete: deleteSelected(); return;
    default: QWidget::keyPressEvent(event); return;
    }
    selected_->setGeometry(g);
    captureToDocument();
    update();
}

void DesignerCanvas::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

// ---------------------------------------------------------------------------
// PropertyEditor
// ---------------------------------------------------------------------------
class PropertyEditor : public QWidget {
public:
    PropertyEditor();

    void setTarget(QWidget* widget);
    void refresh();
    Signal<void(QWidget*, const char*, const QVariant&)> propertyChanged;

protected:
    void resizeEvent(QResizeEvent* event);

private:
    void onCellDoubleClicked(int row, int column);
    bool editValue(QWidget* widget, QMetaProperty prop);

    QTableWidget* table_;
    QWidget* target_;
};

PropertyEditor::PropertyEditor() : QWidget(0), target_(0) {
    table_ = new QTableWidget(this);
    table_->setColumnCount(2);
    QStringList headers;
    headers << L"属性" << L"值";
    table_->setHorizontalHeaderLabels(headers);
    table_->setAlternatingRowColors(true);
    table_->setGridEnabled(true);
    table_->setRowHeight(24);
    table_->setHorizontalHeaderVisible(true);
    QObject::connect(table_, table_->cellDoubleClicked, this, &PropertyEditor::onCellDoubleClicked);
}

void PropertyEditor::setTarget(QWidget* widget) {
    target_ = widget;
    refresh();
}

void PropertyEditor::refresh() {
    table_->clearContents();
    table_->setRowCount(0);
    if (!target_) return;
    const QMetaObject* mo = target_->metaObject();
    int rows = 0;
    for (int i = 0; i < mo->propertyCount(); ++i) {
        QMetaProperty prop = mo->property(i);
        if (!prop.isDesignable()) continue;
        ++rows;
    }
    table_->setRowCount(rows);
    int row = 0;
    for (int i = 0; i < mo->propertyCount(); ++i) {
        QMetaProperty prop = mo->property(i);
        if (!prop.isDesignable()) continue;
        table_->setItem(row, 0, new QTableWidgetItem(QString::fromUtf8(prop.name())));
        QVariant value = prop.read(target_);
        table_->setItem(row, 1, new QTableWidgetItem(value.toString()));
        ++row;
    }
}

void PropertyEditor::resizeEvent(QResizeEvent* event) {
    table_->setGeometry(rect());
    QWidget::resizeEvent(event);
}

void PropertyEditor::onCellDoubleClicked(int row, int) {
    if (!target_) return;
    QTableWidgetItem* nameItem = table_->item(row, 0);
    if (!nameItem) return;
    std::string name = nameItem->text().toUtf8();
    int index = target_->metaObject()->indexOfProperty(name.c_str());
    if (index < 0) return;
    QMetaProperty prop = target_->metaObject()->property(index);
    editValue(target_, prop);
}

bool PropertyEditor::editValue(QWidget* widget, QMetaProperty prop) {
    QString type = QString::fromUtf8(prop.typeName());
    QVariant current = prop.read(widget);
    QString name = QString::fromUtf8(prop.name());
    bool changed = false;
    if (type == L"bool") {
        prop.write(widget, QVariant(!current.toBool()));
        changed = true;
    } else if (prop.isEnumType() || type == L"int" || type == L"uint") {
        const EnumDef* def = prop.isEnumType() ? findEnum(QString::fromUtf8(prop.enumName())) : 0;
        if (def) {
            QStringList labels;
            for (int i = 0; i < def->count; ++i) labels.append(QString(def->items[i].label));
            bool ok = false;
            int currentIndex = 0;
            for (int i = 0; i < def->count; ++i)
                if (def->items[i].value == current.toInt()) currentIndex = i;
            QString picked = QInputDialog::getItem(this, L"选择值", name, labels, currentIndex, false, &ok);
            if (ok) {
                int index = labels.indexOf(picked);
                if (index >= 0) {
                    prop.write(widget, QVariant(def->items[index].value));
                    changed = true;
                }
            }
        } else {
            bool ok = false;
            int value = QInputDialog::getInt(this, L"编辑数值", name, current.toInt(),
                                             -2147483647, 2147483647, 1, &ok);
            if (ok) {
                prop.write(widget, QVariant(value));
                changed = true;
            }
        }
    } else if (type == L"double") {
        bool ok = false;
        double value = QInputDialog::getDouble(this, L"编辑小数", name, current.toDouble(),
                                               -1e12, 1e12, 3, &ok);
        if (ok) {
            prop.write(widget, QVariant(value));
            changed = true;
        }
    } else if (type == L"QString") {
        bool ok = false;
        QString text = QInputDialog::getText(this, L"编辑文本", name, 0, current.toString(), &ok);
        if (ok) {
            prop.write(widget, QVariant(text));
            changed = true;
        }
    } else if (type == L"QColor") {
        QColor color = current.toColor();
        QColor picked = QColorDialog::getColor(color, this, name);
        if (picked.isValid()) {
            prop.write(widget, QVariant(picked));
            changed = true;
        }
    } else if (type == L"QFont") {
        bool ok = false;
        QFont font = QFontDialog::getFont(&ok, current.toFont(), this, name);
        if (ok) {
            prop.write(widget, QVariant(font));
            changed = true;
        }
    } else if (type == L"QRect" || type == L"QSize" || type == L"QPoint") {
        QString text;
        if (type == L"QRect") {
            QRect r = current.toRect();
            text = QString::number(r.x()) + L"," + QString::number(r.y()) + L"," +
                   QString::number(r.width()) + L"," + QString::number(r.height());
        } else if (type == L"QSize") {
            QSize s = current.toSize();
            text = QString::number(s.width()) + L"," + QString::number(s.height());
        } else {
            QPoint p = current.toPoint();
            text = QString::number(p.x()) + L"," + QString::number(p.y());
        }
        bool ok = false;
        QString edited = QInputDialog::getText(this, L"编辑几何", name, 0, text, &ok);
        if (ok) {
            QStringList parts = edited.split(L',');
            if (type == L"QRect" && parts.count() == 4) {
                prop.write(widget, QVariant(QRect(parts[0].toInt(), parts[1].toInt(),
                                                 parts[2].toInt(), parts[3].toInt())));
                changed = true;
            } else if (type == L"QSize" && parts.count() == 2) {
                prop.write(widget, QVariant(QSize(parts[0].toInt(), parts[1].toInt())));
                changed = true;
            } else if (type == L"QPoint" && parts.count() == 2) {
                prop.write(widget, QVariant(QPoint(parts[0].toInt(), parts[1].toInt())));
                changed = true;
            }
        }
    }
    if (changed) {
        widget->update();
        refresh();
        propertyChanged(widget, prop.name(), prop.read(widget));
    }
    return changed;
}

// ---------------------------------------------------------------------------
// DesignerWindow
// ---------------------------------------------------------------------------
class DesignerWindow : public QMainWindow {
public:
    DesignerWindow();

    DesignerCanvas* canvas() const { return canvas_; }
    DesignerDocument* document() { return &doc_; }

    void newDocument();
    void openDocument(const QString& path);
    bool saveDocument(const QString& path);
    void generateCode(const QString& dir);
    void preview();
    void refreshInspector();
    void refreshProperties();
    void setStatus(const QString& text);

    int selfTest();
    bool screenshot(const QString& path);

private:
    void buildUi();
    void buildMenus();
    void onSelectionChanged(QWidget* widget);
    void onDocumentModified();
    void syncInspectorSelection(QWidget* widget);
    void addWidgetByClass(const QString& className);

    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onGenerate();
    void onUndo();
    void onRedo();
    void onDelete();
    void onDuplicate();
    void onToggleGrid();
    void onThemeLight();
    void onThemeDark();
    void onThemeSystem();
    void onPreview();
    void onAbout();
    void onQuit();
    void onToolboxActivated(QTreeWidgetItem* item, int column);
    void onTreeClicked(QTreeWidgetItem* item, int column);
    void onPropertyChanged(QWidget* widget, const char* name, const QVariant& value);

    DesignerDocument doc_;
    DesignerCanvas* canvas_;
    QTreeWidget* tree_;
    QTreeWidget* toolbox_;
    PropertyEditor* properties_;
    QStatusBar* status_;
    bool updatingInspector_;
};

DesignerWindow::DesignerWindow() : QMainWindow(0, Qt::Window), canvas_(0), tree_(0),
                                   toolbox_(0), properties_(0), status_(0), updatingInspector_(false) {
    setWindowTitle(L"QtClone Designer");
    resize(1200, 780);
    buildMenus();
    buildUi();
    newDocument();
}

void DesignerWindow::buildMenus() {
    QMenuBar* bar = new QMenuBar();
    setMenuBar(bar);

    QMenu* file = bar->addMenu(L"文件");
    QAction* newAct = file->addAction(L"新建");
    newAct->setShortcut(L"Ctrl+N");
    QObject::connect(newAct, newAct->triggered, this, &DesignerWindow::onNew);
    QAction* openAct = file->addAction(L"打开…");
    openAct->setShortcut(L"Ctrl+O");
    QObject::connect(openAct, openAct->triggered, this, &DesignerWindow::onOpen);
    QAction* saveAct = file->addAction(L"保存");
    saveAct->setShortcut(L"Ctrl+S");
    QObject::connect(saveAct, saveAct->triggered, this, &DesignerWindow::onSave);
    QAction* saveAsAct = file->addAction(L"另存为…");
    QObject::connect(saveAsAct, saveAsAct->triggered, this, &DesignerWindow::onSaveAs);
    file->addSeparator();
    QAction* codeAct = file->addAction(L"生成代码 (F7)");
    codeAct->setShortcut(L"F7");
    QObject::connect(codeAct, codeAct->triggered, this, &DesignerWindow::onGenerate);
    file->addSeparator();
    QAction* quitAct = file->addAction(L"退出");
    QObject::connect(quitAct, quitAct->triggered, this, &DesignerWindow::onQuit);

    QMenu* edit = bar->addMenu(L"编辑");
    QAction* undoAct = edit->addAction(L"撤销");
    undoAct->setShortcut(L"Ctrl+Z");
    QObject::connect(undoAct, undoAct->triggered, this, &DesignerWindow::onUndo);
    QAction* redoAct = edit->addAction(L"重做");
    redoAct->setShortcut(L"Ctrl+Y");
    QObject::connect(redoAct, redoAct->triggered, this, &DesignerWindow::onRedo);
    edit->addSeparator();
    QAction* deleteAct = edit->addAction(L"删除");
    deleteAct->setShortcut(L"Delete");
    QObject::connect(deleteAct, deleteAct->triggered, this, &DesignerWindow::onDelete);
    QAction* dupAct = edit->addAction(L"复制一份");
    dupAct->setShortcut(L"Ctrl+D");
    QObject::connect(dupAct, dupAct->triggered, this, &DesignerWindow::onDuplicate);

    QMenu* view = bar->addMenu(L"视图");
    QAction* gridAct = view->addAction(L"显示网格");
    gridAct->setCheckable(true);
    gridAct->setChecked(true);
    QObject::connect(gridAct, gridAct->triggered, this, &DesignerWindow::onToggleGrid);
    view->addSeparator();
    QAction* lightAct = view->addAction(L"亮色 Light");
    QObject::connect(lightAct, lightAct->triggered, this, &DesignerWindow::onThemeLight);
    QAction* darkAct = view->addAction(L"暗色 Dark");
    QObject::connect(darkAct, darkAct->triggered, this, &DesignerWindow::onThemeDark);
    QAction* systemAct = view->addAction(L"跟随系统 System");
    QObject::connect(systemAct, systemAct->triggered, this, &DesignerWindow::onThemeSystem);

    QMenu* run = bar->addMenu(L"运行");
    QAction* previewAct = run->addAction(L"预览 (F5)");
    previewAct->setShortcut(L"F5");
    QObject::connect(previewAct, previewAct->triggered, this, &DesignerWindow::onPreview);

    QMenu* help = bar->addMenu(L"帮助");
    QAction* aboutAct = help->addAction(L"关于");
    QObject::connect(aboutAct, aboutAct->triggered, this, &DesignerWindow::onAbout);

    QToolBar* toolbar = new QToolBar();
    addToolBar(toolbar);
    toolbar->addAction(newAct);
    toolbar->addAction(saveAct);
    toolbar->addSeparator();
    toolbar->addAction(undoAct);
    toolbar->addAction(redoAct);
    toolbar->addSeparator();
    toolbar->addAction(codeAct);
    toolbar->addAction(previewAct);
}

void DesignerWindow::buildUi() {
    QSplitter* main = new QSplitter(Qt::Horizontal);

    toolbox_ = new QTreeWidget();
    toolbox_->setHeaderLabels(QStringList() << L"工具箱");
    QList<QWidgetClassInfo> classes = QWidgetFactory::classes();
    QStringList groups;
    for (int i = 0; i < classes.count(); ++i) {
        QString group = QString::fromUtf8(classes[i].group);
        if (groups.indexOf(group) < 0) groups.append(group);
    }
    for (int g = 0; g < groups.count(); ++g) {
        QTreeWidgetItem* groupItem = new QTreeWidgetItem(QStringList() << groups[g]);
        groupItem->setExpanded(true);
        for (int i = 0; i < classes.count(); ++i) {
            if (QString::fromUtf8(classes[i].group) != groups[g]) continue;
            QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::fromUtf8(classes[i].label));
            item->setData(0, Qt::UserRole, QVariant(QString::fromUtf8(classes[i].className)));
            groupItem->addChild(item);
        }
        toolbox_->addTopLevelItem(groupItem);
    }
    QObject::connect(toolbox_, toolbox_->itemDoubleClicked, this, &DesignerWindow::onToolboxActivated);

    canvas_ = new DesignerCanvas(&doc_);
    canvas_->selectionChanged.connect(this, this, &DesignerWindow::onSelectionChanged);
    canvas_->documentModified.connect(this, this, &DesignerWindow::onDocumentModified);

    QSplitter* right = new QSplitter(Qt::Vertical);
    tree_ = new QTreeWidget();
    tree_->setHeaderLabels(QStringList() << L"对象");
    QObject::connect(tree_, tree_->itemClicked, this, &DesignerWindow::onTreeClicked);
    properties_ = new PropertyEditor();
    properties_->propertyChanged.connect(this, this, &DesignerWindow::onPropertyChanged);
    right->addWidget(tree_);
    right->addWidget(properties_);
    right->setSizes(QList<int>() << 220 << 420);

    main->addWidget(toolbox_);
    main->addWidget(canvas_);
    main->addWidget(right);
    main->setSizes(QList<int>() << 190 << 660 << 330);
    setCentralWidget(main);

    status_ = new QStatusBar();
    setStatusBar(status_);
    status_->showMessage(L"就绪 - 双击工具箱添加控件，拖动移动，拖动手柄缩放");
}

void DesignerWindow::setStatus(const QString& text) {
    if (status_) status_->showMessage(text, 5000);
}

void DesignerWindow::newDocument() {
    doc_.reset(L"Form1");
    canvas_->rebuild();
    refreshInspector();
    refreshProperties();
    setStatus(L"新建文档");
}

void DesignerWindow::openDocument(const QString& path) {
    bool ok = false;
    QString xml = uicReadFile(path, &ok);
    if (!ok) {
        QMessageBox::warning(this, L"打开失败", L"无法读取文件：" + path);
        return;
    }
    QString error;
    QXmlNode* root = QXmlNode::parse(xml, &error);
    if (!root) {
        QMessageBox::warning(this, L"打开失败", L"XML 解析错误：" + error);
        return;
    }
    delete root;
    doc_.setXml(xml);
    doc_.setFilePath(path);
    QString base = path;
    int slash = base.find_last_of(L"/\\");
    if (slash >= 0) base = base.mid(slash + 1);
    int dot = base.find_last_of(L'.');
    if (dot > 0) base = base.left(dot);
    if (!base.isEmpty()) doc_.setFormClass(base);
    canvas_->rebuild();
    refreshInspector();
    refreshProperties();
    setStatus(L"已打开 " + path);
}

bool DesignerWindow::saveDocument(const QString& path) {
    if (!uicWriteFile(path, doc_.xml())) {
        QMessageBox::warning(this, L"保存失败", L"无法写入文件：" + path);
        return false;
    }
    doc_.setFilePath(path);
    doc_.setModified(false);
    setStatus(L"已保存 " + path);
    return true;
}

void DesignerWindow::generateCode(const QString& dir) {
    QString error;
    QString header = uicGenerateHeader(doc_.xml(), doc_.formClass(), &error);
    if (header.isEmpty()) {
        QMessageBox::warning(this, L"生成失败", error);
        return;
    }
    QString outPath = dir;
    if (!outPath.isEmpty() && outPath[outPath.size() - 1] != L'/' && outPath[outPath.size() - 1] != L'\\')
        outPath += L"/";
    outPath += L"ui_" + doc_.formClass().toLower() + L".h";
    if (!uicWriteFile(outPath, header)) {
        QMessageBox::warning(this, L"生成失败", L"无法写入 " + outPath);
        return;
    }
    setStatus(L"代码已生成: " + outPath);
    QMessageBox::information(this, L"生成代码", L"已生成 UI 头文件：\n" + outPath);
}

void DesignerWindow::preview() {
    QString error;
    QXmlNode* root = QXmlNode::parse(doc_.xml(), &error);
    if (!root) return;
    QXmlNode* top = root->firstChild(L"widget");
    QWidget* form = new QWidget(0, Qt::Window);
    form->setWindowTitle(doc_.formClass() + L" - Preview");
    form->resize(640, 480);
    if (top) {
        QList<QXmlNode*> props = top->childrenNamed(L"property");
        for (int i = 0; i < props.count(); ++i) {
            QString name = props[i]->attribute(L"name");
            if (name == L"geometry") {
                QVariant v = variantFromXml(L"QRect", props[i]);
                if (v.isValid()) form->resize(v.toRect().size());
            } else if (name == L"windowTitle") {
                QVariant v = variantFromXml(L"QString", props[i]);
                if (v.isValid()) form->setWindowTitle(v.toString());
            }
        }
        QList<QXmlNode*> children = top->childrenNamed(L"widget");
        for (int i = 0; i < children.count(); ++i) buildWidgetTree(children[i], form);
    }
    delete root;
    form->show();
    setStatus(L"预览窗口已打开（不含事件处理代码）");
}

void DesignerWindow::refreshInspector() {
    updatingInspector_ = true;
    tree_->clear();
    if (!canvas_->form()) { updatingInspector_ = false; return; }
    QTreeWidgetItem* root = new QTreeWidgetItem(QStringList() << canvas_->form()->objectName());
    root->setData(0, Qt::UserRole, QVariant());
    tree_->addTopLevelItem(root);
    QList<QWidget*> children = canvas_->form()->childrenWidgets();
    for (int i = 0; i < children.count(); ++i) {
        QTreeWidgetItem* child = new QTreeWidgetItem(QStringList() << children[i]->objectName());
        root->addChild(child);
        QList<QWidget*> grandchildren = children[i]->childrenWidgets();
        for (int g = 0; g < grandchildren.count(); ++g)
            child->addChild(new QTreeWidgetItem(QStringList() << grandchildren[g]->objectName()));
    }
    root->setExpanded(true);
    updatingInspector_ = false;
}

void DesignerWindow::refreshProperties() {
    QWidget* selected = canvas_->selected();
    if (selected) properties_->setTarget(selected);
    else properties_->setTarget(canvas_->form());
}

void DesignerWindow::onSelectionChanged(QWidget* widget) {
    refreshProperties();
    if (!widget) {
        setStatus(L"未选中任何控件");
        return;
    }
    setStatus(L"已选中 " + widget->objectName() + L" (" + QString(widget->className()) + L")");
}

void DesignerWindow::onDocumentModified() {
    refreshInspector();
    refreshProperties();
    doc_.setModified(true);
}

void DesignerWindow::onPropertyChanged(QWidget* widget, const char* name, const QVariant&) {
    if (widget && strcmp(name, "geometry") == 0) canvas_->update();
    canvas_->captureToDocument();
}

void DesignerWindow::onToolboxActivated(QTreeWidgetItem* item, int) {
    if (!item) return;
    QVariant data = item->data(0, Qt::UserRole);
    if (!data.isValid()) return;
    addWidgetByClass(data.toString());
}

void DesignerWindow::addWidgetByClass(const QString& className) {
    QWidget* base = canvas_->form();
    QWidget* parent = canvas_->selected();
    if (!parent || parent == base) parent = base;
    QWidget* widget = QWidgetFactory::create(className.toUtf8().c_str());
    if (!widget) return;
    widget->setParentWidget(parent);
    widget->setObjectName(uniqueName(base, className.toLower()));
    widget->resize(widget->sizeHint());
    QPoint offset(24, 24);
    QWidget* top = widget->parentWidget();
    while (top && top != base) {
        offset += top->pos();
        top = top->parentWidget();
    }
    widget->move(offset);
    widget->show();
    canvas_->select(widget);
    canvas_->captureToDocument();
    refreshInspector();
    refreshProperties();
    setStatus(L"已添加 " + className + L" (" + widget->objectName() + L")");
}

void DesignerWindow::onTreeClicked(QTreeWidgetItem* item, int) {
    if (updatingInspector_ || !item) return;
    QString name = item->text(0);
    if (name == canvas_->form()->objectName()) {
        canvas_->select(0);
        return;
    }
    QWidget* widget = canvas_->form()->findChild<QWidget>(name);
    if (widget) canvas_->select(widget);
}

void DesignerWindow::onNew() { newDocument(); }
void DesignerWindow::onOpen() {
    QString path = QFileDialog::getOpenFileName(this, L"打开设计文件", QString(),
                                                L"QtClone UI (*.ui);;所有文件 (*.*)");
    if (!path.isEmpty()) openDocument(path);
}
void DesignerWindow::onSave() {
    if (doc_.filePath().isEmpty()) onSaveAs();
    else saveDocument(doc_.filePath());
}
void DesignerWindow::onSaveAs() {
    QString path = QFileDialog::getSaveFileName(this, L"保存设计文件", QString(),
                                                L"QtClone UI (*.ui);;所有文件 (*.*)");
    if (!path.isEmpty()) {
        if (!path.endsWith(L".ui")) path += L".ui";
        saveDocument(path);
    }
}
void DesignerWindow::onGenerate() {
    QString dir = doc_.filePath();
    int slash = dir.find_last_of(L"/\\");
    if (slash >= 0) dir = dir.left(slash);
    else dir = L".";
    generateCode(dir);
}
void DesignerWindow::onUndo() {
    if (doc_.undo()) {
        canvas_->rebuild();
        refreshInspector();
        refreshProperties();
        setStatus(L"撤销");
    }
}
void DesignerWindow::onRedo() {
    if (doc_.redo()) {
        canvas_->rebuild();
        refreshInspector();
        refreshProperties();
        setStatus(L"重做");
    }
}
void DesignerWindow::onDelete() { canvas_->deleteSelected(); refreshProperties(); }
void DesignerWindow::onDuplicate() { canvas_->duplicateSelected(); refreshInspector(); }
void DesignerWindow::onToggleGrid() {
    canvas_->setGridVisible(!canvas_->gridVisible());
}
void DesignerWindow::onThemeLight() { QTheme::setMode(QTheme::Light); }
void DesignerWindow::onThemeDark() { QTheme::setMode(QTheme::Dark); }
void DesignerWindow::onThemeSystem() { QTheme::setMode(QTheme::System); }
void DesignerWindow::onPreview() { preview(); }
void DesignerWindow::onAbout() {
    QMessageBox::about(this, L"关于 QtClone Designer",
                       L"QtClone Designer 1.0\n"
                       L"Qt 风格 C++ 界面设计器：绝对布局、属性编辑、.ui 存取、代码生成。\n"
                       L"无 moc、无第三方依赖，支持 VS2010。");
}
void DesignerWindow::onQuit() { QApplication::quit(); }

int DesignerWindow::selfTest() {
    int failures = 0;
    newDocument();
    canvas_->select(0);

    // add a few widgets
    addWidgetByClass(L"QPushButton");
    QWidget* button = canvas_->selected();
    if (!button) { printf("FAIL: no widget added\n"); ++failures; }
    if (button) {
        canvas_->applyProperty(button, "text", QVariant(QString(L"OK")));
        canvas_->applyProperty(button, "geometry", QVariant(QRect(30, 40, 120, 32)));
    }
    canvas_->select(0);
    addWidgetByClass(L"QLabel");
    QWidget* label = canvas_->selected();
    if (label) canvas_->applyProperty(label, "text", QVariant(QString(L"标题")));

    QString xml = doc_.xml();
    if (xml.indexOf(L"<widget class=\"QPushButton\"") < 0) { printf("FAIL: button not serialized\n"); ++failures; }
    if (xml.indexOf(L"geometry") < 0) { printf("FAIL: geometry missing\n"); ++failures; }

    // round trip
    DesignerDocument copy;
    copy.setXml(xml);
    if (copy.xml() != xml) { printf("FAIL: roundtrip\n"); ++failures; }

    // code generation
    QString error;
    QString header = uicGenerateHeader(xml, doc_.formClass(), &error);
    if (header.isEmpty()) { printf("FAIL: uic: %ls\n", error.c_str()); ++failures; }
    if (header.indexOf(L"Ui_Form1") < 0) { printf("FAIL: uic class\n"); ++failures; }
    if (header.indexOf(L"QPushButton* pushbutton1") < 0 && header.indexOf(L"->setGeometry") < 0) {
        printf("FAIL: uic body\n");
        ++failures;
    }

    // undo / redo
    int before = canvas_->form()->childrenWidgets().count();
    onDelete();
    int afterDelete = canvas_->form()->childrenWidgets().count();
    if (afterDelete != before - 1) { printf("FAIL: delete\n"); ++failures; }
    onUndo();
    int afterUndo = canvas_->form()->childrenWidgets().count();
    if (afterUndo != before) { printf("FAIL: undo restored %d (expected %d)\n", afterUndo, before); ++failures; }
    onRedo();
    int afterRedo = canvas_->form()->childrenWidgets().count();
    if (afterRedo != before - 1) { printf("FAIL: redo\n"); ++failures; }

    printf("designer selftest: %s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures;
}

bool DesignerWindow::screenshot(const QString& path) {
    QPixmap pm = grab();
    return pm.save(path);
}

// ---------------------------------------------------------------------------
// entry point
// ---------------------------------------------------------------------------
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    DesignerWindow window;
    window.show();

    QString openPath;
    QString screenshotPath;
    QString theme;
    bool selfTest = false;
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromUtf8(argv[i]);
        if (arg == L"--open" && i + 1 < argc) openPath = QString::fromUtf8(argv[++i]);
        else if (arg == L"--screenshot" && i + 1 < argc) screenshotPath = QString::fromUtf8(argv[++i]);
        else if (arg == L"--theme" && i + 1 < argc) theme = QString::fromUtf8(argv[++i]);
        else if (arg == L"--selftest") selfTest = true;
    }

    if (!theme.isEmpty()) {
        if (theme == L"dark") QTheme::setMode(QTheme::Dark);
        else if (theme == L"light") QTheme::setMode(QTheme::Light);
        else QTheme::setMode(QTheme::System);
    }
    if (!openPath.isEmpty()) window.openDocument(openPath);

    for (int i = 0; i < 10; ++i) QApplication::processEvents();

    if (selfTest) {
        int failures = window.selfTest();
        return failures ? 1 : 0;
    }
    if (!screenshotPath.isEmpty()) {
        bool ok = window.screenshot(screenshotPath);
        printf("designer screenshot %s -> %ls\n", ok ? "saved" : "FAILED", screenshotPath.c_str());
        return ok ? 0 : 1;
    }
    return QApplication::exec();
}
