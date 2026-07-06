#ifndef BLOCKTOWERWIDGET_H
#define BLOCKTOWERWIDGET_H

#include <QWidget>
#include <QVector>
#include <QStringList>
#include "towerdisplaywidget.h"
#include "battlecore.h"
#include "QVBoxLayout"

class QPushButton;
class QScrollArea;
class TowerDisplayWidget;
struct BlockType {
    QString name;
    QString chinese;
    QColor  color;
    int     initialCount;
    int     currentCount;   // 当前库存可用数量
    QString Description;
    BlockType(QString name="",bool islabel=true);
    BlockType(QString nm,QString cn,QColor cl,int ic,int cc):name(nm),chinese(cn),color(cl),initialCount(ic),currentCount(cc){
    }
};// 实现从文件读取的功能（ok

struct You{
    std::vector<int> your_cards;
    PlayBot your_bot;
};

class BlockTowerWidget : public QWidget
{
    Q_OBJECT
public:
    // 模式枚举
    enum Mode { NormalMode, RepairMode };

    explicit BlockTowerWidget(const QVector<BlockType> &types, QWidget *parent = nullptr);
    ~BlockTowerWidget();

    // 模式切换
    void setMode(Mode mode);
    std::vector<std::string> getBlockNamesFromTop() const;
    void updateStockDisplay();
    void Addblock(QString st = "");
    void fromJson(const QJsonObject& obj);
    QJsonObject toJson() const;
    void reset(const QVector<BlockType>& newTypes);
signals:
    void goback();          // 正常模式返回
    void exitRepair();      // 维修模式返回地图（退出维修）

private slots:
    // 原有槽
    void onStockButtonClicked();
    void onInsertRequested(int insertIndex);
    void onDeleteToggled(bool checked);
    void onTowerBlockClicked(int index, bool innerClicked);
    void onClearClicked();
    void OnClickedreturn();   // 正常模式返回

    // 合成
    void onSynthesizeClicked();
    void onRepairClicked();      // 维修回血
    void onReturnMapClicked();   // 返回

private:
  //  void updateButtonsForMode();
    void refreshTowerDisplay();
    // 界面
    QVector<BlockType> m_types;
    QVector<QPushButton*> m_stockButtons;
    QVector<TowerBlock> m_towerBlocks;
    QVector<QColor> m_colorTable;
    TowerDisplayWidget *m_towerDisplay = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QVBoxLayout *stockLayout;


private:

    // 控制按钮
    QPushButton *m_synthesizeBtn = nullptr;   // 正常模式：合成

    QPushButton *m_deleteModeBtn = nullptr;   // 删除模式开关
    QPushButton *m_clearBtn = nullptr;
    QPushButton *m_returnBtn = nullptr;       // 正常模式返回
    QPushButton *m_removeBtn = nullptr;      // 正常模式：移除卡牌
    bool m_deleteMode = false;
    int m_selectedBlockIndex = -1;
    int m_selectedStockType = -1;

    void addBlockToTower(int typeIndex);
    void removeBlockFromTower(int index);
    void removeInnerBlock(int index);
    void updateStockButtonSelection();
};

#endif