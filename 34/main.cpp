#include <QApplication>
#include "gamewindow.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include "style.h"
#include "battlewindow.h"
#include "jsonreader.h"
extern const QVector<BlockType> initial;
QVector<int> relic;
int health=100;
int maxhealth = 100;
int difficulty=5;
int max_energy=5;
int floors=1;
std::vector<EnemyData> g_enemyList;
std::vector<EncounterData> g_encounterList;
const QVector<BlockType> initial = {
    {"Cgoto2", "跳转2"   ,QColor(255, 0, 0),    5, 5},
    {"La", "标签a" ,QColor(0, 255, 0),    4, 4},
    {"Aattack1", "攻击1" , QColor(0, 0, 255),    6, 6},
    {"Ssleep1", "休息1",QColor(255, 255, 0),  3, 3}
};
extern You you;
int next_node;
You you;
long long money_count=0;
extern QPixmap moneylabel;
extern BattleWidget* g_battlewidget;
QVector<Card> stored_card;
int main(int argc, char *argv[])
{
  //  relic.append({9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9});//a'滚动条测试

    stored_card = JsonReader::loadCardsFromFile(":/resources/data/enemy.json");
    cerr<<":::::"<<stored_card.size()<<endl;
    QApplication a(argc, argv);
    GameWindow window;
    a.setWindowIcon(QIcon(":/resources/images/icon.png"));
    window.setWindowTitle("波特塔");
    window.resize(1024, 768);
    window.show();
    return a.exec();
}
