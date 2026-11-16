// QtClone demo: a WeChat PC style client built with the QtClone framework.
//
// It is a UI mock-up (no network, no real accounts): sidebar, conversation
// list, message bubbles, composer, search filter and light/dark themes.
#include <qtc/QtWidgets.h>
#include <qtc/QStyle.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <windows.h>

// ===========================================================================
// palette
// ===========================================================================
struct WeChatColors {
    QColor window;
    QColor sidebar;
    QColor sidebarIcon;
    QColor sidebarIconActive;
    QColor sidebarHover;
    QColor listBg;
    QColor listSelected;
    QColor listHover;
    QColor header;
    QColor chatBg;
    QColor bubbleMine;
    QColor bubbleOther;
    QColor bubbleMineText;
    QColor bubbleOtherText;
    QColor text;
    QColor subText;
    QColor border;
    QColor accent;
    QColor accentHover;
    QColor inputBg;
    QColor badge;
    QColor searchBg;
};

static WeChatColors weChatColors() {
    WeChatColors c;
    if (QTheme::isDark()) {
        c.window = QColor(0x19, 0x19, 0x19);
        c.sidebar = QColor(0x11, 0x11, 0x11);
        c.sidebarIcon = QColor(0x8A, 0x8A, 0x8A);
        c.sidebarIconActive = QColor(0x07, 0xC1, 0x60);
        c.sidebarHover = QColor(0x25, 0x25, 0x25);
        c.listBg = QColor(0x1A, 0x1A, 0x1A);
        c.listSelected = QColor(0x2A, 0x2A, 0x2A);
        c.listHover = QColor(0x22, 0x22, 0x22);
        c.header = QColor(0x19, 0x19, 0x19);
        c.chatBg = QColor(0x19, 0x19, 0x19);
        c.bubbleMine = QColor(0x3E, 0x9E, 0x5A);
        c.bubbleOther = QColor(0x2C, 0x2C, 0x2C);
        c.bubbleMineText = QColor(0xEA, 0xEA, 0xEA);
        c.bubbleOtherText = QColor(0xD8, 0xD8, 0xD8);
        c.text = QColor(0xD8, 0xD8, 0xD8);
        c.subText = QColor(0x7F, 0x7F, 0x7F);
        c.border = QColor(0x2A, 0x2A, 0x2A);
        c.accent = QColor(0x07, 0xC1, 0x60);
        c.accentHover = QColor(0x06, 0xAD, 0x56);
        c.inputBg = QColor(0x23, 0x23, 0x23);
        c.badge = QColor(0xFA, 0x51, 0x51);
        c.searchBg = QColor(0x2A, 0x2A, 0x2A);
    } else {
        c.window = QColor(0xF5, 0xF5, 0xF5);
        c.sidebar = QColor(0x2E, 0x2E, 0x2E);
        c.sidebarIcon = QColor(0xB0, 0xB0, 0xB0);
        c.sidebarIconActive = QColor(0x07, 0xC1, 0x60);
        c.sidebarHover = QColor(0x3A, 0x3A, 0x3A);
        c.listBg = QColor(0xED, 0xED, 0xED);
        c.listSelected = QColor(0xDE, 0xDE, 0xDE);
        c.listHover = QColor(0xE5, 0xE5, 0xE5);
        c.header = QColor(0xF5, 0xF5, 0xF5);
        c.chatBg = QColor(0xF5, 0xF5, 0xF5);
        c.bubbleMine = QColor(0x95, 0xEC, 0x69);
        c.bubbleOther = QColor(0xFF, 0xFF, 0xFF);
        c.bubbleMineText = QColor(0x19, 0x19, 0x19);
        c.bubbleOtherText = QColor(0x19, 0x19, 0x19);
        c.text = QColor(0x19, 0x19, 0x19);
        c.subText = QColor(0x99, 0x99, 0x99);
        c.border = QColor(0xE7, 0xE7, 0xE7);
        c.accent = QColor(0x07, 0xC1, 0x60);
        c.accentHover = QColor(0x06, 0xAD, 0x56);
        c.inputBg = QColor(0xFF, 0xFF, 0xFF);
        c.badge = QColor(0xFA, 0x51, 0x51);
        c.searchBg = QColor(0xFF, 0xFF, 0xFF);
    }
    return c;
}

static QFont uiFont(int pixels, bool bold = false) {
    QFont f(L"Microsoft YaHei UI", 9);
    f.setPixelSize(pixels);
    if (bold) f.setBold(true);
    return f;
}

// ===========================================================================
// data
// ===========================================================================
struct ChatMessage {
    bool mine;
    QString sender;
    QString text;
    QString time;
    int kind; // 0 text, 1 system, 2 image stub
};

struct ChatItem {
    QString name;
    QString preview;
    QString time;
    int unread;
    QColor avatarColor;
    QString avatarText;
    bool group;
    int members;
    QList<ChatMessage> messages;
};

static ChatMessage msg(bool mine, const wchar_t* sender, const wchar_t* text,
                       const wchar_t* time, int kind = 0) {
    ChatMessage m;
    m.mine = mine;
    m.sender = QString(sender);
    m.text = QString(text);
    m.time = QString(time);
    m.kind = kind;
    return m;
}

