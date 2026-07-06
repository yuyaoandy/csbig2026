// jsonreader.h
#ifndef JSONREADER_H
#define JSONREADER_H

#include <QVector>
#include "card.h"
#include "battlecore.h"

class JsonReader {
public:
    static QVector<Card> loadCardsFromFile(const QString& filePath);
    static QMap<int, QString> loadRelicDescriptions(const QString& filePath);
    static QVector<EnemyData> loadEnemiesFromFile(const QString& filePath);
    static QVector<EncounterData> loadEncountersFromFile(const QString& filePath);
};

#endif // JSONREADER_H