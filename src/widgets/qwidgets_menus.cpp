// QtClone - qwidgets_menus.cpp : actions, menus, menu bar, tool bar, status bar, main window.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include "qwidget_p.h"
#include <string.h>

// ===========================================================================
// QAction
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QAction[] = {
    { "text", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "shortcut", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "enabled", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "checkable", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "checked", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "toolTip", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "statusTip", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 },
    { "separator", "bool", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QAction[] = {
    { "triggered", "triggered(bool)" },
    { "toggled", "toggled(bool)" },
    { "hovered", "hovered()" },
    { "changed", "changed()" }
};
const QMetaObject QAction::staticMetaObject = {
    &QObject::staticMetaObject, "QAction",
    qtc_props_QAction, 8, qtc_signals_QAction, 4
};

QVariant QAction::qtcReadProperty(int id) const {
    const int base = QObject::staticMetaObject.propertyCount();
    if (id < base) return QObject::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(text_);
    case 1: return QVariant(shortcut_);
    case 2: return QVariant(enabled_);
    case 3: return QVariant(checkable_);
    case 4: return QVariant(checked_);
    case 5: return QVariant(toolTip_);
    case 6: return QVariant(statusTip_);
    case 7: return QVariant(separator_);
    default: break;
    }
    return QVariant();
}

bool QAction::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QObject::staticMetaObject.propertyCount();
    if (id < base) return QObject::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setText(value.toString()); return true;
    case 1: setShortcut(value.toString()); return true;
    case 2: setEnabled(value.toBool()); return true;
    case 3: setCheckable(value.toBool()); return true;
    case 4: setChecked(value.toBool()); return true;
    case 5: setToolTip(value.toString()); return true;
    case 6: setStatusTip(value.toString()); return true;
    case 7: setSeparator(value.toBool()); return true;
    default: break;
    }
    return false;
}

QAction::QAction(QObject* parent)
    : QObject(parent), enabled_(true), checkable_(false), checked_(false),
      separator_(false), data_(0) {}

QAction::QAction(const QString& text, QObject* parent)
    : QObject(parent), text_(text), enabled_(true), checkable_(false), checked_(false),
      separator_(false), data_(0) {}

QAction::QAction(const QIcon& icon, const QString& text, QObject* parent)
    : QObject(parent), text_(text), icon_(icon), enabled_(true), checkable_(false),
      checked_(false), separator_(false), data_(0) {}

QAction::~QAction() {}

void QAction::setText(const QString& text) { text_ = text; changed(); }
void QAction::setChecked(bool on) {
    if (checked_ == on) return;
    checked_ = on;
    if (!checkable_) checkable_ = true;
    changed();
    toggled(checked_);
}
QList<QWidget*> QAction::associatedWidgets() const { return widgets_; }

// ===========================================================================
// QMenu
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QMenu[] = {
    { "title", "QString", QMetaObject::Readable | QMetaObject::Writable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QMenu[] = {
    { "aboutToShow", "aboutToShow()" },
    { "aboutToHide", "aboutToHide()" },
    { "triggeredAction", "triggeredAction(QAction*)" },
    { "hoveredAction", "hoveredAction(QAction*)" }
};
const QMetaObject QMenu::staticMetaObject = {
    &QWidget::staticMetaObject, "QMenu",
    qtc_props_QMenu, 1, qtc_signals_QMenu, 4
};

QVariant QMenu::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(title_);
    default: break;
    }
    return QVariant();
}

bool QMenu::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: setTitle(value.toString()); return true;
    default: break;
    }
    return false;
}

QMenu::QMenu(QWidget* parent)
    : QWidget(parent, Qt::Popup), active_(0), activeIndex_(-1), itemHeight_(26),
      separatorHeight_(9), leftMargin_(26), rightMargin_(28), parentMenu_(0), openSubmenu_(0) {
    setFocusPolicy(Qt::StrongFocus);
}

QMenu::QMenu(const QString& title, QWidget* parent)
    : QWidget(parent, Qt::Popup), title_(title), active_(0), activeIndex_(-1), itemHeight_(26),
      separatorHeight_(9), leftMargin_(26), rightMargin_(28), parentMenu_(0), openSubmenu_(0) {
    setFocusPolicy(Qt::StrongFocus);
}

QMenu::~QMenu() {}

QAction* QMenu::addAction(const QString& text) {
    QAction* action = new QAction(text, this);
    addAction(action);
    return action;
}

