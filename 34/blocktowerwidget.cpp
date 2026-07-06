//构建 维修复用
#include "blocktowerwidget.h"
#include "towerdisplaywidget.h"
#include <QRegularExpression>
#include <QJsonDocument>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <iostream>
#include <QJsonObject>
#include <QJsonArray>
#include"card.h"
using namespace std;

BlockTowerWidget::BlockTowerWidget(const QVector<BlockType> &types, QWidget *parent)
    : QWidget(parent), m_types(types)
{
    for (const auto &t : m_types)
        m_colorTable.append(t.color);
    // ---------- 左侧塔显示（不变） ----------
    m_towerDisplay = new TowerDisplayWidget;
    refreshTowerDisplay();
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setWidget(m_towerDisplay);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // ---------- 右侧布局 ----------
    QVBoxLayout *rightLayout = new QVBoxLayout;

    // 标题
    QLabel *titleLabel = new QLabel("卡牌种类");
    titleLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(titleLabel);

    // 库存滚动区域（不变）
    QScrollArea *stockScrollArea = new QScrollArea;
    stockScrollArea->setWidgetResizable(true);
    stockScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    stockScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QWidget *stockContainer = new QWidget;
    stockLayout = new QVBoxLayout(stockContainer);
    stockLayout->setContentsMargins(0,0,0,0);
    stockLayout->setSpacing(6);

    // 根据初始类型创建库存按钮（不变）
    for (int i = 0; i < m_types.size(); ++i) {
        QPushButton *btn = new QPushButton;
        QPixmap pix(20,20);
        pix.fill(m_types[i].color);
        btn->setIcon(QIcon(pix));
        btn->setIconSize(QSize(20,20));
        btn->setText(QString("%1 x%2").arg(m_types[i].chinese).arg(m_types[i].currentCount));
        btn->setProperty("typeIndex", i);
        connect(btn, &QPushButton::clicked, this, &BlockTowerWidget::onStockButtonClicked);
        stockLayout->addWidget(btn);
        m_stockButtons.append(btn);
    }
    stockLayout->addStretch();
    stockScrollArea->setWidget(stockContainer);
    rightLayout->addWidget(stockScrollArea, 1);

    // ---------- 底部控制面板 ----------
    QWidget *controlPanel = new QWidget;
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setContentsMargins(0,8,0,0);

    // 正常模式：合成按钮
    m_synthesizeBtn = new QPushButton("合成（消耗2张）", this);
    connect(m_synthesizeBtn, &QPushButton::clicked, this, &BlockTowerWidget::onSynthesizeClicked);
    controlLayout->addWidget(m_synthesizeBtn);


    // 不再创建 m_deleteBtn

    // 删除模式开关（塔中块删除）保留，注意它和库存移除是两回事
    m_deleteModeBtn = new QPushButton("删除模式", this);
    m_deleteModeBtn->setCheckable(true);
    connect(m_deleteModeBtn, &QPushButton::toggled, this, &BlockTowerWidget::onDeleteToggled);
    controlLayout->addWidget(m_deleteModeBtn);

    // 清空（通用）
    m_clearBtn = new QPushButton("清空", this);
    connect(m_clearBtn, &QPushButton::clicked, this, &BlockTowerWidget::onClearClicked);
    controlLayout->addWidget(m_clearBtn);

    // 正常模式返回按钮
    m_returnBtn = new QPushButton("返回", this);
    connect(m_returnBtn, &QPushButton::clicked, this, &BlockTowerWidget::OnClickedreturn);
    controlLayout->addWidget(m_returnBtn);

    // ---------- 维修模式控制栏（默认隐藏） ----------


    // 将控制面板添加到右侧布局
    rightLayout->addWidget(controlPanel);

    // ---------- 整体布局 ----------
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_scrollArea, 3);
    mainLayout->addLayout(rightLayout, 1);
    rightLayout->parentWidget()->setMinimumWidth(300);

    // 连接塔点击信号
    connect(m_towerDisplay, &TowerDisplayWidget::blockClicked,
            this, &BlockTowerWidget::onTowerBlockClicked);
    connect(m_towerDisplay, &TowerDisplayWidget::insertRequested,
            this, &BlockTowerWidget::onInsertRequested);
    refreshTowerDisplay();
    m_towerDisplay->update();
}
BlockTowerWidget::~BlockTowerWidget()
{
    // 如果不需要手动清理资源，可以空实现
}