static void buildChats(QList<ChatItem>* chats) {
    ChatItem c;

    c.name = L"文件传输助手";
    c.preview = L"[文件] QtClone 设计说明.pdf";
    c.time = L"昨天";
    c.unread = 0;
    c.avatarColor = QColor(0x5B, 0x8D, 0xEF);
    c.avatarText = L"文件";
    c.group = false;
    c.members = 0;
    c.messages << msg(false, L"文件传输助手", L"QtClone 设计说明.pdf", L"昨天 18:20")
               << msg(true, L"我", L"QtClone 设计说明.pdf", L"昨天 18:21", 2);
    chats->append(c);

    c = ChatItem();
    c.name = L"张三";
    c.preview = L"好的，明天见！";
    c.time = L"10:24";
    c.unread = 2;
    c.avatarColor = QColor(0xF2, 0x9D, 0x38);
    c.avatarText = L"张";
    c.group = false;
    c.messages
        << msg(false, L"张三", L"在吗？", L"10:18")
        << msg(true, L"我", L"在的，怎么了", L"10:19")
        << msg(false, L"张三", L"那个 QtClone 框架，能在 VS2010 上跑吗？", L"10:20")
        << msg(true, L"我",
              L"可以，已经用 v100 编译通过了。还带设计器和 .ui 代码生成，没有 moc。", L"10:21")
        << msg(false, L"张三", L"牛啊，回头给我发一份", L"10:22")
        << msg(true, L"我", L"没问题，GitHub 上有，地址回头发你", L"10:23")
        << msg(false, L"张三", L"好的，明天见！", L"10:24");
    chats->append(c);

    c = ChatItem();
    c.name = L"产品讨论组";
    c.preview = L"李四：新版设计稿已上传";
    c.time = L"09:41";
    c.unread = 5;
    c.avatarColor = QColor(0x6E, 0xC7, 0x8E);
    c.avatarText = L"产品";
    c.group = true;
    c.members = 12;
    c.messages
        << msg(false, L"王五", L"早，今天过一下 3.0 的交互稿？", L"09:30")
        << msg(false, L"李四", L"新版设计稿已上传，大家看一下：主要改了会话列表和输入框。", L"09:41")
        << msg(true, L"我", L"收到，我下午给反馈", L"09:45")
        << msg(false, L"王五", L"@我 顺便看下深色模式下的气泡颜色", L"09:47");
    chats->append(c);

    c = ChatItem();
    c.name = L"微信团队";
    c.preview = L"登录安全提醒";
    c.time = L"昨天";
    c.unread = 0;
    c.avatarColor = QColor(0x4C, 0xAF, 0x50);
    c.avatarText = L"微信";
    c.group = false;
    c.messages
        << msg(false, L"微信团队", L"你的账号于 10 月 12 日在 Windows 设备登录。", L"昨天 09:02", 1)
        << msg(false, L"微信团队", L"如非本人操作，请及时修改密码。", L"昨天 09:02");
    chats->append(c);

    c = ChatItem();
    c.name = L"妈妈";
    c.preview = L"记得吃饭";
    c.time = L"昨天";
    c.unread = 0;
    c.avatarColor = QColor(0xE0, 0x7A, 0x9A);
    c.avatarText = L"妈";
    c.messages
        << msg(false, L"妈妈", L"今天降温，多穿点", L"昨天 07:30")
        << msg(true, L"我", L"知道啦", L"昨天 08:10")
        << msg(false, L"妈妈", L"记得吃饭", L"昨天 12:01");
    chats->append(c);

    c = ChatItem();
    c.name = L"前端技术群";
    c.preview = L"王五：这个 bug 我看看";
    c.time = L"周一";
    c.unread = 0;
    c.avatarColor = QColor(0x8A, 0x7D, 0xF2);
    c.avatarText = L"前端";
    c.group = true;
    c.members = 86;
    c.messages
        << msg(false, L"赵六", L"有人遇到过 GDI+ 字体测量闪烁吗？", L"周一 14:02")
        << msg(true, L"我", L"试试缓存 QFontMetrics", L"周一 14:05")
        << msg(false, L"王五", L"这个 bug 我看看", L"周一 14:20");
    chats->append(c);

    c = ChatItem();
    c.name = L"李四";
    c.preview = L"收到";
    c.time = L"周一";
    c.unread = 0;
    c.avatarColor = QColor(0x49, 0xB0, 0xC9);
    c.avatarText = L"李";
    c.messages
        << msg(false, L"李四", L"接口文档更新了", L"周一 10:00")
        << msg(true, L"我", L"收到", L"周一 10:02");
    chats->append(c);
}

// ===========================================================================
// helpers
// ===========================================================================
static QStringList wrapText(const QFontMetrics& fm, const QString& text, int maxWidth) {
    QStringList lines;
    QString current;
    for (int i = 0; i < text.size(); ++i) {
        wchar_t ch = text[i];
        if (ch == L'\n') {
            lines.append(current);
            current.clear();
            continue;
        }
        QString next = current + QString(ch);
        if (!current.isEmpty() && fm.width(next) > maxWidth) {
            int space = (int)current.find_last_of(L' ');
            if (space > 0) {
                lines.append(current.left(space));
                current = current.mid(space + 1) + QString(ch);
            } else {
                lines.append(current);
                current = QString(ch);
            }
        } else {
            current = next;
        }
    }
    if (!current.isEmpty() || lines.isEmpty()) lines.append(current);
    return lines;
}