QAction* QMenu::addAction(const QIcon& icon, const QString& text) {
    QAction* action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

void QMenu::addAction(QAction* action) {
    if (!action) return;
    actions_.append(action);
    submenus_.append(0);
    action->widgets_.append(this);
    updateGeometryForActions();
}

QAction* QMenu::addMenu(QMenu* menu) {
    if (!menu) return 0;
    QAction* action = new QAction(menu->title(), this);
    actions_.append(action);
    submenus_.append(menu);
    menu->parentMenu_ = this;
    updateGeometryForActions();
    return action;
}

QMenu* QMenu::addMenu(const QString& title) {
    QMenu* menu = new QMenu(title, this);
    addMenu(menu);
    return menu;
}

void QMenu::addSeparator() {
    QAction* action = new QAction(this);
    action->setSeparator(true);
    actions_.append(action);
    submenus_.append(0);
    updateGeometryForActions();
}

void QMenu::insertAction(QAction* before, QAction* action) {
    int index = actions_.indexOf(before);
    if (index < 0) { addAction(action); return; }
    actions_.insert(index, action);
    submenus_.insert(index, 0);
    updateGeometryForActions();
}

void QMenu::insertSeparator(QAction* before) {
    QAction* action = new QAction(this);
    action->setSeparator(true);
    insertAction(before, action);
}

void QMenu::removeAction(QAction* action) {
    int index = actions_.indexOf(action);
    if (index < 0) return;
    actions_.removeAt(index);
    submenus_.removeAt(index);
    updateGeometryForActions();
}

void QMenu::clear() {
    actions_.clearAll();
    submenus_.clearAll();
    active_ = 0;
    activeIndex_ = -1;
    updateGeometryForActions();
}

void QMenu::updateGeometryForActions() {
    QFontMetrics fm(font());
    int width = 120;
    int height = 8;
    for (int i = 0; i < actions_.count(); ++i) {
        if (actions_[i]->isSeparator()) { height += separatorHeight_; continue; }
        height += itemHeight_;
        int w = fm.width(actions_[i]->text()) + leftMargin_ + rightMargin_;
        if (!actions_[i]->shortcut().isEmpty()) w += fm.width(actions_[i]->shortcut()) + 24;
        width = qMax(width, w);
    }
    height += 8;
    resize(width, height);
}

QRect QMenu::actionRect(int index) const {
    int y = 4;
    for (int i = 0; i < index && i < actions_.count(); ++i)
        y += actions_[i]->isSeparator() ? separatorHeight_ : itemHeight_;
    int h = (index >= 0 && index < actions_.count() && actions_[index]->isSeparator()) ? separatorHeight_ : itemHeight_;
    return QRect(0, y, width(), h);
}

int QMenu::indexAt(const QPoint& pos) const {
    for (int i = 0; i < actions_.count(); ++i)
        if (actionRect(i).contains(pos)) return i;
    return -1;
}

QMenu* QMenu::submenuFor(int index) const {
    if (index < 0 || index >= submenus_.count()) return 0;
    return submenus_[index];
}

QAction* QMenu::actionAt(const QPoint& pos) const {
    int index = indexAt(pos);
    if (index < 0 || index >= actions_.count()) return 0;
    if (actions_[index]->isSeparator()) return 0;
    return actions_[index];
}

int QMenu::actionGeometry(QAction* action) const {
    return actions_.indexOf(action);
}

void QMenu::setActiveAction(QAction* action) {
    int index = actions_.indexOf(action);
    if (index < 0 || index == activeIndex_) return;
    activeIndex_ = index;
    active_ = action;
    update();
    if (index >= 0) hoveredAction(action);
}

void QMenu::popup(const QPoint& globalPos) {
    updateGeometryForActions();
    aboutToShow();
    setGeometry(QRect(globalPos, size()));
    show();
    raise();
    setFocus(Qt::PopupFocusReason);
}

void QMenu::exec(const QPoint& globalPos) { popup(globalPos); }

bool QMenu::isVisible() const { return QWidget::isVisible(); }

QSize QMenu::sizeHint() const { return size(); }

void QMenu::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.menuPopup);
    p.setPen(QPen(c.borderStrong, 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
    QFont f = font();
    p.setFont(f);
    for (int i = 0; i < actions_.count(); ++i) {
        QStyleOptionMenuItem opt;
        opt.rect = actionRect(i);
        opt.text = actions_[i]->text();
        opt.shortcut = actions_[i]->shortcut();
        opt.separator = actions_[i]->isSeparator();
        opt.enabled = actions_[i]->isEnabled();
        opt.checked = actions_[i]->isChecked();
        opt.checkable = actions_[i]->isCheckable();
        opt.active = (i == activeIndex_);
        opt.subMenu = submenuFor(i) != 0;
        opt.iconVisible = !actions_[i]->icon().isNull();
        style()->drawMenuItem(&p, opt, this);
    }
}

void QMenu::mouseMoveEvent(QMouseEvent* event) {
    int index = indexAt(event->pos());
    if (index != activeIndex_) {
        activeIndex_ = index;
        active_ = (index >= 0) ? actions_[index] : 0;
        update();
        if (index >= 0) {
            hoveredAction(active_);
            QMenu* submenu = submenuFor(index);
            if (submenu) {
                QRect r = actionRect(index);
                QPoint global = mapToGlobal(QPoint(r.right(), r.top() - 4));
                if (openSubmenu_ && openSubmenu_ != submenu) openSubmenu_->hide();
                openSubmenu_ = submenu;
                submenu->popup(global);
            } else if (openSubmenu_) {
                openSubmenu_->hide();
                openSubmenu_ = 0;
            }
        }
    }
}

void QMenu::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int index = indexAt(event->pos());
        if (index >= 0) {
            mouseReleaseEvent(event);
            return;
        }
    }
    hide();
}

