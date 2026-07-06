#ifndef STORYLOADER_H
#define STORYLOADER_H

#include <QMap>
#include <QString>
#include "storynode.h"  // 包含 StoryNode 定义

class StoryLoader {
public:
    static QMap<int, StoryNode> loadFromFile(const QString &filePath, QString *errorMsg = nullptr);
};


#endif // STORYLOADER_H