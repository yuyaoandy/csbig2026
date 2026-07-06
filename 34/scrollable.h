#ifndef SCROLLABLE_H
#define SCROLLABLE_H
#include<QWidget>
#include<QScrollArea>
#include<QPainter>



class Scrollable:public QWidget{
public:
    Scrollable(QWidget *parent=nullptr):QWidget(parent){
        QScrollArea *scrollArea=new QScrollArea(this);
        MyCanvas *canvas=new MyCanvas(this);
        scrollArea->setWidget(canvas);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    void show(){

    }
};

#endif // SCROLLABLE_H
