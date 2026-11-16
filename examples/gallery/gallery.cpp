// QtClone gallery : demonstrates the widget set, layouts, styling and themes.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include <stdio.h>
#include <string.h>

class GalleryWindow : public QMainWindow {
public:
    GalleryWindow() {
        setWindowTitle(L"QtClone Widget Gallery");
        resize(980, 660);

        buildMenus();
        buildStatusBar();
        buildCentral();
    }

    void setStatus(const QString& text) {
        if (statusBar()) statusBar()->showMessage(text, 4000);
    }

private:
    QMenuBar* menuBar_;
    QToolBar* toolBar_;

    QWidget* buildControlsTab() {
        QWidget* page = new QWidget();
        QVBoxLayout* root = new QVBoxLayout(page);
        root->setContentsMargins(14, 12, 14, 12);
        root->setSpacing(10);

        QLabel* title = new QLabel(L"常用控件 / Common controls");
        QFont titleFont = title->font();
        titleFont.setBold(true);
        titleFont.setPixelSize(15);
        title->setFont(titleFont);
        root->addWidget(title);

        QHBoxLayout* topRow = new QHBoxLayout();
        topRow->setSpacing(12);

        // buttons
        QGroupBox* buttonBox = new QGroupBox(L"Buttons");
        QVBoxLayout* bl = new QVBoxLayout(buttonBox);
        bl->setContentsMargins(14, 34, 14, 12);
        bl->setSpacing(6);
        QPushButton* prim = new QPushButton(L"主要按钮 Primary");
        prim->setDefault(true);
        QObject::connect(prim, prim->clicked, this, &GalleryWindow::onPrimary);
        bl->addWidget(prim);
        QPushButton* normal = new QPushButton(L"普通按钮 Normal");
        QObject::connect(normal, normal->clicked, this, &GalleryWindow::onNormal);
        bl->addWidget(normal);
        QPushButton* disabled = new QPushButton(L"禁用 Disabled");
        disabled->setEnabled(false);
        bl->addWidget(disabled);
        QHBoxLayout* checkRow = new QHBoxLayout();
        QCheckBox* check = new QCheckBox(L"启用选项");
        check->setChecked(true);
        checkRow->addWidget(check);
        checkRow->addStretch(1);
        bl->addLayout(checkRow);
        QRadioButton* radio1 = new QRadioButton(L"选项 A");
        QRadioButton* radio2 = new QRadioButton(L"选项 B");
        radio1->setChecked(true);
        bl->addWidget(radio1);
        bl->addWidget(radio2);
        bl->addStretch(1);
        topRow->addWidget(buttonBox, 1);

        // inputs
        QGroupBox* inputBox = new QGroupBox(L"Inputs");
        QVBoxLayout* il = new QVBoxLayout(inputBox);
        il->setContentsMargins(14, 34, 14, 12);
        il->setSpacing(6);
        QLineEdit* edit = new QLineEdit();
        edit->setPlaceholderText(L"请输入姓名…");
        QObject::connect(edit, edit->textChanged, this, &GalleryWindow::onTextChanged);
        il->addWidget(edit);
        QComboBox* combo = new QComboBox();
        combo->addItems(QStringList() << L"选项一" << L"选项二" << L"选项三");
        QObject::connect(combo, combo->currentIndexChanged, this, &GalleryWindow::onComboChanged);
        il->addWidget(combo);
        QHBoxLayout* spinRow = new QHBoxLayout();
        QSpinBox* spin = new QSpinBox();
        spin->setRange(0, 100);
        spin->setValue(25);
        spinRow->addWidget(spin, 1);
        QDoubleSpinBox* dspin = new QDoubleSpinBox();
        dspin->setRange(0, 10);
        dspin->setValue(2.5);
        spinRow->addWidget(dspin, 1);
        il->addLayout(spinRow);
        QSlider* slider = new QSlider(Qt::Horizontal);
        slider->setRange(0, 100);
        slider->setValue(60);
        il->addWidget(slider);
        QProgressBar* progress = new QProgressBar();
        progress->setRange(0, 100);
        progress->setValue(45);
        il->addWidget(progress);
        il->addStretch(1);
        topRow->addWidget(inputBox, 1);

        // text / notes
        QGroupBox* textBox = new QGroupBox(L"Text");
        QVBoxLayout* tl = new QVBoxLayout(textBox);
        tl->setContentsMargins(14, 34, 14, 12);
        tl->setSpacing(6);
        QLabel* wrapped = new QLabel(L"这是一个会自动换行的标签。The quick brown fox jumps over the lazy dog, "
                                     L"用来验证 QLabel 的 wordWrap 与对齐行为。");
        wrapped->setWordWrap(true);
        wrapped->setMinimumSize(QSize(0, 96));
        tl->addWidget(wrapped);
        QLabel* centered = new QLabel(L"居中对齐 / centered");
        centered->setAlignment(Qt::AlignCenter);
        centered->setMinimumSize(QSize(0, 40));
        tl->addWidget(centered);
        QTextEdit* notes = new QTextEdit();
        notes->setPlainText(L"多行文本编辑器：\n自动换行、选区、撤销/重做、\n剪贴板与输入法。");
        notes->setMinimumSize(120, 90);
        tl->addWidget(notes, 1);
        tl->addStretch(1);
        topRow->addWidget(textBox, 1);

        root->addLayout(topRow, 1);

        // tabs demo
        QTabWidget* tabs = new QTabWidget();
        QWidget* t1 = new QWidget();
        QVBoxLayout* t1l = new QVBoxLayout(t1);
        t1l->addWidget(new QLabel(L"第一个标签页的内容"));
        QTableWidget* table = new QTableWidget(3, 3);
        QStringList headers;
        headers << L"名称" << L"数量" << L"单价";
        table->setHorizontalHeaderLabels(headers);
        const wchar_t* names[] = { L"键盘", L"鼠标", L"显示器" };
        for (int r = 0; r < 3; ++r) {
            table->setItem(r, 0, new QTableWidgetItem(names[r]));
            table->setItem(r, 1, new QTableWidgetItem(QString::number((r + 1) * 3)));
            table->setItem(r, 2, new QTableWidgetItem(QString::number(99 + r * 30)));
        }
        t1l->addWidget(table);
        tabs->addTab(t1, L"表格");
        QWidget* t2 = new QWidget();
        QVBoxLayout* t2l = new QVBoxLayout(t2);
        t2l->addWidget(new QLabel(L"第二个标签页：列表"));
        QListWidget* list = new QListWidget();
        list->addItem(L"项目 1");
        list->addItem(L"项目 2");
        list->addItem(L"项目 3");
        list->addItem(L"项目 4");
        list->addItem(L"项目 5");
        t2l->addWidget(list);
        tabs->addTab(t2, L"列表");
        root->addWidget(tabs, 2);
        return page;
    }

