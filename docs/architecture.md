# 架构

QtClone 的结构与 Qt Widgets 同构：**QtCore（对象/事件/属性）→ QtGui（绘制/窗口）→ QtWidgets（控件）→ 设计器/工具**。
本文说明各层职责、事件与绘制管线，以及关键设计取舍。

---

## 1. 分层

```
┌────────────────────────────────────────────────────────────┐
│ 应用代码 / 设计器代码生成 (ui_Form.h)                        │
├────────────────────────────────────────────────────────────┤
│ QtWidgets : QWidget QLabel QPushButton … QMainWindow        │
│             QLayout (QBox/QGrid/QForm/QStacked)             │
│             QStyle + QStyleSheet                            │
├────────────────────────────────────────────────────────────┤
│ QtGui     : QPainter(GDI+) QPixmap QImage QColor/QFont      │
│             QWidgetWindow (HWND, 输入, IME, 后台位图)         │
├────────────────────────────────────────────────────────────┤
│ QtCore    : QObject QMetaObject Signal<> QEvent QVariant     │
│             QString QList QXml QTimer 事件循环               │
└────────────────────────────────────────────────────────────┘
```

每个源文件都只依赖下层；`src/gui/qwidget_p.h` 是窗口/控件的内部结构（不安装）。

---

## 2. 元对象与属性（无 moc）

* `QMetaObject` 是 **POD 聚合体**，每个类在 `.cpp` 里静态初始化：
  `{ &Base::staticMetaObject, "QPushButton", props, n, signals, m }`。
  聚合初始化是常量初始化，不存在跨编译单元的静态初始化顺序问题。
* 属性表只存**名字/类型/标志/枚举名**（供设计器展示）；值的读写由每个类自己实现的
  `qtcReadProperty(int)` / `qtcWriteProperty(int, QVariant)` 虚函数完成，索引沿继承链绝对编号，
  派生类先减去基类属性数量再 `switch`。
* `QObject::property("text")` / `setProperty("text", v)` 即走上述路径，因此设计器、
  `.ui` 加载器和属性网格不需要任何生成代码。

细节与示例见 `docs/reflection.md`。

---

## 3. 信号槽

* 信号是成员对象：`Signal<void(int)> valueChanged;`，模板特化固定 0~3 个参数。
* `emit` 为空宏，`emit sig(a)` 就是 `sig(a)`，即 `Signal::operator()`。
* `QObject::connect(sender, sig, receiver, &R::slot)` 把 `(signal, sender, receiver)` 和
  一个类型擦除的 `QSlotCallback` 注册到全局连接表；槽参数少于信号参数时使用忽略尾参的重载。
* 发射时对连接表做快照，逐个检查对象存活（全局 alive 集合）与 `signalsBlocked()`，
  再调用回调；回调里通过 `qtcCurrentSender()` 支持 `sender()` 语义。
* 对象析构时自动移除其作为 sender/receiver 的连接；信号对象析构时移除自身连接。

---

## 4. 事件与输入

`QWidgetWindow` 为每个顶层窗口持有 HWND。WndProc 把 Win32 消息翻译成 `QEvent`：

| Win32 | QtClone |
| --- | --- |
| WM_MOUSEMOVE / BUTTON* / MOUSEWHEEL | `QMouseEvent` / `QWheelEvent`，命中测试到最深层控件，坐标换算为控件局部坐标 |
| WM_KEYDOWN / WM_CHAR | `QKeyEvent`（控制键 text 为空；字符输入走 WM_CHAR） |
| WM_IME_* | `QInputMethodEvent`（preedit + commit），候选窗口定位到焦点控件 |
| WM_PAINT / WM_SIZE | 后台位图重绘、`QResizeEvent`、重新布局 |
| WM_SETCURSOR | 按控件 `cursor()` 设置光标 |
| WM_SETTINGCHANGE | `ImmersiveColorSet` 时重新探测系统暗色模式 |

* **鼠标捕获**：按下时记录 grabber 并 `SetCapture`；拖动/释放始终发给 grabber。
* **悬浮**：按控件祖先链发送 `Enter/Leave`，驱动 `:hover` 伪状态。
* **焦点**：窗口级焦点链（DFS + TabFocus），Tab/Shift+Tab 前进后退，弹出窗口单独处理。
* 事件过滤器：`installEventFilter`，在 `qtcSendEvent` 中先于 `event()` 调用。

---

## 5. 绘制管线

采用 Qt 的 **alien widget** 模型：只有顶层窗口有 HWND，子控件是纯逻辑对象。

```
QWidgetWindow::paint(dc)
  ├─ 若后台位图尺寸不符 → 重建并填窗口色
  ├─ QPainter p(&backBuffer)            // PARGB 位图
  ├─ p.setClipRect(脏矩形)
  ├─ root->renderInternal(&p, (0,0), clip)
  │    ├─ p.save(); p.translate(widget->pos())
  │    ├─ 压入绘制目标（Graphics* + 绝对原点）
  │    ├─ paintSelf: paintBackground → paintEvent
  │    ├─ 递归子控件（同样方式）
  │    └─ p.restore()
  └─ GDI+ DrawImage(backBuffer → 窗口 DC, 脏矩形)
```