static void drawAvatar(QPainter& p, const QRect& rect, const QColor& color, const QString& text) {
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(color));
    p.drawEllipse(rect);
    QFont f = uiFont(qMax(11, rect.width() / 3), true);
    p.setFont(f);
    p.setPen(QPen(QColor(0xFF, 0xFF, 0xFF)));
    p.drawText(rect, Qt::AlignCenter, text.left(2));
    p.setBrush(Qt::NoBrush);
}

// ===========================================================================
// WeChatSidebar
// ===========================================================================
class WeChatSidebar : public QWidget {
public:
    explicit WeChatSidebar(QWidget* parent = 0)
        : QWidget(parent), current_(0), hover_(-1) {
        setFixedWidth(62);
        setMouseTracking(true);
    }

    Signal<void(int)> sectionChanged;

    static int sectionCount() { return 6; }

protected:
    QRect iconRect(int index) const {
        int top = 82;
        int size = 44;
        int gap = 10;
        if (index >= 5) return QRect(9, height() - 78, size, size);
        return QRect(9, top + index * (size + gap), size, size);
    }

    void paintEvent(QPaintEvent*) {
        WeChatColors c = weChatColors();
        QPainter p(this);
        p.fillRect(rect(), c.sidebar);

        // user avatar
        QRect avatar(11, 18, 40, 40);
        drawAvatar(p, avatar, QColor(0x07, 0xC1, 0x60), L"我");
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(0x2E, 0x2E, 0x2E)));
        p.drawEllipse(QRect(43, 50, 12, 12));
        p.setBrush(QBrush(QColor(0x2A, 0xC7, 0x5B)));
        p.drawEllipse(QRect(45, 52, 8, 8));

        for (int i = 0; i < sectionCount(); ++i) {
            QRect r = iconRect(i);
            bool active = (i == current_);
            if (i == hover_)
                p.fillRect(r.adjusted(-4, -4, 4, 4), c.sidebarHover);
            QColor color = active ? c.sidebarIconActive : c.sidebarIcon;
            if (active) {
                p.fillRect(QRect(0, r.top() + 8, 3, r.height() - 16), c.sidebarIconActive);
            }
            drawIcon(p, i, r, color);
            if (i == 0 && active) {
                // unread dot on the chat icon
                p.setPen(Qt::NoPen);
                p.setBrush(QBrush(c.badge));
                p.drawEllipse(QRect(r.right() - 6, r.top() + 2, 9, 9));
                p.setBrush(Qt::NoBrush);
            }
        }
    }

    void drawIcon(QPainter& p, int index, const QRect& r, const QColor& color) {
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen(color, 2));
        p.setBrush(Qt::NoBrush);
        int cx = r.center().x();
        int cy = r.center().y();
        if (index == 0) {
            // chat bubble with a tail
            QRect bubble(r.left() + 8, r.top() + 12, r.width() - 16, r.height() - 24);
            p.drawRoundedRect(bubble, 5, 5);
            QList<QPoint> tail;
            tail << QPoint(bubble.left() + 8, bubble.bottom())
                 << QPoint(bubble.left() + 14, bubble.bottom() + 7)
                 << QPoint(bubble.left() + 18, bubble.bottom());
            p.setBrush(QBrush(color));
            p.drawPolygon(tail);
            p.setBrush(Qt::NoBrush);
        } else if (index == 1) {
            p.drawEllipse(QRect(cx - 6, cy - 15, 12, 12));
            p.drawRoundedRect(QRect(cx - 12, cy + 1, 24, 12), 6, 6);
        } else if (index == 2) {
            QList<QPoint> star;
            for (int i = 0; i < 10; ++i) {
                double a = -1.5708 + i * 0.6283;
                int rr = (i % 2 == 0) ? 14 : 6;
                star.append(QPoint(cx + (int)(rr * cos(a)), cy + (int)(rr * sin(a))));
            }
            p.setBrush(QBrush(color));
            p.drawPolygon(star);
            p.setBrush(Qt::NoBrush);
        } else if (index == 3) {
            QRect folder(r.left() + 7, r.top() + 13, r.width() - 14, r.height() - 26);
            p.drawRoundedRect(folder, 3, 3);
            p.drawLine(folder.left() + 2, folder.top() + 5, folder.left() + 14, folder.top() + 5);
        } else if (index == 4) {
            p.drawEllipse(QRect(cx - 15, cy - 13, 30, 30));
            p.setBrush(QBrush(color));
            p.drawEllipse(QRect(cx - 2, cy - 1, 5, 5));
            p.drawEllipse(QRect(cx + 5, cy - 1, 5, 5));
            p.setBrush(Qt::NoBrush);
        } else {
            p.drawEllipse(QRect(cx - 13, cy - 13, 26, 26));
            p.drawEllipse(QRect(cx - 3, cy - 3, 6, 6));
            for (int i = 0; i < 8; ++i) {
                double a = i * 0.7854;
                int x1 = cx + (int)(17 * cos(a));
                int y1 = cy + (int)(17 * sin(a));
                int x2 = cx + (int)(21 * cos(a));
                int y2 = cy + (int)(21 * sin(a));
                p.drawLine(x1, y1, x2, y2);
            }
        }
    }

    void mousePressEvent(QMouseEvent* event) {
        for (int i = 0; i < sectionCount(); ++i) {
            if (iconRect(i).adjusted(-6, -6, 6, 6).contains(event->pos())) {
                current_ = i;
                update();
                sectionChanged(i);
                return;
            }
        }
    }

    void mouseMoveEvent(QMouseEvent* event) {
        int hover = -1;
        for (int i = 0; i < sectionCount(); ++i)
            if (iconRect(i).adjusted(-6, -6, 6, 6).contains(event->pos())) hover = i;
        if (hover != hover_) {
            hover_ = hover;
            update();
        }
    }

    void leaveEvent(QEvent*) {
        hover_ = -1;
        update();
    }