void QMenu::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    int index = indexAt(event->pos());
    if (index < 0) { hide(); return; }
    QAction* action = actions_[index];
    if (action->isSeparator()) return;
    QMenu* submenu = submenuFor(index);
    if (submenu) {
        QRect r = actionRect(index);
        openSubmenu_ = submenu;
        submenu->popup(mapToGlobal(QPoint(r.right(), r.top() - 4)));
        return;
    }
    hide();
    if (action->isEnabled()) {
        if (action->isCheckable()) action->setChecked(!action->isChecked());
        triggeredAction(action);
        action->triggered(action->isChecked());
    }
}

void QMenu::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        hide();
        if (parentMenu_) parentMenu_->setFocus(Qt::PopupFocusReason);
        return;
    }
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) {
        int step = event->key() == Qt::Key_Down ? 1 : -1;
        int index = activeIndex_;
        for (int i = 0; i < actions_.count(); ++i) {
            index += step;
            if (index < 0) index = actions_.count() - 1;
            if (index >= actions_.count()) index = 0;
            if (!actions_[index]->isSeparator() && actions_[index]->isEnabled()) break;
        }
        setActiveAction(index >= 0 ? actions_[index] : 0);
        return;
    }
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && activeIndex_ >= 0) {
        QAction* action = actions_[activeIndex_];
        if (submenuFor(activeIndex_)) {
            QRect r = actionRect(activeIndex_);
            openSubmenu_ = submenuFor(activeIndex_);
            openSubmenu_->popup(mapToGlobal(QPoint(r.right(), r.top() - 4)));
            return;
        }
        hide();
        if (action->isCheckable()) action->setChecked(!action->isChecked());
        triggeredAction(action);
        action->triggered(action->isChecked());
        return;
    }
    QWidget::keyPressEvent(event);
}

void QMenu::enterEvent(QEvent* event) { update(); QWidget::enterEvent(event); }
void QMenu::leaveEvent(QEvent* event) {
    activeIndex_ = -1;
    active_ = 0;
    update();
    QWidget::leaveEvent(event);
}

// ===========================================================================
// QMenuBar
// ===========================================================================
const QMetaObject QMenuBar::staticMetaObject = {
    &QWidget::staticMetaObject, "QMenuBar", 0, 0, 0, 0
};

QVariant QMenuBar::qtcReadProperty(int id) const { return QWidget::qtcReadProperty(id); }
bool QMenuBar::qtcWriteProperty(int id, const QVariant& value) { return QWidget::qtcWriteProperty(id, value); }

QMenuBar::QMenuBar(QWidget* parent) : QWidget(parent), activeIndex_(-1), hoverIndex_(-1) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumSize(0, 28);
}

QMenuBar::~QMenuBar() {}

QAction* QMenuBar::addMenu(QMenu* menu) {
    if (!menu) return 0;
    QAction* action = new QAction(menu->title(), this);
    actions_.append(action);
    menus_.append(menu);
    updateGeometry();
    update();
    return action;
}

