# 样式表与主题

QtClone 的默认外观由 `QStyle` + 语义色板决定，可用 Qt 风格样式表（QSS 子集）覆盖。

---

## 1. 主题

```cpp
QTheme::setMode(QTheme::Light);
QTheme::setMode(QTheme::Dark);
QTheme::setMode(QTheme::System);      // 跟随 Windows 应用主题
bool dark = QTheme::isDark();          // 当前实际生效
const QTheme::Colors& c = QTheme::colors();
```

* **Dark 是近黑**：窗口 `#121417`、输入框 `#17191D`、面板 `#1A1D22`、边框 `#2E333B`、
  强调色 `#4C8DFF`。
* **System** 读取
  `HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme`，
  窗口收到 `WM_SETTINGCHANGE`（`ImmersiveColorSet`）时重新探测并自动切换。
* 切换会：重建 `QPalette` → 清空 QSS 缓存 → 给所有控件发送 `QEvent::ThemeChange` → 全量重绘。

语义色（`QTheme::Colors`）：`window / windowText / base / alternateBase / surface / surfaceAlt /
border / borderStrong / button / buttonHover / buttonPressed / buttonText / accent / accentText /
accentHover / disabledText / gridLine / selection / selectionText / toolTipBase / toolTipText /
header / headerText / menuBar / menuPopup / scrollbar / scrollbarHandle / canvas / shadow`。
自带控件一律使用这些颜色，因此换主题不需要逐个控件改代码。

---

## 2. 样式表

```cpp
// 控件级
button->setStyleSheet(L"QPushButton { background-color: #0078D4; color: white; }");

// 应用级（优先级最低，控件级覆盖它）
QApplication::setStyleSheet(L"QLineEdit { border-radius: 6px; padding: 3px 8px; }");
```

### 选择器

| 写法 | 含义 |
| --- | --- |
| `QPushButton` | 类型（含派生类，按 `inherits()` 判断） |
| `#okButton` | `objectName` 完全匹配 |
| `QPushButton#okButton` | 类型 + 名称 |
| `QDialog QPushButton` | 后代选择器（空格，可多级） |
| `QPushButton:hover` | 伪状态 |
| `QPushButton, QToolButton` | 逗号分隔多个选择器 |

伪状态：`:hover` `:pressed` `:checked` `:disabled` `:enabled` `:focus` `:on` `:off` `:editable`。

优先级 = id(100) + 类型(10) + 伪状态(10)；同优先级按"后出现者覆盖"，
控件自身的样式表优先于祖先样式表，祖先优先于应用样式表。

### 属性

| 属性 | 值 |
| --- | --- |
| `background-color` / `background` | 颜色（`#RGB/#RRGGBB/#AARRGGBB/rgb()/颜色名`） |
| `color` | 文本颜色 |
| `border` | `1px solid #ccc`（宽度/样式/颜色） |
| `border-color` / `border-width` | 单独设置 |
| `border-radius` | 圆角半径（px） |
| `padding` / `padding-left/right/top/bottom` | 内边距（1~4 值） |
| `font-family` / `font-size` / `font-weight` / `font-style` | 字体 |
| `min-width` / `min-height` / `max-width` / `max-height` | 参与布局的尺寸约束 |
| `selection-background-color` / `selection-color` | 选中文本颜色 |

### 生效范围

`QWidget::paintBackground()` 会读取控件计算出的 QSS 盒模型：有 `background-color` 时
填充（带圆角），有 `border` 时描边；`QStyle` 的原语绘制会读取 `color`/`font`/圆角/边框，
因此 QSS 对原生自绘控件（按钮、输入框、进度条、滚动条、菜单…）同样生效。

> 注意：与 Qt 相同，**选择器不能省略**。`setStyleSheet("background: red")` 不会生效，
> 应写成 `setStyleSheet("QWidget#name { background: red; }")` 或类型选择器。

---

## 3. 配色示例

```cpp
// 亮色卡片
panel->setStyleSheet(L"QWidget#card { background-color: #FFFFFF; border: 1px solid #D8DEE8;"
                     L" border-radius: 10px; }");

// 暗色主题下的强调按钮
okButton->setStyleSheet(L"QPushButton { background-color: #4C8DFF; color: #0B0D10;"
                        L" border-radius: 6px; padding: 4px 14px; font-weight: bold; }");
```

实现见 `src/gui/qstylesheet.cpp`（解析/匹配/盒模型）与 `src/gui/qstyle.cpp`（绘制原语）。
