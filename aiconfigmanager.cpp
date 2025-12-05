#include "aiconfigmanager.h"

AIConfigManager::AIConfigManager() {
    // 配置文件将生成在可执行文件同级目录下，名为 config.ini
    m_configPath = QCoreApplication::applicationDirPath() + "/config.ini";

    // 如果文件不存在，创建一个默认的
    if (!QFile::exists(m_configPath)) {
        QSettings settings(m_configPath, QSettings::IniFormat);
        settings.setValue("QianWen/ApiKey", "sk-c30504bf26dd4dfebb3342e7f2a9af4d");
        settings.setValue("QianWen/Model", "qwen-vl-max");
        settings.sync();
    }
}

QString AIConfigManager::getApiKey() {
    QSettings settings(m_configPath, QSettings::IniFormat);
    return settings.value("QianWen/ApiKey", "").toString();
}

void AIConfigManager::setApiKey(const QString& key) {
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue("QianWen/ApiKey", key);
}

QString AIConfigManager::getModelName() {
    QSettings settings(m_configPath, QSettings::IniFormat);
    return settings.value("QianWen/Model", "qwen-vl-max").toString();
}