QMenu* QMenuBar::addMenu(const QString& title) {
    QMenu* menu = new QMenu(title, this);
    addMenu(menu);
    return menu;
}

void QMenuBar::addAction(QAction* action) {
    if (!action) return;
    actions_.append(action);
    menus_.append(0);
    update();
}

void QMenuBar::removeAction(QAction* action) {
    int index = actions_.indexOf(action);
    if (index < 0) return;
    actions_.removeAt(index);
    menus_.removeAt(index);
    update();
}

QRect QMenuBar::itemRect(int index) const {
    QFontMetrics fm(font());
    int x = 10;
    for (int i = 0; i < index; ++i) x += fm.width(actions_[i]->text()) + 22;
    return QRect(x, 0, fm.width(actions_[index]->text()) + 22, height());
}

int QMenuBar::itemAt(const QPoint& pos) const {
    for (int i = 0; i < actions_.count(); ++i)
        if (itemRect(i).contains(pos)) return i;
    return -1;
}

QSize QMenuBar::sizeHint() const {
    QFontMetrics fm(font());
    int w = 10;
    for (int i = 0; i < actions_.count(); ++i) w += fm.width(actions_[i]->text()) + 22;
    return QSize(w, 28);
}

int QMenuBar::heightForWidth(int) const { return 28; }

void QMenuBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.menuBar);
    p.setPen(QPen(c.border, 1));
    p.drawLine(0, height() - 1, width(), height() - 1);
    p.setFont(font());
    for (int i = 0; i < actions_.count(); ++i) {
        QRect r = itemRect(i);
        if (i == hoverIndex_ || i == activeIndex_) {
            QColor bg = (i == activeIndex_) ? c.surfaceAlt : c.surfaceAlt;
            p.fillRect(r.adjusted(1, 2, -1, -1), bg);
        }
        p.setPen(QPen(c.windowText));
        p.drawText(r, Qt::AlignCenter, actions_[i]->text());
    }
}

void QMenuBar::mousePressEvent(QMouseEvent* event) {
    int index = itemAt(event->pos());
    if (index >= 0 && index < menus_.count() && menus_[index]) {
        activeIndex_ = index;
        update();
        QMenu* menu = menus_[index];
        QRect r = itemRect(index);
        menu->popup(mapToGlobal(QPoint(r.left(), height())));
    }
}

void QMenuBar::mouseMoveEvent(QMouseEvent* event) {
    int index = itemAt(event->pos());
    if (index != hoverIndex_) {
        hoverIndex_ = index;
        update();
    }
}

void QMenuBar::mouseReleaseEvent(QMouseEvent*) {}
void QMenuBar::enterEvent(QEvent* event) { QWidget::enterEvent(event); }
void QMenuBar::leaveEvent(QEvent* event) {
    hoverIndex_ = -1;
    update();
    QWidget::leaveEvent(event);
}

// ===========================================================================
// QToolBar
// ===========================================================================
const QMetaObject QToolBar::staticMetaObject = {
    &QWidget::staticMetaObject, "QToolBar", 0, 0, 0, 0
};

QVariant QToolBar::qtcReadProperty(int id) const { return QWidget::qtcReadProperty(id); }
bool QToolBar::qtcWriteProperty(int id, const QVariant& value) { return QWidget::qtcWriteProperty(id, value); }

QToolBar::QToolBar(QWidget* parent)
    : QWidget(parent), toolStyle_(Qt::ToolButtonTextBesideIcon), movable_(true),
      iconSize_(18), hoverIndex_(-1), pressedIndex_(-1) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumSize(0, 32);
}

QToolBar::QToolBar(const QString& title, QWidget* parent)
    : QWidget(parent), toolStyle_(Qt::ToolButtonTextBesideIcon), movable_(true),
      iconSize_(18), hoverIndex_(-1), pressedIndex_(-1) {
    title_ = title;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumSize(0, 32);
}

QToolBar::~QToolBar() {}

QAction* QToolBar::addAction(const QString& text) {
    QAction* action = new QAction(text, this);
    addAction(action);
    return action;
}

