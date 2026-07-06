// 游戏切换界面
#include"gamewindow.h"
#include "battlewindow.h"
#include<bits/stdc++.h>
#include"map_gen.h"
#include<qvector.h>
#include<QRegularExpression>
#include"card.h"
#include "saveloadmanager.h"
#include "CardSelectionWidget.h"
#include <QMessageBox>
#include "storewidget.h"
#include "storyloader.h"
#include "storywidget.h"
#include <QInputDialog>
#include<QToolButton>
#include "jsonreader.h"
#include<QTooltip>
#include<QPointer>
using namespace std;
extern int health;
extern int maxhealth;
extern QVector<int> relic;
extern long long money_count;
extern int next_node;
extern int difficulty;
extern const QVector<BlockType> initial;
extern mt19937 rnd;
Map_generator::Map gen(int depth = 10, int floor = 1 );
void SaveGame();
void ConWindow::onButtonClicked(){
    emit goback();
}
GameWindow::GameWindow(QWidget* parent) : QMainWindow(parent) {
    central = new QWidget(this);
    this->setCentralWidget(central);
    game_windows = new QStackedWidget(this);
    game_start_interface = new QWidget(this);
    map_page = new InGame(this);

    // ---------- 开始界面（保持不变） ----------
    startLayout = new QVBoxLayout(game_start_interface);
    startLayout->setAlignment(Qt::AlignCenter);
    startLayout->setSpacing(15);
    startLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/resources/images/logo.png");
    if (!logoPixmap.isNull()) {
        logoPixmap = logoPixmap.scaled(300, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        logoLabel->setPixmap(logoPixmap);
        logoLabel->setAlignment(Qt::AlignCenter);
    } else {
        logoLabel->setText("图标丢失");
        logoLabel->setAlignment(Qt::AlignCenter);
        logoLabel->setStyleSheet("font-size: 24px; color: #ecf0f1;");
    }
    startLayout->addWidget(logoLabel);

    game_start = new QPushButton("开始游戏", this);
    game_start->setStyleSheet(Button_Style::GAME_START_BTN);
    game_start->setFixedWidth(200);

    continueButton = new QPushButton("继续游戏", this);
    continueButton->setStyleSheet(
        Button_Style::GAME_START_BTN +
        "QPushButton:disabled {"
        "    background-color: #7f8c8d;"
        "    color: #bdc3c7;"
        "    border-color: #7f8c8d;"
        "}"
        );
    continueButton->setFixedWidth(200);
    continueButton->setEnabled(SaveLoadManager::hasSaveFile());

    show_contributor = new QPushButton("制作者", this);
    show_contributor->setStyleSheet(Button_Style::CONTRIBUTOR_BTN);
    show_contributor->setFixedWidth(200);

    settingsButton = new QPushButton("设置", this);
    settingsButton->setStyleSheet(Button_Style::CONTRIBUTOR_BTN);
    settingsButton->setFixedWidth(200);

    startLayout->addWidget(game_start);
    startLayout->addWidget(continueButton);
    startLayout->addWidget(show_contributor);
    startLayout->addWidget(settingsButton);
    startLayout->addStretch();

    // 设置界面--------------------
    settingsPage = new QWidget(this);
    settingsPage->setStyleSheet(
        "background-color: #1e2a36;"
        "border-radius: 15px;"
        );

    QVBoxLayout *settingsMainLayout = new QVBoxLayout(settingsPage);
    settingsMainLayout->setSpacing(20);
    settingsMainLayout->setContentsMargins(50, 50, 50, 50);

    QLabel *settingsTitle = new QLabel("设置", settingsPage);
    settingsTitle->setAlignment(Qt::AlignCenter);
    settingsTitle->setStyleSheet("font-size: 28px; color: #ecf0f1; font-weight: bold;");
    settingsMainLayout->addWidget(settingsTitle);

    settingsMainLayout->addStretch();


    QWidget *controlContainer = new QWidget(settingsPage);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlContainer);
    controlLayout->setAlignment(Qt::AlignCenter);
    controlLayout->setSpacing(25);

    // 难度调节：滑动条 + 数值标签
    QHBoxLayout *diffLayout = new QHBoxLayout();
    diffLayout->setAlignment(Qt::AlignCenter);
    diffLayout->setSpacing(15);

    QLabel *diffLabel = new QLabel("难度：", settingsPage);
    diffLabel->setStyleSheet("font-size: 18px; color: #ecf0f1;");

    difficultySlider = new QSlider(Qt::Horizontal, settingsPage);
    difficultySlider->setRange(1, 10);
    difficultySlider->setPageStep(1);
    difficultySlider->setValue(2);
    difficultySlider->setFixedWidth(250);// 难度
    difficultySlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    border: 1px solid #999999;"
        "    height: 8px;"
        "    background: #qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3498db, stop:1 #e74c3c);"
        "    border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: white;"
        "    border: 1px solid #5c5c5c;"
        "    width: 18px;"
        "    margin: -5px 0;"
        "    border-radius: 9px;"
        "}"
        );

    difficultyValueLabel = new QLabel("2", settingsPage);
    difficultyValueLabel->setStyleSheet("font-size: 20px; color: #ecf0f1; font-weight: bold;");
    difficultyValueLabel->setFixedWidth(30);
    difficultyValueLabel->setAlignment(Qt::AlignCenter);

    connect(difficultySlider, &QSlider::valueChanged, [this](int value) {
        difficultyValueLabel->setText(QString::number(value));
        extern int difficulty;
        difficulty = value;
    });// refresh

    diffLayout->addWidget(diffLabel);
    diffLayout->addWidget(difficultySlider);
    diffLayout->addWidget(difficultyValueLabel);
    controlLayout->addLayout(diffLayout);

    // 删dang
    deleteSaveButton = new QPushButton("删除存档", settingsPage);
    deleteSaveButton->setStyleSheet(
        "font-size: 18px; padding: 10px;"
        "background-color: #e74c3c; color: white; border-radius: 8px;"
        );
    deleteSaveButton->setFixedWidth(200);
    connect(deleteSaveButton, &QPushButton::clicked, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "确认删除",
            "确定要删除当前存档吗？此操作不可恢复！",
            QMessageBox::Yes | QMessageBox::No
            );
        if (reply == QMessageBox::Yes) {
            if (SaveLoadManager::deleteSaveFile()) {
                QMessageBox::information(this, "成功", "存档已删除");
                continueButton->setEnabled(SaveLoadManager::hasSaveFile());
            } else {
                QMessageBox::warning(this, "失败", "删除存档失败，请检查文件权限");
            }
        }
    });
    controlLayout->addWidget(deleteSaveButton, 0, Qt::AlignCenter);

    //return
    settingsBackButton = new QPushButton("返回", settingsPage);
    settingsBackButton->setStyleSheet(
        "font-size: 18px; padding: 10px;"
        "background-color: #3498db; color: white; border-radius: 8px;"
        );
    settingsBackButton->setFixedWidth(200);
    connect(settingsBackButton, &QPushButton::clicked, [this]() {
        game_windows->setCurrentIndex(0);
    });
    controlLayout->addWidget(settingsBackButton, 0, Qt::AlignCenter);
    settingsMainLayout->addWidget(controlContainer);
    settingsMainLayout->addStretch();
    //setting 结束--------------------
    con_interface = new ConWindow(this);
    game_windows->addWidget(game_start_interface);   // 0
    game_windows->addWidget(con_interface);          // 1
    game_windows->addWidget(map_page);               // 2
    game_windows->addWidget(settingsPage);           // 3
    game_windows->setCurrentIndex(0);

    mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(game_windows);

    connect(game_start, &QPushButton::clicked, [this]() {
        cerr << "use setMap" << endl;
        health = maxhealth = 100;
        money_count = 0;
        relic.clear();
        relic.append(9);
        // 开始前准备,选卡??..
        map_page->coding_interface->reset(initial);

        map_page->setMap(drawmap(gen(), map_page));
        game_windows->setCurrentIndex(2);
    });

    connect(continueButton, &QPushButton::clicked, [this]() {
        if (SaveLoadManager::loadGame(map_page)) {
            game_windows->setCurrentIndex(2);
        } else {
            QMessageBox::warning(this, "加载失败", "存档损坏或无法读取");
        }
    });

    // 同步
    connect(game_windows, &QStackedWidget::currentChanged, this, [this](int index) {
        if (index == 0) {
            continueButton->setEnabled(SaveLoadManager::hasSaveFile());
            extern int difficulty;
            int currentDiff = difficulty;
            if (difficultySlider->value() != currentDiff) {
                difficultySlider->setValue(currentDiff);
                difficultyValueLabel->setText(QString::number(currentDiff));
            }
        }
    });
    connect(game_start, &QPushButton::clicked, [this]() {
        health = maxhealth = 100;
        money_count = 0;
        relic.clear();
        relic.append(9);

        map_page->resetFloor();

        map_page->coding_interface->reset(initial);
        map_page->setMap(drawmap(gen(), map_page));
        game_windows->setCurrentIndex(2);
    });
    connect(show_contributor, &QPushButton::clicked, [this]() {
        game_windows->setCurrentIndex(1);
    });
    connect(con_interface, &ConWindow::goback, [this] {
        game_windows->setCurrentIndex(0);
    });

    connect(settingsButton, &QPushButton::clicked, [this]() {
        game_windows->setCurrentIndex(3);
        extern int difficulty;
        difficultySlider->setValue(difficulty);
        difficultyValueLabel->setText(QString::number(difficulty));
    });

    connect(map_page, &InGame::goback, [this] {
        game_windows->setCurrentIndex(0);
        health = maxhealth = 100;
        money_count = 0;
        relic.clear();
    });
}

