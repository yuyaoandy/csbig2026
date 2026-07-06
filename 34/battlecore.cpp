#include "battlecore.h"
#include <cstring>
#include <random>
#include <algorithm>
#include <QRegularExpression>
using namespace std;

// 全局日志函数（默认输出到控制台，UI会覆盖）
void (*g_battleLog)(const string&) = [](const string& msg){ cout << msg << flush; };
void battleLog(const string& msg) { if(g_battleLog) g_battleLog(msg); }

mt19937 rnd2(0);

inline int select(const PlayBots &x, int type=0){
    vector<int> p;
    int sz = x.player.size();
    for(int i=0;i<sz;++i)
        if(x.player[i].alive())
            p.push_back(i);
    if(!p.size())return 0;
    int target=p.empty() ? 0 : p[rnd2()%p.size()];
    while (x.player[target].relic[99] &&rnd2()%2) {
        target=p[rnd2()%p.size()];
    }
    return target;
}

bool checkenergy(PlayBot& x, int ene){
    if(x.energy >= ene){
        x.energy -= ene;
        // 状态105：藤蔓缠绕，每消耗1点能量受到1点不可格挡伤害
        if(x.status[105] > 0 && ene > 0) {
            x.dealdamage(PlayBot("藤蔓"), ene, 1); // cantblock=1
        }
        return true;
    } else {
        battleLog("能量不足\n");
        return false;
    }
}
void getmove(PlayBots &x){
    for(auto &i : x){
        if(!i.alive()){
            i.exe = "";
            continue;
        }
        if(i.status[6] > 0) {
            i.exe = "";
            battleLog(i.name + " 因重生效果本回合无法行动\n");
            continue;
        }
        if(!i.restart){
            // 循环跳过标签 L 以及已达到次数限制的 Cgoto 指令
            while(i.pos < (int)i.code.size()){
                string &inst = i.code[i.pos];
                // 跳过标签
                if(inst[0]=='L'){
                    i.pos++;
                    continue;
                }
                // 检查是否是 Cgoto 且已达到次数限制
                if(inst.rfind("Cgoto", 0)==0){  // 以 "Cgoto" 开头
                    // 解析次数限制（与 attack 中一致）
                    string label;
                    int limit = INT_MAX;
                    QRegularExpression rxC("^Cgoto(\\d*)([A-Za-z]+)$");
                    QRegularExpressionMatch matchC = rxC.match(QString::fromStdString(inst));
                    if(matchC.hasMatch()){
                        label = matchC.captured(2).toStdString();
                        QString numStr = matchC.captured(1);
                        if(!numStr.isEmpty()) limit = numStr.toInt();
                    } else {
                        // inf
                        label = inst.substr(5);
                    }
                    // 检查使用次数
                    int used = i.Used[i.pos];
                    if(used >= limit){
                        // 已达到限制，跳过该指令
                        battleLog(i.name + " 跳转 " + label + " 已达次数限制，忽略此指令\n");
                        i.pos++;
                        continue;  // 继续检查下一条指令
                    }else {
                        i.Used[i.pos]+=1;
                    }
                }
                // 其他指令，跳出循环准备执行
                break;
            }

            if(i.pos >= (int)i.code.size()){
                battleLog(i.name + " 正在重启\n");
                i.restart = 2;
            } else {
                if(i.status[150])
                    i.status[150] -= 1, i.exe = "";
                else if(i.status[151] && i.code[i.pos][0]=='C')
                    i.status[151] -= 1, i.exe = "";
                else
                    i.exe = i.code[i.pos];
            }
        } else {
            battleLog(i.name + " 正在重启\n");
        }
    }
}
// 状态 151，跳过下次次“跳转”
// 状态 51，攻击2次
void attack(PlayBots &x, PlayBots &y){
    for(auto &i : x){
        string &mname = i.exe;
        if(mname.empty()) continue;
        //battleLog(i.name + " ::: " + mname + " " + to_string(i.status[104]) + "\n");
        QRegularExpression rx("(.*?)(\\d+)$");

        // 3. 执行匹配
        QRegularExpressionMatch match = rx.match(QString::fromStdString(mname));
        string prefix=mname;
        int level=0;
        // 4. 检查是否匹配成功并提取结果
        if (match.hasMatch()) {
            // 提取第一个捕获组：数字前的部分
            prefix = match.captured(1).toStdString();
            // 提取第二个捕获组：末尾的整数（仍为字符串形式）
            level=match.captured(2).toInt();
        }
        auto applyThornsAndLifesteal = [&](PlayBot& attacker, PlayBot& target, int actualDamage) {
            // 反伤 (遗物 12)
            if (target.relic[12] && attacker.alive()) {
                attacker.dealdamage(PlayBot("荆棘之心"), 2, 1); // 不可格挡
                battleLog("荆棘之心对 " + attacker.name + " 造成 2 点伤害\n");
            }
            // 吸血 (遗物 14)
            if (attacker.relic[14] && actualDamage > 0 && attacker.alive()) {
                int heal = (int)(actualDamage * 0.1);
                if (heal > 0) {
                    attacker.health = min(attacker.maxhealth, attacker.health + heal);
                    battleLog(attacker.name + " 因血刃恢复 " + to_string(heal) + " 点生命\n");
                }
            }
        };
        if(mname[0]=='C'){
            int pos=5;
            while(pos<(int)mname.length()&&isdigit(mname[pos]))
                pos+=1;
            string label = mname.substr(pos);
            for(size_t j=0; j<i.code.size(); ++j)
                if(label == i.code[j]){
                    i.pos = j;
                    break;
                }
        }else if(mname[0]=='A'){
            auto applyChargeShield = [&](PlayBot& bot) {
                if (bot.status[111] > 0) {
                    bot.addstatus(3, 1);
                    bot.status[111]--;
                    battleLog(bot.name + " 因充能护盾，额外获得 1 点护甲\n");
                }
            };
            int target;
            i.pos += 1;

            for(int j=0; j<1+i.status[51]; ++j){

                if(prefix == "Aattack"){
                    if(!checkenergy(i,1)) continue;

                    battleLog(i.name+"发动『攻击』"+"(等级为"+to_string(level)+")");
                    target = select(y,0);
                    int catt = i.calcattack(6 + level*2);
                    int actualDmg = y.player[target].dealdamage(i, catt);
                    applyThornsAndLifesteal(i, y.player[target], actualDmg);
                    applyChargeShield(i);
                } else if(prefix == "Aheavy" ){
                    if(!checkenergy(i,2)) continue;
                    battleLog(i.name+"发动『重击』"+"(等级为"+to_string(level)+")");
                    target = select(y,1);
                    int catt = i.calcattack(14 + level*4);
                    int actualDmg = y.player[target].dealdamage(i, catt);
                    applyThornsAndLifesteal(i, y.player[target], actualDmg);
                    applyChargeShield(i);
                } else if(prefix == "Aelbow"){
                    if(!checkenergy(i,2))continue;
                    battleLog(i.name+"发动『肘击』"+"(等级为"+to_string(level)+")");
                    target=select(y,1);
                    int catt=i.calcattack(i.status[3]);
                    int actualDmg = y.player[target].dealdamage(i, catt);
                    applyThornsAndLifesteal(i, y.player[target], actualDmg);
                    applyChargeShield(i);
                }else if(prefix == "Asnowball") {
                    if(!checkenergy(i,2)) continue;
                    battleLog(i.name+"发动『雪球』"+"(等级为"+to_string(level)+")");
                    bool repeat;
                    int death_count=0,lst_death_count=0;
                    do {
                        repeat = false;
                        // 对每个存活的敌人施加伤害（不可格挡）
                        for(auto &enemy : y.player){
                            if(enemy.alive()){
                                int dmg = i.calcattack(4 + level);
                                int actual = enemy.dealdamage(i, dmg, 1); // 不可格挡
                                applyThornsAndLifesteal(i, enemy, actual);
                            }
                        }
                        // 检查是否有敌人会因此死亡（预计算生命值）
                        for(auto &enemy : y.player){
                            if(enemy.alive()){
                                int pendingHealth = enemy.health;
                                for(auto &upd : enemy.updates){
                                    if(upd.first == "hea") pendingHealth -= upd.second;
                                }
                                if(pendingHealth<=0)
                                    death_count++;
                            }
                        }
                        repeat=death_count>lst_death_count;
                        lst_death_count=death_count;
                    } while(repeat);
                    applyChargeShield(i);
                }
                else if(prefix == "Alight") {
                    if(!checkenergy(i,1)) continue;
                    battleLog(i.name+"发动『耀眼光束』"+"(等级为"+to_string(level)+")");
                    int target = select(y,0);
                    int dmg = i.calcattack(3 + 2*level);
                    int actual = y.player[target].dealdamage(i, dmg);
                    applyThornsAndLifesteal(i, y.player[target], actual);
                    y.player[target].addstatus(102, 1 + level); // 虚弱，用原版102
                    applyChargeShield(i);
                }else if(prefix == "Aflame") {

                    if(!checkenergy(i,2)) continue;
                    battleLog(i.name+"发动『火焰斩』"+"(等级为"+to_string(level)+")");
                    int target = select(y,0);
                    int dmg = i.calcattack(5 + 3*level);
                    int actual = y.player[target].dealdamage(i, dmg);
                    applyThornsAndLifesteal(i, y.player[target], actual);
                    // 施加灼烧：持续2回合，每回合伤害 = level+1
                    y.player[target].addstatus(106, level+1);
                    y.player[target].addstatus(107, 2);
                    applyChargeShield(i);
                }
                else if(prefix == "Achain") {
                    if(!checkenergy(i,1)) continue;
                    int jumps = 2 + level;
                    int target = select(y,0);
                    // 先确保目标活着
                    if(!y.player[target].alive()) continue;
                    // 第一次伤害
                    int dmg = i.calcattack(3 + level);
                    jumps--;
                    int actual = y.player[target].dealdamage(i, dmg);
                    applyThornsAndLifesteal(i, y.player[target], actual);
                    battleLog(i.name+"发动『连锁闪电』"+"(等级为"+to_string(level)+")");
                    // 后续跳转
                    while(jumps > 0) {
                        // 找出所有活着的敌人（排除已死亡）
                        vector<int> alive_idx;
                        for(int j=0; j<(int)y.player.size(); ++j)
                            if(y.player[j].alive()) alive_idx.push_back(j);
                        if(alive_idx.empty()) break;
                        // 随机选择一个（可能和上次相同，允许）
                        int next = alive_idx[rnd2() % alive_idx.size()];

                        int actual = y.player[next].dealdamage(i,dmg);
                        applyThornsAndLifesteal(i, y.player[next], actual);
                        jumps--;
                    }
                    applyChargeShield(i);
                }else if(prefix == "Aflamestorm") {
                    if(!checkenergy(i,2)) continue;
                    battleLog(i.name + " 发动『烈焰风暴』(等级 " + to_string(level) + ")\n");
                    int dmg = i.calcattack(3 + 2*level);

                    // 对所有敌人造成伤害并施加灼烧
                    for(auto &enemy : y.player) {
                        if(enemy.alive()) {

                            int actual = enemy.dealdamage(i, dmg);
                            applyThornsAndLifesteal(i, enemy, actual);
                            enemy.addstatus(106, 2);   // 灼烧持续2回合
                            enemy.addstatus(107, 1);   // 每回合伤害1点
                        }
                    }
                    applyChargeShield(i);
                }

                // ----- 新增 Ashieldbash（护盾冲击）-----
                else if(prefix == "Ashieldbash") {
                    if(!checkenergy(i,1)) continue;
                    int armor = i.status[3];
                    if(armor <= 0) {
                        battleLog(i.name + " 没有护甲，『护盾冲击』无法造成伤害\n");
                        // 但依然消耗能量，设计为无伤害
                    } else {
                        int target = select(y,0);
                        int dmg = armor; // 至少1点
                        battleLog(i.name + " 发动『护盾冲击』，造成 " + to_string(dmg) + " 点伤害\n");
                        int actual=y.player[target].dealdamage(i, dmg);
                        applyThornsAndLifesteal(i, y.player[target], actual);
                        // 清除自身护甲
                        i.status[3] = 0;
                    }
                    applyChargeShield(i);
                }else if(prefix == "Abolt") {
                    if(!checkenergy(i,1)) continue;
                    battleLog(i.name+"发动『闪电箭』"+"(等级为"+to_string(level)+")");
                    int target = select(y,0);
                    int dmg = i.calcattack(4 + level * 2);
                    y.player[target].dealdamage(i, dmg);
                    // 减少目标1点能量（至少为0）
                    if(y.player[target].energy > 0) {
                        y.player[target].energy -= 1;
                        battleLog(" 并减少 " + y.player[target].name + " 1点能量\n");
                    } else {
                        battleLog(" 目标能量已为0，无法减少\n");
                    }
                    applyChargeShield(i);
                }else if(prefix == "Acleave") {
                    if(!checkenergy(i,2)) continue;
                    battleLog(i.name+"发动『横扫』"+"(等级为"+to_string(level)+")");
                    int dmg = i.calcattack(2 + level);
                    for(auto &enemy : y.player) {
                        if(enemy.alive()) {
                            int actual=enemy.dealdamage(i, dmg);
                            applyThornsAndLifesteal(i, enemy, actual);
                        }
                    }
                    applyChargeShield(i);
                }
            }
            if(i.status[51]) i.status[51] -= 1;
        } else if(mname[0]=='S'){
            i.pos += 1;
            if(prefix == "Ssleep"){
                i.gainenergy(2 + level);
                battleLog(i.name+"使用了『休息』，增加了"+to_string(2+level)+"点能量");
            }
            if(prefix=="Sdefend"){
                if(!checkenergy(i,1)) continue;
                battleLog(i.name+"使用了『举盾』，获得了"+to_string(5 + level*2)+"点护甲");
                i.addstatus(3, 5 + level*2);
            }
            if(prefix == "Sdouble" ){
                if(!checkenergy(i,1-level)) continue;
                i.addstatus(51,1);
            }
            if(prefix == "Spoison" ){

                if(!checkenergy(i,1)) continue;
                int target = select(y,0);
                battleLog(i.name+"对"+y.player[target].name+"使用了『毒药』，增加"+to_string(4+level)+"层中毒"+"(等级为"+to_string(level)+")");
                y.player[target].addstatus(104, 4 + level);
            }
            if(prefix == "Swarmup"){
                if(!checkenergy(i,1)) continue;
                i.addstatus(4, 3 + level);
                battleLog(i.name+"发动『活动肌肉』，增加"+to_string(3+level)+"层力量"+"(等级为"+to_string(level)+")");
            }
            if(prefix=="Sice"){
                if(!checkenergy(i,0))continue;
                i.gainenergy(3+level);
                battleLog(i.name+"发动『冰！』");
                i.dealdamage(i,4,1);

            }
            if(prefix == "Sprepare") {
                if(!checkenergy(i,0)) continue; // 能量消耗为0
                i.addstatus(5, 3 + level);      // 存到下回合用，状态5
                battleLog(i.name+"发动『作战准备』，下回合开始额外增加"+to_string(3+level)+"点能量"+"(等级为"+to_string(level)+")");
            }
            if(prefix == "Swine") {
                if(!checkenergy(i,1-level)) continue;
                i.addstatus(52, 1);             // 下次攻击翻倍，状态52
                battleLog(i.name+"发动『酒！』，下次攻击翻倍"+"(等级为"+to_string(level)+")");
            }
            if(prefix == "Svine") {
                cerr<<"????"<<endl;
                if(!checkenergy(i,1)) continue;
                cerr<<"!!!!"<<endl;
                int target = select(y,0);
                cerr<<"ok"<<target<<endl;
                battleLog(i.name+"发动『藤蔓缠绕』，赋予1层藤蔓缠绕标记，每次使用一点能量，受到一点伤害"+"(等级为"+to_string(level)+")");

                y.player[target].addstatus(105, 1); // 藤蔓缠绕标记，状态105
            }if(prefix == "Sshield") {
                if(!checkenergy(i,1)) continue;
                i.addstatus(3, 4 + 2*level);
                i.addstatus(7, 1);   // 下回合额外1能量
                battleLog(i.name+"发动『护盾充能』，增加"+to_string(4+2*level)+"点护盾，下回合开始额外获得1点能量"+"(等级为"+to_string(level)+")");
            }
            if(prefix == "Srevive") {
                if(!checkenergy(i,0)) continue;
                // 检查是否已触发过（status[6] == 0 且生命低于30%）
                if(i.status[6] == 0 && i.health < i.maxhealth * 0.3) {
                    int heal = 10 + 5*level;
                    i.health = min(i.maxhealth, i.health + heal);
                    // 清除所有负面状态（高位100+）
                    for(int st=100; st<200; ++st) i.status[st] = 0;
                    // 标记已重生，本回合不能再行动
                    i.status[6] = 1;
                    battleLog(i.name + " 触发重生，恢复 " + to_string(heal) + " 生命\n");
                } else {
                    battleLog("重生条件不满足或已触发\n");
                }
            }
            if(prefix == "Ssteal") {
                if(!checkenergy(i,1)) continue;
                int target = select(y,0);
                int steal = 2 + level;
                int actual = min(steal, y.player[target].energy); // 不能偷超过对方能量
                y.player[target].energy -= actual;
                i.energy += actual;
                battleLog(i.name + "发动『能量窃取』从 " + y.player[target].name + " 偷取 " + to_string(actual) + " 能量"+"(等级为"+to_string(level)+")");
            }
            if(prefix == "Sheal") {
                if(!checkenergy(i,1)) continue;
                int heal = 2 + level;  // 等级上限3，最多5点
                i.health = min(i.maxhealth, i.health + heal);
                battleLog(i.name + " 发动『治愈术』，恢复 " + to_string(heal) + " 点生命\n");
            }

            // ----- 新增 Soverload（能量透支）-----
            if(prefix == "Soverload") {
                if(!checkenergy(i,0)) continue; // 能量消耗为0
                int gain = 3 + level;
                i.gainenergy(gain);
                // 标记本回合结束后失去所有能量（状态108）
                i.addstatus(108, 1);
                battleLog(i.name + " 发动『能量透支』，获得 " + to_string(gain) + " 点能量，回合结束将清空能量\n");
            }

            // ----- 新增 Spurify（净化）-----
            if(prefix == "Spurify") {
                if(!checkenergy(i,1)) continue;
                int removed = 0;
                // 清除所有高位状态（100~199）
                for(int st=100; st<200; ++st) {
                    if(i.status[st] > 0) {
                        i.status[st] = 0;
                        removed++;
                    }
                }
                if(removed > 0) {
                    i.gainenergy(1);
                    battleLog(i.name + " 发动『净化』，移除 " + to_string(removed) + " 种负面状态，额外获得1点能量\n");
                } else {
                    battleLog(i.name + " 发动『净化』，但没有负面状态可移除\n");
                }
            }
            if(prefix == "Sfury") {
                if(!checkenergy(i,1)) continue;
                int powerGain = 2 + level;
                int energyGain = 1 + level;
                i.addstatus(4, powerGain);    // 力量
                i.gainenergy(energyGain);     // 能量
                battleLog(i.name+"发动『狂怒』，增加"+to_string(powerGain)+"点力量，获得"+to_string(energyGain)+"点能量"+"(等级为"+to_string(level)+")");
            }
            if (prefix == "Scharge") {
                if (!checkenergy(i, 1)) continue;
                int armor = 4 + level;
                i.addstatus(3, armor);
                // 设置额外护甲触发次数
                int extraCount = 2 + level;
                i.status[111] = extraCount;   // 剩余触发次数
                i.status[110] = 0;            // 已触发次数（可省略，仅用于调试）
                battleLog(i.name + " 发动『以攻为守』，获得 " + to_string(armor) + " 点护甲，本回合攻击可额外叠盾 " + to_string(extraCount) + " 次\n");
            }

            //if(mname==)
        }
    }
}