void BlockTowerWidget::addBlockToTower(int typeIndex)
{
    if (typeIndex < 0 || typeIndex >= m_types.size()) return;
    if (m_types[typeIndex].currentCount <= 0) {
        QMessageBox::information(this, "库存不足",
                                 QString("没有可用的 %1 卡牌了！").arg(m_types[typeIndex].name));
        return;
    }
    m_types[typeIndex].currentCount--;
    TowerBlock block;
    block.typeIndex = typeIndex;
    block.innerType = -1;
    m_towerBlocks.append(block);
    refreshTowerDisplay();
    updateStockDisplay();
}

void BlockTowerWidget::removeBlockFromTower(int index)
{
    if (index < 0 || index >= m_towerBlocks.size()) return;
    const TowerBlock &block = m_towerBlocks.at(index);
    // 回收外层库存
    if (block.typeIndex >= 0 && block.typeIndex < m_types.size())
        m_types[block.typeIndex].currentCount++;
    // 回收内层库存（如果有）
    if (block.innerType >= 0 && block.innerType < m_types.size())
        m_types[block.innerType].currentCount++;
    m_towerBlocks.removeAt(index);
    refreshTowerDisplay();
    updateStockDisplay();
}

void BlockTowerWidget::removeInnerBlock(int index)
{
    if (index < 0 || index >= m_towerBlocks.size()) return;
    TowerBlock &block = m_towerBlocks[index];
    if (block.innerType < 0) return;
    // 回收内层库存
    if (block.innerType < m_types.size())
        m_types[block.innerType].currentCount++;
    block.innerType = -1;
    refreshTowerDisplay();
    updateStockDisplay();
}



void BlockTowerWidget::onDeleteToggled(bool checked)
{
    m_deleteMode = checked;
    m_deleteModeBtn->setText(checked ? "删除模式(点击退出)" : "删除模式");
    m_towerDisplay->setCursor(checked ? Qt::PointingHandCursor : Qt::ArrowCursor);
    if (checked) {
        m_selectedBlockIndex = -1;
        m_selectedStockType = -1;
        updateStockButtonSelection();
        m_towerDisplay->setInsertionEnabled(false);// 锁住
    } else {
        m_towerDisplay->setInsertionEnabled(true);
    }
}

void BlockTowerWidget::onTowerBlockClicked(int index, bool innerClicked)
{
    if (m_deleteMode) {
        if (innerClicked) {
            // 删除内层卡牌
            removeInnerBlock(index);
        } else {
            // 删除整个卡牌
            removeBlockFromTower(index);
        }
        return;
    }

    // 非删除模式：用于选中卡牌以放置内层
    if (m_selectedBlockIndex == index) {
        m_selectedBlockIndex = -1;
    } else {
        if (index >= 0 && index < m_towerBlocks.size()) {
            int type = m_towerBlocks.at(index).typeIndex;
            std::cerr<<m_types[type].name.toStdString()<<std::endl;
            if (type >= 0 && type < m_types.size() && m_types[type].name.length()>=5 && m_types[type].name.mid(0,5) == "Cgoto") {
                std::cerr<<"ok"<<std::endl;
                m_selectedBlockIndex = index;
            } else {
                m_selectedBlockIndex = -1;
            }
        }
    }
    m_towerDisplay->update();
}

void BlockTowerWidget::onClearClicked()
{
    for (const auto &block : m_towerBlocks) {
        if (block.typeIndex >= 0 && block.typeIndex < m_types.size())
            m_types[block.typeIndex].currentCount++;
        if (block.innerType >= 0 && block.innerType < m_types.size())
            m_types[block.innerType].currentCount++;
    }
    m_towerBlocks.clear();
    refreshTowerDisplay();
    updateStockDisplay();
    m_selectedBlockIndex = -1;
}

