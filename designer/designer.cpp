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
#include <functional>

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

QWidget* findWidgetByName(QWidget* root, const QString& name) {
    if (!root) return 0;
    QList<QWidget*> children = root->childrenWidgets();
    for (int i = 0; i < children.count(); ++i) {
        if (children[i]->objectName() == name) return children[i];
        QWidget* deeper = findWidgetByName(children[i], name);
        if (deeper) return deeper;
    }
    return 0;
}

QString uniqueName(QWidget* form, const QString& base) {
    for (int index = 1;; ++index) {
        QString candidate = base + QString::number(index);
        if (!findWidgetByName(form, candidate)) return candidate;
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
    widget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
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
// MenuRelay : forwards QMenu::exec selection to a callback
// ---------------------------------------------------------------------------
class MenuRelay : public QObject {
public:
    std::function<void(QAction*)> fn;
    void onTriggered(QAction* action) {
        if (fn) fn(action);
    }
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

class DesignerCanvas;

// ---------------------------------------------------------------------------
// DesignerViewport : clipped, mouse-transparent container for the form. Moving
// the form to negative coordinates for scrolling never bleeds outside it.
// ---------------------------------------------------------------------------
class DesignerViewport : public QWidget {
public:
    explicit DesignerViewport(QWidget* parent) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }

protected:
    void paintEvent(QPaintEvent*) {}
};

// ---------------------------------------------------------------------------
// SelectionOverlay : draws hover/selection adornments above the widgets
// ---------------------------------------------------------------------------
class SelectionOverlay : public QWidget {
public:
    explicit SelectionOverlay(DesignerCanvas* canvas);

protected:
    void paintEvent(QPaintEvent*);

private:
    DesignerCanvas* canvas_;
};

// ---------------------------------------------------------------------------
// DesignerCanvas
// ---------------------------------------------------------------------------
class DesignerCanvas : public QWidget {
public:
    DesignerCanvas(DesignerDocument* doc);

    QWidget* form() const { return form_; }
    QWidget* selected() const { return selected_; }
    QList<QWidget*> selection() const { return selection_; }
    void setSelectionList(const QList<QWidget*>& widgets);
    bool gridVisible() const { return grid_; }
    void setGridVisible(bool visible) { grid_ = visible; update(); }

    void rebuild();
    void select(QWidget* widget, bool notify = true);
    void deleteSelected();
    void duplicateSelected();
    void selectAll();
    void alignSelection(int mode);
    void distributeSelection(bool horizontal);
    void scrollByPage(int pages);
    void scrollHome();
    enum AlignMode { AlignLeft, AlignRight, AlignTop, AlignBottom, AlignCenterH, AlignCenterV };
    void centerSelected(bool horizontal) { centerSelection(horizontal); }
    void raiseSelected();
    void lowerSelected();
    void captureToDocument();
    void applyProperty(QWidget* widget, const char* name, const QVariant& value);
    void updateAdornments();
    void paintAdornments(QPainter& p);
    void syncScroll();
    void scrollTo(int x, int y);
    void ensureVisible(QWidget* widget);

    ~DesignerCanvas();

    Signal<void(QWidget*)> selectionChanged;
    Signal<void()> documentModified;
    Signal<void(QWidget*)> widgetActivated;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);
    void leaveEvent(QEvent* event);

private:
    enum Mode { Idle, Moving, Resizing, Rubber };
    QRect handlesRect(QList<QRect>* handles) const;
    int handleAt(const QPoint& pos) const;
    QWidget* widgetAt(const QPoint& pos, QWidget* root) const;
    QPoint snapPoint(const QPoint& p) const;
    void updateHover(const QPoint& pos);
    void showContextMenu(const QPoint& globalPos);
    void centerSelection(bool horizontal);
    void cycleSelection(int step);
    bool isSelected(QWidget* widget) const;
    void toggleSelection(QWidget* widget);
    void applySelectionMove(const QPoint& delta);
    void finishRubberBand(const QRect& band, bool additive);
    static Qt::CursorShape cursorForHandle(int handle);

    friend class SelectionOverlay;

    DesignerDocument* doc_;
    QWidget* form_;
    QWidget* selected_;
    QList<QWidget*> selection_;
    QList<QWidget*> dragWidgets_;
    QList<QRect> dragStarts_;
    QRect bandRect_;
    bool bandAdditive_;
    QWidget* hover_;
    SelectionOverlay* overlay_;
    QMenu* contextMenu_;
    MenuRelay* menuRelay_;
    DesignerViewport* viewport_;
    int scrollX_;
    int scrollY_;
    int maxScrollX_;
    int maxScrollY_;
    int hoverHandle_;
    bool grid_;
    Mode mode_;
    int handle_;
    QPoint dragStart_;
    QRect startGeometry_;
};

DesignerCanvas::DesignerCanvas(DesignerDocument* doc)
    : QWidget(0), doc_(doc), form_(0), selected_(0), hover_(0), overlay_(0), contextMenu_(0),
      menuRelay_(0), viewport_(0), scrollX_(0), scrollY_(0), maxScrollX_(0), maxScrollY_(0),
      hoverHandle_(-1), grid_(true), mode_(Idle), handle_(-1) {
    bandRect_ = QRect();
    bandAdditive_ = false;
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    viewport_ = new DesignerViewport(this);
    viewport_->setGeometry(rect());
    viewport_->show();

    form_ = new DesignerFormWidget(viewport_);
    form_->setObjectName(doc_->formClass());
    form_->setGeometry(QRect(20, 20, 640, 480));
    form_->setObjectName(doc_->formClass());
    form_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    form_->show();
    overlay_ = new SelectionOverlay(this);
    overlay_->setGeometry(rect());
    overlay_->show();

    contextMenu_ = new QMenu(0);
    menuRelay_ = new MenuRelay();
    menuRelay_->fn = [this](QAction* action) {
        if (!action) return;
        int index = contextMenu_->actions().indexOf(action);
        if (index == 0) deleteSelected();
        else if (index == 1) duplicateSelected();
        else if (index == 3) raiseSelected();
        else if (index == 4) lowerSelected();
        else if (index == 6) centerSelection(true);
        else if (index == 7) centerSelection(false);
        else if (index == 9) alignSelection(AlignLeft);
        else if (index == 10) alignSelection(AlignRight);
        else if (index == 11) alignSelection(AlignTop);
        else if (index == 12) alignSelection(AlignBottom);
        else if (index == 14) distributeSelection(true);
        else if (index == 15) distributeSelection(false);
        else if (index == 17) selectAll();
        contextMenu_->hide();
        setFocus(Qt::MouseFocusReason);
    };
    contextMenu_->triggeredAction.connect(contextMenu_, menuRelay_, &MenuRelay::onTriggered);

    rebuild();
}

DesignerCanvas::~DesignerCanvas() {
    delete menuRelay_;
    delete contextMenu_;
}

SelectionOverlay::SelectionOverlay(DesignerCanvas* canvas)
    : QWidget(canvas), canvas_(canvas) {
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void SelectionOverlay::paintEvent(QPaintEvent*) {
    QPainter p(this);
    canvas_->paintAdornments(p);
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
    syncScroll();
    if (overlay_) overlay_->raise();
    updateAdornments();
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
    int s = 9;
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

void DesignerCanvas::setSelectionList(const QList<QWidget*>& widgets) {
    selection_ = widgets;
    selected_ = widgets.isEmpty() ? 0 : widgets.last();
    updateAdornments();
    update();
    selectionChanged(selected_);
}

bool DesignerCanvas::isSelected(QWidget* widget) const {
    return widget && selection_.indexOf(widget) >= 0;
}

void DesignerCanvas::toggleSelection(QWidget* widget) {
    if (!widget) return;
    QList<QWidget*> list = selection_;
    if (list.indexOf(widget) >= 0) list.removeAll(widget);
    else list.append(widget);
    setSelectionList(list);
}

void DesignerCanvas::select(QWidget* widget, bool notify) {
    QList<QWidget*> list;
    if (widget) list.append(widget);
    selection_ = list;
    selected_ = widget;
    if (notify && list.count() == 1) ensureVisible(widget);
    updateAdornments();
    update();
    if (notify) selectionChanged(selected_);
}

void DesignerCanvas::selectAll() {
    setSelectionList(form_->childrenWidgets());
}

void DesignerCanvas::deleteSelected() {
    if (selection_.isEmpty()) return;
    QList<QWidget*> victims = selection_;
    selection_.clear();
    selected_ = 0;
    for (int i = 0; i < victims.count(); ++i) {
        victims[i]->setParentWidget(0);
        delete victims[i];
    }
    captureToDocument();
    updateAdornments();
    update();
    selectionChanged(0);
}

void DesignerCanvas::duplicateSelected() {
    if (selection_.isEmpty()) return;
    QList<QWidget*> sources = selection_;
    QList<QWidget*> copies;
    for (int i = 0; i < sources.count(); ++i) {
        QString xml = captureWidgetXml(sources[i]);
        QXmlNode* node = QXmlNode::parse(xml, 0);
        if (!node) continue;
        QWidget* copy = buildWidgetTree(node, form_);
        delete node;
        if (!copy) continue;
        copy->setObjectName(uniqueName(form_, QString(copy->className()).toLower()));
        copy->setGeometry(sources[i]->geometry().translated(16, 16));
        copy->show();
        copies.append(copy);
    }
    if (copies.isEmpty()) return;
    setSelectionList(copies);
    captureToDocument();
}

void DesignerCanvas::raiseSelected() {
    if (selection_.isEmpty()) return;
    for (int i = 0; i < selection_.count(); ++i) selection_[i]->raise();
    captureToDocument();
    update();
}

void DesignerCanvas::lowerSelected() {
    if (selection_.isEmpty()) return;
    for (int i = 0; i < selection_.count(); ++i) selection_[i]->lower();
    captureToDocument();
    update();
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
}

void DesignerCanvas::paintAdornments(QPainter& p) {
    if (maxScrollX_ > 0) {
        int trackW = width() - 6, y = height() - 6;
        int thumbW = qMax(24, (width() - 6) * viewport_->width() / (form_->width() + 40));
        int thumbX = 3 + (trackW - thumbW) * scrollX_ / qMax(1, maxScrollX_);
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(0x00, 0x00, 0x00, 36)));
        p.drawRect(QRect(3, y, trackW, 4));
        p.setBrush(QBrush(QTheme::colors().accent));
        p.drawRect(QRect(thumbX, y, thumbW, 4));
    }
    if (maxScrollY_ > 0) {
        int trackH = height() - 6, x = width() - 6;
        int thumbH = qMax(24, (height() - 6) * viewport_->height() / (form_->height() + 40));
        int thumbY = 3 + (trackH - thumbH) * scrollY_ / qMax(1, maxScrollY_);
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(0x00, 0x00, 0x00, 36)));
        p.drawRect(QRect(x, 3, 4, trackH));
        p.setBrush(QBrush(QTheme::colors().accent));
        p.drawRect(QRect(x, thumbY, 4, thumbH));
    }
    if (hover_ && !isSelected(hover_)) {
        QRect h = hover_->geometry().translated(form_->pos());
        p.setPen(QPen(QColor(0x00, 0x78, 0xD4, 120), 1, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(h);
    }

    if (!bandRect_.isNull()) {
        p.setPen(QPen(QTheme::colors().accent, 1, Qt::DashLine));
        p.setBrush(QBrush(QColor(0x00, 0x78, 0xD4, 24)));
        p.drawRect(bandRect_);
    }

    bool single = selection_.count() == 1;
    for (int i = 0; i < selection_.count(); ++i) {
        QRect r = selection_[i]->geometry().translated(form_->pos());
        p.setPen(QPen(QTheme::colors().accent, i + 1 == selection_.count() ? 1 : 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(r);
        if (single) {
            QList<QRect> handles;
            handlesRect(&handles);
            p.setBrush(QBrush(QColor(0xFF, 0xFF, 0xFF)));
            for (int h = 0; h < handles.count(); ++h) p.drawRect(handles[h]);
        }
    }
}

Qt::CursorShape DesignerCanvas::cursorForHandle(int handle) {
    switch (handle) {
    case 0: case 7: return Qt::SizeFDiagCursor;
    case 2: case 5: return Qt::SizeBDiagCursor;
    case 1: case 6: return Qt::SizeVerCursor;
    case 3: case 4: return Qt::SizeHorCursor;
    default: return Qt::ArrowCursor;
    }
}

void DesignerCanvas::updateHover(const QPoint& pos) {
    if (mode_ != Idle) return;
    int handle = selected_ ? handleAt(pos) : -1;
    QWidget* hit = 0;
    if (handle < 0) hit = widgetAt(pos - form_->pos(), form_);
    Qt::CursorShape shape = Qt::ArrowCursor;
    if (handle >= 0) shape = cursorForHandle(handle);
    else if (hit) shape = Qt::SizeAllCursor;
    setCursor(QCursor(shape));
    if (hit != hover_ || handle != hoverHandle_) {
        hover_ = hit;
        hoverHandle_ = handle;
        updateAdornments();
        update();
    }
}

void DesignerCanvas::leaveEvent(QEvent* event) {
    hover_ = 0;
    hoverHandle_ = -1;
    QWidget::leaveEvent(event);
    updateAdornments();
    update();
}

void DesignerCanvas::cycleSelection(int step) {
    QList<QWidget*> children = form_->childrenWidgets();
    if (children.isEmpty()) return;
    int index = children.indexOf(selected_);
    index += step;
    if (index < 0) index = children.count() - 1;
    if (index >= children.count()) index = 0;
    select(children[index]);
}

void DesignerCanvas::centerSelection(bool horizontal) {
    if (selection_.isEmpty()) return;
    QRect bounds;
    for (int i = 0; i < selection_.count(); ++i)
        bounds = bounds.isNull() ? selection_[i]->geometry()
                                 : bounds.united(selection_[i]->geometry());
    int dx = 0, dy = 0;
    if (horizontal) dx = (form_->width() - bounds.width()) / 2 - bounds.left();
    else dy = (form_->height() - bounds.height()) / 2 - bounds.top();
    if (dx == 0 && dy == 0) return;
    for (int i = 0; i < selection_.count(); ++i) {
        QRect g = selection_[i]->geometry();
        selection_[i]->setGeometry(g.translated(dx, dy));
    }
    captureToDocument();
    updateAdornments();
    update();
}

void DesignerCanvas::applySelectionMove(const QPoint& delta) {
    if (delta.isNull()) return;
    for (int i = 0; i < dragWidgets_.count(); ++i)
        dragWidgets_[i]->setGeometry(dragStarts_[i].translated(delta));
}

void DesignerCanvas::alignSelection(int mode) {
    if (selection_.count() < 2) return;
    QRect bounds;
    for (int i = 0; i < selection_.count(); ++i)
        bounds = bounds.isNull() ? selection_[i]->geometry()
                                 : bounds.united(selection_[i]->geometry());
    for (int i = 0; i < selection_.count(); ++i) {
        QRect g = selection_[i]->geometry();
        switch (mode) {
        case AlignLeft: g.moveLeft(bounds.left()); break;
        case AlignRight: g.moveLeft(bounds.right() - g.width() + 1); break;
        case AlignTop: g.moveTop(bounds.top()); break;
        case AlignBottom: g.moveTop(bounds.bottom() - g.height() + 1); break;
        case AlignCenterH: g.moveLeft(bounds.center().x() - g.width() / 2); break;
        case AlignCenterV: g.moveTop(bounds.center().y() - g.height() / 2); break;
        default: break;
        }
        selection_[i]->setGeometry(g);
    }
    captureToDocument();
    updateAdornments();
    update();
}

void DesignerCanvas::distributeSelection(bool horizontal) {
    if (selection_.count() < 3) return;
    QList<QWidget*> items = selection_;
    // insertion sort by leading edge
    for (int i = 1; i < items.count(); ++i) {
        QWidget* key = items[i];
        int keyEdge = horizontal ? key->geometry().left() : key->geometry().top();
        int j = i - 1;
        while (j >= 0) {
            int edge = horizontal ? items[j]->geometry().left() : items[j]->geometry().top();
            if (edge <= keyEdge) break;
            items[j + 1] = items[j];
            --j;
        }
        items[j + 1] = key;
    }
    int first = horizontal ? items.first()->geometry().left() : items.first()->geometry().top();
    QWidget* lastItem = items.last();
    int lastEdge = horizontal ? (lastItem->geometry().right() + 1) : (lastItem->geometry().bottom() + 1);
    int total = 0;
    for (int i = 0; i < items.count(); ++i)
        total += horizontal ? items[i]->width() : items[i]->height();
    int gap = (lastEdge - first - total) / (items.count() - 1);
    int cursor = first;
    for (int i = 0; i < items.count(); ++i) {
        QRect g = items[i]->geometry();
        if (horizontal) g.moveLeft(cursor);
        else g.moveTop(cursor);
        items[i]->setGeometry(g);
        cursor += (horizontal ? g.width() : g.height()) + gap;
    }
    captureToDocument();
    updateAdornments();
    update();
}

void DesignerCanvas::finishRubberBand(const QRect& band, bool additive) {
    QList<QWidget*> picked;
    if (additive) picked = selection_;
    QList<QWidget*> children = form_->childrenWidgets();
    for (int i = 0; i < children.count(); ++i) {
        QRect r = children[i]->geometry().translated(form_->pos());
        if (!band.intersects(r)) continue;
        if (picked.indexOf(children[i]) < 0) picked.append(children[i]);
    }
    setSelectionList(picked);
}

void DesignerCanvas::showContextMenu(const QPoint& globalPos) {
    contextMenu_->clear();
    contextMenu_->addAction(QString(L"\u5220\u9664"));
    contextMenu_->addAction(QString(L"\u590d\u5236\u4e00\u4efd"));
    contextMenu_->addSeparator();
    contextMenu_->addAction(QString(L"\u7f6e\u4e8e\u9876\u5c42"));
    contextMenu_->addAction(QString(L"\u7f6e\u4e8e\u5e95\u5c42"));
    contextMenu_->addSeparator();
    contextMenu_->addAction(QString(L"\u6c34\u5e73\u5c45\u4e2d"));
    contextMenu_->addAction(QString(L"\u5782\u76f4\u5c45\u4e2d"));
    contextMenu_->addSeparator();
    contextMenu_->addAction(QString(L"\u5de6\u5bf9\u9f50"));
    contextMenu_->addAction(QString(L"\u53f3\u5bf9\u9f50"));
    contextMenu_->addAction(QString(L"\u9876\u5bf9\u9f50"));
    contextMenu_->addAction(QString(L"\u5e95\u5bf9\u9f50"));
    contextMenu_->addSeparator();
    contextMenu_->addAction(QString(L"\u6c34\u5e73\u5206\u5e03"));
    contextMenu_->addAction(QString(L"\u5782\u76f4\u5206\u5e03"));
    contextMenu_->addSeparator();
    contextMenu_->addAction(QString(L"\u5168\u9009"));
    contextMenu_->popup(globalPos);
}

void DesignerCanvas::mousePressEvent(QMouseEvent* event) {
    if (contextMenu_ && contextMenu_->isVisible() && event->button() == Qt::LeftButton)
        contextMenu_->hide();
    setFocus(Qt::MouseFocusReason);

    if (event->button() == Qt::RightButton) {
        QWidget* hit = widgetAt(event->pos() - form_->pos(), form_);
        if (hit && !isSelected(hit)) select(hit);
        showContextMenu(event->globalPos());
        return;
    }
    if (event->button() != Qt::LeftButton) return;

    if (maxScrollY_ > 0 && event->pos().x() >= width() - 8) {
        scrollTo(scrollX_, scrollY_ + (event->pos().y() < height() / 2 ? -viewport_->height()
                                                                        : viewport_->height()));
        return;
    }
    if (maxScrollX_ > 0 && event->pos().y() >= height() - 8) {
        scrollTo(scrollX_ + (event->pos().x() < width() / 2 ? -viewport_->width()
                                                             : viewport_->width()),
                 scrollY_);
        return;
    }

    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;

    int handle = (selection_.count() == 1 && selected_) ? handleAt(event->pos()) : -1;
    if (handle >= 0) {
        mode_ = Resizing;
        handle_ = handle;
        dragStart_ = event->pos();
        startGeometry_ = selected_->geometry();
        return;
    }

    QWidget* hit = widgetAt(event->pos() - form_->pos(), form_);
    if (!hit) {
        if (!ctrl && !shift) select(0, true);
        bandAdditive_ = shift;
        mode_ = Rubber;
        dragStart_ = event->pos();
        bandRect_ = QRect(dragStart_, dragStart_);
        updateAdornments();
        return;
    }

    if (ctrl) {
        toggleSelection(hit);
        mode_ = Idle;
        return;
    }
    if (shift && !isSelected(hit)) {
        QList<QWidget*> list = selection_;
        list.append(hit);
        setSelectionList(list);
    } else if (!isSelected(hit)) {
        select(hit, true);
    }

    dragWidgets_ = selection_;
    dragStarts_.clearAll();
    for (int i = 0; i < dragWidgets_.count(); ++i)
        dragStarts_.append(dragWidgets_[i]->geometry());
    mode_ = Moving;
    handle_ = -1;
    dragStart_ = event->pos();
}

void DesignerCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (mode_ == Idle) {
        updateHover(event->pos());
        return;
    }
    if (mode_ == Rubber) {
        bandRect_ = QRect(dragStart_, event->pos()).normalized();
        updateAdornments();
        return;
    }
    if (selection_.isEmpty()) return;

    QPoint delta = event->pos() - dragStart_;
    if (mode_ == Moving) {
        int index = dragWidgets_.indexOf(selected_);
        QRect primaryStart = index >= 0 ? dragStarts_[index] : dragStarts_.first();
        QPoint target = snapPoint(primaryStart.topLeft() + delta);
        QPoint move = target - primaryStart.topLeft();

        int minLeft = 1 << 30, minTop = 1 << 30, maxRight = -1, maxBottom = -1;
        for (int i = 0; i < dragStarts_.count(); ++i) {
            minLeft = qMin(minLeft, dragStarts_[i].left());
            minTop = qMin(minTop, dragStarts_[i].top());
            maxRight = qMax(maxRight, dragStarts_[i].right());
            maxBottom = qMax(maxBottom, dragStarts_[i].bottom());
        }
        move.setX(qBound(-minLeft, move.x(), qMax(-minLeft, form_->width() - 1 - maxRight)));
        move.setY(qBound(-minTop, move.y(), qMax(-minTop, form_->height() - 1 - maxBottom)));
        // Shift constrains a drag to the dominant axis.
        if (event->modifiers() & Qt::ShiftModifier) {
            if (qAbs(move.x()) >= qAbs(move.y())) move.setY(0);
            else move.setX(0);
        }
        applySelectionMove(move);
    } else if (mode_ == Resizing) {
        QRect g = startGeometry_;
        QPoint snapped = snapPoint(startGeometry_.topLeft() + delta) - startGeometry_.topLeft();
        switch (handle_) {
        case 0: g.setTopLeft(g.topLeft() + snapped); break;
        case 1: g.setTop(g.top() + snapped.y()); break;
        case 2: g.setTopRight(g.topRight() + snapped); break;
        case 3: g.setLeft(g.left() + snapped.x()); break;
        case 4: g.setRight(g.right() + snapped.x()); break;
        case 5: g.setBottomLeft(g.bottomLeft() + snapped); break;
        case 6: g.setBottom(g.bottom() + snapped.y()); break;
        case 7: g.setBottomRight(g.bottomRight() + snapped); break;
        default: break;
        }
        int needW = qMax(16, selected_->minimumSize().width());
        int needH = qMax(16, selected_->minimumSize().height());
        if (g.width() < needW) {
            if (handle_ == 0 || handle_ == 3 || handle_ == 5) g.setLeft(g.right() - needW + 1);
            else g.setRight(g.left() + needW - 1);
        }
        if (g.height() < needH) {
            if (handle_ == 0 || handle_ == 1 || handle_ == 2) g.setTop(g.bottom() - needH + 1);
            else g.setBottom(g.top() + needH - 1);
        }
        if (g.left() < 0) g.setLeft(0);
        if (g.top() < 0) g.setTop(0);
        if (g.right() >= form_->width()) g.setRight(form_->width() - 1);
        if (g.bottom() >= form_->height()) g.setBottom(form_->height() - 1);
        selected_->setGeometry(g);
    }
    update();
}

void DesignerCanvas::mouseReleaseEvent(QMouseEvent*) {
    if (mode_ == Rubber) {
        QRect band = bandRect_;
        mode_ = Idle;
        bandRect_ = QRect();
        finishRubberBand(band, bandAdditive_);
        updateAdornments();
        return;
    }
    if (mode_ != Idle) {
        mode_ = Idle;
        handle_ = -1;
        if (!selection_.isEmpty()) captureToDocument();
    }
}

void DesignerCanvas::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    QWidget* hit = widgetAt(event->pos() - form_->pos(), form_);
    if (hit) {
        select(hit);
        widgetActivated(hit);
    }
}

void DesignerCanvas::keyPressEvent(QKeyEvent* event) {
    int pageX = qMax(40, viewport_ ? viewport_->width() * 8 / 10 : 40);
    int pageY = qMax(40, viewport_ ? viewport_->height() * 8 / 10 : 40);
    switch (event->key()) {
    case Qt::Key_PageDown: scrollTo(scrollX_, scrollY_ + pageY); return;
    case Qt::Key_PageUp: scrollTo(scrollX_, scrollY_ - pageY); return;
    case Qt::Key_Home:
        if (event->modifiers() & Qt::ControlModifier) { scrollTo(0, scrollY_); return; }
        scrollTo(scrollX_, 0);
        return;
    case Qt::Key_End:
        if (event->modifiers() & Qt::ControlModifier) { scrollTo(maxScrollX_, scrollY_); return; }
        scrollTo(scrollX_, maxScrollY_);
        return;
    default: break;
    }
    if (event->key() == Qt::Key_Escape) {
        select(0);
        return;
    }
    if (event->key() == Qt::Key_Tab) {
        cycleSelection((event->modifiers() & Qt::ShiftModifier) ? -1 : 1);
        return;
    }
    if (selection_.isEmpty()) {
        QWidget::keyPressEvent(event);
        return;
    }
    int step = (event->modifiers() & Qt::ShiftModifier) ? 8 : 1;
    int dx = 0, dy = 0;
    switch (event->key()) {
    case Qt::Key_Left: dx = -step; break;
    case Qt::Key_Right: dx = step; break;
    case Qt::Key_Up: dy = -step; break;
    case Qt::Key_Down: dy = step; break;
    case Qt::Key_Delete: deleteSelected(); return;
    default: QWidget::keyPressEvent(event); return;
    }
    for (int i = 0; i < selection_.count(); ++i) {
        QRect g = selection_[i]->geometry();
        QRect moved = g.translated(dx, dy);
        if (moved.left() < 0 || moved.top() < 0 || moved.right() >= form_->width() ||
            moved.bottom() >= form_->height())
            continue;
        selection_[i]->setGeometry(moved);
    }
    captureToDocument();
    update();
}



void DesignerCanvas::syncScroll() {
    if (!form_ || !viewport_) return;
    int contentW = form_->width() + 40;
    int contentH = form_->height() + 40;
    maxScrollX_ = qMax(0, contentW - viewport_->width());
    maxScrollY_ = qMax(0, contentH - viewport_->height());
    scrollX_ = qBound(0, scrollX_, maxScrollX_);
    scrollY_ = qBound(0, scrollY_, maxScrollY_);
    form_->move(20 - scrollX_, 20 - scrollY_);
    viewport_->update();
    if (overlay_) overlay_->setGeometry(rect());
    updateAdornments();
}

void DesignerCanvas::scrollTo(int x, int y) {
    scrollX_ = x;
    scrollY_ = y;
    syncScroll();
}

void DesignerCanvas::scrollByPage(int pages) {
    int page = viewport_ ? viewport_->height() * 8 / 10 : 200;
    scrollTo(scrollX_, scrollY_ + pages * page);
}

void DesignerCanvas::scrollHome() {
    scrollTo(0, 0);
}

void DesignerCanvas::ensureVisible(QWidget* widget) {
    if (!widget || !viewport_) return;
    QRect r = widget->geometry().translated(form_->pos());
    int margin = 8;
    int nx = scrollX_, ny = scrollY_;
    if (r.left() < margin) nx -= (margin - r.left());
    if (r.right() > viewport_->width() - margin) nx += (r.right() - (viewport_->width() - margin));
    if (r.top() < margin) ny -= (margin - r.top());
    if (r.bottom() > viewport_->height() - margin) ny += (r.bottom() - (viewport_->height() - margin));
    scrollTo(nx, ny);
}

void DesignerCanvas::wheelEvent(QWheelEvent* event) {
    int step = event->delta() > 0 ? -48 : 48;
    if (event->modifiers() & Qt::ShiftModifier) scrollTo(scrollX_ + step, scrollY_);
    else scrollTo(scrollX_, scrollY_ + step);
    event->accept();
}

void DesignerCanvas::resizeEvent(QResizeEvent* event) {
    if (viewport_) viewport_->setGeometry(rect());
    if (overlay_) overlay_->setGeometry(rect());
    syncScroll();
    QWidget::resizeEvent(event);
}

void DesignerCanvas::updateAdornments() {
    if (overlay_) overlay_->update();
}

// ---------------------------------------------------------------------------
// PropertyEditor
// ---------------------------------------------------------------------------
class InlineEdit : public QLineEdit {
public:
    Signal<void()> cancelled;

protected:
    void keyPressEvent(QKeyEvent* event) {
        if (event->key() == Qt::Key_Escape) {
            cancelled();
            return;
        }
        QLineEdit::keyPressEvent(event);
    }
};

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
    void onCellClicked(int row, int column);
    void onFilterChanged(const QString& text);
    void beginInlineEdit(int row);
    void commitInlineEdit();
    void cancelInlineEdit();
    bool editValue(QWidget* widget, QMetaProperty prop);

    QTableWidget* table_;
    QLineEdit* filter_;
    InlineEdit* inline_;
    int inlineRow_;
    QWidget* target_;
};