private:
    int current_;
    int hover_;
};

// ===========================================================================
// ChatListWidget
// ===========================================================================
class ChatListWidget : public QWidget {
public:
    explicit ChatListWidget(QWidget* parent = 0)
        : QWidget(parent), chats_(0), current_(-1), hover_(-1), scroll_(0), vbar_(0), filter_() {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMouseTracking(true);
        vbar_ = new QScrollBar(Qt::Vertical, this);
        vbar_->hide();
        vbar_->valueChanged.connect(vbar_, this, &ChatListWidget::onScroll);
    }

    void setChats(QList<ChatItem>* chats) {
        chats_ = chats;
        rebuildFilter();
    }

    void setFilter(const QString& text) {
        filter_ = text.toLower();
        rebuildFilter();
    }

    void setCurrent(int index) {
        current_ = index;
        update();
    }

    int current() const { return current_; }
    int visibleCount() const { return visible_.count(); }

    Signal<void(int)> currentChanged;

protected:
    int rowHeight() const { return 72; }

    void rebuildFilter() {
        visible_.clearAll();
        if (!chats_) return;
        for (int i = 0; i < chats_->count(); ++i) {
            const ChatItem& c = (*chats_)[i];
            if (!filter_.isEmpty() && c.name.toLower().indexOf(filter_) < 0) continue;
            visible_.append(i);
        }
        layoutScroll();
        update();
    }

    void layoutScroll() {
        int content = visible_.count() * rowHeight();
        if (content > height()) {
            vbar_->setGeometry(width() - 13, 0, 13, height());
            vbar_->setRange(0, qMax(0, content - height()));
            vbar_->setPageStep(height());
            vbar_->show();
        } else {
            vbar_->hide();
            scroll_ = 0;
            vbar_->setValue(0);
        }
    }

    void onScroll(int value) {
        scroll_ = value;
        update();
    }

    void paintEvent(QPaintEvent*) {
        WeChatColors c = weChatColors();
        QPainter p(this);
        p.fillRect(rect(), c.listBg);
        if (!chats_) return;
        QFont nameFont = uiFont(14);
        QFont previewFont = uiFont(12);
        QFont timeFont = uiFont(11);
        int rh = rowHeight();
        for (int i = 0; i < visible_.count(); ++i) {
            int y = i * rh - scroll_;
            if (y + rh < 0 || y > height()) continue;
            const ChatItem& chat = (*chats_)[visible_[i]];
            bool selected = (visible_[i] == current_);
            if (selected) p.fillRect(QRect(0, y, width(), rh), c.listSelected);
            else if (i == hover_) p.fillRect(QRect(0, y, width(), rh), c.listHover);

            drawAvatar(p, QRect(14, y + 14, 44, 44), chat.avatarColor, chat.avatarText);

            p.setFont(nameFont);
            p.setPen(QPen(c.text));
            p.drawText(QRect(70, y + 12, width() - 120, 20), Qt::AlignLeft | Qt::AlignVCenter,
                       chat.name);

            p.setFont(timeFont);
            p.setPen(QPen(c.subText));
            p.drawText(QRect(width() - 80, y + 12, 66, 18), Qt::AlignRight | Qt::AlignVCenter,
                       chat.time);

            p.setFont(previewFont);
            p.setPen(QPen(c.subText));
            QString preview = chat.preview;
            while (preview.size() > 2 && p.fontMetrics().width(preview) > width() - 92)
                preview = preview.left(preview.size() - 2);
            p.drawText(QRect(70, y + 36, width() - 92, 20), Qt::AlignLeft | Qt::AlignVCenter,
                       preview);

            if (chat.unread > 0) {
                p.setPen(Qt::NoPen);
                p.setBrush(QBrush(c.badge));
                QRect badge(width() - 38, y + 40, 20, 18);
                if (chat.unread > 9) badge.setWidth(26);
                p.drawRoundedRect(badge, 9, 9);
                p.setBrush(Qt::NoBrush);
                p.setFont(uiFont(10, true));
                p.setPen(QPen(QColor(0xFF, 0xFF, 0xFF)));
                p.drawText(badge, Qt::AlignCenter, QString::number(chat.unread));
            }
        }
        p.setPen(QPen(c.border, 1));
        p.drawLine(width() - 1, 0, width() - 1, height());
    }

