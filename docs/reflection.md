# 无 moc 的元对象实现

Qt 的核心魔力来自 moc：扫描头文件、生成元数据与信号函数体。QtClone 刻意**不做代码生成**
（构建步骤更少、VS2010 集成更简单），把同样的能力放进普通 C++ 模板与虚函数里。
本文说明其机理、用法与代价。

---

## 1. `QTC_OBJECT`

```cpp
#define QTC_OBJECT(CLASS, BASE)                                   \
public:                                                           \
    static const QMetaObject staticMetaObject;                     \
    virtual const QMetaObject* metaObject() const { return &CLASS::staticMetaObject; } \
    virtual QVariant qtcReadProperty(int id) const;                \
    virtual bool qtcWriteProperty(int id, const QVariant& value);  \
private:                                                          \
    typedef void qtc_metaobject_tag;
```

它只做三件事：

1. 为类声明静态元对象（POD，在 `.cpp` 中聚合初始化）。
2. 让 `metaObject()` 返回自己（不是基类）的元对象。
3. 声明两个属性访问虚函数，供每个类实现自己的 `switch`。

---

## 2. 元对象表

```cpp
static const QMetaObject::PropertyData qtc_props_QLabel[] = {
    { "text", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "alignment", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable,
      "Qt::Alignment" },
    ...
};
static const QMetaObject::SignalInfo qtc_signals_QLabel[] = {
    { "linkActivated", "linkActivated()" },
};

const QMetaObject QLabel::staticMetaObject = {
    &QFrame::staticMetaObject, "QLabel", qtc_props_QLabel, 6, qtc_signals_QLabel, 2
};
```

* 聚合初始化 → **常量初始化**，没有跨 TU 的静态初始化顺序问题。
* `propertyCount()/propertyOffset()` 沿继承链递归计算，属性的绝对索引 = 基类属性数 + 本地索引。
* `SignalInfo` 只用于设计器的"事件"列表与代码生成，不参与运行期连接。

---

## 3. 属性读写

```cpp
QVariant QLabel::qtcReadProperty(int id) const {
    const int base = QFrame::staticMetaObject.propertyCount();
    if (id < base) return QFrame::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(text_);
    case 1: return QVariant((int)alignment_);
    ...
    }
    return QVariant();
}

bool QLabel::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QFrame::staticMetaObject.propertyCount();
    if (id < base) return QFrame::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setText(value.toString()); return true;
    ...
    }
    return false;
}
```

于是：

```cpp
label->setProperty("alignment", QVariant((int)(Qt::AlignCenter)));   // 设计器/属性网格
QVariant v = label->property("text");                                // 反射读取
```

优点：类型安全、无宏魔法、可知可控；缺点：**新增属性要手写两处**（表 + switch），
但仍远少于 moc 的构建集成成本。

> 约定：`switch` 的下标必须与属性表顺序一致；源码中各控件类均相邻书写，便于核对。

---

## 4. 信号与连接

```cpp
class QAbstractButton : public QWidget {
    QTC_OBJECT(QAbstractButton, QWidget)
signals:                                   // = public:
    Signal<void(bool)> clicked;
    Signal<void()> pressed;
    Signal<void(bool)> toggled;
};

// 发射（emit 为空宏）
emit clicked(isChecked());

// 连接
QObject::connect(button, button->clicked, this, &Window::onClicked);
// 槽可少参
QObject::connect(button, button->clicked, this, &Window::onClickedNoArgs);
```

实现要点：

* `Signal<Sig>` 对 `void()`、`void(A1)`、`void(A1,A2)`、`void(A1,A2,A3)` 特化；
  `const T&` 参数用 trait 拆成存储类型与形参类型。
* `QObject::connect` 是模板，签名在编译期确定；回调对象（`QSlot1<R,A1>` 等）把
  "对象 + 成员函数指针"保存下来，`call(void**)` 里做静态转型并调用。
* 连接表是全局 vector；`emitArgs` 先快照再派发，槽里断开/删除接收者都安全
  （派发前检查存活集合）。
* 对象析构会移除所有相关连接；信号成员析构会移除自身连接。没有悬空回调。

---

## 5. 与 moc 的取舍

| | moc（Qt） | QtClone |
| --- | --- | --- |
| 构建步骤 | 每个含 `Q_OBJECT` 的头文件跑 moc | 无 |
| 信号声明 | `void clicked(bool);` | `Signal<void(bool)> clicked;` |
| 连接 | `SIGNAL()/SLOT()` 字符串或成员指针 | 成员指针（类型安全） |
| 属性声明 | `Q_PROPERTY(...)` | 静态表 + 虚函数 switch |
| 字符串反射 | 方法/属性统一索引 | 属性表 + 信号名表 |
| 编译期开销 | moc 生成代码参与编译 | 模板实例化，略大但可控 |

---

## 6. 自定义可设计控件

```cpp
class Gauge : public QWidget {
    QTC_OBJECT(Gauge, QWidget)
public:
    Gauge(QWidget* parent = 0) : QWidget(parent), value_(0), max_(100) {}
    int value() const { return value_; }
    void setValue(int v) { value_ = qBound(0, v, max_); update(); }
signals:
    Signal<void(int)> valueChanged;
protected:
    void paintEvent(QPaintEvent*) { /* 画表盘 */ }
private:
    int value_;
    int max_;
};

static const QMetaObject::PropertyData qtc_props_Gauge[] = {
    { "value", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "maximum", "int", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
const QMetaObject Gauge::staticMetaObject = {
    &QWidget::staticMetaObject, "Gauge", qtc_props_Gauge, 2, 0, 0
};
QVariant Gauge::qtcReadProperty(int id) const { /* 标准写法 */ }
bool Gauge::qtcWriteProperty(int id, const QVariant& v) { /* 标准写法 */ }

QTC_REGISTER_WIDGET(Gauge, "Display", "Gauge", "<my/Gauge.h>")
```

注册宏把类名/分组/标签/工厂函数写入 `QWidgetFactory`，设计器工具箱会自动出现，
属性网格自动列出 `value/maximum`。
