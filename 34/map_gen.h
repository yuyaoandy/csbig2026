#ifndef MAP_GEN_H
#define MAP_GEN_H
#include<bits/stdc++.h>
#include<QApplication>
#include<QGraphicsScene>
#include<QWidget>
#include<QPushButton>
#include <QApplication>
#include <QMainWindow>
#include <QStackedWidget>
#include<QScrollArea>
#include<QPainter>
//#include "scrollable.h"
#include "style.h"
using namespace std;
class MyPushButton:public QPushButton{
    Q_OBJECT
public:
    MyPushButton(QString name="",QWidget* parent=nullptr,int rid=0):QPushButton(name,parent){
        id=rid;
    }
    int id;
};

namespace Map_generator {


class Map_Node{

public:
    int type,depth,id,encounterId,is_vis;
    vector<int> nxt;
    int difficulty;
    pair<int,int> pos;
    Map_Node(int x=0, int dp=0, int i=0, int encId=0, int diff=0)
        : type(x), depth(dp), id(i), encounterId(encId), is_vis(0), difficulty(diff), pos(0,0) {}
    virtual int visit(){
        return 0;
    }
};

class Map{
        public:
            vector<shared_ptr<Map_Node> > nodes;
            int dep,seed;
            int currentpos;
            vector<int> visited_node;
            Map(int dp=0,int sd=0){
                dep=dp;seed=sd;currentpos=0;
            }

            void fromJson(const QJsonObject& obj);
            QJsonObject toJson()const;
};

class MyCanvas:public QWidget{
    Q_OBJECT
public:
   Map my_map;
    MyCanvas (QWidget *parent=nullptr,const Map nmap=Map()):QWidget(parent){
        my_map=nmap;
        setMinimumSize(500,1000);
        // 存一个地图的指针，paintEvent 用
    }
    //vector<Route> route;
    //Node 信息
    void paintEvent(QPaintEvent *)override {
        QPainter painter(this);
        QPen pen(Qt::darkGreen);
        pen.setWidth(2);
        pen.setDashPattern({3,3});
        painter.setPen(pen);
        painter.fillRect(this->rect(),QColor(163,135,92));
       // my_map->
        for(auto &i:my_map.nodes){
            for(auto &j:i->nxt){
                auto &p=my_map.nodes[j];
                int stx=i->pos.first+20,sty=i->pos.second+20;
                int edx=p->pos.first+20,edy=p->pos.second+20;
                painter.drawLine(sty,stx,edy,edx);
            }
        }
    }
    void refresh();
signals:
    void nodeClicked(int nodeId);
};


}; // namespace Map_generator

#endif // MAP_GEN_H