    void mousePressEvent(QMouseEvent* event) {
        int row = (event->pos().y() + scroll_) / rowHeight();
        if (row < 0 || row >= visible_.count()) return;
        current_ = visible_[row];
        update();
        currentChanged(current_);
    }

    void mouseMoveEvent(QMouseEvent* event) {
        int row = (event->pos().y() + scroll_) / rowHeight();
        if (row < 0 || row >= visible_.count()) row = -1;
        if (row != hover_) {
            hover_ = row;
            update();
        }
    }

    void leaveEvent(QEvent*) {
        hover_ = -1;
        update();
    }

    void wheelEvent(QWheelEvent* event) {
        int delta = event->delta() > 0 ? -3 * rowHeight() : 3 * rowHeight();
        vbar_->setValue(qBound(0, vbar_->value() + delta, vbar_->maximum()));
    }

    void resizeEvent(QResizeEvent* event) {
        layoutScroll();
        QWidget::resizeEvent(event);
    }

private:
    QList<ChatItem>* chats_;
    QList<int> visible_;
    int current_;
    int hover_;
    int scroll_;
    QScrollBar* vbar_;
    QString filter_;
};

// ===========================================================================
// MessageView
// ===========================================================================
class MessageView : public QWidget {
public:
    explicit MessageView(QWidget* parent = 0)
        : QWidget(parent), scroll_(0), vbar_(0), chat_(0), dirty_(true) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        vbar_ = new QScrollBar(Qt::Vertical, this);
        vbar_->hide();
        vbar_->valueChanged.connect(vbar_, this, &MessageView::onScroll);
    }

    void setChat(const ChatItem* chat) {
        chat_ = chat;
        scroll_ = 0;
        dirty_ = true;
        layoutMessages();
        scrollToBottom();
        update();
    }

    void appendMessage(const ChatMessage& message) {
        if (!chat_) return;
        dirty_ = true;
        layoutMessages();
        scrollToBottom();
        update();
    }

    void scrollToBottom() {
        if (!vbar_->isVisible()) {
            scroll_ = 0;
            return;
        }
        scroll_ = qMax(0, vbar_->maximum());
        vbar_->setValue(scroll_);
        update();
    }