PropertyEditor::PropertyEditor()
    : QWidget(0), table_(0), filter_(0), inline_(0), inlineRow_(-1), target_(0) {
    filter_ = new QLineEdit(this);
    filter_->setPlaceholderText(L"\u8fc7\u6ee4\u5c5e\u6027...");
    {
        QFont f = filter_->font();
        f.setPixelSize(12);
        filter_->setFont(f);
    }

    inline_ = new InlineEdit();
    inline_->setParentWidget(this);
    inline_->hide();
    QObject::connect(inline_, inline_->returnPressed, this, &PropertyEditor::commitInlineEdit);
    QObject::connect(inline_, inline_->cancelled, this, &PropertyEditor::cancelInlineEdit);
    QObject::connect(filter_, filter_->textChanged, this, &PropertyEditor::onFilterChanged);

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
    QObject::connect(table_, table_->cellClicked, this, &PropertyEditor::onCellClicked);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(filter_);
    layout->addWidget(table_, 1);
}

void PropertyEditor::setTarget(QWidget* widget) {
    cancelInlineEdit();
    target_ = widget;
    refresh();
}

void PropertyEditor::refresh() {
    cancelInlineEdit();
    table_->clearContents();
    table_->setRowCount(0);
    if (!target_) return;
    QString needle = filter_ ? filter_->text().toLower() : QString();
    const QMetaObject* mo = target_->metaObject();
    int rows = 0;
    for (int i = 0; i < mo->propertyCount(); ++i) {
        QMetaProperty prop = mo->property(i);
        if (!prop.isDesignable()) continue;
        if (!needle.isEmpty() &&
            QString::fromUtf8(prop.name()).toLower().indexOf(needle) < 0) continue;
        ++rows;
    }
    table_->setRowCount(rows);
    int row = 0;
    for (int i = 0; i < mo->propertyCount(); ++i) {
        QMetaProperty prop = mo->property(i);
        if (!prop.isDesignable()) continue;
        if (!needle.isEmpty() &&
            QString::fromUtf8(prop.name()).toLower().indexOf(needle) < 0) continue;
        table_->setItem(row, 0, new QTableWidgetItem(QString::fromUtf8(prop.name())));
        table_->setItem(row, 1, new QTableWidgetItem(prop.read(target_).toString()));
        ++row;
    }
}

