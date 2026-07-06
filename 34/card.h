// card.h
#ifndef CARD_H
#define CARD_H

#include <QString>

struct Card {
    int id;
    QString name;
    QString chinese;
    int rarity;
    int difficulty;
    QString description;
    int max_level;
    int energy;
};
#endif