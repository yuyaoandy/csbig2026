// jsonreader.cpp
#include "jsonreader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

QVector<Card> JsonReader::loadCardsFromFile(const QString& filePath) {
    QVector<Card> cards;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        return cards;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString();
        return cards;
    }

    if (!doc.isObject()) {
        qWarning() << "JSON 根节点不是对象";
        return cards;
    }

    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("card") || !rootObj["card"].isArray()) {
        qWarning() << "JSON 中缺少 'card' 数组";
        return cards;
    }

    QJsonArray cardArray = rootObj["card"].toArray();
    for (const QJsonValue& val : cardArray) {
        if (!val.isObject()) continue;

        QJsonObject obj = val.toObject();

        Card card;
        card.id          = obj["id"].toInt();
        card.name        = obj["name"].toString();
        card.chinese     = obj["chinese"].toString();
        card.rarity      = obj["rarity"].toInt();
        card.difficulty  = obj["difficulty"].toInt();
        card.description = obj["description"].toString();
        card.max_level   = obj["max_level"].toInt();
        card.energy      = obj["energy"].toInt();

        cards.append(card);
    }

    return cards;
}


QMap<int, QString> JsonReader::loadRelicDescriptions(const QString& filePath) {
    QMap<int, QString> map;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开遗物描述文件:" << filePath;
        return map;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString();
        return map;
    }

    if (!doc.isObject()) {
        qWarning() << "JSON 根节点不是对象";
        return map;
    }

    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("relic") || !rootObj["relic"].isArray()) {
        qWarning() << "JSON 中缺少 'relic' 数组";
        return map;
    }

    QJsonArray relicArray = rootObj["relic"].toArray();
    for (const QJsonValue& val : relicArray) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        int id = obj["id"].toInt();
        QString desc = obj["description"].toString();
        if (id != 0 && !desc.isEmpty()) {
            map[id] = desc;
        }
    }
    return map;
}
QVector<EnemyData> JsonReader::loadEnemiesFromFile(const QString& filePath) {
    QVector<EnemyData> enemies;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开敌人文件:" << filePath;
        return enemies;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString();
        return enemies;
    }
    if (!doc.isObject()) {
        qWarning() << "JSON 根节点不是对象";
        return enemies;
    }

    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("enemy") || !rootObj["enemy"].isArray()) {
        qWarning() << "JSON 中缺少 'enemy' 数组";
        return enemies;
    }

    QJsonArray enemyArray = rootObj["enemy"].toArray();
    for (const QJsonValue& val : enemyArray) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        EnemyData e;
        e.id = obj["id"].toInt();
        e.name = obj["name"].toString().toStdString();
        e.health = obj["health"].toInt();
        cerr<<"nmsl "<<e.health<<endl;
        e.energy = obj["energy"].toInt();

        QJsonArray codeArray = obj["code"].toArray();
        for (const auto& c : codeArray) {
            if (c.isString()) e.code.push_back(c.toString().toStdString());
        }

        QJsonArray relicArray = obj["relic"].toArray();
        for (const auto& r : relicArray) {
            if (r.isDouble()) e.relic.push_back(r.toInt());
        }

        enemies.append(e);
    }
    return enemies;
}
QVector<EncounterData> JsonReader::loadEncountersFromFile(const QString& filePath) {
    QVector<EncounterData> encounters;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开遭遇文件:" << filePath;
        return encounters;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString();
        return encounters;
    }
    if (!doc.isObject()) return encounters;

    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("encounter") || !rootObj["encounter"].isArray()) {
        qWarning() << "JSON 中缺少 'encounter' 数组";
        return encounters;
    }

    QJsonArray encArray = rootObj["encounter"].toArray();
    for (const QJsonValue& val : encArray) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        EncounterData e;
        e.id = obj["id"].toInt();
        e.name = obj["name"].toString().toStdString();
        e.isBoss = obj["isBoss"].toBool(false);
        e.minDifficulty = obj["minDifficulty"].toInt(1);
        cerr<<"mindif"<<e.minDifficulty<<endl;

        QJsonArray enemiesArray = obj["enemies"].toArray();
        for (auto v : enemiesArray) {
            if (v.isDouble()) e.enemyIds.push_back(v.toInt());
        }

        encounters.append(e);
    }
    return encounters;
}