void BlockTowerWidget::Addblock(QString name)
{
    if (name.isEmpty()) return;

    QString fullName =name;
    for (int i = 0; i < m_types.size(); ++i) {
        if (m_types[i].name == fullName) {
            m_types[i].currentCount++;
            updateStockDisplay();
            return;
        }
    }

    BlockType newType(name,false);
    extern QVector<Card> stored_card;
    bool found = false;
    QRegularExpression rx("^(.*?)(\\d+)$");
    QRegularExpressionMatch match = rx.match(name);
    if (match.hasMatch()) {// 名称+等级
        QString base = match.captured(1);
        int level = match.captured(2).toInt();//
        QString baseCardName = base ;
        for (const Card &c : stored_card) {
            if (c.name == baseCardName) {
                newType.chinese = c.chinese + QString::number(level);
                //cerr<<c.name.toStdString()<<" "<<c.chinese.toStdString()<<" "<<newType.chinese.toStdString()<<endl;
                if (base.startsWith("S")) {
                    newType.color = QColor(100, 149, 237);
                } else if (base.startsWith("A")) {
                    newType.color = QColor(255, 165, 0);
                } else if (base.startsWith("L")) {
                    newType.color = QColor(144, 238, 144);
                } else {
                    newType.color = QColor(200, 200, 200);
                }
                found = true;
                break;
            }
        }
    }
    if (!found) {
        for (const Card &c : stored_card) {
            if (c.name == name) {
                newType.chinese = c.chinese;
                if (name.startsWith("S")) {
                    newType.color = QColor(100, 149, 237);
                } else if (name.startsWith("A")) {
                    newType.color = QColor(255, 165, 0);
                } else if (name.startsWith("L")) {
                    newType.color = QColor(144, 238, 144);
                } else {
                    newType.color = QColor(200, 200, 200);
                }
                found = true;
                break;
            }
        }
    }

    newType.currentCount = 1;

    m_types.append(newType);
    int newIndex = m_types.size() - 1;
    m_colorTable.append(newType.color);
    QPushButton *btn = new QPushButton;
    btn->setProperty("typeIndex", newIndex);
    QPixmap pix(20, 20);
    pix.fill(newType.color);
    btn->setIcon(QIcon(pix));
    btn->setIconSize(QSize(20, 20));
    btn->setText(QString("%1 x%2").arg(newType.chinese).arg(newType.currentCount));
    connect(btn, &QPushButton::clicked, this, &BlockTowerWidget::onStockButtonClicked);
    QLayoutItem *last = stockLayout->takeAt(stockLayout->count() - 1);
    if (last && !last->spacerItem()) {
        stockLayout->addItem(last);
    } else if (last) {
        delete last;
    }
    stockLayout->addWidget(btn);
    stockLayout->addStretch();
    m_stockButtons.append(btn);
    updateStockDisplay();
    if (m_towerDisplay) {
        refreshTowerDisplay();
    }
}
void BlockTowerWidget::updateStockDisplay()
{
    for (int i = 0; i < m_stockButtons.size() && i < m_types.size(); ++i) {
        const BlockType &type = m_types.at(i);
        m_stockButtons[i]->setText(QString("%1 x%2").arg(type.chinese).arg(type.currentCount));
    }
}
std::vector<std::string> BlockTowerWidget::getBlockNamesFromTop() const
{
    std::vector< std::string > names;
    for (const auto &block : m_towerBlocks) {
        QString name;
        if (block.typeIndex >= 0 && block.typeIndex < m_types.size()) {
            name = m_types.at(block.typeIndex).name;
        }
        if (block.innerType >= 0 && block.innerType < m_types.size()) {
            name += m_types.at(block.innerType).name;
        }
        if (!name.isEmpty()) {
            names.push_back(name.toLocal8Bit().toStdString());
        }
        //cerr<<names.back()<<endl;
    }
    return names;
}
void BlockTowerWidget::OnClickedreturn(){
    emit goback();
}
BlockType::BlockType(QString nm,bool islabel){
    if(islabel){
        name="L"+nm;
        chinese="标签"+nm;
    }else {
        name=nm;
    }
    initialCount=currentCount=0;

    color=QColor(0, 255, 0);
}
void BlockTowerWidget::onSynthesizeClicked()
{
    extern QVector<Card> stored_card; // 需要全局卡牌库

    if (m_currentMode == NormalMode) {
        // ========== 原有合成逻辑（消耗2张） ==========
        QStringList items;
        QList<int> typeIndices;
        QList<QString> baseNames;
        QList<int> currentLevels;
        QList<QString> baseChineseList;
        for (int i = 0; i < m_types.size(); ++i) {
            const BlockType &type = m_types[i];
            if (type.currentCount < 2) continue;   // 至少需要2张

            QString rawName = type.name;
            QRegularExpression rx("(\\d+)$");
            QRegularExpressionMatch match = rx.match(rawName);
            if (!match.hasMatch()) continue;

            QString base = rawName.left(match.capturedStart());
            int level = match.captured(1).toInt();

            int maxLevel = -1;
            QString chineseBase;
            for (const Card &c : stored_card) {
                if (c.name == base) {
                    maxLevel = c.max_level;
                    chineseBase = c.chinese;
                    break;
                }
            }
            if (maxLevel == -1 || level >= maxLevel) continue;

            items << type.chinese + QString(" (等级%1 → %2)").arg(level).arg(level + 1);
            typeIndices << i;
            baseNames << base;
            currentLevels << level;
            baseChineseList << chineseBase;
        }

        if (items.isEmpty()) {
            QMessageBox::information(this, "合成", "没有可合成的卡牌（需≥2张且未达最高等级）");
            return;
        }

        bool ok;
        QString selected = QInputDialog::getItem(this, "合成升级", "选择要合成的卡牌（消耗2张）:", items, 0, false, &ok);
        if (!ok || selected.isEmpty()) return;

        int idx = items.indexOf(selected);
        int typeIdx = typeIndices[idx];
        QString base = baseNames[idx];
        int curLevel = currentLevels[idx];
        QString chineseBase = baseChineseList[idx];
        m_types[typeIdx].currentCount -= 2;
        if (m_types[typeIdx].currentCount < 0) m_types[typeIdx].currentCount = 0;
        QString newName = base + QString::number(curLevel + 1);
        Addblock(newName);

        updateStockDisplay();
        if (m_towerDisplay) {
            refreshTowerDisplay();
            m_towerDisplay->update();
        }
        QMessageBox::information(this, "合成成功", QString("成功合成 %1%2！")
                                                       .arg(chineseBase).arg(curLevel + 1));
    } else {
        QStringList items;
        QList<int> typeIndices;
        QList<QString> baseNames;
        QList<int> currentLevels;

        for (int i = 0; i < m_types.size(); ++i) {
            const BlockType &type = m_types[i];
            if (type.currentCount < 1) continue;   // 至少1张

            QString rawName = type.name;
            QRegularExpression rx("(\\d+)$");
            QRegularExpressionMatch match = rx.match(rawName);
            if (!match.hasMatch()) continue;

            QString base = rawName.left(match.capturedStart());
            int level = match.captured(1).toInt();

            int maxLevel = -1;
            for (const Card &c : stored_card) {
                if (c.name == base) {
                    maxLevel = c.max_level;
                    break;
                }
            }
            if (maxLevel == -1 || level >= maxLevel) continue;

            items << type.chinese + QString(" (等级%1 → %2)").arg(level).arg(level + 1);
            typeIndices << i;
            baseNames << base;
            currentLevels << level;
        }

        if (items.isEmpty()) {
            QMessageBox::information(this, "升级", "没有可升级的卡牌（需库存≥1且未达最高等级）");

            return;
        }

        bool ok;
        QString selected = QInputDialog::getItem(this, "升级卡牌", "选择要升级的卡牌（消耗1张）:", items, 0, false, &ok);
        if (!ok || selected.isEmpty()) return;

        int idx = items.indexOf(selected);
        int typeIdx = typeIndices[idx];
        QString base = baseNames[idx];
        int curLevel = currentLevels[idx];

        // 消耗1张
        m_types[typeIdx].currentCount--;

        // 添加高一级卡牌
        QString newName = base + QString::number(curLevel + 1);
        Addblock(newName);

        updateStockDisplay();
        if (m_towerDisplay) {
            refreshTowerDisplay();
            m_towerDisplay->update();
        }
        QMessageBox::information(this, "升级成功", QString("成功升级为 %1%2！")
                                                       .arg(base).arg(curLevel + 1));
        emit goback();
    }
}

