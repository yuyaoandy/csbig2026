#include "storyloader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include<iostream>

QMap<int, StoryNode> StoryLoader::loadFromFile(const QString &filePath, QString *errorMsg) {
    QMap<int, StoryNode> graph;

    QFile file(filePath);
    std::cerr<<filePath.toStdString()<<"\n";
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMsg) *errorMsg = QString("无法打开文件: %1").arg(filePath);
        return graph;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (errorMsg) *errorMsg = QString("JSON 解析错误: %1").arg(parseError.errorString());
        return graph;
    }

    if (!doc.isObject()) {
        if (errorMsg) *errorMsg = "?root";
        return graph;
    }

    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("nodes") || !rootObj["nodes"].isArray()) {
        if (errorMsg) *errorMsg = "?'nodes'";
        return graph;
    }

    QJsonArray nodesArray = rootObj["nodes"].toArray();
    std::cerr<<nodesArray.size()<<"\n";
    for (const QJsonValue &val : nodesArray) {
        if (!val.isObject()) {
            if (errorMsg) *errorMsg = "不是obj";
            continue;
        }

        QJsonObject nodeObj = val.toObject();
        if (!nodeObj.contains("id") || !nodeObj["id"].isDouble()) {
            if (errorMsg) *errorMsg = " 'id' @@@";
            continue;
        }
        if (!nodeObj.contains("text") || !nodeObj["text"].isString()) {
            if (errorMsg) *errorMsg = "'text' ";
            continue;
        }
        if (!nodeObj.contains("choices") || !nodeObj["choices"].isArray()) {
            if (errorMsg) *errorMsg = "'choices' ";
            continue;
        }

        StoryNode node;
        node.id = nodeObj["id"].toInt();
        node.text = nodeObj["text"].toString();

        QJsonArray choicesArray = nodeObj["choices"].toArray();
        for (const QJsonValue &choiceVal : choicesArray) {
            QJsonObject choiceObj = choiceVal.toObject();
            StoryChoice choice;
            choice.text = choiceObj["text"].toString();
            choice.target = choiceObj["target"].toInt();

            // 解析 effects
            if (choiceObj.contains("effects") && choiceObj["effects"].isArray()) {
                QJsonArray effectsArray = choiceObj["effects"].toArray();
                for (const QJsonValue &effVal : effectsArray) {
                    if (!effVal.isObject()) continue;
                    QJsonObject effObj = effVal.toObject();
                    StoryEffect effect;
                    effect.type = effObj["type"].toString();
                    effect.value = effObj["value"].toInt();
                    choice.effects.append(effect);
                }
            }
            node.choices.append(choice);
        }
        graph[node.id] = node;


        graph[node.id] = node;
    }

    return graph;
}