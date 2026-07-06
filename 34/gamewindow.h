#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H
#include<bits/stdc++.h>
#include<QMainWindow>
#include<QStackedWidget>
#include<QWidget>
#include<QVBoxLayout>
#include<QPushButton>
#include<style.h>
#include<QLabel>
#include"map_gen.h"
#include"battlewindow.h"
#include"blocktowerwidget.h"
#include"storewidget.h"
#include"CardSelectionWidget.h"
#include "storywidget.h"
#include<QToolButton>
#include<QComboBox>
#include"map_gen.h"
class ConWindow:public QWidget{
    Q_OBJECT
public:
    ConWindow(QWidget* parent);
    void onButtonClicked();
    QStackedWidget StackedPage;

signals:
    void goback();
private:
    QLabel *infoLabel;
    QVBoxLayout *conLayout;
    QPushButton *backButton;
    BattleWidget* battlePage;

};

class RepairWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RepairWidget(QWidget* parent = nullptr);   // 正确
signals:
    void goback();
    void requestUpgrade();
private slots:
    void onRepair();
private:
    QPushButton *repairBtn;
    QPushButton *upgradeBtn;
    QPushButton *backBtn;
};

// ... 其他类
class InGame:public QWidget{
    Q_OBJECT
public:
    void switchToRepair(int nodeId);
    InGame(QWidget *parent=nullptr);
    void setMap(QScrollArea* mymap);
    QStackedWidget *StackedPage;
    void BackClicked();
    bool loadStory(const QString &filePath);
    void resizeEvent(QResizeEvent *event) ;
    void resetbattle(int nodeId) {
        auto* canvas = qobject_cast<Map_generator::MyCanvas*>(map_interface->widget());
        if (!canvas) return;
        m_currentNodeId = nodeId;
        const auto& mapData = canvas->my_map;
        m_isBossBattle = (mapData.nodes[nodeId]->depth == mapData.dep + 1);
        if (nodeId < 0 || nodeId >= (int)mapData.nodes.size()) return;
        int encounterId = mapData.nodes[nodeId]->encounterId;
        int nodeDifficulty = mapData.nodes[nodeId]->difficulty;
        cerr<<nodeId<<" "<<nodeDifficulty<<" "<<encounterId<<endl;
        battlepage->setupBattle(nodeId, encounterId, nodeDifficulty);
        battlepage->updateUI();
        battlepage->refreshDisplay();
        StackedPage->setCurrentIndex(0);
    }
    void resetstore(int id=0){
        storepage->id=id;
        storepage->rebuild();
    }
    void resetFloor() { currentFloor = 1; }
    void refresh();
    void startStory(int id=0);        // 返回地图（由回血或返回触发）
    BlockTowerWidget *coding_interface;
    QScrollArea *map_interface;
    QList<QToolButton*> relicLabels;          // 存储配件图片标签
    void updateRelicDisplay();
    QPushButton *moneyBtn = nullptr;
    QLabel *healthLabel = nullptr;
    QScrollArea *relicScroll;
    QHBoxLayout *statusLayout;
    QWidget *rightContainer;
    QHBoxLayout *rightLayout;
    QMap<int, QString> relicDescriptions;
    int currentFloor = 1;
private slots:
    void handleEffects(const QList<StoryEffect>& effects);
signals:
    void goback();
    void battleFinished(bool won);
private:
    RepairWidget* repairpage = nullptr;

    QPushButton *bck;
    BattleWidget *battlepage;

    QWidget* statusBar = nullptr;          // 状态栏容器
    QWidget* relicContainer = nullptr;     // 存放 relic 图标的容器
    QHBoxLayout* relicLayout = nullptr;    //
    QPushButton *edit;
    QPushButton *money;
    StoreWidget *storepage;
    QScrollArea *storyContainer;               // 滚动容器，用于容纳节点 Widget
    QMap<int, StoryNode> storyGraph;           // 存储所有节点
    void switchToNode(int nodeId);
    void onStoryTerminated();
  //  CardSelectionWidget cardPag
    bool m_isBossBattle = false;
    int m_currentNodeId = -1;
    CardSelectionWidget* cardPage = nullptr;

private slots:
    void onBattleFinished(bool won);          // 战斗结束处理
    void proceedToNextFloor();                // 进入下一层


};
class GameWindow : public QMainWindow {
    Q_OBJECT
    // ...
public:
    GameWindow(QWidget* parent=nullptr);
 //   ~GameWindow();
    InGame* map_page;
    void showEvent(QShowEvent *event);
private:
    QStackedWidget* game_windows;

    QVBoxLayout* startLayout;
    QWidget* central;
    QWidget * game_start_interface;

    ConWindow *con_interface;
    QPushButton *game_start;
    QPushButton *show_contributor;
    QVBoxLayout *mainLayout;
    QPushButton *continueButton;
    QWidget* settingsPage;
    QPushButton* settingsButton;
    QSlider* difficultySlider;
    QPushButton* deleteSaveButton;
    QPushButton* settingsBackButton;
    QLabel* difficultyValueLabel;

    // 其他页面指针
};


#endif // GAMEWINDOW_H