// 状态 3 是护甲,遗物 10 让每回合能保留一般护甲
// 状态 104 是中毒
// 状态 4 是力量
void endoturn(PlayBots& x){
    for(auto &i : x){
        i.status[110] = 0;
        i.status[111] = 0;
        if(i.restart){
            i.restart--, i.pos=0;
            if (i.restart == 0) i.Used.clear();
        }
        if(i.relic[10]){
            battleLog(i.name+"因能量护盾保留一半护甲");
            i.status[3] /= 2;
        } else i.status[3] = 0;
        if(i.relic[11]){
            i.status[4] = max(i.status[4]-2, 0);
        }
        if(i.status[104] > 0){
            i.dealdamage(PlayBot(), i.status[104], 2);
            i.status[104] -= 1;
        }
        i.status[6] = 0;
        // 如果灼烧持续回合已经为0，清除伤害值（已在上面回合开始时处理，但以防万一）
        if(i.status[106] == 0) i.status[107] = 0;
        if(i.status[108] > 0) {
            i.energy = 0;   // 失去所有剩余能量
            i.status[108] = 0;
            battleLog(i.name + " 因能量透支失去所有剩余能量\n");
        }
    }
}

void regain(PlayBots &x){
    for(auto &i : x){
        i.energy = max(i.energy, i.maxenergy);
    }
}
PlayBot::PlayBot(string nm){
    name = nm;
    memset(status, 0, sizeof(status));
    memset(relic, 0, sizeof(relic));
    energy = health = maxhealth = maxenergy = 0;
    restart = pos = 0;
    hasSplit = false;
}

