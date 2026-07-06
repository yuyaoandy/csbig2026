#ifndef SAVELOADMANAGER_H
#define SAVELOADMANAGER_H

#include <QString>
#include <QJsonObject>
#include<QScrollArea>
#include"map_gen.h"
#include<QFile>
QScrollArea* drawmap(const Map_generator::Map& x, QWidget *parent = nullptr);
// 前置声明
class InGame;
class BlockTowerWidget;
namespace Map_generator { class MyCanvas; }

class SaveLoadManager
{
public:
    // 检查存档文件是否存在
    static bool hasSaveFile();

    // 保存当前游戏状态（由 InGame 调用）
    static bool saveGame(InGame* game);

    // 加载存档（由 InGame 调用）
    static bool loadGame(InGame* game);
    static bool deleteSaveFile() {
        return QFile::remove("savegame.json");
    }
private:
    // 辅助函数：从 InGame 中提取数据
    static QJsonObject serializeGlobalState();
    static QJsonObject serializeMap(InGame* game);
    static QJsonObject serializeCardInventory(InGame* game);

    // 辅助函数：反序列化到 InGame
    static bool deserializeGlobalState(const QJsonObject& root);
    static bool deserializeMap(InGame* game, const QJsonObject& mapObj);
    static bool deserializeCardInventory(InGame* game, const QJsonObject& cardObj);
};

#endif // SAVELOADMANAGER_H