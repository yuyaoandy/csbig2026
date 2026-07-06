// storewidget.cpp
#include "storewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <random>
#include "card.h"
#include<algorithm>
// 构造函数
extern long long money_count;
extern QVector<Card> stored_card;
StoreWidget::StoreWidget(QWidget *parent,int pid)
    : QWidget(parent),id(pid),nextItemId(1)
{
    initUI();
    initDefaultItems();
    rebuildItemsView();
    updateMoneyDisplay();
}
void StoreWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    rebuild();   // 刷新商品列表和金钱
}
void StoreWidget::rebuild(){
    initDefaultItems();
    rebuildItemsView();
    updateMoneyDisplay();
}
StoreWidget::~StoreWidget()
{
    // 清理商品卡片控件
    qDeleteAll(itemWidgets);
    itemWidgets.clear();
}

// 初始化UI布局和样式
void StoreWidget::initUI()
{
    setMinimumSize(900, 650);
    setStyleSheet(R"(
        QWidget {
            background-color: #1a1a2e;
            font-family: "Microsoft YaHei", "Segoe UI", "PingFang SC", "SimHei";
        }
        QLabel#titleLabel {
            font-size: 28px;
            font-weight: bold;
            color: #ffd966;
            padding: 10px;
            qproperty-alignment: AlignCenter;
            background-color: rgba(0,0,0,0.5);
            border-radius: 10px;
            margin: 5px;
        }
        QLabel#moneyLabel {
            font-size: 20px;
            font-weight: bold;
            color: #ffaa33;
            background-color: #2a2a3e;
            border-radius: 15px;
            padding: 8px 20px;
            margin: 5px;
        }
        QLabel#messageLabel {
            font-size: 14px;
            color: #cccccc;
            background-color: #25253a;
            border-radius: 8px;
            padding: 5px;
            margin: 5px;
        }
        QPushButton {
            background-color: #3a3a55;
            color: #eeeeee;
            border: 2px solid #6a6a8a;
            border-radius: 10px;
            padding: 6px 12px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #5a5a7a;
            border-color: #ffaa33;
        }
        QPushButton:pressed {
            background-color: #2a2a45;
        }
        QPushButton#closeBtn {
            background-color: #5a3a3a;
            border-color: #aa6666;
        }
        QPushButton#closeBtn:hover {
            background-color: #7a4a4a;
            border-color: #ff8866;
        }
        QScrollArea {
            border: none;
            background-color: transparent;
        }
        QScrollBar:vertical {
            background: #2a2a3e;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background: #6a6a8a;
            border-radius: 6px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #9a9aba;
        }
    )");

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    QLabel *titleLabel = new QLabel("商店", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 顶部区域：金钱显示 + 占位
    QHBoxLayout *topLayout = new QHBoxLayout();
    moneyLabel = new QLabel(this);
    moneyLabel->setObjectName("moneyLabel");
    moneyLabel->setAlignment(Qt::AlignCenter);
    topLayout->addStretch();
    topLayout->addWidget(moneyLabel);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    // 滚动区域（商品列表）
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    itemsContainer = new QWidget();
    itemsLayout = new QGridLayout(itemsContainer);
    itemsLayout->setContentsMargins(10, 10, 10, 10);
    itemsLayout->setSpacing(20);
    itemsLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(itemsContainer);
    mainLayout->addWidget(scrollArea, 1); // 滚动区域占满剩余空间

    // 底部区域：提示信息 + 关闭按钮
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    messageLabel = new QLabel(this);
    messageLabel->setObjectName("messageLabel");
    messageLabel->setMinimumHeight(40);
    messageLabel->setAlignment(Qt::AlignCenter);
    bottomLayout->addWidget(messageLabel, 1);

    QPushButton *closeBtn = new QPushButton("离开商店", this);
    closeBtn->setObjectName("closeBtn");
    closeBtn->setFixedSize(120, 40);
    connect(closeBtn, &QPushButton::clicked, this, &StoreWidget::onClose);
    bottomLayout->addWidget(closeBtn);

    mainLayout->addLayout(bottomLayout);
}
QString StoreWidget::getRarityColor(int rarity) const
{
    // 深色背景，无圆角，稀有度越高颜色越醒目（但仍是暗色）
    switch (rarity) {
    case 1: return "#16161e";   // 极暗灰
    case 2: return "#1a2a1a";   // 暗绿
    case 3: return "#1a2a4a";   // 暗蓝
    case 4: return "#2a1a3a";   // 暗紫
    case 5: return "#3a2a1a";   // 暗金
    default: return "#14141e";  // 默认深灰
    }
}
// 初始化默认商品列表（类似杀戮尖塔的卡牌/药水/遗物混合）
void StoreWidget::initDefaultItems()
{
    availableItems.clear();
    nextItemId = 1;

    if (stored_card.isEmpty()) {
        StoreItem placeholder;
        placeholder.id = nextItemId++;
        placeholder.name = "无卡牌";
        placeholder.desc = "卡牌库为空";
        placeholder.price = 0;
        placeholder.iconChar = "⚠️";
        placeholder.rarity = 1;
        availableItems.append(placeholder);
        return;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    QVector<Card> shuffled = stored_card;
    std::shuffle(shuffled.begin(), shuffled.end(), g);
    int count = qMin(5, shuffled.size());

    for (int i = 0; i < count; ++i) {
        const Card &card = shuffled[i];
        StoreItem item;
        item.id = nextItemId++;
        item.name = card.chinese+QString("%1").arg(0);
        item.desc = card.description;
        item.rarity = card.rarity;    // ★ 直接使用卡牌中的稀有度数字（1~5）
        item.truename=card.name+QString("%1").arg(0);
        int basePrice = card.rarity * 30 + 50;
        int offset = (rand() % 21) - 10;
        item.price = qMax(10, basePrice + offset);

        // 图标可根据稀有度或名称调整（示例）
        if (card.rarity >= 4)      item.iconChar = "✨";
        else if (card.rarity >= 3) item.iconChar = "⚔️";
        else if (card.rarity >= 2) item.iconChar = "🛡️";
        else                       item.iconChar = "🃏";

        availableItems.append(item);
    }
}

// 重建商品卡片区域（每次购买后重新生成）
void StoreWidget::rebuildItemsView()
{
    qDeleteAll(itemWidgets);
    itemWidgets.clear();
    qDebug() << "[Store] rebuild() called"<<stored_card.size()<<Qt::endl;
    QLayoutItem *child;
    while ((child = itemsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    if (availableItems.isEmpty()) {
        QLabel *emptyLabel = new QLabel("商店已售罄", itemsContainer);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #aaaaaa; font-size: 18px; background-color: transparent;");
        itemsLayout->addWidget(emptyLabel, 0, 0, 1, 3);
        return;
    }

    int columnCount = 3;
    int row = 0, col = 0;
    const int cardWidth = 240;
    const int cardHeight = 280;   // 高度不变，但内边距减小

    for (const StoreItem &item : availableItems) {
        QFrame *cardFrame = new QFrame(itemsContainer);
        cardFrame->setFrameShape(QFrame::Box);
        cardFrame->setFixedSize(cardWidth, cardHeight);

        // 根据稀有度获取背景色
        QString bgColor = getRarityColor(item.rarity);
        QColor base(bgColor);
        int r = qMin(255, base.red() + 20);
        int g = qMin(255, base.green() + 20);
        int b = qMin(255, base.blue() + 20);
        QString hoverColor = QString("#%1%2%3").arg(r, 2, 16, QChar('0'))
                                 .arg(g, 2, 16, QChar('0'))
                                 .arg(b, 2, 16, QChar('0'));

        // ★ 修改：减小内边距至 6px
        cardFrame->setStyleSheet(QString(R"(
            QFrame {
                background-color: %1;
                border: 2px solid #2a2a3e;
                padding: 6px;
            }
            QFrame:hover {
                border-color: #ffaa33;
                background-color: %2;
            }
            QLabel#itemName {
                font-size: 18px;
                font-weight: bold;
                color: #ffcc66;
            }
            QLabel#itemPrice {
                font-size: 16px;
                font-weight: bold;
                color: #ffaa33;
            }
            QLabel#itemDesc {
                font-size: 12px;
                color: #bbbbdd;
                word-wrap: true;
            }
            QPushButton {
                background-color: #2a2a4a;
                border: 2px solid #3a3a5a;
                padding: 6px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #3a3a6a;
                border-color: #ffaa33;
            }
        )").arg(bgColor).arg(hoverColor));

        QVBoxLayout *cardLayout = new QVBoxLayout(cardFrame);
        cardLayout->setSpacing(6);   // ★ 减小内部间距
        cardLayout->setContentsMargins(4, 4, 4, 4);  // ★ 布局内边距也减小

        // 标题行
        QHBoxLayout *titleLayout = new QHBoxLayout();
        QLabel *iconLabel = new QLabel(item.iconChar, cardFrame);
        iconLabel->setStyleSheet("font-size: 32px; background: transparent;");
        QLabel *nameLabel = new QLabel(item.name, cardFrame);
        nameLabel->setObjectName("itemName");
        titleLayout->addWidget(iconLabel);
        titleLayout->addWidget(nameLabel, 1);
        titleLayout->setAlignment(Qt::AlignLeft);
        cardLayout->addLayout(titleLayout);

        // 描述（★ 高度增加至 100px，确保30字完整显示）
        QLabel *descLabel = new QLabel(item.desc, cardFrame);
        descLabel->setObjectName("itemDesc");
        descLabel->setWordWrap(true);
        descLabel->setFixedHeight(100);
        descLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        cardLayout->addWidget(descLabel);

        // 价格 + 购买按钮
        QHBoxLayout *priceLayout = new QHBoxLayout();
        QLabel *priceLabel = new QLabel("💰 " + QString::number(item.price), cardFrame);
        priceLabel->setObjectName("itemPrice");
        priceLayout->addWidget(priceLabel, 1);

        QPushButton *buyBtn = new QPushButton("购买", cardFrame);
        buyBtn->setCursor(Qt::PointingHandCursor);
        connect(buyBtn, &QPushButton::clicked, this, [this, itemId = item.id]() {
            purchaseItemById(itemId);
        });
        priceLayout->addWidget(buyBtn);
        cardLayout->addLayout(priceLayout);

        itemsLayout->addWidget(cardFrame, row, col);
        itemWidgets.append(cardFrame);

        col++;
        if (col >= columnCount) {
            col = 0;
            row++;
        }
    }

    // 补空白占位
    if (col != 0) {
        for (int i = col; i < columnCount; ++i) {
            QWidget *spacer = new QWidget(itemsContainer);
            spacer->setFixedSize(cardWidth, cardHeight);
            spacer->setStyleSheet("background: transparent;");
            itemsLayout->addWidget(spacer, row, i);
        }
    }
}
// 购买商品逻辑（通过唯一ID）
void StoreWidget::purchaseItemById(int itemId)
{
    // 查找商品
    int index = -1;
    StoreItem targetItem;
    for (int i = 0; i < availableItems.size(); ++i) {
        if (availableItems[i].id == itemId) {
            index = i;
            targetItem = availableItems[i];
            break;
        }
    }

    if (index == -1) {
        showTemporaryMessage("商品不存在或已售罄！", true);
        return;
    }

    // 检查金钱是否足够
    if (money_count >= targetItem.price) {
        // 扣款
        money_count -= targetItem.price;
        updateMoneyDisplay();

        // 从列表中移除商品
        availableItems.removeAt(index);

        // 重建界面
        rebuildItemsView();

        // 显示购买成功消息
        showTemporaryMessage(QString("购买成功！获得 %1  (-%2金币)").arg(targetItem.name).arg(targetItem.price));

        // 可在此处触发全局信号或记录到背包，这里简单控制台输出
        qDebug() << "[商店] 玩家购买了:" << targetItem.name << "剩余金币:" << money_count;
        emit cardBought(targetItem.truename,targetItem.price);
    } else {
        showTemporaryMessage(QString("金币不足！需要 %1 金币，当前只有 %2 金币").arg(targetItem.price).arg(money_count), true);
    }
}

// 更新顶部金钱显示
void StoreWidget::updateMoneyDisplay()
{
    moneyLabel->setText(QString("💰 金币 : %1 💰").arg(money_count));
}

// 显示临时提示（2秒后自动清除）
void StoreWidget::showTemporaryMessage(const QString &msg, bool isError)
{
    messageLabel->setText(msg);
    if (isError) {
        messageLabel->setStyleSheet("color: #ff8888; background-color: #3a2525; border-radius: 8px; padding: 5px;");
    } else {
        messageLabel->setStyleSheet("color: #aaffaa; background-color: #253a25; border-radius: 8px; padding: 5px;");
    }
    // 2秒后恢复默认样式并清空消息
    QTimer::singleShot(2000, this, [this]() {
        if (messageLabel->text().isEmpty()) return;
        messageLabel->setText(" ");
        messageLabel->setStyleSheet("color: #cccccc; background-color: #25253a; border-radius: 8px; padding: 5px;");
    });
}

// 关闭商店
void StoreWidget::onClose()
{
    // 可以发出关闭信号，这里直接关闭窗口
    emit goback(this->id);
}