void PropertyEditor::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

void PropertyEditor::onFilterChanged(const QString&) { refresh(); }

void PropertyEditor::onCellClicked(int row, int column) {
    if (!target_ || column != 1) return;
    QTableWidgetItem* nameItem = table_->item(row, 0);
    if (!nameItem) return;
    std::string name = nameItem->text().toUtf8();
    int index = target_->metaObject()->indexOfProperty(name.c_str());
    if (index < 0) return;
    QMetaProperty prop = target_->metaObject()->property(index);
    QString type = QString::fromUtf8(prop.typeName());
    if (type == L"bool") {
        prop.write(target_, QVariant(!prop.read(target_).toBool()));
        target_->update();
        refresh();
        propertyChanged(target_, prop.name(), prop.read(target_));
        return;
    }
    if (type == L"QString" || type == L"int" || type == L"uint" || type == L"double")
        beginInlineEdit(row);
}

void PropertyEditor::beginInlineEdit(int row) {
    if (!target_ || !inline_) return;
    inlineRow_ = row;
    QRect cell = table_->cellRect(row, 1);
    inline_->setGeometry(cell.translated(table_->pos()));
    inline_->setText(table_->item(row, 1) ? table_->item(row, 1)->text() : QString());
    inline_->show();
    inline_->raise();
    inline_->setFocus();
    inline_->selectAll();
}

