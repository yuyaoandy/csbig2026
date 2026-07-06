#include "battlewindow.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include "CardSelectionWidget.h"
#include "blocktowerwidget.h"
#include "battlecore.h"
#include <QStackedWidget>
#include <QTextEdit>
#include <QVector>
#include "card.h"
#include <QRegularExpression>
extern QVector<Card> stored_card;
// 全局日志函数指针（在battlecore.cpp中定义）
extern int health;
extern void (*g_battleLog)(const string&);
extern You you;
extern mt19937 rnd;
extern int difficulty;
extern long long money_count;
BattleWidget* g_battlewidget = nullptr; // 全局主窗口指针，供日志回调使用
BattleWidget::BattleWidget(QWidget *parent)
    : QWidget(parent), battle(nullptr), autoTimer(nullptr), autoMode(false)
{
    g_battlewidget=this;
    setupUi();
    setupBattle();
    updateUI();
}

BattleWidget::~BattleWidget()
{
    delete battle;
    delete autoTimer;
}
static QString translateInstruction(const QString& line) {
    if (line.startsWith("L")) {
        return "标签: " + line.mid(1);
    }
    if (line.startsWith("Cgoto")) {
        int pos=5;
        qDebug()<<line<<'\n';
        while(pos<(int)line.size()&&line[pos]>='0' && line[pos]<='9')
            pos+=1;
        return "跳转到: " + translateInstruction(line.mid(pos));
    }
    static QRegularExpression re("([A-Za-z]+)(\\d*)$");
    QRegularExpressionMatch match = re.match(line);
    cerr<<line.toStdString()<<endl;
    if (match.hasMatch()) {
        QString cardName = match.captured(1);
        QString level = match.captured(2);
        cerr<<cardName.toStdString()<<" "<<level.toStdString()<<endl;
        // 在全局卡牌库中查找匹配的卡牌
        for (const Card& c : stored_card) {
            if (c.name == cardName) {
                return c.chinese + level;
            }
        }
        return line;
    }
    return line;
}

void BattleWidget::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    cerr<<"size:"<<teamA.player.size()<<endl;
    // 队伍显示区域（水平布局）
    QHBoxLayout* teamsLayout = new QHBoxLayout;

    // 创建滚动区域并设置属性
    scrollAreaA = new QScrollArea;
    scrollAreaB = new QScrollArea;
    scrollAreaA->setWidgetResizable(true);  // 让内部 widget 可调整大小
    scrollAreaB->setWidgetResizable(true);
    scrollAreaA->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollAreaB->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollAreaA->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollAreaB->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 创建 GroupBox 作为滚动区域的内部组件
    groupA = new QGroupBox("队伍 A");
    groupB = new QGroupBox("队伍 B");

    // 设置 GroupBox 的布局（稍后填充具体控件）
    layoutA = new QGridLayout(groupA);
    layoutB = new QGridLayout(groupB);

    // 将 GroupBox 设置到滚动区域
    scrollAreaA->setWidget(groupA);
    scrollAreaB->setWidget(groupB);

    // 将滚动区域添加到水平布局
    teamsLayout->addWidget(scrollAreaA);
    teamsLayout->addWidget(scrollAreaB);
    scrollAreaA->setMaximumHeight(300);
    scrollAreaB->setMaximumHeight(300);
    // 日志区域
    logEdit = new QTextEdit;
    logEdit->setReadOnly(true);
    //logEdit->setMaximumHeight(200);

    // 按钮区域
    QHBoxLayout* btnLayout = new QHBoxLayout;
    stepBtn = new QPushButton("单步执行");
    autoBtn = new QPushButton("自动战斗");
    resetBtn = new QPushButton("重置战斗");
    backBtn=new QPushButton("返回");
    btnLayout->addWidget(stepBtn);
    btnLayout->addWidget(autoBtn);
    btnLayout->addWidget(resetBtn);
    btnLayout->addWidget(backBtn);
    mainLayout->addLayout(teamsLayout);
    mainLayout->addWidget(logEdit);
    mainLayout->addLayout(btnLayout);

    // 连接信号
    connect(stepBtn, &QPushButton::clicked, this, &BattleWidget::onStepClicked);
    connect(autoBtn, &QPushButton::clicked, this, &BattleWidget::onAutoToggle);
    connect(resetBtn, &QPushButton::clicked, this, &BattleWidget::onResetClicked);
    connect(backBtn,&QPushButton::clicked,this,&BattleWidget::onBackClicked);
    // 注册日志回调
    g_battleLog = &BattleWidget::logCallback;
}