//new
void BlockTowerWidget::onStockButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int typeIdx = btn->property("typeIndex").toInt();
    if (typeIdx < 0 || typeIdx >= m_types.size()) return;
    if (m_selectedBlockIndex >= 0 && m_selectedBlockIndex < m_towerBlocks.size()) {
        TowerBlock &target = m_towerBlocks[m_selectedBlockIndex];
        bool isRed = (target.typeIndex >= 0 && target.typeIndex < m_types.size() &&
                      m_types[target.typeIndex].name.length() >= 5 &&
                      m_types[target.typeIndex].name.mid(0, 5) == "Cgoto");
        bool isBlue = (typeIdx < m_types.size() && m_types[typeIdx].name.startsWith("L"));
        if (isRed && isBlue) {
            if (m_types[typeIdx].currentCount <= 0) {
                QMessageBox::information(this, "库存不足", "没有可用的蓝色卡牌了！");
            } else {
                if (target.innerType >= 0 && target.innerType < m_types.size()) {
                    m_types[target.innerType].currentCount++;
                }
                m_types[typeIdx].currentCount--;
                target.innerType = typeIdx;
                refreshTowerDisplay();
                updateStockDisplay();
            }
            m_selectedBlockIndex = -1;
            return;
        } else {
            m_selectedBlockIndex = -1;
        }
    }

    // 选中/取消
    if (m_selectedStockType == typeIdx) {
        m_selectedStockType = -1;
    } else {
        if (m_types[typeIdx].currentCount <= 0) {
            QMessageBox::information(this, "库存不足", "该卡牌已无库存，无法选中");
            return;
        }
        m_selectedStockType = typeIdx;
    }
    updateStockButtonSelection();
    m_towerDisplay->update();
}

