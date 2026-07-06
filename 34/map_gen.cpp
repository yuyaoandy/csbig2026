#include"map_gen.h"
#include<bits/stdc++.h>
#include"gamewindow.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
using namespace std;
using namespace Map_generator;
mt19937 rnd(0);
extern int health;
extern int maxhealth;
extern int next_node;
extern long long money_count;
extern int floors;
static vector<int> getEncounterCandidates(bool isBoss) {
    vector<int> result;
    for (int i = 0; i < (int)g_encounterList.size(); ++i) {
        if (g_encounterList[i].isBoss == isBoss) {
            result.push_back(i);
        }
    }
    return result;
}
Map gen(int depth = 10, int floor = 1) {
    Map x(depth, rnd());
    x.nodes.push_back(make_shared<Map_Node>());
    int nodecount = 1, lstnode = 1;

    for (int di = 1; di < depth; ++di) {
        int currentnode = 0;
        int p[10] = {}, q[10] = {};
        do {
            for (int i = 0; i < lstnode; ++i) {
                p[i] = rnd() % 2 + 1 + (rnd() % 5 == 0);
                q[i] = (!i ? 0 : p[i - 1] + rnd() % 2);
                p[i] += q[i] - 1;
            }
            currentnode = p[lstnode - 1] + 1;
        } while (currentnode > 6 || abs(currentnode - lstnode) > 2 ||
                 (di >= depth - 1 && currentnode > 5) ||
                 (rnd() % 3 && currentnode == lstnode) ||
                 (rnd() % 2 && currentnode == 3));

        for (int i = nodecount - lstnode, pos = 0; i < nodecount; ++i, ++pos) {
            for (int j = nodecount + q[pos]; j <= nodecount + p[pos]; ++j) {
                x.nodes[i]->nxt.push_back(j);
            }
        }

        for (int i = nodecount; i < nodecount + currentnode; ++i) {
            int type = rnd() % 10;
            if (type >= 5) type = 1;
            else if (type >= 3) type = 2;
            else if (type >= 2) type = 0;
            else type = 3;
            //type=2;

            int nodeDifficulty = floor * 10 + di;
            int encId = 0;
            if (type == 1 && !g_encounterList.empty()) {
                int nodeDifficulty = floor * 10 + di;
                vector<int> candidates;
                for (int idx = 0; idx < (int)g_encounterList.size(); ++idx) {
                    const auto& enc = g_encounterList[idx];
                    if (enc.isBoss) continue;
                    if (enc.minDifficulty <= nodeDifficulty) {
                        candidates.push_back(idx);
                    }
                }
                if (candidates.empty()) {
                    for (int idx = 0; idx < (int)g_encounterList.size(); ++idx) {
                        if (!g_encounterList[idx].isBoss) candidates.push_back(idx);
                    }
                }
                if (!candidates.empty()) {
                    int sel = candidates[rnd() % candidates.size()];
                    encId = g_encounterList[sel].id;
                }
            }
            x.nodes.push_back(make_shared<Map_Node>(type, di, i, encId, nodeDifficulty));
        }
        lstnode = currentnode;
        nodecount += currentnode;
    }
    for (int i = nodecount - lstnode; i < nodecount; ++i)
        x.nodes[i]->nxt.push_back(nodecount);

    // Boss
    int bossId = 0;
    if (!g_encounterList.empty()) {
        int bossDifficulty = floor * 10 + depth;
        vector<int> candidates;
        for (int idx = 0; idx < (int)g_encounterList.size(); ++idx) {
            const auto& enc = g_encounterList[idx];
            if (!enc.isBoss) continue;
            if (enc.minDifficulty <= bossDifficulty) {
                candidates.push_back(idx);
            }
        }
        if (candidates.empty()) {
            for (int idx = 0; idx < (int)g_encounterList.size(); ++idx) {
                if (g_encounterList[idx].isBoss) candidates.push_back(idx);
            }
        }
        if (!candidates.empty()) {
            int sel = candidates[rnd() % candidates.size()];
            bossId = g_encounterList[sel].id;
        }
    }
    int bossDifficulty = floor * 10 + depth;
    x.nodes.push_back(make_shared<Map_Node>(1, depth + 1, nodecount, bossId, bossDifficulty));

    x.currentpos = 0;
    return x;
}
static vector<int> availableStoryIds = {0,3,4,5,6,7,12,14,16};
QScrollArea* drawmap(const Map& x,QWidget *parent=nullptr){
    QScrollArea *scroll_map=new QScrollArea(parent);

 //   cerr<<"!!!!"<<x.nodes.size()<<endl;
    InGame *game_windows=static_cast<InGame*>(parent);
    MyCanvas *canvas=new MyCanvas(parent,x);

    canvas->update();
    int dep=x.dep;
    int height=2000,width=700;
    canvas->setFixedSize(width,height);
    vector<Map_Node*> lst;
    int total=0;
    for(int i=0;i<=dep+1;++i){
        vector<shared_ptr<Map_Node>> p;
        for(auto j:x.nodes){
            if(j->depth==i)
                p.push_back(j);
        }
        int cnt=0;

        for(auto j:p){
            j->pos.first=height-((height-100)/(dep+1)*i+50);
            j->pos.second=(width-80)/((int)p.size()+1)*(cnt+1)+40;
            ++cnt;
         //   cerr<<j->pos.first<<" "<<j->pos.second<<endl;
            MyPushButton *btn=new MyPushButton("",canvas,total);
            btn->id=total;
            btn->move(j->pos.second,j->pos.first);
            if(total==0)
                btn->setStyleSheet(Button_Style::START_BTN);
            else
            if(j->type==1)
                btn->setStyleSheet(Button_Style::NORMALFIGHT_BTN);
            else if(j->type==0)
                btn->setStyleSheet(Button_Style::SHOP_BTN);
            else if(j->type==2)
                btn->setStyleSheet(Button_Style::MYSTERY_BTN);
            else if(j->type==3)
                btn->setStyleSheet(Button_Style::REPAIR_BTN);
            if(j->type==1){
                QObject::connect(btn, &QPushButton::clicked, [game_windows,btn]() {
                // 创建战斗，根据
              //  cerr<<"!!!!!"<<endl;
                    next_node=btn->id;
                    game_windows->resetbattle(btn->id);
                    game_windows->StackedPage->setCurrentIndex(0);
                });
            }
            else if(j->type==0){
                QObject::connect(btn, &QPushButton::clicked, [game_windows,btn]() {
                    // 商店
                    next_node=btn->id;
                    qobject_cast<StoreWidget*>(game_windows->StackedPage->widget(4))->id=btn->id;
                    game_windows->StackedPage->setCurrentIndex(4);
                });
            }else if(j->type==2){
                QObject::connect(btn,&QPushButton::clicked, [game_windows,btn/**/](){
                    next_node=btn->id;
                    game_windows->startStory(availableStoryIds[rnd() % availableStoryIds.size()]);
                });
            }else if(j->type==3){
                QObject::connect(btn, &QPushButton::clicked, [game_windows, btn]() {
                    cerr<<"oooo"<<next_node<<endl;
                    next_node = btn->id;
                    cerr<<"isok???"<<next_node<<endl;
                    game_windows->switchToRepair(btn->id);
                });
            }
            total+=1;
        }
    }
    scroll_map->setWidget(canvas);
    scroll_map->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll_map->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

//    QScrollBar *bar=scroll_map->verticalScrollBar();
  //  bar->setValue(bar->maximum());
    return scroll_map;
}
void Map_generator::MyCanvas::refresh(){// 返回主界面的时候要更新
    cerr<<"????"<<endl;
    auto kids=children();
    cerr<<"????"<<endl;
    auto &mp=my_map;
    cerr<<"!!!!"<<endl;
    enum NodeType{
        CANVISIT,VISITED,BLOCKED,CURRENT
    };
    QPushButton *foundButton = findChild<QPushButton*>("money_count");
    if (foundButton) {
        foundButton->setText(QString("%1").arg(money_count));
    }
    QLabel *healthLabel = findChild<QLabel*>("health_label");
    if (healthLabel) {
        healthLabel->setText(QString("❤️ %1/%2").arg(health).arg(maxhealth));
    }
    for(QObject *obj:kids){
        if(MyPushButton *btn = qobject_cast<MyPushButton*>(obj)){
            NodeType nodetype=BLOCKED;
            if(btn->id==0){
                if(mp.currentpos==0){
                    btn->setStyleSheet(Button_Style::START_BTN_SL.arg("#FF0000"));
                }else {
                    btn->setStyleSheet(Button_Style::START_BTN);
                }
                continue;
            }
            if(mp.currentpos==btn->id)
                nodetype=CURRENT;
            if(nodetype==BLOCKED)
                for(auto &j:mp.visited_node){
                    if(btn->id==j){
                        nodetype=VISITED;
                        break;
                    }
                }
            if(nodetype==BLOCKED)
                for(auto &j:mp.nodes[mp.currentpos]->nxt){
                    if(btn->id==j){
                        nodetype=CANVISIT;
                        break;
                    }
                }
            if(nodetype==BLOCKED){
                btn->blockSignals(true);
                if(mp.nodes[btn->id]->type==1)
                    btn->setStyleSheet(Button_Style::NORMALFIGHT_BTN);
                else if(mp.nodes[btn->id]->type==0){
                    btn->setStyleSheet(Button_Style::SHOP_BTN);
                }else if(mp.nodes[btn->id]->type==2){
                    btn->setStyleSheet(Button_Style::MYSTERY_BTN);
                }else if (mp.nodes[btn->id]->type == 3)
                    btn->setStyleSheet(Button_Style::REPAIR_BTN);
            }
            if(nodetype==VISITED){
                btn->blockSignals(true);
                if(mp.nodes[btn->id]->type==1)
                    btn->setStyleSheet(Button_Style::NORMALFIGHT_BTN);
                else if(mp.nodes[btn->id]->type==0){
                    btn->setStyleSheet(Button_Style::SHOP_BTN);
                }else if(mp.nodes[btn->id]->type==2){
                    btn->setStyleSheet(Button_Style::MYSTERY_BTN);
                }else if (mp.nodes[btn->id]->type == 3)
                    btn->setStyleSheet(Button_Style::REPAIR_BTN);
            }
            if(nodetype==CURRENT){
                btn->blockSignals(true);
                if(mp.nodes[btn->id]->type==1)
                    btn->setStyleSheet(Button_Style::NORMALFIGHT_BTN_SL.arg("#FF0000"));
                else if(mp.nodes[btn->id]->type==0){
                    btn->setStyleSheet(Button_Style::SHOP_BTN_SL.arg("#FF0000"));
                }else if(mp.nodes[btn->id]->type==2){
                    btn->setStyleSheet(Button_Style::MYSTERY_BTN_SL.arg("#FF0000"));
                }else if (mp.nodes[btn->id]->type == 3)
                    btn->setStyleSheet(Button_Style::REPAIR_BTN_SL.arg("#FF0000"));
            }
            if(nodetype==CANVISIT){
                btn->blockSignals(false);
                if(mp.nodes[btn->id]->type==1)
                    btn->setStyleSheet(Button_Style::NORMALFIGHT_BTN_SL.arg("#FFFF00"));
                else if(mp.nodes[btn->id]->type==0){
                    btn->setStyleSheet(Button_Style::SHOP_BTN_SL.arg("#FFFF00"));
                }else if(mp.nodes[btn->id]->type==2){
                    btn->setStyleSheet(Button_Style::MYSTERY_BTN_SL.arg("#FFFF00"));
                }else if (mp.nodes[btn->id]->type == 3)
                    btn->setStyleSheet(Button_Style::REPAIR_BTN_SL.arg("#FFFF00"));
            }
        }
    }
}