QAction* QToolBar::addAction(const QIcon& icon, const QString& text) {
    QAction* action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

void QToolBar::addAction(QAction* action) {
    if (!action) return;
    actions_.append(action);
    relayout();
}

void QToolBar::addSeparator() {
    QAction* action = new QAction(this);
    action->setSeparator(true);
    actions_.append(action);
    relayout();
}

void QToolBar::addWidget(QWidget* widget) {
    if (!widget) return;
    widget->setParentWidget(this);
    widgets_.append(widget);
    widget->show();
    relayout();
}

void QToolBar::clear() {
    actions_.clearAll();
    widgets_.clearAll();
    relayout();
}

void QToolBar::relayout() {
    QFontMetrics fm(font());
    int x = 6;
    actionRects_.clearAll();
    for (int i = 0; i < actions_.count(); ++i) {
        if (actions_[i]->isSeparator()) {
            actionRects_.append(QRect(x, 4, 6, height() - 8));
            x += 10;
            continue;
        }
        int w = fm.width(actions_[i]->text()) + 18;
        if (!actions_[i]->icon().isNull()) w += iconSize_ + 6;
        actionRects_.append(QRect(x, 3, w, height() - 6));
        x += w + 2;
    }
    updateGeometry();
    update();
}

QAction* QToolBar::actionAt(const QPoint& pos) const {
    for (int i = 0; i < actionRects_.count(); ++i)
        if (actionRects_[i].contains(pos) && i < actions_.count() && !actions_[i]->isSeparator())
            return actions_[i];
    return 0;
}

QRect QToolBar::actionRect(QAction* action) const {
    int index = actions_.indexOf(action);
    if (index < 0 || index >= actionRects_.count()) return QRect();
    return actionRects_[index];
}

QSize QToolBar::sizeHint() const {
    int x = 6;
    QFontMetrics fm(font());
    for (int i = 0; i < actions_.count(); ++i) {
        if (actions_[i]->isSeparator()) { x += 10; continue; }
        x += fm.width(actions_[i]->text()) + 20;
        if (!actions_[i]->icon().isNull()) x += iconSize_ + 6;
    }
    return QSize(x + 6, 32);
}

void QToolBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.surface);
    p.setPen(QPen(c.border, 1));
    p.drawLine(0, height() - 1, width(), height() - 1);
    p.setFont(font());
    for (int i = 0; i < actions_.count(); ++i) {
        if (actions_[i]->isSeparator()) {
            if (i < actionRects_.count()) {
                QRect r = actionRects_[i];
                p.setPen(QPen(c.border, 1));
                p.drawLine(r.center().x(), r.top() + 2, r.center().x(), r.bottom() - 2);
            }
            continue;
        }
        if (i >= actionRects_.count()) break;
        QRect r = actionRects_[i];
        if (i == hoverIndex_ || i == pressedIndex_) {
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(i == pressedIndex_ ? c.buttonPressed : c.buttonHover));
            p.drawRoundedRect(r, 4, 4);
        }
        int x = r.left() + 6;
        if (!actions_[i]->icon().isNull()) {
            QPixmap pm = actions_[i]->icon().pixmap(QSize(iconSize_, iconSize_));
            p.drawPixmap(QPoint(x, r.center().y() - iconSize_ / 2), pm);
            x += iconSize_ + 5;
        }
        p.setPen(QPen(actions_[i]->isEnabled() ? c.windowText : c.disabledText));
        p.drawText(QRect(x, r.top(), r.right() - x - 2, r.height()), Qt::AlignLeft | Qt::AlignVCenter, actions_[i]->text());
    }
    for (int i = 0; i < widgets_.count(); ++i) {
        // widgets position themselves through addWidget callers
    }
}

void QToolBar::mousePressEvent(QMouseEvent* event) {
    int index = -1;
    for (int i = 0; i < actionRects_.count(); ++i) {
        if (actionRects_[i].contains(event->pos())) { index = i; break; }
    }
    if (index >= 0 && index < actions_.count() && !actions_[index]->isSeparator()) {
        pressedIndex_ = index;
        update();
    }
}

void QToolBar::mouseMoveEvent(QMouseEvent* event) {
    int index = -1;
    for (int i = 0; i < actionRects_.count(); ++i) {
        if (actionRects_[i].contains(event->pos())) { index = i; break; }
    }
    if (index != hoverIndex_) {
        hoverIndex_ = index;
        update();
    }
}

