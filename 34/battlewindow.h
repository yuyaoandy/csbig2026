#ifndef BATTLEWINDOW_H
#define BATTLEWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "battlecore.h"
#include <QScrollArea>
#include "battlecore.h"

QT_BEGIN_NAMESPACE
class QTextEdit;
class QPushButton;
class QProgressBar;
class QLabel;
class QGridLayout;
class QGroupBox;
QT_END_NAMESPACE

class BattleWidget : public QWidget
{
    Q_OBJECT

public:
    BattleWidget(QWidget *parent = nullptr);
    ~BattleWidget();
    void setTeams(const PlayBots& teamA,const PlayBots& teamB);
    void setupBattle(int nodeId=0, int encounterId=0, int nodeDifficulty=0);
    void refreshDisplay();
    void updateUI();
    PlayBots teamA, teamB;
    QMap<QString, bool> codeExpanded;
signals:
    void battleFinished(bool won);
    void card(int id=0,int gain=0);
private slots:
    void onStepClicked();
    void onAutoToggle();
    void onResetClicked();

    void onBackClicked();
private:
    int m_currentNodeId = 0;
    int m_currentEncounterId = 0;
    PlayBot  buildEnemyFromId(int enemyId);
    void setupUi();
    void appendLog(const QString& msg);
    static void logCallback(const std::string& msg); // 静态回调

    // 应该通过某种方式传进来
    Battle* battle;
    QTimer* autoTimer;
    bool autoMode;
    int battleid;
    QGroupBox* groupA;
    QGroupBox* groupB;
    QGridLayout* layoutA;
    QGridLayout* layoutB;
    QVector<QLabel*> hpLabels, energyLabels, armorLabels, powerLabels, poisonLabels;
    QVector<QProgressBar*> hpBars;
    QTextEdit* logEdit;
    QPushButton* stepBtn;
    QPushButton* autoBtn;
    QPushButton* resetBtn;
    QPushButton* backBtn;
    QScrollArea* scrollAreaA;
    QScrollArea* scrollAreaB;
};

#endif