bool PlayBot::alive() const { return health > 0; }

void PlayBot::change(string name, int amount){
    updates.push_back({name, amount});
}
// 状态 102 表示虚弱标记，减伤
// 状态 103 表示弱点标记，增伤
int PlayBot::calcattack(int x){
    int base = x + status[4];
    if(status[102]) {
        base = (int)(base * 0.7);
        status[102]--;   // Alight 施加的虚弱，每次攻击消耗一层
    }
    if(status[52]) {     // Swine 的酒效果
        base *= 2;
        status[52]--;
    }
    return base;
}

int PlayBot::dealdamage(const PlayBot &z, int x, int cantblock){
    if (status[109] > 0) {
        x = x / 2;
        if (x < 0) x = 0;
        status[109] = 0;
        battleLog(name + " 闪避了部分伤害，伤害减半\n");
    }
    if(status[103]) x = (int)(x * 1.3);
    int protect = min(status[3], x) * (!cantblock);
    change("s3", -protect);
    change("hea", x-protect);
    battleLog(z.name + " 对 " + name + "的护甲造成" + to_string(protect) +
              "点伤害，对" + name + "造成" + to_string(x-protect) + "点伤害\n");
    return x-protect;
}
// 遗物 8，获得>1点能量时额外再获得一点能量
void PlayBot::gainenergy(int x){
    change("ene", x);
    if(relic[8] && x>1)
        change("ene",1);
}

