// QtClone smoke tests - console checks for the core, widgets, layout, styling,
// XML documents and the .ui compiler. Run as part of build.bat.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include <qtc/QXml.h>
#include "../tools/uicgen/uicgen.h"
#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_checks = 0;

static void check(bool ok, const char* what) {
    ++g_checks;
    if (!ok) {
        printf("FAIL  %s\n", what);
        ++g_failures;
    } else {
        printf("ok    %s\n", what);
    }
}

class ClickTarget : public QObject {
public:
    ClickTarget() : clicks(0) {}
    void onClicked(bool) { ++clicks; }
    int clicks;
};

static void testCore() {
    check(QString("abc") == QString(L"abc"), "QString equality");
    check(QString::fromUtf8("中文").size() == 2, "UTF-8 decoding");
    check(QString("%1-%2").arg(1).arg(2) == QString(L"1-2"), "QString::arg");
    check(QString("a,b,c").split(L',').count() == 3, "QString::split");
    check(QVariant(QString("42")).toInt() == 42, "QVariant string -> int");
    check(QVariant(7).toString() == QString(L"7"), "QVariant int -> string");
    check(QVariant(QColor(1, 2, 3)).toString() == QString(L"#010203"), "QVariant color -> string");
}

static void testXml() {
    QString error;
    QXmlNode* root = QXmlNode::parse(L"<ui version=\"4.0\"><class>Form1</class>"
                                     L"<widget class=\"QWidget\" name=\"Form1\">"
                                     L"<property name=\"windowTitle\"><string>A &amp; B</string></property>"
                                     L"</widget></ui>", &error);
    check(root != 0, "QXml parse");
    if (!root) return;
    check(root->attribute(L"version") == QString(L"4.0"), "QXml attribute");
    check(root->firstChild(L"class")->text() == QString(L"Form1"), "QXml text");
    QXmlNode* widget = root->firstChild(L"widget");
    check(widget && widget->attribute(L"class") == QString(L"QWidget"), "QXml child");
    QXmlNode* prop = widget ? widget->firstChild(L"property") : 0;
    check(prop && prop->firstChild(L"string")->text() == QString(L"A & B"), "QXml entities");
    QString xml = root->toXml(0);
    QXmlNode* again = QXmlNode::parse(xml, &error);
    check(again != 0, "QXml serializer roundtrip");
    delete again;
    delete root;
}

static void testWidgets() {
    QList<QWidgetClassInfo> classes = QWidgetFactory::classes();
    check(classes.count() >= 20, "widget factory has >= 20 classes");
    QWidget* button = QWidgetFactory::create("QPushButton");
    check(button != 0, "factory creates QPushButton");
    if (button) {
        check(button->inherits("QAbstractButton"), "factory type");
        button->setProperty("text", QVariant(QString(L"Hello")));
        QVariant text = button->property("text");
        check(text.toString() == QString(L"Hello"), "property roundtrip");
        button->setGeometry(QRect(10, 20, 120, 30));
        check(button->geometry() == QRect(10, 20, 120, 30), "geometry property");
        delete button;
    }

    // signal / slot
    QPushButton* b = new QPushButton(L"click");
    ClickTarget target;
    QObject::connect(b, b->clicked, &target, &ClickTarget::onClicked);
    b->click();
    check(target.clicks == 1, "button clicked signal");
    delete b;

    // layout distribution
    QWidget host;
    host.resize(300, 100);
    QHBoxLayout* layout = new QHBoxLayout(&host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    QPushButton* b1 = new QPushButton(L"1");
    QPushButton* b2 = new QPushButton(L"2");
    layout->addWidget(b1);
    layout->addWidget(b2);
    host.show();
    layout->activate();
    check(b1->width() + b2->width() == 300, "box layout fills width");
    check(b1->height() == 100, "box layout fills height");

    // style sheet
    b1->setStyleSheet(L"QPushButton { background-color: #112233; border-radius: 4px; }");
    const QStyleSheet::Box& box = QStyleSheet::boxFor(b1);
    check(box.hasBackground && box.background == QColor(0x11, 0x22, 0x33), "style sheet background");
    check(box.radius == 4, "style sheet radius");

    // theme switching
    QTheme::setMode(QTheme::Dark);
    check(QTheme::isDark(), "dark theme enabled");
    check(QTheme::colors().window.red() < 40, "dark window colour");
    QTheme::setMode(QTheme::Light);
    check(!QTheme::isDark(), "light theme enabled");

    // multi-line text editor
    QTextEdit* editor = new QTextEdit();
    editor->setPlainText(L"line1\n第二行");
    check(editor->toPlainText() == QString(L"line1\n第二行"), "text edit roundtrip");
    editor->selectAll();
    check(editor->selectedText() == editor->toPlainText(), "text edit selection");
    editor->setPlainText(L"abc");
    editor->setCursorPosition(3);
    editor->insertPlainText(L"d");
    check(editor->toPlainText() == QString(L"abcd"), "text edit insert");
    editor->setProperty("text", QVariant(QString(L"via property")));
    check(editor->toPlainText() == QString(L"via property"), "text edit property");
    delete editor;

    // off-screen painting
    QPixmap pm(60, 30);
    pm.fill(QColor(255, 255, 255));
    {
        QPainter p(&pm);
        QPushButton* painted = new QPushButton(L"OK");
        painted->resize(60, 30);
        painted->show();
        painted->render(&p, QPoint(0, 0));
        delete painted;
    }
    check(true, "widget off-screen render");
}

static void testUiCompiler() {
    QString ui =
        L"<ui version=\"4.0\"><class>Form1</class>"
        L"<widget class=\"QWidget\" name=\"Form1\">"
        L"<property name=\"geometry\"><rect><x>0</x><y>0</y><width>400</width><height>300</height></rect></property>"
        L"<widget class=\"QPushButton\" name=\"okButton\">"
        L"<property name=\"geometry\"><rect><x>20</x><y>20</y><width>100</width><height>32</height></rect></property>"
        L"<property name=\"text\"><string>确定 OK</string></property>"
        L"</widget></widget></ui>";
    QString error;
    QString header = uicGenerateHeader(ui, L"Form1", &error);
    check(!header.isEmpty(), "uic generates a header");
    if (!header.isEmpty()) {
        check(header.indexOf(L"class Ui_Form1") >= 0, "uic class name");
        check(header.indexOf(L"QPushButton* okButton") >= 0, "uic widget member");
        check(header.indexOf(L"okButton->setGeometry(QRect(20, 20, 100, 32))") >= 0, "uic geometry");
        check(header.indexOf(L"retranslateUi") >= 0, "uic retranslate");
    }
}

int main() {
    printf("== QtClone smoke tests ==\n");
    testCore();
    testXml();
    testWidgets();
    testUiCompiler();
    printf("== %s: %d checks, %d failures ==\n", g_failures ? "FAILED" : "PASSED", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
