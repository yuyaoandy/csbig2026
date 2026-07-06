#ifndef STORYWIDGET_H
#define STORYWIDGET_H
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "storynode.h"
class StoryNodeWidget : public QWidget {
    Q_OBJECT
public:
    explicit StoryNodeWidget(const StoryNode &node, QWidget *parent = nullptr);

signals:
    void nodeSelected(int target);
    void terminate();
    void effectsTriggered(const QList<StoryEffect>& effects);//预留 效果
};
#endif