void BlockTowerWidget::updateStockButtonSelection()
{
    for (int i = 0; i < m_stockButtons.size(); ++i) {
        QPushButton *btn = m_stockButtons[i];
        if (i == m_selectedStockType) {
            btn->setStyleSheet("border: 2px solid red;");
        } else {
            btn->setStyleSheet("");
        }
    }
}
void BlockTowerWidget::onInsertRequested(int insertIndex)
{
    if (m_selectedStockType < 0 || m_selectedStockType >= m_types.size()) {
        QMessageBox::information(this, "提示", "请先在右侧选择一种卡牌");
        return;
    }
    if (m_types[m_selectedStockType].currentCount <= 0) {
        QMessageBox::information(this, "库存不足", "该卡牌已无库存");
        m_selectedStockType = -1;
        updateStockButtonSelection();
        return;
    }

    m_types[m_selectedStockType].currentCount--;
    TowerBlock newBlock;
    newBlock.typeIndex = m_selectedStockType;
    newBlock.innerType = -1;

    // 插入
    if (insertIndex < 0) insertIndex = 0;
    if (insertIndex > m_towerBlocks.size()) insertIndex = m_towerBlocks.size();
    m_towerBlocks.insert(insertIndex, newBlock);

    refreshTowerDisplay();
    updateStockDisplay();
    // 清空
    if (m_types[m_selectedStockType].currentCount == 0) {
        m_selectedStockType = -1;
    }
    updateStockButtonSelection();
    m_towerDisplay->update();
}

