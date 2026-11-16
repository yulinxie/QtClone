# 设计器与 .ui 代码生成

`wfdesigner.exe` 是一个 Qt Designer 风格的可视化表单编辑器；`qtcuic.exe` 把 `.ui`
编译成 C++ 头文件。两者共享同一套 `.ui` 解析与生成代码（`tools/uicgen`）。

---

## 1. 工作流

```
新建/打开 .ui ──► 设计器画布（真实控件） ──► 保存 .ui
                              │
                              ├─ F5 预览（离屏重建一个窗口）
                              └─ F7 生成代码 ──► ui_<form>.h ──► 编译进你的工程
```

命令行：

```bat
wfdesigner.exe --open examples\form\Form1.ui
wfdesigner.exe --screenshot out.png [--theme dark|light|system]
wfdesigner.exe --selftest          :: 无界面自测（CI 用）

qtcuic.exe Form1.ui -o ui_form1.h   :: 只做代码生成
qtcuic.exe Form1.ui                 :: 输出到同目录 ui_form1.h（默认）
```

---

## 2. 界面

| 区域 | 操作 |
| --- | --- |
| 工具箱 | 顶部过滤框；双击条目添加控件，或把条目拖到画布上指定位置；控件加进当前选中的容器，否则加到窗体（Dialog/MainWindow/MessageBox 属于顶层窗口，不在工具箱里） |
| 画布 | 单击选中（8 个白色手柄）；Ctrl+单击增减选中、Shift+单击追加、空白处拖框多选；多选后整体拖动、整体删除/复制；拖动手柄缩放（不小于控件最小尺寸）；网格 8px 吸附（Alt 关闭）；方向键 1px，Shift 8px；拖动时按住 Shift 锁定单轴移动；Esc 取消选中；Tab / Shift+Tab 切换控件；右键菜单：删除、复制、置于顶层/底层、水平/垂直居中、左/右/顶/底对齐、水平/垂直分布、全选；双击控件把焦点切到属性网格 |
| 对象树 | 两列（对象 / 类型），层级展示；单击选中并同步画布（按名称查找，支持嵌套）；双击重命名；右键重命名、删除、置于顶层/底层 |
| 属性网格 | 顶部过滤框；单击文本/数字单元格就地编辑（Enter 提交、Esc 取消）；布尔单击切换；枚举下拉；颜色/字体调系统对话框；几何用 `x,y,w,h` 文本双击编辑 |
| 状态栏 | 显示当前选中与文件路径 |

快捷键：`Ctrl+N` 新建、`Ctrl+O` 打开、`Ctrl+S` 保存、`Ctrl+Shift+S` 另存、
`Ctrl+Z/Y` 撤销/重做、`Ctrl+D` 复制、`Delete` 删除、`F5` 预览、`F7` 生成代码。

撤销/重做以 `.ui` 快照实现（最多 200 步），任何属性或几何修改都会入栈。

---

## 3. .ui 文档格式

与 Qt Designer 兼容的子集：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>Form1</class>
 <widget class="QWidget" name="Form1">
  <property name="geometry">
   <rect><x>0</x><y>0</y><width>460</width><height>280</height></rect>
  </property>
  <property name="windowTitle"><string>客户信息 / Customer</string></property>

  <widget class="QLineEdit" name="nameEdit">
   <property name="geometry">
    <rect><x>104</x><y>68</y><width>320</width><height>28</height></rect>
   </property>
   <property name="placeholderText"><string>请输入姓名</string></property>
  </widget>
 </widget>
 <connections/>
</ui>
```

值元素：`<string>` `<bool>` `<number>` `<double>` `<rect>` `<size>` `<point>` `<color>`
`<font>` `<stringlist>`。属性名与 `QMetaObject` 的属性表一致。

> `connections` 目前写空：设计器不编辑信号槽连接，事件处理在生成的窗类里手写
> （这样生成的代码不依赖不存在的槽函数）。

---

## 4. 生成代码

`Form1.ui` → `ui_form1.h`：

```cpp
class Ui_Form1 {
public:
    QLineEdit* nameEdit;
    QPushButton* okButton;
    void setupUi(QWidget* Form1) {
        Form1->setObjectName(QString::fromUtf8("Form1"));
        Form1->resize(460, 280);
        nameEdit = new QLineEdit(Form1);
        nameEdit->setObjectName(QString::fromUtf8("nameEdit"));
        nameEdit->setGeometry(QRect(104, 68, 320, 28));
        ...
        retranslateUi(Form1);
    }
    void retranslateUi(QWidget* Form1) { /* 文本属性 */ }
};
namespace Ui { class Form1 : public Ui_Form1 {}; }
```

* 字符串一律用 `QString::fromUtf8("...")` + 转义字节，**中文在任何代码页下都安全**。
* 非字符串属性通过 `setProperty("name", QVariant(...))` 写入，复用同一套反射。
* 文件带 UTF-8 BOM，MSVC 2010 可直接编译。

使用示例（`examples/form/main.cpp`）：

```cpp
#include "ui_form1.h"

class Form1 : public QWidget {
public:
    Form1() {
        ui_.setupUi(this);
        ui_.levelCombo->addItems(QStringList() << L"普通客户" << L"重要客户");
        QObject::connect(ui_.okButton, ui_.okButton->clicked, this, &Form1::onOk);
    }
private:
    void onOk(bool) { QMessageBox::information(this, L"确定", ui_.nameEdit->text()); }
    Ui_Form1 ui_;
};
```

---

## 5. VS2010 自定义生成步骤

`wf_form.vcxproj` 中（`.ui` 的 CustomBuild）：

```xml
<CustomBuild Include="examples\form\Form1.ui">
  <Message>qtcuic %(Identity)</Message>
  <Command>if not exist "$(IntDir)" mkdir "$(IntDir)"
"$(SolutionDir)out\$(Configuration)\qtcuic.exe" "%(FullPath)" -o "$(IntDir)ui_form1.h" -q</Command>
  <Outputs>$(IntDir)ui_form1.h;%(Outputs)</Outputs>
  <AdditionalInputs>$(SolutionDir)out\$(Configuration)\qtcuic.exe;%(AdditionalInputs)</AdditionalInputs>
</CustomBuild>
```

并在项目的 `AdditionalIncludeDirectories` 里加入 `$(IntDir)`，
这样 `#include "ui_form1.h"` 能找到生成文件。`qtcuic` 作为 ProjectReference 先构建。

---

## 6. 设计器自测

`wfdesigner.exe --selftest` 会（无窗口交互）：

1. 新建文档并添加 `QPushButton`、`QLabel`；
2. 通过属性系统设置 `text/geometry`，检查 `.ui` 序列化包含对应节点；
3. 文档 XML 往返一致性；
4. 调用 `uicGenerateHeader` 检查生成的类名/成员/几何；
5. 删除 → 撤销 → 重做，校验控件数量变化。

`build.bat` 第 6 步会执行它，失败即构建失败。