void PropertyEditor::commitInlineEdit() {
    if (!inline_ || !inline_->isVisible() || !target_ || inlineRow_ < 0) return;
    QTableWidgetItem* nameItem = table_->item(inlineRow_, 0);
    if (!nameItem) {
        cancelInlineEdit();
        return;
    }
    std::string name = nameItem->text().toUtf8();
    int index = target_->metaObject()->indexOfProperty(name.c_str());
    if (index >= 0) {
        QMetaProperty prop = target_->metaObject()->property(index);
        QString type = QString::fromUtf8(prop.typeName());
        QString text = inline_->text();
        if (type == L"int" || type == L"uint") {
            bool ok = false;
            int value = text.toInt(&ok);
            if (ok) prop.write(target_, QVariant(value));
        } else if (type == L"double") {
            bool ok = false;
            double value = text.toDouble(&ok);
            if (ok) prop.write(target_, QVariant(value));
        } else {
            prop.write(target_, QVariant(text));
        }
        target_->update();
        propertyChanged(target_, prop.name(), prop.read(target_));
    }
    cancelInlineEdit();
    refresh();
}

void PropertyEditor::cancelInlineEdit() {
    if (!inline_) return;
    inline_->hide();
    inlineRow_ = -1;
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
// ToolboxTree : drag a widget class onto the canvas
// ---------------------------------------------------------------------------
class ToolboxTree : public QTreeWidget {
public:
    ToolboxTree() : dragActive_(false) {}

    Signal<void(const QString&, const QPoint&)> itemDropped;
    Signal<void(const QString&)> classActivated;

    void applyFilter(const QString& text) {
        QString needle = text.toLower();
        for (int i = 0; i < topLevelItemCount(); ++i) {
            QTreeWidgetItem* group = topLevelItem(i);
            int visible = 0;
            for (int c = 0; c < group->childCount(); ++c) {
                QTreeWidgetItem* child = group->child(c);
                bool match = needle.isEmpty() ||
                             child->text(0).toLower().indexOf(needle) >= 0;
                child->setHidden(!match);
                if (match) ++visible;
            }
            group->setHidden(visible == 0);
            if (!needle.isEmpty() && visible > 0) group->setExpanded(true);
        }
        update();
    }

protected:
    void keyPressEvent(QKeyEvent* event) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            QString cls = classNameOf(currentItem());
            if (!cls.isEmpty()) {
                classActivated(cls);
                return;
            }
        }
        QTreeWidget::keyPressEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) {
        QTreeWidget::mousePressEvent(event);
        if (event->button() != Qt::LeftButton) return;
        dragActive_ = false;
        dragStart_ = event->pos();
        dragClass_ = classNameOf(currentItem());
    }

    void mouseMoveEvent(QMouseEvent* event) {
        if (!(event->buttons() & Qt::LeftButton) || dragClass_.isEmpty()) return;
        int dx = event->pos().x() - dragStart_.x();
        int dy = event->pos().y() - dragStart_.y();
        if (!dragActive_ && (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy) > 6) {
            dragActive_ = true;
            setCursor(QCursor(Qt::ClosedHandCursor));
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) {
        if (dragActive_) {
            dragActive_ = false;
            setCursor(QCursor(Qt::ArrowCursor));
            QString cls = dragClass_;
            dragClass_.clear();
            if (!cls.isEmpty()) itemDropped(cls, event->globalPos());
            return;
        }
        QTreeWidget::mouseReleaseEvent(event);
    }

private:
    static QString classNameOf(QTreeWidgetItem* item) {
        if (!item) return QString();
        QVariant data = item->data(0, Qt::UserRole);
        return data.isValid() ? data.toString() : QString();
    }

    bool dragActive_;
    QPoint dragStart_;
    QString dragClass_;
};