void PlayBot::update(){
    if(alive()){
        for(auto &i : updates){
            if(i.first == "ene"){
                energy = max(energy + i.second, 0);
            } else if(i.first[0]=='s'){
                int z = stoi(i.first.substr(1));
                status[z] += i.second;
                status[z] = max(status[z], 0);
            } else if(i.first == "hea"){
                health -= i.second;
            }
        }
    }
    updates.clear();
}
// 遗物 9，获得护甲时额外获得 2 点
void PlayBot::addstatus(int x, int val){
    string change_code = "s" + to_string(x);
    change(change_code, val);
    if(x==3 && relic[9])
        change("s3", 2),battleLog("因额外装甲，另获得2点护甲");
}

void PlayBot::loseenergy(int x){
    change("ene", -x);
}

void PlayBots::addBots(PlayBot x){
    player.push_back(x);
}

bool PlayBots::alive() const {
    for(auto &i : player)
        if(!i.alive()) return false;
    return true;
}

int PlayBots::getrelic(int relic_num) const {
    int total = 0;
    for(auto &i : player)
        if(i.relic[relic_num]) total++;
    return total;
}

// ----- Battle 状态机实现 -----
Battle::Battle(PlayBots& a, PlayBots& b) : state(ROUND_START),battlestatus(LOSE), teamA(a), teamB(b),  round(0), totalmove(0), currentAction(0) {}