    void buildCentral() {
        QSplitter* splitter = new QSplitter(Qt::Horizontal);
        splitter->addWidget(buildControlsTab());

        QWidget* side = new QWidget();
        QVBoxLayout* sl = new QVBoxLayout(side);
        sl->setContentsMargins(12, 12, 12, 12);
        QLabel* treeTitle = new QLabel(L"目录树 / Tree");
        treeTitle->setMinimumSize(QSize(0, 24));
        QFont f = treeTitle->font();
        f.setBold(true);
        treeTitle->setFont(f);
        sl->addWidget(treeTitle);
        QTreeWidget* tree = new QTreeWidget();
        QStringList th;
        th << L"节点";
        tree->setHeaderLabels(th);
        QTreeWidgetItem* root1 = new QTreeWidgetItem(QStringList() << L"项目");
        root1->addChild(new QTreeWidgetItem(QStringList() << L"src"));
        root1->addChild(new QTreeWidgetItem(QStringList() << L"include"));
        root1->addChild(new QTreeWidgetItem(QStringList() << L"docs"));
        root1->setExpanded(true);
        QTreeWidgetItem* root2 = new QTreeWidgetItem(QStringList() << L"构建");
        root2->addChild(new QTreeWidgetItem(QStringList() << L"out"));
        tree->addTopLevelItem(root1);
        tree->addTopLevelItem(root2);
        sl->addWidget(tree, 1);

        QPushButton* themeButton = new QPushButton(L"切换亮色 / 暗色");
        QObject::connect(themeButton, themeButton->clicked, this, &GalleryWindow::toggleTheme);
        sl->addWidget(themeButton);
        splitter->addWidget(side);
        splitter->setSizes(QList<int>() << 640 << 320);
        setCentralWidget(splitter);
    }

