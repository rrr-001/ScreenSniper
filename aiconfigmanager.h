#ifndef AICONFIGMANAGER_H
#define AICONFIGMANAGER_H

#include <QString>
#include <QSettings>
#include <QCoreApplication>
#include <QFile>

class AIConfigManager {
public:
    static AIConfigManager& instance() {
        static AIConfigManager instance;
        return instance;
    }

    QString getApiKey();
    void setApiKey(const QString& key);
    QString getModelName(); // 例如 qwen-vl-max

private:
    AIConfigManager();
    QString m_configPath;
};

#endif // AICONFIGMANAGER_H