// ---------------------------------------------------------------------------
// InspectorTree : object tree with a context menu
// ---------------------------------------------------------------------------
class InspectorTree : public QTreeWidget {
public:
    InspectorTree() {}
    Signal<void(QTreeWidgetItem*, const QPoint&)> contextMenuRequested;

protected:
    void mousePressEvent(QMouseEvent* event) {
        if (event->button() == Qt::RightButton) {
            QTreeWidgetItem* item = itemAt(event->pos());
            if (item) setCurrentItem(item);
            contextMenuRequested(item, event->globalPos());
            return;
        }
        QTreeWidget::mousePressEvent(event);
    }
};

// ---------------------------------------------------------------------------
// TabOrderDialog : reorder form children (the focus chain follows this order)
// ---------------------------------------------------------------------------
class TabOrderDialog : public QDialog {
public:
    TabOrderDialog(QWidget* owner, QWidget* form)
        : QDialog(owner), form_(form), changed_(false) {
        setWindowTitle(L"Tab 顺序");
        resize(420, 500);
        setFixedSize(420, 500);
        order_ = form->childrenWidgets();

        QLabel* hint = new QLabel(L"列表从上到下就是 Tab 焦点顺序，选中条目后用“上移/下移”调整。", this);
        hint->setWordWrap(true);
        hint->setGeometry(10, 10, 400, 34);

        list_ = new QListWidget(this);
        list_->setGeometry(10, 50, 400, 366);
        list_->setAlternatingRowColors(true);

        QPushButton* up = new QPushButton(L"上移", this);
        up->setGeometry(10, 424, 80, 28);
        QPushButton* down = new QPushButton(L"下移", this);
        down->setGeometry(96, 424, 80, 28);
        QPushButton* ok = new QPushButton(L"确定", this);
        ok->setGeometry(238, 424, 80, 28);
        QPushButton* cancel = new QPushButton(L"取消", this);
        cancel->setGeometry(324, 424, 86, 28);

        QObject::connect(up, up->clicked, static_cast<TabOrderDialog*>(this), &TabOrderDialog::moveUp);
        QObject::connect(down, down->clicked, static_cast<TabOrderDialog*>(this), &TabOrderDialog::moveDown);
        QObject::connect(ok, ok->clicked, static_cast<TabOrderDialog*>(this), &TabOrderDialog::confirm);
        QObject::connect(cancel, cancel->clicked, static_cast<QDialog*>(this), &QDialog::reject);
        refresh(0);
    }

