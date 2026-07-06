#include "saveloadmanager.h"
#include "gamewindow.h"          // 为了 InGame 完整定义
#include "blocktowerwidget.h"
#include "map_gen.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonArray>
#include <QScrollArea>

// 外部全局变量声明（在 gamewindow.cpp 或其他地方定义）
extern int health;
extern int maxhealth;
extern long long money_count;
extern int next_node;
extern int difficulty;
extern QVector<int> relic;

bool SaveLoadManager::hasSaveFile()
{
    return QFile::exists("savegame.json");
}

bool SaveLoadManager::saveGame(InGame* game)
{
    if (!game) return false;
    QJsonObject root = serializeGlobalState();
    QScrollArea* mapArea = game->map_interface;
    if (mapArea) {
        auto canvas = qobject_cast<Map_generator::MyCanvas*>(mapArea->widget());
        if (canvas) {
            root["map"] = canvas->my_map.toJson();
        }
    }
    BlockTowerWidget* tower = game->coding_interface;
    if (tower) {
        root["card_inventory"] = tower->toJson();
    }
    root["floor"]=game->currentFloor;
    QFile file("savegame.json");
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(game, "保存失败", "无法写入存档文件");
        return false;
    }
    file.write(QJsonDocument(root).toJson());
    file.close();
    QMessageBox::information(game, "保存成功", "游戏已保存");
    return true;
}

bool SaveLoadManager::loadGame(InGame* game)
{
    cerr<<"loadinggame"<<endl;
    if (!game) return false;
    cerr<<"????"<<endl;
    QFile file("savegame.json");
    if (!file.open(QIODevice::ReadOnly)) return false;
    cerr<<"open"<<endl;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (doc.isNull()) return false;

    QJsonObject root = doc.object();
    if (!deserializeGlobalState(root)) return false;
    if (root.contains("map")) {
        if (!deserializeMap(game, root["map"].toObject())) return false;
    }
    if (root.contains("card_inventory")) {
        if (!deserializeCardInventory(game, root["card_inventory"].toObject())) return false;
    }
    game->currentFloor=root["floor"].toInt();
    if (game->map_interface) {
        auto canvas = qobject_cast<Map_generator::MyCanvas*>(game->map_interface->widget());
        if (canvas) canvas->refresh();
    }
    cerr<<"load_success"<<endl;
    return true;
}

// ---------- 私有辅助实现 ----------

QJsonObject SaveLoadManager::serializeGlobalState()
{
    QJsonObject obj;
    obj["health"] = health;
    obj["maxhealth"] = maxhealth;
    obj["money"] = (qint64)money_count;
    obj["next_node"] = next_node;
    obj["difficulty"]=difficulty;
    QJsonArray relicArray;
    for (int id : relic) {
        relicArray.append(id);
    }
    obj["relic"] = relicArray;
    return obj;
}

QJsonObject SaveLoadManager::serializeMap(InGame* game)
{
    QJsonObject empty;
    QScrollArea* mapArea = game->map_interface;
    if (mapArea) {
        auto canvas = qobject_cast<Map_generator::MyCanvas*>(mapArea->widget());
        if (canvas) {
            return canvas->my_map.toJson();
        }
    }
    return empty;
}

QJsonObject SaveLoadManager::serializeCardInventory(InGame* game)
{
    QJsonObject empty;
    BlockTowerWidget* tower = game->coding_interface;
    if (tower) {
        return tower->toJson();
    }
    return empty;
}

bool SaveLoadManager::deserializeGlobalState(const QJsonObject& root)
{
    if (!root.contains("health") || !root.contains("maxhealth") ||
        !root.contains("money") || !root.contains("next_node")) {
        return false;
    }
    health = root["health"].toInt();
    maxhealth = root["maxhealth"].toInt();
    money_count = root["money"].toVariant().toLongLong();
    next_node = root["next_node"].toInt();
    difficulty=root["difficulty"].toInt();
    relic.clear();
    QJsonArray relicArray = root["relic"].toArray();
    for (auto val : relicArray) {
        relic.append(val.toInt());
    }
    return true;
}

bool SaveLoadManager::deserializeMap(InGame* game, const QJsonObject& mapObj)
{
    Map_generator::Map loadedMap;
    loadedMap.fromJson(mapObj);
    QScrollArea* newMap = drawmap(loadedMap, game);
    game->setMap(newMap);
    // 强制刷新
    game->refresh();
    return true;
}

bool SaveLoadManager::deserializeCardInventory(InGame* game, const QJsonObject& cardObj)
{
    BlockTowerWidget* tower = game->coding_interface;
    if (!tower) return false;
    tower->fromJson(cardObj);
    return true;
}