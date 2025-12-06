#ifndef AICONFIGMANAGER_H
#define AICONFIGMANAGER_H

#include <QString>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QStringList>
#include <QTextStream>

class AIConfigManager
{
public:
    enum ServiceType
    {
        Aliyun,
        OpenAI
    };

    static AIConfigManager &instance()
    {
        static AIConfigManager instance;
        return instance;
    }

    // 获取服务类型
    ServiceType getServiceType();
    // 设置服务类型
    void setServiceType(ServiceType type);

    // 获取API密钥（根据服务类型）
    QString getApiKey();
    // 设置API密钥（根据服务类型）
    void setApiKey(const QString &key);

    // 获取模型名称（根据服务类型）
    QString getModelName();
    // 获取OpenAI端点
    QString getEndpoint();

private:
    AIConfigManager();
    QString m_configPath;

    // 手动解析INI文件
    QMap<QString, QMap<QString, QString>> parseIniFile(const QString &filePath);
    // 保存INI文件
    void saveIniFile(const QString &filePath, const QMap<QString, QMap<QString, QString>> &config);
    
    // 将ServiceType转换为字符串
    QString serviceTypeToString(ServiceType type) const;
    // 将字符串转换为ServiceType
    ServiceType stringToServiceType(const QString &typeStr) const;
    
    // 从配置中获取值
    QString getConfigValue(const QString &section, const QString &key, const QString &defaultValue = "");
    // 设置配置值
    void setConfigValue(const QString &section, const QString &key, const QString &value);
    
    // 当前配置缓存
    QMap<QString, QMap<QString, QString>> m_configCache;
};

#endif // AICONFIGMANAGER_H
