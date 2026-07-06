#ifndef BATTLECORE_H
#define BATTLECORE_H

#include <bits/stdc++.h>
using namespace std;
extern void battleLog(const string& msg);
extern int health;
struct EncounterData {
    int id;
    std::string name;
    std::vector<int> enemyIds;      // 敌人 ID 列表
    int minDifficulty = 1;
    int maxDifficulty = 10;
    int minDepth = 1;
    int maxDepth = 10;
    bool isBoss = false;
};
struct PlayBot {
    vector<string> code;
    vector<pair<string,int>> updates;
    unordered_map<int, int> Used;
    int health, energy;
    int maxhealth, maxenergy;
    bool hasSplit;
    int status[200];
    bool relic[200];
    int restart;
    int pos;
    string name;
    string exe;
    void changecode(const vector<string>& p){
        code=p;
    }
    PlayBot (string nm="testbot");
    bool alive() const;
    void change(string name, int amount);
    int calcattack(int x);
    int dealdamage(const PlayBot &z, int x, int cantblock=0);
    void gainenergy(int x);
    void update();
    void addstatus(int x, int val);
    void loseenergy(int x);
    void resetUesdCounts() { Used.clear(); }
};

struct PlayBots {
    vector<PlayBot> player;
    void addBots(PlayBot x);
    bool alive() const;
    int getrelic(int relic_num) const;
    auto begin() { return player.begin(); }
    auto end() { return player.end(); }
    auto begin() const { return player.begin(); }
    auto end() const { return player.end(); }
};

// 战斗状态机
class Battle {
public:
    enum State { ROUND_START, ACTION, ROUND_END, FINISHED };
    enum Battlestatus {WIN,LOSE};
    Battle(PlayBots& a, PlayBots& b);
    bool step();               // 执行一步，返回true表示战斗结束
    bool isFinished() const { return state == FINISHED; }
    const PlayBots& getTeamA() const { return teamA; }
    const PlayBots& getTeamB() const { return teamB; }
    State state;
    Battlestatus battlestatus;
private:
    PlayBots& teamA;
    PlayBots& teamB;

    int round;
    int totalmove;
    int currentAction;
    bool teamAAlive() const;
    bool teamBAlive() const;

    void doRoundStart();
    void doAction();
    void doRoundEnd();
};
struct EnemyData {
    int id;
    std::string name;
    int health;
    int energy;
    std::vector<std::string> code;
    std::vector<int> relic;   // 遗物 ID 列表
};

extern std::vector<EnemyData> g_enemyList;   // 全局敌人数据
extern std::vector<EncounterData> g_encounterList;
extern int difficulty;
#endif // BATTLECORE_H