void BattleWidget::setupBattle(int nodeId, int encounterId, int nodeDifficulty) {
    // 1. 停止自动战斗
    cerr<<"fuck"<<nodeId<<" "<<encounterId<<endl;
    if (autoMode) {
        if (autoTimer) autoTimer->stop();
        autoMode = false;
        autoBtn->setText("自动战斗");
    }

    // 2. 清空日志和展开记录
    logEdit->clear();
    codeExpanded.clear();

    // 保存当前节点信息（用于重置）
    m_currentNodeId = nodeId;
    m_currentEncounterId = encounterId;

    // 3. 清空敌人队伍
    teamB.player.clear();

    // 4. 查找遭遇数据
    EncounterData* encData = nullptr;
    for (auto& enc : g_encounterList) {
        if (enc.id == encounterId) {
            encData = &enc;
            cerr<<enc.id<<" "<<enc.minDifficulty<<endl;
            break;
        }
    }
    if (!encData) {
        for (auto& enc : g_encounterList) {
            if (!enc.isBoss) {
                encData = &enc;
                break;
            }
        }
    }

    // 5. 构建敌人（根据遭遇数据或默认）
    if (encData) {
        int delta = nodeDifficulty - encData->minDifficulty;
        double healthMultiplier = 1.0 + delta * 0.05;   // 每点难度 +5% 生命
        if(encData->isBoss)healthMultiplier=max(healthMultiplier,1.0);
        int levelOffset = delta / 2;                    // 每2点难度 +1 等级
        if(nodeDifficulty<0)healthMultiplier=1;
        double extra=0.7+difficulty*0.06;
        healthMultiplier*=extra;
        for (int enemyId : encData->enemyIds) {
            EnemyData* enemyData = nullptr;
            for (auto& e : g_enemyList) {
                if (e.id == enemyId) {
                    enemyData = &e;
                    break;
                }
            }
            if (!enemyData) continue;

            int newHealth = (int)(enemyData->health * healthMultiplier);
            if (newHealth < 1)
                newHealth = 1;

            PlayBot enemyBot(enemyData->name);
            enemyBot.maxhealth = enemyBot.health = newHealth;
            enemyBot.maxenergy = enemyBot.energy = enemyData->energy;

            for (const string& cmd : enemyData->code) {
                QRegularExpression re("^(.*?)(\\d*)$");
                QRegularExpressionMatch match = re.match(QString::fromStdString(cmd));
                if (match.hasMatch()) {
                    string prefix = match.captured(1).toStdString();
                    int oldLevel = match.captured(2).toInt();
                    int newLevel = max(0, oldLevel + levelOffset);
                    enemyBot.code.push_back(prefix + to_string(newLevel));
                } else {
                    enemyBot.code.push_back(cmd);
                }
            }

            for (int r : enemyData->relic) {
                if (r >= 0 && r < 200) enemyBot.relic[r] = true;
            }
            teamB.addBots(enemyBot);
        }
    } else {
        PlayBot bot("Default");
        bot.maxhealth = bot.health = 50;
        bot.maxenergy = bot.energy = 4;
        bot.code = {"Lstart", "Aattack0", "Sdefend0", "CgotoLstart"};
        teamB.addBots(bot);
    }

    // 6. 重置玩家队伍（从全局变量恢复）
    if (!teamA.player.empty()) {
        auto& player = teamA.player[0];
        extern int health,maxhealth,max_energy;
        extern QVector<int> relic;
        player.health = health;
        player.maxhealth = maxhealth;
        player.maxenergy = max_energy;
        player.energy = max_energy;   // 战斗开始时能量回满

        // 清空状态
        memset(player.status, 0, sizeof(player.status));

        // 重置遗物
        memset(player.relic, 0, sizeof(player.relic));
        for (int r : relic) {
            if (r >= 0 && r < 200) player.relic[r] = true;
        }

        // 重置执行位置
        player.pos = 0;
        player.restart = 0;
        player.exe = "";
        player.Used.clear();
    }

    // 7. 创建或重建战斗
    if (battle) delete battle;
    battle = new Battle(teamA, teamB);

    // 8. 更新UI
    updateUI();
}