void QToolBar::mouseReleaseEvent(QMouseEvent* event) {
    if (pressedIndex_ >= 0 && pressedIndex_ < actions_.count()) {
        QAction* action = actions_[pressedIndex_];
        int index = pressedIndex_;
        pressedIndex_ = -1;
        update();
        if (index >= 0 && index < actionRects_.count() && actionRects_[index].contains(event->pos()) &&
            action->isEnabled()) {
            if (action->isCheckable()) action->setChecked(!action->isChecked());
            action->triggered(action->isChecked());
        }
    }
}

void QToolBar::resizeEvent(QResizeEvent* event) {
    relayout();
    QWidget::resizeEvent(event);
}

// ===========================================================================
// QStatusBar
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QStatusBar[] = {
    { "currentMessage", "QString", QMetaObject::Readable | QMetaObject::Designable, 0 }
};
static const QMetaObject::SignalInfo qtc_signals_QStatusBar[] = {
    { "messageChanged", "messageChanged(QString)" }
};
const QMetaObject QStatusBar::staticMetaObject = {
    &QWidget::staticMetaObject, "QStatusBar",
    qtc_props_QStatusBar, 1, qtc_signals_QStatusBar, 1
};

QVariant QStatusBar::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return QVariant(message_);
    default: break;
    }
    return QVariant();
}

bool QStatusBar::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: showMessage(value.toString()); return true;
    default: break;
    }
    return false;
}

QStatusBar::QStatusBar(QWidget* parent) : QWidget(parent), messageTimer_(0) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumSize(0, 24);
}

QStatusBar::~QStatusBar() {}

void QStatusBar::showMessage(const QString& text, int timeout) {
    message_ = text;
    update();
    messageChanged(message_);
    if (messageTimer_) {
        killTimer(messageTimer_);
        messageTimer_ = 0;
    }
    if (timeout > 0) messageTimer_ = startTimer(timeout);
}

void QStatusBar::clearMessage() { showMessage(QString(), 0); }

void QStatusBar::addWidget(QWidget* widget) {
    if (!widget) return;
    widget->setParentWidget(this);
    normal_.append(widget);
    widget->show();
    update();
}

void QStatusBar::addPermanentWidget(QWidget* widget) {
    if (!widget) return;
    widget->setParentWidget(this);
    permanent_.append(widget);
    widget->show();
    update();
}

void QStatusBar::removeWidget(QWidget* widget) {
    permanent_.removeAll(widget);
    normal_.removeAll(widget);
    widget->setParentWidget(0);
    update();
}

QSize QStatusBar::sizeHint() const { return QSize(200, 24); }

void QStatusBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const QTheme::Colors& c = QTheme::colors();
    p.fillRect(rect(), c.surface);
    p.setPen(QPen(c.border, 1));
    p.drawLine(0, 0, width(), 0);
    p.setPen(QPen(c.windowText));
    p.setFont(font());
    p.drawText(rect().adjusted(8, 0, -8, 0), Qt::AlignLeft | Qt::AlignVCenter, message_);
}

void QStatusBar::resizeEvent(QResizeEvent* event) {
    int x = width() - 8;
    for (int i = permanent_.count() - 1; i >= 0; --i) {
        QSize s = permanent_[i]->sizeHint();
        if (s.width() <= 0) s = permanent_[i]->size();
        x -= s.width();
        permanent_[i]->setGeometry(x, 2, s.width(), height() - 4);
        x -= 8;
    }
    QWidget::resizeEvent(event);
}

void QStatusBar::timerEvent(QTimerEvent* event) {
    if (event->timerId() == messageTimer_) {
        killTimer(messageTimer_);
        messageTimer_ = 0;
        clearMessage();
        return;
    }
    QWidget::timerEvent(event);
}

// ===========================================================================
// QMainWindow
// ===========================================================================
static const QMetaObject::PropertyData qtc_props_QMainWindow[] = {
    { "centralWidget", "QWidget", QMetaObject::Readable | QMetaObject::Designable, 0 }
};
const QMetaObject QMainWindow::staticMetaObject = {
    &QWidget::staticMetaObject, "QMainWindow", qtc_props_QMainWindow, 1, 0, 0
};

QVariant QMainWindow::qtcReadProperty(int id) const {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcReadProperty(id);
    switch (id - base) {
    case 0: return central_ ? QVariant(central_->objectName()) : QVariant();
    default: break;
    }
    return QVariant();
}

bool QMainWindow::qtcWriteProperty(int id, const QVariant& value) {
    const int base = QWidget::staticMetaObject.propertyCount();
    if (id < base) return QWidget::qtcWriteProperty(id, value);
    switch (id - base) {
    case 0: if (central_) central_->setObjectName(value.toString()); return true;
    default: break;
    }
    return false;
}