    bool changed() const { return changed_; }

private:
    void refresh(int currentRow) {
        list_->clear();
        for (int i = 0; i < order_.count(); ++i) {
            list_->addItem(QString::number(i + 1) + L". " + order_[i]->objectName() +
                           L"  (" + QString(order_[i]->className()) + L")");
        }
        if (order_.count() > 0)
            list_->setCurrentRow(qBound(0, currentRow, order_.count() - 1));
        list_->repaint();
    }

    void moveUp() { move(-1); }
    void moveDown() { move(1); }

    void move(int step) {
        int row = list_->currentRow();
        int target = row + step;
        if (row < 0 || target < 0 || target >= order_.count()) return;
        QWidget* tmp = order_[row];
        order_[row] = order_[target];
        order_[target] = tmp;
        changed_ = true;
        refresh(target);
    }

    void confirm() {
        for (int i = 0; i < order_.count(); ++i) order_[i]->raise();
        accept();
    }

    QWidget* form_;
    QListWidget* list_;
    QList<QWidget*> order_;
    bool changed_;
};

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

protected:
    void closeEvent(QCloseEvent* event);

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
    void onToolboxDropped(const QString& className, const QPoint& globalPos);
    QWidget* addWidgetAt(const QString& className, const QPoint& formPos);
    void onTreeClicked(QTreeWidgetItem* item, int column);
    void onPropertyChanged(QWidget* widget, const char* name, const QVariant& value);
    void onToolboxFilter(const QString& text);
    void onToolboxClassActivated(const QString& className);
    void onWidgetActivated(QWidget* widget);
    void onInspectorContextMenu(QTreeWidgetItem* item, const QPoint& globalPos);
    void onTreeDoubleClicked(QTreeWidgetItem* item, int column);
    void onFormSize();
    void onTabOrder();
    void onCenterH();
    void onCenterV();
    void onSelectAll();
    void onScrollUp();
    void onScrollDown();
    void onScrollHome();
    void onAlignLeft();
    void onAlignRight();
    void onAlignTop();
    void onAlignBottom();
    void onDistributeH();
    void onDistributeV();
    void onRaise();
    void onLower();
    void renameWidget(QWidget* widget);
    void updateWindowTitle();

    DesignerDocument doc_;
    DesignerCanvas* canvas_;
    InspectorTree* tree_;
    QMenu* objectMenu_;
    MenuRelay* objectRelay_;
    QString contextObject_;
    ToolboxTree* toolbox_;
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
    edit->addSeparator();
    QAction* formSizeAct = edit->addAction(L"窗体大小...");
    QObject::connect(formSizeAct, formSizeAct->triggered, this, &DesignerWindow::onFormSize);
    QAction* centerHAct = edit->addAction(L"水平居中");
    QObject::connect(centerHAct, centerHAct->triggered, this, &DesignerWindow::onCenterH);
    QAction* centerVAct = edit->addAction(L"垂直居中");
    QObject::connect(centerVAct, centerVAct->triggered, this, &DesignerWindow::onCenterV);
    edit->addSeparator();
    QAction* frontAct = edit->addAction(L"置于顶层");
    QObject::connect(frontAct, frontAct->triggered, this, &DesignerWindow::onRaise);
    QAction* backAct = edit->addAction(L"置于底层");
    QObject::connect(backAct, backAct->triggered, this, &DesignerWindow::onLower);
    edit->addSeparator();
    QAction* tabOrderAct = edit->addAction(L"Tab 顺序...");
    QObject::connect(tabOrderAct, tabOrderAct->triggered, this, &DesignerWindow::onTabOrder);
    edit->addSeparator();
    QAction* selectAllAct = edit->addAction(L"全选");
    selectAllAct->setShortcut(L"Ctrl+A");
    QObject::connect(selectAllAct, selectAllAct->triggered, this, &DesignerWindow::onSelectAll);
    edit->addSeparator();
    QAction* alignLeftAct = edit->addAction(L"左对齐");
    QObject::connect(alignLeftAct, alignLeftAct->triggered, this, &DesignerWindow::onAlignLeft);
    QAction* alignRightAct = edit->addAction(L"右对齐");
    QObject::connect(alignRightAct, alignRightAct->triggered, this, &DesignerWindow::onAlignRight);
    QAction* alignTopAct = edit->addAction(L"顶对齐");
    QObject::connect(alignTopAct, alignTopAct->triggered, this, &DesignerWindow::onAlignTop);
    QAction* alignBottomAct = edit->addAction(L"底对齐");
    QObject::connect(alignBottomAct, alignBottomAct->triggered, this, &DesignerWindow::onAlignBottom);
    edit->addSeparator();
    QAction* distHAct = edit->addAction(L"水平分布");
    QObject::connect(distHAct, distHAct->triggered, this, &DesignerWindow::onDistributeH);
    QAction* distVAct = edit->addAction(L"垂直分布");
    QObject::connect(distVAct, distVAct->triggered, this, &DesignerWindow::onDistributeV);

    QMenu* view = bar->addMenu(L"视图");
    QAction* scrollUpAct = view->addAction(L"向上滚动");
    QObject::connect(scrollUpAct, scrollUpAct->triggered, this, &DesignerWindow::onScrollUp);
    QAction* scrollDownAct = view->addAction(L"向下滚动");
    QObject::connect(scrollDownAct, scrollDownAct->triggered, this, &DesignerWindow::onScrollDown);
    QAction* scrollHomeAct = view->addAction(L"回到左上角");
    QObject::connect(scrollHomeAct, scrollHomeAct->triggered, this, &DesignerWindow::onScrollHome);
    view->addSeparator();
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
    toolbar->addSeparator();
    QAction* scrollUpTb = toolbar->addAction(L"滚动 ↑");
    QObject::connect(scrollUpTb, scrollUpTb->triggered, this, &DesignerWindow::onScrollUp);
    QAction* scrollDownTb = toolbar->addAction(L"滚动 ↓");
    QObject::connect(scrollDownTb, scrollDownTb->triggered, this, &DesignerWindow::onScrollDown);
}