protected:
    struct LaidMessage {
        int y;
        int height;
        QStringList lines;
        bool showTime;
    };

    int contentWidth() const { return width() - (vbar_->isVisible() ? 16 : 0); }

    void layoutMessages() {
        laid_.clearAll();
        if (!chat_ || chat_->messages.isEmpty()) {
            updateScroll();
            return;
        }
        QFont font = uiFont(14);
        QFontMetrics fm(font);
        int maxBubble = qMax(120, (int)(contentWidth() * 0.58));
        int y = 16;
        QString lastTime;
        for (int i = 0; i < chat_->messages.count(); ++i) {
            const ChatMessage& m = chat_->messages[i];
            LaidMessage lm;
            lm.showTime = (m.time != lastTime && m.kind != 1);
            if (lm.showTime) {
                lastTime = m.time;
                y += 26;
            }
            if (m.kind == 1) {
                lm.lines = wrapText(fm, m.text, contentWidth() - 160);
                lm.height = 24 + qMax(1, lm.lines.count()) * (fm.height() + 2);
            } else {
                lm.lines = wrapText(fm, m.text, maxBubble - 24);
                lm.height = qMax(38, lm.lines.count() * (fm.height() + 4) + 20);
            }
            lm.y = y;
            y += lm.height + 12;
            laid_.append(lm);
        }
        contentHeight_ = y + 8;
        updateScroll();
    }

    void updateScroll() {
        int view = height();
        if (contentHeight_ > view) {
            vbar_->setGeometry(width() - 13, 0, 13, height());
            vbar_->setRange(0, contentHeight_ - view);
            vbar_->setPageStep(view);
            vbar_->show();
        } else {
            vbar_->hide();
            scroll_ = 0;
            vbar_->setValue(0);
        }
    }

    void onScroll(int value) {
        scroll_ = value;
        update();
    }

    void paintEvent(QPaintEvent*) {
        WeChatColors c = weChatColors();
        QPainter p(this);
        p.fillRect(rect(), c.chatBg);
        if (!chat_) return;
        if (dirty_) {
            dirty_ = false;
            layoutMessages();
        }
        p.setFont(uiFont(14));
        QFontMetrics fm = p.fontMetrics();
        int cw = contentWidth();
        int maxBubble = qMax(120, (int)(cw * 0.58));

        for (int i = 0; i < chat_->messages.count() && i < laid_.count(); ++i) {
            const ChatMessage& m = chat_->messages[i];
            const LaidMessage& lm = laid_[i];
            int y = lm.y - scroll_;
            if (y + lm.height < 0 || y > height()) continue;

            if (lm.showTime) {
                p.setFont(uiFont(11));
                p.setPen(QPen(c.subText));
                p.drawText(QRect(0, y - 20, width(), 18), Qt::AlignCenter, m.time);
                p.setFont(uiFont(14));
            }

            if (m.kind == 1) {
                p.setFont(uiFont(12));
                p.setPen(QPen(c.subText));
                p.drawText(QRect(70, y, cw - 140, lm.height), Qt::AlignCenter, m.text);
                p.setFont(uiFont(14));
                continue;
            }

            int avatarX = m.mine ? cw - 56 : 16;
            drawAvatar(p, QRect(avatarX, y, 40, 40), m.mine ? QColor(0x07, 0xC1, 0x60)
                                                           : chat_->avatarColor,
                       m.mine ? QString(L"我") : (m.sender.isEmpty() ? chat_->avatarText : m.sender.left(1)));

            int bubbleW = maxBubble;
            int textW = 0;
            for (int l = 0; l < lm.lines.count(); ++l)
                textW = qMax(textW, fm.width(lm.lines[l]));
            bubbleW = qMin(maxBubble, textW + 24);
            if (bubbleW < 40) bubbleW = 40;
            int bubbleH = qMax(38, lm.lines.count() * (fm.height() + 4) + 20);
            int bubbleX = m.mine ? (cw - 70 - bubbleW) : 70;

            if (chat_->group && !m.mine && !m.sender.isEmpty()) {
                p.setFont(uiFont(11));
                p.setPen(QPen(c.subText));
                p.drawText(QRect(bubbleX, y - 16, bubbleW, 14), Qt::AlignLeft, m.sender);
                p.setFont(uiFont(14));
            }

            p.setRenderHint(QPainter::Antialiasing, true);
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(m.mine ? c.bubbleMine : c.bubbleOther));
            p.drawRoundedRect(QRect(bubbleX, y, bubbleW, bubbleH), 6, 6);
            // small tail
            QList<QPoint> tail;
            if (m.mine) {
                tail << QPoint(bubbleX + bubbleW, y + 14)
                     << QPoint(bubbleX + bubbleW + 7, y + 18)
                     << QPoint(bubbleX + bubbleW, y + 22);
            } else {
                tail << QPoint(bubbleX, y + 14)
                     << QPoint(bubbleX - 7, y + 18)
                     << QPoint(bubbleX, y + 22);
            }
            p.drawPolygon(tail);
            p.setBrush(Qt::NoBrush);

            p.setFont(uiFont(14));
            p.setPen(QPen(m.mine ? c.bubbleMineText : c.bubbleOtherText));
            for (int l = 0; l < lm.lines.count(); ++l) {
                p.drawText(QPoint(bubbleX + 12, y + 10 + fm.ascent() + l * (fm.height() + 4)),
                           lm.lines[l]);
            }
        }
        p.setPen(QPen(c.border, 1));
        p.drawLine(0, height() - 1, width(), height() - 1);
    }

    void wheelEvent(QWheelEvent* event) {
        int delta = event->delta() > 0 ? -60 : 60;
        vbar_->setValue(qBound(0, vbar_->value() + delta, vbar_->maximum()));
    }

    void resizeEvent(QResizeEvent* event) {
        dirty_ = true;
        QWidget::resizeEvent(event);
    }

private:
    const ChatItem* chat_;
    QList<LaidMessage> laid_;
    int contentHeight_;
    int scroll_;
    QScrollBar* vbar_;
    bool dirty_;
};

// ===========================================================================
// InputEdit : Enter sends, Shift+Enter inserts a newline
// ===========================================================================
class InputEdit : public QTextEdit {
public:
    explicit InputEdit(QWidget* parent = 0) : QTextEdit(parent) {
        setWordWrap(true);
        setPlaceholderText(L"输入消息...");
        setFont(uiFont(14));
    }

    Signal<void()> submitted;

protected:
    void keyPressEvent(QKeyEvent* event) {
        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
            !(event->modifiers() & Qt::ShiftModifier)) {
            submitted();
            return;
        }
        QTextEdit::keyPressEvent(event);
    }
};

// ---------------------------------------------------------------------------
// ComposerPanel : background + separator for the input area
// ---------------------------------------------------------------------------
class ComposerPanel : public QWidget {
public:
    explicit ComposerPanel(QWidget* parent = 0) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent*) {
        WeChatColors c = weChatColors();
        QPainter p(this);
        p.fillRect(rect(), c.window);
        p.setPen(QPen(c.border, 1));
        p.drawLine(0, 0, width(), 0);
    }
};

// ===========================================================================
// WeChatWindow
// ===========================================================================
class WeChatWindow : public QWidget {
public:
    WeChatWindow();
    ~WeChatWindow();

    void sendMessage();
    void switchChat(int index);
    void setFilter(const QString& text);
    void toggleTheme();

    int selfTest();
    bool screenshot(const QString& path);

private:
    class ChatListWidget* chatList_;
    class MessageView* messages_;
    class InputEdit* input_;
    class WeChatSidebar* sidebar_;
    QLabel* titleLabel_;
    QLabel* subtitleLabel_;
    QLabel* statusLabel_;
    QList<ChatItem> chats_;
    int current_;