    void buildMenus() {
        menuBar_ = new QMenuBar();
        setMenuBar(menuBar_);

        QMenu* fileMenu = menuBar_->addMenu(L"文件");
        QAction* newAction = fileMenu->addAction(L"新建");
        newAction->setShortcut(L"Ctrl+N");
        QObject::connect(newAction, newAction->triggered, this, &GalleryWindow::onNew);
        QAction* saveAction = fileMenu->addAction(L"保存");
        saveAction->setShortcut(L"Ctrl+S");
        fileMenu->addSeparator();
        QAction* quitAction = fileMenu->addAction(L"退出");
        quitAction->setShortcut(L"Ctrl+Q");
        QObject::connect(quitAction, quitAction->triggered, this, &GalleryWindow::onQuit);

        QMenu* viewMenu = menuBar_->addMenu(L"视图");
        QAction* light = viewMenu->addAction(L"亮色 Light");
        QObject::connect(light, light->triggered, this, &GalleryWindow::onThemeLight);
        QAction* dark = viewMenu->addAction(L"暗色 Dark");
        QObject::connect(dark, dark->triggered, this, &GalleryWindow::onThemeDark);
        QAction* system = viewMenu->addAction(L"跟随系统 System");
        QObject::connect(system, system->triggered, this, &GalleryWindow::onThemeSystem);
        viewMenu->addSeparator();
        QAction* about = viewMenu->addAction(L"关于");
        QObject::connect(about, about->triggered, this, &GalleryWindow::onAbout);

        toolBar_ = new QToolBar();
        addToolBar(toolBar_);
        toolBar_->addAction(newAction);
        toolBar_->addAction(saveAction);
        toolBar_->addSeparator();
        toolBar_->addAction(light);
        toolBar_->addAction(dark);
    }

    void buildStatusBar() {
        QStatusBar* bar = new QStatusBar();
        setStatusBar(bar);
        bar->showMessage(L"就绪 / Ready");
    }

    void toggleTheme() {
        if (QTheme::isDark()) QTheme::setMode(QTheme::Light);
        else QTheme::setMode(QTheme::Dark);
        update();
    }

    void onPrimary(bool) { setStatus(L"主要按钮 clicked"); }
    void onNormal(bool) { setStatus(L"普通按钮 clicked"); }
    void onNew(bool) { setStatus(L"新建 / New"); }
    void onQuit(bool) { QApplication::quit(); }
    void onAbout(bool) {
        QMessageBox::about(this, L"关于 QtClone",
                           L"QtClone 1.0 - Qt 风格 C++ UI 框架\n"
                           L"无 moc、无第三方依赖，支持 VS2010。");
    }
    void onThemeLight(bool) { QTheme::setMode(QTheme::Light); }
    void onThemeDark(bool) { QTheme::setMode(QTheme::Dark); }
    void onThemeSystem(bool) { QTheme::setMode(QTheme::System); }
    void onTextChanged(const QString& text) {
        setStatus(L"文本：" + text);
    }
    void onComboChanged(int index) {
        setStatus(QString(L"选择项 index=") + QString::number(index));
    }
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setStyleSheet(QString());

    GalleryWindow window;
    printf("window built\n");
    window.show();
    printf("window shown\n");

    const char* shot = 0;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) shot = argv[++i];
        else if (strcmp(argv[i], "--theme") == 0 && i + 1 < argc) {
            const char* theme = argv[++i];
            if (strcmp(theme, "dark") == 0) QTheme::setMode(QTheme::Dark);
            else if (strcmp(theme, "light") == 0) QTheme::setMode(QTheme::Light);
            else QTheme::setMode(QTheme::System);
        } else if (strcmp(argv[i], "--qss") == 0 && i + 1 < argc) {
            QApplication::setStyleSheet(QString::fromUtf8(argv[++i]));
        }
    }

    for (int i = 0; i < 20; ++i) QApplication::processEvents();

    if (shot) {
        QPixmap pm = window.grab();
        bool ok = pm.save(QString::fromUtf8(shot));
        printf("gallery screenshot %s (%dx%d) -> %s\n", ok ? "saved" : "FAILED",
               pm.width(), pm.height(), shot);
        return ok ? 0 : 1;
    }
    return QApplication::exec();
}
