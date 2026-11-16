// QtClone example: a form built from Form1.ui through the qtcuic compiler.
//
// build.bat (and the VS2010 project) run:
//     qtcuic Form1.ui -o ui_form1.h
// before compiling this file, exactly like uic in a Qt project.
#include "ui_form1.h"
#include <qtc/QtWidgets.h>
#include <stdio.h>
#include <string.h>

class Form1 : public QWidget {
public:
    Form1() {
        ui_.setupUi(this);
        ui_.levelCombo->addItems(QStringList() << L"普通客户" << L"重要客户" << L"VIP 客户");
        ui_.levelCombo->setCurrentIndex(0);
        QObject::connect(ui_.okButton, ui_.okButton->clicked, this, &Form1::onOk);
        QObject::connect(ui_.cancelButton, ui_.cancelButton->clicked, this, &Form1::onCancel);
        QObject::connect(ui_.nameEdit, ui_.nameEdit->textChanged, this, &Form1::onNameChanged);
    }

    Ui_Form1* ui() { return &ui_; }

private:
    void onOk(bool) {
        QMessageBox::information(this, L"确定",
                                 L"客户：" + ui_.nameEdit->text() + L"\n等级：" +
                                 ui_.levelCombo->currentText());
    }
    void onCancel(bool) { close(); }
    void onNameChanged(const QString& text) {
        ui_.progress->setValue(qMin(100, (int)text.size() * 10));
    }

    Ui_Form1 ui_;
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    Form1 form;
    form.show();

    const char* shot = 0;
    const char* theme = 0;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) shot = argv[++i];
        else if (strcmp(argv[i], "--theme") == 0 && i + 1 < argc) theme = argv[++i];
    }
    if (theme) {
        if (strcmp(theme, "dark") == 0) QTheme::setMode(QTheme::Dark);
        else if (strcmp(theme, "light") == 0) QTheme::setMode(QTheme::Light);
        else QTheme::setMode(QTheme::System);
    }
    for (int i = 0; i < 10; ++i) QApplication::processEvents();
    if (shot) {
        QPixmap pm = form.grab();
        bool ok = pm.save(QString::fromUtf8(shot));
        printf("form screenshot %s -> %s\n", ok ? "saved" : "FAILED", shot);
        return ok ? 0 : 1;
    }
    return QApplication::exec();
}