void InGame::refresh(){
    QScrollArea* scroll = qobject_cast<QScrollArea*>(StackedPage->widget(6));
    if (scroll) {
        auto canvas = qobject_cast<Map_generator::MyCanvas*>(scroll->widget());
        if (canvas) {
            canvas->refresh();
        }
    }
    if (statusBar) {
        QPushButton* moneyBtn = statusBar->findChild<QPushButton*>("money_count");
        if (moneyBtn) {
            moneyBtn->setText(QString("%1").arg(money_count));
        }

        QLabel* healthLabel = statusBar->findChild<QLabel*>("health_label");
        if (healthLabel) {
            healthLabel->setText(QString("❤️ %1/%2").arg(health).arg(maxhealth));
            QFont font = healthLabel->font();
            font.setPointSize(14);
            healthLabel->setFont(font);
        }
    }
}
InGame::InGame(QWidget* parent):QWidget(parent){
    relicDescriptions = JsonReader::loadRelicDescriptions(":/resources/data/enemy.json");// 构造的时候读入
    QVector<EnemyData> enemyVec = JsonReader::loadEnemiesFromFile(":/resources/data/enemy.json");
    g_enemyList.clear();
    for (auto& e : enemyVec) g_enemyList.push_back(e);
    QVector<EncounterData> encVec = JsonReader::loadEncountersFromFile(":/resources/data/enemy.json");
    g_encounterList.clear();
    for (auto& e : encVec) g_encounterList.push_back(e);
    cerr<<"description size::"<<relicDescriptions.size()<<" "<<relicDescriptions[9].toStdString()<<endl;
    StackedPage=new QStackedWidget(this);
    battlepage=new BattleWidget(this);
    PlayBots tmp;
    PlayBot playerA("attack");
    playerA.health=playerA.maxhealth=100;
    playerA.energy=playerA.maxenergy=5;
    tmp.addBots(playerA);
    bck=edit=nullptr;
    battlepage->setTeams(tmp,PlayBots());// 先添加滚木
    coding_interface=new BlockTowerWidget(QVector<BlockType>(initial),this);
    map_interface=nullptr;
    cardPage = new CardSelectionWidget(StackedPage);
    connect(battlepage, &BattleWidget::battleFinished, this, &InGame::onBattleFinished);
    storyContainer = new QScrollArea(this);


    QWidget *resultPage = new QWidget(StackedPage);
    resultPage->setStyleSheet(
        "background-color: #1e2a36;"
        "border-radius: 15px;"
        "border: 1px solid #3d4a5a;"
        );

    QVBoxLayout *resultLayout = new QVBoxLayout(resultPage);
    resultLayout->setContentsMargins(70, 70, 70, 70);
    resultLayout->setSpacing(50);
    resultLayout->setAlignment(Qt::AlignCenter);

    QLabel *cardNameLabel = new QLabel("等待选卡...", resultPage);
    cardNameLabel->setAlignment(Qt::AlignCenter);
    cardNameLabel->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #ecf0f1;"
        "padding: 10px;"
        );

    QLabel *moneyLabel = new QLabel("", resultPage);
    moneyLabel->setAlignment(Qt::AlignCenter);
    moneyLabel->setStyleSheet(
        "font-size: 26px;"
        "color: #f1c40f;"
        "font-weight: bold;"
        "padding: 5px;"
        );

    QLabel *tipLabel = new QLabel("即将返回地图...", resultPage);
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setStyleSheet(
        "font-size: 16px;"
        "color: #95a5a6;"
        "padding-top: 10px;"
        );

    resultLayout->addWidget(cardNameLabel);
    resultLayout->addWidget(moneyLabel);
    resultLayout->addWidget(tipLabel);

    StackedPage->addWidget(resultPage);

    connect(cardPage, &CardSelectionWidget::selectionConfirmed,
            [this, cardNameLabel, moneyLabel, tipLabel](const QString &cardType, int pid, int gain) {
                if (cardType.isEmpty()) {
                    QMessageBox::warning(StackedPage, "提示", "请先选择一张卡片！");
                    return;
                }
             //   cerr<<"????"<<endl;

                cardNameLabel->setText(QString("选择成功"));
                moneyLabel->setText(QString("获得 <span style='color:#f1c40f;'>%1</span> 金钱").arg(gain));
                tipLabel->setText("即将返回地图...");

                // 更新积木和地图状态
                this->coding_interface->Addblock(cardType);
                int nodeId = next_node;

                // 切换到结果页面
                StackedPage->setCurrentIndex(3);
                QPointer<InGame> self = this;
                QTimer::singleShot(2500, this, [nodeId, self]() {
                    if(!self)return;
                    cerr<<"self is not null"<<endl;
                    if(self->map_interface){
                    //    cerr<<"map_interface is not null"<<nodeId<<endl;
                        static_cast<Map_generator::MyCanvas*>(self->map_interface->widget())->my_map.currentpos = nodeId;
                        static_cast<Map_generator::MyCanvas*>(self->map_interface->widget())->my_map.visited_node.push_back(nodeId);
                     //   cerr<<"ok"<<nodeId<<endl;
                        self->refresh();
                     //   cerr<<"isok"<<endl;
                        self->StackedPage->setCurrentIndex(6);
                     //   cerr<<"end"<<endl;
                    }
                });
             //   cerr<<"end2"<<endl;
            });
    storepage=new StoreWidget(this);
    StackedPage->addWidget(battlepage);
    StackedPage->setCurrentIndex(0);
    StackedPage->addWidget(coding_interface);
    StackedPage->addWidget(cardPage);
    StackedPage->addWidget(resultPage);
    StackedPage->addWidget(storepage);
    StackedPage->addWidget(storyContainer);
    QWidget* placeholder = new QWidget(this);
    StackedPage->addWidget(placeholder);
    repairpage = new RepairWidget(this);
    StackedPage->addWidget(repairpage);   // 索引7

    // 连接
    connect(repairpage, &RepairWidget::requestUpgrade, [this]() {
        coding_interface->setMode(BlockTowerWidget::RepairMode);
        // 切换到 coding_interface ）
        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.currentpos=next_node;
        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.visited_node.push_back(next_node);

        StackedPage->setCurrentIndex(1);
    });

    connect(repairpage, &RepairWidget::goback, [this]() {

        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.currentpos = next_node;
        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.visited_node.push_back(next_node);
        refresh();
        StackedPage->setCurrentIndex(6);   // 地图页面索引
    });
    connect(coding_interface, &BlockTowerWidget::goback, [this]() {
        coding_interface->setMode(BlockTowerWidget::NormalMode);/*
        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.currentpos = next_node;
        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.visited_node.push_back(next_node);*/
        refresh();
        StackedPage->setCurrentIndex(6);
    });
    connect(storepage, &StoreWidget::cardBought, this, [this](const QString &cardName, int price) {
        if (coding_interface) {
            cerr<<"????"<<cardName.toStdString()<<endl;
            coding_interface->Addblock(cardName);
        }
    });
    connect(storepage,&StoreWidget::goback,[this](int id){
        cerr<<id<<endl;
        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.currentpos=id;
        cerr<<"mycurrentpos is"<<id<<endl;
        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.visited_node.push_back(id);
        refresh();
        StackedPage->setCurrentIndex(6);
    });
    connect(coding_interface,&BlockTowerWidget::goback,[this]{
        coding_interface->setMode(BlockTowerWidget::NormalMode);
        auto it=coding_interface->getBlockNamesFromTop();
        battlepage->teamA.player[0].changecode(it);
        refresh();
        StackedPage->setCurrentIndex(6);
    });

}
void InGame::BackClicked(){
    //save recoding
    //----------to complete
    //SaveGame();
    // 废稿
    emit goback();
}
void InGame::setMap(QScrollArea* mymap)
{
    QWidget* repair = repairpage;
    if (map_interface) {
        int idx = StackedPage->indexOf(map_interface);
        if (idx != -1) {
            StackedPage->removeWidget(map_interface);
            delete map_interface;
        }
        map_interface = nullptr;
    }

    for (int i = StackedPage->count() - 1; i >= 0; --i) {
        QWidget* w = StackedPage->widget(i);
        if (w && w->objectName() == "placeholder") {
            StackedPage->removeWidget(w);
            delete w;
        }
    }

    if (repair && StackedPage->indexOf(repair) == -1) {
        StackedPage->addWidget(repair);
    }
    if (StackedPage->count() > 6) {
        QWidget* cur6 = StackedPage->widget(6);
        if (cur6 && cur6 != repair) {
            StackedPage->removeWidget(cur6);
            delete cur6;
        }
    }
    //    插入新地图
    map_interface = mymap;
    StackedPage->insertWidget(6, map_interface);
    if (repair) {
        int repIdx = StackedPage->indexOf(repair);
        if (repIdx == -1) {
            StackedPage->addWidget(repair);
            repIdx = StackedPage->indexOf(repair);
        }
        if (repIdx != 7) {
            StackedPage->removeWidget(repair);
            if (StackedPage->count() > 7) {
                QWidget* cur7 = StackedPage->widget(7);
                if (cur7 && cur7 != map_interface) {
                    StackedPage->removeWidget(cur7);
                    delete cur7;
                }
            }
            StackedPage->insertWidget(7, repair);
        }
    }
    cerr<<"????ok2    "<<StackedPage->count()<<endl;
    for (int i = StackedPage->count() - 1; i >= 0; --i) {
        QWidget* w = StackedPage->widget(i);
        if (w && w->objectName() == "placeholder") {
            StackedPage->removeWidget(w);
            delete w;
        }
    }
    //cerr<<"pppp"<<endl;
    statusBar = nullptr;
    statusLayout = nullptr;
    relicLayout = nullptr;
    relicContainer = nullptr;
    bck = nullptr;
    money = nullptr;
    edit = nullptr;
    //cerr<<"ok3"<<StackedPage->count()<<endl;
    QWidget *viewport = map_interface->viewport();
    if (!viewport) return;

    statusBar = new QWidget(viewport);
    statusBar->setObjectName("statusBar");
    statusBar->setStyleSheet("background-color: rgba(0,0,0,180); border-radius:0px; padding:0px;");
    statusBar->setFixedHeight(50);
    statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    statusBar->move(0, 0);
    statusBar->raise();

    statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(8, 5, 8, 5);
    statusLayout->setSpacing(5);

    // 左侧控件
    bck = new QPushButton("返回", statusBar);
    bck->setFixedWidth(60);
    bck->setStyleSheet("background: #3498db; color: white; border-radius: 4px;");
    connect(bck, &QPushButton::clicked, this, &InGame::BackClicked);

    money = new QPushButton(QString::number(money_count), statusBar);
    money->setObjectName("money_count");
    money->setFixedWidth(80);
    money->setStyleSheet("background: #f1c40f; color: black; border-radius: 4px;");

    QLabel* healthLabel = new QLabel(QString("❤️ %1/%2").arg(health).arg(maxhealth), statusBar);
    healthLabel->setObjectName("health_label");
    healthLabel->setFixedWidth(80);
    healthLabel->setStyleSheet("color: #ecf0f1; font-size: 14px;");

    statusLayout->addWidget(bck);
    statusLayout->addWidget(money);
    statusLayout->addWidget(healthLabel);
    statusLayout->addStretch(1);

    // 遗物滚动区域
    QScrollArea* relicScroll = new QScrollArea(statusBar);
    relicScroll->setObjectName("relicScroll");
    relicScroll->setWidgetResizable(false);
    relicScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    relicScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    relicScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    relicScroll->setStyleSheet("background: transparent; border: none;");
    relicScroll->setMinimumWidth(100);

    relicContainer = new QWidget();
    relicContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    relicLayout = new QHBoxLayout(relicContainer);
    relicLayout->setContentsMargins(0, 0, 0, 0);
    relicLayout->setSpacing(2);
    relicLayout->setSizeConstraint(QLayout::SetFixedSize);
    relicContainer->setLayout(relicLayout);
    relicContainer->setStyleSheet("padding: 0; margin: 0;");
    relicScroll->setWidget(relicContainer);

    statusLayout->addWidget(relicScroll, 2);

    // 右侧控件
    QPushButton* saveBtn = new QPushButton("保存", statusBar);
    saveBtn->setFixedWidth(60);
    saveBtn->setStyleSheet("background: #2ecc71; color: white; border-radius: 4px;");
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        SaveLoadManager::saveGame(this);
    });

    edit = new QPushButton("编辑机器人", statusBar);
    edit->setFixedWidth(90);
    edit->setStyleSheet("background: #9b59b6; color: white; border-radius: 4px;");
    connect(edit, &QPushButton::clicked, [this]() {
        coding_interface->updateStockDisplay();
        StackedPage->setCurrentIndex(1);
    });

    statusLayout->addWidget(saveBtn);
    statusLayout->addWidget(edit);
    updateRelicDisplay();
    QTimer::singleShot(0, this, [this]() {
        if (map_interface && statusBar) {
            QWidget* vp = map_interface->viewport();
            if (vp) {
                statusBar->setFixedWidth(vp->width());
                statusBar->update();
                if (statusLayout) {
                    statusLayout->invalidate();
                    statusLayout->activate();
                }
            }
        }
    });
    refresh();
    StackedPage->setCurrentIndex(6);
}
ConWindow::ConWindow(QWidget* parent):QWidget(parent){
    infoLabel= new QLabel("制作者：傅翔宇（代码），董思远（图像），韩博（设计）\n游戏版本 1.0",this);
    infoLabel->setAlignment(Qt::AlignCenter);
    conLayout  = new QVBoxLayout(this);
    backButton = new QPushButton("返回",this);
    conLayout->addWidget(infoLabel);
    conLayout->addWidget(backButton);
    conLayout->addStretch();
    QObject::connect(backButton, &QPushButton::clicked, this,&ConWindow::onButtonClicked);
}
void InGame::switchToNode(int nodeId) {
    if (!storyGraph.contains(nodeId)) {
        qWarning() << "节点" << nodeId << "不存在";
        return;
    }
    QWidget *oldWidget = storyContainer->takeWidget();
    if (oldWidget) oldWidget->deleteLater();

    StoryNodeWidget *newWidget = new StoryNodeWidget(storyGraph[nodeId], storyContainer);
    connect(newWidget, &StoryNodeWidget::nodeSelected, this, &InGame::switchToNode);
    connect(newWidget, &StoryNodeWidget::terminate, this, &InGame::onStoryTerminated);
    connect(newWidget, &StoryNodeWidget::effectsTriggered, this, &InGame::handleEffects);
    storyContainer->setWidget(newWidget);
    storyContainer->setFixedSize(800,600);//********
    //storyContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    StackedPage->setCurrentWidget(storyContainer);
}