* `QPainter(QWidget*)` 从**当前绘制目标**借用同一个 `Graphics`（保存/恢复其状态），
  因此控件在自己的 `paintEvent` 里直接 `QPainter p(this)` 即可，坐标系自动是控件局部坐标。
* 纹理/文字/圆角全部走 GDI+ 抗锯齿；后台位图是 32bpp PARGB，支持半透明与阴影。
* 脏矩形累积在 `QRegion`，`update()` 只标记区域，`WM_PAINT` 只重绘该区域。
* `QWidget::grab()` 与设计器画布复用同一管线，把控件树渲染到离屏 `QPixmap`
  （`--screenshot` 模式即基于此，无需真实屏幕）。

---

## 6. 布局

* `QLayoutItem` 抽象（`QWidgetItem` / `QSpacerItem` / 嵌套 `QLayout`）。
* `QLayout` 保存 item 列表、边距、间距；`setGeometry()` 由父控件在 resize 时调用。
* `QBoxLayout` 使用自写的 `distribute()`：以 sizeHint 为基准，先施加 min/max 约束，
  剩余空间按 stretch 分配，没有 stretch 时按 `Expanding` 策略分配；空间不足时按 min 收缩。
* `QWidgetItem` 尊重 `QSizePolicy`：`Fixed`/`Maximum` 的轴最大值取 sizeHint，
  因此按钮/复选框不会被布局拉高。
* `QGridLayout` 先按跨行列计算行高列宽，再分别对列、行做一次 `distribute()`。
* `QFormLayout` 两列（标签/字段），行高取两者 hint 的较大值。

---

## 7. 样式与主题

* `QStyle` 是绘制原语的唯一实现（按钮、复选框、进度条、滚动条、菜单项、表头…），
  所有颜色来自 `QTheme::colors()`（语义色板）。
* `QStyleSheet` 解析样式表并计算每个控件的 `Box`（背景/边框/圆角/内边距/字体/前景色），
  结果按控件缓存，`setStyleSheet`/主题切换时失效。
* 主题：`QTheme::Mode { Light, Dark, System }`；`System` 读取
  `HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme`，
  并在窗口收到 `WM_SETTINGCHANGE("ImmersiveColorSet")` 时重新探测，自动切换。
* 切换主题会重建应用调色板、清空 QSS 缓存、给所有控件发送 `QEvent::ThemeChange` 并全量重绘。

---

## 8. 线程与生命周期

* 所有 GUI 对象必须在主线程创建和销毁（与 Qt 相同的线程约束）。
* 对象所有权是父-子树：删除父对象会删除全部子对象；控件析构时自动从父窗口的
  画布/焦点/捕获状态中摘除。
* 全局存活集合保证信号发射、投递事件不会访问已销毁对象。
* `deleteLater()` 通过投递事件在消息循环中安全删除。

---

## 9. 已知限制（生产使用前请确认）

| 限制 | 说明 / 规避 |
| --- | --- |
| 设计器仅绝对布局 | 控件使用绝对 geometry；不生成 QLayout（Qt Designer 默认布局容器） |
| 无 QML / Quick | 本框架有意只做 Widgets 编程模型 |
| item-based 视图 | `QListWidget/QTreeWidget/QTableWidget` 为便捷类；无 `QAbstractItemModel` 自定义模型/委托 |
| 无 OpenGL/图形视图 | 绘制为 GDI+ 软件光栅 |
| 无障碍 | 未实现 Microsoft UIA / MSAA |
| 高 DPI | 调用 `SetProcessDPIAware()`，按 96 DPI 解释字体；未做 per-monitor v2 缩放 |
| IME | 基础 preedit/commit；复杂输入法（如自绘候选窗）未覆盖 |
| 线程 | GUI 单线程；无跨线程排队连接 |
| 剪贴板 | 仅文本（`QClipboard::text/setText`） |
| 菜单 | 子菜单点击/悬停打开、键盘上下/回车；无滚轮分页与加速键匹配 |

这些边界在头文件与本文中明确标注，避免把"能编译"误当成"完整 Qt"。

## 输入诊断

怀疑“界面点击没反应”时，设置环境变量后启动任意 demo：

```bat
set QTCLONE_CLICK_DEBUG=1
out\wechat.exe
```

- 每次鼠标按下都会在点击位置画一个红十字；
- 同时把坐标和目标控件类名追加到 `%TEMP%\qtclone-clicks.log`，例如：
  `press 192,245 target=QWidget`

日志里有记录 = 鼠标消息到达了窗口（问题在控件状态/重绘）；日志为空 = 点击根本没到该窗口
（被别的窗口挡住、或输入设备/远程会话问题）。