QMainWindow::QMainWindow(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), central_(0), menuBar_(0), statusBar_(0), toolbarBreak_(false) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QMainWindow::~QMainWindow() {}

void QMainWindow::setCentralWidget(QWidget* widget) {
    if (central_ == widget) return;
    if (central_) {
        central_->hide();
        central_->setParentWidget(0);
    }
    central_ = widget;
    if (central_) {
        central_->setParentWidget(this);
        central_->show();
    }
    layoutMain();
    update();
}

void QMainWindow::setMenuBar(QMenuBar* menuBar) {
    if (menuBar_ == menuBar) return;
    if (menuBar_) {
        menuBar_->hide();
        menuBar_->setParentWidget(0);
    }
    menuBar_ = menuBar;
    if (menuBar_) {
        menuBar_->setParentWidget(this);
        menuBar_->show();
        menuBar_->raise();
    }
    layoutMain();
}

QMenuBar* QMainWindow::qtcCreateMenuBar() {
    if (!menuBar_) setMenuBar(new QMenuBar(this));
    return menuBar_;
}

void QMainWindow::setStatusBar(QStatusBar* statusBar) {
    if (statusBar_ == statusBar) return;
    if (statusBar_) {
        statusBar_->hide();
        statusBar_->setParentWidget(0);
    }
    statusBar_ = statusBar;
    if (statusBar_) {
        statusBar_->setParentWidget(this);
        statusBar_->show();
        statusBar_->raise();
    }
    layoutMain();
}

QStatusBar* QMainWindow::qtcCreateStatusBar() {
    if (!statusBar_) setStatusBar(new QStatusBar(this));
    return statusBar_;
}

void QMainWindow::addToolBar(QToolBar* toolbar) {
    if (!toolbar) return;
    toolbar->setParentWidget(this);
    toolBars_.append(toolbar);
    toolbar->show();
    toolbar->raise();
    layoutMain();
}

void QMainWindow::addToolBarBreak() { toolbarBreak_ = true; }

void QMainWindow::insertToolBar(QToolBar* before, QToolBar* toolbar) {
    Q_UNUSED(before);
    addToolBar(toolbar);
}

void QMainWindow::removeToolBar(QToolBar* toolbar) {
    toolBars_.removeAll(toolbar);
    toolbar->hide();
    toolbar->setParentWidget(0);
    layoutMain();
}

void QMainWindow::addDockWidgetArea(QWidget* widget, int) { Q_UNUSED(widget); }

void QMainWindow::layoutMain() {
    int top = 0;
    if (menuBar_ && menuBar_->isVisible()) {
        int h = menuBar_->sizeHint().height();
        menuBar_->setGeometry(0, 0, width(), h);
        top += h;
    }
    for (int i = 0; i < toolBars_.count(); ++i) {
        if (!toolBars_[i]->isVisible()) continue;
        int h = toolBars_[i]->sizeHint().height();
        toolBars_[i]->setGeometry(0, top, width(), h);
        top += h;
    }
    int bottom = height();
    if (statusBar_ && statusBar_->isVisible()) {
        int h = statusBar_->sizeHint().height();
        statusBar_->setGeometry(0, height() - h, width(), h);
        bottom -= h;
    }
    if (central_) central_->setGeometry(0, top, width(), qMax(0, bottom - top));
}

void QMainWindow::resizeEvent(QResizeEvent* event) {
    layoutMain();
    QWidget::resizeEvent(event);
}

void QMainWindow::showEvent(QShowEvent* event) {
    layoutMain();
    QWidget::showEvent(event);
}

void QMainWindow::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QTheme::colors().window);
}

QSize QMainWindow::sizeHint() const { return QSize(900, 620); }

QTC_REGISTER_WIDGET(QDialog, "Containers", "Dialog", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QMessageBox, "Dialogs", "Message Box", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QMenu, "Menus", "Menu", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QMenuBar, "Menus", "Menu Bar", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QToolBar, "Menus", "Tool Bar", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QStatusBar, "Menus", "Status Bar", "<qtc/QtWidgets.h>")
QTC_REGISTER_WIDGET(QMainWindow, "Containers", "Main Window", "<qtc/QtWidgets.h>")

// Keeps this translation unit linked when the framework is a static library.
void qtcWidgetAnchor_menus() {}