void DesignerWindow::buildUi() {
    QSplitter* main = new QSplitter(Qt::Horizontal);

    // ---- toolbox pane: filter + class tree ----
    QWidget* toolboxPane = new QWidget();
    QVBoxLayout* toolboxLayout = new QVBoxLayout(toolboxPane);
    toolboxLayout->setContentsMargins(0, 0, 0, 0);
    toolboxLayout->setSpacing(2);
    QLineEdit* toolboxFilter = new QLineEdit();
    toolboxFilter->setPlaceholderText(L"\u8fc7\u6ee4\u63a7\u4ef6...");
    toolboxFilter->setClearButtonEnabled(true);
    QObject::connect(toolboxFilter, toolboxFilter->textChanged, this, &DesignerWindow::onToolboxFilter);
    toolboxLayout->addWidget(toolboxFilter);

    toolbox_ = new ToolboxTree();
    toolbox_->setHeaderLabels(QStringList() << L"\u63a7\u4ef6");
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
            QString clsName = QString::fromUtf8(classes[i].className);
            if (clsName == L"QDialog" || clsName == L"QMainWindow" || clsName == L"QMessageBox")
                continue;
            QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::fromUtf8(classes[i].label));
            item->setData(0, Qt::UserRole, QVariant(QString::fromUtf8(classes[i].className)));
            groupItem->addChild(item);
        }
        toolbox_->addTopLevelItem(groupItem);
    }
    QObject::connect(toolbox_, toolbox_->itemDoubleClicked, this, &DesignerWindow::onToolboxActivated);
    toolbox_->itemDropped.connect(toolbox_, this, &DesignerWindow::onToolboxDropped);
    toolbox_->classActivated.connect(toolbox_, this, &DesignerWindow::onToolboxClassActivated);
    toolboxLayout->addWidget(toolbox_, 1);

    // ---- canvas ----
    canvas_ = new DesignerCanvas(&doc_);
    canvas_->selectionChanged.connect(this, this, &DesignerWindow::onSelectionChanged);
    canvas_->documentModified.connect(this, this, &DesignerWindow::onDocumentModified);
    canvas_->widgetActivated.connect(canvas_, this, &DesignerWindow::onWidgetActivated);

    // ---- right: object tree + properties ----
    QSplitter* right = new QSplitter(Qt::Vertical);
    tree_ = new InspectorTree();
    tree_->setHeaderLabels(QStringList() << L"\u5bf9\u8c61" << L"\u7c7b\u578b");
    tree_->setIndentation(14);
    QObject::connect(tree_, tree_->itemClicked, this, &DesignerWindow::onTreeClicked);
    QObject::connect(tree_, tree_->itemDoubleClicked, this, &DesignerWindow::onTreeDoubleClicked);
    tree_->contextMenuRequested.connect(tree_, this, &DesignerWindow::onInspectorContextMenu);
    objectMenu_ = new QMenu(0);
    objectRelay_ = new MenuRelay();
    objectRelay_->fn = [this](QAction* action) {
        if (!action || !canvas_->form()) return;
        QString name = contextObject_;
        QWidget* widget = (name == canvas_->form()->objectName())
                              ? canvas_->form()
                              : findWidgetByName(canvas_->form(), name);
        int index = objectMenu_->actions().indexOf(action);
        if (!widget) return;
        if (index == 0) {
            renameWidget(widget);
        } else if (index == 1) {
            canvas_->select(widget);
            canvas_->deleteSelected();
            refreshInspector();
            refreshProperties();
        } else if (index == 3) {
            widget->raise();
            canvas_->captureToDocument();
            canvas_->update();
        } else if (index == 4) {
            widget->lower();
            canvas_->captureToDocument();
            canvas_->update();
        }
        objectMenu_->hide();
    };
    objectMenu_->triggeredAction.connect(objectMenu_, objectRelay_, &MenuRelay::onTriggered);

    properties_ = new PropertyEditor();
    properties_->propertyChanged.connect(this, this, &DesignerWindow::onPropertyChanged);
    right->addWidget(tree_);
    right->addWidget(properties_);
    right->setSizes(QList<int>() << 220 << 420);

    main->addWidget(toolboxPane);
    main->addWidget(canvas_);
    main->addWidget(right);
    main->setSizes(QList<int>() << 200 << 660 << 340);
    setCentralWidget(main);

    status_ = new QStatusBar();
    setStatusBar(status_);
    status_->showMessage(L"\u5c31\u7eea - \u53cc\u51fb\u6216\u62d6\u62fd\u5de5\u5177\u7bb1\u6dfb\u52a0\u63a7\u4ef6");
}

void DesignerWindow::setStatus(const QString& text) {
    if (status_) status_->showMessage(text, 5000);
}

void DesignerWindow::newDocument() {
    doc_.reset(L"Form1");
    doc_.setModified(false);
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
    QTreeWidgetItem* root = new QTreeWidgetItem(QStringList()
        << canvas_->form()->objectName() << L"QWidget");
    tree_->addTopLevelItem(root);
    QList<QWidget*> children = canvas_->form()->childrenWidgets();
    for (int i = 0; i < children.count(); ++i) {
        QTreeWidgetItem* child = new QTreeWidgetItem(QStringList()
            << children[i]->objectName() << QString(children[i]->className()));
        root->addChild(child);
        QList<QWidget*> grandchildren = children[i]->childrenWidgets();
        for (int g = 0; g < grandchildren.count(); ++g)
            child->addChild(new QTreeWidgetItem(QStringList()
                << grandchildren[g]->objectName() << QString(grandchildren[g]->className())));
        child->setExpanded(true);
    }
    root->setExpanded(true);
    updatingInspector_ = false;
    updateWindowTitle();
}

void DesignerWindow::refreshProperties() {
    QWidget* selected = canvas_->selected();
    if (selected) properties_->setTarget(selected);
    else properties_->setTarget(canvas_->form());
}

void DesignerWindow::onSelectionChanged(QWidget* widget) {
    refreshProperties();
    QList<QWidget*> list = canvas_->selection();
    if (list.isEmpty() || !widget) {
        setStatus(L"未选中任何控件");
        return;
    }
    if (list.count() > 1) {
        setStatus(QString::number(list.count()) +
                  L" 个控件已选中（拖动整体移动，Ctrl+单击增减，框选空白处）");
        return;
    }
    QRect g = widget->geometry();
    setStatus(widget->objectName() + L"  " + QString(widget->className()) + L"  " +
              QString::number(g.x()) + L"," + QString::number(g.y()) + L"  " +
              QString::number(g.width()) + L" x " + QString::number(g.height()));
}

void DesignerWindow::onDocumentModified() {
    refreshInspector();
    refreshProperties();
    doc_.setModified(true);
    updateWindowTitle();
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

void DesignerWindow::onToolboxDropped(const QString& className, const QPoint& globalPos) {
    QPoint canvasPos = canvas_->mapFromGlobal(globalPos);
    QPoint formPos = canvasPos - canvas_->form()->pos();
    addWidgetAt(className, formPos);
}

QWidget* DesignerWindow::addWidgetAt(const QString& className, const QPoint& formPos) {
    QWidget* base = canvas_->form();
    QWidget* widget = QWidgetFactory::create(className.toUtf8().c_str());
    if (!widget) return 0;
    widget->setParentWidget(base);
    widget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    widget->setObjectName(uniqueName(base, className.toLower()));
    widget->resize(widget->sizeHint());
    QSize size = widget->size();
    int x = formPos.x() - size.width() / 2;
    int y = formPos.y() - size.height() / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + size.width() > base->width()) x = qMax(0, base->width() - size.width());
    if (y + size.height() > base->height()) y = qMax(0, base->height() - size.height());
    widget->move(QPoint(x, y));
    widget->show();
    canvas_->select(widget);
    canvas_->captureToDocument();
    refreshInspector();
    refreshProperties();
    setStatus(L"已添加 " + className + L" (" + widget->objectName() + L")");
    return widget;
}