bool InGame::loadStory(const QString &filePath) {
    QString error;
    storyGraph = StoryLoader::loadFromFile(filePath, &error);
    if (storyGraph.isEmpty()) {
        QMessageBox::warning(this, "剧情加载失败", error);
        return false;
    }
    return true;
}
void InGame::startStory(int startNodeId) {
    if (storyGraph.isEmpty()) {
        if (!loadStory(":/resources/data/story.json")) {
            qWarning() << "无法加载剧情，请检查文件";
            return;
        }
    }
    switchToNode(startNodeId);
}
void InGame::onStoryTerminated() {
    if (StackedPage->count() > 6) {
        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.currentpos=next_node;
        static_cast<Map_generator::MyCanvas*>(map_interface->widget())->my_map.visited_node.push_back(next_node);
        refresh();
        StackedPage->setCurrentIndex(6);
    }
}
RepairWidget::RepairWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    repairBtn = new QPushButton("维修 (回复30%生命)", this);
    repairBtn->setStyleSheet("font-size: 24px; padding: 20px; background-color: #2ecc71; border-radius: 10px;");
    layout->addWidget(repairBtn);

    upgradeBtn = new QPushButton("升级卡牌", this);
    upgradeBtn->setStyleSheet("font-size: 24px; padding: 20px; background-color: #3498db; border-radius: 10px;");
    layout->addWidget(upgradeBtn);

    backBtn = new QPushButton("返回", this);
    layout->addWidget(backBtn);

    connect(repairBtn, &QPushButton::clicked, this, &RepairWidget::onRepair);
    connect(upgradeBtn, &QPushButton::clicked, this, &RepairWidget::requestUpgrade);
    connect(backBtn, &QPushButton::clicked, this, &RepairWidget::goback);
}