    void onSection(int section);
    void onSearch(const QString& text);
    void onChatSelected(int index);
};

WeChatWindow::WeChatWindow() : QWidget(0, Qt::Window), current_(1) {
    setWindowTitle(L"微信");
    resize(1000, 700);
    buildChats(&chats_);

    QHBoxLayout* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    sidebar_ = new WeChatSidebar();
    sidebar_->sectionChanged.connect(sidebar_, this, &WeChatWindow::onSection);
    root->addWidget(sidebar_);

    // ---- conversation column ----
    QWidget* listColumn = new QWidget();
    listColumn->setFixedWidth(260);
    QVBoxLayout* listLayout = new QVBoxLayout(listColumn);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->setSpacing(0);

    QWidget* searchRow = new QWidget();
    searchRow->setFixedHeight(54);
    QHBoxLayout* searchLayout = new QHBoxLayout(searchRow);
    searchLayout->setContentsMargins(10, 9, 10, 9);
    searchLayout->setSpacing(6);
    QLineEdit* search = new QLineEdit();
    search->setPlaceholderText(L"搜索");
    search->setClearButtonEnabled(true);
    search->setFont(uiFont(13));
    QObject::connect(search, search->textChanged, this, &WeChatWindow::onSearch);
    searchLayout->addWidget(search, 1);
    QPushButton* addButton = new QPushButton(L"＋");
    addButton->setFlat(true);
    addButton->setFixedSize(28, 28);
    addButton->setFont(uiFont(16));
    searchLayout->addWidget(addButton);
    listLayout->addWidget(searchRow);

    chatList_ = new ChatListWidget();
    chatList_->setChats(&chats_);
    chatList_->currentChanged.connect(chatList_, this, &WeChatWindow::onChatSelected);
    listLayout->addWidget(chatList_, 1);
    root->addWidget(listColumn);

    // ---- chat column ----
    QWidget* chatColumn = new QWidget();
    QVBoxLayout* chatLayout = new QVBoxLayout(chatColumn);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    QWidget* header = new QWidget();
    header->setFixedHeight(60);
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 0, 16, 0);
    headerLayout->setSpacing(10);
    titleLabel_ = new QLabel();
    QFont titleFont = uiFont(16, true);
    titleLabel_->setFont(titleFont);
    subtitleLabel_ = new QLabel();
    subtitleLabel_->setFont(uiFont(11));
    QLabel* more = new QLabel(L"⋯");
    more->setFont(uiFont(20));
    headerLayout->addWidget(titleLabel_);
    headerLayout->addWidget(subtitleLabel_);
    headerLayout->addStretch(1);
    headerLayout->addWidget(more);
    chatLayout->addWidget(header);

    messages_ = new MessageView();
    chatLayout->addWidget(messages_, 1);

    // ---- composer ----
    QWidget* composer = new ComposerPanel();
    composer->setFixedHeight(170);
    QVBoxLayout* composerLayout = new QVBoxLayout(composer);
    composerLayout->setContentsMargins(16, 6, 16, 10);
    composerLayout->setSpacing(4);

    QWidget* toolRow = new QWidget();
    toolRow->setFixedHeight(28);
    QHBoxLayout* toolLayout = new QHBoxLayout(toolRow);
    toolLayout->setContentsMargins(0, 0, 0, 0);
    toolLayout->setSpacing(4);
    const wchar_t* tools[] = { L"表情", L"文件", L"截图", L"记录" };
    for (int i = 0; i < 4; ++i) {
        QPushButton* b = new QPushButton(QString(tools[i]));
        b->setFlat(true);
        b->setFixedSize(40, 26);
        b->setFont(uiFont(11));
        toolLayout->addWidget(b);
    }
    toolLayout->addStretch(1);
    statusLabel_ = new QLabel(L"Enter 发送 / Shift+Enter 换行");
    statusLabel_->setFont(uiFont(10));
    toolLayout->addWidget(statusLabel_);
    composerLayout->addWidget(toolRow);

    input_ = new InputEdit();
    input_->setMinimumSize(0, 86);
    input_->submitted.connect(input_, this, &WeChatWindow::sendMessage);
    composerLayout->addWidget(input_, 1);

    QWidget* sendRow = new QWidget();
    QHBoxLayout* sendLayout = new QHBoxLayout(sendRow);
    sendLayout->setContentsMargins(0, 2, 0, 0);
    sendLayout->setSpacing(8);
    sendLayout->addStretch(1);
    QPushButton* send = new QPushButton(L"发送(S)");
    send->setFixedSize(96, 30);
    send->setStyleSheet(L"QPushButton { background-color: #07C160; color: white; border: none;"
                        L" border-radius: 4px; }"
                        L"QPushButton:hover { background-color: #06AD56; }"
                        L"QPushButton:disabled { background-color: #B8E6C8; }");
    send->setFont(uiFont(13));
    QObject::connect(send, send->clicked, this, &WeChatWindow::sendMessage);
    sendLayout->addWidget(send);
    composerLayout->addWidget(sendRow);
    chatLayout->addWidget(composer);

    root->addWidget(chatColumn, 1);

    chatList_->setCurrent(current_);
    switchChat(current_);
}

