#ifndef STORYNODE_H
#define STORYNODE_H
#include <QString>
#include <QList>
struct StoryEffect {
QString type;   // 效果类型，如 "addHealth", "addMoney", "addRelic", "startBattle" 等
int value;      // 数值（正值表示增加，负值表示减少）
// 可额外添加参数，比如物品ID等，用 QVariantMap 扩展
};
struct StoryChoice {
    QString text;                  // 按钮文字
    int target;                    // 目标节点ID，-1表示终止
    QList<StoryEffect> effects;    // 该选项触发的效果列表
};


struct StoryNode {
    int id;
    QString text;
    QList<StoryChoice> choices;  // <按钮文字, 目标节点ID>
};

#endif // STORYNODE_H