void RepairWidget::onRepair()
{
    extern int health;
    extern int maxhealth;
    int delta = (int)(maxhealth * 0.3);
    health = qMin(health + delta, maxhealth);
    QMessageBox::information(this, "维修成功", QString("生命值已回复至 %1").arg(health));
    emit goback();
}
void InGame::switchToRepair(int nodeId)
{
    next_node = nodeId;
    std::cerr<<"pagecount"<<StackedPage->count()<<std::endl;
    StackedPage->setCurrentIndex(7);
}
void InGame::updateRelicDisplay() {
    if (!relicLayout) return;
    QLayoutItem *child;
    while ((child = relicLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    relicLabels.clear();

    const int iconSize = 20;
    const int padding = 4;

    int scrollHeight = iconSize + padding * 2 + 5;
    QScrollArea *relicScroll = statusBar->findChild<QScrollArea*>("relicScroll");
    if (relicScroll) {
        relicScroll->setFixedHeight(scrollHeight);
        if (relicContainer) {
            relicContainer->setFixedHeight(iconSize + padding);
        }
    }
    statusBar->setFixedHeight(scrollHeight + 14);

    for (int id : relic) {
        QString path = QString(":/resources/images/relic_%1.png").arg(id);
        QPixmap pixmap(path);
        if (pixmap.isNull()) {
            pixmap = QPixmap(iconSize, iconSize);
            pixmap.fill(Qt::gray);
        }
        QPixmap scaled = pixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QToolButton* btn = new QToolButton(relicContainer);
        btn->setIcon(QIcon(scaled));
        btn->setIconSize(QSize(iconSize, iconSize));
        btn->setFixedSize(iconSize, iconSize);
        btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        btn->setAutoRaise(true);
        btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
        btn->setStyleSheet("border: none; background: transparent;");
        QString desc = relicDescriptions.value(id, "未知遗物");
        qDebug() << "Setting tooltip for relic id" << id << "desc:" << desc;
        btn->setToolTip(desc);   // 设置 ToolTip
        btn->setAttribute(Qt::WA_AlwaysShowToolTips);
        relicLayout->addWidget(btn);
        relicLabels.append(btn);
    }
    relicContainer->updateGeometry();
    relicContainer->adjustSize();
    if (relicScroll) {
        relicScroll->updateGeometry();
    }
    if (statusLayout) {
        statusLayout->invalidate();
        statusLayout->activate();
    }/*
    QTimer::singleShot(1000, [this]() {
        if (!relicLabels.isEmpty()) {
            QPoint pos = relicLabels.first()->mapToGlobal(QPoint(10, 10));
            QToolTip::showText(pos, "测试ToolTip", relicLabels.first());
        }
    });*/
}

void InGame::resizeEvent(QResizeEvent *event) {
    if (map_interface) {
        map_interface->setGeometry(0, 0, width(), height());
        StackedPage->setGeometry(0, 0, width(), height());

        if (statusBar) {
            QWidget *viewport = map_interface->viewport();
            if (viewport) {
                statusBar->setFixedWidth(viewport->width());
                if (statusLayout) {
                    statusLayout->invalidate();
                    statusLayout->activate();
                }
            }
        }
    }
    QWidget::resizeEvent(event);
}
void GameWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    if (continueButton) {
     //   continueButton->setEnabled(SaveLoadManager::hasSaveFile());
    }
}
static vector<int> avarelic={9,10,12,13,14};
void InGame::handleEffects(const QList<StoryEffect>& effects) {
    extern int health;
    extern int maxhealth;
    extern long long money_count;
    extern QVector<int> relic;

    for (const StoryEffect &eff : effects) {
        if (eff.type == "addHealth") {
            health = qBound(0, health + eff.value, maxhealth);
        } else if (eff.type == "addMoney") {
            money_count = qMax(0LL, money_count + eff.value);
        } else if (eff.type == "addRelic") {
            int nvalue=avarelic[rnd()%avarelic.size()];
            relic.append(nvalue);
            updateRelicDisplay();
        } else if (eff.type == "startBattle") {
            cerr<<"!!!"<<endl;
            if (battlepage) {
                battlepage->setupBattle(0, eff.value, -1);
                battlepage->updateUI();
                battlepage->refreshDisplay();

                StackedPage->setGeometry(this->rect());
                battlepage->setGeometry(StackedPage->rect());

                QTimer::singleShot(0, [this]() {
                    StackedPage->setCurrentWidget(battlepage);
                    battlepage->show();
                    battlepage->raise();
                    battlepage->setAutoFillBackground(true);
                    QPalette pal = battlepage->palette();
                    battlepage->setPalette(pal);
                    battlepage->update();
                    StackedPage->update();
                    qDebug() << "Delayed battlepage geometry:" << battlepage->geometry();
                });
            }
        } else if (eff.type == "upgradeCard") {
            if (coding_interface) {
                coding_interface->setMode(BlockTowerWidget::RepairMode);
                StackedPage->setCurrentIndex(1);
            }
        }else if(eff.type=="addMaxHealth"){
            health+=eff.value;
            maxhealth+=eff.value;
        }else if(eff.type=="randomHealth"){
            health = qBound(0, int(health + rnd()%(2*eff.value+1)-eff.value), maxhealth);
        }
    }
    refresh();
}//剧情
void InGame::onBattleFinished(bool won) {
    if (!won) {
        refresh();
        StackedPage->setCurrentIndex(6);
        return;
    }
    health = battlepage->teamA.player.front().health;
    if (m_isBossBattle) {
        proceedToNextFloor();
    } else {
        int gain = 50 + rnd() % 30;
        money_count += gain;
        if (cardPage) {
            cardPage->setupUI(m_currentNodeId, gain);
            StackedPage->setCurrentIndex(2);
        } else {
            refresh();
            StackedPage->setCurrentIndex(6);
        }
    }
}

void InGame::proceedToNextFloor() {
    currentFloor++;
    Map_generator::Map newMap = gen(10, currentFloor);
    QScrollArea* newMapWidget = drawmap(newMap, this);
    setMap(newMapWidget);
    refresh();
}