WeChatWindow::~WeChatWindow() {}

void WeChatWindow::onSection(int section) {
    Q_UNUSED(section);
    if (section == 0) {
        chatList_->setCurrent(current_);
        return;
    }
    statusLabel_->setText(L"该功能为演示：会话列表/消息区保持不变");
}

void WeChatWindow::onSearch(const QString& text) {
    setFilter(text);
}

void WeChatWindow::setFilter(const QString& text) {
    chatList_->setFilter(text);
}

void WeChatWindow::onChatSelected(int index) {
    switchChat(index);
}

void WeChatWindow::switchChat(int index) {
    if (index < 0 || index >= chats_.count()) return;
    current_ = index;
    ChatItem& chat = chats_[index];
    chat.unread = 0;
    titleLabel_->setText(chat.name);
    subtitleLabel_->setText(chat.group ? QString::number(chat.members) + L" 人" : QString());
    messages_->setChat(&chat);
    chatList_->setCurrent(index);
    chatList_->update();
    input_->setFocus();
    statusLabel_->setText(L"Enter 发送 / Shift+Enter 换行");
}

void WeChatWindow::sendMessage() {
    QString text = input_->toPlainText().trimmed();
    if (text.isEmpty()) return;
    ChatItem& chat = chats_[current_];
    ChatMessage m;
    m.mine = true;
    m.sender = L"我";
    m.text = text;
    m.time = L"刚刚";
    m.kind = 0;
    chat.messages.append(m);
    chat.preview = text;
    chat.time = L"刚刚";
    input_->clear();
    messages_->appendMessage(m);
    chatList_->update();
    statusLabel_->setText(L"已发送");
}

void WeChatWindow::toggleTheme() {
    if (QTheme::isDark()) QTheme::setMode(QTheme::Light);
    else QTheme::setMode(QTheme::Dark);
    update();
}

int WeChatWindow::selfTest() {
    int failures = 0;
    // end-to-end input test: the coordinates go through WM_LBUTTONDOWN/UP
    HWND hwnd = (HWND)winId();
    int clickX = 62 + 130;
    int clickY = 54 + 2 * 72 + 36; // third conversation row
    SendMessage(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(clickX, clickY));
    SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(clickX, clickY));
    SendMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(clickX, clickY));
    if (current_ != 2) {
        printf("FAIL: mouse click selected %d\n", current_);
        ++failures;
    }
    switchChat(1);
    if (chats_.count() < 5) { printf("FAIL: chats\n"); ++failures; }
    if (chats_[1].messages.isEmpty()) { printf("FAIL: messages\n"); ++failures; }
    int before = chats_[1].messages.count();
    input_->setPlainText(L"自动化测试消息");
    sendMessage();
    if (chats_[1].messages.count() != before + 1) { printf("FAIL: send\n"); ++failures; }
    if (!chats_[1].messages.last().mine) { printf("FAIL: mine flag\n"); ++failures; }
    if (input_->toPlainText().size() != 0) { printf("FAIL: input clear\n"); ++failures; }
    switchChat(2);
    if (current_ != 2) { printf("FAIL: switch\n"); ++failures; }
    if (chats_[2].unread != 0) { printf("FAIL: unread reset\n"); ++failures; }
    setFilter(L"产品");
    if (chatList_->visibleCount() != 1) { printf("FAIL: filter %d\n", chatList_->visibleCount()); ++failures; }
    setFilter(QString());
    toggleTheme();
    bool dark = QTheme::isDark();
    toggleTheme();
    if (dark == QTheme::isDark()) { printf("FAIL: theme\n"); ++failures; }
    printf("wechat demo selftest: %s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures;
}

bool WeChatWindow::screenshot(const QString& path) {
    QPixmap pm = grab();
    return pm.save(path);
}

// ===========================================================================
// entry point
// ===========================================================================
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QTheme::setMode(QTheme::Light);

    WeChatWindow window;
    QString shot;
    QString theme;
    bool selfTest = false;
    int chat = -1;
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromUtf8(argv[i]);
        if (arg == L"--screenshot" && i + 1 < argc) shot = QString::fromUtf8(argv[++i]);
        else if (arg == L"--theme" && i + 1 < argc) theme = QString::fromUtf8(argv[++i]);
        else if (arg == L"--chat" && i + 1 < argc) chat = QString::fromUtf8(argv[++i]).toInt();
        else if (arg == L"--selftest") selfTest = true;
    }
    if (theme == L"light") QTheme::setMode(QTheme::Light);
    else if (theme == L"dark") QTheme::setMode(QTheme::Dark);
    else if (theme == L"system") QTheme::setMode(QTheme::System);
    if (chat >= 0) window.switchChat(chat);

    window.show();
    for (int i = 0; i < 20; ++i) QApplication::processEvents();

    if (selfTest) return window.selfTest() ? 1 : 0;
    if (!shot.isEmpty()) {
        bool ok = window.screenshot(shot);
        printf("wechat demo screenshot %s -> %s\n", ok ? "saved" : "FAILED", shot.toUtf8().c_str());
        return ok ? 0 : 1;
    }
    return QApplication::exec();
}