void BattleWidget::refreshDisplay()
{
    // 清空布局
    QLayoutItem* child;
    while ((child = layoutA->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    while ((child = layoutB->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // 清空标签容器（原有数据）
    hpLabels.clear();
    energyLabels.clear();
    armorLabels.clear();
    powerLabels.clear();
    poisonLabels.clear();
    hpBars.clear();

    // 辅助 lambda：添加一个机器人及其代码行
    auto addBot = [&](QGridLayout* layout, QGroupBox* groupBox, PlayBot& bot, const QString& teamPrefix, int idx) {
        QString id = teamPrefix + "_" + QString::number(idx);
        bool expanded = codeExpanded.value(id, false);
        int infoRow = idx * 2;
        int codeRow = infoRow + 1;

        // ----- 信息行 (原样) -----
        QLabel* nameLabel = new QLabel(QString::fromStdString(bot.name));
        QLabel* hpLabel = new QLabel(QString("血量: %1").arg(bot.health));
        QProgressBar* hpBar = new QProgressBar;
        hpBar->setRange(0, bot.maxhealth > 0 ? bot.maxhealth : 100);
        hpBar->setValue(bot.health);
        QLabel* energyLabel = new QLabel(QString("能量: %1/%2").arg(bot.energy).arg(bot.maxenergy));
        QLabel* armorLabel = new QLabel(QString("护甲: %1").arg(bot.status[3]));
        QLabel* powerLabel = new QLabel(QString("力量: %1").arg(bot.status[4]));
        QLabel* poisonLabel = new QLabel(QString("中毒: %1").arg(bot.status[104]));

        // 折叠按钮
        QPushButton* toggleBtn = new QPushButton(expanded ? "▲" : "▼");
        toggleBtn->setFixedWidth(30);
        toggleBtn->setToolTip("显示/隐藏代码");

        // 添加到布局
        layout->addWidget(nameLabel, infoRow, 0);
        layout->addWidget(hpLabel, infoRow, 1);
        layout->addWidget(hpBar, infoRow, 2);
        layout->addWidget(energyLabel, infoRow, 3);
        layout->addWidget(armorLabel, infoRow, 4);
        layout->addWidget(powerLabel, infoRow, 5);
        layout->addWidget(poisonLabel, infoRow, 6);
        layout->addWidget(toggleBtn, infoRow, 7);

        // 保存指针（用于后续更新）
        hpLabels.append(hpLabel);
        energyLabels.append(energyLabel);
        armorLabels.append(armorLabel);
        powerLabels.append(powerLabel);
        poisonLabels.append(poisonLabel);
        hpBars.append(hpBar);

        // ----- 代码显示区域（带高亮） -----
        // 明确指定父类为 groupBox，避免成为独立窗口
        QTextEdit* codeEdit = new QTextEdit(groupBox);
        codeEdit->setReadOnly(true);
        codeEdit->setMaximumHeight(120);
        codeEdit->setVisible(expanded);

        // 拼接代码文本
        QString codeText;
        int total=0;
        for (const string& line : bot.code) {

            QString Qline=QString::fromStdString(line);
            QString original = QString::fromStdString(line);
            QString translated = translateInstruction(original);
            if(Qline.startsWith("Cgoto")){
                QString numStr;
                int limit = INT_MAX;
                QRegularExpression rxC("^Cgoto(\\d*)([A-Za-z]+)$");
                QRegularExpressionMatch matchC = rxC.match(Qline);
                if(matchC.hasMatch()){
                    numStr = matchC.captured(1);
                    if(!numStr.isEmpty()) limit = numStr.toInt();
                } else {
                    // inf
                    numStr="999";
                }
                translated+=QString("(%1/%2)").arg(bot.Used[total]).arg(numStr);
            }total+=1;
            codeText += translated + "\n";
        }
        codeEdit->setPlainText(codeText);

        // ---- 高亮当前执行行 ----
        if (bot.pos >= 0 && bot.pos < (int)bot.code.size()) {
            QTextCursor cursor(codeEdit->document());
            cursor.movePosition(QTextCursor::Start);
            // 跳转到第 bot.pos 行（0-based）
            for (int i = 0; i < bot.pos; ++i) {
                cursor.movePosition(QTextCursor::NextBlock);
            }
            cursor.select(QTextCursor::LineUnderCursor);

            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(QColor(255, 255, 0, 100)); // 半透明黄色
            selection.cursor = cursor;

            QList<QTextEdit::ExtraSelection> selections;
            selections.append(selection);
            codeEdit->setExtraSelections(selections);

            // 可选：滚动到可见区域
            codeEdit->ensureCursorVisible();
        }

        // 将 codeEdit 添加到布局（跨 8 列）
        layout->addWidget(codeEdit, codeRow, 0, 1, 8);

        // ---- 折叠按钮信号 ----
        connect(toggleBtn, &QPushButton::clicked, [this, id, codeEdit, toggleBtn]() {
            bool now = codeExpanded.value(id, false);
            now = !now;
            codeExpanded[id] = now;
            codeEdit->setVisible(now);
            toggleBtn->setText(now ? "▲" : "▼");
            // 可选：强制更新布局
            codeEdit->parentWidget()->updateGeometry();
        });
    };

    // 遍历队伍 A
    for (int i = 0; i <(int) teamA.player.size(); ++i) {
        addBot(layoutA, groupA, teamA.player[i], "A", i);
    }
    // 遍历队伍 B
    for (int i = 0; i <(int) teamB.player.size(); ++i) {
        addBot(layoutB, groupB, teamB.player[i], "B", i);
    }
}

void BattleWidget::updateUI()
{
    refreshDisplay();
    // 更新按钮状态
    bool finished = battle->isFinished();
    stepBtn->setEnabled(!finished && !autoMode);
    if(finished) {
        if(autoTimer) autoTimer->stop();
        autoBtn->setText("自动战斗");
        autoMode = false;
        QString winner = teamA.alive() ? "队伍 A 获胜！" : "队伍 B 获胜！";
        appendLog(QString("战斗结束！%1").arg(winner));
    }
}

void BattleWidget::appendLog(const QString& msg)
{
    logEdit->append(msg);
}

void BattleWidget::logCallback(const std::string& msg)
{
    // 通过主窗口实例追加日志（需要获取实例指针）
    // 这里使用qApp->activeWindow() 不安全，最好使用单例或全局指针
    // 简单方法：在main.cpp中设置全局指针
    extern BattleWidget* g_battlewidget;
    if(g_battlewidget) {
        g_battlewidget->appendLog(QString::fromStdString(msg));
    }
}

void BattleWidget::onStepClicked()
{
    if(battle->isFinished()) return;
    bool finished = battle->step();
    updateUI();
    if(finished) {
        if(autoMode) onAutoToggle();
    }
}

void BattleWidget::onAutoToggle()
{
    if(autoMode) {
        // 停止自动战斗
        if(autoTimer) autoTimer->stop();
        autoBtn->setText("自动战斗");
        autoMode = false;
        stepBtn->setEnabled(!battle->isFinished());
    } else {
        // 开始自动战斗
        if(!autoTimer) {
            autoTimer = new QTimer(this);
            connect(autoTimer, &QTimer::timeout, this, &BattleWidget::onStepClicked);
        }
        autoTimer->start(500); // 每0.5秒一步
        autoBtn->setText("暂停");
        autoMode = true;
        stepBtn->setEnabled(false);
    }
}

void BattleWidget::onResetClicked() {
    if(autoMode) onAutoToggle();
    if (m_currentEncounterId != 0 || m_currentNodeId != 0) {
        setupBattle(m_currentNodeId, m_currentEncounterId);
    } else {
        // 保底：如果从未设置过，则默认使用第一个遭遇（或单敌）
        setupBattle(0, 0);
    }
    updateUI();
    appendLog("战斗已重置。");
}
void BattleWidget::onBackClicked() {
    bool finished = battle->isFinished();
    bool won = finished && (battle->battlestatus == Battle::WIN);
    emit battleFinished(won);
}
void BattleWidget::setTeams(const PlayBots& tmA,const PlayBots& tmB){
    teamA=tmA;
    teamB=tmB;
}
PlayBot BattleWidget::buildEnemyFromId(int enemyId) {
    // 1. 查找敌人数据
    EnemyData* enemyData = nullptr;
    for (auto& e : g_enemyList) {
        if (e.id == enemyId) {
            enemyData = &e;
            break;
        }
    }

    // 2. 若未找到，返回一个默认敌人（保底）
    if (!enemyData) {
        PlayBot defaultBot("Default");
        defaultBot.maxhealth = defaultBot.health = 50;
        defaultBot.maxenergy = defaultBot.energy = 4;
        defaultBot.code = {"Lstart", "Aattack0", "Sdefend0", "CgotoLstart"};
        return defaultBot;
    }

    // 3. 根据难度调整属性
    double healthMultiplier = 1.0 + difficulty * 0.1;   // 难度1~10，血量+10%~100%
    int newHealth = (int)(enemyData->health * healthMultiplier);
    int levelOffset = difficulty / 2;   // 每2级难度，卡牌等级+1

    // 4. 构建 PlayBot
    PlayBot enemyBot(enemyData->name);
    enemyBot.maxhealth = enemyBot.health = newHealth;
    enemyBot.maxenergy = enemyBot.energy = enemyData->energy;

    // 5. 处理代码并调整等级
    for (const string& cmd : enemyData->code) {
        QRegularExpression re("^(.*?)(\\d*)$");
        QRegularExpressionMatch match = re.match(QString::fromStdString(cmd));
        if (match.hasMatch()) {
            string prefix = match.captured(1).toStdString();
            int oldLevel = match.captured(2).toInt();
            int newLevel = max(0, oldLevel + levelOffset);
            enemyBot.code.push_back(prefix + to_string(newLevel));
        } else {
            enemyBot.code.push_back(cmd);
        }
    }

    // 6. 复制遗物
    for (int r : enemyData->relic) {
        if (r >= 0 && r < 200) enemyBot.relic[r] = true;
    }

    return enemyBot;
}