void DesignerWindow::addWidgetByClass(const QString& className) {
    QWidget* base = canvas_->form();
    QWidget* parent = canvas_->selected();
    if (!parent || parent == base) parent = base;
    QWidget* widget = QWidgetFactory::create(className.toUtf8().c_str());
    if (!widget) return;
    widget->setParentWidget(parent);
    widget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
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
    QWidget* widget = findWidgetByName(canvas_->form(), name);
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

void DesignerWindow::onToolboxFilter(const QString& text) {
    toolbox_->applyFilter(text);
}

void DesignerWindow::onToolboxClassActivated(const QString& className) {
    addWidgetByClass(className);
}

void DesignerWindow::onWidgetActivated(QWidget* widget) {
    Q_UNUSED(widget);
    refreshProperties();
    properties_->setFocus();
}

void DesignerWindow::onTreeDoubleClicked(QTreeWidgetItem* item, int) {
    if (!item) return;
    QWidget* widget = findWidgetByName(canvas_->form(), item->text(0));
    if (widget) renameWidget(widget);
}

void DesignerWindow::renameWidget(QWidget* widget) {
    if (!widget || widget == canvas_->form()) return;
    bool ok = false;
    QString name = QInputDialog::getText(this, L"\u91cd\u547d\u540d", L"\u5bf9\u8c61\u540d", 0,
                                         widget->objectName(), &ok);
    if (!ok) return;
    name = name.trimmed();
    if (name.isEmpty()) return;
    if (findWidgetByName(canvas_->form(), name)) {
        QMessageBox::warning(this, L"\u91cd\u547d\u540d", L"\u540d\u79f0\u5df2\u5b58\u5728");
        return;
    }
    widget->setObjectName(name);
    canvas_->captureToDocument();
    refreshInspector();
    refreshProperties();
    setStatus(L"\u5df2\u91cd\u547d\u540d\u4e3a " + name);
}

void DesignerWindow::onInspectorContextMenu(QTreeWidgetItem* item, const QPoint& globalPos) {
    if (!item) return;
    contextObject_ = item->text(0);
    objectMenu_->clear();
    objectMenu_->addAction(QString(L"\u91cd\u547d\u540d..."));
    objectMenu_->addAction(QString(L"\u5220\u9664"));
    objectMenu_->addSeparator();
    objectMenu_->addAction(QString(L"\u7f6e\u4e8e\u9876\u5c42"));
    objectMenu_->addAction(QString(L"\u7f6e\u4e8e\u5e95\u5c42"));
    objectMenu_->popup(globalPos);
}

void DesignerWindow::onFormSize() {
    bool ok = false;
    QWidget* form = canvas_->form();
    QString text = QString::number(form->width()) + L" x " + QString::number(form->height());
    QString input = QInputDialog::getText(this, L"\u7a97\u4f53\u5927\u5c0f",
                                          L"\u5bbd x \u9ad8 (\u50cf\u7d20)", 0, text, &ok);
    if (!ok) return;
    QStringList parts = input.split(L'x');
    if (parts.count() != 2) {
        QMessageBox::warning(this, L"\u7a97\u4f53\u5927\u5c0f", L"\u8bf7\u8f93\u5165\u5982 640 x 480");
        return;
    }
    int w = parts[0].trimmed().toInt();
    int h = parts[1].trimmed().toInt();
    if (w < 80 || h < 60) {
        QMessageBox::warning(this, L"\u7a97\u4f53\u5927\u5c0f", L"\u6700\u5c0f 80 x 60");
        return;
    }
    form->resize(w, h);
    canvas_->captureToDocument();
    canvas_->update();
    refreshProperties();
}

void DesignerWindow::onTabOrder() {
    if (!canvas_->form()) return;
    TabOrderDialog dlg(this, canvas_->form());
    if (dlg.exec() == QDialog::Accepted && dlg.changed()) {
        canvas_->captureToDocument();
        refreshInspector();
        refreshProperties();
        setStatus(L"已更新 Tab 顺序");
    }
}

void DesignerWindow::onCenterH() { canvas_->centerSelected(true); }
void DesignerWindow::onCenterV() { canvas_->centerSelected(false); }

void DesignerWindow::onScrollUp() { canvas_->scrollByPage(-1); }
void DesignerWindow::onScrollDown() { canvas_->scrollByPage(1); }
void DesignerWindow::onScrollHome() { canvas_->scrollHome(); }

void DesignerWindow::onSelectAll() {
    canvas_->selectAll();
    refreshProperties();
}

void DesignerWindow::onAlignLeft() {
    canvas_->alignSelection(DesignerCanvas::AlignLeft);
    refreshProperties();
}
void DesignerWindow::onAlignRight() {
    canvas_->alignSelection(DesignerCanvas::AlignRight);
    refreshProperties();
}
void DesignerWindow::onAlignTop() {
    canvas_->alignSelection(DesignerCanvas::AlignTop);
    refreshProperties();
}
void DesignerWindow::onAlignBottom() {
    canvas_->alignSelection(DesignerCanvas::AlignBottom);
    refreshProperties();
}
void DesignerWindow::onDistributeH() {
    canvas_->distributeSelection(true);
    refreshProperties();
}
void DesignerWindow::onDistributeV() {
    canvas_->distributeSelection(false);
    refreshProperties();
}
void DesignerWindow::onRaise() { canvas_->raiseSelected(); }
void DesignerWindow::onLower() { canvas_->lowerSelected(); }

void DesignerWindow::updateWindowTitle() {
    QString name = doc_.filePath().isEmpty() ? QString(L"\u672a\u547d\u540d") : doc_.filePath();
    setWindowTitle(L"QtClone Designer - " + name + (doc_.modified() ? L" *" : L""));
}

void DesignerWindow::closeEvent(QCloseEvent* event) {
    if (doc_.modified()) {
        QMessageBox::StandardButton answer = QMessageBox::question(
            this, L"\u9000\u51fa", L"\u6709\u672a\u4fdd\u5b58\u7684\u4fee\u6539\uff0c\u786e\u5b9a\u9000\u51fa\uff1f",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    event->accept();
    QWidget::closeEvent(event);
}

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

    // multi selection: unique names, align, distribute
    newDocument();
    QWidget* a = addWidgetAt(L"QPushButton", QPoint(80, 80));
    QWidget* b = addWidgetAt(L"QPushButton", QPoint(260, 160));
    if (!a || !b) {
        printf("FAIL: multi add\n");
        ++failures;
    } else {
        if (a->objectName() == b->objectName()) {
            printf("FAIL: duplicate object names\n");
            ++failures;
        }
        a->setGeometry(QRect(10, 10, 100, 32));
        b->setGeometry(QRect(200, 60, 100, 32));
        QList<QWidget*> both;
        both.append(a);
        both.append(b);
        canvas_->setSelectionList(both);
        canvas_->alignSelection(DesignerCanvas::AlignTop);
        if (a->y() != b->y()) {
            printf("FAIL: align top (%d vs %d)\n", a->y(), b->y());
            ++failures;
        }
        canvas_->alignSelection(DesignerCanvas::AlignLeft);
        if (a->x() != b->x()) {
            printf("FAIL: align left (%d vs %d)\n", a->x(), b->x());
            ++failures;
        }

        QWidget* c = addWidgetAt(L"QPushButton", QPoint(420, 280));
        if (c) {
            a->setGeometry(QRect(10, 10, 60, 32));
            b->setGeometry(QRect(150, 10, 60, 32));
            c->setGeometry(QRect(400, 10, 60, 32));
            QList<QWidget*> three;
            three.append(a);
            three.append(b);
            three.append(c);
            canvas_->setSelectionList(three);
            canvas_->distributeSelection(true);
            int gap1 = b->x() - (a->x() + a->width());
            int gap2 = c->x() - (b->x() + b->width());
            if (gap1 != gap2) {
                printf("FAIL: distribute (%d vs %d)\n", gap1, gap2);
                ++failures;
            }
            QList<QWidget*> allThree;
            allThree.append(a);
            allThree.append(b);
            allThree.append(c);
            canvas_->setSelectionList(allThree);
            canvas_->deleteSelected();
            if (canvas_->form()->childrenWidgets().count() != 0) {
                printf("FAIL: multi delete\n");
                ++failures;
            }
        }
    }

    // canvas scrolling: large form, scroll by page, then back home
    newDocument();
    canvas_->form()->resize(2000, 1500);
    int y0 = canvas_->form()->y();
    canvas_->scrollByPage(1);
    int y1 = canvas_->form()->y();
    if (y1 >= y0) {
        printf("FAIL: scroll down (%d -> %d)\n", y0, y1);
        ++failures;
    }
    canvas_->scrollHome();
    if (canvas_->form()->y() != 20) {
        printf("FAIL: scroll home (%d)\n", canvas_->form()->y());
        ++failures;
    }

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