## 绘制裁剪

`QWidget::renderInternal` 在递归绘制子控件时，会把子控件的裁剪区域与父控件矩形求交：
**子控件永远不会画到父控件之外**。因此滚动/平移容器内容、或把子控件移到负坐标时，
不会“漏”到相邻面板上。


## 点击与重绘

鼠标按下/抬起后，窗口会 `updateAll()` + `UpdateWindow()`，**保证整窗立即重绘一次**。
原因是部分控件在窗口创建前就已挂到父控件上（`ownerWindow` 为空），
或位于裁剪视口内，它们的 `update()` 可能被丢掉；不强制重绘时会出现
“状态已经变了，但界面看起来没反应”。这条保证让任何点击都至少能看到结果。


## 构建戳与点击回归

**构建戳**：`QApplication::buildInfo()` 读取当前 exe 的文件时间（编译链接时间），
各程序把它附加在窗口标题末尾，例如 `[build 2026-11-17 09:15]`。
排查“点击没反应”时先看标题，确认跑的不是旧二进制。

**点击回归**：`tests/clicktest.cpp` 建立真实窗口，走
`WM_LBUTTONDOWN/UP → 命中测试 → 事件分发 → 重绘` 全链路，并断言三件事：

1. 被点的控件收到事件（状态变化）；
2. 屏幕像素发生变化（用 BitBlt 抓屏幕，不用 PrintWindow，避免强制重绘掩盖问题）；
3. 一个**故意不调用 `update()`** 的控件也能显示新状态（覆盖“状态变了但没重绘”这一类回归）。

它的构建/运行已接入 `build.bat` 第 6 步和 VS 的 `wf_clicktest` 项目；失败即中断构建。
移除 `handleMouse` 里的强制重绘后，这个测试会立刻失败——可用来验证兜底逻辑是否还在。


## 退出与模态对话框

- **关闭最后一个可见顶层窗口会结束进程**（等价于 Qt 的 `quitOnLastWindowClosed`）。
  以前窗口消失后进程还在，看起来就是“退不出”。
- `QDialog::exec()` 是真正的嵌套消息循环：禁用 owner、`show()`、循环 `GetMessage/DispatchMessage`。
  对话框会**居中到 owner 窗口**，而不是停在 (0,0)。
- `QMessageBox` 的尺寸按文本和按钮数量计算，不再沿用 800×600 的默认窗口。

### 一个容易踩的坑：三参数 connect

`Signal::connect(sender, receiver, slot)` 会把传入的 `sender` 记成“当前发送者”。
如果写成 `button->clicked.connect(this, this, &Box::onButtonClicked)`，
发送者会被记成**对话框**而不是按钮，槽里用 `qtcCurrentSender()` 比较按钮时永远匹配不上——
点击其实到了，但什么都没发生。必须用四参数形式：

```cpp
QObject::connect(button, button->clicked, this, &Box::onButtonClicked);
```

`clicktest` 里专门有一条用例：点击 QMessageBox 的标准按钮，断言对话框会关闭；
把 connect 改回三参数形式，这条用例立刻失败。


## 弹出菜单坐标

`QMenu` 是顶层弹出窗口（`Qt::Popup`），几何位置就是**屏幕坐标**。
`QMenu::popup(globalPos)` 以前会先把 globalPos 映射成父控件局部坐标再 `setGeometry`，
于是菜单被整体上移/左移了一个“客户区原点”（标题栏高度、菜单栏偏移）——
表现为菜单弹在点击项的上方、左边错位。

正确做法：窗口直接用 globalPos，只有非窗口控件才做 `mapFromGlobal`。

```cpp
QPoint pos = globalPos;
if (parentWidget() && !isWindow()) pos = parentWidget()->mapFromGlobal(globalPos);
setGeometry(QRect(pos, size()));
```

`clicktest` 里新增了回归用例：`popup()` 之后断言菜单几何位置与请求的全局坐标一致，
并且下沿在菜单栏之下。改回旧写法时该用例失败。


## 子控件裁剪（嵌套 painter）

`renderInternal` 给每个控件设置裁剪后，会在 `qtcPushPaintTarget` 里把
**当前有效裁剪（控件局部坐标）** 一并压栈。控件在 `paintEvent` 里新建
`QPainter p(this)` 时继承这个裁剪，而不是只用自己的 `rect()`。

如果不继承：一个被移动到负坐标的子控件（例如滚动画布里的大窗体）会以
自己的矩形为界绘制，直接盖到菜单栏和工具栏上——表现为“滚动后顶部变白”。
修复前 `QPainter(QWidget*)` 用 `widget->rect()` 作为裁剪，父级裁剪全部丢失。

`QTCLONE_CLICK_DEBUG=1` 之外还有 `QTC_PAINT_DEBUG=1`，会把每次
`renderInternal` 的类名、绝对原点、尺寸与裁剪写进 `out/paint.log`，
排查“谁画到了别处”时很有用。