void BlockTowerWidget::onReturnMapClicked()
{
    emit exitRepair();
}
void BlockTowerWidget::onRepairClicked()
{
    extern int health;
    extern int maxhealth;
    int delta = (int)(maxhealth * 0.3);
    health = qMin(health + delta, maxhealth);
    QMessageBox::information(this, "维修成功", QString("生命值已回复至 %1").arg(health));
}
void BlockTowerWidget::setMode(Mode mode)
{
    m_currentMode = mode;
    if (mode == NormalMode)
        m_synthesizeBtn->setText("合成（消耗2张）");
    else
        m_synthesizeBtn->setText("升级（消耗1张）");
}
void BlockTowerWidget::refreshTowerDisplay()
{
    QVector<QString> names;
    for (const BlockType &type : m_types)
        names.append(type.chinese);
    m_towerDisplay->setTowerData(m_towerBlocks, m_colorTable, names);
}
QJsonObject BlockTowerWidget::toJson() const {
    QJsonObject obj;
    QJsonArray typesArray;
    for (const auto& type : m_types) {
        QJsonObject typeObj;
        typeObj["name"] = type.name;
        typeObj["chinese"] = type.chinese;
        typeObj["color"] = type.color.name();
        typeObj["currentCount"] = type.currentCount;
        typesArray.append(typeObj);
    }
    obj["types"] = typesArray;

    QJsonArray blocksArray;
    for (const auto& block : m_towerBlocks) {
        QJsonObject blockObj;
        blockObj["typeIndex"] = block.typeIndex;
        blockObj["innerType"] = block.innerType;
        blocksArray.append(blockObj);
    }
    obj["towerBlocks"] = blocksArray;
    return obj;
}

void BlockTowerWidget::fromJson(const QJsonObject& obj) {
    m_types.clear();
    m_towerBlocks.clear();
    m_stockButtons.clear();
    m_colorTable.clear();
    QLayoutItem* item;
    while ((item = stockLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }
    QJsonArray typesArray = obj["types"].toArray();
    for (const auto& val : typesArray) {
        QJsonObject typeObj = val.toObject();
        BlockType type;
        type.name = typeObj["name"].toString();
        type.chinese = typeObj["chinese"].toString();
        type.color = QColor(typeObj["color"].toString());
        type.currentCount = typeObj["currentCount"].toInt();
        type.initialCount = type.currentCount; // 或单独保存
        m_types.append(type);
        m_colorTable.append(type.color);
    }
    QJsonArray blocksArray = obj["towerBlocks"].toArray();
    for (const auto& val : blocksArray) {
        QJsonObject blockObj = val.toObject();
        TowerBlock block;
        block.typeIndex = blockObj["typeIndex"].toInt();
        block.innerType = blockObj["innerType"].toInt();
        m_towerBlocks.append(block);
    }
    for (int i = 0; i < m_types.size(); ++i) {
        QPushButton* btn = new QPushButton;
        btn->setProperty("typeIndex", i);
        QPixmap pix(20,20);
        pix.fill(m_types[i].color);
        btn->setIcon(QIcon(pix));
        btn->setIconSize(QSize(20,20));
        btn->setText(QString("%1 x%2").arg(m_types[i].chinese).arg(m_types[i].currentCount));
        connect(btn, &QPushButton::clicked, this, &BlockTowerWidget::onStockButtonClicked);
        stockLayout->addWidget(btn);
        m_stockButtons.append(btn);
    }
    stockLayout->addStretch();

    refreshTowerDisplay();
    updateStockDisplay();
}
void BlockTowerWidget::reset(const QVector<BlockType>& newTypes)
{
    QLayoutItem* child;
    while ((child = stockLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    m_types.clear();
    m_towerBlocks.clear();
    m_stockButtons.clear();
    m_colorTable.clear();
    m_types = newTypes;
    for (const auto& t : m_types) {
        m_colorTable.append(t.color);
    }
    for (int i = 0; i < m_types.size(); ++i) {
        QPushButton* btn = new QPushButton;
        btn->setProperty("typeIndex", i);
        QPixmap pix(20,20);
        pix.fill(m_types[i].color);
        btn->setIcon(QIcon(pix));
        btn->setIconSize(QSize(20,20));
        btn->setText(QString("%1 x%2").arg(m_types[i].chinese).arg(m_types[i].currentCount));
        connect(btn, &QPushButton::clicked, this, &BlockTowerWidget::onStockButtonClicked);
        stockLayout->addWidget(btn);
        m_stockButtons.append(btn);
    }
    stockLayout->addStretch();
    refreshTowerDisplay();
    updateStockDisplay();
    m_selectedBlockIndex = -1;
    m_selectedStockType = -1;
    m_deleteMode = false;
    m_deleteModeBtn->setChecked(false);
    m_deleteModeBtn->setText("删除模式");
    m_towerDisplay->setInsertionEnabled(true);
    setMode(NormalMode);
}