bool Battle::teamAAlive() const {
    for(auto& p : teamA) if(p.alive()) return true;
    return false;
}
bool Battle::teamBAlive() const {
    for(auto& p : teamB) if(p.alive()) return true;
    return false;
}

void Battle::doRoundStart(){
    battleLog("新的一回合开始了！");
    vector<int> splitIndices;
    for (int i = 0; i < (int)teamB.player.size(); ++i) {
        auto& bot = teamB.player[i];
        if (bot.alive() && bot.name == "大史莱姆" && !bot.hasSplit && bot.health < bot.maxhealth / 2) {
            splitIndices.push_back(i);
        }
    }
    // 从后往前处理，避免索引错乱
    for (int idx = (int)splitIndices.size() - 1; idx >= 0; --idx) {
        int i = splitIndices[idx];
        // 创建两个小史莱姆（复制构造）
        PlayBot small1("小史莱姆"), small2("小史莱姆");
        small1.maxhealth = small1.health = 20;
        small1.maxenergy = small1.energy = 2;
        small1.code = {"Lstart","Aattack0","CgotoLstart"};
        small2 = small1;  // 复制

        // 标记原大史莱姆已分裂（之后会被移除）
        teamB.player[i].hasSplit = true;

        // 在 i 位置插入两个小史莱姆（顺序任意）
        teamB.player.insert(teamB.player.begin() + i, small1);
        teamB.player.insert(teamB.player.begin() + i + 1, small2);
        // 删除原大史莱姆（现在在 i+2 位置）
        teamB.player.erase(teamB.player.begin() + i + 2);

        battleLog("大史莱姆 分裂成两个小史莱姆！\n");
    }
    round++;
    regain(teamA);
    regain(teamB);
    for(auto& p : teamA) {
        if(p.alive() && p.status[5] > 0) {
            p.energy += p.status[5];
            p.status[5] = 0;
        }
    }
    for(auto& p : teamB) {
        if(p.alive() && p.status[5] > 0) {
            p.energy += p.status[5];
            p.status[5] = 0;
        }
    }
    for(auto& p : teamA) {
        if(p.alive() && p.status[7] > 0) {
            p.energy += p.status[7];
            p.status[7] = 0;
        }
    }
    for(auto& p : teamB) {
        if(p.alive() && p.status[7] > 0) {
            p.energy += p.status[7];
            p.status[7] = 0;
        }
    }
    for(auto& p : teamA) {
        if(p.alive() && p.status[106] > 0) {
            int burn_dmg = p.status[107];
            p.dealdamage(PlayBot("灼烧"), burn_dmg, 1); // 不可格挡
            p.status[106]--;
            if(p.status[106] == 0) p.status[107] = 0;
        }
    }
    for(auto& p : teamB) {
        if(p.alive() && p.status[106] > 0) {
            int burn_dmg = p.status[107];
            p.dealdamage(PlayBot("灼烧"), burn_dmg, 1); // 不可格挡
            p.status[106]--;
            if(p.status[106] == 0) p.status[107] = 0;
        }
    }
    totalmove = max(1, 10 + teamA.getrelic(4)*2 + teamB.getrelic(4)*2 - teamA.getrelic(5)*2 - teamB.getrelic(5)*2);
    currentAction = 1;
    state = ACTION;
}

void Battle::doAction(){
    // 清除上一轮的exe
    for(auto& p : teamA) p.exe = "";
    for(auto& p : teamB) p.exe = "";
    getmove(teamA);
    getmove(teamB);
    attack(teamA, teamB);
    attack(teamB, teamA);
    for(auto& p : teamA) p.update();
    for(auto& p : teamB) p.update();

    currentAction++;
    if(currentAction > totalmove || !teamAAlive() || !teamBAlive()){
        state = ROUND_END;
    }
}

void Battle::doRoundEnd(){
    endoturn(teamA);
    endoturn(teamB);
    for(auto& p : teamA) p.update();
    for(auto& p : teamB) p.update();
    if(teamAAlive() && teamBAlive())
        state = ROUND_START;
    else {
        state = FINISHED;
        if(teamAAlive()){
            battlestatus=WIN;
        }else{
            battlestatus=LOSE;
        }
    }
}

bool Battle::step(){
    if(state == FINISHED) return true;

    switch(state){
    case ROUND_START:
        doRoundStart();
        break;
    case ACTION:
        doAction();
        break;
    case ROUND_END:
        doRoundEnd();
        break;
    default: break;
    }
    return state == FINISHED;
}