QJsonObject Map_generator::Map::toJson() const {
    QJsonObject obj;
    obj["dep"] = dep;
    obj["seed"] = seed;
    obj["currentpos"] = currentpos;
    QJsonArray visitedArray;
    for (int v : visited_node) visitedArray.append(v);
    obj["visited"] = visitedArray;

    QJsonArray nodesArray;
    for (const auto& node : nodes) {
        if (!node) continue;
        QJsonObject nodeObj;
        nodeObj["type"] = node->type;
        nodeObj["depth"] = node->depth;
        nodeObj["id"] = node->id;
        nodeObj["encounterId"] = node->encounterId;
        QJsonArray nxtArray;
        for (int n : node->nxt) nxtArray.append(n);
        nodeObj["nxt"] = nxtArray;
        nodesArray.append(nodeObj);
    }
    obj["nodes"] = nodesArray;
    return obj;
}


void Map_generator::Map::fromJson(const QJsonObject& obj) {
    dep = obj["dep"].toInt();
    seed = obj["seed"].toInt();
    currentpos = obj["currentpos"].toInt();
    visited_node.clear();
    for (auto v : obj["visited"].toArray()) visited_node.push_back(v.toInt());

    nodes.clear();
    QJsonArray nodesArray = obj["nodes"].toArray();
    for (const auto& val : nodesArray) {
        QJsonObject nodeObj = val.toObject();
        auto node = std::make_shared<Map_Node>();
        node->type = nodeObj["type"].toInt();
        node->depth = nodeObj["depth"].toInt();
        node->id = nodeObj["id"].toInt();
        node->encounterId = nodeObj["encounterId"].toInt();
        for (auto n : nodeObj["nxt"].toArray()) {
            node->nxt.push_back(n.toInt());
        }
        nodes.push_back(node);
    }
}
